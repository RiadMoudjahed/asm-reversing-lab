#!/usr/bin/env python3
"""Module 20 example: run a small decoding routine of an ELF file in an emulator.

Usage: 20_emulate.py ELF FUNC BLOB LENGTH KEY
  FUNC    address of the decoder (hexadecimal)
  BLOB    address of the encoded bytes (hexadecimal)
  LENGTH  number of bytes to decode
  KEY     the key or seed argument (hexadecimal)

The routine is called as decode(out, blob, length, key), as the code in the lesson does. The
program itself is never run; only the routine's own instructions are emulated, with no system
calls and no access to anything outside the emulated memory.
"""
import sys

from elftools.elf.elffile import ELFFile
from unicorn import UC_ARCH_X86, UC_HOOK_CODE, UC_MODE_64, Uc, UcError
from unicorn.x86_const import UC_X86_REG_RCX, UC_X86_REG_RDI, UC_X86_REG_RDX, UC_X86_REG_RSI, UC_X86_REG_RSP

PAGE = 0x1000
STACK, OUT, RET = 0x7FF00000, 0x10000000, 0x20000000


def load(mu, path):
    with open(path, "rb") as f:
        elf = ELFFile(f)
        for seg in elf.iter_segments():
            if seg["p_type"] != "PT_LOAD":
                continue
            start = seg["p_vaddr"] & ~(PAGE - 1)
            end = (seg["p_vaddr"] + seg["p_memsz"] + PAGE - 1) & ~(PAGE - 1)
            try:
                mu.mem_map(start, end - start)
            except UcError:
                pass                                    # overlaps a page that is already mapped
            mu.mem_write(seg["p_vaddr"], seg.data())


def main():
    path = sys.argv[1]
    func, blob, length, key = int(sys.argv[2], 16), int(sys.argv[3], 16), int(sys.argv[4], 0), int(sys.argv[5], 16)
    mu = Uc(UC_ARCH_X86, UC_MODE_64)
    load(mu, path)
    mu.mem_map(STACK, 0x10000)
    mu.mem_map(OUT, PAGE)
    mu.mem_map(RET, PAGE)
    sp = STACK + 0x8000
    mu.mem_write(sp, RET.to_bytes(8, "little"))        # return address of the routine
    mu.reg_write(UC_X86_REG_RSP, sp)
    mu.reg_write(UC_X86_REG_RDI, OUT)
    mu.reg_write(UC_X86_REG_RSI, blob)
    mu.reg_write(UC_X86_REG_RDX, length)
    mu.reg_write(UC_X86_REG_RCX, key)
    steps = [0]
    mu.hook_add(UC_HOOK_CODE, lambda uc, addr, size, data: steps.__setitem__(0, steps[0] + 1))
    mu.emu_start(func, RET, count=100000)
    out = bytes(mu.mem_read(OUT, length + 1)).split(b"\0")[0]
    print(f"{steps[0]} instructions emulated")
    print(out.decode("latin-1"))


main()
