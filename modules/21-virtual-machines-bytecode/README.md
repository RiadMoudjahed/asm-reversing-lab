[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# 21. Virtual machines and bytecode

Time: about 2.5 hours. Prerequisites: [20. Reading obfuscated code](../20-reading-obfuscated-code/README.md)

## Why this matters

Some programs do not contain their logic as machine code. They contain a small processor written in software, an interpreter, and a block of data that it executes, the bytecode. Languages use this design to run on any hardware, applications use it for scripts and macros, and protected or obfuscated programs use it to hide what they do: the real logic is a stream of bytes for an instruction set that exists nowhere else, and a disassembler shows only the interpreter. The method for reading such a program is always the same. You treat the interpreter as the documentation of an unknown processor, you recover its instruction set from the code of its handlers, and you write a disassembler for it. This module teaches that method on two small, harmless machines written for the lesson. The module is about understanding programs; the machines in it do nothing except compute and print numbers.

## Concepts

### What a virtual machine is

A virtual machine here means a program that behaves like a processor: it has a state, it reads instructions from memory, and it carries them out one by one. The state is small:

| Part | Meaning |
|---|---|
| Program counter | The offset of the next bytecode instruction. It is an ordinary variable, not the `rip` register. |
| Data storage | A stack, or a set of virtual registers, or both. |
| Memory | Variables, or a byte array that the bytecode can read and write. |
| Flags and status | A halted indicator, an error indicator, sometimes comparison results. |
| The program | The bytecode: an array of bytes read by the interpreter. It is data, not code. |

The interpreter is a loop that does the same three things for every instruction: it fetches the opcode (the number that selects the operation) at the program counter, decodes it together with its operands, and executes it by running the code of one handler. The loop is the same for every program. Only the bytecode changes.

The consequence for reverse engineering is that the useful information moves. In an ordinary program the disassembly is the logic. In a program with a virtual machine the disassembly is the same for every program that uses the machine, and the logic is in the bytecode. You will read the interpreter once, and then read the bytecode with a tool you build.

### Two designs

The two example machines represent the two common designs.

| | Stack machine | Register machine |
|---|---|---|
| Operands | Implicit: operations take their inputs from the top of a stack and push the result | Explicit: instructions name the registers |
| Instruction size | Short; a few instructions carry one operand byte | Usually a fixed size with several fields |
| Typical code | `push 3`, `push 4`, `add` | `add r1, r2, r3` |
| In the interpreter | A stack array and a stack index | An array of registers; the field of the instruction is an index into it |
| Easy to recognize by | An index that goes up and down in almost every handler | Handlers that read two or three small fields and index one array |

The first machine, `examples/21_stackvm.c`, is a stack machine with 11 instructions, one opcode byte each, and some with one operand byte. It reads its program from a file. The second, `examples/21_regvm.c`, is a register machine with four registers and fixed 4-byte instructions. Both are described in the comments at the top of their sources; in the lesson you will recover the instruction sets from the compiled programs, as you would for an unknown sample.

### Ways to dispatch

Dispatch is the step from the opcode to its handler. It is the part of the interpreter that is easiest to recognize.

| Style | In the source | In the disassembly |
|---|---|---|
| A `switch` | `switch (op) { case 0: ... }` | A bound check (`cmp`, then `ja` to the default case), then an indirect `jmp` through a table of code addresses |
| A table of handlers | `table[op](state, instruction)` | A load of a pointer from an array indexed by the opcode, a test for a null pointer, then an indirect `call` |
| Threaded dispatch | Each handler ends by jumping to the next one | Many indirect jumps, one at the end of each handler, and no central loop |
| A chain of comparisons | `if (op == 1) ... else if (op == 2) ...` | A series of `cmp` and conditional jumps; slow, found in simple or deliberately bare interpreters |

The first two are the subject of the examples. In both cases the key object is the table: an array of pointers in the read-only data section. Reading the table gives you, for each opcode, the address of its handler. An opcode with no handler has a zero entry (the table of handlers) or goes to the error case (the `switch`). Several opcodes may share a handler.

### Recognizing a virtual machine

Look for these signs together; none is enough alone.

- A large function with one hub: a loop containing an indirect jump or call, and many small blocks that jump back to the top of the loop.
- A load of one byte (or one word) from a buffer at a variable position, followed by a bound check against a small number.
- A table of code addresses in the read-only data (a list of values that all point into the text section).
- A buffer of bytes in the data section, or a file read at start, with no readable strings in it and a structure that repeats.
- Handlers that are short and each change the same few variables: the stack index, the registers array, the program counter.
- A program counter that is changed in only a few handlers (jumps), and incremented by a fixed amount elsewhere.

### Recovering the instruction set

The method is a sequence you can apply to any interpreter.

1. **Find the state.** Identify the variables that every handler reads or writes. One of them advances with every instruction (the program counter). One is an index that goes up and down (a stack pointer) or an array indexed by a small field (registers). The others are the memory and the status.
2. **Find the dispatch.** Find the load of the opcode and the indirect transfer that follows it. Note the register that holds the opcode and the bound check, which tells you the number of opcodes.
3. **Read the table.** Dump the table from the data section and list each opcode with the address of its handler. Note the opcodes that share a handler or that are missing.
4. **Read each handler.** For each handler, write down which operands it consumes (how far it advances the program counter), what it reads, what it writes, and what the operation is. Handlers that share code may differ by a test on the opcode.
5. **Write the specification.** One row per opcode: number, name you choose, operands, effect. The names are yours; the behavior is the interpreter's.
6. **Write a disassembler** from the table (below), check it on bytecode whose behavior you know, and then use it on the bytecode you care about.

#### The stack machine in the disassembly

The stripped build has no names, and the interpreter is merged into `main`. This is the dispatch:

```bash
make
objdump -d -M intel --no-show-raw-insn build/21_stackvm_stripped | sed -n '/401286:/,/4012aa:/p'
```

```text
  401286:  lea    r12,[r8+0x1]
  40128a:  movzx  ecx,BYTE PTR [rsp+r8*1+0xa0]
  401293:  add    rbp,0x1
  401297:  cmp    cl,0xa
  40129a:  ja     4013e9 <__fprintf_chk@plt+0x369>
  4012a0:  movzx  eax,cl
  4012a3:  jmp    QWORD PTR [rax*8+0x402078]
```

`r8` is the program counter: the opcode is loaded from the buffer at `[rsp+r8+0xa0]`, and the program counter plus one is kept in `r12`. `rbp` is incremented for every instruction, so it is a step counter. The opcode is compared with 0xa, and anything above goes to the error case, so there are 11 opcodes, 0 to 10. The table is at 0x402078. Dumping it gives the handler of every opcode:

```bash
objdump -s -j .rodata build/21_stackvm_stripped
```

```text
 402070 00000000 00000000 37124000 00000000  ........7.@.....
 402080 55124000 00000000 aa124000 00000000  U.@.......@.....
 402090 aa124000 00000000 aa124000 00000000  ..@.......@.....
 4020a0 e8124000 00000000 09134000 00000000  ..@.......@.....
 4020b0 44134000 00000000 7e134000 00000000  D.@.....~.@.....
 4020c0 95134000 00000000 c0134000 00000000  ..@.......@.....
```

Read the eight-byte values from 0x402078, little-endian. Opcode 0 goes to 0x401237, opcode 1 to 0x401255, opcodes 2, 3 and 4 all to 0x4012aa, opcode 5 to 0x4012e8, and so on. Three opcodes share one handler: that handler must test the opcode to choose between three operations, and the code at 0x4012aa begins with `cmp rbx,0x1` and `jbe`, a check that the stack has two values, followed by two pops. That is the shape of the binary operations. Reading the other handlers the same way gives the specification in the comments of the source; the handler of opcode 1 reads one more byte and increments the program counter by 2, the handlers of 8 and 9 assign the program counter from the operand, and the handler of 10 calls `printf` with the format string `%d`.

Following the program counter is the way to decide the size of an instruction. A handler that reads the byte after the opcode consumes one operand. If you make a mistake in the size of one instruction, every instruction after it in the disassembly is wrong, which is the sign to look for: a disassembly that starts to produce nonsense in the middle of a program is a size that was wrong before that point.

#### The register machine in the disassembly

The register machine uses a table of handlers. The call looks like this:

```text
  4012b5:  lea    rsi,[rsp+r8*4+0x20]
  4012ba:  movzx  ecx,BYTE PTR [rsi]
  4012bd:  movsxd rax,ecx
  4012c0:  mov    rax,QWORD PTR [rax*8+0x4020a0]
  4012c8:  test   rax,rax
  4012cb:  je     401359 <__fprintf_chk@plt+0x2d9>
  4012d1:  mov    rdi,rsp
  4012d4:  call   rax
```

The address of the instruction is computed as base plus four times the program counter, which shows that instructions are 4 bytes wide. The opcode is the first byte of the instruction, the table is at 0x4020a0, a null entry goes to the error case, and the handler is called with `rdi` pointing at the machine state and `rsi` pointing at the instruction. The table has 256 entries, because the opcode is a whole byte, and only seven of them are not zero:

```text
0x10 -> 0x401176
0x11 -> 0x40118e
0x12 -> 0x4011ad
0x13 -> 0x4011cc
0x14 -> 0x4011ec
0x15 -> 0x40120a
0xff -> 0x401202
```

The handlers are short and regular. This is the handler for opcode 0x11:

```text
  40118e:  movzx  eax,BYTE PTR [rsi+0x1]
  401192:  and    eax,0x3
  401195:  movzx  ecx,BYTE PTR [rsi+0x2]
  401199:  and    ecx,0x3
  40119c:  movzx  edx,BYTE PTR [rsi+0x3]
  4011a0:  and    edx,0x3
  4011a3:  mov    edx,DWORD PTR [rdi+rdx*4]
  4011a6:  add    edx,DWORD PTR [rdi+rcx*4]
  4011a9:  mov    DWORD PTR [rdi+rax*4],edx
  4011ac:  ret
```

The three bytes after the opcode are masked with 3 and used as indexes into an array of 32-bit values at the start of the state: a register file with four registers. The handler adds the registers named by the second and the third field and stores the result in the register named by the first. That is `add a, b, c`. The mask means that a register number above 3 wraps around instead of failing, a detail that belongs in the specification. The handler at 0x40118e differs from the ones at 0x4011ad and 0x4011cc only in the operation (`sub`, `imul`). The handler at 0x4011ec compares a register with zero and, if it is not zero, writes a byte of the instruction into a field of the state at offset 0x10: that field is the program counter, so this is a conditional jump. The handler at 0x401202 sets a field at offset 0x18, the halted indicator.

### Reading the bytecode

With the specification you can read a program. The assembler `examples/21_asm.py` builds the sample programs from text, and the disassembler `examples/21_disasm.py` prints them back. The stack machine program in `programs/fact.asm` computes a product in a loop and prints it:

```bash
python3 21_asm.py stack programs/fact.asm build/fact.bin
xxd build/fact.bin
build/21_stackvm build/fact.bin
```

```text
build/fact.bin: 32 bytes, 18 instructions
00000000: 0105 0700 0101 0701 0600 091c 0601 0600  ................
00000010: 0407 0106 0001 0103 0700 0808 0601 0a00  ................
120
halt after 64 steps
```

The bytes do not show anything by themselves. The disassembler turns them into a listing:

```bash
python3 21_disasm.py stack build/fact.bin
```

```text
0000  01 05           push 5
0002  07 00           store 0
0004  01 01           push 1
0006  07 01           store 1
0008  06 00           load 0
000a  09 1c           jz @28
000c  06 01           load 1
000e  06 00           load 0
0010  04              mul
0011  07 01           store 1
0013  06 00           load 0
0015  01 01           push 1
0017  03              sub
0018  07 00           store 0
001a  08 08           jmp @8
001c  06 01           load 1
001e  0a              print
001f  00              halt
```

The listing has the structure of a loop: the jump at 0x1a goes back to offset 8, the conditional jump at 0x0a leaves the loop to offset 0x1c when the variable 0 is zero, and the body multiplies variable 1 by variable 0 and decrements variable 0. The program prints the factorial of 5. Notice that the jump operands are byte offsets, not instruction numbers, and that a disassembler that prints them as `@28` is more useful if it also marks the target line with a label.

### Writing a disassembler

A disassembler for a custom instruction set has three parts: a table that describes the instruction set, a loop that walks the bytes, and a formatter. The code in `21_disasm.py` has this shape:

```c-fragment
for each position in the bytecode:
    look up the opcode in the table
    if it is not in the table:
        print the byte as data and move on by one
    read as many operand bytes as the table says
    print the address, the bytes, the name and the formatted operands
    move on by 1 + the number of operand bytes
```

Keep the instruction set as data, one table per machine, and keep the loop generic. When you recover another machine you fill in a new table, and you do not touch the loop. Four points make the difference between a disassembler that works and one that only looks right:

- **Unknown bytes.** Never stop at a byte that is not an opcode. Print it as data and continue, so that you see all of the program and notice where your table is incomplete.
- **Instruction size.** The size is part of the table. A wrong size shifts everything after it, so check the output against a program whose behavior you know, such as one that prints a result you can compute by hand.
- **Operand meaning.** A register number, an immediate value, a target offset and a variable number are different kinds of operand. Print each kind in its own way (`r2`, `5`, `@28`).
- **Targets.** For a jump, the useful output is the line it leads to. Collect the targets in a first pass and print a label at each of them in a second.

The same disassembler handles the register machine with a second table. Its output for `programs/sum.asm`:

```bash
python3 21_asm.py reg programs/sum.asm build/sum.bin
python3 21_disasm.py reg build/sum.bin
build/21_regvm build/sum.bin
```

```text
build/sum.bin: 32 bytes, 8 instructions
0000  10 00 0a 00     movi r0, 10
0004  10 01 00 00     movi r1, 0
0008  10 02 01 00     movi r2, 1
000c  11 01 01 00     add r1, r1, r0
0010  12 00 00 02     sub r0, r0, r2
0014  14 00 03 00     jnz r0, @3
0018  15 01 00 00     out r1
001c  ff 00 00 00     halt
55
halt after 35 steps
```

Here the jump target is an instruction number, `@3`, not a byte offset: the disassembler of a machine with fixed-size instructions can use either, and the specification must say which the machine uses.

### Tracing the machine

A static reading gives you the instruction set. To check it, and to follow a program you cannot yet disassemble, trace the machine while it runs: break at the dispatch and print the opcode every time. The script `examples/21_trace.gdb` does this for the stack machine. The address of the dispatch jump comes from the disassembly of this exact build, so it is the one place where you adapt the script to a different binary:

```bash
gdb -nx -q -batch -x 21_trace.gdb build/21_stackvm_stripped 2>&1 | grep "^offset" | head -9
```

```text
offset   0  opcode 01  stack depth 0
offset   2  opcode 07  stack depth 1
offset   4  opcode 01  stack depth 0
offset   6  opcode 07  stack depth 1
offset   8  opcode 06  stack depth 0
offset  10  opcode 09  stack depth 1
offset  12  opcode 06  stack depth 0
offset  14  opcode 06  stack depth 1
offset  16  opcode 04  stack depth 2
```

The trace is a list of the instructions executed, in order, and it matches the listing: push 5, store 0, push 1, store 1, load 0, a conditional jump, and so on. The whole run is 64 lines, the same number as the steps that the program reports. A trace is also the way around the problem of a bytecode you cannot parse yet: it shows only the instructions that actually run, with the correct sizes, even if you do not know the instruction set completely.

### A working order

1. **Triage.** Check for the signs of a virtual machine: the large function with a hub, the table of code addresses, the buffer with no strings.
2. **Find the state and the dispatch.** Name the program counter, the stack index or the register array, and the load of the opcode.
3. **Dump the table.** List each opcode and its handler. Note shared and missing entries.
4. **Read the handlers.** Write one row of the specification for each, with the operands it consumes.
5. **Disassemble.** Fill in the table of your disassembler and check it against a trace.
6. **Read the bytecode.** Find loops, calls and the instructions that produce output, and describe the behavior in terms of what the program does.

## Try it

Run the lab commands from `modules/21-virtual-machines-bytecode/examples`.

1. Build with `make` and run both programs. Disassemble `build/fact.bin` and `build/sum.bin`, and describe in a sentence what each program computes.
2. Disassemble `build/21_stackvm_stripped`. Find the load of the opcode, the bound check and the indirect jump. How many opcodes does the machine have? Which opcodes share a handler?
3. Dump the table of the register machine from `build/21_regvm_stripped` and write down the handler address of each opcode. Read the handler of opcode 0x14 and describe its effect in one line.
4. Run `21_trace.gdb` and compare the first lines with the disassembly. Change the program in `programs/fact.asm` (for example, start from 6), rebuild it and compare the trace length with the step count the program reports.
5. Write a program of your own for the register machine that prints 2 to the power of 10 by repeated multiplication in a loop, using only the instructions in the table. Assemble it with `21_asm.py` and run it.
6. Extend the stack machine in your own copy of `21_stackvm.c` with a new opcode, for example `swap`. Add it to the table in `21_disasm.py`. Which parts of the disassembler did you change?
7. Mark the jump targets: change `21_disasm.py` so that it prints a label at each line that a jump leads to, and prints the label instead of the offset in the jump.

## Common pitfalls

- **Reading the interpreter as the program.** The interpreter is the same for every program; the logic is in the bytecode. If you spend your time on the handlers after you have the specification, you are not reading the program.
- **A wrong instruction size.** One wrong size makes everything after it nonsense. Compare with a trace, and look for the point where the output turns to nonsense.
- **Missing the shared handlers.** Several opcodes can share one handler and select the operation by a test inside it. Read the table before the handlers, and look for repeated addresses.
- **Ignoring masks and bounds.** The `and 3` on a register number, the bound check on the opcode and the check on the stack depth are part of the behavior. Put them in the specification, and test what happens at the edges.
- **Confusing the two program counters.** The `rip` register runs the interpreter; the virtual program counter indexes the bytecode. The targets of jumps in the bytecode are values of the second.
- **Trusting a table address from another build.** The addresses in a trace script or a note belong to one binary. Look them up again for each build.

## Summary

- A virtual machine is an interpreter loop with a state and a block of bytecode; the disassembly shows the interpreter, and the logic is in the bytecode.
- Machines are stack machines (implicit operands) or register machines (named operands); dispatch is a `switch`, a table of handlers, or threaded jumps.
- You recognize one by a hub loop, a table of code addresses, a byte load with a bound check and short handlers that change the same few variables.
- Recovering the instruction set means finding the state, finding the dispatch, dumping the table and reading each handler into one row of a specification.
- A disassembler is a table plus a generic loop; it prints unknown bytes as data, and it is checked against a trace of the running machine.

## Next steps

- Take the [quiz](quiz.md).
- Continue with module 22, managed code.
- Exercises that use this module: [Exercise 20](../../exercises/20-capstone-tiny-vm/README.md).
