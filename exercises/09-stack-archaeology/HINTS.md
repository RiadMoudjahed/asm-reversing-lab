[English](HINTS.md) | [العربية](HINTS.ar.md) | [Deutsch](HINTS.de.md) | [Español](HINTS.es.md) | [Français](HINTS.fr.md)

# Hints: exercise 09

Open one level at a time. Try again between levels.

## Part 1: Reverse

<details>
<summary>Level 1: nudge</summary>

Read the first and the last instructions of `chain`. The prologue pushes registers and moves `rsp`; the epilogue undoes both in reverse order. Find the instruction that prepares the second argument of `note`, because it shows where the table starts.

</details>

<details>
<summary>Level 2: direction</summary>

Count the `push` instructions before the `sub rsp`. Each one is 8 bytes. A level of recursion is the return address (8 bytes) plus the pushes plus the amount subtracted. The table starts at the address passed to `note`; the saved return address is above the pushes. The stores into `[rsp]`, `[rsp + 8]` and `[rsp + 16]` show which part of the subtracted space is used.

</details>

<details>
<summary>Level 3: nearly the answer</summary>

Two registers are pushed, so the saved registers take 16 bytes. The subtracted value is in hexadecimal in the listing; convert it. The table uses three 8-byte entries starting at `rsp`. The return address sits above the subtracted space and the pushed registers: add the subtracted amount and the pushed bytes to get its distance from `rsp`. The unused space is the subtracted amount minus 24. To check your numbers, run the program under `gdb`, stop at `note` twice and subtract the two values of `rsp`.

</details>

## Part 2: Assemble

<details>
<summary>Level 1: nudge</summary>

Write the formulas of `chain` on paper first. Then list which values you need after `note` returns and after the recursive call returns, and decide where each one lives.

</details>

<details>
<summary>Level 2: direction</summary>

After `note`, `rdi` and `rsi` are gone, so keep `depth` and `seed` in callee-saved registers, which you must save with `push` and restore with `pop`. The table can live on the stack and stay there across both calls. At entry `rsp` is 8 modulo 16; every `push` changes that, and `sub rsp` must bring it back to a multiple of 16 before each `call`.

</details>

<details>
<summary>Level 3: nearly the answer</summary>

Push two callee-saved registers, then subtract 24 bytes for the table. Entry is 8 modulo 16, the pushes keep it at 8 modulo 16, and 24 more bytes make it a multiple of 16. That level uses 8 + 16 + 24 bytes. If the checker reports `align`, the total of the pushes and the subtraction is wrong. If it reports `regs`, a callee-saved register is changed and not restored, or restored in the wrong order. If it reports `stack`, the frame is larger than `L`.

</details>
