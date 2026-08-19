#!/usr/bin/env python3
"""gen_lab_index — rebuild docs/design/index.html from the labs themselves.

Each card's text is the lab's own <title> and .tagline, read at generation time.
Writing descriptions into the index by hand would be a second copy of what every
lab already says about itself, and it would drift the first time a lab changed —
the same reason the GUI derives its controls from the presentation table and the
bend graphs are drawn by the shipped core rather than a JS twin.
"""
import html
import pathlib
import re

ROOT = pathlib.Path(__file__).resolve().parent.parent
D = ROOT / "docs/design"


def labs():
    for f in sorted(D.glob("*.html")):
        if f.name == "index.html":
            continue
        t = f.read_text(errors="ignore")
        m = re.search(r"<title>(.*?)</title>", t, re.S)
        title = html.unescape(m.group(1).strip()) if m else f.stem
        tag = re.search(r'class="tagline"[^>]*>(.*?)</div>', t, re.S)
        desc = re.sub(r"<[^>]+>", "", tag.group(1)) if tag else ""
        yield f.name, title, html.unescape(re.sub(r"\s+", " ", desc)).strip()[:260]


def main():
    n = sum(1 for _ in labs())
    print(f"gen_lab_index: {n} lab(s) in docs/design/")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
