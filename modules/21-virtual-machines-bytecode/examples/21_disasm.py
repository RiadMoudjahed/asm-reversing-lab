#!/usr/bin/env python3
"""Module 21: table-driven disassembler for the two instruction sets of the lesson.

Usage: 21_disasm.py stack|reg program.bin
The instruction set is data: one table per machine. Recovering a machine from its interpreter
means filling in such a table; the loop below does not change.
"""
import sys

# opcode: (name, operand bytes, operand format)
STACK = {0x00: ("halt", 0, ""), 0x01: ("push", 1, "{0}"), 0x02: ("add", 0, ""),
         0x03: ("sub", 0, ""), 0x04: ("mul", 0, ""), 0x05: ("dup", 0, ""),
         0x06: ("load", 1, "{0}"), 0x07: ("store", 1, "{0}"), 0x08: ("jmp", 1, "@{0}"),
         0x09: ("jz", 1, "@{0}"), 0x0A: ("print", 0, "")}
REG = {0x10: ("movi", 3, "r{0}, {n}"), 0x11: ("add", 3, "r{0}, r{1}, r{2}"),
       0x12: ("sub", 3, "r{0}, r{1}, r{2}"), 0x13: ("mul", 3, "r{0}, r{1}, r{2}"),
       0x14: ("jnz", 3, "r{0}, @{1}"), 0x15: ("out", 3, "r{0}"), 0xFF: ("halt", 3, "")}


def disassemble(code, table):
    pos, lines = 0, []
    while pos < len(code):
        opcode = code[pos]
        if opcode not in table:
            lines.append(f"{pos:04x}  {opcode:02x}              .byte 0x{opcode:02x}")
            pos += 1
            continue
        name, size, fmt = table[opcode]
        ops = list(code[pos + 1:pos + 1 + size])
        raw = " ".join(f"{b:02x}" for b in code[pos:pos + 1 + size])
        text = fmt.format(*ops, n=(ops[1] + 256 * ops[2]) if len(ops) == 3 else 0)
        lines.append(f"{pos:04x}  {raw:<14}  {name} {text}".rstrip())
        pos += 1 + size
    return lines


if __name__ == "__main__":
    isa, path = sys.argv[1:3]
    print("\n".join(disassemble(open(path, "rb").read(), STACK if isa == "stack" else REG)))
