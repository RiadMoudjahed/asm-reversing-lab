[English](HINTS.md) | [العربية](HINTS.ar.md) | [Deutsch](HINTS.de.md) | [Español](HINTS.es.md) | [Français](HINTS.fr.md)

# Hints: exercise 20

Open one level at a time. Try again between levels.

## Part 1: Reverse

<details>
<summary>Level 1: nudge</summary>

Start with the dispatch, as in module 21. Find where the program reads the first byte of an instruction and then jumps through a table. Before the jump the byte goes through something else: look for a read from a small table indexed by the byte. In the read-only data there are three tables near each other: one of 256 bytes, one with a byte per instruction, and one with code addresses. Dump them.

</details>

<details>
<summary>Level 2: direction</summary>

The table of 256 bytes turns an opcode byte into a small number, with 0 for a byte that is not an instruction. Every non-zero entry is a valid opcode, and the entry is the index of the handler in the table of code addresses. The table with a byte per instruction holds the length. For each handler, find what it reads from the instruction bytes and which parts of the state it changes. The state is a structure on the stack of the main function; work out its layout from the offsets that the handlers use, and write it down. To find the register fields, look at the shifts and masks applied to the byte after the opcode, and at the order of the two 16-bit bytes in the handlers that take a number or a target.

</details>

<details>
<summary>Level 3: nearly the answer</summary>

Match the handlers to names by what they do: the handler that stores the return address and jumps is `call`, the one that makes the output call with a character format is `outc`, and so on. Three of the arithmetic handlers differ only in the operation. Read the flag updates with care: some instructions set both flags, some set one, and shifts leave the flags alone in one case. For `I`, write the disassembler from your table and count the lines for `bin/sample.tvm`; the code length is in the header, and an instruction is as long as its entry in the length table says. For `N`, count the non-zero entries of the 256-byte table. For `C`, find the index of the handler that pushes a return address and look up which byte of the 256-byte table holds that index.

</details>

## Part 2: Assemble

<details>
<summary>Level 1: nudge</summary>

Do the handlers one at a time and test each, so that the checker's report is short. The registers of the machine are an array at the start of the structure, so a register field turns into an offset by multiplying by 4. The first thing every handler does is take the byte after the opcode apart. For the program, think of the memory as an array of twelve bytes and write the numbers with a loop.

</details>

<details>
<summary>Level 2: direction</summary>

Take the register numbers from the nibbles of the byte after the opcode, with the mask that the original applies. Use `setcc` to turn a comparison into a 0 or 1 that you store in a flag. Read `shl` and `shr` from the original for the carry flag: it is not the same bit in both, and a count of zero is special. For `call` and `ret` follow the depth: the return address goes to the slot at the current depth, and `ret` takes it back from the slot below. The checker prints the fields that differ, with the value before the instruction, which tells you what you did and what you missed. For the program: a loop that adds the last two numbers and stores each one, then a subroutine that walks the memory downwards and prints.

</details>

<details>
<summary>Level 3: nearly the answer</summary>

For `cmp`, the carry flag is set when the first value is smaller than the second as unsigned numbers, and the zero flag when they are equal; nothing is stored. For `mul`, only the low 32 bits are kept, and the carry flag is cleared. For `ldb` and `stb` only the low byte of the address register is used, and `stb` stores only the low byte of the other register. In `shl`, the carry flag is the last bit that leaves at the top; in `shr`, the last bit that leaves at the bottom. The program counter that `call` stores is the one that was already advanced. For the program: the machine starts with every register at zero, so a register that you never set can serve as the constant 0 in a comparison; compare the counter with it to stop the loop that prints.

</details>
