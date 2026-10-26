"""Build a vector A4 toner-transfer sheet: copper and silk, two copies."""
from __future__ import annotations

import argparse
from copy import deepcopy
from datetime import datetime, timezone
import hashlib
from io import BytesIO
import json
from pathlib import Path
import re
import subprocess

import pdfplumber
from pypdf import PageObject, PdfReader, PdfWriter, Transformation
from pypdf.generic import DictionaryObject, NameObject
from reportlab.lib.units import mm
from reportlab.pdfgen import canvas


def parse_sexpr(text):
    tokens = iter(re.findall(r'"(?:\\.|[^"\\])*"|[()]|[^\s()]+', text))

    def block():
        result = []
        for token in tokens:
            if token == "(":
                result.append(block())
            elif token == ")":
                return result
            elif token.startswith('"'):
                result.append(json.loads(token))
            else:
                result.append(token)
        return result

    return block()[0]


def nodes(parent, key):
    return [item for item in parent if isinstance(item, list) and item and item[0] == key]


def outline_dimensions(board_bytes):
    root = parse_sexpr(board_bytes.decode("utf-8-sig"))
    outlines = [item for item in nodes(root, "gr_rect")
                if ["layer", "Edge.Cuts"] in item]
    if len(outlines) != 1:
        raise ValueError("Expected one rectangular Edge.Cuts outline")
    outline = outlines[0]
    x0, y0 = map(float, nodes(outline, "start")[0][1:3])
    x1, y1 = map(float, nodes(outline, "end")[0][1:3])
    return abs(x1 - x0), abs(y1 - y0)


def export_plot(cli, board, output, layers, holes):
    command = [str(cli), "pcb", "export", "pdf", "--layers", layers,
               "--black-and-white", "--drill-shape-opt", str(holes),
               "--mode-single", "--scale", "1", "--no-property-popups",
               "--output", str(output), str(board)]
    subprocess.run(command, check=True, capture_output=True, text=True)
    return command


