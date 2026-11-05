[English](WRITEUP.md) | [العربية](WRITEUP.ar.md) | [Deutsch](WRITEUP.de.md) | [Español](WRITEUP.es.md) | [Français](WRITEUP.fr.md)

# Write-up: exercise 08. Calling card

The flag is `ARL{calling_scfcmv}`: `f1` is stdcall, `f2` is cdecl, `f3` is fastcall, `f4` is cdecl, `g1` is Microsoft x64 and `g2` is System V AMD64. Each convention leaves a signature in two places: how the function reads its arguments, and how the stack is balanced after the call.

## Part 1: Reverse

### The 32-bit functions

```text
f1:  mov eax,[esp+4]       f2:  mov eax,[esp+0xc]
     lea eax,[eax+eax*2]        imul eax,eax,0x64
     mov edx,[esp+8]            mov edx,[esp+8]
     lea eax,[eax+edx*2]        lea edx,[edx+edx*4]
     add eax,[esp+0xc]          lea eax,[eax+edx*2]
     ret 0xc                    add eax,[esp+4]
                                ret

f3:  lea eax,[ecx+ecx*4]   f4:  mov eax,[esp+4]
     sub eax,edx                imul eax,[esp+8]
     add eax,[esp+4]            ret
     ret 4
```

| Function | Evidence | Convention |
|---|---|---|
| `f1` | Three stack arguments at `[esp+4]`, `[esp+8]`, `[esp+0xc]`; ends in `ret 0xc`, so the callee removes them; the caller does not touch `esp` after the call | stdcall |
| `f2` | Three stack arguments; plain `ret`; the caller runs `add esp,0xc` after the call | cdecl |
| `f3` | Reads `ecx` and `edx` without writing them first; one stack argument at `[esp+4]`; ends in `ret 4`; the caller loads `ecx` and `edx` and pushes one value | fastcall |
| `f4` | Two stack arguments; plain `ret`; the caller runs `add esp,8` after the call | cdecl |

The callers in `_start` confirm each reading. After `call f1` there is no stack adjustment, after `call f2` there is `add esp,0xc`, before `call f3` only one `push` is executed together with two register loads, and after `call f4` there is `add esp,8`. Three of the functions take only stack arguments, so the argument locations alone cannot separate stdcall from cdecl; the `ret` is the deciding instruction.

The formulas, read from the code: `f1(a, b, c) = 3a + 2b + c`, `f2(a, b, c) = a + 10b + 100c` and `f3(a, b, c) = 5a - b + c` with `a` in `ecx` and `b` in `edx`.

### The 64-bit functions

```text
g1:  mov [rsp+8],rcx        g2:  lea rax,[rsi+rsi]
     mov [rsp+0x10],rdx          add rax,rdi
     mov rax,[rsp+8]             lea rdx,[rdx+rdx*2]
     imul rax,rax,7              add rax,rdx
     add rax,[rsp+0x10]          ret
     add rax,r8
     ret
```

| Function | Evidence | Convention |
|---|---|---|
| `g1` | Arguments arrive in `rcx`, `rdx` and `r8`; the function stores `rcx` and `rdx` into the 32 bytes above its return address (the shadow space the caller reserved); the caller runs `sub rsp,0x28` before the call | Microsoft x64 |
| `g2` | Arguments arrive in `rdi`, `rsi` and `rdx`; no shadow space, no stack adjustment in the caller | System V AMD64 |

In the order `f1`, `f2`, `f3`, `f4`, `g1`, `g2` the letters are `s`, `c`, `f`, `c`, `m`, `v`.

The flag is `ARL{calling_scfcmv}`.

## Part 2: Assemble

The reference solution is `solution.asm`. The points that matter:

- **`cc_f1` (stdcall).** The three arguments are at `[esp+4]`, `[esp+8]` and `[esp+12]`. The result is `3a + 2b + c`, computed with `lea` and one `add`. The function ends in `ret 12`.
- **`cc_f2` (cdecl).** The result is `a + 10b + 100c`. The function ends in a plain `ret`, because the caller removes the arguments.
- **`cc_f3` (fastcall).** `a` is in `ecx` and `b` is in `edx`; only `c` is on the stack, at `[esp+4]`. The result is `5a - b + c`. The function ends in `ret 4`.

All arithmetic wraps in 32 bits, which `lea`, `add` and `imul` do without extra code. No callee-saved register is touched, so nothing needs saving.

## Mistakes that cost time

- **Using `ret` where a number is needed.** The result is correct, but the stack pointer differs after the call; the checker reports `stack` for every case.
- **Using `ret N` in the cdecl function.** The caller then removes the arguments a second time; the stack pointer moves and the program typically ends with signal 11.
- **Forgetting `ret 4` in the fastcall function.** The pushed argument stays on the stack; the checker reports `stack`.
- **Reading the fastcall arguments from the stack.** Only `c` is there. With `a` and `b` taken from the wrong slots, 18 of the 27 test vectors fail.
- **Mixing up `b` and `c` in the cdecl formula.** The weights 10 and 100 are not symmetric, so only inputs with different `b` and `c` expose it.
