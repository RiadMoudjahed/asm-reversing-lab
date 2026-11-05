[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# 08. The stack, procedures and calling conventions

Time: about 3 hours. Prerequisites: [07. Strings and string instructions](../07-strings/README.md)

## Why this matters

Almost every program is built from functions that call each other, and the stack is where that structure lives: return addresses, arguments, local variables and saved registers. A calling convention is the agreement that lets separately written code work together. When you reverse engineer a binary, recognizing the convention tells you where each argument is and who cleans up, and recognizing a stack frame tells you how big a function's variables are. When you write assembly that talks to C, getting the convention wrong is the most common cause of crashes.

## Concepts

### The stack

The stack is a region of memory addressed by `rsp` (`esp` in 32-bit code). It grows toward lower addresses: pushing a value moves `rsp` down. Four instructions are defined in terms of it:

| Instruction | Equivalent |
|---|---|
| `push x` | `sub rsp, 8` then store `x` at `[rsp]` |
| `pop x` | Load `x` from `[rsp]` then `add rsp, 8` |
| `call target` | Push the address of the next instruction, then jump to `target` |
| `ret` | Pop an address into `rip` |

`ret` trusts whatever is on top of the stack. If a function leaves `rsp` pointing somewhere other than its return address, it jumps to the wrong place.

### Stack frames

A function that needs local variables carves out space on the stack for them. The classic way keeps a second register, `rbp`, as a fixed reference point:

```nasm-fragment
sum_locals:
    push    rbp                         ; save the caller's frame pointer
    mov     rbp, rsp                    ; this function's frame pointer
    sub     rsp, 32                     ; room for four 8-byte locals
    mov     [rbp - 8], rdi              ; locals live below rbp
    ; ...
    leave                               ; mov rsp, rbp ; pop rbp
    ret
```

The saved `rbp` and the return address sit at `[rbp]` and `[rbp + 8]`. Locals have negative offsets from `rbp`, and arguments that were passed on the stack have positive ones. With optimization compilers often drop the frame pointer and address everything relative to `rsp` instead. Both forms are complete versions of the same idea, and [08_frames.asm](examples/08_frames.asm) contains working examples.

### Calling conventions

A convention decides where the arguments go, where the result comes back, who removes the arguments from the stack, and which registers a function must leave unchanged. These are the five you will meet:

| | cdecl | stdcall | fastcall | System V AMD64 | Microsoft x64 |
|---|---|---|---|---|---|
| Bits | 32 | 32 | 32 | 64 | 64 |
| Typical OS | Linux, Windows | Windows API | Windows | Linux, macOS, BSD | Windows |
| Integer arguments | Stack, right to left | Stack, right to left | `ecx`, `edx`, then stack | `rdi`, `rsi`, `rdx`, `rcx`, `r8`, `r9`, then stack | `rcx`, `rdx`, `r8`, `r9`, then stack |
| Who removes stack arguments | The caller | The callee (`ret N`) | The callee (`ret N`) | The caller | The caller |
| Result | `eax` | `eax` | `eax` | `rax` | `rax` |
| Preserved by the callee | `ebx`, `esi`, `edi`, `ebp` | `ebx`, `esi`, `edi`, `ebp` | `ebx`, `esi`, `edi`, `ebp` | `rbx`, `rbp`, `r12` to `r15` | `rbx`, `rbp`, `rdi`, `rsi`, `r12` to `r15` |
| Extra rules | None | None | None | Red zone of 128 bytes | 32 bytes of shadow space |

A register the callee must preserve is **callee-saved**: a function that wants to use it saves the old value first and restores it before returning. The others are **caller-saved**: a function may overwrite them freely, so a caller that needs a value across a call must keep it elsewhere. Return values for floating-point numbers go in `xmm0`, and floating-point arguments use `xmm` registers in the 64-bit conventions.

### The three 32-bit conventions

[08_conv32.asm](examples/08_conv32.asm) writes one function three ways. The only visible differences are where the arguments are read and how the function returns:

```nasm-fragment
add3_cdecl:     ; a, b, c at [esp+4], [esp+8], [esp+12]
    ; ...
    ret                                 ; the caller removes the arguments

add3_stdcall:   ; the same locations
    ; ...
    ret     12                          ; the callee removes 12 bytes

add3_fastcall:  ; a in ecx, b in edx, c at [esp+4]
    ; ...
    ret     4                           ; the callee removes its 4 bytes
```

`ret N` is the giveaway for stdcall and fastcall: it returns and then adds `N` to `esp`. If the caller and the callee disagree about who cleans up, `esp` drifts after every call.

### The two 64-bit conventions

**System V** (Linux, macOS, BSD) passes the first six integer arguments in `rdi`, `rsi`, `rdx`, `rcx`, `r8` and `r9`. Further arguments are on the stack: at the first instruction of the function, the seventh is at `[rsp + 8]` (just above the return address), the eighth at `[rsp + 16]`. A leaf function (one that makes no calls) may also use the **red zone**, the 128 bytes below `rsp`, without adjusting `rsp`.

**Microsoft x64** passes the first four in `rcx`, `rdx`, `r8` and `r9`. The caller must always reserve **32 bytes of shadow space** above the return address, even for functions with fewer arguments, and the callee may use it to store the register arguments. There is no red zone. [08_win64.asm](examples/08_win64.asm) shows a function and a caller that reserves the shadow space.

### Stack alignment

Both 64-bit conventions require `rsp` to be a multiple of 16 at the moment of a `call`. Because the call itself pushes 8 bytes, at the first instruction of a function `rsp + 8` is a multiple of 16. Inside a function that makes calls, you keep this true by making the total amount you subtract an odd multiple of 8: a single `push` (as `factorial` does with `rbx`) or a `sub rsp, 8` (as `length_of` does) restores alignment. The reason is that some instructions, such as the aligned SSE stores that library code uses to save floating-point registers, fault on a misaligned address.

### Calling C from assembly, and the reverse

To call a C library function, declare it with `extern`, put the arguments in the registers of the convention, keep `rsp` aligned, and `call` it. For a variadic function such as `printf`, `al` must hold the number of vector registers used by the arguments; set it to 0 with `xor eax, eax` when none are. [08_printf.asm](examples/08_printf.asm) is a complete program:

```nasm-fragment
main:
    sub     rsp, 8                      ; realign: main was entered with rsp + 8 aligned
    lea     rdi, [fmt]
    mov     esi, 42
    xor     eax, eax                    ; no vector registers used
    call    printf wrt ..plt
```

In the other direction, a function that C should call is marked `global`, and C declares it with a normal prototype. The Makefile links the assembled object files with the C program using `gcc`.

### Macros and several source files

NASM macros write repetitive code for you. [08_frame.inc](examples/08_frame.inc) defines `FUNC_ENTER` and `FUNC_LEAVE`, and [08_macros.asm](examples/08_macros.asm) includes the file with `%include` and uses them. Each `.asm` file is assembled separately into an object file, and the linker combines them: `global` exports a symbol and `extern` imports one.

### The reversing view

Run `make compiled` in the examples folder to compare the same function compiled without and with optimization. These are real excerpts:

```text
== gcc -O0
<with_locals>:
    push   rbp
    mov    rbp,rsp
    sub    rsp,0x40
    mov    QWORD PTR [rbp-0x38],rdi          ; arguments are stored in the frame
    mov    QWORD PTR [rbp-0x40],rsi
    mov    rax,QWORD PTR fs:0x28             ; stack protector: a canary value
    mov    QWORD PTR [rbp-0x8],rax
    ...
    call   <helper>
    ...
    sub    rdx,QWORD PTR fs:0x28             ; the canary is checked before returning
    leave
    ret

== gcc -O2
<with_locals>:
    lea    rax,[rdi+rsi*1]
    push   rbx                               ; a callee-saved register is used, so it is saved
    mov    rbx,rsi
    imul   rdi,rsi
    add    rdi,rax
    call   <helper>
    add    rax,rbx
    pop    rbx                               ; and restored
    ret
```

What to read from this:

- **Prologue and epilogue.** `push rbp` and `mov rbp, rsp` at the start, `leave` and `ret` at the end, mark a classic frame. The number in `sub rsp, N` is the size of the locals plus padding.
- **No frame pointer.** In optimized code the frame disappears. A `push` of a callee-saved register at the start and the matching `pop` before `ret` is what remains. The array of four locals is gone entirely, kept in registers.
- **A canary.** A load from `fs:0x28` at the start and a compare with it before the epilogue is a stack protector. It is a security feature, not part of the program's logic.
- **The convention.** Arguments read from `rdi`, `rsi`, `rdx`, `rcx`, `r8`, `r9` mean System V. Reads from `rcx`, `rdx`, `r8`, `r9` followed by stores into `[rsp + 8]`, `[rsp + 16]` and so on (shadow space) mean Microsoft x64. `ret N` means stdcall or fastcall, and `ecx` and `edx` read before they are written mean fastcall.
- **Recursion and the stack.** Each level pushes a return address and often a saved register, so a recursive function leaves a repeating pattern on the stack.

## Try it

Build and run the examples:

```bash
cd modules/08-stack-and-calls/examples
make run
```

Expected output:

```text
== System V (64-bit)
sum3(1, 2, 3)                = 6
sum_locals(5)                = 50
factorial(10)                = 3628800
weighted8(1,1,1,1,1,1,1,1)   = 36
weighted8(1,2,3,4,5,6,7,8)   = 204
apply_twice(add_ten, 1)      = 21
length_of("stack frame")     = 11
macro_demo(14)               = 43
== assembly calling printf
The answer is 42
== 32-bit conventions
cdecl:    add3(1, 2, 3) = 123, stack balanced
stdcall:  add3(1, 2, 3) = 123, stack balanced
fastcall: add3(1, 2, 3) = 123, stack balanced
== Microsoft x64 under Wine
ms_sum4(9, 8, 7, 6)  = 9876
ms_call_sum4()       = 1234
```

The three 32-bit lines return the same value with a balanced stack, although the functions clean up differently. `weighted8(1,2,3,4,5,6,7,8)` is 1 + 4 + 9 + 16 + 25 + 36 + 49 + 64 = 204, which proves that the seventh and eighth arguments are read from the right stack slots.

Now watch the stack during recursion. Start the debugger and stop in the fourth call of `factorial`:

```bash
gdb -q build/08_main
```

```text
break factorial
run
continue 3
p $rdi
x/8gx $rsp
```

`rdi` is 7. The stack, from `rsp` upward, holds this pattern (the return addresses on your machine may differ, the structure does not):

```text
0x...e8:  0x0000000000401305  0x0000000000000009
0x...f8:  0x0000000000401305  0x000000000000000a
0x...08:  0x0000000000401305  0x00007ffd...48
0x...18:  0x00000000004011aa  ...
```

Read it in pairs. The first value of a pair is a return address into `factorial` (the instruction after the recursive `call`), and the second is the `rbx` that the caller saved with `push rbx`: 9, then 10, which are the values of `n` in the two frames above. The third pair holds the `rbx` that existed before `factorial(10)` started, and the last row starts with the return address into `main`. The whole call chain is visible in memory.

Then change something and predict the result before you rebuild:

1. In `factorial`, remove the `push rbx` and `pop rbx` and use `rdi` after the call instead. Why does the result break?
2. In `08_printf.asm`, remove `sub rsp, 8` and `add rsp, 8`. Does the program crash? (The next section explains what you see.)

## Common pitfalls

- **A misaligned stack.** Removing the realignment from `08_printf.asm` does not crash it, because `printf` with no vector arguments never uses an aligned store. The same mistake with a floating-point argument (`al` set to 1) crashes with a segmentation fault inside `printf`, at a `movaps`. Misalignment can lie dormant for a long time: align every call.
- **Cleanup mismatch.** Calling a stdcall function as if it were cdecl, or the reverse, leaves `esp` wrong after every call. The program may seem to work until the stack drifts far enough to matter.
- **Forgetting to save a callee-saved register.** If a function changes `rbx` and does not restore it, its caller's data silently changes.
- **Keeping a value in a caller-saved register across a call.** `rax`, `rcx`, `rdx`, `rsi`, `rdi` and `r8` to `r11` can be changed by any function you call.
- **Using the red zone in a function that makes calls.** The call pushes a return address into the red zone. Only leaf functions may use it, and only in System V.
- **Wrong offsets for stack arguments.** At entry the seventh argument is at `[rsp + 8]`, not `[rsp]`, because the return address is on top.
- **Forgetting the shadow space on Windows.** The callee may write to it. A caller that did not reserve it has its stack overwritten.
- **Forgetting `al` for a variadic call.** Set it to the number of vector registers used, or to 0.

## Summary

- The stack grows downward; `call` pushes a return address and `ret` pops it, and a frame holds locals, saved registers and the saved `rbp`.
- A calling convention fixes where arguments and results go, who cleans the stack, and which registers must be preserved.
- cdecl, stdcall and fastcall differ in argument placement and in who removes the arguments (`ret N`); System V uses six integer registers and a red zone; Microsoft x64 uses four registers and 32 bytes of shadow space.
- Keep `rsp` a multiple of 16 at every `call`; use a `push` or a `sub rsp, 8` to fix it.
- A prologue, an epilogue, a canary, a pattern of pushes and pops, and the registers an argument is read from identify the convention and the frame in compiled code.

## Next steps

- Take the [quiz](quiz.md).
- Continue with module 09, what reverse engineering is.
- Exercises that use this module: [Exercise 08](../../exercises/08-calling-card/README.md), [Exercise 09](../../exercises/09-stack-archaeology/README.md), [Exercise 11](../../exercises/11-portable-puzzle/README.md).
