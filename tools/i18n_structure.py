#!/usr/bin/env python3
"""Compare every translation with its English source for structural drift.

Checks, per translated page:
  - fenced code blocks are byte-identical and in the same order;
  - the same number of headings per level, table rows, list items, <details> blocks;
  - the multiset of inline `code` spans is identical;
  - the same set of relative link targets after language-suffix normalization.
Exit status 1 when any page differs. Complements tools/i18n_check.py, which only checks that
translations exist and are current.
"""
import re
import sys
from collections import Counter
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
LANGS = ["ar", "de", "es", "fr"]
SKIP = {".git", ".claude", "node_modules", "build"}
FENCE = re.compile(r"^```.*?^```[ \t]*$", re.S | re.M)


def facts(text: str):
    codes = FENCE.findall(text)
    prose = FENCE.sub("", text)
    prose = re.sub(r"\A(?:\s*<!--.*?-->)+", "", prose, flags=re.S)   # header comments only
    lines = prose.splitlines()
    inline = Counter(re.findall(r"`([^`]+)`", prose))
    links = Counter()
    for target in re.findall(r"\]\(([^)\s]+)\)", prose):
        if re.match(r"^(https?:|mailto:|#)", target):
            continue
        links[re.sub(r"\.(ar|de|es|fr)\.md", ".md", target)] += 1
    return {
        "code blocks": codes,
        "headings": Counter(len(m.group(1)) for l in lines if (m := re.match(r"^(#{1,6}) ", l))),
        "table rows": sum(1 for l in lines if l.startswith("|")),
        "bullets": sum(1 for l in lines if re.match(r"^\s*- ", l)),
        "numbered": sum(1 for l in lines if re.match(r"^\s*\d+\. ", l)),
        "details": prose.count("<details>"),
        "inline code": inline,
        "links": links,
    }


def main():
    bad = pages = 0
    for en in sorted(ROOT.rglob("*.md")):
        rel = en.relative_to(ROOT)
        if set(rel.parts) & SKIP or re.search(r"\.(ar|de|es|fr)\.md$", en.name):
            continue
        fe = facts(en.read_text(encoding="utf-8"))
        for lang in LANGS:
            tr = en.with_name(f"{en.stem}.{lang}.md")
            if not tr.exists():
                continue
            pages += 1
            ft = facts(tr.read_text(encoding="utf-8"))
            for key in fe:
                if fe[key] == ft[key]:
                    continue
                bad += 1
                detail = ""
                if isinstance(fe[key], Counter):
                    detail = f" only-in-English={dict(fe[key] - ft[key])} only-in-translation={dict(ft[key] - fe[key])}"
                elif isinstance(fe[key], list):
                    detail = f" english={len(fe[key])} translation={len(ft[key])}"
                else:
                    detail = f" english={fe[key]} translation={ft[key]}"
                print(f"{tr.relative_to(ROOT)}: {key} differ;{detail}"[:400])
    print(f"i18n_structure: {pages} translations compared, {bad} differences")
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main())
