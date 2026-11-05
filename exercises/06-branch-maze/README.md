[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# Exercise 06. Branch maze

Level 2. Time: about 1.5 hours. Architecture: x86-64 Linux. Modules: [05. Arithmetic and logic](../../modules/05-arithmetic-and-logic/README.md), [06. Conditions, branches and loops](../../modules/06-conditions-and-loops/README.md)

## Scenario

A gate controller accepts three whole numbers and either opens the path or answers "wall". Nobody kept the rules. The program never says which check you failed, so trial and error gets you nowhere: the rules have to be read from the code. Once you know them, a new component needs the same decision in assembly.

## Objectives

- Read a chain of comparisons and conditional jumps and recover the rule behind each.
- Tell signed jumps from unsigned jumps in disassembly.
- Reproduce the decision logic with `cmp`, `test` and conditional jumps only.

## Part 1: Reverse

The binary is `bin/maze`. It takes three integers on the command line.

```bash
bin/maze 1 2 3
objdump -d -M intel bin/maze
```

The program prints `wall` when any check fails and `path open` when all of them pass. Several triples are accepted. Find the accepted triple that is smallest when you compare first A, then B, then C (the one with the smallest A; among those, the smallest B; among those, the smallest C).

Build the flag from that triple. Write a negative number with the letter `m` instead of the minus sign, so -7 becomes `m7`:

```text
ARL{maze_<A>_<B>_<C>}
```

For example, the triple 5, -3, -4 would give `ARL{maze_5_m3_m4}`.

## Part 2: Assemble

Edit `starter/solution.asm` to implement one function in the System V calling convention: `a` arrives in `edi`, `b` in `esi`, `c` in `edx`, and the result goes back in `eax`.

```c
#include <stdint.h>

int32_t maze_ok(int32_t a, int32_t b, int32_t c);
```

`maze_ok` returns 1 exactly when `bin/maze` prints `path open` for the same three numbers, and 0 otherwise. The tests include boundary values and values near every limit, so each rule must be right.

## Rules

- Use only `cmp`, `test` and conditional jumps for the decisions. Do not use `cmov` or `setcc`.
- Treat `a`, `b` and `c` as 32-bit signed integers.

## Check your work

```bash
make check EX=06 ARGS='--flag ARL{your_answer_here}'
```

The checker reports Part 1 and Part 2 separately. Without `--flag`, it checks Part 2 only.

## Hints

Open [HINTS.md](HINTS.md) one level at a time.
