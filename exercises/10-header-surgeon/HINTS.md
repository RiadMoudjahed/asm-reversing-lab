[English](HINTS.md) | [العربية](HINTS.ar.md) | [Deutsch](HINTS.de.md) | [Español](HINTS.es.md) | [Français](HINTS.fr.md)

# Hints: exercise 10

Open one level at a time. Try again between levels.

## Part 1: Reverse

<details>
<summary>Level 1: nudge</summary>

Run `readelf -h`, `readelf -l`, `readelf -S` and `readelf -d` on the file. Every value you need is in one of these four listings.

</details>

<details>
<summary>Level 2: direction</summary>

The image base is the `VirtAddr` of the first `LOAD` entry. The entry point is a virtual address; the first `LOAD` entry has file offset 0, so the file offset is the entry point minus the base. The program header count is in the ELF header. `MemSiz` and `FileSiz` are columns of the `LOAD` entries. Section names that you do not recognize are rare; look for one in the section list. The `NEEDED` entries are in the dynamic section.

</details>

<details>
<summary>Level 3: nearly the answer</summary>

Use the `LOAD` entry with the flags `RW` for `M`: subtract the two sizes. For `V`, the section that is not in the usual list of `.text`, `.data`, `.rodata`, `.bss` and so on has a size in the `Size` column; it is in hexadecimal, so convert it. There are two `NEEDED` libraries; one is the C library `libc.so.6`, and the other gives `L`.

</details>

## Part 2: Assemble

<details>
<summary>Level 1: nudge</summary>

Write the file in the order it appears on disk: ELF header, program headers, code, padding, data. Use `org` with the image base so that labels have the right virtual addresses, and use `$$` to compute file offsets.

</details>

<details>
<summary>Level 2: direction</summary>

You need three program headers: a `PT_LOAD` for the headers and code (flags 5), a `PT_LOAD` for the message (flags 6) and a `PT_GNU_STACK` (flags 6). The ELF header is 64 bytes and each program header is 56 bytes. Put the message at a file offset such as 0x200, pad the file up to it with `times`, and give the segment a virtual address that has the same remainder modulo 0x1000: for example the image base plus 0x1000 plus 0x200.

</details>

<details>
<summary>Level 3: nearly the answer</summary>

In the ELF header set `e_phoff` to 64, `e_phentsize` to 56 and `e_phnum` to 3, and leave `e_shoff`, `e_shentsize`, `e_shnum` and `e_shstrndx` at zero. For the first segment use `p_offset` 0, `p_vaddr` equal to the base and `p_filesz` equal to the end of the code. The code does `write` with the data address in `esi` and then `exit`. If the checker says the offset and the address must agree modulo `p_align`, subtract the two and check that the difference is a multiple of 0x1000. If the program dies with signal 11, the entry point or the message address is outside the mapped ranges.

</details>
