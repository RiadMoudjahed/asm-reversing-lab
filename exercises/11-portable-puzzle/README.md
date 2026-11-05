[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# Exercise 11. Portable puzzle

Level 3. Time: about 4 hours. Architecture: x86-64 Windows (run under Wine). Modules: [08. The stack, procedures and calling conventions](../../modules/08-stack-and-calls/README.md), [10. Operating system fundamentals for reversers](../../modules/10-os-for-reversers/README.md), [12. Executable formats: ELF and PE](../../modules/12-executable-formats/README.md)

## Scenario

A Windows console tool, `puzzle.exe`, arrives without documentation. A packaging team needs a few facts from its headers, and they also want a small helper library, written in assembly so that it has no runtime dependencies, that finds where a given address of a loaded PE image lives in the file on disk. You first read the headers yourself. Then you write the helper.

## Objectives

- Parse the headers, section table and import directory of a PE32+ file.
- Convert a relative virtual address (RVA) to a file offset.
- Write Windows x64 functions that respect the Microsoft calling convention: argument registers, shadow space, stack alignment and non-volatile registers.

## Part 1: Reverse

The binary is `bin/puzzle.exe`. It is a console program that prints one line; you can run it under Wine.

```bash
wine bin/puzzle.exe
x86_64-w64-mingw32-objdump -p bin/puzzle.exe
python3 ../../modules/12-executable-formats/examples/12_pe_dump.py bin/puzzle.exe
```

Find these six values:

1. `S`: the number of sections, in decimal.
2. `I`: the image base, in lowercase hexadecimal without `0x`.
3. `R`: the RVA of the entry point, in lowercase hexadecimal without `0x`.
4. `O`: the file offset of the byte at RVA `0x12348`, in lowercase hexadecimal without `0x`.
5. `D`: the number of libraries in the import directory, in decimal.
6. `K`: the number of functions imported from `KERNEL32.dll`, in decimal.

```text
ARL{portable_<S>_<I>_<R>_<O>_<D>_<K>}
```

For example, the values 5, 140000000, 1410, 7a30, 2 and 9 would give `ARL{portable_5_140000000_1410_7a30_2_9}`.

## Part 2: Assemble

Edit `starter/solution.asm` to implement two functions in the Microsoft x64 calling convention.

```c
#include <stdint.h>

uint32_t pe_rva_to_offset(const void *sections, uint32_t count, uint32_t rva);
uint32_t pe_entry_offset(const void *file);

/* Provided by the tests. */
void log_rva(uint32_t rva);
```

- `pe_rva_to_offset` receives a pointer to a section table of `count` entries, and an RVA. It returns the file offset of that RVA, or `0xFFFFFFFF` if no section contains the RVA in its raw data. A section contains the RVAs from its virtual address up to, but not including, its virtual address plus its size of raw data.
- `pe_entry_offset` receives a pointer to the start of a PE file in memory. If the file does not start with `MZ`, or the NT headers do not start with `PE\0\0`, it returns `0xFFFFFFFF`. Otherwise it reads the entry point RVA from the optional header and calls `log_rva` once with that RVA. It then returns the file offset of the entry point, computed with the section table that follows the optional header. If the entry point is in no section it returns `0xFFFFFFFF`.

You need the layout of the headers and of the section table; Part 1 is where you find it.

## Rules

- Follow the Microsoft x64 convention: arguments in `rcx`, `rdx`, `r8` and `r9`; 32 bytes of shadow space for every call; `rsp` a multiple of 16 at every `call`.
- Preserve `rbx`, `rbp`, `rdi`, `rsi` and `r12` to `r15`. Note that `rdi` and `rsi` are non-volatile on Windows.
- `log_rva` destroys every volatile register and writes into its shadow space. Keep what you need across the call in non-volatile registers or in your own stack frame.
- Do not call any other function.
- Results use 32 bits and wrap around as unsigned numbers.

## Check your work

```bash
make check EX=11 ARGS='--flag ARL{your_answer_here}'
```

The checker builds a test program with the MinGW cross compiler, links it with your object file and runs it under Wine. It reports Part 1 and Part 2 separately. Without `--flag`, it checks Part 2 only. Each Part 2 test prints a result followed by `ok` or a list of what went wrong: `align`, `regs` or `calls`.

## Hints

Open [HINTS.md](HINTS.md) one level at a time.
