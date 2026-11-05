[English](quiz.md) | [العربية](quiz.ar.md) | [Deutsch](quiz.de.md) | [Español](quiz.es.md) | [Français](quiz.fr.md)

# Quiz: 14. Compiled arithmetic

Answer first, then open the answer.

**1. What does `lea eax, [rdi+rdi*4]` compute, and does it read memory?**

<details>
<summary>Answer</summary>

It computes `5 * x` (`x + 4*x`) for `x` in `rdi`. It does not read memory and does not change the flags; `lea` only evaluates the address expression.

</details>

**2. Which C expression is `lea eax, [0+rdi*8]` followed by `sub eax, edi`?**

<details>
<summary>Answer</summary>

`x * 7`, computed as `8 * x - x`.

</details>

**3. A signed `x / 2` compiles to `mov eax, edi / shr eax, 31 / add eax, edi / sar eax`. Why not only `sar`?**

<details>
<summary>Answer</summary>

C rounds division toward zero, while `sar` rounds toward minus infinity. For negative odd values the two differ by one. The `shr eax, 31` and `add` add 1 to negative inputs before the shift to fix the rounding.

</details>

**4. You find `imul rax, rdx` after `mov edx, 3435973837`, then `shr rax, 35`. What is the divisor, and is the operation signed?**

<details>
<summary>Answer</summary>

The divisor is `2^35 / 3435973837`, which is about 10, so it is a division by 10. The shift is `shr` and there is no sign correction, so it is unsigned.

</details>

**5. What does the pair `cmp edi, esi` and `sbb eax, eax` produce?**

<details>
<summary>Answer</summary>

A mask: all ones when `edi` is below `esi` as unsigned values (the carry flag is set), and zero otherwise. It is `-(a < b)` computed without a jump.

</details>

**6. Name three instruction differences that tell you a value is signed rather than unsigned.**

<details>
<summary>Answer</summary>

Any three of: `jl`, `jg` and `setl` instead of `jb`, `ja` and `setb`; `sar` instead of `shr`; `movsx` instead of a plain `mov` to a 32-bit register; `cdq` with `idiv` instead of `xor edx, edx` with `div`; `cmovl` or `cmovge` instead of `cmovb` or `cmovnb`.

</details>

**7. A signed remainder `x % 8` is compiled. Why is it not a single `and eax, 7`?**

<details>
<summary>Answer</summary>

For negative `x`, the result in C is negative or zero, while `and` with 7 gives a value from 0 to 7. The compiler adds a bias before the `and` and subtracts it afterwards, which gives the sequence `sar 31`, `shr 29`, `add`, `and 7`, `sub`.

</details>
