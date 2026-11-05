[English](HINTS.md) | [العربية](HINTS.ar.md) | [Deutsch](HINTS.de.md) | [Español](HINTS.es.md) | [Français](HINTS.fr.md)

# Hints: exercise 11

Open one level at a time. Try again between levels.

## Part 1: Reverse

<details>
<summary>Level 1: nudge</summary>

`objdump -p` prints the optional header values, the section table and the import tables. The script from module 12 prints the section table with both the RVAs and the file offsets.

</details>

<details>
<summary>Level 2: direction</summary>

The image base, the entry point RVA and the section count are fields of the headers. For `O`, find the section whose RVA range contains 0x12348 and apply: file offset = RVA - section RVA + section raw offset. The script from module 12 shows only the first few function names of each library; for `K`, count every name in the `KERNEL32.dll` import list, for example with `objdump -p`.

</details>

<details>
<summary>Level 3: nearly the answer</summary>

Check the section list for the one that starts at or just below 0x12348, then make sure that 0x12348 is below its start plus its raw size. The result is in hexadecimal, written without `0x`. The number of imported libraries is the number of `DLL Name` lines. The entry point value is an RVA, not an address: do not add the image base.

</details>

## Part 2: Assemble

<details>
<summary>Level 1: nudge</summary>

Start with `pe_rva_to_offset`: it needs only volatile registers and no calls. Write it as a loop over the section table, test the RVA against each section, and return `0xFFFFFFFF` after the loop. Then write `pe_entry_offset` on top of it.

</details>

<details>
<summary>Level 2: direction</summary>

A section header is 40 bytes. The fields you need are the virtual address, the size of raw data and the pointer to raw data; read their positions from the section table that you printed in Part 1. For the unsigned test, subtract the virtual address from the RVA and compare the result with the size of raw data using an unsigned jump: if the RVA is below the section, the subtraction wraps to a huge number and the comparison fails by itself. In `pe_entry_offset`, the optional header starts 24 bytes after the NT signature, the entry point RVA is at offset 16 inside it, and the section table starts after the optional header, whose size is a field of the file header.

</details>

<details>
<summary>Level 3: nearly the answer</summary>

`pe_entry_offset` makes a call, so it needs a frame: save the address of the NT headers in `rbx` (push it first), reserve at least 32 bytes of shadow space, and make sure `rsp` is a multiple of 16 at the call. After one `push` the stack is aligned, so subtract a multiple of 16. Keep the entry RVA in a stack slot above the shadow space, because `log_rva` destroys `rcx` and every other volatile register. If the checker prints `align`, the frame size is wrong; `regs` means a non-volatile register was changed and not restored (remember `rsi` and `rdi`); `calls` means `log_rva` was not called exactly once with the entry RVA.

</details>
