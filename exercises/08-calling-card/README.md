[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# Exercise 08. Calling card

Level 3. Time: about 3 hours. Architecture: x86 and x86-64 Linux. Modules: [08. The stack, procedures and calling conventions](../../modules/08-stack-and-calls/README.md)

## Scenario

A vendor shipped two small test programs, `callcard32` and `callcard64`, together with a header file that was lost. The header named the calling convention of every function, and the integration team now has to call these functions from other code. Without the header, the convention has to be read from the machine code. Then a replacement for three of the functions must be written, with the same conventions, so existing callers keep working.

## Objectives

- Tell cdecl, stdcall and fastcall apart in 32-bit code from the way arguments are read and the way the stack is cleaned up.
- Tell System V AMD64 from Microsoft x64 in 64-bit code from the argument registers, the shadow space and the caller's stack adjustment.
- Write functions whose stack effect matches their convention, so that a caller written for that convention stays balanced.

## Part 1: Reverse

The binaries are `bin/callcard32` (32-bit) and `bin/callcard64` (64-bit). Each runs a self-test of its functions and prints `callcard32: ok` or `callcard64: ok`. The functions are the global symbols `f1` to `f4` in `callcard32` and `g1` and `g2` in `callcard64`.

```bash
bin/callcard32
bin/callcard64
objdump -d -M intel bin/callcard32
objdump -d -M intel bin/callcard64
```

Each function uses one of five conventions. Assign a letter to each function:

| Letter | Convention |
|---|---|
| `c` | cdecl: arguments on the stack, the caller removes them |
| `s` | stdcall: arguments on the stack, the callee removes them |
| `f` | fastcall (32-bit): first two arguments in `ecx` and `edx`, the rest on the stack, the callee removes those |
| `v` | System V AMD64: first arguments in `rdi`, `rsi`, `rdx`, `rcx`, `r8`, `r9` |
| `m` | Microsoft x64: first arguments in `rcx`, `rdx`, `r8`, `r9`, with 32 bytes of shadow space |

Build the flag from the six letters, in the order `f1`, `f2`, `f3`, `f4`, `g1`, `g2`:

```text
ARL{calling_<f1><f2><f3><f4><g1><g2>}
```

For example, if the letters were `c`, `c`, `s`, `f`, `v` and `m`, the flag would be `ARL{calling_ccsfvm}`.

## Part 2: Assemble

Edit `starter/solution.asm` to implement three functions in 32-bit code. Each must compute the same result as the function of the same position in Part 1 and use the same calling convention as that function:

- `cc_f1` matches `f1`.
- `cc_f2` matches `f2`.
- `cc_f3` matches `f3`.

All three take three `int` arguments and return an `int`. The tests call them through prototypes that declare each convention:

```c
int __attribute__((stdcall))  cc_f1(int a, int b, int c);
int                           cc_f2(int a, int b, int c);   /* cdecl */
int __attribute__((fastcall)) cc_f3(int a, int b, int c);
```

Read the formula of each function from its code in `callcard32`, and notice which argument sits where.

## Rules

- The harness compares the stack pointer before and after every call. A function that removes too few or too many bytes fails, even when its result is correct.
- Preserve `ebx`, `esi`, `edi` and `ebp`.
- Do not use `call`.
- Results wrap around in 32 bits; the tests include very large and very small values.

## Check your work

```bash
make check EX=08 ARGS='--flag ARL{your_answer_here}'
```

The checker reports Part 1 and Part 2 separately. Without `--flag`, it checks Part 2 only.

## Hints

Open [HINTS.md](HINTS.md) one level at a time.
