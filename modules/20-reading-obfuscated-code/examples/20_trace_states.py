#!/usr/bin/env python3
"""Module 20 example: recover the control flow of a flattened function by tracing its state.

Usage: 20_trace_states.py ELF FUNC STATE_ADDRESS
Emulates the function for the inputs 0 to 14 and records every value stored to the state
variable. The sequences, put together, are the edges of the original control-flow graph.
"""
import sys

from elftools.elf.elffile import ELFFile
from unicorn import UC_ARCH_X86, UC_HOOK_MEM_WRITE, UC_MODE_64, Uc, UcError
from unicorn.x86_const import UC_X86_REG_EAX, UC_X86_REG_RDI, UC_X86_REG_RSP

PAGE = 0x1000
STACK, RET = 0x7FF00000, 0x20000000


def main():
    path, func, state = sys.argv[1], int(sys.argv[2], 16), int(sys.argv[3], 16)
    edges = {}
    for n in range(15):
        mu = Uc(UC_ARCH_X86, UC_MODE_64)
        with open(path, "rb") as f:
            for seg in ELFFile(f).iter_segments():
                if seg["p_type"] != "PT_LOAD":
                    continue
                s = seg["p_vaddr"] & ~(PAGE - 1)
                e = (seg["p_vaddr"] + seg["p_memsz"] + PAGE - 1) & ~(PAGE - 1)
                try:
                    mu.mem_map(s, e - s)
                except UcError:
                    pass
                mu.mem_write(seg["p_vaddr"], seg.data())
        mu.mem_map(STACK, 0x10000)
        mu.mem_map(RET, PAGE)
        sp = STACK + 0x8000
        mu.mem_write(sp, RET.to_bytes(8, "little"))
        mu.reg_write(UC_X86_REG_RSP, sp)
        mu.reg_write(UC_X86_REG_RDI, n)
        seen = []

        def on_write(uc, access, address, size, value, data):
            if address == state and size == 4:
                seen.append(value)

        mu.hook_add(UC_HOOK_MEM_WRITE, on_write)
        mu.emu_start(func, RET, count=10000)
        result = mu.reg_read(UC_X86_REG_EAX)
        print(f"n={n:2}: states {' -> '.join(map(str, seen))}  result {result}")
        for a, b in zip(seen, seen[1:]):
            edges.setdefault(a, set()).add(b)
    print("edges of the recovered graph:")
    for a in sorted(edges):
        print(f"  {a} -> {', '.join(map(str, sorted(edges[a])))}")


main()
