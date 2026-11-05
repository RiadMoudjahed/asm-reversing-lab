[English](quiz.md) | [العربية](quiz.ar.md) | [Deutsch](quiz.de.md) | [Español](quiz.es.md) | [Français](quiz.fr.md)

# Quiz: 11. The reverser's toolkit

Answer first, then open the answer.

**1. Which tool tells you what kind of file you have, and which prints the readable text inside it with file offsets?**

<details>
<summary>Answer</summary>

`file` identifies the type, and `strings -t x` prints the text with offsets in hexadecimal.

</details>

**2. In the section list, what do the flags `AX` and `WA` mean?**

<details>
<summary>Answer</summary>

`AX` is allocated and executable, which is code such as `.text`. `WA` is writable and allocated, which is data such as `.data` and `.bss`.

</details>

**3. Convert `movzbl %al,%eax` and `lea 0xea9(%rip),%rsi` to Intel syntax.**

<details>
<summary>Answer</summary>

`movzx eax, al` and `lea rsi, [rip + 0xea9]`. Reverse the operands, drop `%` and `$`, move the displacement into brackets, and replace the size suffix with an explicit mnemonic or size prefix.

</details>

**4. What does Rizin's `axt @ sym.check` list, and why is it useful?**

<details>
<summary>Answer</summary>

It lists the cross-references to `sym.check`, which are the places that call or reference it. It answers "who uses this?", the usual way to work from a string or function toward the code that matters.

</details>

**5. Why compare the decompiler's output with the instructions?**

<details>
<summary>Answer</summary>

The decompiler reconstructs C from machine code and can be wrong about types, loop forms and variable boundaries. The instructions are the ground truth.

</details>

**6. At a breakpoint on the first instruction of a function, where are the first two integer arguments in 64-bit System V code, and how do you print a string argument in GDB?**

<details>
<summary>Answer</summary>

In `rdi` and `rsi`. Print a string argument with `x/s $rdi`.

</details>

**7. A program compares your input with a fixed string. Name two quick ways to learn that string without reading the code.**

<details>
<summary>Answer</summary>

`ltrace` shows the arguments of `strcmp`, and `strings` lists text stored in the file. Neither works when the string is built or decoded at run time.

</details>
