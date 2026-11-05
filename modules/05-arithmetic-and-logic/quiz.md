[English](quiz.md) | [العربية](quiz.ar.md) | [Deutsch](quiz.de.md) | [Español](quiz.es.md) | [Français](quiz.fr.md)

# Quiz: 05. Arithmetic and logic

**1. After `add rax, 1` with `rax = 0x7FFFFFFFFFFFFFFF`, which of CF and OF is set?**

<details>
<summary>Answer</summary>

OF only. As an unsigned addition nothing carries out of the register, but as a signed addition the result wraps from the largest positive value to the most negative one.

</details>

**2. Why must `cqo` come before `idiv`?**

<details>
<summary>Answer</summary>

`idiv` divides the 128-bit value `rdx:rax`. `cqo` fills `rdx` with the sign extension of `rax`; without it, `rdx` holds an unrelated value and the result is wrong.

</details>

**3. What does `mul rsi` write besides `rax`?**

<details>
<summary>Answer</summary>

`rdx`. The 128-bit product is stored as `rdx:rax`, so any value you kept in `rdx` is overwritten.

</details>

**4. `-17 / 5` is computed with `idiv`. What are the quotient and the remainder?**

<details>
<summary>Answer</summary>

Quotient -3 and remainder -2. `idiv` rounds toward zero and the remainder takes the sign of the dividend.

</details>

**5. What is the difference between `sar` and `shr`?**

<details>
<summary>Answer</summary>

`shr` shifts in zeros (logical shift, for unsigned values). `sar` copies the sign bit in (arithmetic shift, for signed values), so a negative number stays negative.

</details>

**6. A compiled function contains `shr rax, 3`. What did the C source most likely do?**

<details>
<summary>Answer</summary>

Divided an unsigned value by 8.

</details>

**7. Why can `mov [rdx], rdi` sit between `add rdi, rsi` and `seto al`?**

<details>
<summary>Answer</summary>

`mov` does not change any flag, so OF still describes the `add` when `seto` reads it.

</details>

**8. Why does `daa` fail to assemble in 64-bit code?**

<details>
<summary>Answer</summary>

The decimal adjust instructions are not available in 64-bit mode. NASM reports `error: instruction not supported in 64-bit mode`.

</details>
