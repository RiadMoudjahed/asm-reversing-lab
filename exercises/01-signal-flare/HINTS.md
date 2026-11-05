[English](HINTS.md) | [العربية](HINTS.ar.md) | [Deutsch](HINTS.de.md) | [Español](HINTS.es.md) | [Français](HINTS.fr.md)

# Hints: exercise 01

Open one level at a time. Try again between levels.

## Part 1: Reverse

<details>
<summary>Level 1: nudge</summary>

`strace` prints each system call with its arguments and its result. Look at the last number after each `=` sign, and at the last argument of each call.

</details>

<details>
<summary>Level 2: direction</summary>

In the `objdump` listing, every `syscall` instruction is preceded by `mov` instructions that load `eax`, `edi`, `rsi` and `edx`. `eax` holds the call number. For `write`, `edi` is the file descriptor and `edx` is the byte count. Numbers in the listing are hexadecimal.

</details>

<details>
<summary>Level 3: nearly the answer</summary>

There are three `syscall` instructions. The first two load the same value into `eax`, and that value is the number of `write`. They differ in `edi`. Convert each `edx` value from hexadecimal to decimal. The third call is `exit`; the value loaded into `edi` before it is the exit status.

</details>

## Part 2: Assemble

<details>
<summary>Level 1: nudge</summary>

Your program makes the same three calls as the original, in the same order. Write them down first, with their arguments, before you write any code.

</details>

<details>
<summary>Level 2: direction</summary>

Put the two texts in `.rodata` with `db`. The byte `10` is a newline. Let NASM compute each length with `$ - label` placed right after the text. Use `lea rsi, [label]` to load an address.

</details>

<details>
<summary>Level 3: nearly the answer</summary>

Each `write` sets `eax` to the call number, `edi` to the descriptor, `rsi` to the text address and `edx` to the length, then runs `syscall`. `exit` sets `eax` to 60 and `edi` to the status. If the checker says the program was terminated by signal 11, the program ran past its last instruction because the `exit` call is missing.

</details>
