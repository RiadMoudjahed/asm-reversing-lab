[English](quiz.md) | [العربية](quiz.ar.md) | [Deutsch](quiz.de.md) | [Español](quiz.es.md) | [Français](quiz.fr.md)

# Quiz: 13. Recognizing code structures

Answer first, then open the answer.

**1. What is a leaf function, and why does optimized code for one often have no prologue?**

<details>
<summary>Answer</summary>

A leaf function makes no calls. It needs no saved registers across calls and, if it needs no memory for locals, no frame, so the compiler can leave out the prologue and epilogue entirely.

</details>

**2. The source says `if (x > 0) { A }`. The listing has `cmp` followed by `jle`. Where does `jle` jump, and why that condition?**

<details>
<summary>Answer</summary>

It jumps over block A to the code after the `if`. The compiler jumps when the condition is false, so the jump tests the opposite of `x > 0`.

</details>

**3. What is the signature of a loop, and how does an optimized `for` loop differ in layout from an unoptimized one?**

<details>
<summary>Answer</summary>

A backward conditional jump. The unoptimized loop starts with a `jmp` to a test at the bottom. The optimized loop checks once whether the loop runs, then has the body followed by the test at the bottom, so each iteration costs one jump.

</details>

**4. Two `cmp` and conditional jumps in a row, both going to the same label that sets the result to 0. Which source construct is this?**

<details>
<summary>Answer</summary>

A condition with `&&`: the code reaches the success path only if neither test fails.

</details>

**5. How do you tell a jump table from a compare chain, and where do you find the case targets?**

<details>
<summary>Answer</summary>

A jump table has a bounds check (`cmp reg, N / ja default`) followed by an indirect `jmp` through `[reg*8 + table]`. The table is in `.rodata` and holds one address per case value, in order, with `N` + 1 entries. A compare chain is a series of `cmp` and `je` pairs with no indirect jump.

</details>

**6. A function ends with `jmp other_function` instead of `call other_function; ret`. What is this, and what does it do to a call graph?**

<details>
<summary>Answer</summary>

A tail call. The callee returns directly to the caller's caller, so the function does not appear in stack traces and the call graph can show it as having no callers.

</details>

**7. In optimized code you find `cmp edi, esi`, then `mov eax, esi`, then `cmovge eax, edi`. What C expression is this?**

<details>
<summary>Answer</summary>

A maximum of two signed values: `a >= b ? a : b`, where `a` is in `edi` and `b` is in `esi`.

</details>
