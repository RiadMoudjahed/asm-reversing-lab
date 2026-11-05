[English](quiz.md) | [العربية](quiz.ar.md) | [Deutsch](quiz.de.md) | [Español](quiz.es.md) | [Français](quiz.fr.md)

# Quiz: 08. The stack, procedures and calling conventions

**1. What does `call target` do, and what does `ret` do?**

<details>
<summary>Answer</summary>

`call` pushes the address of the next instruction and jumps to `target`. `ret` pops an address from the stack into `rip`.

</details>

**2. In a function with the standard frame, where are the first local variable and the return address relative to `rbp`?**

<details>
<summary>Answer</summary>

Locals are at negative offsets, for example `[rbp - 8]`. The saved `rbp` is at `[rbp]` and the return address at `[rbp + 8]`.

</details>

**3. Which registers carry the first four integer arguments in System V AMD64 and in Microsoft x64?**

<details>
<summary>Answer</summary>

System V: `rdi`, `rsi`, `rdx`, `rcx`. Microsoft x64: `rcx`, `rdx`, `r8`, `r9`.

</details>

**4. What is the difference between cdecl and stdcall, and how can you tell them apart in a callee?**

<details>
<summary>Answer</summary>

In cdecl the caller removes the arguments from the stack; in stdcall the callee does. A stdcall callee ends with `ret N`, while a cdecl callee ends with a plain `ret`.

</details>

**5. At the first instruction of a System V function, where is the seventh integer argument?**

<details>
<summary>Answer</summary>

At `[rsp + 8]`. The return address is at `[rsp]`, and the stack arguments follow it.

</details>

**6. Why does `factorial` save and restore `rbx`?**

<details>
<summary>Answer</summary>

It uses `rbx` to keep `n` across the recursive call, and `rbx` is callee-saved, so the original value must be preserved for the caller. The `push` also keeps `rsp` aligned for the call.

</details>

**7. What must `rsp` be at the moment of a `call` in 64-bit code, and how do you fix it in a function that has not pushed anything?**

<details>
<summary>Answer</summary>

A multiple of 16. At function entry `rsp + 8` is a multiple of 16, so subtract 8 (`sub rsp, 8`) before the call, or push one register.

</details>

**8. What is shadow space?**

<details>
<summary>Answer</summary>

In Microsoft x64, 32 bytes that the caller reserves above the return address for every call, where the callee may spill the four register arguments.

</details>

**9. Why can a misaligned stack go unnoticed in one call and crash in another?**

<details>
<summary>Answer</summary>

Only some code paths use instructions that require 16-byte alignment, such as aligned SSE stores. A `printf` call with no vector arguments avoids them; one with a floating-point argument executes them and faults.

</details>
