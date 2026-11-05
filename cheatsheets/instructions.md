[English](instructions.md) | [العربية](instructions.ar.md) | [Deutsch](instructions.de.md) | [Español](instructions.es.md) | [Français](instructions.fr.md)

# Common instructions

Intel syntax: the destination comes first. Flags column: which flags the instruction changes.

## Data movement

| Instruction | Effect | Flags |
|---|---|---|
| `mov d, s` | Copy `s` to `d` | none |
| `movzx d, s` | Copy with zero extension | none |
| `movsx d, s` | Copy with sign extension | none |
| `lea d, [m]` | Load the address `m`, without reading memory | none |
| `xchg a, b` | Swap `a` and `b` | none |
| `push s` | Decrease `rsp` and store `s` | none |
| `pop d` | Load `d` from the stack and increase `rsp` | none |

## Arithmetic and logic

| Instruction | Effect | Flags |
|---|---|---|
| `add d, s` / `sub d, s` | Addition and subtraction | all arithmetic flags |
| `adc d, s` / `sbb d, s` | Add or subtract with carry | all arithmetic flags |
| `inc d` / `dec d` | Add or subtract 1 | all except `CF` |
| `neg d` | Two's complement negation | all arithmetic flags |
| `imul d, s` | Signed multiplication, truncated | `CF`, `OF` |
| `mul s` / `div s` | Unsigned multiply or divide using `rax` and `rdx` | varies |
| `idiv s` | Signed divide using `rax` and `rdx` | undefined |
| `and`, `or`, `xor`, `not` | Bitwise operations (`not` leaves flags unchanged) | `ZF`, `SF`, `PF`; `CF` and `OF` cleared |
| `shl`, `shr`, `sar` | Shift left, logical right, arithmetic right | `CF`, `ZF`, `SF`, `PF` |
| `rol`, `ror` | Rotate | `CF`, `OF` |
| `cmp a, b` | Compute `a - b`, keep only the flags | all arithmetic flags |
| `test a, b` | Compute `a & b`, keep only the flags | `ZF`, `SF`, `PF` |

## Control flow

| Instruction | Effect |
|---|---|
| `jmp t` | Jump to `t` |
| `je` / `jne` | Jump if equal / not equal (`ZF`) |
| `ja` / `jb` / `jae` / `jbe` | Jump after / below, unsigned (`CF`, `ZF`) |
| `jg` / `jl` / `jge` / `jle` | Jump greater / less, signed (`SF`, `OF`, `ZF`) |
| `js` / `jns` | Jump if sign set / clear |
| `call t` | Push the return address, jump to `t` |
| `ret` / `ret N` | Pop the return address into `rip`; `ret N` also removes `N` bytes |
| `loop t` | Decrease `rcx`, jump if not zero |

## String instructions

| Instruction | Effect |
|---|---|
| `movs`, `stos`, `lods` | Copy, store `al`/`ax`/`eax`/`rax`, load into it |
| `cmps`, `scas` | Compare two memory operands, compare memory with `al`/`ax`/`eax`/`rax` |
| `rep`, `repe`, `repne` | Repeat while `rcx` is not zero (and `ZF` is set or clear, for `repe` and `repne`) |
| `cld` / `std` | Clear or set the direction flag |

## System

| Instruction | Effect |
|---|---|
| `syscall` | 64-bit system call |
| `int 0x80` | 32-bit Linux system call |
| `nop` | Do nothing |
| `leave` | `mov rsp, rbp` then `pop rbp` |
