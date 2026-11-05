[English](HINTS.md) | [العربية](HINTS.ar.md) | [Deutsch](HINTS.de.md) | [Español](HINTS.es.md) | [Français](HINTS.fr.md)

# Hints: exercise 03

Open one level at a time. Try again between levels.

## Part 1: Reverse

<details>
<summary>Level 1: nudge</summary>

Set a breakpoint on each of the four marks and look at the registers at every stop. The instruction that matters is the one just above the mark you stopped at.

</details>

<details>
<summary>Level 2: direction</summary>

A breakpoint on a label stops before the instruction at that label, so the registers and flags show the result of the instruction above it. Use `p/d $rax` to print `rax` in decimal. Flags that GDB does not list are clear. Module 02 explains what each flag means.

</details>

<details>
<summary>Level 3: nearly the answer</summary>

At `mark1`, `rax` holds the result of a multiplication. At `mark2` a subtraction borrowed and produced a negative result. At `mark3` a 32-bit addition wrapped around to zero. At `mark4` a 64-bit addition overflowed as a signed number. Translate each set of flag names into four digits in the order CF, ZF, SF, OF.

</details>

## Part 2: Assemble

<details>
<summary>Level 1: nudge</summary>

The operation itself sets the flags. Capture them straight after it, before another arithmetic instruction changes them.

</details>

<details>
<summary>Level 2: direction</summary>

`setc`, `setz`, `sets` and `seto` each copy one flag into an 8-bit register. `mov` and `movzx` do not change flags, but `add`, `or` and `shl` do, so run all four `set` instructions before you combine the bytes.

</details>

<details>
<summary>Level 3: nearly the answer</summary>

Widen each byte with `movzx`, shift it to its bit position with `shl` (ZF by 1, SF by 2, OF by 3), and combine the values with `or`. Store the result before you collect the flags; a store with `mov` leaves the flags alone. Both functions can share the code that collects the flags: let `ledger_sub` run into it after its own store.

</details>
