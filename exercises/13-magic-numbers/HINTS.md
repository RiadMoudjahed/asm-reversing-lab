[English](HINTS.md) | [العربية](HINTS.ar.md) | [Deutsch](HINTS.de.md) | [Español](HINTS.es.md) | [Français](HINTS.fr.md)

# Hints: exercise 13

Open one level at a time. Try again between levels.

## Part 1: Reverse

<details>
<summary>Level 1: nudge</summary>

Look for `imul` instructions that multiply by a large constant, each followed by a right shift. There are four such pairs in `mix`, one for each division. Write each constant and its shift in a table.

</details>

<details>
<summary>Level 2: direction</summary>

The divisor is 2 raised to the total shift, divided by the constant, rounded to the nearest integer. The total shift counts every right shift that follows the multiplication, including a `shr` by 32 followed by a smaller one. Run `14_magic.py` from module 14 to check your reading. For `K` and `M`, look at the `lea` instructions after each quotient: `lea r, [x+x*4]` is a multiplication by 5, `lea r, [x+x*2]` by 3, `lea r, [x+x*8]` by 9.

</details>

<details>
<summary>Level 3: nearly the answer</summary>

One of the signed sequences loads a constant that the disassembler shows with many `f` digits, such as `0xffffffff88888889`. It is a 32-bit constant with its top bit set that was sign-extended; the real magic number is the low 32 bits read as a positive number, and the extra `add` of `x` after the `shr 32` makes up for the sign. The unsigned remainder is computed as the value minus the quotient times the divisor, and the quotient times the divisor is a `lea`; the factor `M` multiplies the result of that subtraction. To check a candidate formula, run `bin/magic` on a few inputs and compare.

</details>

## Part 2: Assemble

<details>
<summary>Level 1: nudge</summary>

Write the formula in terms of the four divisors you found. Then write one block for each term. Each block turns a division by a constant into `x * M` followed by a right shift, with `M` and the shift taken from the binary.

</details>

<details>
<summary>Level 2: direction</summary>

Use 64-bit registers for the products: `mov eax, edi` zero-extends `x` for the unsigned terms, and `movsxd rax, edi` sign-extends it for the signed terms. A multiply by a 32-bit constant that has its top bit set needs the constant loaded as a positive 64-bit value, for example with `mov edx, constant`. The remainder is `x - q * d`. For the signed terms, shift arithmetically (`sar`), and add 1 to the quotient when `x` is negative; the value `x >> 31` is -1 for negative `x`, so subtract it.

</details>

<details>
<summary>Level 3: nearly the answer</summary>

For the unsigned quotient, `(x * M) >> k` is enough with the constant and the shift from the binary. The signed quotient and remainder need the correction for negative inputs: if the quotient comes out one too small for negative `x`, you forgot to subtract `x >> 31`. If only inputs of 2^31 and above fail, the unsigned terms were handled with a sign extension. If the checker says the instruction `div` was found, replace it with the multiplication; you may combine all terms with `lea` at the end.

</details>
