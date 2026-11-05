[English](elf.md) | [العربية](elf.ar.md) | [Deutsch](elf.de.md) | [Español](elf.es.md) | [Français](elf.fr.md)

# ELF

The executable format of Linux. Lesson: [12. Executable formats](../modules/12-executable-formats/README.md).

## Layout of a file

| Part | Where | Used by |
|---|---|---|
| ELF header | Offset 0, 64 bytes in a 64-bit file | Everything |
| Program header table | At `e_phoff`, usually 64 | The loader |
| Section data | Between the tables | Both |
| Section header table | At `e_shoff`, usually at the end | Linker, debugger, reverser (not needed to run) |

The loader reads only the program headers. A file with its section headers removed still runs.

## ELF header fields (64-bit)

| Offset | Size | Field | Meaning |
|---|---|---|---|
| 0 | 4 | `e_ident` magic | `7f 45 4c 46` (`\x7fELF`) |
| 4 | 1 | Class | 1 = 32-bit, 2 = 64-bit |
| 5 | 1 | Data | 1 = little endian, 2 = big endian |
| 16 | 2 | `e_type` | 2 = EXEC, 3 = DYN (shared object or position-independent executable) |
| 18 | 2 | `e_machine` | 3 = x86, 62 = x86-64 |
| 24 | 8 | `e_entry` | Virtual address of the first instruction |
| 32 | 8 | `e_phoff` | File offset of the program header table |
| 40 | 8 | `e_shoff` | File offset of the section header table |
| 54 | 2 | `e_phentsize` | Size of one program header (56) |
| 56 | 2 | `e_phnum` | Number of program headers |
| 58 | 2 | `e_shentsize` | Size of one section header (64) |
| 60 | 2 | `e_shnum` | Number of section headers |
| 62 | 2 | `e_shstrndx` | Index of the section that holds section names |

## Program header types

| Type | Meaning |
|---|---|
| `PT_LOAD` | A range of the file to map: file offset, virtual address, file size, memory size, permissions |
| `PT_INTERP` | Path of the dynamic loader |
| `PT_DYNAMIC` | The dynamic section: needed libraries, relocation tables |
| `PT_NOTE` | Notes such as the build identifier |
| `PT_GNU_STACK` | Permissions of the stack |
| `PT_GNU_RELRO` | Range made read-only after relocation |

A segment whose memory size exceeds its file size is zero-filled beyond the file data. This is how `.bss` takes no space in the file.

## Common sections

| Section | Content |
|---|---|
| `.text` | Code |
| `.rodata` | Read-only data, string literals |
| `.data` | Initialized writable data |
| `.bss` | Zero-initialized data, no bytes in the file |
| `.plt`, `.plt.sec` | Stubs that jump through the GOT |
| `.got`, `.got.plt` | Addresses filled in by the dynamic loader |
| `.dynsym`, `.dynstr` | Symbols and names needed at load time |
| `.symtab`, `.strtab` | Full symbol table; removed by `strip` |
| `.rela.dyn`, `.rela.plt` | Relocation entries |
| `.init_array`, `.fini_array` | Functions run before `main` and at exit |
| `.eh_frame` | Unwind information |

## Dynamic calls

A call to a library function goes to a stub in the PLT. The stub jumps through an entry in the GOT. With lazy binding, the first call lands in the dynamic loader, which resolves the name and writes the real address into the GOT entry; later calls go straight there. A binary linked with `-z now` resolves everything at start.

## Commands

```bash
file prog                    # kind, bits, stripped or not
readelf -h prog              # ELF header
readelf -lW prog             # program headers and the section-to-segment map
readelf -SW prog             # section headers
readelf -sW prog             # symbol tables
readelf -dW prog             # dynamic section (needed libraries)
readelf -rW prog             # relocations
objdump -d -M intel prog     # disassemble code sections
objdump -s -j .rodata prog   # hex dump of one section
nm -D prog                   # dynamic symbols
ldd prog                     # libraries the loader would bring in
strings -a -t x prog         # printable strings with file offsets
```

## Conversions

- File offset of an address in a `PT_LOAD` segment: `offset = address - p_vaddr + p_offset`.
- Check the segment, not the section, when the two disagree. The loader uses segments.
- A position-independent executable (`e_type` 3) is loaded at a random base. Addresses in the file are relative to that base.
