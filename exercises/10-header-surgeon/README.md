[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# Exercise 10. Header surgeon

Level 3. Time: about 3 hours. Architecture: x86-64 Linux. Modules: [12. Executable formats: ELF and PE](../../modules/12-executable-formats/README.md)

## Scenario

A supplier delivered an ELF program, `surgeon`, whose build settings nobody wrote down. The deployment team needs its image base, its entry point and a few size facts before they can place it in a restricted launcher. They also need a minimal test executable that follows the same image base but has no libraries and no compiler behind it, so that the launcher can be checked without any dependencies. The test executable has to be written by hand, header by header.

## Objectives

- Read an ELF file's headers, program headers, sections, dynamic entries and relocations with command-line tools.
- Convert between virtual addresses and file offsets.
- Write a valid executable with `nasm -f bin`, including the congruence rule between file offsets and virtual addresses.

## Part 1: Reverse

The binary is `bin/surgeon`. It runs and prints one line.

```bash
bin/surgeon
readelf -h -l -S -d bin/surgeon
```

Find these six values:

1. `B`: the image base, the lowest virtual address of any `PT_LOAD` segment. Write it in lowercase hexadecimal without `0x`.
2. `E`: the file offset of the entry point, in lowercase hexadecimal without `0x`.
3. `P`: the number of program headers, in decimal.
4. `M`: how many bytes of the read-write `PT_LOAD` segment exist only in memory, that is `MemSiz` minus `FileSiz`, in decimal.
5. `V`: the size in bytes, in decimal, of the one section whose name is not a standard compiler or linker section name.
6. `L`: the name of the shared library other than the C library, without the `lib` prefix and the `.so` suffix and version (for `libz.so.1` write `z`).

```text
ARL{header_<B>_<E>_<P>_<M>_<V>_<L>}
```

For example, the values 400000, 1050, 13, 8, 21 and z would give `ARL{header_400000_1050_13_8_21_z}`.

## Part 2: Assemble

Edit `starter/solution.asm` so that `nasm -f bin` writes a complete executable file. Nothing is added to it: you write every header byte yourself. The program prints `header surgery ok` followed by a newline on standard output and exits with status 0.

The file must satisfy all of these:

- It is a 64-bit little-endian ELF executable for x86-64.
- The lowest `PT_LOAD` virtual address is the image base `B` of `bin/surgeon`.
- It has one `PT_LOAD` segment with flags read + execute that holds the headers and the code, and a different `PT_LOAD` segment with flags read + write that holds the message. No segment may be both writable and executable.
- For every `PT_LOAD` segment, `p_offset` and `p_vaddr` have the same remainder modulo `p_align`.
- The entry point lies inside the read + execute segment.
- A `PT_GNU_STACK` program header with flags read + write marks the stack as not executable.
- The message text is stored in the file range of the read-write segment, and the code reads it from there.
- The file is at most 1024 bytes long.

The checker reads `B` from `bin/surgeon` itself, then inspects the headers of your file and runs it.

## Rules

- Use `nasm -f bin` only: the checker assembles your file that way and runs the result directly.
- Do not use a linker, a C library or any dynamic loading.
- Section headers are not required.

## Check your work

```bash
make check EX=10 ARGS='--flag ARL{your_answer_here}'
```

The checker reports Part 1 and Part 2 separately. Without `--flag`, it checks Part 2 only. For Part 2 it lists every requirement that your file breaks.

## Hints

Open [HINTS.md](HINTS.md) one level at a time.
