[English](HINTS.md) | [العربية](HINTS.ar.md) | [Deutsch](HINTS.de.md) | [Español](HINTS.es.md) | [Français](HINTS.fr.md)

# Hints: exercise 05

Open one level at a time. Try again between levels.

## Part 1: Reverse

<details>
<summary>Level 1: nudge</summary>

Run the program with a few inputs and read the exit status each time with `echo $?`. Then look at the routine that checks each line; the limits are constants in compare instructions.

</details>

<details>
<summary>Level 2: direction</summary>

`sub al, 0x30` followed by `cmp al, 9` and `ja` is the classic test for an ASCII digit: anything that is not `0` to `9` ends up above 9 as an unsigned number. A compare of the digit count with a constant is the length limit. The exit status is the value in `ebx` when `eax` is 1 and `int 0x80` runs.

</details>

<details>
<summary>Level 3: nearly the answer</summary>

The length check compares `ecx` with a constant, and anything above it jumps to the error path. Each of the two ending paths, the error and the sum-too-large, loads `ebx` with its own value just before it exits. Read the constant from the length check, and read the two statuses from the end of each path. Convert hexadecimal to decimal before you build the flag.

</details>

## Part 2: Assemble

<details>
<summary>Level 1: nudge</summary>

A 12-digit number does not fit in a 32-bit register, so work on the text itself: add digit by digit from the right, passing a carry along.

</details>

<details>
<summary>Level 2: direction</summary>

For each position, subtract `0x30` from the character to get the digit, add the two digits and the carry, and subtract 10 (setting the carry to 1) if the result is 10 or more. Add `0x30` again and store the character into a buffer that you fill from the right. Treat a missing digit in the shorter number as 0.

</details>

<details>
<summary>Level 3: nearly the answer</summary>

Check each line before you add anything: every character up to the newline must be a digit, a line must have at least one and at most 12 digits, and a line with no newline at the end is an error too. Reserve 13 positions for the sum: if the first one is nonzero, the answer is `OVERFLOW`. When printing, skip leading zeros but always print at least one digit.

</details>
