#!/usr/bin/env python3
"""Advisory terminology scan: does each translated page use the term fixed in i18n/terminology.csv?

For every page whose English text contains a term of the table (outside code), the translated
page is searched for the fixed translation (allowing an inflected ending). A term that is
missing from a translation is reported with the pages. The result is a work list for the
native-speaker review, not a failure: a translator may legitimately rephrase. Use --strict
to return a non-zero status when a term is missing from more than --limit pages.

Usage: python3 tools/terminology_check.py [--lang de] [--limit 3] [--strict]
"""
import argparse
import csv
import re
import sys
from collections import defaultdict
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
LANGS = ["ar", "de", "es", "fr"]
SKIP = {".git", ".claude", "node_modules", "build", "i18n"}


def prose(path: Path) -> str:
    text = path.read_text(encoding="utf-8")
    text = re.sub(r"```.*?```", " ", text, flags=re.S)
    text = re.sub(r"`[^`]*`", " ", text)
    text = re.sub(r"<!--.*?-->", " ", text, flags=re.S)
    text = re.sub(r"\]\([^)]*\)", "]", text)
    return text.lower()


def pattern(term: str):
    """A regular expression that finds the term with inflected endings on each word."""
    parts = []
    for w in term.lower().split():
        parts.append(re.escape(w[:-2] if len(w) > 6 else w) + r"\w*")
    return re.compile(r"\W+".join(parts))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--lang", choices=LANGS)
    ap.add_argument("--limit", type=int, default=3)
    ap.add_argument("--strict", action="store_true")
    args = ap.parse_args()
    rows = list(csv.DictReader(open(ROOT / "i18n" / "terminology.csv", encoding="utf-8")))
    missing = defaultdict(list)
    for page in sorted(ROOT.rglob("*.md")):
        if set(page.relative_to(ROOT).parts) & SKIP or re.search(r"\.(ar|de|es|fr)\.md$", page.name):
            continue
        if page.name in ("PLAN.md", "CLAUDE.md"):
            continue
        en = prose(page)
        for lang in ([args.lang] if args.lang else LANGS):
            tr = page.with_name(page.name[:-3] + f".{lang}.md")
            if not tr.exists():
                continue
            trans = prose(tr)
            for row in rows:
                term, local = row["en"].lower(), row[lang].strip()
                if not local or not re.search(r"\b" + re.escape(term) + r"\b", en):
                    continue
                if "(" in local:
                    continue
                if not pattern(local).search(trans) and term not in trans:
                    missing[(lang, row["en"], local)].append(str(page.relative_to(ROOT)))
    over = 0
    for (lang, term, local), pages in sorted(missing.items()):
        if len(pages) >= args.limit:
            over += 1
            print(f"{lang}: {term!r} -> {local!r} missing on {len(pages)} page(s), for example {pages[0]}")
    print(f"terminology_check: {over} term(s) missing on {args.limit} or more pages")
    return 1 if args.strict and over else 0


if __name__ == "__main__":
    sys.exit(main())
