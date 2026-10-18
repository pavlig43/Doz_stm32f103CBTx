from pathlib import Path
import struct
import tempfile
import unittest
from unittest.mock import patch

import firmware_size as size


MAP = """Discarded input sections
 .text.unused 0x00000000 0x1000 unused.o

Memory Configuration
Name Origin Length Attributes
RAM 0x20000000 0x00000100 xrw
FLASH 0x08000000 0x00001000 xr
*default* 0x00000000 0xffffffff

Linker script and memory map
                0x00000008 _Min_Heap_Size = 0x8
                0x00000008 _Min_Stack_Size = 0x8
.text 0x08000000 0x20
 .text.first
                0x08000000 0x10 CMakeFiles/fw.dir/Core/Src/app.c.obj
                0x08000000 first
 .text 0x08000010 0x10 C:/Compiler With Spaces/libgcc.a(math.o)
.rodata 0x08000020 0x8
 .rodata.str1.1 0x08000020 0x8 CMakeFiles/fw.dir/Core/Src/app.c.obj
 .rodata.merged.str1.1 0x08000028 0x90 CMakeFiles/fw.dir/Core/Src/settings.c.obj
                0x90 (size before relaxing)
.data 0x20000000 0x4 load address 0x08000028
 .data.value 0x20000000 0x4 CMakeFiles/fw.dir/Core/Src/main.c.obj
.bss 0x20000004 0x8 load address 0x0800002c
 .bss.values 0x20000004 0x8 CMakeFiles/fw.dir/Core/Src/app.c.obj
._user_heap_stack
                0x2000000c 0x10 load address 0x08000034
.debug_info 0x00000000 0xffffffff
 .debug_info 0x00000000 0xffffffff anything.o
"""


