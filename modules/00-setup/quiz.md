[English](quiz.md) | [العربية](quiz.ar.md) | [Deutsch](quiz.de.md) | [Español](quiz.es.md) | [Français](quiz.fr.md)

# Quiz: 00. Lab setup

Answer first, then open the answer.

**1. Where does your repository folder appear inside the lab, and why does that matter?**

<details>
<summary>Answer</summary>

At `/lab`. The folder is mounted, not copied, so a file you edit on your machine is immediately visible in the lab and the reverse.

</details>

**2. What does the `ld` command do that `nasm` does not?**

<details>
<summary>Answer</summary>

`nasm` assembles source code into an object file. `ld` is the linker: it turns one or more object files into an executable.

</details>

**3. After `./build/00_exit42`, how do you read the program's exit status in the shell?**

<details>
<summary>Answer</summary>

With `echo $?`, which prints the exit status of the last command. The smoke test prints 42.

</details>

**4. Why is a flag never stored in plain text in the exercise folders?**

<details>
<summary>Answer</summary>

So that reading the repository cannot reveal the answer. The checker stores a salted hash of the flag and compares your answer against it.

</details>

**5. You downloaded Ghidra on your host. Which path do you use to import `exercises/01-signal-flare/bin/signal_flare`: the host path or the `/lab` path?**

<details>
<summary>Answer</summary>

The host path. Ghidra runs on the host and cannot see the `/lab` mount that exists only inside the container.

</details>

**6. A check prints `Part 1 (Reverse): skipped, no --flag given`. What did you forget?**

<details>
<summary>Answer</summary>

To pass your flag: `make check EX=01 ARGS='--flag ARL{...}'`. Part 2 still runs without it.

</details>
