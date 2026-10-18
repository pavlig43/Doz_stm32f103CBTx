"""Отчёт о размере прошивки GNU Arm GCC. Нужен только путь к .map.

Запуск: python firmware_size.py "C:\\project\\build\\Release\\firmware.map"
Без аргумента скрипт сам попросит путь. Отчёт сохраняется рядом со скриптом.
В Windows можно запускать двойным щелчком: после отчёта окно ждёт Enter.
Использует только стандартную библиотеку Python.
"""

from __future__ import annotations

from collections import defaultdict
from dataclasses import dataclass, field
from pathlib import Path
import re
import struct
import sys


HEX = r"0x([0-9a-fA-F]+)"
POSITION = re.compile(rf"^\s+{HEX}\s+{HEX}\s+(.+?)\s*$")
OUTPUT = re.compile(rf"^(\.[\w.$-]+)\s+{HEX}\s+{HEX}(?:\s+load address {HEX})?")
INPUT = re.compile(rf"^ (\S+)\s+{HEX}\s+{HEX}\s+(.+?)\s*$")
NAME_ONLY = re.compile(r"^( ?)(\.[\w.$-]+)\s*$")
GROUP_HAL = "STM32 HAL"
GROUP_PROJECT = "Код проекта"
GROUP_LIBC = "Библиотека C: строки, числа, память"
GROUP_MATH = "Библиотеки арифметики"
GROUP_CONSTANTS = "Строки и константы (общие)"
GROUP_RUNTIME = "Запуск, прочий код и выравнивание"


@dataclass
class Region:
    name: str
    origin: int
    capacity: int
    used: int = 0

    def contains(self, address: int) -> bool:
        return self.origin <= address < self.origin + self.capacity


@dataclass
class Part:
    name: str
    address: int
    size: int
    owner: str


@dataclass
class Section:
    name: str
    address: int
    size: int
    load_address: int | None = None
    parts: list[Part] = field(default_factory=list)


@dataclass
class MapData:
    regions: list[Region]
    sections: list[Section]
    reserves: dict[str, int]


def has_load_bytes(section: Section) -> bool:
    # GNU ld may show an LMA even for zero-filled RAM and stack/heap reserves.
    # These sections occupy RAM but do not have stored initial values in Flash.
    return not re.search(r"bss|noinit|heap|stack", section.name, re.I)


def parse_map(text: str) -> MapData:
    try:
        memory, contents = text.split("Linker script and memory map", 1)
        memory = memory.split("Memory Configuration", 1)[1]
    except (ValueError, IndexError) as error:
        raise ValueError("Это не карта GNU ld: нет таблицы памяти или списка секций.") from error

    regions = []
    for line in memory.splitlines():
        match = re.match(rf"^(\S+)\s+{HEX}\s+{HEX}(?:\s+\S+)?\s*$", line)
        if match and match[1] != "*default*":
            regions.append(Region(match[1], int(match[2], 16), int(match[3], 16)))
    if not regions:
        raise ValueError("В карте не найдены области памяти.")

    sections = []
    current = None
    pending_output = None
    pending_input = None
    reserves = {}
    for line in contents.splitlines():
        reserve = re.match(rf"^\s+{HEX}\s+(_Min_(?:Stack|Heap)_Size)\s*=", line)
        if reserve:
            reserves[reserve[2]] = int(reserve[1], 16)
        output = OUTPUT.match(line)
        if output:
            current = Section(output[1], int(output[2], 16), int(output[3], 16),
                              int(output[4], 16) if output[4] else None)
            sections.append(current)
            pending_output = pending_input = None
            continue
        name = NAME_ONLY.match(line)
        if name:
            if name[1] == "":
                pending_output = name[2]
                pending_input = None
                current = None
            else:
                pending_input = name[2]
            continue
        position = POSITION.match(line)
        if pending_output and position:
            load = re.search(rf"load address {HEX}", position[3])
            current = Section(pending_output, int(position[1], 16), int(position[2], 16),
                              int(load[1], 16) if load else None)
            sections.append(current)
            pending_output = None
            continue
        # A wrapped output header may have only address and size, without an owner.
        bare_position = re.match(rf"^\s+{HEX}\s+{HEX}\s*$", line)
        if pending_output and bare_position:
            current = Section(pending_output, int(bare_position[1], 16),
                              int(bare_position[2], 16))
            sections.append(current)
            pending_output = None
            continue
        part = INPUT.match(line)
        if current and part:
            current.parts.append(Part(part[1], int(part[2], 16), int(part[3], 16), part[4]))
            pending_input = None
        elif current and pending_input and position:
            current.parts.append(Part(pending_input, int(position[1], 16),
                                      int(position[2], 16), position[3]))
            pending_input = None

    allocated = []
    for section in sections:
        if not section.size or section.name.startswith((".debug", ".comment", ".ARM.attributes")):
            continue
        destinations = [section.address]
        if (has_load_bytes(section) and section.load_address is not None
                and section.load_address != section.address):
            destinations.append(section.load_address)
        present = False
        for address in destinations:
            for region in regions:
                if region.contains(address):
                    region.used = max(region.used, address + section.size - region.origin)
                    present = True
        if present:
            allocated.append(section)
    if not allocated:
        raise ValueError("В карте не найдены секции, размещённые в памяти устройства.")
    return MapData(regions, allocated, reserves)


