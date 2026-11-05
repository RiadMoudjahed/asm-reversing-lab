#!/usr/bin/env python3
"""Build a translated page from an English page, keeping code blocks identical.

The English page is split into prose blocks (separated by blank lines) and fenced code blocks.
You supply a Python file that defines a dict of translated prose blocks keyed by block number;
the code blocks are copied byte for byte from the English file, so they cannot drift.

  python3 tools/translate_blocks.py list  PAGE.md
      Print the numbered prose blocks (block 0 is the language bar and stays unchanged).

  python3 tools/translate_blocks.py build PAGE.md LANG TRANSLATIONS.py [--var NAME]
      Write PAGE.LANG.md. TRANSLATIONS.py defines a dict (default name T) mapping every block
      number to its translation, or to None to keep the English text.

What `build` does for you:
  - adds the header comment with the source hash placeholder;
  - rewrites links to sibling pages (README, quiz, HINTS, WRITEUP, syscalls-linux) to the
    same-language file, except in the language bar;
  - for Arabic, wraps prose in <div dir="rtl"> and leaves code blocks outside it.
After building, run `python3 tools/i18n_check.py --stamp-new` to fill in the source hash.
"""
import argparse
import re
import runpy
import sys
from pathlib import Path

LINK = re.compile(r"\]\(([^)#\s]*?(?:README|quiz|HINTS|WRITEUP|syscalls-linux))\.md\)")


def split_blocks(text):
    """Return a list of (kind, text) with kind 'text' or 'code'."""
    out, cur, fence = [], [], False
    for line in text.split("\n"):
        if line.startswith("```"):
            if not fence and cur:
                out.append(("text", "\n".join(cur)))
                cur = []
            fence = not fence
            cur.append(line)
            if not fence:
                out.append(("code", "\n".join(cur)))
                cur = []
        elif fence:
            cur.append(line)
        elif line.strip() == "":
            if cur:
                out.append(("text", "\n".join(cur)))
                cur = []
        else:
            cur.append(line)
    if cur:
        out.append(("text", "\n".join(cur)))
    return out


def build(page: Path, lang: str, tr: dict) -> str:
    blocks = split_blocks(page.read_text(encoding="utf-8"))
    needed = [i for i in range(sum(k == "text" for k, _ in blocks))]
    missing = [i for i in needed if i not in tr]
    extra = [i for i in tr if i not in needed]
    if missing or extra:
        sys.exit(f"{lang}: missing blocks {missing}, unknown blocks {extra}")
    localize = lambda t: LINK.sub(lambda m: f"]({m.group(1)}.{lang}.md)", t)
    pieces, n = [], 0
    for kind, text in blocks:
        if kind == "code":
            pieces.append((kind, text))
            continue
        new = text if tr[n] is None else tr[n]
        pieces.append((kind, new if n == 0 else localize(new)))
        n += 1
    if lang == "ar":
        out, run = [], []

        def flush():
            nonlocal run
            if run:
                out.append('<div dir="rtl">\n\n' + "\n\n".join(run) + "\n\n</div>")
                run = []

        for i, (kind, text) in enumerate(pieces):
            if i == 0:
                out.append(text)          # language bar stays outside the div
            elif kind == "code":
                flush()
                out.append(text)
            else:
                run.append(text)
        flush()
        body = "\n\n".join(out)
    else:
        body = "\n\n".join(t for _, t in pieces)
    head = f"<!-- source: {page.name} @ sha256:SOURCEHASH -->\n"
    return head + body + "\n"


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = ap.add_subparsers(dest="cmd", required=True)
    ls = sub.add_parser("list")
    ls.add_argument("page")
    bd = sub.add_parser("build")
    bd.add_argument("page")
    bd.add_argument("lang", choices=["ar", "de", "es", "fr"])
    bd.add_argument("translations")
    bd.add_argument("--var", default="T")
    args = ap.parse_args()
    page = Path(args.page)
    if args.cmd == "list":
        n = 0
        for kind, text in split_blocks(page.read_text(encoding="utf-8")):
            if kind == "text":
                print(f"[{n}] {text.splitlines()[0][:76]}")
                n += 1
        return 0
    tr = runpy.run_path(args.translations)[args.var]
    target = page.with_name(f"{page.stem}.{args.lang}.md")
    target.write_text(build(page, args.lang, tr), encoding="utf-8")
    print(f"wrote {target}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
