[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# Exercise 01. Signal flare

Level 1. Time: about 45 minutes. Architecture: x86-64 Linux. Modules: [00. Lab setup](../../modules/00-setup/README.md), [03. Your first program](../../modules/03-first-program/README.md)

## Scenario

A small beacon program called `signal_flare` reports its launch and exits. Its source code has been lost, and the maintainers need to know exactly what it does so they can rebuild it. You have the binary and nothing else.

## Objectives

- Read a program's system calls with `strace` and `objdump`.
- Work out the number, arguments and result of each call.
- Rebuild the program in assembly using raw system calls and no C library.

## Part 1: Reverse

The binary is `bin/signal_flare`. Run these commands from the exercise folder inside the lab:

```bash
file bin/signal_flare
strace ./bin/signal_flare
objdump -d -M intel bin/signal_flare
```

`strace` lists every system call the program makes. `objdump -d -M intel` shows the instructions in Intel syntax; the code you need starts at the label `_start`. The [Linux system call cheatsheet](../../cheatsheets/syscalls-linux.md) lists what each call number means and which register carries each argument.

Answer these questions:

1. Which system call numbers does the program use, in order?
2. For each `write`, which file descriptor does it use and how many bytes does it send?
3. What exit status does the program return?

Build the flag from your answers:

```text
ARL{flare_<bytes written to descriptor 1>_<bytes written to descriptor 2>_<exit status>}
```

For example, if the numbers were 10, 20 and 3, the flag would be `ARL{flare_10_20_3}`.

## Part 2: Assemble

Edit `starter/solution.asm` so that your program behaves exactly like `signal_flare`:

- The same bytes on standard output.
- The same bytes on standard error.
- The same exit status.

Build and run it from the `starter` folder:

```bash
make run
```

## Rules

- Use only the `write` and `exit` system calls. Do not link the C library.
- Write the text and the calls yourself. Your program must not start `signal_flare`.
- Define the text lengths with NASM (`$ - label`), not by counting by hand.

## Check your work

```bash
make check EX=01 ARGS='--flag ARL{your_answer_here}'
```

The checker reports Part 1 and Part 2 separately. Without `--flag`, it checks Part 2 only.

## Hints

Open [HINTS.md](HINTS.md) one level at a time.
