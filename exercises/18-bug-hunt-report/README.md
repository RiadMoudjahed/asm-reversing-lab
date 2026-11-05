[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# Exercise 18. Bug hunt report

Level 4. Time: about 4 hours. Architecture: x86-64 Linux. Modules: [18. Auditing binaries for bugs](../../modules/18-auditing-binaries/README.md)

## Scenario

A small program, `recparse`, reads a file of records from a monitoring device and prints a summary. It has to be reviewed before it is used on files that come from outside. The source is not available. You audit the binary, find its memory-safety defects, and write a report for its maintainers. Then you write the corrected parsing routine in assembly, so that the maintainers can replace the faulty one.

## Objectives

- Audit a stripped binary: find the sinks, trace the sizes and indexes back to the input, and read every check.
- Identify defects of several classes and state the cause of each precisely.
- Write a report that the owner of the code can act on, without exploiting anything.
- Write a parser in assembly that refuses malformed input without ever touching memory it should not.

## Part 1: Reverse

The binary is `bin/recparse`. It reads the file named on its command line and prints a summary of the records in it.

```bash
bin/recparse sample.rec
objdump -d -M intel bin/recparse
```

The program is deliberately flawed. Run it only on files that you made yourself, and use the debugger to stop it before a suspect operation instead of letting it run past the end of a buffer. Audit the code the way module 18 describes. The program has six memory-safety defects: one in the way the program walks from one record to the next, and one in each of the five record types. Find them.

Record the following five values:

1. `N`: the largest length of a name that the record type for names accepts, in decimal.
2. `V`: the smallest length byte, in decimal, for which the check in the record type for lists of numbers lets an impossible size through.
3. `T`: the largest tag value that the record type for tags accepts, in decimal.
4. `B`: the smallest item count, in decimal, for which the size computed for the record type for data blocks no longer matches the number of bytes later copied.
5. `U`: the type number, in decimal, of the record type that releases memory which is used again afterwards.

```text
ARL{bughunt_<N>_<V>_<T>_<B>_<U>}
```

For example, the values 8, 200, 4, 32 and 3 would give `ARL{bughunt_8_200_4_32_3}`.

Then write a report. Create a file `REPORT.md` and give each of the six defects an entry in the format of module 18: title, class, location, severity, cause, evidence, impact, fix and test. The checker does not read the report. The write-up in `solutions/18/` contains a model report for you to compare with afterwards.

## Part 2: Assemble

Edit `starter/solution.asm` to implement one function in the System V calling convention:

```c-fragment
struct out {
    char     name[16];
    int64_t  sum;
    uint32_t tags[8];
    uint32_t blob_sum;
    uint32_t pad;
};

long parse_records(const unsigned char *buf, size_t n, struct out *o);
```

The buffer is the `n` bytes at `buf`. The structure `o` is zeroed before the call. A record is a type byte, a length byte and that many payload bytes. The function processes records one after another until it finds the record of type 0, which ends the list. It returns the number of records processed before that one, or a negative error code. The rules, in the order they are applied to each record:

- If fewer than 2 bytes remain, or fewer than `length` bytes remain after the two header bytes, the record is truncated: return -1. This also applies to the record of type 0, and a buffer that ends before a record of type 0 is truncated.
- Type 0 ends the list. Bytes after it are ignored, and its payload, if any, is skipped.
- Type 1 sets the name. It needs a length of at most 15, otherwise return -2. The name field becomes exactly the payload bytes followed by zero bytes up to 16 bytes in all.
- Type 2 adds numbers to `sum`. The length must be a multiple of 4 and at most 32, otherwise return -3. Every group of 4 payload bytes is a signed 32-bit number, low byte first, and is added to `sum`.
- Type 3 counts tags. Every payload byte must be at most 7, otherwise return -4. For every byte, `tags[byte]` is incremented.
- Type 4 sets a data block. It needs a length of at least 1 that equals 1 plus 4 times the value of the first payload byte, otherwise return -5. `blob_sum` becomes the sum of the payload bytes after the first one, so a later block replaces an earlier one.
- Type 5 discards the data block. It needs a length of 0, otherwise return -5. `blob_sum` becomes 0.
- Any other type: return -6.

When the return value is negative, the content of `o` is not examined, but nothing may be written outside `o` and the `pad` field must stay zero.

The tests build many buffers, from valid ones to truncated, damaged and random ones, and compare the return value and every field with those of a reference.

## Rules

- Never read a byte outside the `n` bytes of the buffer, whatever the records say. The tests place the buffer just before an unmapped page, and a read beyond its end ends the test run with a signal.
- Never write outside the 64 bytes of `o`, and leave `pad` zero.
- Preserve the callee-saved registers `rbx`, `rbp` and `r12` to `r15`.
- Keep the stack aligned at every `call`.

## Check your work

```bash
make check EX=18 ARGS='--flag ARL{your_answer_here}'
```

The checker reports Part 1 and Part 2 separately. Without `--flag`, it checks Part 2 only. Each result in a Part 2 test ends with `ok` or a list of what went wrong: `overrun` or `regs`.

## Hints

Open [HINTS.md](HINTS.md) one level at a time.
