#!/usr/bin/env python3
"""Module 22: print the managed-code structure of a .NET assembly without any .NET tool.

Usage: 22_meta.py Assembly.dll
Shows the CLR header, the metadata streams, the number of rows in the main metadata tables and
the user strings (the #US heap, stored as UTF-16, which a plain ASCII string search does not find).
"""
import struct
import sys

import pefile

TABLES = {0x00: "Module", 0x01: "TypeRef", 0x02: "TypeDef", 0x04: "Field", 0x06: "MethodDef",
          0x08: "Param", 0x0A: "MemberRef", 0x0C: "CustomAttribute", 0x20: "Assembly",
          0x23: "AssemblyRef"}


def read_user_strings(heap):
    """Entries are a compressed length, UTF-16 text and one final flag byte."""
    pos, out = 1, []
    while pos < len(heap):
        first = heap[pos]
        if first & 0x80 == 0:
            size, pos = first, pos + 1
        elif first & 0xC0 == 0x80:
            size, pos = ((first & 0x3F) << 8) | heap[pos + 1], pos + 2
        else:
            size, pos = ((first & 0x1F) << 24) | (heap[pos + 1] << 16) | (heap[pos + 2] << 8) | heap[pos + 3], pos + 4
        if size == 0:
            continue
        out.append((pos, heap[pos:pos + size - 1].decode("utf-16-le", "replace")))
        pos += size
    return out


def main():
    pe = pefile.PE(sys.argv[1])
    com = pe.OPTIONAL_HEADER.DATA_DIRECTORY[14]
    print(f"CLR header: rva 0x{com.VirtualAddress:x}, size {com.Size}")
    data = pe.get_data(com.VirtualAddress, com.Size)
    cb, major, minor, md_rva, md_size, flags, entry = struct.unpack_from("<IHHIIII", data)
    print(f"  runtime version {major}.{minor}, flags 0x{flags:x}, entry point token 0x{entry:08x}")
    md = pe.get_data(md_rva, md_size)
    sig, _, _, _, vlen = struct.unpack_from("<IHHII", md)
    print(f"metadata: signature {md[:4].decode()}, version string {md[16:16 + vlen].split(bytes(1))[0].decode()}")
    pos = 16 + vlen + 2
    (count,) = struct.unpack_from("<H", md, pos)
    pos += 2
    streams = {}
    for _ in range(count):
        offset, size = struct.unpack_from("<II", md, pos)
        end = md.index(b"\0", pos + 8)
        name = md[pos + 8:end].decode()
        pos = (end + 4) & ~3
        streams[name] = (offset, size)
        print(f"  stream {name:<8} offset 0x{offset:05x}  size {size}")
    off, size = streams["#~"]
    tbl = md[off:off + size]
    heap_sizes = tbl[6]
    (valid,) = struct.unpack_from("<Q", tbl, 8)
    rows, p = {}, 24
    for bit in range(64):
        if valid >> bit & 1:
            (rows[bit],) = struct.unpack_from("<I", tbl, p)
            p += 4
    print("table rows: " + ", ".join(f"{TABLES.get(t, hex(t))} {n}" for t, n in rows.items() if t in TABLES))
    off, size = streams["#US"]
    print("user strings (#US heap):")
    for where, text in read_user_strings(md[off:off + size]):
        print(f"  +0x{where:03x}  {text!r}")


if __name__ == "__main__":
    main()
