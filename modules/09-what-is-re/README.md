[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# 09. What reverse engineering is

Time: about 1 hour. Prerequisites: [08. The stack, procedures and calling conventions](../08-stack-and-calls/README.md)

## Why this matters

Modules 00 to 08 taught you to write assembly. Reverse engineering is the same skill used in the other direction: you start from a finished program and work out what it does and how. Before the tools and formats of the next modules, you need a clear definition, a set of legitimate uses, the boundaries you must respect, and a repeatable way of working that keeps you from getting lost in a binary.

## Concepts

### Definition

**Reverse engineering** is recovering the design, behavior or data of a product from the finished product itself, without the original design documents. For software, the product is usually an executable file, a library or a firmware image, and the result is knowledge: a description of a file format, the logic of a function, the list of actions a program performs.

It is not the same as writing the program again. The goal is understanding. Rewriting is sometimes a later step, as in exercise 08 and exercise 09, where you re-implement functions you first understood.

### Legitimate uses

| Use | Example |
|---|---|
| Interoperability | Documenting an undocumented file format so another tool can read it |
| Security audit | Checking a closed-source component for memory-safety bugs |
| Malware analysis | Finding out what a hostile sample does so defenders can detect and remove it |
| Recovering lost knowledge | Understanding a tool whose source code is gone |
| Debugging | Finding why a crash happens in code you cannot rebuild with symbols |
| Learning | Seeing what a compiler does with your own code |

### Boundaries

This course teaches analysis and understanding. It does not teach, and its exercises never ask for:

- developing exploits, shellcode or malware;
- defeating copy protection, licensing or digital rights management;
- any activity against systems or software you do not have the right to examine.

Before you analyze a program that is not yours, check three things: who owns it, what the license or contract says about analysis, and what the law in your country says. Interoperability and security research have specific legal protections in some places and none in others. The rules differ between countries and change over time, so this course does not give legal advice; when the situation is not clear, ask a qualified person first. Every binary in this repository is built from source included in the repository, so working on the exercises is always permitted. See [ETHICS.md](../../ETHICS.md).

### Two levels of reversing

**System-level** reversing looks at how a program behaves from the outside: which files it opens, which network connections it makes, which system calls it issues, what it writes to disk. The program is treated as a black box and observed.

**Code-level** reversing reads the program's instructions to learn exactly how it computes. It is slower and more precise. Real investigations alternate between the two: the outside view tells you where to look, and the code view tells you why.

### Static and dynamic analysis

| | Static analysis | Dynamic analysis |
|---|---|---|
| What you do | Read the file without running it | Run the program and observe it |
| Tools | `file`, `strings`, `readelf`, `objdump`, a disassembler, a decompiler | A debugger, `strace`, `ltrace` |
| Strengths | Safe; sees all code, including paths that never run | Shows real values; unpacks data the program decodes itself |
| Weaknesses | Cannot see values computed at run time; obfuscated code is hard to read | Sees only the paths that actually run; running unknown code needs a safe environment |

Neither is better. A good workflow uses static analysis to form a question and dynamic analysis to answer it, then returns to static analysis with the new facts.

### A repeatable workflow

1. **Triage.** Identify the file without running it: type, architecture, size, hash, strings, imported functions, sections. This takes minutes and often answers the question or shows where to look.
2. **Observe.** Run the program in a safe place and record what it does from the outside: output, system calls, files.
3. **Form a question.** Write down one specific thing you need to know, for example "where is the input checked?" or "what is the layout of this record?".
4. **Find the code.** Use strings, imports and system calls as starting points and follow cross-references to the code that matters.
5. **Read and name.** Read the instructions, work out what each function does, and give it a meaningful name. Names are the main output of the work.
6. **Test the idea.** Check your understanding with a debugger, by stepping through the code with chosen inputs, or by re-implementing the function and comparing results.
7. **Report.** Record what you found, how you know, and what remains unknown. Separate facts from guesses.

Take notes as you go. Memory fails quickly in a large binary, and a note such as "the function at 0x401136 copies the header into a buffer of 32 bytes" saves an hour the next day.

### A safe place to run programs

Run unknown programs only in an environment that you can throw away. For this course the lab container is enough, because all binaries come from this repository. For real samples, use a virtual machine with no network and a snapshot to return to. Never run an unknown sample on the machine that holds your files.

## Try it

Triage the smallest program in the course without running it. The commands run from `modules/03-first-program/examples`, after `make` has built the examples.

```bash
file build/03_hello64
strings -n 4 build/03_hello64 | head -6
readelf -h build/03_hello64 | grep -E "Type|Entry"
```

Real output:

```text
build/03_hello64: ELF 64-bit LSB executable, x86-64, version 1 (SYSV), statically linked, with debug_info, not stripped
Hello, assembly!
03_hello64.asm
/lab/modules/03-first-program/examples/
NASM 2.16.01
03_hello64.asm
YYuY0Y/
  Type:                              EXEC (Executable file)
  Entry point address:               0x401000
```

Without running anything, you know that this is a 64-bit Linux program for x86-64 with no libraries, that it was written in assembly with NASM (the producer string and the source file name are left in the file, along with the build directory), that it was not stripped of symbols, and that it prints a greeting. That is triage.

Now observe it from the outside:

```bash
strace -q build/03_hello64
```

```text
execve("build/03_hello64", ["build/03_hello64"], 0x7ffcb16ebed8 /* 15 vars */) = 0
write(1, "Hello, assembly!\n", 17Hello, assembly!
)      = 17
exit(0)                                 = ?
+++ exited with 0 +++
```

The system calls confirm what the strings suggested: one `write` of 17 bytes to standard output, then `exit`. The program's own output is mixed into the trace because both go to the terminal. This is system-level reversing; reading the instructions would be code-level.

Write down in three lines what you know about this file and how you know it. That is the habit to build.

## Common pitfalls

- **Starting in the disassembler.** Without triage you read code with no idea what the program is for. Spend the first minutes on the outside view.
- **Reading everything.** A binary can hold thousands of functions. Follow the question, not the file order.
- **Trusting names.** Symbol names and strings can be wrong or deliberately misleading. Treat them as hints that need confirmation.
- **Running unknown code on your own machine.** Use a disposable environment.
- **Mixing facts and guesses in notes.** Mark what you verified and what you assumed.

## Summary

- Reverse engineering recovers how a program works from the program itself; the goal is understanding.
- Legitimate uses include interoperability, security audit, malware analysis, recovering lost knowledge and learning. Check ownership, license and law before analyzing software that is not yours.
- Static analysis reads the file; dynamic analysis runs it. Good work alternates between them.
- A workflow of triage, observation, a question, finding the code, naming, testing and reporting keeps the work focused.
- Run unknown programs only in a disposable environment.

## Next steps

- Take the [quiz](quiz.md).
- Continue with module 10, operating system fundamentals for reversers.
