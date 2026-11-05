[English](HINTS.md) | [العربية](HINTS.ar.md) | [Deutsch](HINTS.de.md) | [Español](HINTS.es.md) | [Français](HINTS.fr.md)

# Hints: exercise 07

Open one level at a time. Try again between levels.

## Part 1: Reverse

<details>
<summary>Level 1: nudge</summary>

Open the six routines one by one. Each is short and contains a `rep`, `repz` or `repnz` string instruction. Note which instruction and which prefix each uses.

</details>

<details>
<summary>Level 2: direction</summary>

`rep stos` fills memory. `rep movs` copies it. `repz cmps` compares two blocks while they are equal. `repnz scas` searches for the byte in `al`. Two routines use `repnz scas`: one starts with `al` set to zero and turns the final `rcx` into a length, the other starts with `al` taken from an argument and returns a pointer. Two routines copy: one only goes forward, the other contains `std` and `cld`.

</details>

<details>
<summary>Level 3: nearly the answer</summary>

The routine with `std` and `cld` is the overlap-safe copy. The one that copies a quadword at a time and then the rest byte by byte is the plain copy. The `repz cmps` routine subtracts two bytes at the end, which is how a comparison gets its result. The routine that returns `rdi - 1` after a match is the search; the one with `not rcx` is the length. Use the numbers in the symbol names to fill in the flag.

</details>

## Part 2: Assemble

<details>
<summary>Level 1: nudge</summary>

Each routine is short. Set up `rsi`, `rdi` and `rcx` (and `al` where a byte is involved), then let a single `rep` instruction do the work.

</details>

<details>
<summary>Level 2: direction</summary>

For `my_strlen`, start with `al` zero and `rcx` at -1, run `repnz scasb`, and turn the leftover `rcx` into a length. For `my_memchr`, check for `n` equal to 0 first, then run `repnz scasb` and look at the zero flag. For `my_memmove`, compare the addresses: if the destination is above the source and the blocks overlap, set `std` and copy from the last byte.

</details>

<details>
<summary>Level 3: nearly the answer</summary>

After `repnz scasb` over `n` bytes, the zero flag is set only if a match was found, and `rdi` is one byte past the match. `not rcx` after the scan counts the terminator as well, so subtract one. Copy backward only when `rdi` is above `rsi` and `rdi` is below `rsi + n`, using `std`, starting at `rsi + n - 1` and `rdi + n - 1`, and finish with `cld`. If the checker says the direction flag was left set, a path is missing its `cld`.

</details>
