#!/usr/bin/env python3
"""Module 19 example: first-look triage of an executable.

Usage: 19_triage.py FILE
Prints the hashes, the kind of file, the entropy, the imported functions and the strings that
match patterns worth a second look. It never runs the file.
"""
import hashlib
import math
import re
import sys
from collections import Counter

PATTERNS = {
    "registry path": rb"(?:HK(?:CU|LM)|HKEY_[A-Z_]+)\\[ -~]{4,}",
    "command line": rb"(?:schtasks|cmd\.exe|powershell|rundll32|regsvr32|wscript)[ -~]{0,100}",
    "url or host": rb"(?:https?://)?[a-z0-9-]+(?:\.[a-z0-9-]+)*\.(?:test|com|net|org|io)[ -~]{0,40}",
    "windows path": rb"[A-Za-z]:\\[ -~]{4,}",
    "process name": rb"[A-Za-z0-9_]+\.exe",
}


def entropy(data: bytes) -> float:
    if not data:
        return 0.0
    counts = Counter(data)
    return -sum(c / len(data) * math.log2(c / len(data)) for c in counts.values())


def kind(data: bytes) -> str:
    if data[:4] == b"\x7fELF":
        return "ELF " + ("64-bit" if data[4] == 2 else "32-bit")
    if data[:2] == b"MZ":
        return "PE (MZ header)"
    return "unknown"


def imports(path: str, data: bytes):
    names = []
    if data[:4] == b"\x7fELF":
        from elftools.elf.elffile import ELFFile
        with open(path, "rb") as f:
            elf = ELFFile(f)
            sec = elf.get_section_by_name(".dynsym")
            if sec:
                names = [s.name for s in sec.iter_symbols() if s.name and s["st_shndx"] == "SHN_UNDEF"]
    elif data[:2] == b"MZ":
        import pefile
        pe = pefile.PE(path)
        for entry in getattr(pe, "DIRECTORY_ENTRY_IMPORT", []):
            for imp in entry.imports:
                names.append(f"{entry.dll.decode()}!{imp.name.decode() if imp.name else imp.ordinal}")
    return names


def main():
    path = sys.argv[1]
    data = open(path, "rb").read()
    print(f"file:     {path}")
    print(f"size:     {len(data)} bytes")
    print(f"sha256:   {hashlib.sha256(data).hexdigest()}")
    print(f"type:     {kind(data)}")
    print(f"entropy:  {entropy(data):.2f} bits per byte (above about 7.2 suggests packed or encrypted data)")
    names = imports(path, data)
    print(f"imports:  {len(names)}")
    for n in sorted(names)[:12]:
        print(f"    {n}")
    if len(names) > 12:
        print(f"    ... and {len(names) - 12} more (the C runtime and basic KERNEL32 functions)")
    print("strings worth a second look:")
    seen = set()
    for label, pat in PATTERNS.items():
        for m in re.finditer(pat, data):
            s = m.group().decode("latin-1")
            if (label, s) not in seen:
                seen.add((label, s))
                print(f"    [{label}] {s}")


main()
