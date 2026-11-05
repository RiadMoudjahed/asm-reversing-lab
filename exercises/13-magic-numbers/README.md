[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# Exercise 13. Magic numbers

Level 3. Time: about 3 hours. Architecture: x86-64 Linux. Modules: [05. Arithmetic and logic](../../modules/05-arithmetic-and-logic/README.md), [14. Compiled arithmetic](../../modules/14-compiled-arithmetic/README.md)

## Scenario

A scoring routine, `mix`, was compiled with optimization and its source was lost. It takes a 32-bit integer and returns a score, and it contains no division instructions at all: every division and remainder in it was replaced by a multiplication with a constant. A maintainer needs the exact formula, because the same score has to be computed on a platform that has no 64-bit multiply and no divide instruction. First you recover the formula. Then you write the routine again, without `div` and `idiv`.

## Objectives

- Recognize the multiply-and-shift form of division and remainder, signed and unsigned.
- Recover the divisor behind each magic constant.
- Rebuild a division by a constant from multiplication, shifts and a sign correction.

## Part 1: Reverse

The binary is `bin/magic`. It reads integers from standard input and prints the score of each. The function that computes the score is `mix`.

```bash
printf '100\n-100\n' | bin/magic
objdump -d -M intel bin/magic
```

The script `14_magic.py` in module 14 turns a magic constant and its shift into a divisor and checks it.

The score combines four operations on the input `x`: an unsigned quotient, an unsigned remainder, a signed quotient and a signed remainder, each by a different constant. The two unsigned operations work on `x` read as an unsigned number. The results are then combined with small factors.

Find these six values:

1. `A`: the divisor of the unsigned quotient.
2. `B`: the divisor of the unsigned remainder.
3. `C`: the divisor of the signed quotient.
4. `D`: the divisor of the signed remainder.
5. `K`: the factor that multiplies the unsigned quotient in the sum.
6. `M`: the factor that multiplies the unsigned remainder in the sum.

```text
ARL{magic_<A>_<B>_<C>_<D>_<K>_<M>}
```

For example, the values 5, 3, 25, 7, 2 and 6 would give `ARL{magic_5_3_25_7_2_6}`.

## Part 2: Assemble

Edit `starter/solution.asm` to implement one function in the System V calling convention:

```c
#include <stdint.h>

int32_t mix_asm(int32_t x);   // x in edi, result in eax
```

`mix_asm` must return exactly what `mix` returns in `bin/magic`, for every 32-bit value of `x`, including negative values and the extremes of the range.

## Rules

- Do not use `div` or `idiv`; the checker rejects them. Use multiplication, shifts, `lea` and similar instructions.
- Do not use `call`.
- Arithmetic wraps around at 32 bits exactly as in the binary.
- Signed quotients and remainders round toward zero, as in C.

## Check your work

```bash
make check EX=13 ARGS='--flag ARL{your_answer_here}'
```

The checker reports Part 1 and Part 2 separately. Without `--flag`, it checks Part 2 only.

## Hints

Open [HINTS.md](HINTS.md) one level at a time.
