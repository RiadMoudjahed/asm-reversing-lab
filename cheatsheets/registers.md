[English](registers.md) | [العربية](registers.ar.md) | [Deutsch](registers.de.md) | [Español](registers.es.md) | [Français](registers.fr.md)

# x86 and x86-64 registers

## General-purpose registers

Writing a 32-bit register in 64-bit mode clears the upper 32 bits of the full register. Writing an 8-bit or 16-bit register leaves the other bits unchanged.

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
| `r8` to `r15` | `r8d` to `r15d` | `r8w` to `r15w` | `r8b` to `r15b` | none |

The registers `r8` to `r15`, and the 8-bit names `sil`, `dil`, `bpl` and `spl`, exist only in 64-bit mode.

## Special registers

| Register | Purpose |
|---|---|
| `rip` / `eip` | Address of the next instruction. Changed by jumps, `call` and `ret`. In 64-bit code it can be used as a base for addressing, as in `[rel label]`. |
| `rsp` / `esp` | Stack pointer. Points at the most recently pushed item. |
| `rbp` / `ebp` | Frame pointer by convention; a general register in optimized code. |
| `rflags` / `eflags` | Status and control flags (see below). |

## Flags

| Flag | Name | Set when |
|---|---|---|
| `CF` | Carry | An unsigned operation carried out or borrowed |
| `ZF` | Zero | The result is zero |
| `SF` | Sign | The result's top bit is set |
| `OF` | Overflow | A signed operation overflowed |
| `PF` | Parity | The low byte of the result has an even number of 1 bits |
| `AF` | Adjust | A carry or borrow crossed bit 3 (used by BCD instructions) |
| `DF` | Direction | String instructions move backward; clear it with `cld` |

## Who must preserve what

| Register | System V AMD64 | Microsoft x64 | 32-bit cdecl, stdcall, fastcall |
|---|---|---|---|
| `rax` | Caller-saved, result | Caller-saved, result | Caller-saved, result in `eax` |
| `rbx` | Callee-saved | Callee-saved | Callee-saved |
| `rcx`, `rdx` | Caller-saved | Caller-saved | Caller-saved |
| `rsi`, `rdi` | Caller-saved | Callee-saved | Callee-saved |
| `rbp` | Callee-saved | Callee-saved | Callee-saved |
| `r8` to `r11` | Caller-saved | Caller-saved | not available |
| `r12` to `r15` | Callee-saved | Callee-saved | not available |

See [Calling conventions](calling-conventions.md) for argument registers.
