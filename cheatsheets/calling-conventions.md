[English](calling-conventions.md) | [العربية](calling-conventions.ar.md) | [Deutsch](calling-conventions.de.md) | [Español](calling-conventions.es.md) | [Français](calling-conventions.fr.md)

# Calling conventions

## 32-bit

| | cdecl | stdcall | fastcall |
|---|---|---|---|
| Arguments | Stack, right to left | Stack, right to left | First two integers in `ecx` and `edx`, the rest on the stack |
| Who removes stack arguments | Caller | Callee, with `ret N` | Callee, with `ret N` |
| Result | `eax` | `eax` | `eax` |
| Callee-saved | `ebx`, `esi`, `edi`, `ebp` | `ebx`, `esi`, `edi`, `ebp` | `ebx`, `esi`, `edi`, `ebp` |

## 64-bit

| | System V AMD64 (Linux, macOS) | Microsoft x64 (Windows) |
|---|---|---|
| Integer and pointer arguments | `rdi`, `rsi`, `rdx`, `rcx`, `r8`, `r9` | `rcx`, `rdx`, `r8`, `r9` |
| Floating-point arguments | `xmm0` to `xmm7` | `xmm0` to `xmm3` |
| More arguments | Stack | Stack, above the shadow space |
| Result | `rax` (`xmm0` for floating point) | `rax` (`xmm0` for floating point) |
| Shadow space | None | 32 bytes reserved by the caller |
| Red zone | 128 bytes below `rsp` for leaf functions | None |
| Variadic calls | `al` holds the number of vector registers used | Floating-point arguments are also copied to integer registers |
| Callee-saved | `rbx`, `rbp`, `r12` to `r15` | `rbx`, `rbp`, `rdi`, `rsi`, `r12` to `r15`, `xmm6` to `xmm15` |

## Stack alignment

`rsp` must be a multiple of 16 at the moment of every `call` in 64-bit code. At the first instruction of a function it is 8 modulo 16, because the return address has just been pushed.

## Recognizing a convention in a binary

- A `ret` followed by a number: stdcall or fastcall.
- `ecx` or `edx` read before they are written, in 32-bit code: fastcall.
- A caller that runs `add esp, N` after a call: cdecl.
- `rdi` and `rsi` read first: System V. `rcx` and `rdx` read first, with writes into `[rsp + 8]` and above: Microsoft x64.
- A caller that runs `sub rsp, 0x28` (or a larger multiple of 8 that keeps alignment) before a call: Microsoft x64 shadow space.
