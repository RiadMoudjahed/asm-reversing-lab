[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# Exercise 09. Stack archaeology

Level 3. Time: about 3 hours. Architecture: x86-64 Linux. Modules: [08. The stack, procedures and calling conventions](../../modules/08-stack-and-calls/README.md), [13. Recognizing code structures](../../modules/13-code-structures/README.md)

## Scenario

A build of an internal tool, `stackdig`, crashes with a stack overflow on deep inputs, and the source of its recursive function `chain` is gone. Only the optimized binary remains. To decide whether the function can be rewritten more cheaply, someone has to measure exactly what one level of recursion puts on the stack. Then a hand-written replacement has to behave identically while using no more stack per level.

## Objectives

- Read the prologue and epilogue of an optimized function that has no frame pointer.
- Reconstruct a stack frame: saved registers, local data, unused space and the return address.
- Write a recursive function that keeps the stack 16-byte aligned at every call and preserves the callee-saved registers.

## Part 1: Reverse

The binary is `bin/stackdig`. It runs one computation and prints `stackdig: ok`. The function to study is the global symbol `chain`. It was compiled with optimization and without a frame pointer, and it calls another function, `note`, once per level with a pointer to a three-entry table of 8-byte values that it keeps on its stack.

```bash
bin/stackdig
objdump -d -M intel bin/stackdig
```

Find these five numbers, all in decimal:

1. `P`: how many callee-saved registers `chain` pushes.
2. `S`: how many bytes the prologue subtracts from `rsp`.
3. `L`: how many bytes of stack one level of recursion occupies, counting the return address, the saved registers and the subtracted space.
4. `O`: the distance in bytes from the first table entry, as seen by `note`, up to the saved return address of that same level.
5. `U`: how many bytes of the subtracted space are not used by the table.

```text
ARL{stack_<P>_<S>_<L>_<O>_<U>}
```

For example, the values 1, 8, 24, 16 and 0 would give `ARL{stack_1_8_24_16_0}`.

## Part 2: Assemble

Edit `starter/solution.asm` to write `chain_asm`, a hand-written version of `chain` in the System V calling convention:

```c
long chain_asm(long depth, long seed);

/* Provided by the tests. Clobbers every caller-saved register. */
void note(long depth, const long *entries);
```

`chain_asm` must do what `chain` does: build the same table, call `note(depth, table)` once per level, recurse, and return the same value. Read the formulas from the code of `chain`. The table entries and the arithmetic are part of what Part 1 lets you see.

## Rules

- `rsp` must be a multiple of 16 at every `call note`. The test checks it inside `note`.
- `rbx`, `rbp` and `r12` to `r15` must have their original values when `chain_asm` returns.
- `note` destroys `rax`, `rcx`, `rdx`, `rsi`, `rdi` and `r8` to `r11`. Keep anything you need after the call somewhere else.
- One level of your function may use no more stack than one level of `chain` does. The test compares the stack pointer at consecutive calls to `note` against the number `L` from Part 1.
- Recursion is the only way to descend; the tests go as deep as 2000 levels.
- Results wrap around in 64 bits.

## Check your work

```bash
make check EX=09 ARGS='--flag ARL{your_answer_here}'
```

The checker reports Part 1 and Part 2 separately. Without `--flag`, it checks Part 2 only. For Part 2 each test prints the result followed by `ok` or a list of what went wrong: `result`, `align`, `table`, `calls`, `regs` or `stack`.

## Hints

Open [HINTS.md](HINTS.md) one level at a time.