def group_for(owner: str) -> str:
    if "STM32" in owner and "HAL_Driver" in owner:
        return GROUP_HAL
    if "libgcc.a(" in owner or "libm.a(" in owner:
        return GROUP_MATH
    if re.search(r"lib(?:g|c)(?:_nano)?\.a\(", owner):
        return GROUP_LIBC
    project_path = owner.replace("\\", "/")
    if re.search(r"(?:^|/)(?:Core/Src|app(?:/[^/]+)?|lib/[^/]+)/[^/]+\.c\.(?:obj|o)$", project_path):
        return GROUP_PROJECT
    return GROUP_RUNTIME


def owner_label(owner: str) -> str:
    owner = owner.replace("\\", "/")
    archive = re.search(r"([^/]+\.a)\(([^)]+)\)$", owner)
    if archive:
        return f"{archive[1]} / {archive[2]}"
    return owner.rsplit("/", 1)[-1].removesuffix(".obj")


def consume(intervals: list[tuple[int, int]], start: int, end: int) -> int:
    """Count each byte once, including maps with overlapping input sections."""
    remaining = []
    count = 0
    for left, right in intervals:
        overlap_start, overlap_end = max(left, start), min(right, end)
        if overlap_start < overlap_end:
            count += overlap_end - overlap_start
            if left < overlap_start:
                remaining.append((left, overlap_start))
            if overlap_end < right:
                remaining.append((overlap_end, right))
        else:
            remaining.append((left, right))
    intervals[:] = remaining
    return count


def flash_details(data: MapData, region: Region):
    groups = defaultdict(int)
    objects = defaultdict(int)
    for section in data.sections:
        if not has_load_bytes(section):
            continue
        location = section.load_address if section.load_address is not None else section.address
        if not region.contains(location):
            continue
        if section.name.startswith((".rodata", ".srodata")):
            # GNU ld can print the original sizes of strings merged into another input.
            # The output section is authoritative; never sum its input string sizes.
            groups[GROUP_CONSTANTS] += section.size
            continue
        remaining = [(section.address, section.address + section.size)]
        for part in section.parts:
            if not part.owner.endswith((".obj", ".o", ".o)")):
                continue
            size = consume(remaining, part.address, part.address + part.size)
            groups[group_for(part.owner)] += size
            objects[part.owner] += size
    attributed = sum(groups.values())
    groups[GROUP_RUNTIME] += region.used - attributed
    return groups, objects


