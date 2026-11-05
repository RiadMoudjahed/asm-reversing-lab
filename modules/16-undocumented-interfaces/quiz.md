[English](quiz.md) | [العربية](quiz.ar.md) | [Deutsch](quiz.de.md) | [Español](quiz.es.md) | [Français](quiz.fr.md)

# Quiz: 16. Undocumented interfaces

Answer first, then open the answer.

**1. A function reads `edi`, `rsi` and `edx` before writing them, and ends with `mov eax, ...`. What is the most you can say about its signature?**

<details>
<summary>Answer</summary>

Three parameters: a 32-bit value, a 64-bit value (a pointer or a `size_t`) and a 32-bit value, in that order, and a 32-bit result. Whether the 32-bit values are signed depends on how they are compared or extended, and whether `rsi` is a pointer depends on whether it is dereferenced.

</details>

**2. The code sets `rax` with `mov rax, 0xffffffffffffffff` on its failure paths. What does it tell you?**

<details>
<summary>Answer</summary>

The result is a 64-bit signed value, such as `long`, `ssize_t` or a pointer-sized integer, and -1 is an error code. A 32-bit `int` would use `mov eax, -1` or `or eax, 0xffffffff`.

</details>

**3. How do you recognize an output parameter?**

<details>
<summary>Answer</summary>

A pointer argument that the function only writes through: stores to `[reg]` or `[reg+N]` and no loads from it before the stores. The caller passes the address of a variable and reads the value there afterwards.

</details>

**4. One library function returns a pointer from `calloc`. The caller never reads or writes through it and only passes it to other functions of the library, one of which ends in `free`. What is this pointer, and what rules of use follow?**

<details>
<summary>Answer</summary>

An opaque handle. Every handle that an opening function returns must be closed exactly once, must not be used after it is closed, and a null result from the opening function must be checked.

</details>

**5. `subsd xmm1, xmm0` and `mulsd xmm1, xmm2` appear, and `xmm0` holds the result. Which registers are arguments, and what type do they have?**

<details>
<summary>Answer</summary>

`xmm0`, `xmm1` and `xmm2` are read before they are written, so there are three `double` arguments (`sd` means scalar double; `ss` would be `float`), and the return type is `double`. Integer registers do not take part.

</details>

**6. You tested a hypothesis with three inputs and all matched. Is it confirmed?**

<details>
<summary>Answer</summary>

Not yet. Agreement on typical inputs is weak evidence. Test the boundaries (empty or one-element input, zero, the largest and smallest values, a buffer that just fits and one byte too small) and compare with an independent model of the rule, written before the run.

</details>

**7. Why does the probe print the three values of `bb_next` in three separate statements?**

<details>
<summary>Answer</summary>

C does not define the order in which the arguments of one call are evaluated. In a single `printf` with three calls, the values can appear in any order, and the output would no longer match what the library does.

</details>

**8. A function reads `[rdi]` before it checks that its count argument is non-zero. What does this add to the interface?**

<details>
<summary>Answer</summary>

A rule of use: the count must be at least 1, or the function reads memory it does not own. A signature cannot express it, so it belongs in the comment of the header.

</details>
