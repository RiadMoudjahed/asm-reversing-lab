[English](HINTS.md) | [العربية](HINTS.ar.md) | [Deutsch](HINTS.de.md) | [Español](HINTS.es.md) | [Français](HINTS.fr.md)

# Hints: exercise 17

Open one level at a time. Try again between levels.

## Part 1: Reverse

<details>
<summary>Level 1: nudge</summary>

Start with samples: pack files whose contents are easy to recognize, such as a single repeated letter, and look at the hex dump. Find the magic value, the numbers that look like counts or offsets, and the names. Then pack one more file and see what moves. In the program, find the code that stores the first bytes of the output; it stores constants and values computed from the number of files.

</details>

<details>
<summary>Level 2: direction</summary>

Compare archives made from different numbers of files and from different contents. A file of identical bytes does not appear as identical bytes in the archive, so the program changes the data; and a short loop with a rotation computes a value that is stored per file. The step between table entries is a shift or a multiplication in the code. For the limits, look for comparisons against constants before any file is read: they guard the error messages you can see with `strings`.

</details>

<details>
<summary>Level 3: nearly the answer</summary>

The offset of each data block is rounded up with an addition and a mask; the mask tells the alignment. The key is computed from the number of files by a multiplication and an addition, both with constants in the code; evaluate it for 3 files. The rotation instruction is in the loop that goes over the data bytes. Check each number by packing a file with the tool and reading the result in the dump. The limit on the number of files is the constant in the comparison that precedes the message about too many files.

</details>

## Part 2: Assemble

<details>
<summary>Level 1: nudge</summary>

Write `ark_count` first and let the other two call it, so that the header is validated in one place. Use 64-bit registers for every offset, size and sum, and zero-extend what you load from the file.

</details>

<details>
<summary>Level 2: direction</summary>

Work out where the table entry for an index is: the table offset plus 16 times the index. An unsigned comparison of the index with the count rejects negative indexes at the same time as large ones. For names, scan for the zero byte with a bound at the end of the archive and not past it. For extraction, decode each byte with the key and the position, update the checksum with the decoded byte, and compare at the end; do the bounds and capacity checks before touching the data.

</details>

<details>
<summary>Level 3: nearly the answer</summary>

Because the table offset, the data offset and the size are 32-bit values in the file, add them in 64-bit registers: `mov ecx, [..]` already clears the upper half. A data range ends at offset plus size, and that sum must not exceed `n`. The key byte of position `i` is the low byte of the sum of the key and `i`, which a `lea` into a 32-bit register followed by a byte `xor` gives. The checksum starts at the constant you found, is rotated and added for each decoded byte, and is finally combined with the size. If the checker ends with a signal, you read outside the archive; check the name scan and the sums first.

</details>
