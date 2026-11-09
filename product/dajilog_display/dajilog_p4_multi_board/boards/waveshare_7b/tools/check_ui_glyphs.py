#!/usr/bin/env python3
"""Check Korean EEZ fonts against UI text, product names and runtime labels."""

import json
import re
from pathlib import Path

BOARD = Path(__file__).resolve().parents[1]
UI = BOARD / "apps/brookesia_app_eez_demo/eez/ui/generated/waveshare_7b_nongjinwon"
PROJECT = UI / "waveshare_7b_nongjinwon.eez-project"
C_STRING = re.compile(r'"(?:[^"\\]|\\.)*"')
GLYPH = re.compile(r"/\* U\+([0-9A-Fa-f]{4,6}) ")
TEXT_KEYS = {"text", "value", "defaultValue", "description", "label"}
SKIP_KEYS = {"embeddedFontFile", "embeddedFontFileIndex", "lvglSymbols",
             "lvglGlyphs", "source"}


def non_ascii(text):
    return {char for char in text if ord(char) > 127 and not char.isspace()}


def ui_characters(value):
    if isinstance(value, list):
        for child in value:
            yield from ui_characters(child)
    elif isinstance(value, dict):
        for key, child in value.items():
            if key in SKIP_KEYS:
                continue
            if key in TEXT_KEYS and isinstance(child, str):
                yield from non_ascii(child)
            else:
                yield from ui_characters(child)


def main():
    project = json.loads(PROJECT.read_text(encoding="utf-8"))
    required = set(ui_characters(project))
    sources = list((BOARD / "main").glob("*.c"))
    sources += [p for p in (BOARD / "apps/brookesia_app_eez_demo").rglob("*.cpp")
                if "generated" not in p.parts]
    sources += list((BOARD / "config/data").glob("*.csv"))
    for source in sources:
        for string in C_STRING.findall(source.read_text(encoding="utf-8", errors="replace")):
            required |= non_ascii(string)
    failed = False
    for font in project["fonts"]:
        if "_KR" not in font["name"]:
            continue
        size = font["source"]["size"]
        generated = UI / f"src/ui/ui_font_maru_buri_regular_kr{size}.c"
        present = {chr(int(code, 16)) for code in GLYPH.findall(
            generated.read_text(encoding="utf-8"))}
        missing = required - present
        print(f"{font['name']}: {len(required)} required, {len(missing)} missing")
        if missing:
            print("  missing:", "".join(sorted(missing)))
            failed = True
    raise SystemExit(1 if failed else 0)


if __name__ == "__main__":
    main()
