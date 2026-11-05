[English](WRITEUP.md) | [العربية](WRITEUP.ar.md) | [Deutsch](WRITEUP.de.md) | [Español](WRITEUP.es.md) | [Français](WRITEUP.fr.md)

# Write-up: exercise 11. Portable puzzle

The flag is `ARL{portable_11_150000000_2410_8d48_4_13}`: the file has 11 sections, the image base is 0x150000000, the entry point RVA is 0x2410, RVA 0x12348 is at file offset 0x8d48, four libraries are imported, and 13 functions come from `KERNEL32.dll`.

## Part 1: Reverse

### The headers

```bash
x86_64-w64-mingw32-objdump -p bin/puzzle.exe | grep -E "^(ImageBase|AddressOfEntryPoint|SectionAlignment|FileAlignment)|DLL Name"
```

```text
AddressOfEntryPoint    0000000000002410
ImageBase              0000000150000000
SectionAlignment       00002000
FileAlignment          00000200
    DLL Name: ADVAPI32.dll
    DLL Name: KERNEL32.dll
    DLL Name: msvcrt.dll
    DLL Name: USER32.dll
```

The columns of `objdump -p` are separated by tabs; they are shown here with spaces. `AddressOfEntryPoint` is an RVA, so `R` is `2410` and not the sum with the image base. `ImageBase` gives `I` as `150000000`. There are four `DLL Name` lines, so `D` is 4.

### The sections

```bash
python3 ../../modules/12-executable-formats/examples/12_pe_dump.py bin/puzzle.exe
```

```text
name      virtual size  RVA       raw size  raw offset  flags
.text     0x00006cc8    0x002000  0x006e00  0x000400    0x60000060
.data     0x000000d0    0x00a000  0x000200  0x007200    0xc0000040
.rdata    0x00000dc0    0x00c000  0x000e00  0x007400    0x40000040
.ledger   0x00000020    0x00e000  0x000200  0x008200    0x40000040
.pdata    0x00000480    0x010000  0x000600  0x008400    0x40000040
.xdata    0x00000528    0x012000  0x000600  0x008a00    0x40000040
.bss      0x00000da0    0x014000  0x000000  0x000000    0xc0000080
.idata    0x00000780    0x016000  0x000800  0x009000    0xc0000040
.CRT      0x00000060    0x018000  0x000200  0x009800    0xc0000040
.tls      0x00000010    0x01a000  0x000200  0x009a00    0xc0000040
.reloc    0x00000084    0x01c000  0x000200  0x009c00    0x42000040
```

There are 11 rows, so `S` is 11. The section `.ledger` has no standard name; it is not part of the questions.

For `O`, find the section that contains RVA 0x12348. `.xdata` starts at RVA 0x12000 and its raw size is 0x600, so it covers 0x12000 to 0x125ff. Apply the formula from module 12:

```text
file offset = RVA - section RVA + section raw offset
            = 0x12348 - 0x12000 + 0x8a00
            = 0x348 + 0x8a00
            = 0x8d48
```

### The imports

The import directory is in `.idata`. Counting the names of each library in the import name table gives these numbers for `KERNEL32.dll`:

```text
DeleteCriticalSection, EnterCriticalSection, GetLastError, InitializeCriticalSection,
IsDBCSLeadByteEx, LeaveCriticalSection, MultiByteToWideChar, SetUnhandledExceptionFilter,
Sleep, TlsGetValue, VirtualProtect, VirtualQuery, WideCharToMultiByte
```

That is 13 functions, so `K` is 13. The script from module 12 prints only the first six names of each library, so it is not enough for this question; counting entries in the table or reading `objdump -p` is.

The flag is `ARL{portable_11_150000000_2410_8d48_4_13}`.

## Part 2: Assemble

The reference solution is `solution.asm`. The points that matter:

- **Reading the section table.** Each entry is 40 bytes. The virtual address is at offset 12, the size of raw data at offset 16 and the pointer to raw data at offset 20. Module 12's script unpacks them in the order name, virtual size, virtual address, raw size, raw offset.
- **The membership test.** `rva - VirtualAddress` is computed in 32 bits and compared with the size of raw data using an unsigned jump (`jae`). If the RVA is below the section, the subtraction wraps to a large number, so one comparison replaces the two checks "not below" and "below the end".
- **Virtual size is not used.** A section can have a virtual size larger than its raw size (the bytes are zeroes in memory). Those RVAs have no file offset, and the function returns `0xFFFFFFFF` for them.
- **`pe_entry_offset`.** It reads `e_lfanew` at offset 0x3c, adds it to the file pointer, and checks `PE\0\0`. The entry point RVA is at NT headers + 24 + 16; the number of sections is at NT headers + 6; the size of the optional header is at NT headers + 20, and the section table starts at NT headers + 24 + that size.
- **The frame.** The function makes a call, so it saves `rbx` with `push`, which leaves `rsp` aligned. It then reserves 48 bytes: 32 for the shadow space and 16 for a local that holds the entry RVA. The local is above the shadow space because `log_rva` writes into the 32 bytes just above its return address.
- **The calls.** `log_rva` destroys the volatile registers, so the entry RVA is reloaded from the stack for the call to `pe_rva_to_offset`, and the section table pointer is recomputed from `rbx`.

## Mistakes that cost time

- **A frame of 40 bytes.** The stack is 8 modulo 16 at the call; the checker reports `align` in every test that reaches `log_rva`.
- **Using `rbx` without saving it.** The results are right, and the checker reports `regs`.
- **Using `esi` as a scratch register.** In the System V convention `rsi` is volatile, but in the Microsoft convention it is not. The checker reports `regs` in every case.
- **Not calling `log_rva`, or calling it with a different value.** The checker reports `calls`.
- **A signed comparison.** `jge` instead of `jae` makes RVAs below a section look as if they were inside it; the cases "below the first section", "rva zero" and "largest rva" fail.
- **Comparing with the virtual size.** The case where the RVA is in the zero-filled tail of a section returns an offset instead of `0xFFFFFFFF`.
- **Keeping a local in the shadow space.** `log_rva` writes over it and the entry RVA is wrong afterwards.
