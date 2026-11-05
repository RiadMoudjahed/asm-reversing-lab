[English](quiz.md) | [العربية](quiz.ar.md) | [Deutsch](quiz.de.md) | [Español](quiz.es.md) | [Français](quiz.fr.md)

# Quiz: 06. Conditions, branches and loops

**1. What is the difference between `cmp rdi, rsi` and `sub rdi, rsi`?**

<details>
<summary>Answer</summary>

Both compute `rdi - rsi` and set the same flags. `cmp` throws the result away; `sub` stores it in `rdi`.

</details>

**2. After `cmp rax, rbx`, which jump is taken when `rax` is greater than `rbx` as signed numbers, and which as unsigned numbers?**

<details>
<summary>Answer</summary>

`jg` for signed and `ja` for unsigned.

</details>

**3. Why does `max_signed(-1, 1)` return 1 while `max_unsigned(-1, 1)` returns 18446744073709551615?**

<details>
<summary>Answer</summary>

The bits of -1 are all ones. As a signed number that is smaller than 1; as an unsigned number it is the largest possible value. `cmovl` reads the signed flags and `cmovb` reads the unsigned ones.

</details>

**4. Which is guaranteed to run the body at least once, a while loop or a do-while loop?**

<details>
<summary>Answer</summary>

A do-while loop, because its test comes after the body.

</details>

**5. Why must `xor eax, eax` come before `cmp`, not between `cmp` and `setg al`?**

<details>
<summary>Answer</summary>

`xor` changes the flags. Placed after `cmp`, it replaces the comparison result before `setg` reads it.

</details>

**6. What does `test rdi, rcx` do when `rcx` is `rdi - 1`?**

<details>
<summary>Answer</summary>

It computes `rdi AND (rdi - 1)` for its flags only. The result is zero when `rdi` has at most one bit set, so `setz` after it reports a power of two (once zero has been excluded separately).

</details>

**7. A compiled function ends a loop with `cmp rax, rsi` followed by `jne` back to the top of the body. What does this tell you about how it was optimized?**

<details>
<summary>Answer</summary>

The compiler moved the loop's test to the bottom (loop inversion) and rewrote the counter, with a guard before the loop for the case where it should not run at all.

</details>

**8. What happens if `loop` runs with `rcx` equal to 0?**

<details>
<summary>Answer</summary>

`loop` subtracts 1 first, so `rcx` wraps to a huge nonzero value and the loop runs 2^64 times.

</details>
