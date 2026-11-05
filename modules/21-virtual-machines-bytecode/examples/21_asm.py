#!/usr/bin/env python3
"""Module 21: assembler for the two instruction sets of the lesson.

Usage: 21_asm.py stack|reg source.asm output.bin
Labels end with a colon; a comment starts with a semicolon.
"""
import sys

STACK = {"halt": (0x00, ""), "push": (0x01, "n"), "add": (0x02, ""), "sub": (0x03, ""),
         "mul": (0x04, ""), "dup": (0x05, ""), "load": (0x06, "n"), "store": (0x07, "n"),
         "jmp": (0x08, "t"), "jz": (0x09, "t"), "print": (0x0A, "")}
REG = {"movi": (0x10, "rn"), "add": (0x11, "rrr"), "sub": (0x12, "rrr"), "mul": (0x13, "rrr"),
       "jnz": (0x14, "rt"), "out": (0x15, "r"), "halt": (0xFF, "")}


def parse(lines):
    items, labels, pos = [], {}, 0
    for raw in lines:
        text = raw.split(";")[0].strip()
        if not text:
            continue
        while ":" in text:
            name, text = text.split(":", 1)
            labels[name.strip()] = pos
            text = text.strip()
        if not text:
            continue
        mnemonic, _, rest = text.partition(" ")
        args = [a.strip() for a in rest.split(",")] if rest.strip() else []
        items.append((mnemonic.lower(), args))
        pos += 1
    return items, labels


def value(token, labels):
    if token in labels:
        return labels[token]
    return int(token.lstrip("r"), 0) if token.startswith("r") and token[1:].isdigit() else int(token, 0)


def assemble_stack(items, labels):
    out = bytearray()
    for mnemonic, args in items:
        opcode, shape = STACK[mnemonic]
        out.append(opcode)
        if shape:
            out.append(value(args[0], labels) if shape == "n" else byte_offset(args[0], labels, items))
    return bytes(out)


def byte_offset(name, labels, items):
    """Stack machine jumps use byte offsets; labels were counted in instructions."""
    target = labels[name]
    return sum(1 + (1 if STACK[m][1] else 0) for m, _ in items[:target])


def assemble_reg(items, labels):
    out = bytearray()
    for mnemonic, args in items:
        opcode, shape = REG[mnemonic]
        fields = [value(a, labels) for a in args]
        if shape == "rn":
            fields = [fields[0], fields[1] & 0xFF, fields[1] >> 8]
        out += bytes([opcode] + fields + [0] * (3 - len(fields)))
    return bytes(out)


def main():
    isa, source, target = sys.argv[1:4]
    items, labels = parse(open(source).read().splitlines())
    code = assemble_stack(items, labels) if isa == "stack" else assemble_reg(items, labels)
    open(target, "wb").write(code)
    print(f"{target}: {len(code)} bytes, {len(items)} instructions")


if __name__ == "__main__":
    main()
