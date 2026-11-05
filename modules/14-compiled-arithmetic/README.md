[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# 14. Compiled arithmetic

Time: about 2.5 hours. Prerequisites: [13. Recognizing code structures](../13-code-structures/README.md)

## Why this matters

Optimizing compilers rarely emit the instruction you would expect for an arithmetic operator. A multiplication becomes a `lea`, a division becomes a multiplication and a shift, a comparison becomes a flag trick with no jump. These sequences look like noise until you know the idioms; once you do, each one reads as a single operation, and the constants in them tell you the original numbers. This module teaches the idioms, shows how to recover the original constant from the compiled one, and explains how to tell signed from unsigned arithmetic.

## Concepts

### How the listings in this module were made

The C source is `examples/14_arith.c`. The `Makefile` compiles it with GCC at `-O0` and `-O2` and writes Intel-syntax assembly to `build/14_O0.s` and `build/14_O2.s`, with the same flags as in module 13. The listings are those files with the assembler directives removed. All code is 32-bit `int` and `unsigned` arithmetic in x86-64 System V code: the first argument is in `edi`, the second in `esi`, and the result in `eax`.

```bash
make
```

### Optimization changes the arithmetic

At `-O0` the compiler follows the source closely, but even then it never emits a slow instruction where a fast sequence exists.

```text
times7:
    push    rbp
    mov    rbp, rsp
    mov    DWORD PTR [rbp-4], edi
    mov    edx, DWORD PTR [rbp-4]
    mov    eax, edx
    sal    eax, 3
    sub    eax, edx
    pop    rbp
    ret
```

`x * 7` is `(x << 3) - x`: `sal eax, 3` is a left shift by 3, which multiplies by 8, and the `sub` takes one `x` away. At `-O2` the same function loses its frame and one instruction:

```text
times7:
    lea    eax, [0+rdi*8]
    sub    eax, edi
    ret
```

`lea eax, [0+rdi*8]` computes `8 * x` and `sub` subtracts `x`. In both versions you recognize the same shape: multiplication by a constant one less than a power of two is a shift and a subtraction.

### `lea` as arithmetic

`lea` (load effective address) computes the address expression `base + index*scale + displacement` and stores the result **without reading memory** and without changing flags. The scale can be 1, 2, 4 or 8. Compilers use it as a small, fast adder and multiplier.

```text
times5:
    lea    eax, [rdi+rdi*4]
    ret
```

```text
madd:
    lea    eax, [rsi+3+rdi*8]
    ret
```

`times5` is `x + 4*x`. `madd` computes `a * 8 + b + 3` with a single instruction: `rdi` scaled by 8, plus `rsi`, plus 3. Whenever you see `lea` with a register pair and a scale, and the "address" is never used to access memory, it is arithmetic.

| `lea` form | Value computed |
|---|---|
| `lea eax, [rdi+rdi*2]` | `3 * x` |
| `lea eax, [rdi+rdi*4]` | `5 * x` |
| `lea eax, [rdi+rdi*8]` | `9 * x` |
| `lea eax, [rdi*8+0]` | `8 * x` |
| `lea eax, [rsi+rdi*8+3]` | `8 * a + b + 3` |

### Multiplication by constants

A multiplication by a small constant is rewritten as shifts, additions and `lea`. The result must be the same as the real multiplication, including when it wraps around in 32 bits.

```text
times10:
    lea    eax, [rdi+rdi*4]
    add    eax, eax
    ret
```

```text
times_neg3:
    lea    edx, [0+rdi*4]
    mov    eax, edi
    sub    eax, edx
    ret
```

`times10` is `(x + 4*x) * 2`: a `lea` for 5x and an `add eax, eax` to double it. `times_neg3` computes `x - 4*x`, which is `-3 * x`, with a `lea` and a `sub`. For a constant such as 100 the compiler decides that one multiplication is cheaper:

```text
times100:
    imul    eax, edi, 100
    ret
```

`imul eax, edi, 100` is the three-operand multiply: destination, source, immediate. A multiplication by a constant with no shifts or `lea` around it is just a multiplication.

| Constant | Typical sequence |
|---|---|
| 2, 4, 8, ... | `shl` (or `lea` with a scale) |
| 3, 5, 9 | `lea eax, [rdi+rdi*2]`, `*4`, `*8` |
| 2^n - 1 (7, 15, 31) | `shl` and `sub` |
| 2^n + 1 (17, 33) | `shl` and `add` |
| 6, 10, 12, ... | `lea` or `shl` followed by `add` or `lea` |
| -1, -3 | `neg`, or `sub` from a scaled value |
| Large or irregular | `imul reg, reg, constant` |

### Division and modulo by powers of two

Unsigned division by 8 is a right shift, and the unsigned remainder is a mask:

```text
udiv8:
    mov    eax, edi
    shr    eax, 3
    ret
```

Signed division is more subtle, because C rounds toward zero while `sar` rounds toward minus infinity. For a negative `x`, `x / 2` and `x >> 1` differ by one when `x` is odd. The compiler fixes this by adding a bias to negative inputs before the shift.

```text
sdiv2:
    mov    eax, edi
    shr    eax, 31
    add    eax, edi
    sar    eax
    ret
```

`shr eax, 31` extracts the sign bit (1 for negative values, otherwise 0), `add` adds it to `x`, and `sar eax` shifts right by one. For `x = -3` the steps give `-3 + 1 = -2`, then `-2 >> 1 = -1`, which is what C returns for `-3 / 2`. This four-instruction idiom is the signature of signed division by 2. For larger powers of two the bias is `2^k - 1`:

```text
sdiv8:
    test    edi, edi
    lea    eax, [rdi+7]
    cmovns    eax, edi
    sar    eax, 3
    ret
```

`lea eax, [rdi+7]` is `x + 7`; `test` and `cmovns` keep `x` unchanged when it is not negative. Then `sar eax, 3` shifts. The remainder uses the same bias:

```text
smod8:
    mov    edx, edi
    sar    edx, 31
    shr    edx, 29
    lea    eax, [rdi+rdx]
    and    eax, 7
    sub    eax, edx
    ret
```

The first three instructions build the bias (`7` for negative `x`, `0` otherwise), `add` and `and eax, 7` reduce the biased value, and `sub` removes the bias. The signature is `sar 31`, `shr 29`, then an `and` with 7. A signed remainder by a power of two is never a bare `and`.

### Division by other constants: multiplication by a magic number

A division instruction is slow, so for a constant divisor the compiler replaces it with a multiplication by a precomputed **magic number** and a shift. The idea is to compute `x / d` as `x * M >> k`, where `M` is about `2^k / d`, rounded up. Unsigned division by 10 shows it:

```text
udiv10:
    mov    eax, edi
    mov    edx, 3435973837
    imul    rax, rdx
    shr    rax, 35
    ret
```

`3435973837` is `0xCCCCCCCD`. The 64-bit `imul` multiplies `x` by the magic number without overflow, and `shr rax, 35` removes 35 bits of fraction. Check the numbers: `3435973837 * 10 = 34359738370`, while `2^35 = 34359738368`. The magic number is `2^35 / 10` rounded up, and the shift of 35 is the exponent.

**Recovering the divisor from the code is a division:** divisor = `2^shift / magic`, rounded to the nearest integer. The script `examples/14_magic.py` does it and checks the result against a real division on 200 000 inputs:

```bash
python3 14_magic.py 3435973837 35
```

```text
2**35 / 3435973837 = 9.9999999994  ->  divisor 10
checked 200007 inputs, 0 differences
```

Signed division by 10 uses a different magic number and a correction for negative inputs:

```text
sdiv10:
    movsx    rax, edi
    sar    edi, 31
    imul    rax, rax, 1717986919
    sar    rax, 34
    sub    eax, edi
    ret
```

`movsx rax, edi` sign-extends `x` to 64 bits, and `sar edi, 31` turns the sign bit into 0 or -1. The product is shifted right by 34, and subtracting the sign (0 or -1) adds 1 for negative inputs, which turns the floor into a truncation toward zero:

```bash
python3 14_magic.py 1717986919 34 signed
```

```text
2**34 / 1717986919 = 9.9999999965  ->  divisor 10
checked 200007 inputs, 0 differences
```

Some divisors, 7 among them, need a magic number of 33 bits, which does not fit in a 32-bit multiply. The compiler then uses a longer sequence:

```text
udiv7:
    mov    eax, edi
    imul    rax, rax, 613566757
    shr    rax, 32
    sub    edi, eax
    shr    edi
    add    eax, edi
    shr    eax, 2
    ret
```

The first `imul` and `shr rax, 32` give the high half of the product. The `sub / shr / add` steps add back the missing bit, and the final `shr eax, 2` is the rest of the shift. The helper script handles this "fixup" form: the magic is `613566757 + 2^32` and the total shift is `32 + 2 + 1`:

```bash
python3 14_magic.py 613566757 2 fixup
```

```text
2**35 / 4908534053 = 6.9999999994  ->  divisor 7
checked 200007 inputs, 0 differences
```

A remainder by a constant is computed as `x - (x / d) * d`. For 10, the quotient comes from the magic sequence, the multiplication by 10 is `lea` and `add`, and a `sub` finishes:

```text
umod10:
    mov    eax, edi
    mov    edx, 3435973837
    imul    rax, rdx
    shr    rax, 35
    lea    edx, [rax+rax*4]
    mov    eax, edi
    add    edx, edx
    sub    eax, edx
    ret
```

When the divisor is not a constant, the compiler cannot precompute anything and uses the real instruction. The extension of the dividend is the signature of the signedness:

```text
sdiv_var:
    mov    eax, edi
    cdq
    idiv    esi
    ret
```

```text
udiv_var:
    mov    eax, edi
    xor    edx, edx
    div    esi
    ret
```

Signed division sign-extends `eax` into `edx` with `cdq` and uses `idiv`. Unsigned division clears `edx` with `xor edx, edx` and uses `div`. Both leave the quotient in `eax` and the remainder in `edx`.

| Sequence | Operation |
|---|---|
| `shr reg, k` | Unsigned division by `2^k` |
| `and reg, 2^k - 1` | Unsigned remainder by `2^k` |
| `shr 31 / add / sar` or `lea / cmovns / sar` | Signed division by a power of two |
| `imul` by a large odd constant, then `shr` | Division by a constant (unsigned) |
| The same plus `sar 31` and a `sub` | Division by a constant (signed) |
| `cdq` then `idiv` | Signed division by a variable |
| `xor edx, edx` then `div` | Unsigned division by a variable |

### Branchless code

Several idioms turn a comparison into a value without any jump. The condition flags are the input, and `setcc`, `cmov` and `sbb` produce the output.

```text
is_zero:
    xor    eax, eax
    test    edi, edi
    sete    al
    ret
```

`is_zero` is `x == 0`: `test` sets the zero flag, and `sete al` stores it as 0 or 1 (the `xor eax, eax` before it clears the rest of the register, because `setcc` writes only one byte).

```text
absval:
    mov    eax, edi
    neg    eax
    cmovs    eax, edi
    ret
```

`absval` negates `x` into `eax`. If the result is negative (`cmovs` tests the sign flag from `neg`), `x` was positive, so `eax` is replaced by the original. The result is the absolute value.

```text
sign:
    xor    eax, eax
    test    edi, edi
    setg    al
    shr    edi, 31
    sub    eax, edi
    ret
```

`sign` returns `(x > 0) - (x < 0)`: `setg al` gives 1 for positive values, `shr edi, 31` gives 1 for negative values, and the subtraction yields -1, 0 or 1.

```text
mask_lt:
    cmp    edi, esi
    sbb    eax, eax
    ret
```

`mask_lt` returns `-(a < b)`, which is all ones when `a < b` and zero otherwise. `cmp edi, esi` sets the carry flag when `edi` is below `esi` (unsigned), and `sbb eax, eax` computes `eax - eax - CF`, which is `-CF`. The pair `cmp / sbb reg, reg` creates a mask from a comparison; it is also a common building block of constant-time code.

| Idiom | Meaning |
|---|---|
| `xor eax, eax` / `test` / `sete al` | `x == 0` as 0 or 1 |
| `cmp` / `setl` or `setb` | A comparison as 0 or 1 |
| `neg` / `cmovs` | Absolute value |
| `cmp reg, reg` / `sbb reg, reg` | A mask of all ones if the first is below the second (unsigned), otherwise 0 |
| `cmp` / `cmovge` or `cmovnb` | Maximum of two values |

### Signed or unsigned

The machine does not store the signedness of a value. It is a property of the instructions that use it, which makes it one of the most useful facts you can recover from compiled code.

```text
less_s:
    xor    eax, eax
    cmp    edi, esi
    setl    al
    ret
```

```text
less_u:
    xor    eax, eax
    cmp    edi, esi
    setb    al
    ret
```

Both functions return `a < b`. The only difference is `setl` against `setb`, and that difference separates `int` from `unsigned`.

| | Signed | Unsigned |
|---|---|---|
| Jumps on a comparison | `jl`, `jle`, `jg`, `jge` | `jb`, `jbe`, `ja`, `jae` |
| Set from a comparison | `setl`, `setg`, ... | `setb`, `seta`, ... |
| Conditional move | `cmovl`, `cmovge`, ... | `cmovb`, `cmovnb` (also `cmovae`), ... |
| Right shift | `sar` | `shr` |
| Widening to 64 bits | `movsx` or `movsxd` | `mov` to a 32-bit register |
| Division | `cdq` / `idiv` | `xor edx, edx` / `div` |

The flags carry both interpretations at once: the carry flag reports an unsigned borrow, while the sign and overflow flags report a signed comparison. The jump or `setcc` that follows chooses which one the program cares about. Compare the two ways to widen a 32-bit value to 64 bits:

```text
extend_s:
    movsx    rax, edi
    ret
```

```text
extend_u:
    mov    eax, edi
    ret
```

For the signed `int`, `movsx rax, edi` copies the sign bit upward. For the `unsigned`, a plain `mov eax, edi` is enough, because on x86-64 every write to a 32-bit register sets the upper 32 bits of the full register to zero.

## Try it

1. Run `make` and open `build/14_O2.s`. For each function in `14_arith.c`, say what it computes before you read it, then check.
2. Run `python3 14_magic.py` with the constants from `umod10` and `smod10`. Do they give the same divisor as `udiv10` and `sdiv10`?
3. Write a C function `int times9(int x)` and a function `unsigned udiv3(unsigned x)` in a new file, compile with `-O2 -S -masm=intel`, and identify the idiom in each.
4. Find the magic number for division by 3, 5 and 100 (unsigned) in the compiler output, and recover each divisor with the script.
5. Explain, without compiling, why `x % 8` for a signed `int` cannot be a single `and`.

## Common pitfalls

- **Taking `lea` for a memory access.** `lea` never touches memory. Check whether the result is used as an address or as a number.
- **Missing the sign bias.** A `sar` alone is not a signed division. Look for the added bias, or for `cmovns`.
- **Forgetting the 64-bit multiply.** The magic multiplication needs the upper bits of the product, so it is done in a 64-bit register even though the values are 32 bits.
- **Reading a shift of 32 or more as a bug.** `shr rax, 32` extracts the high half of a 64-bit product.
- **Calling every `setcc` result a boolean.** `sete al` can feed an arithmetic expression, such as the `sign` function.

## Summary

- `lea` is a fast adder and multiplier by 2, 3, 4, 5, 8 and 9 that does not touch flags or memory.
- Multiplication by a constant is shifts, additions and `lea`; a plain `imul reg, reg, constant` means a constant that does not decompose well.
- Unsigned division by a power of two is `shr`; signed division adds a bias first. Remainders use `and`, with a bias for signed values.
- Division by a constant is multiplication by `2^k / d` rounded up, then a shift; the divisor is `2^k` divided by the magic number.
- `setcc`, `cmov` and `sbb` turn comparisons into values without jumps.
- The choice of `jl` or `jb`, `sar` or `shr`, `movsx` or `mov`, and `idiv` or `div` tells you whether the values are signed.

## Next steps

- Take the [quiz](quiz.md).
- Continue with module 15, recovering data.
- Cheatsheets: [Compiler idioms](../../cheatsheets/compiler-idioms.md).
- Exercises that use this module: [Exercise 13](../../exercises/13-magic-numbers/README.md).
