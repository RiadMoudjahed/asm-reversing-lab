[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# Exercise 12. Switchboard

Level 3. Time: about 3 hours. Architecture: x86-64 Linux. Modules: [13. Recognizing code structures](../../modules/13-code-structures/README.md)

## Scenario

A small command processor, `switchboard`, reads pairs of numbers, a command and a value, and prints a result. Its source is lost. A maintainer has to document exactly which commands exist, which of them share code, and what happens for every other command number. The maintainer then wants a replacement written in assembly that is organized the same way, with a jump table, so that it is as fast as the original for any command number.

## Objectives

- Recognize a `switch` compiled as a jump table and read its bounds check, its index adjustment and its table.
- Recover the code each command number reaches, including shared code, fall-through and the default case.
- Write a dispatcher on a jump table in NASM.

## Part 1: Reverse

The binary is `bin/switchboard`. It reads lines of the form `command value` from standard input and prints one result per line. The function that does the work is `dispatch`.

```bash
printf '7 10\n3 2\n' | bin/switchboard
objdump -d -M intel bin/switchboard
objdump -s -j .rodata bin/switchboard
```

Find these six values:

1. `N`: the number of entries in the jump table, in decimal.
2. `L`: the lowest command number that the jump table handles, in decimal.
3. `B`: the number of different code addresses in the table, not counting the address that handles commands without a case, in decimal.
4. `G`: the number of table entries that point to the code for commands without a case, in decimal.
5. `A`: the address that the table holds for command 7, in lowercase hexadecimal without `0x`.
6. `S`: the command number, other than 6, whose table entry holds the same address as the entry for command 6, in decimal.

```text
ARL{switch_<N>_<L>_<B>_<G>_<A>_<S>}
```

For example, the values 8, 1, 6, 3, 401234 and 4 would give `ARL{switch_8_1_6_3_401234_4}`.

## Part 2: Assemble

Edit `starter/solution.asm` to implement one function in the System V calling convention:

```c
#include <stdint.h>

int32_t run_cmd(int32_t cmd, uint32_t acc);   // cmd in edi, acc in esi, result in eax
```

`run_cmd` must return exactly what `dispatch` returns in `bin/switchboard`, for every 32-bit command number (including negative and very large ones) and every 32-bit value. The tests include all of these.

## Rules

- Dispatch through a jump table: an indirect `jmp` that reads its target from a table of addresses scaled by 8.
- Check the range of the command with an unsigned comparison before using the table, so that every command outside the table goes to the default case.
- Use at most two comparison instructions (`cmp`) in the whole routine.
- Do not use `call`.
- Several table entries may point to the same code, and a case may continue into the code of the next one.

## Check your work

```bash
make check EX=12 ARGS='--flag ARL{your_answer_here}'
```

The checker reports Part 1 and Part 2 separately. Without `--flag`, it checks Part 2 only. Besides the results, it checks the rules above on the instructions of your object file.

## Hints

Open [HINTS.md](HINTS.md) one level at a time.
