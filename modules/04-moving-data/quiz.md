[English](quiz.md) | [العربية](quiz.ar.md) | [Deutsch](quiz.de.md) | [Español](quiz.es.md) | [Français](quiz.fr.md)

# Quiz: 04. Moving data and addressing modes

**1. Why does `mov [rdi], 1` fail to assemble, and how do you fix it?**

<details>
<summary>Answer</summary>

Nothing says how wide the store is: a constant has no size and `[rdi]` has none either. Add a size keyword, for example `mov dword [rdi], 1`.

</details>

**2. What does `[rdi + rsi*4]` usually mean in compiled code?**

<details>
<summary>Answer</summary>

Element `rsi` of an array of 4-byte values whose address is in `rdi`, such as `a[i]` for an `int` array.

</details>

**3. What is the difference between `lea rax, [rdi + 8]` and `mov rax, [rdi + 8]`?**

<details>
<summary>Answer</summary>

`lea` stores the address `rdi + 8` in `rax` and reads no memory. `mov` reads 8 bytes from that address and stores the value.

</details>

**4. Which instruction loads a signed byte and extends it to 32 bits?**

<details>
<summary>Answer</summary>

`movsx eax, byte [rdi]`. `movzx` would treat the byte as unsigned.

</details>

**5. A compiled function contains `shl rsi, 4` followed by `mov rax, [rdi + rsi + 8]`. What does this suggest about the C source?**

<details>
<summary>Answer</summary>

It indexes an array of 16-byte structures (the shift by 4 multiplies the index by 16) and reads a field at offset 8 of the selected element.

</details>

**6. Why is there no `movzx rax, eax`?**

<details>
<summary>Answer</summary>

A normal 32-bit write, such as `mov eax, eax`, already clears the upper 32 bits of the 64-bit register, so it zero-extends by itself.

</details>

**7. The bytes `34 12` are read as a word on x86. What value results?**

<details>
<summary>Answer</summary>

`0x1234`. x86 is little-endian, so the first byte is the low byte.

</details>
