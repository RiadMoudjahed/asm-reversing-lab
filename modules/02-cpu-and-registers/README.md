[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# 02. The CPU and its registers

Time: about 90 minutes. Prerequisites: [01. Data representation](../01-data-representation/README.md)

## Why this matters

Registers are where a processor does its work, and flags are how one instruction passes information to the next. Every debugger view, every disassembly and every exercise in this course is written in terms of them. If you can read a register table at a glance, you can follow what any piece of code is doing.

## Concepts

### The fetch, decode, execute cycle

A processor repeats three steps. It **fetches** the instruction at the address in the instruction pointer, **decodes** it, and **executes** it. Then the instruction pointer moves to the next instruction, unless the instruction was a jump, call or return that set it somewhere else.

### General-purpose registers

64-bit mode has sixteen general-purpose registers of 64 bits each. Eight of them have names inherited from older processors, and each can be used at four widths:

| 64-bit | 32-bit | 16-bit | 8-bit low | 8-bit high |
|---|---|---|---|---|
| `rax` | `eax` | `ax` | `al` | `ah` |
| `rbx` | `ebx` | `bx` | `bl` | `bh` |
| `rcx` | `ecx` | `cx` | `cl` | `ch` |
| `rdx` | `edx` | `dx` | `dl` | `dh` |
| `rsi` | `esi` | `si` | `sil` | none |
| `rdi` | `edi` | `di` | `dil` | none |
| `rbp` | `ebp` | `bp` | `bpl` | none |
| `rsp` | `esp` | `sp` | `spl` | none |

The other eight are numbered: `r8` to `r15`. Their narrower views are `r8d` (32-bit), `r8w` (16-bit) and `r8b` (8-bit). In 32-bit code only the first eight exist, at 32, 16 and 8 bits.

Some registers have conventional roles, which later modules explain: `rsp` is the stack pointer, `rbp` is often the frame pointer, and `rax` carries function results and system call numbers.

### Writing to a narrow view

The narrow names are windows onto one register, not separate registers. What happens to the other bits depends on the width you write:

- Writing an **8-bit or 16-bit** view changes only those bits. The rest is preserved.
- Writing a **32-bit** view changes the low 32 bits and **clears the upper 32 bits to zero**.

You can watch this in [02_registers.asm](examples/02_registers.asm). The value of `rax` after each instruction, read in the debugger:

| Instruction | `rax` afterwards |
|---|---|
| `mov rax, 0x1122334455667788` | `0x1122334455667788` |
| `mov al, 0xFF` | `0x11223344556677ff` |
| `mov ah, 0xEE` | `0x112233445566eeff` |
| `mov ax, 0xDDCC` | `0x112233445566ddcc` |
| `mov eax, 0xAABBCCDD` | `0x00000000aabbccdd` |
| `mov rax, -1` | `0xffffffffffffffff` |
| `mov eax, eax` | `0x00000000ffffffff` |

The last row is a deliberate idiom: `mov eax, eax` zero-extends `eax` into `rax`. You will meet it in compiled code, where it means "treat this 32-bit value as unsigned 64-bit".

### The instruction pointer

`rip` holds the address of the next instruction to execute. You do not write to it with `mov`; jumps, calls and returns change it. In 32-bit code it is `eip`.

### The flags register

`rflags` (or `eflags`) holds single-bit flags that the processor sets as a side effect of arithmetic and comparisons. Later instructions read them. The ones you need first:

| Flag | Name | Set when |
|---|---|---|
| CF | carry | An unsigned operation overflowed or borrowed |
| ZF | zero | The result is zero |
| SF | sign | The result's top bit is 1 (negative if signed) |
| OF | overflow | A signed operation overflowed |
| PF | parity | The low byte of the result has an even number of 1 bits |
| AF | adjust | A carry crossed from bit 3 to bit 4 (used by decimal arithmetic) |
| DF | direction | Controls the direction of string instructions (module 07) |

CF and OF answer the same question for different interpretations. Adding 255 and 1 in a byte wraps to 0 and sets CF, an unsigned overflow; read as signed it is -1 + 1 = 0, so OF stays clear. Adding 127 and 1 gives `0x80`: that is 128 unsigned with no carry (CF=0), but -128 signed, so OF is set.

You can see each case in [02_flags.asm](examples/02_flags.asm). The debugger shows the flags after each pair of steps:

| Instructions | Flags set afterwards |
|---|---|
| `mov al, 255` then `add al, 1` | CF, PF, AF, ZF |
| `mov al, 127` then `add al, 1` | AF, SF, OF |
| `mov al, 5` then `sub al, 5` | PF, ZF |
| `mov al, 3` then `cmp al, 5` | CF, AF, SF |

`IF` also appears in every line; it is the interrupt flag, which you can ignore. Note that `cmp al, 5` computes 3 - 5 and discards the result, keeping only the flags: CF reports that a subtraction borrowed (3 is less than 5 unsigned), and SF reports a negative result.

### Segment registers

`cs`, `ss`, `ds`, `es`, `fs` and `gs` come from the segmented memory of 16-bit processors. In modern flat-memory programs the first four play no practical role. On Linux and Windows, `fs` or `gs` points to per-thread data, which you will see in some disassembly.

### The reversing view

A debugger prints this whole register set after every step, so reading it quickly is the core skill. GEF prints flag names, and uppercase means set, for example `[ZERO carry PARITY adjust sign ...]` after `xor edi, edi`. When you see `[... CARRY ...]` after a `cmp`, the left operand was unsigned-less-than the right one.

## Try it

Build the examples and step through them in the debugger:

```bash
cd modules/02-cpu-and-registers/examples
make
gdb -q build/02_registers
```

At the prompt, type these commands, repeating `stepi` and `p/x $rax` for each instruction:

```text
break _start
run
stepi
p/x $rax
```

Compare each value with the table above. Then do the same for flags in `build/02_flags`:

```text
break _start
run
stepi 2
p $eflags
```

Predict the flags for each pair of instructions before you press Enter, then check.

## Common pitfalls

- **Expecting a 32-bit write to preserve the upper half.** It clears it. An 8-bit or 16-bit write does preserve it.
- **Reading CF as "overflow" for signed numbers.** Signed overflow is OF.
- **Using `ah` with `sil`, `dil`, `bpl` or `spl`.** You cannot combine the high-byte registers with instructions that need the newer byte registers.
- **Assuming a flag survives.** Almost every arithmetic instruction rewrites several flags. Read a flag immediately after the instruction that sets it.

## Summary

- Sixteen 64-bit registers; `rax` contains `eax`, which contains `ax`, which contains `al` and `ah`.
- 8-bit and 16-bit writes preserve the other bits; 32-bit writes clear the upper 32.
- `rip` is the next instruction; jumps, calls and returns change it.
- CF reports unsigned overflow, OF signed overflow, ZF zero, SF sign.
- A debugger shows all of these after every step.

## Next steps

- Take the [quiz](quiz.md).
- Continue with [03. Your first program](../03-first-program/README.md).
- Exercises that use this module: [Exercise 02](../../exercises/02-two-worlds/README.md), [Exercise 03](../../exercises/03-register-ledger/README.md).
