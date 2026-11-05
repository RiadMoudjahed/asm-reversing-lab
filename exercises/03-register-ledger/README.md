[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# Exercise 03. Register ledger

Level 1. Time: about 1 hour. Architecture: x86-64 Linux. Modules: [02. The CPU and its registers](../../modules/02-cpu-and-registers/README.md), [04. Moving data and addressing modes](../../modules/04-moving-data/README.md), [05. Arithmetic and logic](../../modules/05-arithmetic-and-logic/README.md)

## Scenario

An accounting tool runs a short calculation before it closes the books. The auditors do not trust the tool's own report and want to know what the processor itself says at four points in the calculation: the values in the registers and the state of the flags. Then they want a reusable routine that reports the flags of any addition or subtraction.

## Objectives

- Step through a program in a debugger and read registers and flags at chosen points.
- Connect each flag to the instruction that set it.
- Capture the flags of an operation in assembly, without comparison instructions.

## Part 1: Reverse

The binary is `bin/ledger`. It has four marked points, the symbols `mark1`, `mark2`, `mark3` and `mark4`. A breakpoint on a label stops before the instruction at that label runs, so you see the result of the instruction just above it.

```bash
gdb -q bin/ledger
```

```text
break mark1
break mark2
break mark3
break mark4
run
info registers rax rcx rdx eflags
continue
```

GDB prints the flags that are set by name, for example `[ CF PF SF IF ]`. A flag that is not listed is 0.

Answer these questions:

1. At `mark1`: what is `rax`, in decimal?
2. At `mark2`: which of the flags CF, ZF, SF and OF are set?
3. At `mark3`: which of CF, ZF, SF and OF are set, and what is `rcx`?
4. At `mark4`: which of CF, ZF, SF and OF are set?

Write each group of flags as four digits in the order CF, ZF, SF, OF, with 1 for set and 0 for clear. Build the flag from your answers:

```text
ARL{ledger_<rax at mark1>_<flags at mark2>_<flags at mark3>_<flags at mark4>}
```

For example, if `rax` were 5 and the three groups were `0000`, `0001` and `0010`, the flag would be `ARL{ledger_5_0000_0001_0010}`.

## Part 2: Assemble

Edit `starter/solution.asm` to implement two functions. They use the System V calling convention: arguments in `rdi`, `rsi` and `rdx`, result in `rax`.

```c
#include <stdint.h>

uint32_t ledger_add(int64_t a, int64_t b, int64_t *out);   // *out = a + b
uint32_t ledger_sub(int64_t a, int64_t b, int64_t *out);   // *out = a - b
```

Each function stores the result in `*out` and returns the flags of that operation as a bit mask:

| Bit | Flag |
|---|---|
| 0 | CF |
| 1 | ZF |
| 2 | SF |
| 3 | OF |

All other bits are 0. For `ledger_sub`, CF is the carry flag as the processor sets it for a subtraction.

## Rules

- Do not use `cmp` or `test`. The status must come from the flags of the addition or subtraction itself.
- Store the result in `*out` in every case.

## Check your work

```bash
make check EX=03 ARGS='--flag ARL{your_answer_here}'
```

The checker reports Part 1 and Part 2 separately. Without `--flag`, it checks Part 2 only.

## Hints

Open [HINTS.md](HINTS.md) one level at a time.
