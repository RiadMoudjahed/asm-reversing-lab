[English](WRITEUP.md) | [العربية](WRITEUP.ar.md) | [Deutsch](WRITEUP.de.md) | [Español](WRITEUP.es.md) | [Français](WRITEUP.fr.md)

# Write-up: exercise 10. Header surgeon

The flag is `ARL{header_10000000_520_11_152_17_m}`: the image base is 0x10000000, the entry point is at file offset 0x520, there are 11 program headers, the writable segment has 152 bytes that exist only in memory, the custom section `.vault` is 17 bytes, and the extra library is `libm`.

## Part 1: Reverse

### The ELF header and the program headers

```bash
readelf -h -l -W bin/surgeon
```

```text
  Type:                              EXEC (Executable file)
  Entry point address:               0x10000520
  Number of program headers:         11

Program Headers:
  Type           Offset   VirtAddr           PhysAddr           FileSiz  MemSiz   Flg Align
  LOAD           0x000000 0x0000000010000000 0x0000000010000000 0x000760 0x000760 R E 0x1000
  LOAD           0x000de8 0x0000000010001de8 0x0000000010001de8 0x000240 0x0002d8 RW  0x1000
```

The listing is shortened to the lines that answer the questions.

| Value | Where it comes from |
|---|---|
| `B` | The first `LOAD` entry has `VirtAddr` 0x10000000, the lowest of the two. Write `10000000` |
| `E` | The entry point is 0x10000520. The first `LOAD` entry maps file offset 0 to 0x10000000, so the file offset is 0x10000520 - 0x10000000 = 0x520. Write `520` |
| `P` | `Number of program headers: 11` |
| `M` | The `RW` `LOAD` entry has `MemSiz` 0x2d8 and `FileSiz` 0x240; the difference is 0x98, which is 152 in decimal |

The value 0x10000000 is unusual for a non-position-independent program, whose default is 0x400000. The linker was told to place the first segment higher, and every address in the file follows from that base.

`M` is larger than the 0x80 bytes of the `.bss` section, because the segment also extends over alignment space between the end of the file data and the start of `.bss`.

### The sections

```bash
readelf -S -W bin/surgeon | grep -E "\.(rodata|vault|data|bss) "
```

```text
  [15] .rodata           PROGBITS        00000000100006a4 0006a4 00001c 00   A  0   0  4
  [16] .vault            PROGBITS        00000000100006c0 0006c0 000011 00   A  0   0 16
  [24] .data             PROGBITS        0000000010002010 001010 000018 00  WA  0   0  8
  [25] .bss              NOBITS          0000000010002040 001028 000080 00  WA  0   0 32
```

`.vault` is the only section whose name is not a standard one. Its `Size` column is 0x11, which is 17 decimal. The section list is shown in an abbreviated form; the other sections are `.interp`, `.dynsym`, `.text`, `.dynamic`, `.got` and similar standard names.

### The libraries

```bash
readelf -d bin/surgeon | grep NEEDED
```

```text
 0x0000000000000001 (NEEDED)             Shared library: [libm.so.6]
 0x0000000000000001 (NEEDED)             Shared library: [libc.so.6]
```

The library other than the C library is `libm.so.6`; without the `lib` prefix and the `.so.6` suffix it is `m`.

The flag is `ARL{header_10000000_520_11_152_17_m}`.

## Part 2: Assemble

The reference solution is `solution.asm`. The layout:

```text
file offset   0x000  ELF header                  64 bytes
              0x040  PT_LOAD, R + X              56 bytes
              0x078  PT_LOAD, R + W              56 bytes
              0x0b0  PT_GNU_STACK                56 bytes
              0x0e8  code                        the end is at 0x107
              0x200  message (18 bytes)
```

The points that matter:

- **The image base.** `org 0x10000000` makes every label an absolute virtual address, and `$$` is the start of the file, so `label - $$` is a file offset. The base is the one found in Part 1.
- **The first segment.** `p_offset` is 0 and `p_vaddr` is the base, so the headers are inside the segment, and the entry point 0x10000000 + 0xe8 is an address inside it. `p_filesz` is the end of the code.
- **The data segment.** It is at file offset 0x200 and virtual address base + 0x1200. The two values leave the same remainder, 0x200, modulo the alignment 0x1000. That is the condition under which the loader can map a page of the file directly. With a mismatch the loader maps the page at the wrong place and the program crashes on the first access.
- **The stack header.** `PT_GNU_STACK` with flags 6 says that the program needs a readable and writable stack and does not need to execute code on it. Compilers always emit it; this exercise requires it.
- **The code.** `write` takes the data address in `esi`, which is 0x10001200; the size of the message is in `edx`. `exit` follows with status 0.

The file is 530 bytes: 0x200 bytes of headers, code and padding, and 18 bytes of message.

## Mistakes that cost time

- **Using the default image base 0x400000.** Everything else is correct and the checker reports that the lowest `PT_LOAD` address must match `bin/surgeon`.
- **A segment with flags 7.** A read, write and execute segment is rejected, and the segment is also not found as a read-write segment.
- **An offset and an address that disagree modulo 0x1000.** The checker reports the congruence rule, and the program dies with signal 11 because the loader maps the page at the wrong place.
- **No `PT_GNU_STACK`.** The program runs, but the checker asks for the header. If `e_phnum` is smaller than the number of headers you wrote, the last header is ignored.
- **An entry point outside the code segment.** The program dies with signal 11 as soon as it starts.
- **A message with different text.** The checker reports that the text is missing from the file and that the output differs.
- **A `class` byte of 1 in `e_ident`.** The file claims to be a 32-bit ELF. The program may still run, but the header is wrong and the checker reports it.
