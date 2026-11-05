[English](HINTS.md) | [العربية](HINTS.ar.md) | [Deutsch](HINTS.de.md) | [Español](HINTS.es.md) | [Français](HINTS.fr.md)

# Hints: exercise 06

Open one level at a time. Try again between levels.

## Part 1: Reverse

<details>
<summary>Level 1: nudge</summary>

In `main`, look at what happens after the third number is converted. Each check is a compare followed by a conditional jump to the code that prints `wall`. Count the checks.

</details>

<details>
<summary>Level 2: direction</summary>

Pair each compare with the jump after it. `ja` and `jb` are unsigned; `jl` and `jg` are signed; `jns` tests the sign bit. A `lea` or `sub` followed by an unsigned `cmp` is a range check: subtracting the lower bound turns "between low and high" into one unsigned comparison.

</details>

<details>
<summary>Level 3: nearly the answer</summary>

The first check is a range check on A. The second keeps only some bits of A with `and`. The third is the same kind of range check on B. The last three concern C: its sign, its low byte (an 8-bit compare), and a signed lower bound. Work out the set of values each check allows, intersect them, and take the smallest value of each number.

</details>

## Part 2: Assemble

<details>
<summary>Level 1: nudge</summary>

Write the same checks as the binary, in the same order. Every failing jump can go to one shared label that returns 0.

</details>

<details>
<summary>Level 2: direction</summary>

A condition `low <= x <= high` becomes one subtraction and one unsigned compare: `lea eax, [rdi - low]`, `cmp eax, high - low`, `ja no`. Use `test` and `jns` for the sign, and `cmp dl, value` to look at the low byte only.

</details>

<details>
<summary>Level 3: nearly the answer</summary>

Pick each jump by the type of the check: `ja` for unsigned "above", `jl` for signed "less". A negative number looks huge when compared as unsigned, so a test on the sign or on a signed bound needs a signed jump. Put `mov eax, 1` after the last check and `xor eax, eax` at the shared failure label.

</details>
