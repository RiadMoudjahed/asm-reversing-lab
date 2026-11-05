[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# 13. Recognizing code structures

Time: about 3 hours. Prerequisites: [12. Executable formats: ELF and PE](../12-executable-formats/README.md)

## Why this matters

A compiler turns loops, conditions and function calls into a small set of instruction patterns. Once you recognize the patterns, a page of assembly becomes a few lines of intent, and you read code at the speed of the compiler's output rather than one instruction at a time. This module teaches the shapes: function entry and exit, locals, `if`, short-circuit logic, every common loop form, `switch`, inlining and tail calls. It compares the same C code compiled without optimization and with it, because you will meet both.

## Concepts

### How the listings in this module were made

The C source is `examples/13_structs.c`. The `Makefile` compiles it with GCC at `-O0`, `-O1` and `-O2` and writes Intel-syntax assembly to `build/13_O0.s`, `build/13_O1.s` and `build/13_O2.s`. The listings below are those files with the assembler directives removed. Labels such as `.L11` are the compiler's names for jump targets; in a binary they are plain addresses, and you must find the targets yourself. All code is x86-64 System V, compiled with `-fcf-protection=none -fno-stack-protector` so that no extra instructions hide the structure.

```bash
make
```

### Function entry and exit

**Without optimization** every function builds a frame with the frame pointer `rbp`. Arguments arrive in registers but are copied into the frame, and every variable lives in memory.

```text
sum3:
    push    rbp
    mov    rbp, rsp
    mov    DWORD PTR [rbp-20], edi
    mov    DWORD PTR [rbp-24], esi
    mov    DWORD PTR [rbp-28], edx
    mov    edx, DWORD PTR [rbp-20]
    mov    eax, DWORD PTR [rbp-24]
    add    eax, edx
    mov    DWORD PTR [rbp-4], eax
    mov    edx, DWORD PTR [rbp-4]
    mov    eax, DWORD PTR [rbp-28]
    add    eax, edx
    pop    rbp
    ret
```

The first two instructions, `push rbp` and `mov rbp, rsp`, are the **prologue**. They are matched at the end by `pop rbp` (or `leave`) and `ret`, the **epilogue**. The three `mov DWORD PTR [rbp-N], reg` instructions save the arguments `a`, `b` and `c` into the frame. The variable `total` is at `[rbp-4]`. The reads and writes to memory are why this code is long.

**With optimization** the same function needs no frame at all:

```text
sum3:
    add    edi, esi
    lea    eax, [rdi+rdx]
    ret
```

The arguments stay in `edi`, `esi` and `edx`, the result is built with `add` and `lea`, and there is no prologue. A function that makes no calls and needs no memory for locals is a **leaf function**, and such functions often have neither prologue nor epilogue.

**A function that calls others** must preserve values across the calls. At `-O2`, GCC does not use `rbp` as a frame pointer; it saves the callee-saved registers it needs and adjusts `rsp`:

```text
two_calls:
    push    rbp
    push    rbx
    mov    ebx, edi
    sub    rsp, 8
    call    act_a
    lea    edi, [rbx+1]
    mov    ebp, eax
    call    act_b
    add    rsp, 8
    add    eax, ebp
    pop    rbx
    pop    rbp
    ret
```

Read this as the frame you reconstructed in exercise 09: two `push` instructions for `rbp` and `rbx`, a `sub rsp, 8` that keeps `rsp` aligned at the calls, and the same registers popped in reverse order at the end. Here `rbx` holds `v` and `rbp` holds the first result across the second call. The `sub rsp, 8` has no local variable to hold; it exists only for alignment.

| Pattern | What it tells you |
|---|---|
| `push rbp; mov rbp, rsp` ... `pop rbp` or `leave` | A frame pointer; likely unoptimized code or a function with a variable-size stack area |
| Pushes of `rbx`, `rbp`, `r12` to `r15` at the start | Values that live across calls |
| `sub rsp, N` | Space for locals, saved temporaries or alignment |
| No prologue, ends in `ret` | A leaf function |
| Ends in `jmp` to another function | A tail call (see below) |

### Locals and parameters

At `-O0`, a parameter is stored once on entry and then read from memory each time, and a local variable is a negative offset from `rbp`: `[rbp-4]`, `[rbp-8]`. Variables get distinct slots in the order the compiler allocates them. In `sum3`, `a` is at `[rbp-20]`, `b` at `[rbp-24]` and `c` at `[rbp-28]` (in hexadecimal `-0x14`, `-0x18` and `-0x1c`, as `objdump` writes them), and `total` is at `[rbp-4]`.

At `-O1` and above, most variables live in registers and have no address at all. A variable only gets a memory slot when its address is taken, when it is an array or structure, or when the compiler runs out of registers. When you see a stack slot at `-O2`, ask why: it is often an array, a structure or a value whose address was passed to another function, as in exercise 09.

### If and else

A condition compiles to a comparison followed by a conditional jump. The jump **skips** the code that runs when the condition is true, so the jump condition is the **opposite** of the C condition.

```text
sign_text:
    push    rbp
    mov    rbp, rsp
    mov    DWORD PTR [rbp-4], edi
    cmp    DWORD PTR [rbp-4], 0
    jle    .L7
    mov    eax, 1
    jmp    .L8
.L7:
    cmp    DWORD PTR [rbp-4], 0
    jns    .L9
    mov    eax, -1
    jmp    .L8
.L9:
    mov    eax, 0
.L8:
    pop    rbp
    ret
```

This is `sign_text`: `if (x > 0) return 1; else if (x < 0) return -1; else return 0;`. The first test is `x > 0`, but the jump is `jle`, which skips the `return 1` block when the condition is false. In the same way `x < 0` is tested with `jns`. Every `jmp .L` to the epilogue is a `return`. The chain `cmp / jcc / body / jmp end` repeats for each branch.

Optimized code often avoids the branch entirely. Two functions at `-O2`:

```text
clamp_low:
    cmp    edi, esi
    mov    eax, esi
    cmovge    eax, edi
    ret
```

```text
pick:
    mov    edx, edi
    mov    eax, esi
    sub    edx, esi
    sub    eax, edi
    cmp    edi, esi
    cmovg    eax, edx
    ret
```

`clamp_low` returns `x` or `low`, whichever is larger: `cmp` sets the flags and `cmovge` copies one value into the result only if the condition holds. A **conditional move** (`cmov`) is how the compiler writes `a ? b : c` without a jump when both sides are cheap and safe to compute. In `pick`, both `a - b` and `b - a` are computed first, and `cmovg` selects one. If you see two arithmetic results computed side by side and a `cmov`, you are looking at a ternary expression or a small `if`.

### Short-circuit logic

C evaluates `a && b` left to right and does not evaluate `b` when `a` is false. The compiled code shows this as a separate comparison and jump for each operand, all jumping to the same place.

```text
in_range:
    push    rbp
    mov    rbp, rsp
    mov    DWORD PTR [rbp-4], edi
    mov    DWORD PTR [rbp-8], esi
    mov    DWORD PTR [rbp-12], edx
    mov    eax, DWORD PTR [rbp-4]
    cmp    eax, DWORD PTR [rbp-8]
    jl    .L15
    mov    eax, DWORD PTR [rbp-4]
    cmp    eax, DWORD PTR [rbp-12]
    jg    .L15
    mov    eax, 1
    jmp    .L17
.L15:
    mov    eax, 0
.L17:
    pop    rbp
    ret
```

`in_range` is `x >= lo && x <= hi`. The first test `x < lo` (the opposite of `x >= lo`) jumps to `.L15`, the "result is 0" block; the second test `x > hi` jumps to the same block. Only if neither jump is taken does the code reach `mov eax, 1`. Two conditional jumps to the same false label means `&&`.

```text
either:
    push    rbp
    mov    rbp, rsp
    mov    DWORD PTR [rbp-4], edi
    mov    DWORD PTR [rbp-8], esi
    cmp    DWORD PTR [rbp-4], 0
    je    .L19
    cmp    DWORD PTR [rbp-8], 0
    jne    .L20
.L19:
    mov    eax, 1
    jmp    .L22
.L20:
    mov    eax, 0
.L22:
    pop    rbp
    ret
```

For `a == 0 || b == 0` the pattern is different: the first jump goes to the **true** block (`je .L19`), and the last test jumps to the false block. A conditional jump to the true label followed by a test that falls into true means `||`. In short:

| Shape | Source |
|---|---|
| Several `jcc` to one "false" label, success falls through | A chain of conditions joined with `&&` |
| `jcc` to a "true" label, then the last test jumps away on false | Conditions joined with the OR operator |

At `-O2`, the compiler may remove the jumps and combine the results with `setcc` and `and`/`or`. This is allowed when the second operand has no side effects and cannot fault.

```text
in_range:
    mov    eax, edx
    cmp    edi, esi
    setge    dl
    cmp    edi, eax
    setle    al
    movzx    eax, al
    and    eax, edx
    ret
```

### Loops

Every loop has an initialization, a test, a body and an update, and some conditional jump goes backward. **A backward conditional jump is the signature of a loop.**

At `-O0`, a `for` loop is laid out with the test at the bottom:

```text
count_up:
    push    rbp
    mov    rbp, rsp
    mov    DWORD PTR [rbp-20], edi
    mov    DWORD PTR [rbp-4], 0
    mov    DWORD PTR [rbp-8], 0
    jmp    .L24
.L25:
    mov    eax, DWORD PTR [rbp-8]
    add    DWORD PTR [rbp-4], eax
    add    DWORD PTR [rbp-8], 1
.L24:
    mov    eax, DWORD PTR [rbp-8]
    cmp    eax, DWORD PTR [rbp-20]
    jl    .L25
    mov    eax, DWORD PTR [rbp-4]
    pop    rbp
    ret
```

The code first sets `total` and `i` to zero (`[rbp-4]` and `[rbp-8]`), then jumps to the test at `.L24`. The body is at `.L25`; it adds `i` to `total` and increments `i`. The test compares `i` with `n` (at `[rbp-20]`) and jumps back with `jl .L25` while `i < n`. Jumping first to the test is why you see a `jmp` over the body at the start.

At `-O1`, the same function is **rotated**:

```text
count_up:
    test    edi, edi
    jle    .L14
    mov    eax, 0
    mov    edx, 0
.L13:
    add    edx, eax
    add    eax, 1
    cmp    edi, eax
    jne    .L13
.L11:
    mov    eax, edx
    ret
.L14:
    mov    edx, 0
    jmp    .L11
```

The compiler checks once whether the loop runs at all (`test edi, edi / jle`) and then places the test at the bottom of the body, so each iteration costs one conditional jump. `edx` is `total` and `eax` is `i`. Note that the loop counter runs from 0 up and the exit test is `cmp edi, eax / jne`: the compiler knew the loop stops when `i` reaches `n`, so it uses `jne` rather than `jl`. Optimized loops can have a different exit condition from the one you read in the source.

A `while` loop over a string looks like this at `-O1`:

```text
count_while:
    lea    rcx, [rdi+1]
    cmp    BYTE PTR [rdi], 0
    je    .L19
    mov    rax, rcx
.L18:
    mov    rdx, rax
    add    rax, 1
    cmp    BYTE PTR [rax-1], 0
    jne    .L18
    sub    edx, ecx
    lea    eax, [rdx+1]
    ret
.L19:
    mov    eax, 0
    ret
```

The pointer `rax` walks through the string and the loop ends when the byte it just read is zero (`cmp BYTE PTR [rax-1], 0 / jne`). The count is not a separate counter: after the loop the compiler computes the length as the difference between the end pointer and the start pointer (`sub edx, ecx` then `lea eax, [rdx+1]`). Whenever a counter is replaced by pointer arithmetic, the compiler is **strength-reducing** the loop.

A `do { } while` loop runs the body at least once, so it has no guard before the first iteration:

```text
do_once:
    mov    eax, 0
.L22:
    mov    edx, edi
    mov    ecx, edi
    shr    ecx, 31
    add    edi, ecx
    sar    edi
    add    eax, 1
    cmp    edx, 1
    jg    .L22
    ret
```

The body is executed first, and the backward jump `jg .L22` is the only test. Inside the body, `n / 2` for a signed value is the four-instruction idiom `mov / shr 31 / add / sar`: the sign bit is added before the shift so that the division rounds toward zero. The next module explains why.

| Loop form | Layout |
|---|---|
| `for`/`while`, unoptimized | `jmp test` at the start, body, update, test at the bottom with a backward `jcc` |
| `for`/`while`, optimized | A guard that skips the loop, then the body, then the test with a backward `jcc` |
| `do`/`while` | The body first; the only test is the backward `jcc` |
| Infinite loop | An unconditional backward `jmp` with an exit inside the body |

### Switch

A `switch` can compile in four ways, depending on the case values and the optimization level. Learn to tell them apart by the instructions that precede the jumps.

**A compare chain** for a few sparse cases:

```text
sparse:
    mov    eax, 2
    cmp    edi, 400
    je    .L35
    mov    eax, 3
    cmp    edi, 9000
    je    .L35
    xor    eax, eax
    cmp    edi, 3
    sete    al
.L35:
    ret
```

The first two `cmp` and `je` pairs test the case values 400 and 9000, and the last case (3) is turned into a `sete`. With a handful of cases, a chain is as cheap as anything else. At `-O0` the same code uses `cmp / je` pairs in a different order, but the shape is the same.

**A jump table** for dense case values, each doing something different:

```text
dispatch:
    mov    eax, edi
    mov    edi, esi
    cmp    eax, 4
    ja    .L44
    jmp    [QWORD PTR .L46[0+rax*8]]
.L46:
    .quad    .L50
    .quad    .L49
    .quad    .L48
    .quad    .L47
    .quad    .L45
.L47:
    jmp    act_d
.L45:
    jmp    act_e
.L50:
    jmp    act_a
.L49:
    jmp    act_b
.L48:
    jmp    act_c
.L44:
    mov    eax, -1
    ret
```

`cmp eax, 4 / ja .L44` is the **bounds check**: if the value is above the largest case, go to `default`. The next instruction is the indirect `jmp [QWORD PTR .L46[0+rax*8]]`, which reads an 8-byte address from a table and jumps through it. The table `.L46` lives in `.rodata` and holds five addresses, one per case. The targets here are `jmp act_a` and so on: each case is a tail call. In a binary, you see `cmp reg, N / ja default / jmp qword ptr [reg*8 + table]`. To recover the cases, read the table from the file: the entries are the case targets in order, and `N` + 1 is the number of entries.

At `-O0` the same pattern appears with a `mov rax, QWORD PTR .L37[0+rax*8]` and `jmp rax`:

```text
dense:
    push    rbp
    mov    rbp, rsp
    mov    DWORD PTR [rbp-4], edi
    cmp    DWORD PTR [rbp-4], 5
    ja    .L35
    mov    eax, DWORD PTR [rbp-4]
    mov    rax, QWORD PTR .L37[0+rax*8]
    jmp    rax
.L37:
    .quad    .L42
    .quad    .L41
    .quad    .L40
    .quad    .L39
    .quad    .L38
    .quad    .L36
.L42:
    mov    eax, 10
    jmp    .L43
.L41:
    mov    eax, 20
    jmp    .L43
.L40:
    mov    eax, 35
    jmp    .L43
.L39:
    mov    eax, 41
    jmp    .L43
.L38:
    mov    eax, 52
    jmp    .L43
.L36:
    mov    eax, 63
    jmp    .L43
.L35:
    mov    eax, -1
.L43:
    pop    rbp
    ret
```

**A lookup table**, when every case only returns a constant:

```text
dense:
    mov    eax, -1
    cmp    edi, 5
    ja    .L32
    mov    edi, edi
    mov    eax, DWORD PTR CSWTCH.13[0+rdi*4]
.L32:
    ret
```

The optimizer saw that `dense` returns a constant for each value, so it replaced the whole `switch` with an array of results (`CSWTCH.13`) and one load. There is a bounds check, but no indirect jump. You will find the array in `.rodata`: 10, 20, 35, 41, 52, 63.

**A binary search** for many sparse cases:

```text
many:
    cmp    edi, 400
    je    .L52
    jg    .L53
    cmp    edi, 77
    je    .L54
    jg    .L55
    cmp    edi, 3
    je    .L56
    cmp    edi, 40
    jne    .L51
    mov    edi, 40
    jmp    act_b
.L53:
    cmp    edi, 2000
    je    .L60
    cmp    edi, 9000
    je    .L61
    cmp    edi, 900
    jne    .L51
    mov    edi, 901
    jmp    act_a
.L56:
    mov    edi, 3
    jmp    act_a
.L61:
    mov    edi, 9001
    jmp    act_c
.L55:
    cmp    edi, 150
    jne    .L51
    mov    edi, 150
    jmp    act_d
.L52:
    mov    edi, 400
    jmp    act_e
.L54:
    mov    edi, 77
    jmp    act_c
.L60:
    mov    edi, 2001
    jmp    act_b
.L51:
    xor    eax, eax
    ret
```

The compiler compares against the middle value (400), then branches below or above it, halving the candidates at each step. A tree of `cmp / je / jg` instructions where the constants are in sorted order, descending through several levels, is a switch compiled as a binary search. Sort the constants to get the case list.

| Evidence | What it is |
|---|---|
| Several `cmp reg, constant / je` in a row | A compare chain |
| `cmp reg, N / ja default` then an indirect `jmp` through `[reg*8 + table]` | A jump table with N + 1 entries |
| `cmp reg, N / ja default` then a load from an array | A lookup table |
| `cmp` with a middle constant followed by `je` and `jg`, repeated | A binary search |

### Inlined functions

An optimizing compiler may paste a small function's body into its callers. The call, the arguments and the return disappear, and the code is mixed with the surroundings.

```text
uses_helper:
    push    rbp
    mov    rbp, rsp
    push    rbx
    sub    rsp, 8
    mov    DWORD PTR [rbp-12], edi
    mov    eax, DWORD PTR [rbp-12]
    mov    edi, eax
    call    helper
    mov    ebx, eax
    mov    eax, DWORD PTR [rbp-12]
    add    eax, 1
    mov    edi, eax
    call    helper
    add    eax, ebx
    mov    rbx, QWORD PTR [rbp-8]
    leave
    ret
```

At `-O0` `uses_helper` calls `helper` twice. At `-O2` the calls are gone:

```text
uses_helper:
    lea    eax, [rdi+rdi*2]
    lea    eax, [rax+5+rax]
    ret
```

The expression `helper(v) + helper(v + 1)` where `helper(x) = x * 3 + 1` was simplified algebraically to `6v + 5`, and computed with two `lea` instructions. No call remains, and no function named `helper` exists in the output. Signs of inlining: the same instruction pattern repeated in several functions, and a function that does work which clearly belongs to a separate routine. The reverser's task is to find the shape, name it, and move on.

### Tail calls

When a function ends with a call whose result it returns unchanged, an optimizing compiler can replace `call` and `ret` with a `jmp`:

```text
tail:
    add    edi, 1
    jmp    forwards
```

`tail` increments its argument and **jumps** to `forwards`; `forwards` will return directly to `tail`'s caller. Seeing a `jmp` to another function at the end of a function, rather than to a label inside it, means a tail call. In the jump table above, every case ends this way. Tail calls hide the caller from stack traces and from the call graph, so a function can appear to have no callers even though it is used.

## Try it

1. Run `make` and open `build/13_O0.s` and `build/13_O2.s`. For each function in `13_structs.c`, predict the shape before you read the listing, then compare.
2. Compile `sparse` and `dense` with `-O1`. Which strategy does each use?
3. Add a function `int max3(int a, int b, int c)` to `13_structs.c`, rebuild, and find the `cmp` and `cmov` instructions at `-O2`.
4. In `build/13_O2.s`, find the table `.L46` used by `dispatch` and match each entry to the case it serves.
5. Compile `modules/08-stack-and-calls/examples/08_compiled.c` with `gcc -O0 -S -masm=intel` and with `-O2`, and classify the prologue and the locals of each version.

## Common pitfalls

- **Reading the jump condition as the source condition.** The compiler usually jumps over the code that runs when the condition is true, so the jump tests the opposite.
- **Misreading signed and unsigned jumps.** `jg`, `jl`, `jge`, `jle` are signed; `ja`, `jb`, `jae`, `jbe` are unsigned. The choice tells you the variable's type.
- **Looking for the source loop bound.** An optimized loop may count down, count by a pointer or end on a different test than the source.
- **Assuming every call is in the source.** Inlining removes calls; compilers also add calls to helper routines.
- **Treating a `jmp` to a function as a jump inside it.** It is a tail call.

## Summary

- A prologue with `push rbp; mov rbp, rsp` means a frame pointer; optimized code usually has none and saves only the registers it needs.
- A conditional jump skips the code that runs when the condition is true. `cmov` and `setcc` replace small branches.
- `&&` and `||` appear as several conditional jumps to a shared label; optimized code may turn them into `setcc` and logic.
- A backward conditional jump is a loop. Optimized loops are rotated, may use pointers instead of counters, and may exit on a different test.
- A `switch` is a compare chain, a jump table, a lookup table or a binary search; the instructions before the jump tell you which.
- Inlining removes calls, and tail calls turn `call; ret` into `jmp`.

## Next steps

- Take the [quiz](quiz.md).
- Continue with module 14, compiled arithmetic.
- Cheatsheets: [Compiler idioms](../../cheatsheets/compiler-idioms.md).
- Exercises that use this module: [Exercise 09](../../exercises/09-stack-archaeology/README.md), [Exercise 12](../../exercises/12-switchboard/README.md), [Exercise 20](../../exercises/20-capstone-tiny-vm/README.md).
