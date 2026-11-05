[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# Exercise 07. String sweeper

Level 2. Time: about 2 hours. Architecture: x86-64 Linux. Modules: [04. Moving data and addressing modes](../../modules/04-moving-data/README.md), [06. Conditions, branches and loops](../../modules/06-conditions-and-loops/README.md), [07. Strings and string instructions](../../modules/07-strings/README.md)

## Scenario

A diagnostics tool called `sweeper` carries a tiny library of its own: six memory and string routines, with no C library behind them. The routines have meaningless names, and the maintainers need to know which is which before they can document or replace them. Then a new component needs four of the same routines written from scratch.

## Objectives

- Recognize memory and string routines by the string instructions and registers they use.
- Tell a forward copy from an overlap-safe copy, and a length from a search.
- Write `strlen`, `memchr`, `memcpy` and `memmove` with the string instructions, keeping the direction flag correct.

## Part 1: Reverse

The binary is `bin/sweeper`. It runs a self-test of six routines and prints `sweeper: all routines behave`. The routines are the global symbols `fn1` to `fn6`.

```bash
bin/sweeper
objdump -d -M intel bin/sweeper
```

Each routine implements one of these C library functions: `strlen`, `memchr`, `memcpy`, `memmove`, `memset` and `memcmp`. Work out which `fn` number is which function.

Build the flag from the six numbers, in this order of functions:

```text
ARL{sweeper_<strlen>_<memchr>_<memcpy>_<memmove>_<memset>_<memcmp>}
```

For example, if `strlen` were `fn3`, `memchr` were `fn1`, `memcpy` were `fn2`, `memmove` were `fn4`, `memset` were `fn5` and `memcmp` were `fn6`, the flag would be `ARL{sweeper_3_1_2_4_5_6}`.

## Part 2: Assemble

Edit `starter/solution.asm` to implement four of the routines in the System V calling convention: arguments in `rdi`, `rsi` and `rdx`, result in `rax`.

```c
#include <stddef.h>

size_t my_strlen(const char *s);                      // bytes before the first zero byte
void  *my_memchr(const void *s, int c, size_t n);     // first byte equal to (unsigned char) c
                                                      // among the n bytes, or NULL
void  *my_memcpy(void *d, const void *s, size_t n);   // copy n bytes, no overlap; returns d
void  *my_memmove(void *d, const void *s, size_t n);  // copy n bytes, overlap allowed; returns d
```

## Rules

- Use the string instructions: the checker requires at least one `rep movs` and one `repnz scas` in your code.
- Do not use `call`.
- The direction flag is clear when your function is entered. It must be clear when you return.
- Read and write only the bytes that belong to the operation. The tests place each block right before an inaccessible page, so touching a byte beyond it crashes the test.
- Handle `n` equal to 0 correctly.

## Check your work

```bash
make check EX=07 ARGS='--flag ARL{your_answer_here}'
```

The checker reports Part 1 and Part 2 separately. Without `--flag`, it checks Part 2 only.

## Hints

Open [HINTS.md](HINTS.md) one level at a time.
