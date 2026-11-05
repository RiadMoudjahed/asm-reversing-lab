[English](quiz.md) | [العربية](quiz.ar.md) | [Deutsch](quiz.de.md) | [Español](quiz.es.md) | [Français](quiz.fr.md)

# Quiz: 10. Operating system fundamentals for reversers

Answer first, then open the answer.

**1. What do threads of one process share, and what does each thread have of its own?**

<details>
<summary>Answer</summary>

They share the address space and open files. Each thread has its own registers and its own stack.

</details>

**2. What happens when code writes to a page that is read-only?**

<details>
<summary>Answer</summary>

The processor raises a fault, and the kernel normally ends the process with a signal (`SIGSEGV` on Linux), unless the program handles the signal.

</details>

**3. Why should you record offsets and names instead of absolute addresses?**

<details>
<summary>Answer</summary>

Address space layout randomization places the stack, heap and libraries at different addresses on every run, so an absolute address from one run is wrong in the next.

</details>

**4. Why is `main` not the first code that runs in a program?**

<details>
<summary>Answer</summary>

The loader and the start-up code run first: they map libraries, resolve references and set up the runtime. The start-up code then calls `main`.

</details>

**5. A program calls `printf` three times and `strace` shows one `write`. Explain.**

<details>
<summary>Answer</summary>

The C library buffers output. `printf` fills a buffer, and the library issues one `write` when the buffer is flushed. `ltrace` would show three calls to `printf`.

</details>

**6. Name the layers a Windows API call can pass through before it reaches the kernel.**

<details>
<summary>Answer</summary>

The Windows API (`kernel32.dll` and similar), the native API in `ntdll.dll` (the `Nt...` functions), and then the `syscall` instruction into the kernel.

</details>

**7. What is a handle on Windows, and what is its counterpart on Linux?**

<details>
<summary>Answer</summary>

A handle is a number that indexes a per-process table of kernel objects. The Linux counterpart is the file descriptor. In both cases the number means nothing by itself; the object behind it does.

</details>
