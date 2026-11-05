[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# 05. Arithmetic and logic

Time: about 2.5 hours. Prerequisites: [04. Moving data and addressing modes](../04-moving-data/README.md)

## Why this matters

Arithmetic instructions do two jobs at once: they compute a result and they describe it in the flags. Every comparison, every overflow check and every conditional jump in the next module depends on those flags. When you read compiled code, you also need to recognize what a short sequence of shifts, adds and masks really computes.

## Concepts

### Addition and subtraction

| Instruction | Effect | Flags |
|---|---|---|
| `add a, b` | `a = a + b` | CF, ZF, SF, OF, PF, AF |
| `sub a, b` | `a = a - b` | CF (borrow), ZF, SF, OF, PF, AF |
| `inc a`, `dec a` | `a = a + 1`, `a = a - 1` | Everything except CF, which is left unchanged |
| `neg a` | `a = 0 - a` | CF is 1 unless `a` was 0 |

The processor does not know whether your numbers are signed or unsigned. It sets both interpretations' flags and you read the right one: **CF** for unsigned overflow (a carry out of an addition, a borrow in a subtraction) and **OF** for signed overflow. `add rax, 1` on `0x7FFFFFFFFFFFFFFF` sets OF, because the signed result wraps from the largest positive number to the most negative, but not CF. The same instruction on `0xFFFFFFFFFFFFFFFF` sets CF and ZF, and not OF.

### Carry chains

`adc a, b` adds `b` and the carry flag; `sbb a, b` subtracts `b` and the borrow. They let you build arithmetic wider than a register. Adding two 128-bit numbers is an `add` on the low halves followed by an `adc` on the high halves:

```nasm-fragment
    mov     rax, [rsi]
    add     [rdi], rax                  ; low halves: may set CF
    mov     rax, [rsi + 8]
    adc     [rdi + 8], rax              ; high halves plus the carry
```

### Multiplication

| Form | Meaning |
|---|---|
| `mul b` | Unsigned: `rdx:rax = rax * b`. The 128-bit result occupies two registers. |
| `imul b` | Signed, same layout as `mul`. |
| `imul a, b` | Signed, `a = a * b`, truncated to the register size. |
| `imul a, b, constant` | Signed, `a = b * constant`, truncated. |

`mul` writes `rdx` as part of its result. If `rdx` held a pointer you still need, save it first; the `mul_wide` example does this. For the truncating `imul` forms, CF and OF are set when the full result did not fit in the register.

### Division

`div b` and `idiv b` divide the double-width value `rdx:rax` by `b`. The quotient goes to `rax` and the remainder to `rdx`.

- Before `idiv`, set `rdx` to the sign extension of `rax` with `cqo` (`cdq` for 32-bit values). Without it, `rdx` holds garbage and the result is wrong.
- Before `div`, set `rdx` to zero (`xor edx, edx`).
- Dividing by zero, or a quotient too large for `rax`, raises a processor fault, which Linux delivers to the program as the signal `SIGFPE`.

`idiv` rounds the quotient toward zero, so the remainder takes the sign of the dividend: `-17 / 5` gives quotient -3 and remainder -2.

### Logic

| Instruction | Effect |
|---|---|
| `and a, b` | Keeps the bits that are 1 in both. Use it to select bits with a mask. |
| `or a, b` | Sets the bits that are 1 in either. Use it to force bits to 1. |
| `xor a, b` | Flips the bits where `b` is 1. `xor eax, eax` sets `eax` to 0 in two bytes. |
| `not a` | Inverts every bit. No flags change. |

`and`, `or` and `xor` clear CF and OF and set ZF, SF and PF from the result.

### Shifts and rotates

| Instruction | Effect | Use |
|---|---|---|
| `shl a, n` | Shift left, filling with 0 | Multiply by 2^n |
| `shr a, n` | Shift right, filling with 0 (logical) | Unsigned divide by 2^n |
| `sar a, n` | Shift right, copying the sign bit in (arithmetic) | Signed divide by 2^n, rounding down |
| `rol a, n`, `ror a, n` | Rotate: bits leaving one end enter the other | Byte swapping, hashing |

The count `n` is a constant or the register `cl`. CF receives the last bit shifted out. `sar` rounds toward negative infinity while `idiv` rounds toward zero, so `-17 >> 2` is -5 but `-17 / 4` is -4.

### Decimal arithmetic

Module 01 introduced packed BCD, where `0x38` means the number 38. Adding `0x38` and `0x45` in binary gives `0x7D`, not a valid BCD byte. The decimal adjust instructions (`daa`, `das`, `aaa`, `aas`, `aam`, `aad`) repair the result after an addition, subtraction, multiplication or division. They exist only in 32-bit code. In 64-bit mode NASM rejects them with `error: instruction not supported in 64-bit mode`, and compilers never emit them. [05_bcd32.asm](examples/05_bcd32.asm) adds 38 and 45 with `daa` and exits with `0x83`, which is the packed BCD for 83.

### The reversing view

Compilers choose the cheapest instruction for each arithmetic operation. Run `make compiled` in the examples folder:

```text
<times8>:        lea    rax,[rdi*8+0x0]
<div8_unsigned>: mov    rax,rdi
                 shr    rax,0x3
<shift_signed>:  mov    rax,rdi
                 sar    rax,0x2
<divide_vars>:   mov    rax,rdi
                 cqo
                 idiv   rsi
<remainder_vars>: mov    rax,rdi
                 cqo
                 idiv   rsi
                 mov    rax,rdx
```

Reading these backwards: a multiply by 8 never uses `imul`; it is a scale or a `shl`. A `shr` by 3 is an unsigned divide by 8, and a `sar` by 2 is a signed divide by 4 for a type you know is signed. A `cqo` followed by `idiv` is a signed division by a variable, and the same sequence ending with `mov rax, rdx` is the remainder. The choice between `shr` and `sar`, or `div` and `idiv`, tells you whether the original type was unsigned or signed. Module 14 covers the more elaborate rewrites compilers use for division by constants.

## Try it

Build and run the examples:

```bash
cd modules/05-arithmetic-and-logic/examples
make run
```

Expected output:

```text
add_overflows(INT64_MAX, 1)      = 1  (sum = -9223372036854775808)
add_overflows(5, 7)              = 0
add_carries(UINT64_MAX, 1)       = 1
add_carries(INT64_MAX, 1)        = 0
mul_wide(2^32, 3*2^32)           = hi 3, lo 0
divide(-17, 5)                   = quotient -3, remainder -2
sar_by(-16, 2)                   = -4
shr_by(-16, 2)                   = 4611686018427387900
add128(2^64 - 1, 1)              = high 1, low 0
negate(10)                       = -10
combine_bits(0xAB)               = 0x50
05_bcd32 exit status: 131
```

Check each line against the theory. Lines 1 and 4 add the same operands: OF reports an overflow and CF does not. `sar_by` and `shr_by` shift the same bits and give -4 and a huge positive number. `05_bcd32` exits with 131, which is `0x83`.

Then step through `add_overflows` in the debugger and watch the flags:

```bash
gdb -q build/05_main
```

```text
break add_overflows
run
stepi
p $eflags
```

Before you press Enter, predict which flags are set after the `add`.

## Common pitfalls

- **Skipping `cqo` before `idiv`.** `rdx` still holds an old value, so the division uses a garbage upper half. The result is wrong, or the process dies with `SIGFPE`.
- **Losing a pointer in `rdx`.** `mul` and `div` write `rdx`. Save what you need in another register first.
- **Dividing by zero.** `div` and `idiv` fault on a zero divisor. In compiled code the check usually sits before the instruction.
- **`sar` where `shr` is meant, and the reverse.** `sar` on an unsigned value keeps a top bit of 1 as 1; `shr` on a negative value turns it into a large positive number.
- **Reading CF after `inc` or `dec`.** They leave CF alone. Use `add` or `sub` when you need the carry.
- **Reading a flag after an instruction that overwrote it.** Almost every arithmetic instruction sets several flags. `mov` and `lea` set none, which is why `mov [rdx], rdi` can sit between `add` and `seto`.
- **Using BCD adjust instructions in 64-bit code.** They do not exist there.

## Summary

- `add`, `sub` and friends compute a result and set CF (unsigned overflow), OF (signed overflow), ZF and SF.
- `adc` and `sbb` chain operations through the carry flag to build wider arithmetic.
- `mul` and `div` work on `rdx:rax`; set `rdx` with `cqo` before `idiv` and with `xor` before `div`.
- `and`, `or`, `xor` and `not` work bit by bit; shifts multiply and divide by powers of two, with `sar` for signed values.
- Compilers pick `shr` or `sar`, and `div` or `idiv`, by the signedness of the type, which you can read back from the code.

## Next steps

- Take the [quiz](quiz.md).
- Continue with module 06, conditions, branches and loops.
- Exercises that use this module: [Exercise 03](../../exercises/03-register-ledger/README.md), [Exercise 05](../../exercises/05-ticket-counter/README.md), [Exercise 06](../../exercises/06-branch-maze/README.md), [Exercise 13](../../exercises/13-magic-numbers/README.md).
