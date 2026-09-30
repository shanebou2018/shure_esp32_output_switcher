#!/usr/bin/env python3
"""Regenerate MXN_AMP_Switcher/index_html.h from web/index.html.

Run this after editing web/index.html:  python3 tools/embed_web.py
The Arduino IDE compiles the generated header into the sketch.
"""
from pathlib import Path

root = Path(__file__).resolve().parent.parent
html = (root / "web" / "index.html").read_text(encoding="utf-8")
delim = "html"
assert f"){delim}\"" not in html, "pick another raw-string delimiter"

out = root / "MXN_AMP_Switcher" / "index_html.h"
out.write_text(
    "#pragma once\n\n"
    "// GENERATED from web/index.html by tools/embed_web.py. Do not edit by hand.\n\n"
    f"static const char kIndexHtml[] = R\"{delim}(" + html + f"){delim}\";\n",
    encoding="utf-8",
)
print(f"wrote {out.relative_to(root)} ({len(html)} bytes of HTML)")
