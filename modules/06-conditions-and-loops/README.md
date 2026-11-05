[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# 06. Conditions, branches and loops

Time: about 2 hours. Prerequisites: [05. Arithmetic and logic](../05-arithmetic-and-logic/README.md)

## Why this matters

Assembly has no `if`, `while` or `for`. Every decision is a comparison that sets flags, followed by a jump that reads them. Once you can write these patterns by hand, you can recognize them in compiled code and recover the conditions, bounds and loop structure of a program you have no source for. The most common mistake, picking a signed jump where the code is unsigned, is also one of the most common misreadings in reverse engineering.

## Concepts

### Comparing without changing anything

| Instruction | Computes | Keeps |
|---|---|---|
| `cmp a, b` | `a - b` | Only the flags |
| `test a, b` | `a AND b` | Only the flags |

`cmp` is a subtraction whose result is thrown away. `test` is an `and` whose result is thrown away. `test rax, rax` is the usual way to ask whether `rax` is zero or negative, because the result of `rax AND rax` is `rax` itself.

### Conditional jumps

`jmp label` always jumps. A conditional jump jumps only when its condition holds, and otherwise execution continues with the next instruction. After `cmp a, b`, the jump names read as a comparison of `a` with `b`:

| Condition | Signed | Unsigned |
|---|---|---|
| equal | `je` (`jz`) | `je` (`jz`) |
| not equal | `jne` (`jnz`) | `jne` (`jnz`) |
| less | `jl` | `jb` ("below") |
| less or equal | `jle` | `jbe` |
| greater | `jg` | `ja` ("above") |
| greater or equal | `jge` | `jae` |

Equality does not depend on the interpretation; ordering does. `jl`, `jle`, `jg` and `jge` read SF and OF (and ZF), which describe a signed result. `jb`, `jbe`, `ja` and `jae` read CF (and ZF), which describe an unsigned one. Module 02 explained why: CF is the unsigned overflow flag and OF is the signed one. Some jumps read a single flag: `js` and `jns` (sign), `jo` and `jno` (overflow), `jc` and `jnc` (carry; the same as `jb` and `jae`), `jp` and `jnp` (parity).

### Setting a value or choosing one without jumping

| Instruction | Effect |
|---|---|
| `setcc r8` | Sets an 8-bit register to 1 if the condition holds, else 0 (`setg`, `setl`, `sete`, `setz`, `setc`, and so on) |
| `cmovcc r, source` | Copies the source into the register only if the condition holds (`cmovl`, `cmovge`, `cmovb`, and so on) |

Both read the flags exactly like a conditional jump, and neither changes them. `setcc` writes only 8 bits, so the rest of the register keeps its old value; clear the register first, and do it before the comparison, because `xor` changes the flags. These instructions avoid a branch, which compilers like for short conditions.

### Building the usual structures

**if / else.** Test, jump over the block you want to skip, and jump over the else part at the end of the first block:

```nasm-fragment
    cmp     rdi, rsi
    jle     .else               ; condition false: go to the else part
    ; ... then part ...
    jmp     .end
.else:
    ; ... else part ...
.end:
```

**while.** The test comes first, so the body may never run:

```nasm-fragment
.while:
    cmp     rcx, rdi
    jg      .done               ; condition false: leave the loop
    ; ... body ...
    jmp     .while
.done:
```

**do-while.** The test comes last, so the body runs at least once. This is the shape you want for counting the digits of a number, since 0 has one digit:

```nasm-fragment
.again:
    ; ... body ...
    test    rax, rax
    jnz     .again              ; condition true: go around again
```

**for.** An initialization, a test, and a step, which is a while loop with the step at the end of the body. Complete versions of all of these are in [06_control.asm](examples/06_control.asm).

### The `loop` instruction

`loop label` subtracts 1 from `rcx` and jumps if the result is not zero. It looks convenient, but on most processors it is slower than `dec rcx` followed by `jnz`, and compilers do not emit it. It also has a trap: with `rcx` equal to 0 on entry, it subtracts first, gets a huge nonzero value, and runs 2^64 times.

### The reversing view

Compilers do not give your loops back in the shape you wrote them. Run `make compiled` in the examples folder. These are real excerpts:

```text
<max_signed>:     cmp    rsi,rdi
                  mov    rax,rdi
                  cmovge rax,rsi
                  ret

<sum_to>:         test   rdi,rdi
                  jle    <skip>               ; n <= 0: do not enter the loop
                  add    rdi,0x1
                  mov    eax,0x1
                  mov    edx,0x0
                  add    rdx,rax              ; the loop body
                  add    rax,0x1
                  cmp    rax,rdi
                  jne    <body>               ; the test is at the bottom
                  mov    rax,rdx
                  ret

<sum_array>:      test   rsi,rsi
                  jle    <skip>
                  mov    rax,rdi
                  lea    rsi,[rdi+rsi*4]      ; end pointer
                  mov    edx,0x0
                  movsxd rcx,DWORD PTR [rax]  ; the body walks a pointer, no index
                  add    rdx,rcx
                  add    rax,0x4
                  cmp    rax,rsi
                  jne    <body>
```

What to read from this:

- **Swapped operands.** `max_signed` compares `rsi` with `rdi` and uses `cmovge`: "if b >= a, take b". The condition is rewritten, but a signed condition (`ge`) tells you the type is signed. `max_unsigned` gives the same code with `cmovae`.
- **Loop inversion.** A guard (`test` and `jle`) decides whether to enter the loop at all, and the real test sits at the bottom with a backward jump. A loop with a backward conditional jump is the signature of `for` and `while` in optimized code.
- **Rewritten counters.** `sum_to` runs a counter up to `n + 1` with `jne`, and `sum_array` has no index at all: it walks a pointer up to an end pointer. The loop is equivalent to yours, not identical.
- **The jump tells you the type.** `jl` or `jg` means signed; `jb` or `ja` means unsigned.

## Try it

Build and run the examples:

```bash
cd modules/06-conditions-and-loops/examples
make run
```

Expected output:

```text
max_signed(-1, 1)        = 1
max_unsigned(-1, 1)      = 18446744073709551615
sign_of(-7), (0), (12)   = -1 0 1
sign_branchless same     = -1 0 1
sum_to(10), sum_to(0)    = 55 0
count_digits(0), (9), (12345) = 1 1 5
sum_array({4,-9,20,1}, 4) = 16
is_power_of_two(64), (96), (0) = 1 0 0
```

The first two lines compare the same bits, -1 and 1, and give different answers: signed, -1 is smaller; unsigned, -1 is the largest value there is. That is the difference between `cmovl` and `cmovb`.

Then change something and predict the result before you rebuild:

1. In `sum_to`, change `jg .done` to `jge .done`. What does `sum_to(10)` return?
2. In `max_unsigned`, change `cmovb` to `cmovl`. Which call changes its answer?

## Common pitfalls

- **A signed jump on unsigned data, or the reverse.** `jl` on an unsigned length misbehaves as soon as the top bit is set. Decide the type first, then pick `jl` or `jb`.
- **The operand order of `cmp`.** `cmp a, b` followed by `jg` jumps when `a` is greater than `b`. Reading it backwards gives the opposite condition.
- **Changing the flags between the compare and the jump.** An `add` or `xor` in between replaces the flags. `mov` and `lea` are safe.
- **Clearing a register after the compare.** `xor eax, eax` between `cmp` and `setg al` destroys the flags. Clear first, then compare.
- **Off by one.** `jg` and `jge` differ by one pass of the loop. Test the first and the last value.
- **Forgetting the step.** A loop that never increments its counter never ends.
- **`loop` with `rcx` equal to 0.** It runs 2^64 times.

## Summary

- `cmp` and `test` set flags and discard the result; conditional jumps, `setcc` and `cmovcc` read the flags.
- Signed comparisons use `jl`, `jle`, `jg`, `jge`; unsigned ones use `jb`, `jbe`, `ja`, `jae`; equality is the same for both.
- `if`, `while`, `do-while` and `for` are all a compare and a jump; the position of the test decides how often the body runs.
- Compilers put the test at the bottom of loops, rewrite counters into pointers, and replace short branches with `cmov` or `setcc`.
- The jump condition in compiled code tells you whether the original type was signed or unsigned.

## Next steps

- Take the [quiz](quiz.md).
- Continue with module 07, strings and string instructions.
- Exercises that use this module: [Exercise 05](../../exercises/05-ticket-counter/README.md), [Exercise 06](../../exercises/06-branch-maze/README.md), [Exercise 07](../../exercises/07-string-sweeper/README.md).
