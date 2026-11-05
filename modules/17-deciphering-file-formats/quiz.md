[English](quiz.md) | [العربية](quiz.ar.md) | [Deutsch](quiz.de.md) | [Español](quiz.es.md) | [Français](quiz.fr.md)

# Quiz: 17. Deciphering file formats

Answer first, then open the answer.

**1. A file starts with the bytes `50 4b 03 04` and the same four bytes start every sample you have. What is this, and what does it let you do?**

<details>
<summary>Answer</summary>

A magic value, a fixed identifier at offset 0. It lets a reader recognize the format and reject other files, and it lets you search a program for the code that checks it, such as a comparison with this constant or with a string of these bytes.

</details>

**2. You change one parameter when making a file and the dump differs in two places: four bytes near the start and the last four bytes. What do you conclude?**

<details>
<summary>Answer</summary>

The four bytes near the start hold the parameter you changed, and the last four bytes depend on the rest of the file, so they are probably a checksum or a hash. The experiment also shows that the checksum covers at least the region that holds the parameter.

</details>

**3. The code assembles a value from four bytes with `movzx` loads, `shl` by 8, 16 and 24 and `or`, where the byte at the lowest address has no shift. What is the byte order?**

<details>
<summary>Answer</summary>

Little endian: the byte at the lowest address is the least significant. A big-endian reader would shift the first byte by 24.

</details>

**4. You find a loop that fills 256 table entries with eight rounds of `shr 1` and a conditional xor with 0xedb88320. What is it, and which three details must you still establish?**

<details>
<summary>Answer</summary>

It builds the table of a CRC-32 (the reflected polynomial 0xedb88320). You still need the initial value of the running checksum, whether the result is complemented at the end, and the range of bytes the checksum covers. They change the result, so each must be taken from the code and then confirmed against a sample.

</details>

**5. How do you find the code that reads a file when the program is stripped?**

<details>
<summary>Answer</summary>

Start from what survives stripping: the error messages and other strings (follow their cross-references to the conditions that jump to them), the imported functions such as `fopen`, `fread` and `memcmp`, and constants such as tags, magic values and polynomials. The sizes passed to `fread` give the sizes of the next field or structure.

</details>

**6. Why is a specification not finished until you have written a parser from it and compared that parser with the original?**

<details>
<summary>Answer</summary>

The specification is a set of claims, and writing code from it forces every claim to be precise. Comparing the parser with the original on valid and damaged files shows the claims that are wrong or missing, such as the order of the checks, limits, ignored tags and bytes after the end of the data.

</details>

**7. A reader accepts a file that has extra bytes after the last chunk. Is this a bug in your parser if it accepts the file too?**

<details>
<summary>Answer</summary>

No. The specification should record that the original does not look at bytes after the end chunk, and an equivalent parser behaves the same way. If the format is later used by a stricter reader, that is a different matter, and the specification should mention it as an observation.

</details>

**8. Why do you edit the field first and the checksum last when you build a test file?**

<details>
<summary>Answer</summary>

The checksum depends on the field. If you fix the checksum first and then change the field, the reader stops at the checksum check and never reaches the code that handles the field you wanted to test.

</details>
