#!/usr/bin/env python3
"""Verify that every relative Markdown link and anchor-free file reference resolves.

Checks [text](relative/path.md) links in all Markdown files. External (http, https,
mailto) links are not fetched, so the check works offline and in CI without network flakiness.
"""
import re
import sys
from pathlib import Path
from urllib.parse import unquote

ROOT = Path(__file__).resolve().parent.parent
SKIP_DIRS = {".git", ".claude", "node_modules", "build"}
LINK = re.compile(r"(?<!!)\[[^\]]*\]\(([^)\s]+)(?:\s+\"[^\"]*\")?\)")
FENCE = re.compile(r"^```.*?^```", re.S | re.M)


def main():
    broken = checked = 0
    for md in sorted(ROOT.rglob("*.md")):
        rel = md.relative_to(ROOT)
        if set(rel.parts) & SKIP_DIRS:
            continue
        text = FENCE.sub("", md.read_text(encoding="utf-8"))
        for m in LINK.finditer(text):
            target = m.group(1)
            if re.match(r"^(https?:|mailto:|#)", target):
                continue
            path = unquote(target.split("#", 1)[0])
            if not path:
                continue
            checked += 1
            if not (md.parent / path).resolve().exists():
                broken += 1
                print(f"{rel}: broken link -> {target}")
    print(f"check_links: {checked} relative links checked, {broken} broken")
    return 1 if broken else 0


if __name__ == "__main__":
    sys.exit(main())
