[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# 04. Moving data and addressing modes

Time: about 2 hours. Prerequisites: [03. Your first program](../03-first-program/README.md)

## Why this matters

Most instructions in a program move data: from memory to a register, from a register to memory, from one register to another. The way an instruction names a memory location, its addressing mode, is how arrays, structures and local variables look in assembly. When you read compiled code, the bracketed expression in an instruction tells you what the original program indexed, and with what element size.

## Concepts

### The `mov` instruction

`mov destination, source` copies the source to the destination. The source is not changed. Five forms exist:

| Form | Example | Meaning |
|---|---|---|
| register, immediate | `mov eax, 5` | Put a constant in a register |
| register, register | `mov rbx, rax` | Copy one register to another |
| register, memory | `mov eax, [rdi]` | Load from memory |
| memory, register | `mov [rdi], eax` | Store to memory |
| memory, immediate | `mov dword [rdi], 5` | Store a constant to memory |

There is no memory-to-memory form. To copy between two memory locations, go through a register. Both operands must have the same size, and the size must be known: a register operand fixes it, but a constant stored to memory does not, so you must say how wide it is with `byte`, `word`, `dword` or `qword`.

### Memory operands

Square brackets mean "the memory at this address". Without brackets you get the value of the expression itself. The general form of a memory operand is:

```text
[ base + index * scale + displacement ]
```

- **base** and **index** are registers. Either may be left out.
- **scale** is 1, 2, 4 or 8, and multiplies the index. It matches the element size of an array of bytes, words, dwords or qwords.
- **displacement** is a constant, such as a field offset in a structure.

Typical uses:

| Operand | Typical meaning |
|---|---|
| `[rdi]` | The memory a pointer points to |
| `[rdi + 8]` | A field at offset 8 in a structure |
| `[rdi + rsi*4]` | Element `rsi` of an array of 4-byte values |
| `[rdi + rsi + 8]` | A field at offset 8 of an element whose offset is already in `rsi` |
| `[table + 8]` | A global variable, addressed relative to `rip` in 64-bit code |

### Operand size

The size of a memory access comes from the register involved (`eax` is 4 bytes, `al` is 1) or from a keyword (`byte`, `word`, `dword`, `qword`). A 32-bit load into `eax` also clears the upper half of `rax`, as module 02 showed.

### Widening loads

Module 01 explained zero and sign extension. Three instructions apply them while loading:

| Instruction | Does | Use for |
|---|---|---|
| `movzx eax, byte [rdi]` | Zero-extend 8 or 16 bits | Unsigned values |
| `movsx eax, byte [rdi]` | Sign-extend 8 or 16 bits | Signed values |
| `movsxd rax, dword [rdi]` | Sign-extend 32 to 64 bits | A signed 32-bit value used as a 64-bit one |

There is no `movzx` from 32 to 64 bits, because an ordinary 32-bit `mov` already clears the upper half.

### `lea`: computing an address

`lea rax, [rdi + rdi*4 + 7]` computes the address expression and stores the result in `rax`. It never reads memory. Compilers use it as a fast way to do arithmetic: this one calculates `5 * rdi + 7` in a single instruction.

### `xchg`

`xchg a, b` swaps two operands. With a memory operand it is slow because the processor locks the memory automatically, so ordinary code swaps with `mov` through registers.

### Addressing relative to `rip`

In 64-bit code, a reference to a global such as `[table + 8]` is encoded as an offset from the instruction pointer when you write `default rel` at the top of the file. This lets the program load at any address. You will see `[rip + 0x...]` everywhere in 64-bit disassembly. In 32-bit code the address is an absolute constant instead, which you saw in exercise 02.

### A note on calling these functions

The examples are functions called from a C program. Until module 08 explains the calling convention, take two facts as given: the first two integer arguments arrive in `rdi` and `rsi`, and the result goes back in `rax`. `ret` returns to the caller.

### The reversing view

Compare the functions in [04_addressing.asm](examples/04_addressing.asm) with what a compiler produces for equivalent C in [04_compiled.c](examples/04_compiled.c). Run `make compiled` in the examples folder:

```text
<get_dword>:
  movsxd rax,DWORD PTR [rdi+rsi*4]
  ret
<load_u8>:
  movzx  eax,BYTE PTR [rdi]
  ret
<load_s8>:
  movsx  eax,BYTE PTR [rdi]
  ret
<times5plus7>:
  lea    rax,[rdi+rdi*4+0x7]
  ret
<item_price>:
  shl    rsi,0x4
  mov    rax,QWORD PTR [rsi+rdi*1+0x8]
  ret
```

The match is not an accident. The C source `a[i]` on an `int` array becomes `[rdi+rsi*4]`; `*p` on an unsigned byte becomes `movzx` and on a signed byte becomes `movsx`; `x * 5 + 7` becomes one `lea`; and `v[i].price` becomes `shl rsi, 4` (the structure is 16 bytes) plus a displacement of 8 (the field offset). Reading the bracket expression backwards recovers the data structure: a scale of 4 means 4-byte elements, a constant displacement means a field, a shift by 4 means 16-byte records.

## Try it

Build and run the examples:

```bash
cd modules/04-moving-data/examples
make run
```

Expected output:

```text
get_dword(numbers, 3)    = -40
load_u8(0xF6)            = 246
load_s8(0xF6)            = -10
load_le16(34 12)         = 0x1234
times5plus7(6)           = 37
item_price(items, 1)     = 975
after set_second         = 10 0 30 -40
after swap_ints          = 2 1
table_second()           = 222
```

Map each line to the function that produced it. `load_u8` and `load_s8` read the same byte and give 246 and -10, which is module 01's lesson in action. `load_le16` reads the bytes `34 12` and returns `0x1234`: the low byte comes first.

Then change something and predict the result before you rebuild:

1. In `get_dword`, change the scale from 4 to 8. What does `get_dword(numbers, 1)` read now?
2. In `times5plus7`, change the instruction to `lea rax, [rdi + rdi*2 + 1]`. What does `times5plus7(6)` return?

## Common pitfalls

- **Forgetting the size on a store of a constant.** `mov [rdi], 1` fails with `error: operation size not specified`. Write `mov dword [rdi], 1`.
- **Memory to memory.** `mov [rdi], [rsi]` fails with `error: invalid combination of opcode and operands`. Go through a register.
- **Forgetting the brackets.** `mov eax, x` loads the address of `x`; `mov eax, [x]` loads the value stored there.
- **Expecting `lea` to read memory.** It only computes an address. `lea rax, [rdi + 8]` and `mov rax, [rdi + 8]` do different things.
- **`movzx` from 32 to 64 bits.** It does not exist and NASM rejects it with `error: invalid combination of opcode and operands`. Use `mov eax, eax` or any 32-bit write.
- **Using the wrong extension.** `movzx` on a signed byte turns -10 into 246.

## Summary

- `mov` copies; there is no memory-to-memory form, and a constant stored to memory needs a size keyword.
- A memory operand is `[base + index*scale + displacement]`; brackets mean "the memory at".
- `movzx` and `movsx` zero- or sign-extend while loading; `movsxd` widens 32 to 64 bits.
- `lea` computes an address without reading memory and is often used for arithmetic.
- Reading a bracket expression backwards reveals array element sizes and structure field offsets.

## Next steps

- Take the [quiz](quiz.md).
- Continue with module 05, arithmetic and logic.
- Exercises that use this module: [Exercise 03](../../exercises/03-register-ledger/README.md), [Exercise 04](../../exercises/04-byte-order/README.md), [Exercise 07](../../exercises/07-string-sweeper/README.md).
