[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# 12. Executable formats: ELF and PE

Time: about 3 hours. Prerequisites: [11. The reverser's toolkit](../11-toolkit/README.md)

## Why this matters

Every executable is a file that tells the loader what to put in memory, where, with which permissions, which libraries to bring in, and where to start. If you can read those headers you can identify a binary at a glance, find its code and data without a disassembler, understand imports and exports, and notice a file that has been built or modified unusually. Linux uses ELF and Windows uses PE; this module covers both, and ends with writing a complete executable by hand.

## Concepts

### Two views of one file

Both formats describe the same file in two ways.

- The **loading view** says how to map the file into memory: which byte ranges, to which addresses, with which permissions. The loader uses only this view.
- The **linking view** divides the file into named parts (sections) for the linker, the debugger and the reverser.

Keep two kinds of address apart. A **file offset** counts bytes from the start of the file. A **virtual address** is where those bytes will be in memory. They differ because the loader aligns pieces to page boundaries, and because some pieces, such as uninitialized data, take memory but no file space. Converting between them is a daily task.

### ELF

An ELF file starts with the **ELF header**, followed by the program header table, the section data, and the section header table.

#### The ELF header

Real output for the sample program built from `examples/12_sample.c`, run from `modules/12-executable-formats/examples`:

```bash
make
readelf -h build/12_sample
```

```text
ELF Header:
  Magic:   7f 45 4c 46 02 01 01 00 00 00 00 00 00 00 00 00 
  Class:                             ELF64
  Data:                              2's complement, little endian
  Version:                           1 (current)
  OS/ABI:                            UNIX - System V
  ABI Version:                       0
  Type:                              EXEC (Executable file)
  Machine:                           Advanced Micro Devices X86-64
  Version:                           0x1
  Entry point address:               0x401050
  Start of program headers:          64 (bytes into file)
  Start of section headers:          14008 (bytes into file)
  Flags:                             0x0
  Size of this header:               64 (bytes)
  Size of program headers:           56 (bytes)
  Number of program headers:         13
  Size of section headers:           64 (bytes)
  Number of section headers:         30
```

| Offset | Size | Field | Meaning |
|---|---|---|---|
| 0x00 | 4 | magic | `7f 45 4c 46`, the bytes `.ELF` |
| 0x04 | 1 | class | 1 for 32-bit, 2 for 64-bit |
| 0x05 | 1 | data | 1 for little-endian, 2 for big-endian |
| 0x10 | 2 | type | 2 executable, 3 shared object (also position-independent executables) |
| 0x12 | 2 | machine | 0x3e for x86-64, 3 for x86 |
| 0x18 | 8 | entry | virtual address of the first instruction |
| 0x20 | 8 | program header offset | file offset of the segment table |
| 0x28 | 8 | section header offset | file offset of the section table |
| 0x36 | 2 | program header entry size | 56 in 64-bit files |
| 0x38 | 2 | program header count | number of segments |
| 0x3a | 2 | section header entry size | 64 in 64-bit files |
| 0x3c | 2 | section header count | number of sections |

The offsets are for the 64-bit format. The 32-bit header is smaller because addresses and offsets are 4 bytes wide.

#### Program headers: the loading view

```bash
readelf -l -W build/12_sample
```

```text
Program Headers:
  Type           Offset   VirtAddr           PhysAddr           FileSiz  MemSiz   Flg Align
  PHDR           0x000040 0x0000000000400040 0x0000000000400040 0x0002d8 0x0002d8 R   0x8
  INTERP         0x000318 0x0000000000400318 0x0000000000400318 0x00001c 0x00001c R   0x1
      [Requesting program interpreter: /lib64/ld-linux-x86-64.so.2]
  LOAD           0x000000 0x0000000000400000 0x0000000000400000 0x000548 0x000548 R   0x1000
  LOAD           0x001000 0x0000000000401000 0x0000000000401000 0x000179 0x000179 R E 0x1000
  LOAD           0x002000 0x0000000000402000 0x0000000000402000 0x0000c4 0x0000c4 R   0x1000
  LOAD           0x002df8 0x0000000000403df8 0x0000000000403df8 0x00022c 0x000230 RW  0x1000
  DYNAMIC        0x002e08 0x0000000000403e08 0x0000000000403e08 0x0001d0 0x0001d0 RW  0x8
  GNU_STACK      0x000000 0x0000000000000000 0x0000000000000000 0x000000 0x000000 RW  0x10
  GNU_RELRO      0x002df8 0x0000000000403df8 0x0000000000403df8 0x000208 0x000208 R   0x1
```

The listing is shortened to the entries that matter here. Each `LOAD` entry tells the loader to map `FileSiz` bytes from file offset `Offset` at address `VirtAddr`, with the permissions in `Flg`, and to extend the mapping with zeroes up to `MemSiz`. There are four: a read-only part with the headers, a part that can be read and executed (the code), a read-only part (constants), and a read-write part (data). The last one has `MemSiz` larger than `FileSiz`, by 4 bytes, which is the `.bss`: memory that is zero at start and needs no space in the file.

| Type | Meaning |
|---|---|
| `LOAD` | A range to map into memory |
| `INTERP` | The path of the dynamic loader to start first |
| `DYNAMIC` | The table the dynamic loader reads: libraries needed, symbol tables, relocations |
| `GNU_STACK` | Permissions of the stack; here read-write without execute |
| `GNU_RELRO` | A range to make read-only after relocations have been applied |

#### Sections: the linking view

```bash
readelf -S -W build/12_sample
```

```text
  [Nr] Name              Type            Address          Off    Size   ES Flg Lk Inf Al
  [12] .init             PROGBITS        0000000000401000 001000 00001b 00  AX  0   0  4
  [13] .plt              PROGBITS        0000000000401020 001020 000030 10  AX  0   0 16
  [14] .text             PROGBITS        0000000000401050 001050 00011c 00  AX  0   0 16
  [15] .fini             PROGBITS        000000000040116c 00116c 00000d 00  AX  0   0  4
  [16] .rodata           PROGBITS        0000000000402000 002000 00000e 00   A  0   0  4
  [21] .dynamic          DYNAMIC         0000000000403e08 002e08 0001d0 10  WA  7   0  8
  [22] .got              PROGBITS        0000000000403fd8 002fd8 000010 08  WA  0   0  8
  [23] .got.plt          PROGBITS        0000000000403fe8 002fe8 000028 08  WA  0   0  8
  [24] .data             PROGBITS        0000000000404010 003010 000014 00  WA  0   0  8
  [25] .bss              NOBITS          0000000000404024 003024 000004 00  WA  0   0  1
```

This listing is also shortened. The sections fit inside the segments: `.init`, `.plt`, `.text` and `.fini` are in the executable `LOAD`; `.rodata` is in the read-only one; the writable sections form the last. Sections matter to tools; the loader ignores them, which is why a file can run with the section table removed or damaged.

| Section | Contents |
|---|---|
| `.text` | The program's code |
| `.rodata` | Constants and string literals |
| `.data` | Initialized writable data |
| `.bss` | Zero-initialized data, no file space |
| `.plt`, `.got`, `.got.plt` | Glue for calling shared-library functions |
| `.dynsym`, `.dynstr` | Symbols imported and exported through the dynamic loader |
| `.symtab`, `.strtab` | The full symbol table, present unless the file is stripped |
| `.dynamic` | The dynamic table |

#### Imports: PLT, GOT and relocations

The program calls `puts` and `printf` from the C library, whose addresses are unknown until load time. The linker therefore emits a **procedure linkage table** (PLT) stub for each, and a **global offset table** (GOT) slot that the stub jumps through.

```bash
readelf -r build/12_sample
```

```text
Relocation section '.rela.dyn' at offset 0x4e8 contains 2 entries:
  Offset          Info           Type           Sym. Value    Sym. Name + Addend
000000403fd8  000100000006 R_X86_64_GLOB_DAT 0000000000000000 __libc_start_main@GLIBC_2.34 + 0
000000403fe0  000300000006 R_X86_64_GLOB_DAT 0000000000000000 __gmon_start__ + 0

Relocation section '.rela.plt' at offset 0x518 contains 2 entries:
  Offset          Info           Type           Sym. Value    Sym. Name + Addend
000000404000  000200000007 R_X86_64_JUMP_SLO 0000000000000000 puts@GLIBC_2.2.5 + 0
000000404008  000400000007 R_X86_64_JUMP_SLO 0000000000000000 __printf_chk@GLIBC_2.3.4 + 0
```

A **relocation** is an instruction to the loader: "write the address of this symbol at this location." The `JUMP_SLOT` entries say that the slots at 0x404000 and 0x404008 will receive the addresses of `puts` and `__printf_chk`. In `main` the code calls the stub, not the library:

```bash
objdump -d -M intel --no-show-raw-insn -j .plt build/12_sample
```

```text
0000000000401020 <puts@plt-0x10>:
  401020:    push   QWORD PTR [rip+0x2fca]        # 403ff0 <_GLOBAL_OFFSET_TABLE_+0x8>
  401026:    jmp    QWORD PTR [rip+0x2fcc]        # 403ff8 <_GLOBAL_OFFSET_TABLE_+0x10>
  40102c:    nop    DWORD PTR [rax+0x0]

0000000000401030 <puts@plt>:
  401030:    jmp    QWORD PTR [rip+0x2fca]        # 404000 <puts@GLIBC_2.2.5>
  401036:    push   0x0
  40103b:    jmp    401020 <_init+0x20>
```

Before the first call, the slot at 0x404000 contains 0x401036, the address of the `push` just after the `jmp`. So the first call to `puts` falls through to `push 0` (the index of the relocation) and jumps to the shared stub at 0x401020, which enters the dynamic loader's resolver. The resolver looks up `puts`, writes its address into the slot, and jumps to it. Every later call jumps straight to `puts`. This is **lazy binding**. Many systems build with immediate binding instead, which resolves everything at start-up, so you may see a different pattern on other files; the stubs and the table are the same.

For a reverser the consequence is simple: a call to `foo@plt` is a call to the library function `foo`, and the `.rela.plt` and `.dynsym` tables list every library function the program can call.

### PE

A PE file (Portable Executable) starts with a **DOS header** that begins with the letters `MZ`. It is a remnant of the 16-bit DOS format, and the only field that still matters is at offset 0x3c: `e_lfanew`, the file offset of the **NT headers**. Between the DOS header and the NT headers there is usually a small DOS stub that prints "This program cannot be run in DOS mode".

The NT headers begin with the signature `PE\0\0`, followed by the **file header**, which names the machine and the number of sections, and the **optional header**, which is mandatory for executables despite its name. A **section table** follows.

The program `examples/12_pe_dump.py` parses these structures with the standard library only. A Windows program built with MinGW on the lab image:

```bash
python3 12_pe_dump.py build/12_win.exe
```

```text
e_lfanew (offset of the NT headers): 0x80
machine: 0x8664  sections: 10  optional header size: 240
optional header magic: 0x20b (0x20b means PE32+)
entry point RVA: 0x1410  image base: 0x140000000
section alignment: 0x1000  file alignment: 0x200
import directory: RVA 0x8000, size 0x52c

name      virtual size  RVA       raw size  raw offset  flags
.text     0x00001768    0x001000  0x001800  0x000400    0x60000060
.data     0x00000080    0x003000  0x000200  0x001c00    0xc0000040
.rdata    0x000008a0    0x004000  0x000a00  0x001e00    0x40000040
.pdata    0x00000210    0x005000  0x000400  0x002800    0x40000040
.xdata    0x000001dc    0x006000  0x000200  0x002c00    0x40000040
.bss      0x00000180    0x007000  0x000000  0x000000    0xc0000080
.idata    0x0000052c    0x008000  0x000600  0x002e00    0xc0000040
.CRT      0x00000060    0x009000  0x000200  0x003400    0xc0000040
.tls      0x00000010    0x00a000  0x000200  0x003600    0xc0000040
.reloc    0x00000078    0x00b000  0x000200  0x003800    0x42000040

entry point RVA 0x1410 is at file offset 0x810

imported libraries and functions:
  KERNEL32.dll: DeleteCriticalSection, EnterCriticalSection, GetLastError, InitializeCriticalSection, LeaveCriticalSection, SetUnhandledExceptionFilter ...
  msvcrt.dll: __C_specific_handler, __getmainargs, __initenv, __iob_func, __set_app_type, __setusermatherr ...
```

Key fields:

| Field | Meaning |
|---|---|
| Machine | 0x8664 is x86-64; 0x14c is x86 |
| Optional header magic | 0x10b is PE32 (32-bit); 0x20b is PE32+ (64-bit) |
| Image base | The preferred address the file is loaded at (here 0x140000000) |
| Entry point | An RVA: relative to the image base |
| Section alignment | Alignment in memory (0x1000, one page) |
| File alignment | Alignment in the file (0x200) |
| Data directories | A table of 16 `(RVA, size)` pairs pointing to imports, exports, relocations, resources and more |

#### RVAs and file offsets

PE addresses inside the file are **relative virtual addresses** (RVAs): offsets from the image base once loaded. Each section occupies one range in memory and another in the file. To convert an RVA to a file offset, find the section that contains it and apply:

```text
file offset = RVA - section RVA + section raw offset
```

Worked example from the table above: the entry point RVA 0x1410 lies in `.text` (RVA 0x1000, raw offset 0x400), so the offset is 0x1410 - 0x1000 + 0x400 = 0x810. That matches the program's output. The section's virtual size (0x1768) can exceed its raw size, and `.bss` has a virtual size but no raw data, just as in ELF.

#### Imports

The import directory (the `.idata` section here) is an array of descriptors, one per library, ending with an all-zero descriptor. Each descriptor points to the library name and to two parallel arrays: the **import name table** (INT), which lists the functions wanted, and the **import address table** (IAT), which the loader fills with their addresses. Each INT entry points to a `hint` and a name. Calls in the program go through the IAT, as `call qword ptr [rip + offset]`. This is the PE counterpart of the PLT and GOT, with the difference that the loader fills the whole IAT before the program starts.

#### Other tables

The **export directory** lists the functions a DLL offers. The **relocation table** (`.reloc`) lists the places to adjust when the image is loaded at an address other than the preferred base. Resources, thread-local storage, exception data (`.pdata` and `.xdata` on 64-bit) and the debug directory have their own entries in the data directories.

### Comparison

| | ELF | PE |
|---|---|---|
| Magic | `7f 45 4c 46` | `MZ`, then `PE\0\0` at `e_lfanew` |
| Entry point | Virtual address | RVA from the image base |
| Loading view | Program headers (segments) | Section table, plus the optional header |
| Imports | `.dynsym`, `.rela.plt`, PLT and GOT | Import directory, INT and IAT |
| Exports | `.dynsym` entries marked defined | Export directory |
| Relocations | `.rela.dyn`, `.rela.plt` | `.reloc` |
| Loader | `ld.so`, named in `INTERP` | The Windows loader in the kernel and `ntdll` |

### Writing an executable by hand

Because the loader needs only a few fields, a valid Linux program can be as small as an ELF header, one program header and a few instructions. The file `examples/12_tiny_elf.asm` assembles to a flat binary with `nasm -f bin`, which writes exactly the bytes you declare and nothing else.

```bash
make
build/12_tiny_elf; echo $?
wc -c < build/12_tiny_elf
readelf -h -l -W build/12_tiny_elf | grep -E "Class|Type|Machine|Entry|Number of|LOAD"
```

```text
42
132
  Class:                             ELF64
  Type:                              EXEC (Executable file)
  Machine:                           Advanced Micro Devices X86-64
  Entry point address:               0x400078
  Number of program headers:         1
  Number of section headers:         0
  Type           Offset   VirtAddr           PhysAddr           FileSiz  MemSiz   Flg Align
  LOAD           0x000000 0x0000000000400000 0x0000000000400000 0x000084 0x000084 R E 0x1000
```

The whole program is 132 bytes: 64 for the ELF header, 56 for the one program header, and 12 for the instructions. The single `LOAD` entry maps the entire file from offset 0 to the address 0x400000 with read and execute permission, so the header bytes and the code share one mapping, and the entry point 0x400078 is 120 bytes in, right after the two headers. There are no sections and no symbols; the program runs because the loader needs neither.

Read the file in full: every `db`, `dw`, `dd` and `dq` is a field from the tables above, with a comment. Note that `p_offset` is zero and `p_vaddr` equals the load address; the loader requires the offset and the virtual address to agree modulo the page size.

A PE file can be written the same way, with a DOS header, NT headers, a section table and one section. It is longer because the PE format has more mandatory fields, and Exercise 11 asks you to read one closely rather than write one.

## Try it

1. Run `readelf -h build/12_tiny_elf` and match every line to a `dw`, `dd` or `dq` in `12_tiny_elf.asm`.
2. Change the exit status in the assembly to 7, rebuild, and confirm with `echo $?`.
3. Using `readelf -S` on `12_sample`, convert the address of `counter` (0x404020, from `objdump -d`) to a file offset with the section's address and offset. Then check it with `xxd -s OFFSET -l 4 build/12_sample`; the value is 5.
4. Run `python3 12_pe_dump.py` on the PE and convert the RVA 0x8000 of the import directory to a file offset by hand. Compare with the section table.
5. Find the string `hello` in `12_sample` with `strings -t x` and work out the virtual address from the `.rodata` section.

## Common pitfalls

- **Mixing file offsets and virtual addresses.** Tools print both. Check the column heading before you use a number.
- **Forgetting that RVAs are relative to the image base.** In a PE, add the base to get the address the debugger shows.
- **Trusting the section table.** The loader ignores it. A file with a missing or doctored section table can still run, and a section name says nothing about what the bytes are.
- **Reading headers with the wrong word size.** A 32-bit ELF or PE uses different field widths. Check the class or magic first.
- **Assuming calls to `foo@plt` run `foo` the first time.** The first call goes through the resolver when lazy binding is on.

## Summary

- A loader maps a file using one view (ELF segments, PE sections with the optional header), while tools use another (sections).
- ELF has an ELF header, program headers and section headers; PE has a DOS header, NT headers and a section table.
- Imports reach library functions through PLT and GOT in ELF, and through the import directory and IAT in PE.
- Converting an address to a file offset is subtracting the section's address and adding the section's file offset.
- A valid executable can be very small, because the loader needs only the loading fields.

## Next steps

- Take the [quiz](quiz.md).
- Continue with module 13, recognizing code structures.
- Cheatsheets: [ELF](../../cheatsheets/elf.md), [PE](../../cheatsheets/pe.md).
- Exercises that use this module: [Exercise 10](../../exercises/10-header-surgeon/README.md), [Exercise 11](../../exercises/11-portable-puzzle/README.md).
