[English](quiz.md) | [العربية](quiz.ar.md) | [Deutsch](quiz.de.md) | [Español](quiz.es.md) | [Français](quiz.fr.md)

# Quiz: 23. Decompilers and their limits

Answer first, then open the answer.

**1. Why does a decompiler not undo the compiler?**

<details>
<summary>Answer</summary>

The compiler discarded names, types, comments, the structure of loops and the choice of source constructs. The decompiler can only build a plausible program that has the same behavior, using guesses where the information is gone.

</details>

**2. Name the stages that a decompiler goes through.**

<details>
<summary>Answer</summary>

Disassembly and the control-flow graph, lifting (instructions to operations on variables, flags to conditions), data-flow analysis (removing assignments nothing reads, merging values into expressions, recovering locals), type recovery, structuring (rebuilding `if` and loops, with `goto` where that fails), and printing.

</details>

**3. What is a back edge, and what does it tell a decompiler?**

<details>
<summary>Answer</summary>

An edge from a block to a block that dominates it, that is, one that lies on every path from the entry to the first block. It marks a loop, with the target as the loop header.

</details>

**4. What problem does a decompiler face when a block changes a register that the block's own branch condition reads?**

<details>
<summary>Answer</summary>

The condition is built from the values the registers had at the start of the block, but the assignments come first in the printed text. If they are printed in that order the condition uses the new values. The decompiler must save the condition in a variable first or order the statements so that each reads old values. A mistake here changes the meaning without changing how the text reads.

</details>

**5. The compiled code reads a byte with `movsx`. A listing shows `p[0]` for an `unsigned char` pointer, as a plain `int`. What is wrong, and how did you see it?**

<details>
<summary>Answer</summary>

`movsx` is sign extension, so the byte is read as a signed number (a cast to `signed char`), and for 0x80 the value is -128. A plain `p[0]` of an `unsigned char` is zero-extended and gives 128. The mnemonic in the disassembly shows it.

</details>

**6. Why is `x & (x - 1) == 0` not the same as `(x & (x - 1)) == 0` in C?**

<details>
<summary>Answer</summary>

The comparison `==` binds tighter than the bitwise `&`, so the first means `x & ((x - 1) == 0)`. Dropping parentheses while rewriting or simplifying decompiler output can silently change the function.

</details>

**7. A reading of a function differs from the binary on only 3 of 215 test inputs. What does that show about testing?**

<details>
<summary>Answer</summary>

A test is only as strong as its inputs. The wrong reading agrees on all inputs that are not powers of two, and the test set has few of them. Include boundary values and the special cases that the logic singles out, not only random values.

</details>

**8. When should you read the instructions instead of the decompiled code?**

<details>
<summary>Answer</summary>

For small routines where every operation, width and flag matters (checksums, ciphers, bit tricks), hand-written assembly and instructions with no C counterpart, unusual control flow (jump tables not found, a virtual machine), output with many `goto` statements or changing types, and when the decompiler fails or gives a result that seems wrong.

</details>