def read_plot(path, width, height):
    reader = PdfReader(path)
    assert len(reader.pages) == 1
    with pdfplumber.open(path) as document:
        page = document.pages[0]
        assert not page.images, "Source artwork must be vector"
        outlines = [curve for curve in page.curves
                    if abs(curve["width"] / mm - width) < .01
                    and abs(curve["height"] / mm - height) < .01]
        assert len(outlines) == 1, (path.name, len(outlines))
        edge = outlines[0]
        bounds = (edge["x0"], page.height - edge["bottom"],
                  edge["x1"], page.height - edge["top"])
        white_shapes = [curve for curve in page.curves + page.rects
                        if curve.get("fill") and curve.get("non_stroking_color")
                        in [(1, 1, 1), (1.0, 1.0, 1.0), 1, 1.0]]
    return reader, reader.pages[0], bounds, len(white_shapes)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--board", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--work-dir", type=Path, required=True)
    parser.add_argument("--kicad-cli", type=Path, required=True)
    parser.add_argument("--copper-pdf", type=Path)
    parser.add_argument("--silk-pdf", type=Path)
    parser.add_argument("--copper-print-ready", action="store_true",
                        help="Input copper PDF is already mirrored for transfer")
    args = parser.parse_args()
    board = args.board.resolve(strict=True)
    output = args.output.resolve()
    work = args.work_dir.resolve()
    source = board.read_bytes()
    source_hash = hashlib.sha256(source).hexdigest()
    width, height = outline_dimensions(source)
    page_w, page_h = 210.0, 297.0
    gap, offset, top_cut = 5.0, 1.0, 5.0
    assert 2 * width + gap + 2 * offset < page_w
    assert top_cut + 2 * height + gap + 2 * offset < page_h
    work.mkdir(parents=True, exist_ok=True)
    output.parent.mkdir(parents=True, exist_ok=True)

    commands = []
    if bool(args.copper_pdf) != bool(args.silk_pdf):
        parser.error("Provide both input PDFs or neither")
    if args.copper_pdf:
        paths = {"copper": args.copper_pdf.resolve(strict=True),
                 "silk": args.silk_pdf.resolve(strict=True)}
    else:
        paths = {name: work / f"{name}_source.pdf" for name in ("copper", "silk")}
        for name, layers, holes in [("copper", "F.Cu,Edge.Cuts", 2),
                                     ("silk", "B.Silkscreen,Edge.Cuts", 0)]:
            commands.append(export_plot(args.kicad_cli, board, paths[name], layers, holes))
    assert hashlib.sha256(board.read_bytes()).hexdigest() == source_hash, "Board changed during export"
    plots = {name: read_plot(paths[name], width, height)
             for name in ("copper", "silk")}
    # Independent PDF plots may place their board outlines at different page
    # coordinates (especially after mirroring); only physical sizes must agree.
    copper_bounds, silk_bounds = plots["copper"][2], plots["silk"][2]
    assert abs((copper_bounds[2] - copper_bounds[0]) -
               (silk_bounds[2] - silk_bounds[0])) < .001
    assert abs((copper_bounds[3] - copper_bounds[1]) -
               (silk_bounds[3] - silk_bounds[1])) < .001
    assert plots["copper"][3] > 0, "No white hole cutouts found in copper PDF"

    page = PageObject.create_blank_page(width=page_w * mm, height=page_h * mm)
    left = (page_w - 2 * width - gap) / 2
    placements = []
    for row in range(2):
        for column, name in enumerate(("copper", "silk")):
            _, original, (x0, y0, x1, y1), _ = plots[name]
            source_page = deepcopy(original)
            sx, sy = width * mm / (x1 - x0), height * mm / (y1 - y0)
            assert abs(sx - sy) < 1e-5
            assert abs(sx - 1) < .0001 and abs(sy - 1) < .0001
            source_page.cropbox.lower_left = (x0 - offset * mm / sx, y0 - offset * mm / sy)
            source_page.cropbox.upper_right = (x1 + offset * mm / sx, y1 + offset * mm / sy)
            x = left + column * (width + gap)
            top = top_cut + offset + row * (height + gap)
            mirror = name == "copper" and not args.copper_print_ready
            matrix = (-sx if mirror else sx, 0, 0, sy,
                      x * mm + sx * x1 if mirror else x * mm - sx * x0,
                      (page_h - top) * mm - sy * y1)
            page.merge_transformed_page(source_page, Transformation(matrix))
            placements.append(dict(row=row + 1, column=column + 1, layer=name,
                                   board_left_mm=x, board_top_mm=top,
                                   width_mm=width, height_mm=height,
                                   extra_horizontal_reflection=mirror, matrix=matrix))

    overlay = BytesIO()
    drawing = canvas.Canvas(overlay, pagesize=(page_w * mm, page_h * mm))
    drawing.setStrokeColorRGB(0, 0, 0)
    drawing.setLineWidth(.1 * mm)
    drawing.setDash(1 * mm, .6 * mm)
    for placement in placements:
        drawing.rect((placement["board_left_mm"] - offset) * mm,
                     (page_h - placement["board_top_mm"] - height - offset) * mm,
                     (width + 2 * offset) * mm, (height + 2 * offset) * mm,
                     fill=0, stroke=1)
    drawing.showPage()
    drawing.save()
    overlay.seek(0)
    page.merge_page(PdfReader(overlay).pages[0])
    writer = PdfWriter()
    writer.add_page(page)
    writer.add_metadata({"/Title": f"{board.stem} - A4 toner transfer - two sets - 1:1",
                         "/Subject": "Mirrored F.Cu with actual holes; native B.Silkscreen; dashed cut frames."})
    writer.root_object[NameObject("/ViewerPreferences")] = DictionaryObject({
        NameObject("/PrintScaling"): NameObject("/None")})
    with output.open("wb") as stream:
        writer.write(stream)

    verified = PdfReader(output)
    assert len(verified.pages) == 1
    assert abs(float(verified.pages[0].mediabox.width) / mm - page_w) < 1e-6
    assert abs(float(verified.pages[0].mediabox.height) / mm - page_h) < 1e-6
    with pdfplumber.open(output) as document:
        final = document.pages[0]
        outlines = [curve for curve in final.curves
                    if abs(curve["width"] / mm - width) < .001
                    and abs(curve["height"] / mm - height) < .001]
        cuts = [rect for rect in final.rects
                if abs(rect["width"] / mm - width - 2 * offset) < .001
                and abs(rect["height"] / mm - height - 2 * offset) < .001]
        assert len(outlines) == 4 and len(cuts) == 4
        assert not final.images
        for cut in cuts:
            assert cut["x0"] > 0 and cut["x1"] < final.width
            assert cut["top"] >= top_cut * mm - .001
        for placement in placements:
            assert sum(abs(curve["x0"] / mm - placement["board_left_mm"]) < .001
                       and abs(curve["top"] / mm - placement["board_top_mm"]) < .001
                       for curve in outlines) == 1
    assert hashlib.sha256(board.read_bytes()).hexdigest() == source_hash, "Board changed during assembly"
    manifest = dict(created_at=datetime.now(timezone.utc).isoformat(), source=str(board),
                    source_sha256=source_hash, output=str(output),
                    page_mm=[page_w, page_h], board_mm=[width, height],
                    gap_mm=gap, cutting_offset_mm=offset, first_cutting_line_top_mm=top_cut,
                    copper_white_hole_shapes=plots["copper"][3], commands=commands,
                    input_pdfs={name: dict(path=str(path),
                               sha256=hashlib.sha256(path.read_bytes()).hexdigest())
                               for name, path in paths.items()},
                    placements=placements,
                    checks=dict(pages=1, board_outlines=4, dashed_cut_frames=4,
                                scale_1_to_1=True, vector_only=True))
    (work / "verification.json").write_text(json.dumps(manifest, indent=2), encoding="utf-8")
    print(json.dumps(manifest, indent=2))


if __name__ == "__main__":
    main()