def functions_from_elf(path: Path, data: MapData, region: Region):
    """Read exact function sizes from a matching ELF32 without external tools."""
    raw = path.read_bytes()
    if raw[:6] != b"\x7fELF\x01\x01":
        raise ValueError("поддерживается ELF32 с порядком байтов little-endian")
    header = struct.unpack_from("<HHIIIIIHHHHHH", raw, 16)
    offset, entry_size, count, names_index = header[5], header[10], header[11], header[12]
    headers = [struct.unpack_from("<IIIIIIIIII", raw, offset + i * entry_size)
               for i in range(count)]
    names = headers[names_index]
    section_names = raw[names[4]:names[4] + names[5]]

    def name_at(table: bytes, index: int) -> str:
        return table[index:].split(b"\0", 1)[0].decode("utf-8", errors="replace")

    map_sections = {section.name: section for section in data.sections}
    matched = 0
    for section in headers:
        name = name_at(section_names, section[0])
        if name in (".text", ".rodata", ".data", ".bss") and name in map_sections:
            expected = map_sections[name]
            if (section[3], section[5]) != (expected.address, expected.size):
                raise ValueError("ELF и карта относятся к разным сборкам")
            matched += 1
    if not matched:
        raise ValueError("не удалось сопоставить ELF с картой")

    functions = {}
    for section in headers:
        if section[1] != 2 or section[9] < 16:  # SHT_SYMTAB
            continue
        strings = headers[section[6]]
        table = raw[strings[4]:strings[4] + strings[5]]
        for position in range(section[4], section[4] + section[5], section[9]):
            name, value, size, info, _, index = struct.unpack_from("<IIIBBH", raw, position)
            address = value & ~1  # ARM Thumb bit is not part of the address.
            if info & 15 != 2 or index == 0 or not size or not region.contains(address):
                continue
            key = (address, size)
            function_name = name_at(table, name)
            if key not in functions:
                owner = next((part.owner for output in data.sections for part in output.parts
                              if part.address <= address < part.address + part.size), "—")
                functions[key] = [function_name, size, owner]
            elif function_name not in functions[key][0].split(", "):
                functions[key][0] += ", " + function_name
    return sorted(functions.values(), key=lambda item: (-item[1], item[0]))


def number(value: int) -> str:
    return f"{value:,}".replace(",", " ")


def cell(value: str) -> str:
    return value.replace("|", "\\|").replace("\n", " ")


