[English](quiz.md) | [العربية](quiz.ar.md) | [Deutsch](quiz.de.md) | [Español](quiz.es.md) | [Français](quiz.fr.md)

# Quiz: 03. Your first program

**1. Which part of `mov eax, 1` is the destination?**

<details>
<summary>Answer</summary>

`eax`. In NASM's Intel syntax the destination comes first and the source second.

</details>

**2. Why does `.bss` take no space in the executable file?**

<details>
<summary>Answer</summary>

It holds only a size. The loader supplies zeroed memory of that size when the program starts, so no bytes need to be stored.

</details>

**3. What does `message_len equ $ - message` compute when it is placed right after a string?**

<details>
<summary>Answer</summary>

The number of bytes between the label `message` and the current position, which is the length of the string including any bytes such as the newline.

</details>

**4. Fill in the 64-bit registers for `write(1, text, 17)`: call number, descriptor, address, length.**

<details>
<summary>Answer</summary>

`rax` = 1, `rdi` = 1, `rsi` = address of the text, `rdx` = 17.

</details>

**5. What are the differences between the 64-bit and the 32-bit way of calling `write`?**

<details>
<summary>Answer</summary>

64-bit: `syscall` instruction, call number 1 in `rax`, arguments in `rdi`, `rsi`, `rdx`. 32-bit: `int 0x80`, call number 4 in `eax`, arguments in `ebx`, `ecx`, `edx`.

</details>

**6. After a `syscall` instruction, which two registers does the processor overwrite besides `rax`?**

<details>
<summary>Answer</summary>

`rcx` (it receives the address of the next instruction) and `r11` (it receives the saved flags).

</details>

**7. You disassemble a program without source and see `mov eax, 0x3c` followed by `syscall`. What does it do?**

<details>
<summary>Answer</summary>

It calls `exit` (64-bit call number 60 = 0x3c). The value in `edi` before it is the exit status.

</details>

**8. The program runs and then crashes with a segmentation fault after printing. What is the most likely cause?**

<details>
<summary>Answer</summary>

The program has no `exit` call at the end, so the processor keeps executing the bytes after the last instruction.

</details>
