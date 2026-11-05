[English](quiz.md) | [العربية](quiz.ar.md) | [Deutsch](quiz.de.md) | [Español](quiz.es.md) | [Français](quiz.fr.md)

# Quiz: 21. Virtual machines and bytecode

Answer first, then open the answer.

**1. In a program that contains a virtual machine, where is the logic that the program carries out?**

<details>
<summary>Answer</summary>

In the bytecode, which is data that the interpreter reads. The disassembly shows the interpreter, which is the same for every program that uses the machine. To understand the program you recover the instruction set from the interpreter and then read the bytecode.

</details>

**2. What are the three steps that the interpreter loop repeats for every instruction?**

<details>
<summary>Answer</summary>

It fetches the opcode at the virtual program counter, decodes it together with its operands, and executes it by running the handler for that opcode. The virtual program counter is an ordinary variable, not the `rip` register.

</details>

**3. How does a stack machine differ from a register machine in the code of its interpreter?**

<details>
<summary>Answer</summary>

In a stack machine the operands are implicit: the handlers work on the top of a stack and change a stack index, so an index that goes up and down appears in almost every handler. In a register machine the instruction names its registers: the handlers read small fields of the instruction and use them as indexes into an array of registers.

</details>

**4. What does a `switch` dispatch look like in the disassembly, and what does a table of handlers look like?**

<details>
<summary>Answer</summary>

A `switch` shows as a bound check on the opcode (`cmp` and `ja` to the default case) followed by an indirect `jmp` through a table of code addresses. A table of handlers shows as a load of a pointer from an array indexed by the opcode, a test for a null pointer, and an indirect `call`.

</details>

**5. You find a table of 11 code addresses and three of them are equal. What does that tell you?**

<details>
<summary>Answer</summary>

Three opcodes share one handler. The handler must then test the opcode to choose between the operations, as the handler of `add`, `sub` and `mul` does in the stack machine of the lesson. It also means that the table has to be read before the handlers, so that you do not describe the same code three times.

</details>

**6. Your disassembler of a custom bytecode prints sensible instructions at the start and nonsense after a certain point. What is the most likely cause, and how do you check it?**

<details>
<summary>Answer</summary>

An instruction before that point has the wrong size in your table, so every instruction after it is read from the wrong position. Check by tracing the machine while it runs, which gives the instructions that really execute with the right sizes, or by testing the disassembler on a program whose behavior you know.

</details>

**7. Why should a disassembler print an unknown byte as data and continue instead of stopping?**

<details>
<summary>Answer</summary>

Stopping hides the rest of the program. Printing the byte as data and moving on by one lets you see everything and notice where the table of the instruction set is incomplete or wrong.

</details>

**8. In the register machine of the lesson, the register numbers are masked with 3. What does this mean for the specification?**

<details>
<summary>Answer</summary>

A register number above 3 does not cause an error; it wraps around and names one of the four registers. This is behavior of the machine and belongs in the specification, together with the other masks and bounds.

</details>
