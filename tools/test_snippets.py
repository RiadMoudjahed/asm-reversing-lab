#!/usr/bin/env python3
"""Assemble or compile every fenced code block in the repository's Markdown files.

Fence tags and what they check:
  nasm            assembled with nasm (elf64 by default; elf32 when the block says
                  `bits 32`; win64 when the info string contains `win64`)
  c               compiled with gcc -fsyntax-only -std=c11
  bash            syntax-checked with `bash -n`
  *-fragment      intentionally partial (nasm-fragment, c-fragment): skipped
Extra words after the tag select options, for example:  ```nasm win64

Blocks in PLAN.md, CLAUDE.md and .claude/ are skipped (maintainer documents).
Exit status: 0 when every checked block passes, 1 otherwise.
"""
import re
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SKIP_DIRS = {".git", ".claude", "node_modules", "build"}
SKIP_FILES = {"PLAN.md", "CLAUDE.md"}
FENCE = re.compile(r"^```([A-Za-z0-9_+-]*)[ \t]*([^\n]*)\n(.*?)^```[ \t]*$", re.S | re.M)


def blocks(path: Path):
    text = path.read_text(encoding="utf-8")
    for m in FENCE.finditer(text):
        line = text.count("\n", 0, m.start()) + 1
        yield line, m.group(1), m.group(2).split(), m.group(3)


def run(cmd, **kw):
    return subprocess.run(cmd, capture_output=True, text=True, **kw)


def check(tag, opts, code, tmp: Path):
    if tag == "nasm":
        fmt = "win64" if "win64" in opts else ("elf32" if re.search(r"^\s*bits\s+32", code, re.M) else "elf64")
        src = tmp / "b.asm"
        src.write_text(code, encoding="utf-8")
        return run(["nasm", "-f", fmt, str(src), "-o", str(tmp / "b.o")])
    if tag == "c":
        src = tmp / "b.c"
        src.write_text(code, encoding="utf-8")
        return run(["gcc", "-std=c11", "-fsyntax-only", "-Wall", str(src)])
    if tag == "bash":
        return run(["bash", "-n"], input=code)
    return None


def main():
    failures = checked = skipped = 0
    for md in sorted(ROOT.rglob("*.md")):
        rel = md.relative_to(ROOT)
        if set(rel.parts) & SKIP_DIRS or md.name in SKIP_FILES:
            continue
        for line, tag, opts, code in blocks(md):
            if tag.endswith("-fragment") or tag not in {"nasm", "c", "bash"}:
                skipped += 1
                continue
            with tempfile.TemporaryDirectory() as d:
                res = check(tag, opts, code, Path(d))
            checked += 1
            if res.returncode != 0:
                failures += 1
                print(f"FAIL {rel}:{line} ({tag})\n{res.stderr.strip()}\n")
    print(f"test_snippets: {checked} blocks checked, {skipped} skipped, {failures} failed")
    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main())
