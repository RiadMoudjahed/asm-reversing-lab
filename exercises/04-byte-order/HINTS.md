[English](HINTS.md) | [العربية](HINTS.ar.md) | [Deutsch](HINTS.de.md) | [Español](HINTS.es.md) | [Français](HINTS.fr.md)

# Hints: exercise 04

Open one level at a time. Try again between levels.

## Part 1: Reverse

<details>
<summary>Level 1: nudge</summary>

Look at how each field is loaded before it is added to the total. Some loads are followed by an extra instruction and some are not.

</details>

<details>
<summary>Level 2: direction</summary>

`bswap` reverses the four bytes of a register. `rol dx, 8` reverses the two bytes of `dx`. A load with no extra instruction keeps the order in memory, which on x86 means little-endian. The `cmp` constant lets you check your decoding: the four values must add up to it.

</details>

<details>
<summary>Level 3: nearly the answer</summary>

Field A is 4 bytes at offset 0 and is swapped, so it is big-endian. Field B is 2 bytes at offset 4 and not swapped. Field C is 2 bytes at offset 6 and swapped. Field D is 4 bytes at offset 8 and not swapped. Take the bytes from the `.rodata` dump, reverse the ones that are swapped, and read each result as a number. Convert hexadecimal to decimal where the question asks for decimal.

</details>

## Part 2: Assemble

<details>
<summary>Level 1: nudge</summary>

On x86 a plain load already reads little-endian values correctly. Only the big-endian fields need work.

</details>

<details>
<summary>Level 2: direction</summary>

To build a big-endian value, start with the first byte, shift left by 8, combine the next byte with `or`, and repeat. Use `movzx` to load each byte so the upper bits of the register are zero. Loading a whole dword and then fixing it would read the right bytes but needs `bswap`, which is not allowed.

</details>

<details>
<summary>Level 3: nearly the answer</summary>

`load_le16` is one instruction: `movzx eax, word [rdi]`. For `decode_record`, build `version` and `flags` with the shift-and-`or` pattern and store them with a 4-byte and a 2-byte store. Copy `port` and `timeout_ms` with plain loads and stores of the same width. A store wider than the field overwrites its neighbor, and a load wider than the field can read past the end of the input; both make the checker fail.

</details>
