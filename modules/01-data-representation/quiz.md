[English](quiz.md) | [العربية](quiz.ar.md) | [Deutsch](quiz.de.md) | [Español](quiz.es.md) | [Français](quiz.fr.md)

# Quiz: 01. Data representation

**1. What is the range of an 8-bit signed value?**

<details>
<summary>Answer</summary>

-128 to 127.

</details>

**2. What is the two's complement representation of -1 in one byte?**

<details>
<summary>Answer</summary>

`0xFF`. Invert `0000 0001` to get `1111 1110`, then add one to get `1111 1111`.

</details>

**3. The byte `0x80` is sign-extended to 32 bits. What is the result?**

<details>
<summary>Answer</summary>

`0xFFFFFF80`. The top bit of `0x80` is 1, so the new bits are filled with 1.

</details>

**4. A hex dump shows `78 56 34 12` at an address on x86. What 32-bit value is stored there?**

<details>
<summary>Answer</summary>

`0x12345678`. The first byte is the least significant, so reverse the bytes.

</details>

**5. How many bytes does the UTF-8 encoding of the letter `A` use?**

<details>
<summary>Answer</summary>

One byte, `0x41`. Characters below 128 are identical in ASCII and UTF-8.

</details>

**6. What is the number 47 in packed BCD, as a hexadecimal byte?**

<details>
<summary>Answer</summary>

`0x47`. Each decimal digit occupies one half-byte.

</details>

**7. A word contains `0xFFFE`. What are its unsigned and signed values?**

<details>
<summary>Answer</summary>

Unsigned 65534; signed -2.

</details>
