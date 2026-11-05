[English](HINTS.md) | [العربية](HINTS.ar.md) | [Deutsch](HINTS.de.md) | [Español](HINTS.es.md) | [Français](HINTS.fr.md)

# Hints: exercise 12

Open one level at a time. Try again between levels.

## Part 1: Reverse

<details>
<summary>Level 1: nudge</summary>

Disassemble `dispatch`. Its first instructions are the bounds check and the index adjustment, and then there is an indirect `jmp` that reads from a table in `.rodata`.

</details>

<details>
<summary>Level 2: direction</summary>

The instruction before the bounds check subtracts a constant from the command: that constant is the lowest command. The constant in the comparison is the highest index; the number of entries is one more. Dump the table with `objdump -s -j .rodata`: each entry is an 8-byte little-endian address, so read the bytes in reverse order. The first entry is for the lowest command, the next for the following command, and so on.

</details>

<details>
<summary>Level 3: nearly the answer</summary>

Write the entries in a list with the command number next to each, then look for repeated addresses. The default code is the target of the `ja` that follows the bounds check; entries that equal it are commands without a case. Some entries point into the middle of another entry's code: that is a case that falls through into the next. They are different addresses, and they all count in `B`. To check yourself, run `bin/switchboard` with a command from each group.

</details>

## Part 2: Assemble

<details>
<summary>Level 1: nudge</summary>

First write down, for each command from 3 to 13, what `dispatch` returns as a function of `acc`. Use the program, or read the code blocks. Group the commands that share code.

</details>

<details>
<summary>Level 2: direction</summary>

Compute `edi - L` into another register, compare it with the highest index using `ja` to the default case, load the table address with `lea` and `jmp [reg + index*8]`. Put the table in `.rodata` as `dq label` entries. Write each block so that it computes its result in `eax` and returns; for a case that falls through, put its code directly before the next block.

</details>

<details>
<summary>Level 3: nearly the answer</summary>

Local labels of NASM belong to the last non-local label, so refer to them in the table with the full name, such as `run_cmd.c3`. The default case returns 0 because `dispatch` sets `acc` to 0xFFFFFFFF and adds one. One command returns `acc - 2` without adding one; check each block's final step against the program's output. If the checker says that a command above 13 gives the wrong answer, your bound is one too small; if the program crashes, your bounds check is a signed comparison.

</details>
