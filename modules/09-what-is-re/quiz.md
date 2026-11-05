[English](quiz.md) | [العربية](quiz.ar.md) | [Deutsch](quiz.de.md) | [Español](quiz.es.md) | [Français](quiz.fr.md)

# Quiz: 09. What reverse engineering is

Answer first, then open the answer.

**1. What is the goal of reverse engineering, and how does it differ from rewriting a program?**

<details>
<summary>Answer</summary>

The goal is understanding: recovering how a program works from the program itself. Rewriting produces new code; it may follow from understanding, but it is not the aim.

</details>

**2. Name three legitimate uses of reverse engineering.**

<details>
<summary>Answer</summary>

Any three of: interoperability, security audit, malware analysis, recovering lost knowledge, debugging without source, learning how a compiler translates code.

</details>

**3. What three things should you check before analyzing a program that is not yours?**

<details>
<summary>Answer</summary>

Who owns it, what its license or contract says about analysis, and what the law in your country says.

</details>

**4. Give one strength and one weakness of static analysis.**

<details>
<summary>Answer</summary>

Strength: it is safe and sees all code, including paths that never run. Weakness: it cannot see values that are computed at run time, and obfuscated code is hard to read.

</details>

**5. Why is dynamic analysis never done on the machine that holds your personal files?**

<details>
<summary>Answer</summary>

Because it runs code you do not fully understand yet. Use a disposable environment, such as the lab container for this course, or a virtual machine with no network and a snapshot for real samples.

</details>

**6. Put these steps in order: read and name, triage, report, observe, test the idea, form a question, find the code.**

<details>
<summary>Answer</summary>

Triage, observe, form a question, find the code, read and name, test the idea, report.

</details>

**7. After triage of a file you see `statically linked, not stripped` and the string `NASM`. What can you conclude, and what can you not?**

<details>
<summary>Answer</summary>

You can conclude that the file has no shared library dependencies, that symbol names are still present, and that it was probably assembled with NASM. You cannot conclude what the program does; strings and headers are hints that code analysis or observation must confirm.

</details>
