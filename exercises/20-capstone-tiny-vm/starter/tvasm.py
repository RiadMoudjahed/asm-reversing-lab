#!/usr/bin/env python3
"""Exercise 20: assembler for the machine you reverse engineered.

Usage: tvasm.py [--isa isa.json] program.tvs program.tvm

The instruction set comes from your isa.json, so the output is only as good as your notes.

Source format: one instruction per line, an optional `label:` in front, `;` starts a comment.
  r0 .. r7        registers
  5, 0x1f, 'A'    numbers (movi takes 0 to 65535; the count of shl and shr takes 0 to 15)
  [r3]            a register used as an address (ldb and stb only)
  name            a label, as the operand of jmp, jz, jnz and call
  .data "text"    appends bytes to the data segment (copied to memory address 0 at start)
  .byte 1, 2, 3   appends bytes to the data segment

Operands are written to the file in the order they appear in the source line. Two registers, or
a register and a count, share one byte: the first register goes in the nibble that isa.json
names as "register_a", the other operand in the other nibble. A single register uses the same
nibble. A 16-bit number is written in the byte order named by "immediate". If the "length" in
your isa.json is longer than the natural encoding the instruction is padded with zero bytes; if
it is shorter the assembler stops.
"""
import argparse
import json
import re
import struct
import sys

# mnemonic: operand shape
#   n none, r one register, rr two registers, rc register and count, ri register and 16-bit
#   number, t 16-bit code offset
SHAPES = {"movi": "ri", "mov": "rr", "add": "rr", "sub": "rr", "mul": "rr", "xor": "rr",
          "and": "rr", "shl": "rc", "shr": "rc", "cmp": "rr", "jmp": "t", "jz": "t", "jnz": "t",
          "call": "t", "ret": "n", "ldb": "rr", "stb": "rr", "out": "r", "outc": "r",
          "halt": "n"}


class AsmError(Exception):
    pass


def number(token):
    token = token.strip()
    if re.fullmatch(r"'.'", token):
        return ord(token[1])
    try:
        return int(token, 0)
    except ValueError:
        raise AsmError(f"not a number: {token!r}")


def register(token):
    m = re.fullmatch(r"\[?\s*r([0-7])\s*\]?", token.strip())
    if not m:
        raise AsmError(f"not a register: {token!r}")
    return int(m.group(1))


def strip_comment(raw):
    """Remove a ; comment, but keep a ; inside a quoted string."""
    quoted = False
    for i, ch in enumerate(raw):
        if ch == '"':
            quoted = not quoted
        elif ch == ";" and not quoted:
            return raw[:i]
    return raw


def parse(lines):
    items, labels, data, pos = [], {}, bytearray(), 0
    for n, raw in enumerate(lines, 1):
        text = strip_comment(raw)
        text = text.strip()
        if not text:
            continue
        m = re.match(r"([A-Za-z_]\w*):\s*(.*)", text)
        if m:
            labels[m.group(1)] = pos
            text = m.group(2).strip()
            if not text:
                continue
        if text.startswith(".data"):
            data += re.search(r'"(.*)"', text).group(1).encode("latin-1")
            continue
        if text.startswith(".byte"):
            data += bytes(number(x) & 255 for x in text[5:].split(","))
            continue
        mnemonic, _, rest = text.partition(" ")
        mnemonic = mnemonic.lower()
        if mnemonic not in SHAPES:
            raise AsmError(f"line {n}: unknown instruction {mnemonic!r}")
        operands = [o.strip() for o in rest.split(",")] if rest.strip() else []
        items.append((n, mnemonic, operands))
        pos += 1
    return items, labels, data


def encode(isa, mnemonic, operands, labels, offsets, line):
    entry = isa["opcodes"].get(mnemonic)
    if not entry or entry.get("code") in (None, ""):
        raise AsmError(f"line {line}: isa.json has no code for {mnemonic!r}")
    code = int(str(entry["code"]), 0)
    high = isa.get("register_a", "high") == "high"
    little = isa.get("immediate", "little") == "little"

    def pack(a, b):
        return ((a << 4) | b) if high else ((b << 4) | a)

    def word(v):
        return struct.pack("<H" if little else ">H", v & 0xFFFF)

    shape = SHAPES[mnemonic]
    expect = {"n": 0, "r": 1, "rr": 2, "rc": 2, "ri": 2, "t": 1}[shape]
    if len(operands) != expect:
        raise AsmError(f"line {line}: {mnemonic} takes {expect} operand(s)")
    out = bytes([code])
    if shape == "r":
        out += bytes([pack(register(operands[0]), 0)])
    elif shape == "rr":
        out += bytes([pack(register(operands[0]), register(operands[1]))])
    elif shape == "rc":
        count = number(operands[1])
        if not 0 <= count <= 15:
            raise AsmError(f"line {line}: the count must be 0 to 15")
        out += bytes([pack(register(operands[0]), count)])
    elif shape == "ri":
        value = number(operands[1])
        if not 0 <= value <= 0xFFFF:
            raise AsmError(f"line {line}: the number must be 0 to 65535")
        out += bytes([pack(register(operands[0]), 0)]) + word(value)
    elif shape == "t":
        target = operands[0]
        if target not in labels:
            raise AsmError(f"line {line}: unknown label {target!r}")
        out += word(offsets[labels[target]])
    length = entry.get("length")
    length = len(out) if length in (None, "") else int(length)
    if length < len(out):
        raise AsmError(f"line {line}: {mnemonic} needs {len(out)} bytes but isa.json says {length}")
    return out + bytes(length - len(out))


def assemble(isa, source_lines):
    items, labels, data = parse(source_lines)
    # byte offset of every instruction, from the lengths in the isa file
    offsets, pos = [], 0
    for line, mnemonic, ops in items:
        entry = isa["opcodes"].get(mnemonic) or {}
        offsets.append(pos)
        natural = {"n": 1, "r": 2, "rr": 2, "rc": 2, "ri": 4, "t": 3}[SHAPES[mnemonic]]
        length = entry.get("length")
        pos += natural if length in (None, "") else int(length)
    offsets.append(pos)
    code = b"".join(encode(isa, mn, ops, labels, offsets, line) for line, mn, ops in items)
    return b"TVM1" + struct.pack("<HH", len(code), len(data)) + code + bytes(data), len(items)


def main():
    ap = argparse.ArgumentParser(description="Assembler for the exercise 20 machine")
    ap.add_argument("--isa", default="isa.json")
    ap.add_argument("source")
    ap.add_argument("output")
    args = ap.parse_args()
    try:
        isa = json.load(open(args.isa, encoding="utf-8"))
        blob, count = assemble(isa, open(args.source, encoding="utf-8").read().splitlines())
    except (AsmError, OSError, json.JSONDecodeError, KeyError, ValueError, TypeError) as err:
        print(f"tvasm: {err}", file=sys.stderr)
        return 1
    open(args.output, "wb").write(blob)
    print(f"{args.output}: {count} instructions, {len(blob)} bytes")
    return 0


if __name__ == "__main__":
    sys.exit(main())
