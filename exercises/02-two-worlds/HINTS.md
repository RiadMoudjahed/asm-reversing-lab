[English](HINTS.md) | [العربية](HINTS.ar.md) | [Deutsch](HINTS.de.md) | [Español](HINTS.es.md) | [Français](HINTS.fr.md)

# Hints: exercise 02

Open one level at a time. Try again between levels.

## Part 1: Reverse

<details>
<summary>Level 1: nudge</summary>

`strace` shows the name of each call, but only the disassembly shows the number. Open both listings side by side and look at what loads `eax`.

</details>

<details>
<summary>Level 2: direction</summary>

Each kernel entry (`int 0x80` or `syscall`) is preceded by a `mov eax, <number>`. The numbers are shown in hexadecimal, so convert them to decimal before you look them up in the cheatsheet. The call that is neither `write` nor `exit` is `sync`.

</details>

<details>
<summary>Level 3: nearly the answer</summary>

Each binary has three kernel entries. Read the value in `eax` before the second one for the `sync` number, and the value in `eax` before the third one for the `exit` number. Do this for the 32-bit listing and then for the 64-bit listing. The two `sync` numbers differ, and so do the two `exit` numbers.

</details>

## Part 2: Assemble

<details>
<summary>Level 1: nudge</summary>

Everything in the starter that is specific to 32-bit mode needs to change: the mode directive, the instruction that enters the kernel, the call numbers, and the registers that carry arguments.

</details>

<details>
<summary>Level 2: direction</summary>

Use what you found in Part 1: the 64-bit call numbers, and the 64-bit argument registers from the cheatsheet. Remember `default rel` and `lea` to load the address of the text.

</details>

<details>
<summary>Level 3: nearly the answer</summary>

For `write` in 64-bit code, `rdi` holds the descriptor, `rsi` the address and `rdx` the length. `sync` takes no arguments. For `exit`, `rdi` holds the status. If the checker says it found a forbidden instruction, a copy of `int 0x80` is still in the file. If it lists different system calls than expected, a call number is wrong.

</details>
