[English](pe.md) | [العربية](pe.ar.md) | [Deutsch](pe.de.md) | [Español](pe.es.md) | [Français](pe.fr.md)

# PE

The executable format of Windows. Lesson: [12. Executable formats](../modules/12-executable-formats/README.md). Offsets below were checked against a 64-bit MinGW executable.

## Layout of a file

| Part | Where | Size |
|---|---|---|
| DOS header | Offset 0, starts with `MZ` | 64 bytes; the field at 0x3c is `e_lfanew` |
| DOS stub | After the DOS header | Varies |
| NT signature | At `e_lfanew` | 4 bytes: `PE\0\0` |
| File header | Right after the signature | 20 bytes |
| Optional header | Right after the file header | 240 bytes for PE32+, 224 for PE32 |
| Section table | Right after the optional header | 40 bytes per section |
| Section data | At the raw offsets | Aligned to the file alignment |

## File header

| Offset | Size | Field | Notes |
|---|---|---|---|
| 0 | 2 | Machine | 0x8664 x86-64, 0x14c x86 |
| 2 | 2 | Number of sections | |
| 16 | 2 | Size of optional header | Where the section table starts |
| 18 | 2 | Characteristics | 0x2000 marks a DLL |

## Optional header (offsets from its start)

| Offset | Size | Field | Notes |
|---|---|---|---|
| 0 | 2 | Magic | 0x10b PE32, 0x20b PE32+ |
| 16 | 4 | Entry point | An RVA |
| 24 | 8 | Image base | PE32+. In PE32 the image base is a 4-byte field at offset 28 |
| 32 | 4 | Section alignment | In memory, usually 0x1000 |
| 36 | 4 | File alignment | In the file, usually 0x200 |
| 56 | 4 | Size of image | Total memory size, a multiple of the section alignment |
| 60 | 4 | Size of headers | Rounded up to the file alignment |
| 108 | 4 | Number of data directories | Usually 16 (PE32+) |
| 112 | 8 each | Data directories | `(RVA, size)` pairs (PE32+). In PE32 they start at offset 96 |

## Data directories

| Index | Directory |
|---|---|
| 0 | Export |
| 1 | Import |
| 2 | Resource |
| 3 | Exception (`.pdata`, 64-bit) |
| 4 | Certificate (a file offset, not an RVA) |
| 5 | Base relocation |
| 6 | Debug |
| 9 | Thread-local storage |
| 12 | Import address table |
| 13 | Delay import |

## Section header (40 bytes)

| Offset | Size | Field |
|---|---|---|
| 0 | 8 | Name, padded with zeros |
| 8 | 4 | Virtual size |
| 12 | 4 | RVA |
| 16 | 4 | Raw size |
| 20 | 4 | Raw offset |
| 36 | 4 | Characteristics |

Flag bits of the characteristics: 0x20 code, 0x40 initialized data, 0x80 uninitialized data, 0x20000000 executable, 0x40000000 readable, 0x80000000 writable.

## Common sections

| Section | Content |
|---|---|
| `.text` | Code |
| `.rdata` | Read-only data, import names, and often the import directory |
| `.data`, `.bss` | Initialized and zero-initialized writable data |
| `.idata` | Import directory and tables (MinGW) |
| `.edata` | Export directory |
| `.pdata`, `.xdata` | Unwind and exception data (64-bit) |
| `.reloc` | Base relocations |
| `.rsrc` | Resources |
| `.tls` | Thread-local storage template |

## Conversions

- RVA to file offset: find the section with `section RVA <= RVA < section RVA + virtual size`, then `offset = RVA - section RVA + section raw offset`.
- Virtual address: `image base + RVA`. With ASLR the loader may choose another base, and the relocation table fixes the absolute addresses.
- A section's virtual size can be larger or smaller than its raw size. The part beyond the raw data is zero-filled.

## Imports

An array of descriptors, one per DLL, ends with an all-zero descriptor. Each one points to the DLL name, the import name table (what is wanted) and the import address table (where the loader writes the addresses). Calls in the code go through the IAT, as `call [rip + disp]`.

## Commands

```bash
file prog.exe                                  # kind, bits
x86_64-w64-mingw32-objdump -x prog.exe         # headers, sections, imports
x86_64-w64-mingw32-objdump -d -M intel prog.exe
x86_64-w64-mingw32-objdump -p prog.exe         # import tables and data directories
strings -a -t x prog.exe
python3 modules/12-executable-formats/examples/12_pe_dump.py prog.exe
```
