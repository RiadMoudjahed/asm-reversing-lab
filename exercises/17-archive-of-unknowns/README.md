[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# Exercise 17. Archive of unknowns

Level 4. Time: about 4 hours. Architecture: x86-64 Linux. Modules: [17. Deciphering file formats](../../modules/17-deciphering-file-formats/README.md)

## Scenario

A packing tool, `arkpack`, bundles several files into one archive. Nobody kept the description of the archive format, and the unpacking tool was lost. An embedded device now has to read these archives and has room only for a small routine written in assembly. You first recover the format by studying the program that writes it. Then you write the reader: list the names, extract the contents, and refuse archives that are damaged.

## Objectives

- Recover the layout of a file format (header, table, records, names, checksum) by reading the program that writes it and testing it on files of your own.
- Recognize the encoding applied to the data and the checksum, and reproduce both.
- Write a reader in assembly that never trusts the offsets and sizes stored in the file.

## Part 1: Reverse

The binary is `bin/arkpack`. Run it as `arkpack OUTPUT FILE...` and examine the output with a hex dump.

```bash
printf 'AAA' > a.txt
printf 'hello world\n' > hello.txt
bin/arkpack t.ark a.txt hello.txt
xxd t.ark
objdump -d -M intel bin/arkpack
```

Find these five values:

1. `E`: the size in bytes of one entry in the table that describes the stored files, in decimal.
2. `A`: the alignment in bytes of the start of each stored file's data (every data offset is a multiple of `A`), in decimal.
3. `R`: the number of bits by which the checksum is rotated to the left for each data byte, in decimal.
4. `K`: the value of the key byte that the tool stores in the header of an archive with 3 files, in decimal.
5. `N`: the largest number of files that the tool accepts, in decimal.

```text
ARL{ark_<E>_<A>_<R>_<K>_<N>}
```

For example, the values 32, 8, 3, 100 and 20 would give `ARL{ark_32_8_3_100_20}`.

## Part 2: Assemble

Edit `starter/solution.asm` to implement three functions in the System V calling convention. The archive is the `n` bytes at `a`.

```c-fragment
long ark_count(const unsigned char *a, size_t n);
long ark_name(const unsigned char *a, size_t n, long index, char *out, size_t cap);
long ark_extract(const unsigned char *a, size_t n, long index, unsigned char *out, size_t cap);
```

- `ark_count` returns the number of files in the archive, or -1 if the archive is invalid. It is invalid if it is shorter than the header, has the wrong magic value, or its header describes a table that does not lie entirely inside the archive (it must start after the header and end at or before the end of the archive) or a name area that starts beyond the end of the archive.
- `ark_name` copies the name of file number `index` (counted from 0) to `out`, with its terminating zero byte, and returns the length of the name without that byte. It returns -1 if the archive is invalid or `index` is not a valid file number, -2 if the name does not start inside the archive or has no terminating zero byte before the end of the archive, and -3 if `cap` is smaller than the length of the name plus one.
- `ark_extract` writes the contents of file number `index` to `out` and returns their size in bytes. It returns -1 if the archive is invalid or `index` is not a valid file number, -2 if the stored data does not lie inside the archive (it must start after the header and end at or before the end of the archive), -3 if `cap` is smaller than the size, and -4 if the checksum of the decoded bytes differs from the stored one. The checks are made in this order. On an error the content of `out` is not examined, but nothing may be written beyond `cap` bytes.

All offsets and sizes in the file are unsigned 32-bit numbers, and sums of them can exceed 32 bits. `index` is a signed 64-bit number.

The tests build archives with `arkpack` and also damage them in many ways. For each command they compare the return value and the bytes written with those of a reference.

## Rules

- Never read a byte outside the `n` bytes of the archive, whatever the header says. The tests place the archive just before an unmapped page, and a read beyond its end ends the test run with a signal.
- Never write beyond `cap` bytes of `out`.
- Preserve the callee-saved registers `rbx`, `rbp` and `r12` to `r15`.
- Keep the stack aligned at every `call`.

## Check your work

```bash
make check EX=17 ARGS='--flag ARL{your_answer_here}'
```

The checker reports Part 1 and Part 2 separately. Without `--flag`, it checks Part 2 only. Each result in a Part 2 test is followed by `ok` or a list of what went wrong: `overrun` or `regs`.

## Hints

Open [HINTS.md](HINTS.md) one level at a time.
