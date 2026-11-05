[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# Exercise 20. Capstone: Tiny VM

Level 5. Time: about 8 hours. Architecture: x86-64 Linux. Modules: [13. Recognizing code structures](../../modules/13-code-structures/README.md), [15. Recovering data](../../modules/15-recovering-data/README.md), [21. Virtual machines and bytecode](../../modules/21-virtual-machines-bytecode/README.md)

## Scenario

An old maintenance tool runs small programs on a virtual machine that nobody documented. The vendor is gone and the source is lost. What is left is the stripped interpreter `tinyvm` and one sample program, `sample.tvm`. Your job is to document the machine well enough to work with it: to write down its instruction set, to build the missing pieces of a replacement interpreter in assembly, and to write a new program for it with an assembler that is driven by your notes. If your notes are wrong anywhere, something in this chain breaks.

## Objectives

- Reverse engineer a bytecode interpreter: find the state, the dispatch, the table of opcodes and the behavior of every instruction, including flags, masks and error cases.
- Write a complete specification of the instruction set in a form that a tool can use.
- Implement eight instruction handlers in NASM that behave exactly like the original, on a state structure that you read from the binary.
- Write a bytecode program for the machine that produces a given output within a step limit.

## Part 1: Reverse

The files are `bin/tinyvm` and `bin/sample.tvm`. Run the sample to see what the machine does:

```bash
bin/tinyvm bin/sample.tvm
```

The machine takes one argument, a program file. The file has an 8-byte header and then the code and the data: the four bytes `TVM1`, the length of the code as a 16-bit number, the length of the data as a 16-bit number (both low byte first), the code, and the data. The data is copied to the start of the machine's 256 bytes of memory before the program runs. The step count that the machine prints on standard error is the number of instructions executed.

The machine has twenty instructions. Their names and operand forms are fixed, because your tools need them:

| Name | Operands |
|---|---|
| `movi` | a register and a number from 0 to 65535 |
| `mov`, `add`, `sub`, `mul`, `xor`, `and`, `cmp` | two registers |
| `shl`, `shr` | a register and a count from 0 to 15 |
| `jmp`, `jz`, `jnz`, `call` | a code offset |
| `ret`, `halt` | none |
| `ldb` | a register and a register used as an address: `ldb r1, [r2]` |
| `stb` | a register used as an address and a register: `stb [r1], r2` |
| `out`, `outc` | a register |

Everything else has to come from the binary: which opcode byte is which instruction, how many bytes each instruction occupies, which nibble holds the first register, the byte order of 16-bit numbers, and what each instruction does to the registers, the flags, the memory and the call stack.

Recover the following:

1. Fill in `starter/isa.json`, a file that describes the instruction set. For every instruction give the opcode byte (as a string such as `"0x2b"`) and the total length in bytes. Give `register_a`, the nibble of the byte after the opcode that holds the first register operand (`"high"` or `"low"`), and `immediate`, the byte order of 16-bit numbers (`"little"` or `"big"`).
2. Three numbers for the flag: `N`, the number of valid opcodes; `C`, the opcode byte of `call` in decimal; and `I`, the number of instructions in `bin/sample.tvm`.

```text
ARL{tinyvm_<N>_<C>_<I>}
```

For example, the values 12, 100 and 30 would give `ARL{tinyvm_12_100_30}`.

Write a disassembler for the machine to find `I` and to check your notes; the method is the one of module 21, and the sample is the program on which to test it. The write-up in `solutions/20/` contains a model disassembler to compare with afterwards.

## Part 2: Assemble

Part 2 has two pieces. Both depend on what you found in Part 1.

### The handlers

A replacement interpreter calls one function for each instruction. The other twelve handlers are written; you write these eight, in `starter/solution.asm`:

```c-fragment
void h_mul(struct vm *m, const unsigned char *ins);
void h_shl(struct vm *m, const unsigned char *ins);
void h_shr(struct vm *m, const unsigned char *ins);
void h_cmp(struct vm *m, const unsigned char *ins);
void h_call(struct vm *m, const unsigned char *ins);
void h_ret(struct vm *m, const unsigned char *ins);
void h_ldb(struct vm *m, const unsigned char *ins);
void h_stb(struct vm *m, const unsigned char *ins);
```

`m` points to the state of the machine, described in `starter/vm.h`, which has the layout of the structure in the binary: eight 32-bit registers, the program counter, the zero flag, the carry flag, a fault code, a halted indicator, the depth of the call stack, the call stack of sixteen 16-bit return addresses, and 256 bytes of memory. `ins` points to the bytes of the instruction, starting with the opcode byte, and has exactly the length of that instruction. When a handler is called, the program counter has already been moved past the instruction.

Each handler must leave the state exactly as the instruction in `tinyvm` does, including the flags, which instructions leave a flag alone, the effect of register numbers above 7, and what happens at the edges. Two cases are given: `call` with a full call stack sets the fault code to 3 and the halted indicator to 1 and changes nothing else, and `ret` with an empty call stack sets the fault code to 2 and the halted indicator to 1 and changes nothing else. All other behavior is for you to find.

The tests build many states, from ordinary to extreme, run one handler on each, and compare the whole state afterwards with that of the original.

### The program

Write a bytecode program in `starter/program.tvs` that stores the first twelve Fibonacci numbers (1, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144) in the machine's memory as bytes, one at each address from 0 to 11, and then prints them with `out` from the last to the first, one per line, so that the output is

```text
144
89
55
34
21
13
8
5
3
2
1
1
```

The program must use `call` and `ret` (the printing is a subroutine), `stb` (to store the numbers) and `ldb` (to read them back), and it must stop with `halt` within 400 steps. Assemble it with the assembler of the exercise, which is driven by your `isa.json`, and run it on the original machine:

```bash
python3 starter/tvasm.py --isa starter/isa.json starter/program.tvs starter/program.tvm
bin/tinyvm starter/program.tvm
```

The assembler's source format is described at the top of `starter/tvasm.py`. If your notes are wrong, the assembled program will not run correctly on the original machine.

## Rules

- Do not read a byte outside the state or the instruction in a handler, and do not write outside the state. The tests place both just before an unmapped page, and an access beyond the end ends the test run with a signal.
- Preserve the callee-saved registers `rbx`, `rbp` and `r12` to `r15`.
- Keep the stack aligned at every `call` if you make one.
- The files `isa.json`, `solution.asm` and `program.tvs` in `starter/` are the ones that are checked. Do not change `tvasm.py`.

## Check your work

```bash
make check EX=20 ARGS='--flag ARL{your_answer_here}'
```

The checker reports Part 1 and Part 2 separately. Part 1 passes when the flag is correct and `starter/isa.json` is correct; the output names each instruction whose entry is wrong. Part 2 passes when all handler tests pass and the program prints the right output within the limits. For a failing handler test the checker lists the fields of the state that differ, with their values before the instruction, the expected value and your value. Without `--flag`, it skips Part 1.

## Hints

Open [HINTS.md](HINTS.md) one level at a time.
