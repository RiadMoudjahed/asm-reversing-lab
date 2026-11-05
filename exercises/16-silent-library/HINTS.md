[English](HINTS.md) | [العربية](HINTS.ar.md) | [Deutsch](HINTS.de.md) | [Español](HINTS.es.md) | [Français](HINTS.fr.md)

# Hints: exercise 16

Open one level at a time. Try again between levels.

## Part 1: Reverse

<details>
<summary>Level 1: nudge</summary>

Start with the export list and group the functions by what they share: the ones that take the same first argument work on the same object. The creating function ends in a call to the allocator, and its arguments tell you the size. Read the feeding function next; it is a short loop.

</details>

<details>
<summary>Level 2: direction</summary>

In the feeding loop, track what is read from the state and what is written back, and note the offset of each. Two 32-bit fields change on every byte and one 64-bit field changes once at the end; the instruction that adds a register to a memory operand after the loop is the count. The rotation is a single instruction with a constant. The function that reads the digest combines three values from the state; one of them is rotated. For the packing function, do not try to count by reading: write a short C program with your recovered prototype and ask the library.

</details>

<details>
<summary>Level 3: nearly the answer</summary>

Do not confuse the two rotation amounts: one instruction is in the loop of the feeding function and the other is in the short function that reads the digest. The offsets are measured from the start of the block that the creating function returns. For the packed size, remember that the encoding is variable in length and that small values take few bytes; the library will tell you the exact count if you give it enough room, so pass a generous capacity.

</details>

## Part 2: Assemble

<details>
<summary>Level 1: nudge</summary>

Write `digest_buffer` first. It is four calls: create, feed, read, release. The handle returned by the first call must survive the next three, and so must the digest during the release, so decide which values go into callee-saved registers.

</details>

<details>
<summary>Level 2: direction</summary>

Push as many callee-saved registers as you need and check the alignment: on entry `rsp` is 8 modulo 16, and an odd number of pushes brings it to a multiple of 16. Declare the library functions with `extern` and call them through the PLT. For `pack_values`, work out the encoding from the results of your own probe: negative and positive values are first mapped to unsigned numbers, then written in groups of seven bits, and the high bit of a byte says that more bytes follow. Find out which of the two directions of the shift a negative number needs, and what exactly happens to the output when a value does not fit.

</details>

<details>
<summary>Level 3: nearly the answer</summary>

The mapping doubles the value and, for a negative value, flips all the bits of the result; do this with an arithmetic shift of a copy of the original to get zero or minus one, and an exclusive or. Compute how many bytes the mapped value needs before you write any of them, and compare the written total plus that number with the capacity. If it does not fit, return -1 without touching the output for that value; the values before it have already been written and stay. A value of 0x80 or more needs a continuation: write the low seven bits with the high bit set, shift right by seven, and repeat. If the checker prints `handle`, a state was not released or was released twice; `regs` means a callee-saved register was changed.

</details>
