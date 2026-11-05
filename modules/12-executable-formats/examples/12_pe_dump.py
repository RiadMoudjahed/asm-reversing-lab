#!/usr/bin/env python3
"""Print the main structures of a 64-bit PE file using only the standard library.

Usage: python3 12_pe_dump.py FILE.exe
Shows the DOS header pointer, the NT headers, the section table and the import table,
and converts one RVA to a file offset by hand.
"""
import struct
import sys


def main(path):
    data = open(path, "rb").read()
    if data[:2] != b"MZ":
        sys.exit("not a PE file: missing MZ signature")
    (e_lfanew,) = struct.unpack_from("<I", data, 0x3C)
    if data[e_lfanew:e_lfanew + 4] != b"PE\0\0":
        sys.exit("missing PE signature")
    print(f"e_lfanew (offset of the NT headers): 0x{e_lfanew:x}")

    machine, nsections, _, _, _, size_opt, characteristics = struct.unpack_from(
        "<HHIIIHH", data, e_lfanew + 4)
    print(f"machine: 0x{machine:x}  sections: {nsections}  optional header size: {size_opt}")

    opt = e_lfanew + 24
    magic, = struct.unpack_from("<H", data, opt)
    entry, = struct.unpack_from("<I", data, opt + 16)
    image_base, = struct.unpack_from("<Q", data, opt + 24)
    section_align, file_align = struct.unpack_from("<II", data, opt + 32)
    size_image, size_headers = struct.unpack_from("<II", data, opt + 56)
    print(f"optional header magic: 0x{magic:x} (0x20b means PE32+)")
    print(f"entry point RVA: 0x{entry:x}  image base: 0x{image_base:x}")
    print(f"section alignment: 0x{section_align:x}  file alignment: 0x{file_align:x}")

    dirs = opt + 112
    import_rva, import_size = struct.unpack_from("<II", data, dirs + 8)
    print(f"import directory: RVA 0x{import_rva:x}, size 0x{import_size:x}")

    sections = []
    table = opt + size_opt
    print("\nname      virtual size  RVA       raw size  raw offset  flags")
    for i in range(nsections):
        name, vsize, rva, rsize, roff = struct.unpack_from("<8sIIII", data, table + 40 * i)
        flags, = struct.unpack_from("<I", data, table + 40 * i + 36)
        name = name.rstrip(b"\0").decode()
        sections.append((name, vsize, rva, rsize, roff))
        print(f"{name:<9} 0x{vsize:08x}    0x{rva:06x}  0x{rsize:06x}  0x{roff:06x}    0x{flags:08x}")

    def rva_to_offset(rva):
        for name, vsize, srva, rsize, roff in sections:
            if srva <= rva < srva + max(vsize, rsize):
                return rva - srva + roff
        return None

    print(f"\nentry point RVA 0x{entry:x} is at file offset 0x{rva_to_offset(entry):x}")

    print("\nimported libraries and functions:")
    pos = rva_to_offset(import_rva)
    shown = 0
    while True:
        oft, _, _, name_rva, iat = struct.unpack_from("<IIIII", data, pos)
        if name_rva == 0:
            break
        dll = data[rva_to_offset(name_rva):].split(b"\0")[0].decode()
        names = []
        thunk = rva_to_offset(oft or iat)
        while True:
            (value,) = struct.unpack_from("<Q", data, thunk)
            if value == 0:
                break
            if not value >> 63:
                hint_name = rva_to_offset(value & 0x7FFFFFFF)
                names.append(data[hint_name + 2:].split(b"\0")[0].decode())
            thunk += 8
        print(f"  {dll}: {', '.join(names[:6])}{' ...' if len(names) > 6 else ''}")
        pos += 20
        shown += 1


if __name__ == "__main__":
    if len(sys.argv) != 2:
        sys.exit(__doc__)
    main(sys.argv[1])
