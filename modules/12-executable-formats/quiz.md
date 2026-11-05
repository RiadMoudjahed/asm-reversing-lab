[English](quiz.md) | [العربية](quiz.ar.md) | [Deutsch](quiz.de.md) | [Español](quiz.es.md) | [Français](quiz.fr.md)

# Quiz: 12. Executable formats: ELF and PE

Answer first, then open the answer.

**1. What is the difference between a file offset and a virtual address?**

<details>
<summary>Answer</summary>

A file offset counts bytes from the start of the file. A virtual address is where those bytes appear in the process's memory after loading. They differ because of page alignment and because some sections, such as `.bss`, take memory but no file space.

</details>

**2. Which part of an ELF file does the loader use to map it, and which part do tools such as the linker and debugger use?**

<details>
<summary>Answer</summary>

The loader uses the program headers (segments). The linker, debugger and reverser use the section headers. A file can run with its section table removed.

</details>

**3. A `LOAD` segment has file size 0x22c and memory size 0x230. What does the difference mean?**

<details>
<summary>Answer</summary>

The extra 4 bytes are zero-initialized memory with no file space: the `.bss`.

</details>

**4. Explain what happens on the first call to `puts@plt` with lazy binding.**

<details>
<summary>Answer</summary>

The stub jumps through its GOT slot, which initially points back to the next instruction in the stub. That instruction pushes the relocation index and jumps to the shared stub, which enters the dynamic loader's resolver. The resolver finds `puts`, writes its address into the GOT slot and jumps to it. Later calls go straight to `puts`.

</details>

**5. In a PE file, the entry point RVA is 0x2150, and the section containing it has RVA 0x2000 and raw offset 0xa00. What is the file offset?**

<details>
<summary>Answer</summary>

0x2150 - 0x2000 + 0xa00 = 0xb50.

</details>

**6. Which PE structures play the role of the ELF PLT and GOT?**

<details>
<summary>Answer</summary>

The import directory with the import name table and the import address table. The loader fills the IAT before the program starts, and the program calls through it.

</details>

**7. Name two fields the loader must find in a minimal 64-bit ELF executable.**

<details>
<summary>Answer</summary>

Any two of: the magic number and class, the machine type, the entry point, the program header table offset, size and count, and in the program header the type, flags, offset, virtual address and sizes of the `LOAD` segment.

</details>