def render_report(path: Path, data: MapData) -> str:
    lines = ["# Размер прошивки", "", f"Карта: `{path}`", "",
             "Все размеры — в байтах. 1 КБ = 1024 байта.", "",
             "## Память", "", "| Область | Занято, байт | КБ | Всего, байт | Занято | Свободно, байт |",
             "| --- | ---: | ---: | ---: | ---: | ---: |"]
    for region in data.regions:
        percent = region.used / region.capacity * 100 if region.capacity else 0
        lines.append(f"| {cell(region.name)} | **{number(region.used)}** | {region.used / 1024:.2f} | "
                     f"{number(region.capacity)} | {percent:.2f}% | {number(region.capacity - region.used)} |")
    lines += ["", "Занятое место рассчитано до конца последней секции, с учётом промежутков "
              "и выравнивания. Начальные значения переменных в RAM также занимают Flash.", ""]
    if data.reserves:
        stack = data.reserves.get("_Min_Stack_Size", 0)
        heap = data.reserves.get("_Min_Heap_Size", 0)
        lines += [f"В RAM уже включены заданные скриптом сборки резервы: стек — **{number(stack)} байт**, "
                  f"куча — **{number(heap)} байт**. Это не замер расхода памяти при работе.", ""]
    flashes = [region for region in data.regions if re.search(r"FLASH|ROM", region.name, re.I)]
    if not flashes:
        lines += ["Не найдена область с именем FLASH или ROM; подробный разбор Flash недоступен.", ""]
    for region in flashes:
        groups, objects = flash_details(data, region)
        lines += [f"## Что занимает {region.name}", "", "| Группа | Байт | Доля занятого места |",
                  "| --- | ---: | ---: |"]
        for label, size in sorted(groups.items(), key=lambda item: -item[1]):
            if size:
                lines.append(f"| {label} | {number(size)} | {size / region.used * 100:.2f}% |")
        lines += [f"| **Всего** | **{number(region.used)}** | **100%** |", "",
                  "## Модули и части библиотек", "",
                  "Здесь учтены код и начальные значения переменных. Общие строки и константы "
                  "учтены отдельно в таблице выше: компоновщик может объединять одинаковые строки.", "",
                  "| Модуль | Группа | Flash, байт |", "| --- | --- | ---: |"]
        for owner, size in sorted(objects.items(), key=lambda item: (-item[1], item[0])):
            if size:
                lines.append(f"| {cell(owner_label(owner))} | {group_for(owner)} | {number(size)} |")
        elf = path.with_suffix(".elf")
        reason = "рядом с картой нет файла .elf"
        functions = []
        if elf.is_file():
            try:
                functions = functions_from_elf(elf, data, region)
                reason = ""
            except (ValueError, OSError, IndexError, struct.error) as error:
                reason = str(error)
        lines += ["", "## Самые крупные функции", ""]
        if reason:
            lines += [f"Точные размеры из ELF недоступны: {reason}. Ниже — размеры секций `.text.*` "
                      "из карты; код библиотек в общей секции `.text` здесь не разделён на функции.", ""]
            functions = [[part.name.removeprefix(".text."), part.size, part.owner]
                         for section in data.sections if region.contains(section.address)
                         for part in section.parts if part.name.startswith(".text.") and part.size
                         and section.address <= part.address < section.address + section.size]
            functions.sort(key=lambda item: (-item[1], item[0]))
        else:
            lines += [f"Точные размеры взяты из соседнего `{elf.name}`. Имена, относящиеся к одной "
                      "функции, объединены; её размер посчитан один раз.", ""]
        lines += ["Показаны 30 самых крупных функций. Они уже входят в размеры модулей выше.", "",
                  "| Функция | Байт | Модуль |", "| --- | ---: | --- |"]
        for name, size, owner in functions[:30]:
            lines.append(f"| {cell(name)} | {number(size)} | {cell(owner_label(owner))} |")
    return "\n".join(lines) + "\n"


def generate_report(value: str) -> Path:
    path = Path(value.strip().strip('"').strip("'"))
    if not path.is_absolute():
        raise ValueError("Нужен абсолютный путь к файлу .map.")
    if path.suffix.lower() != ".map":
        raise ValueError("Нужен файл .map, созданный при сборке GNU Arm GCC.")
    text = path.read_text(encoding="utf-8-sig", errors="replace")
    data = parse_map(text)
    report = Path(__file__).resolve().parent / f"{path.stem}_sizes.md"
    report.write_text(render_report(path, data), encoding="utf-8")
    return report


def main() -> int:
    for stream in (sys.stdin, sys.stdout, sys.stderr):
        if hasattr(stream, "reconfigure"):
            stream.reconfigure(encoding="utf-8")
    if len(sys.argv) > 2:
        print("Нужен только один аргумент: абсолютный путь к .map.", file=sys.stderr)
        return 1
    try:
        value = sys.argv[1] if len(sys.argv) == 2 else input("Абсолютный путь к .map: ")
        report = generate_report(value)
    except (ValueError, OSError, EOFError) as error:
        print(f"Ошибка: {error}", file=sys.stderr)
        return 1
    print(f"Отчёт сохранён: {report}")
    return 0


if __name__ == "__main__":
    exit_code = main()
    if len(sys.argv) == 1 and sys.stdin.isatty():
        try:
            input("Нажмите Enter, чтобы закрыть окно...")
        except (EOFError, KeyboardInterrupt):
            pass
    raise SystemExit(exit_code)
