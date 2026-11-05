#!/usr/bin/env python3
"""Exercise 20, model solution: a table-driven disassembler for the machine, built from isa.json.

Usage: tvdis.py program.tvm [isa.json]
"""
import json
import struct
import sys
from pathlib import Path

SHAPES = {"movi": "ri", "mov": "rr", "add": "rr", "sub": "rr", "mul": "rr", "xor": "rr",
          "and": "rr", "shl": "rc", "shr": "rc", "cmp": "rr", "jmp": "t", "jz": "t", "jnz": "t",
          "call": "t", "ret": "n", "ldb": "rr", "stb": "rr", "out": "r", "outc": "r", "halt": "n"}


def disassemble(blob, isa):
    assert blob[:4] == b"TVM1"
    code_len, data_len = struct.unpack_from("<HH", blob, 4)
    code, data = blob[8:8 + code_len], blob[8 + code_len:8 + code_len + data_len]
    by_code = {int(e["code"], 0): (name, e["length"]) for name, e in isa["opcodes"].items()}
    high = isa["register_a"] == "high"
    pos, lines, targets = 0, [], set()
    while pos < len(code):
        name, length = by_code.get(code[pos], (None, 1))
        raw = code[pos:pos + length]
        if name is None:
            lines.append((pos, raw, ".byte"))
            pos += 1
            continue
        b = raw[1] if len(raw) > 1 else 0
        a, c = ((b >> 4) & 7, b & 7) if high else (b & 7, (b >> 4) & 7)
        n = (b & 15) if high else (b >> 4)
        shape = SHAPES[name]
        if shape == "n":
            text = name
        elif shape == "r":
            text = f"{name} r{a}"
        elif shape == "rr":
            text = f"{name} [r{a}], r{c}" if name == "stb" else f"{name} r{a}, [r{c}]" if name == "ldb" else f"{name} r{a}, r{c}"
        elif shape == "rc":
            text = f"{name} r{a}, {n}"
        elif shape == "ri":
            text = f"{name} r{a}, {struct.unpack_from('<H', raw, 2)[0]}"
        else:
            tgt = struct.unpack_from("<H", raw, 1)[0]
            targets.add(tgt)
            text = f"{name} L{tgt:03x}"
        lines.append((pos, raw, text))
        pos += length
    out = []
    for pos, raw, text in lines:
        label = f"L{pos:03x}:" if pos in targets else ""
        out.append(f"{label:<6}{pos:04x}  {raw.hex(' '):<12}  {text}")
    return out, data, len(lines)


if __name__ == "__main__":
    isa = json.loads(Path(sys.argv[2] if len(sys.argv) > 2 else Path(__file__).with_name("isa.json")).read_text())
    listing, data, count = disassemble(Path(sys.argv[1]).read_bytes(), isa)
    print("\n".join(listing))
    print(f"{count} instructions; data segment: {data!r}")
