[English](compiler-idioms.md) | [العربية](compiler-idioms.ar.md) | [Deutsch](compiler-idioms.de.md) | [Español](compiler-idioms.es.md) | [Français](compiler-idioms.fr.md)

# Compiler idioms

What optimizing compilers emit for common C constructs, and how to read it back. Lessons: [13. Recognizing code structures](../modules/13-code-structures/README.md), [14. Compiled arithmetic](../modules/14-compiled-arithmetic/README.md), [15. Recovering data](../modules/15-recovering-data/README.md). Examples are 32-bit `int` and `unsigned` code for x86-64 System V, with the first argument in `edi`.

## Function shape

| Pattern | What it tells you |
|---|---|
| `push rbp; mov rbp, rsp` ... `pop rbp` or `leave` | A frame pointer: unoptimized code, or a variable-size stack area |
| Pushes of `rbx`, `rbp`, `r12` to `r15` at the start | Values that live across calls |
| `sub rsp, N` | Space for locals, saved temporaries or alignment |
| No prologue, ends in `ret` | A leaf function |
| Ends in `jmp` to another function | A tail call: the callee returns to the caller's caller |
| A function body copied into the caller | Inlining; look for a call that is missing |

## Control flow

| Evidence | Source |
|---|---|
| `cmp` / `jcc` over a block | `if` |
| Two blocks with a `jmp` over the second | `if` / `else` |
| Several `jcc` to one "false" label, success falls through | Conditions joined with `&&` |
| `jcc` to a "true" label, last test jumps away on false | Conditions joined with the OR operator |
| `jmp test` at the start, test at the bottom with a backward `jcc` | `for` / `while`, unoptimized |
| A guard that skips the loop, body, then a backward `jcc` | `for` / `while`, optimized |
| Body first, only a backward `jcc` | `do` / `while` |
| Unconditional backward `jmp` with an exit in the body | Infinite loop |

## Switch

| Evidence | What it is |
|---|---|
| Several `cmp reg, constant / je` in a row | Compare chain |
| `cmp reg, N / ja default` then an indirect `jmp` through `[reg*8 + table]` | Jump table with N + 1 entries |
| `cmp reg, N / ja default` then a load from an array | Lookup table |
| `cmp` with a middle constant, then `je` and `jg`, repeated | Binary search |

## Multiplication by a constant

| Constant | Typical code |
|---|---|
| 2, 4, 8 | `shl` or `lea eax, [rdi*8]` |
| 3, 5, 9 | `lea eax, [rdi+rdi*2]`, `[rdi+rdi*4]`, `[rdi+rdi*8]` |
| 2^n - 1 (7, 15, 31) | `shl`, then `sub` of the original |
| 2^n + 1 (17, 33) | `shl`, then `add` of the original |
| 6, 10, 12 | `lea`, then `add` or `shl` |
| -1, -3 | `neg`, or `sub` from a scaled value |
| Large or irregular | `imul reg, reg, constant` |

`lea` computes `base + index*scale + displacement` (scale 1, 2, 4 or 8) without reading memory or changing flags. A `lea` whose result is never used as an address is arithmetic.

## Division and remainder by a constant

| Sequence | Operation |
|---|---|
| `shr reg, k` | Unsigned division by 2^k |
| `and reg, 2^k - 1` | Unsigned remainder modulo 2^k |
| `shr 31`, `add`, `sar` or `lea`, `test`, `cmovns`, `sar` | Signed division by a power of two (adds a bias of 2^k - 1 to negative values) |
| `sar 31`, `shr 32-k`, `add`, `and`, `sub` | Signed remainder by a power of two (never a bare `and`) |
| `imul` by a large odd constant, then `shr` | Unsigned division by a constant |
| The same plus `sar 31` and a `sub` | Signed division by a constant |
| `sub`, `shr`, `add`, `shr` after the high multiply | Division by a constant whose magic number needs 33 bits (for example 7) |
| `x - (x / d) * d` using the sequences above | Remainder by a constant |
| `cdq`, `idiv` | Signed division by a variable |
| `xor edx, edx`, `div` | Unsigned division by a variable |

Recovering the divisor: divisor = 2^shift / magic, rounded to the nearest integer. The script `modules/14-compiled-arithmetic/examples/14_magic.py` does this and checks it.

## Branch-free code

| Sequence | Meaning |
|---|---|
| `xor eax, eax` / `test` / `sete al` | `x == 0` as 0 or 1 |
| `cmp` / `setl` or `setb` | A comparison as 0 or 1 |
| `neg` / `cmovs` | Absolute value |
| `cmp a, b` / `sbb reg, reg` | All ones if `a < b` unsigned, else 0 |
| `cmp` / `cmovge` or `cmovnb` | Maximum of two values |

## Signed or unsigned

| | Signed | Unsigned |
|---|---|---|
| Jump after a compare | `jl`, `jle`, `jg`, `jge` | `jb`, `jbe`, `ja`, `jae` |
| Set from a compare | `setl`, `setg` | `setb`, `seta` |
| Conditional move | `cmovl`, `cmovge` | `cmovb`, `cmovae` |
| Right shift | `sar` | `shr` |
| Widening to 64 bits | `movsx` or `movsxd` | `mov` into a 32-bit register |
| Division | `cdq` / `idiv` | `xor edx, edx` / `div` |

## Data

| Evidence | Meaning |
|---|---|
| Several accesses at fixed offsets from one pointer | A structure; the offsets are its fields |
| `[base + index*scale]` | An array of elements of size `scale` |
| An access with a constant displacement and an index | An array inside a structure |
| A call to the allocator with a constant size | The size of a structure |
| A pointer stored at an offset in a structure that is loaded and followed in a loop | A linked list or tree |
| A store of a constant address into the first word of a new object | A vtable pointer (C++): the address points past the type information |
| `call [rax + 8*n]` after loading `rax` from `[rdi]` | A virtual call, slot `n` |
