[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# Exercise 02. Two worlds

Level 1. Time: about 1 hour. Architecture: x86 and x86-64 Linux. Modules: [02. The CPU and its registers](../../modules/02-cpu-and-registers/README.md), [03. Your first program](../../modules/03-first-program/README.md)

## Scenario

A team keeps two builds of the same small utility: one for old 32-bit systems and one for 64-bit systems. Nobody remembers how the two builds differ at the lowest level. Your job is to document the difference from the binaries, and then use that knowledge to move a third, legacy 32-bit program to 64-bit.

## Objectives

- Compare how the 32-bit and 64-bit Linux interfaces enter the kernel.
- Identify system call numbers and argument registers in disassembly.
- Port a program from the 32-bit interface to the 64-bit interface.

## Part 1: Reverse

The binaries are `bin/two_worlds32` and `bin/two_worlds64`. They behave the same way. They differ in how they are written.

```bash
file bin/two_worlds32 bin/two_worlds64
objdump -d -M intel bin/two_worlds32
objdump -d -M intel bin/two_worlds64
strace ./bin/two_worlds32
strace ./bin/two_worlds64
```

Use the [Linux system call cheatsheet](../../cheatsheets/syscalls-linux.md) to look up numbers. Answer these questions:

1. Which three system call numbers does each binary load into `eax`, in order? (Numbers in the listing are hexadecimal.)
2. Which instruction enters the kernel in each binary?
3. In each binary, which register carries the first argument of `write`?

Build the flag from the call that is not `write`, and from `exit`:

```text
ARL{worlds_<sync in 32-bit>_<sync in 64-bit>_<exit in 32-bit>_<exit in 64-bit>}
```

## Part 2: Assemble

`starter/solution.asm` is a legacy 32-bit program. It prints a line, calls `sync`, and exits with status 5. Edit it so that it is a 64-bit program with the same behavior:

- The same output on standard output.
- The same three system calls, in the same order.
- The same exit status.

Build and run it from the `starter` folder:

```bash
make run
```

## Rules

- The result must assemble with `nasm -f elf64` and link with plain `ld`.
- Do not use `int 0x80`. Use the 64-bit `syscall` instruction.
- Keep all three calls, including `sync`. The checker watches the calls the program makes.

## Check your work

```bash
make check EX=02 ARGS='--flag ARL{your_answer_here}'
```

The checker reports Part 1 and Part 2 separately. Without `--flag`, it checks Part 2 only.

## Hints

Open [HINTS.md](HINTS.md) one level at a time.
