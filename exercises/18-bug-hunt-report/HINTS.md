[English](HINTS.md) | [العربية](HINTS.ar.md) | [Deutsch](HINTS.de.md) | [Español](HINTS.es.md) | [Français](HINTS.fr.md)

# Hints: exercise 18

Open one level at a time. Try again between levels.

## Part 1: Reverse

<details>
<summary>Level 1: nudge</summary>

Start by making small input files and watching the output, so that you know what each record type does. Then look for the dispatch on the record type in the disassembly: a comparison with a small number followed by an indirect jump through a table. Each table entry is the code of one record type. Read the code of every type as a sink: what size or index comes from the input, and what check stands between the input and the use?

</details>

<details>
<summary>Level 2: direction</summary>

For each handler, write down the size of the destination, the largest size the check lets through, and the widths and signs of the values involved. In the handler for numbers, look at the width of the register in the comparison and which jump follows it: signed or unsigned. In the handler for data blocks, follow the count through the allocation and then through the copy, and compare the width of the register used each time. Look at how the record walk computes the next record and at what it compares that position with. In the handlers that release memory, follow the pointer to the end of the function.

</details>

<details>
<summary>Level 3: nearly the answer</summary>

The name handler compares the length with a constant, and then writes one more byte than the length: what is the constant, and what does the extra byte do at the largest accepted length? For the numbers, a byte compared as a signed value is negative from a certain value on; use that value. For the tags, the table has eight entries, and the comparison constant tells you the largest index that passes; compare the two. For the data blocks, find the instruction that truncates the allocation size to 8 bits, and compute the smallest count whose size in bytes does not fit in 8 bits. For the release of memory, find the type number in the jump table whose code calls the function that frees memory and then leaves the pointer alone; the entry for type 4 also frees memory, but you are asked for the record type that does it as its whole job.

</details>

## Part 2: Assemble

<details>
<summary>Level 1: nudge</summary>

Write the loop first: how many bytes are left, read the type and the length, check that the payload fits, and advance. Do these checks before you look at the type. Put each record type in a block of its own and make each one jump back to the loop. Use only the registers that a function may change, so there is nothing to save.

</details>

<details>
<summary>Level 2: direction</summary>

Keep a position and the length of the buffer, and always compute the bytes that are left as a subtraction before comparing: that cannot overflow, while position plus length can. For the numbers, load each 32-bit value with a sign-extending move and add it to the 64-bit sum in memory. For the tags, check the byte before you use it as an index. For the data block, check the length against the count before you read anything after the count. Write the results into the structure as you go; the tests do not look at it when you return an error.

</details>

<details>
<summary>Level 3: nearly the answer</summary>

For the name, write 16 zero bytes first (one 16-byte store does it) and then copy the bytes, so that a shorter name replaces a longer one. For the numbers, test the low bits of the length for the multiple of 4, then compare it with 32. For the data block, compute 1 plus 4 times the count in a 32-bit register, which cannot wrap for a count that fits in a byte, and compare it with the length. A block replaces an earlier one, so set the sum to 0 before you add. The record of type 0 follows the same truncation rule as the others. If the checker ends with a signal, a read went outside the buffer: look at the header check and the payload check first.

</details>
