#!/usr/bin/env python3
r"""Render every fig-*.svg to a sibling PDF with rsvg-convert.

On Windows rsvg-convert has no fontconfig by default and then silently drops
all <text>. This writes a minimal fonts.conf (pointing at C:\Windows\Fonts)
and runs rsvg-convert with FONTCONFIG_FILE set, so the labels survive.
"""
from __future__ import annotations

import os
import subprocess
import sys
from pathlib import Path

BASE = Path(__file__).resolve().parent
CONF = BASE / ".fontconfig" / "fonts.conf"

CONFIG = (
    '<?xml version="1.0"?>\n'
    '<!DOCTYPE fontconfig SYSTEM "fonts.dtd">\n'
    "<fontconfig>\n"
    "  <dir>C:\\Windows\\Fonts</dir>\n"
    "  <cachedir>__CACHE__</cachedir>\n"
    "</fontconfig>\n"
)


def main() -> int:
    CONF.parent.mkdir(parents=True, exist_ok=True)
    CONF.write_text(
        CONFIG.replace("__CACHE__", str(CONF.parent / "cache")), encoding="utf-8"
    )
    env = dict(os.environ, FONTCONFIG_FILE=str(CONF))
    figs = sorted(BASE.glob("fig-*.svg"))
    for svg in figs:
        pdf = svg.with_suffix(".pdf")
        print(f"[figs] {svg.name} -> {pdf.name}", flush=True)
        subprocess.run(
            ["rsvg-convert", "-f", "pdf", "-o", str(pdf), str(svg)],
            check=True,
            env=env,
        )
    print(f"rendered {len(figs)} figure(s)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
