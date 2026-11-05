[English](quiz.md) | [العربية](quiz.ar.md) | [Deutsch](quiz.de.md) | [Español](quiz.es.md) | [Français](quiz.fr.md)

# Quiz: 02. The CPU and its registers

**1. `rax` holds `0x1122334455667788`. After `mov al, 0xFF`, what does it hold?**

<details>
<summary>Answer</summary>

`0x11223344556677FF`. An 8-bit write changes only bits 0 to 7.

</details>

**2. `rax` holds `0x1122334455667788`. After `mov eax, 1`, what does it hold?**

<details>
<summary>Answer</summary>

`0x0000000000000001`. A 32-bit write clears the upper 32 bits.

</details>

**3. Which flag reports that an unsigned addition overflowed, and which reports a signed overflow?**

<details>
<summary>Answer</summary>

CF (carry) for unsigned; OF (overflow) for signed.

</details>

**4. After `mov al, 127` and `add al, 1`, is CF set? Is OF set?**

<details>
<summary>Answer</summary>

CF is clear (127 + 1 = 128 fits in an unsigned byte). OF is set (128 does not fit in a signed byte; the result reads as -128).

</details>

**5. What is the effect of `cmp al, 5` on `al`?**

<details>
<summary>Answer</summary>

None. It computes `al - 5`, discards the result and keeps only the flags.

</details>

**6. Which register holds the address of the next instruction?**

<details>
<summary>Answer</summary>

`rip` in 64-bit code, `eip` in 32-bit code.

</details>

**7. What does the instruction `mov eax, eax` do in 64-bit code?**

<details>
<summary>Answer</summary>

It zero-extends `eax` into `rax`, because the 32-bit write clears the upper 32 bits. The visible value in `eax` does not change.

</details>
