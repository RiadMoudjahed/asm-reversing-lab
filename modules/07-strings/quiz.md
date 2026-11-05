[English](quiz.md) | [العربية](quiz.ar.md) | [Deutsch](quiz.de.md) | [Español](quiz.es.md) | [Français](quiz.fr.md)

# Quiz: 07. Strings and string instructions

**1. Which registers does `rep movsb` use, and what does each hold?**

<details>
<summary>Answer</summary>

`rsi` holds the source address, `rdi` the destination address, and `rcx` the number of bytes to copy. Both pointers advance by one byte per pass.

</details>

**2. What does the direction flag control, and what must its value be when a function returns?**

<details>
<summary>Answer</summary>

It decides whether the string instructions move their pointers up (DF = 0, `cld`) or down (DF = 1, `std`). The System V ABI requires it to be clear when a function returns.

</details>

**3. What is the difference between `repe` and `repne`?**

<details>
<summary>Answer</summary>

`repe` repeats while the last comparison was equal (ZF = 1) and `rcx` is above zero. `repne` repeats while it was not equal (ZF = 0) and `rcx` is above zero.

</details>

**4. After `repne scasb` ends with `rcx` equal to 0 and ZF equal to 0, what does it mean for a `memchr` implementation?**

<details>
<summary>Answer</summary>

All the bytes were compared and none matched, so the byte was not found and the function returns NULL.

</details>

**5. Why does `my_memmove` copy backward when the destination is above the source and overlaps it?**

<details>
<summary>Answer</summary>

A forward copy would overwrite source bytes before reading them. Starting from the last byte reads each byte before it can be overwritten.

</details>

**6. What does `mov ecx, 0x40` followed by `rep stos DWORD` with `eax` equal to 0 do?**

<details>
<summary>Answer</summary>

It clears 0x40 times 4 = 256 bytes at `[rdi]`, which is `memset(p, 0, 256)`.

</details>

**7. What happens to the flags when `repe cmpsb` runs with `rcx` equal to 0?**

<details>
<summary>Answer</summary>

Nothing is executed and the flags keep their previous values. That is why `my_memcmp` sets ZF with `xor eax, eax` before the instruction, so a zero length counts as equal.

</details>

**8. In `memcmp(a, b, n)`, `a` arrives in `rdi` and `b` in `rsi`. Which bytes do you subtract to produce the result after a mismatch, and in which order?**

<details>
<summary>Answer</summary>

The byte from `a` minus the byte from `b`, that is `[rdi - 1]` minus `[rsi - 1]` (both pointers have already moved past the differing bytes). The opposite order reverses the sign of the result.

</details>
