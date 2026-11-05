#!/usr/bin/env python3
"""Fail if any text file contains an emoji, pictograph or decorative symbol.

Usage: python3 tools/no_emoji.py [path ...]   (default: repository root)
Exit status: 0 when clean, 1 when findings exist.
"""
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SKIP_DIRS = {".git", "node_modules", "__pycache__", ".venv", "build"}
TEXT_SUFFIXES = {
    ".md", ".txt", ".asm", ".inc", ".c", ".h", ".cpp", ".py", ".sh", ".yml",
    ".yaml", ".json", ".csv", ".toml", ".cfg", ".mk", ".yar", ".html",
}
TEXT_NAMES = {"Makefile", "Dockerfile", "LICENSE", "LICENSE-CONTENT", ".gitignore",
              ".gitattributes", ".dockerignore", "SHA256SUMS"}

# Ranges that hold emoji, pictographs, dingbats, arrows and decorative symbols.
BANNED = [
    (0x1F000, 0x1FAFF),  # emoji, pictographs, transport, supplemental symbols
    (0x2190, 0x21FF),    # arrows
    (0x2300, 0x23FF),    # miscellaneous technical (hourglass, watch, and so on)
    (0x2460, 0x24FF),    # enclosed alphanumerics
    (0x25A0, 0x25FF),    # geometric shapes
    (0x2600, 0x27BF),    # miscellaneous symbols and dingbats (stars, check marks)
    (0x2900, 0x297F),    # supplemental arrows-B
    (0x2B00, 0x2BFF),    # miscellaneous symbols and arrows
    (0xFE0F, 0xFE0F),    # emoji variation selector
    (0x200D, 0x200D),    # zero width joiner (emoji sequences)
]


def banned(ch: str) -> bool:
    cp = ord(ch)
    return any(lo <= cp <= hi for lo, hi in BANNED)


def is_text(path: Path) -> bool:
    return path.suffix.lower() in TEXT_SUFFIXES or path.name in TEXT_NAMES


def scan(path: Path):
    try:
        text = path.read_text(encoding="utf-8")
    except (UnicodeDecodeError, OSError):
        return
    for no, line in enumerate(text.splitlines(), 1):
        for col, ch in enumerate(line, 1):
            if banned(ch):
                yield no, col, ch


def main(argv):
    targets = [Path(a) for a in argv] or [ROOT]
    files = []
    for t in targets:
        if t.is_dir():
            files += [p for p in t.rglob("*")
                      if p.is_file() and not (set(p.parts) & SKIP_DIRS) and is_text(p)]
        elif t.is_file():
            files.append(t)
    findings = 0
    for f in sorted(files):
        for no, col, ch in scan(f):
            print(f"{f}:{no}:{col}: U+{ord(ch):04X} is not allowed")
            findings += 1
    print(f"no_emoji: {len(files)} files scanned, {findings} findings")
    return 1 if findings else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