def elf_fixture(text_size=0x20):
    """Two functions and one alias: the alias must not count as another function."""
    section_names = b"\0.shstrtab\0.text\0.strtab\0.symtab\0"
    symbols = b"\0first\0first_alias\0second\0"
    raw = bytearray(768)
    raw[:16] = b"\x7fELF\x01\x01\x01" + bytes(9)
    struct.pack_into("<HHIIIIIHHHHHH", raw, 16,
                     2, 40, 1, 0x08000001, 0, 64, 0, 52, 0, 0, 40, 5, 1)
    headers = [
        (0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
        (1, 3, 0, 0, 320, len(section_names), 0, 0, 1, 0),
        (11, 1, 6, 0x08000000, 400, text_size, 0, 0, 4, 0),
        (17, 3, 0, 0, 480, len(symbols), 0, 0, 1, 0),
        (25, 2, 0, 0, 600, 64, 3, 1, 4, 16),
    ]
    for index, header in enumerate(headers):
        struct.pack_into("<IIIIIIIIII", raw, 64 + index * 40, *header)
    raw[320:320 + len(section_names)] = section_names
    raw[480:480 + len(symbols)] = symbols
    struct.pack_into("<IIIBBH", raw, 616, 1, 0x08000001, 8, 0x12, 0, 2)
    struct.pack_into("<IIIBBH", raw, 632, 7, 0x08000001, 8, 0x12, 0, 2)
    struct.pack_into("<IIIBBH", raw, 648, 19, 0x08000009, 8, 0x12, 0, 2)
    return raw


class MapTests(unittest.TestCase):
    def setUp(self):
        self.data = size.parse_map(MAP)
        self.flash = next(region for region in self.data.regions if region.name == "FLASH")

    def test_totals_include_data_but_not_bss_load_addresses(self):
        ram = next(region for region in self.data.regions if region.name == "RAM")
        self.assertEqual(self.flash.used, 44)
        self.assertEqual(ram.used, 28)
        self.assertEqual(self.data.reserves, {"_Min_Heap_Size": 8, "_Min_Stack_Size": 8})

    def test_merged_strings_and_discarded_code_do_not_inflate_sizes(self):
        groups, objects = size.flash_details(self.data, self.flash)
        self.assertEqual(sum(groups.values()), 44)
        self.assertEqual(groups[size.GROUP_CONSTANTS], 8)
        self.assertEqual(groups[size.GROUP_PROJECT], 20)
        self.assertEqual(groups[size.GROUP_MATH], 16)
        self.assertNotIn("unused.o", objects)
        self.assertFalse(any("settings.c.obj" in owner for owner in objects))

    def test_overlap_counted_once(self):
        intervals = [(0, 16)]
        self.assertEqual(size.consume(intervals, 0, 12), 12)
        self.assertEqual(size.consume(intervals, 8, 20), 4)
        self.assertEqual(intervals, [])

    def test_project_groups_after_module_moves(self):
        for module_path in ("app/app.c.obj", "app/work/work.c.obj", "app/settings/settings.c.obj",
                            "app/doz_fee/doz_fee.c.obj", "lib/hx/hx.c.obj"):
            for windows_paths in (False, True):
                with self.subTest(module=module_path, windows=windows_paths):
                    altered = MAP.replace("Core/Src/app.c.obj", module_path)
                    if windows_paths:
                        altered = altered.replace("/", "\\")
                    data = size.parse_map(altered)
                    flash = next(region for region in data.regions if region.name == "FLASH")
                    groups, _ = size.flash_details(data, flash)
                    self.assertEqual(sum(groups.values()), 44)
                    self.assertEqual(groups[size.GROUP_PROJECT], 20)
                    self.assertEqual(groups[size.GROUP_MATH], 16)

    def test_system_libraries_keep_their_groups(self):
        owners = {
            "CMakeFiles/STM32_Drivers.dir/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal.c.obj": size.GROUP_HAL,
            "C:/toolchain/lib/libgcc.a(math.o)": size.GROUP_MATH,
            "C:/toolchain/lib/libm.a(math.o)": size.GROUP_MATH,
            "C:/toolchain/lib/libc_nano.a(printf.o)": size.GROUP_LIBC,
            "C:/toolchain/lib/libg_nano.a(syscalls.o)": size.GROUP_LIBC,
            "C:/toolchain/lib/gcc/arm-none-eabi/14.3.1/thumb/v7-m/nofp/crtbegin.o": size.GROUP_RUNTIME,
            "CMakeFiles/fw.dir/startup_stm32f103xb.s.obj": size.GROUP_RUNTIME,
        }
        for owner, expected in owners.items():
            with self.subTest(owner=owner):
                self.assertEqual(size.group_for(owner), expected)

    def test_wrapped_output_without_load_address(self):
        altered = MAP.replace(".text 0x08000000 0x20", ".text\n                0x08000000 0x20")
        parsed = size.parse_map(altered)
        self.assertEqual(parsed.regions[1].used, 44)
        self.assertEqual(parsed.sections[0].size, 32)

    def test_report_saved_beside_script_and_accepts_quoted_path(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            inputs, scripts = root / "input with spaces", root / "script"
            inputs.mkdir()
            scripts.mkdir()
            map_path = inputs / "firmware.map"
            map_path.write_text(MAP, encoding="utf-8")
            with patch.object(size, "__file__", str(scripts / "firmware_size.py")):
                report = size.generate_report(f'"{map_path}"')
            self.assertEqual(report.parent, scripts)
            content = report.read_text(encoding="utf-8")
            self.assertIn("**44**", content)
            self.assertIn("нет файла .elf", content)
            self.assertIn("first", content)
            self.assertNotIn("unused", content)

    def test_relative_path_rejected(self):
        with self.assertRaisesRegex(ValueError, "абсолютный"):
            size.generate_report("build/fw.map")

    def test_wrong_file_type_rejected(self):
        with self.assertRaisesRegex(ValueError, "файл .map"):
            size.generate_report(str(Path.cwd() / "fw.elf"))

    def test_invalid_map_rejected(self):
        with self.assertRaisesRegex(ValueError, "карта GNU"):
            size.parse_map("not a linker map")

    def test_elf_aliases_grouped_and_thumb_bit_removed(self):
        with tempfile.TemporaryDirectory() as directory:
            elf = Path(directory) / "firmware.elf"
            elf.write_bytes(elf_fixture())
            functions = size.functions_from_elf(elf, self.data, self.flash)
            self.assertEqual(len(functions), 2)
            self.assertEqual(sum(function[1] for function in functions), 16)
            self.assertEqual(functions[0][0], "first, first_alias")

    def test_elf_from_other_build_rejected(self):
        with tempfile.TemporaryDirectory() as directory:
            elf = Path(directory) / "firmware.elf"
            elf.write_bytes(elf_fixture(text_size=31))
            with self.assertRaisesRegex(ValueError, "разным сборкам"):
                size.functions_from_elf(elf, self.data, self.flash)

    def test_malformed_elf_falls_back_to_map(self):
        with tempfile.TemporaryDirectory() as directory:
            map_path = Path(directory) / "firmware.map"
            map_path.with_suffix(".elf").write_bytes(b"not an ELF")
            report = size.render_report(map_path, self.data)
            self.assertIn("Точные размеры из ELF недоступны", report)
            self.assertIn("first", report)


if __name__ == "__main__":
    unittest.main()
