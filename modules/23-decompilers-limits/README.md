[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# 23. Decompilers and their limits

Time: about 1.5 hours. Prerequisites: [22. Managed code: .NET](../22-managed-code-dotnet/README.md)

## Why this matters

A decompiler is the most useful tool in reverse engineering and the one that is most often believed too much. It turns machine code into something that looks like C, with loops, conditions and named variables, and for most functions the result is close enough to the truth to read fast. For some it is wrong in ways that do not show: a number of the wrong width, a comparison of the wrong signedness, a condition whose operators group differently from what the listing suggests. A decision made on such a line is a wrong decision, and the output looks as confident when it is wrong as when it is right. This module explains how a decompiler works, so that you can predict where it fails, shows the typical failures on real compiled code, and teaches the habit that makes decompilers safe to use: treat the output as a hypothesis about the code, and check it against the disassembly and against the running program. The examples use a small decompiler written for the lesson, so that every stage can be shown, and small functions whose compiled form is easy to misread.

## Concepts

### What a decompiler does

A decompiler does not undo the compiler. The compiler discarded names, types, comments, the structure of loops and the choice of source constructs; the decompiler builds a plausible program that would compile to the same behavior. Every decompiler goes through the same stages:

| Stage | What it does | What can go wrong |
|---|---|---|
| Disassembly and control-flow graph | Finds the instructions of a function and groups them into basic blocks joined by jumps | A wrong function boundary, a jump table that is not found, code reached only through an indirect jump |
| Lifting | Turns each instruction into an operation on variables; registers become variables and flags become conditions | An instruction with an unusual effect is modeled approximately |
| Data-flow analysis | Follows the values: removes assignments that nothing reads, merges temporary values into expressions, recovers local variables from stack slots | A value that is used in a way the model does not see is dropped or merged wrongly |
| Type recovery | Chooses a width, a signedness and a pointer or number type for each variable from how it is used | The evidence is thin or contradictory and a guess is made |
| Structuring | Rebuilds `if`, `while`, `for` and `switch` from the graph; what cannot be rebuilt stays as `goto` | The compiler reshaped the loops and no structure matches |
| Output | Names the variables, prints expressions with the fewest parentheses and casts that keep the meaning | Casts and parentheses are dropped or misplaced in the printed text |

The lesson follows these stages with `examples/23_lift.py`, a decompiler for a handful of x86-64 instructions. It is deliberately small; it understands only what the examples need and stops with a message at anything else. A real decompiler (Ghidra, which you used in module 11) does the same work for the whole instruction set, with far more analysis. The point is to see each stage.

### The functions used here

`examples/23_cases.c` has seven short functions, compiled with `-O2` into a shared library. Each is easy to misread:

| Function | What it does | The trap |
|---|---|---|
| `div10` | Unsigned division by 10 | The compiler uses a multiplication by a constant and a shift |
| `widen` | Multiplies two 32-bit numbers into a 64-bit result | The width of the multiplication |
| `sat_add` | Adds two signed numbers and saturates on overflow | The overflow is a flag, not a comparison |
| `is_pow2` | True for 1, 2, 4, 8, ... | A condition made from a test, a subtraction and a flag |
| `sum_to` | Sum of the numbers from 0 to n-1, in a loop | The optimizer changed the shape of the loop |
| `sign_byte` | Reads one byte as a signed number | Sign extension against zero extension |
| `count_lt` | Counts the elements smaller than a value | A comparison and a count without a branch |

Build the examples and run the verification (explained later) and one function through the tool:

```bash
make
make run
```

### Stage 1: the graph

The first stage splits the instructions into basic blocks and records where control goes. A block starts at the first instruction of the function, at every jump target and after every jump; it ends at a jump or a return. A back edge, an edge from a block to one that dominates it (one that is on every path from the entry to the block), marks a loop. For `count_lt`:

```bash
python3 23_lift.py build/libcases.so count_lt --stage cfg
```

```text
== stage 1: control-flow graph of count_lt ==
block L_11b0:
    11b0:  test esi, esi
    11b2:  jle 0x11d8
    -> taken L_11d8, fall-through L_11b4
block L_11b4:
    11b4:  movsxd rsi, esi
    11b7:  xor eax, eax
    11b9:  lea rsi, [rdi + rsi*4]
    -> L_11c0
block L_11c0:
    11c0:  xor ecx, ecx
    11c2:  cmp edx, dword ptr [rdi]
    11c4:  setg cl
    11c7:  add rdi, 4
    11cb:  add eax, ecx
    11cd:  cmp rsi, rdi
    11d0:  jne 0x11c0
    -> taken L_11c0, fall-through L_11d2
block L_11d2:
    11d2:  ret
    -> return
block L_11d8:
    11d8:  xor eax, eax
    11da:  ret
    -> return
back edge L_11c0 -> L_11c0: L_11c0 is a loop header
```

Five blocks and one loop: the entry block tests the count (`jle` skips everything when it is zero or negative), the second prepares the loop (the end pointer and the sum), the third is the loop and jumps back to itself, and two blocks return. All of this comes from the jumps alone, and it is where a decompiler can already go wrong: a jump table is a jump whose targets are not in the instruction, and if the decompiler does not find the table it sees a function that ends in the middle.

### Stage 2: lifting

Lifting gives each instruction a meaning as an assignment. Two things are needed that the machine does not have: a variable for every register at every point, and conditions in place of flags. A compare and a conditional jump are separate instructions joined by the flags register; the decompiler fuses them. In `count_lt`:

```text
cmp    edx, dword ptr [rdi]
setg   cl
```

becomes `rcx = (i32)rdx > (i32)(*(u32*)(rdi))`, because `setg` tests the signed greater-than condition of the preceding compare. The condition, not the flag, is what a reader needs. The tool does this within a block by tracking an expression for every register and the last flag-setting operation, and it substitutes expressions into each other. The result for the whole function:

```bash
python3 23_lift.py build/libcases.so count_lt --stage lift
```

```text
== stage 2: lifted statements (every register that a block changes) ==
L_11b0:
    if ((i32)rsi <= 0) goto L_11d8; else goto L_11b4;
L_11b4:
    rsi = rdi + 4 * (i64)(i32)(u32)rsi;
    rax = 0;
    goto L_11c0;
L_11c0:
    flag = rsi != (rdi + 4);
    rcx = (i32)rdx > (i32)(*(u32*)(rdi));
    rax = (u32)((u32)rax + ((i32)rdx > (i32)(*(u32*)(rdi))));
    rdi = rdi + 4;
    if (flag) goto L_11c0; else goto L_11d2;
L_11d2:
    return rax;
L_11d8:
    rax = 0;
    return rax;
(7 assignments)
```

Look at the loop block. Four instructions of the original (`xor`, `cmp`, `setg`, `add`) have become one assignment to `rax`, and the comparison with the end pointer is saved in a variable `flag`, because the block also changes `rdi` and the condition must use the old value. A decompiler faces this problem for every block, and a mistake in the order of evaluation is one of the ways a listing becomes wrong.

### Stage 3: data-flow analysis

The lifted code assigns to every register that a block changes. Most of those values are never read again. Liveness analysis finds out which are: a register is live at a point if some path from there reads it before writing it. An assignment to a register that is not live afterwards is removed. The same pass tells the decompiler which registers are arguments (read before any write) and which holds the result (read at the return).

```bash
python3 23_lift.py build/libcases.so count_lt --stage prune
```

```text
== stage 3: after removing assignments that nothing reads ==
L_11b0:
    if ((i32)rsi <= 0) goto L_11d8; else goto L_11b4;
L_11b4:
    rsi = rdi + 4 * (i64)(i32)(u32)rsi;
    rax = 0;
    goto L_11c0;
L_11c0:
    flag = rsi != (rdi + 4);
    rax = (u32)((u32)rax + ((i32)rdx > (i32)(*(u32*)(rdi))));
    rdi = rdi + 4;
    if (flag) goto L_11c0; else goto L_11d2;
L_11d2:
    return rax;
L_11d8:
    rax = 0;
    return rax;
(6 assignments)
```

The assignment to `rcx` is gone: it was a temporary that the loop wrote and then folded into `rax`. Real decompilers do much more here, including recovering the local variables that the compiler kept in stack slots, propagating constants, and deciding which values to name and which to leave inside an expression. Each of these decisions is a place where the listing can differ from the code in a way that is not visible in the printed text.

### Stage 4: types and structure

Types are the part that is guessed. The machine knows widths (the register or memory size of each access) and almost nothing else. Whether a 32-bit value is signed or unsigned, a number or a pointer, comes from how it is used: a `movsx` or a signed compare (`jl`, `setg`) points to signed, a `movzx` or an unsigned compare (`jb`, `seta`) points to unsigned, a value added to a scaled index is an address. The tool does this implicitly by choosing the cast in each expression from the instruction that produced it: `(i32)` for a signed compare, `(u32)` for an unsigned one. A real decompiler has to make one choice per variable, from all the uses, and when they conflict it picks one.

Structuring turns the graph into `if` and loops. The tool recognizes a conditional that splits and rejoins, a conditional that skips code, and a loop whose test is at the end; anything else it leaves as `goto` with a label.

```bash
python3 23_lift.py build/libcases.so count_lt --stage struct
```

```text
== stage 4: structured ==
result count_lt(...)
{
    if ((i32)rsi <= 0) {
        rax = 0;
        return rax;
    } else {
        rsi = rdi + 4 * (i64)(i32)(u32)rsi;
        rax = 0;
        do {
            flag = rsi != (rdi + 4);
            rax = (u32)((u32)rax + ((i32)rdx > (i32)(*(u32*)(rdi))));
            rdi = rdi + 4;
        } while (flag);
        return rax;
    }
}
(goto statements left: 0)
```

This is a function that a reader understands at once: if the count is not positive return 0, otherwise walk the array from the start to the end pointer and add one for every element smaller than the value in `rdx`, and return the sum. The variable names are those of the registers. A real decompiler names the arguments from the calling convention and the locals by their type.

### Where the output is wrong or misleading

The following cases come from the example library. Each has the same shape: what the machine code does, how a listing can mislead, and what to check. The output of `23_lift.py` for the function is shown, since it makes the machine behavior explicit.

#### Division by a constant

```bash
objdump -d -M intel --no-show-raw-insn build/libcases.so | sed -n '/<div10>:/,/ret/p'
```

```text
0000000000001100 <div10>:
    1100:    mov    eax,edi
    1102:    mov    edx,0xcccccccd
    1107:    imul   rax,rdx
    110b:    shr    rax,0x23
    110f:    ret
```

There is no division. `0xcccccccd` is the 32-bit approximation of 2 to the power 35 divided by 10, and the product shifted right by 35 gives the quotient for every 32-bit input. A decompiler may print the multiplication and the shift, which is faithful and hard to read, or may recognize the pattern and print `x / 10`. Both are right. The mistake comes from the reader: the width and the signedness of the division are in the instructions around the multiplication (`mov eax,edi` zero-extends; a signed division by 10 has a different sequence with a sign correction), and a reading that says `(int)x / 10` is wrong for every `x` of 2 to the power 31 and above.

#### Width of a multiplication

```text
== stage 4: structured ==
result widen(...)
{
    rax = (u64)(u32)rdi * (u64)(u32)rsi;
    return rax;
}
(goto statements left: 0)
```

The two arguments are zero-extended to 64 bits and multiplied as 64-bit numbers (`imul rax, rsi`). In C that is `(unsigned long)a * b`. Written as `(unsigned long)(a * b)`, which looks the same at a glance, the multiplication would be done in 32 bits and lose the upper half. The location of the cast is the whole difference, and a printed listing can place it wrongly or omit it.

#### Overflow is a flag

```bash
python3 23_lift.py build/libcases.so sat_add --stage struct
```

```text
== stage 4: structured ==
result sat_add(...)
{
    flag = overflow_add_i32((u32)rsi, (u32)rdi);
    rsi = (u32)((u32)rsi + (u32)rdi);
    if (flag) {
        rax = (u32)(((u32)rdi >> 0x1f) + 0x7fffffff);
        return rax;
    } else {
        rax = (u32)rsi;
        return rax;
    }
}
(goto statements left: 0)
```

`jo` branches on the overflow flag, which the preceding `add` set. A decompiler can show it as a call to an overflow predicate, as the tool does, as a written-out condition on the signs of the operands and the result, or, when it does not model the flag, as a comparison that is not equivalent. The saturation value comes from the sign of the first argument: `(x >> 31) + 0x7fffffff` is `0x7fffffff` for a non-negative `x` and `0x80000000` for a negative one. A reading that handles only the overflow of two positive numbers misses half of the function.

#### A condition made of several steps

```bash
python3 23_lift.py build/libcases.so is_pow2 --stage struct
```

```text
== stage 4: structured ==
result is_pow2(...)
{
    rax = 0;
    if ((u32)rdi != 0) {
        rax = ((u32)(rdi - 1) & (u32)rdi) == 0;
    }
    return rax;
}
(goto statements left: 0)
```

The result is a Boolean: `lea eax,[rdi-1]` computes `x - 1`, `test eax,edi` computes `(x - 1) & x` and sets the zero flag, and `sete al` turns the flag into 0 or 1. In C this reads `x != 0 && (x & (x - 1)) == 0`. The common mistake in a hand-written reading is to drop the parentheses: in C the comparison `==` binds tighter than `&`, so `x & (x - 1) == 0` means `x & ((x - 1) == 0)`, which is a different function. A decompiler prints the parentheses, but when you rewrite or simplify the output, it is the first thing that goes.

#### A loop the optimizer reshaped

```bash
python3 23_lift.py build/libcases.so sum_to --stage struct
```

```text
== stage 4: structured ==
result sum_to(...)
{
    if ((u32)rdi == 0) {
        rax = 0;
        return rax;
    } else {
        rax = 0;
        rdx = 0;
        if (((u8)rdi & 1) == 0) {
            L_1180:
            do {
                flag = (u32)rdi != (u32)((u32)rax + 2);
                rdx = (u32)(rdx + 2 * rax + 1);
                rax = (u32)((u32)rax + 2);
            } while (flag);
        } else {
            rax = 1;
            if ((u32)rdi != 1) {
                goto L_1180;
            }
        }
        rax = (u32)rdx;
        return rax;
    }
}
(goto statements left: 1)
```

The source is a loop that adds the numbers from 0 to n-1 one at a time. The compiled loop adds two numbers per iteration, and an extra step before it handles an odd count; the entry jumps into the middle of the structure. The tool cannot give this one clean structure: one `goto` into the loop remains, with a label inside the `if`. Real decompilers meet the same shape and may print the loop twice, print a `goto`, or print a loop that is correct but does not look like the source. Reading it from the graph gives the answer: the loop adds `2 * i + 1` to the sum and steps `i` by 2, which is the sum of the consecutive pair `i` and `i + 1`; for an odd count the step before the loop starts the pairs at 1, because the first term is 0.

#### Signedness and extension

```bash
python3 23_lift.py build/libcases.so sign_byte --stage struct
```

```text
== stage 4: structured ==
result sign_byte(...)
{
    rax = (i32)(i8)*(u8*)(rdi);
    return rax;
}
(goto statements left: 0)
```

`movsx` is sign extension; `movzx` is zero extension. In C the first is a cast to `signed char` and the second a read of an `unsigned char`. For the byte `0x80` they give -128 and 128. In `count_lt` the same distinction hides in the instruction `setg`, which tests a signed comparison, where `setb` would test an unsigned one. Reading the right mnemonic is the check.

### Verifying a listing

Treat every decompiled function as a claim: "this C does what the binary does". Three checks, from cheapest to strongest.

**Read the listing against the disassembly.** For each line that matters, find the instructions that produced it, and check these points:

| Point | What to look at in the disassembly |
|---|---|
| Width | The register name (`eax` against `rax`), the size keyword in memory operands (`byte`, `word`, `dword`, `qword`), and where a value is widened |
| Signedness | `movsx`, `movsxd`, `sar`, `idiv`, `jl/jg/jle/jge` and `setl/setg` against `movzx`, `shr`, `div`, `jb/ja/jbe/jae` and `setb/seta` |
| Constants | The immediate values, in hexadecimal; a magic constant is a division or a modulus |
| Conditions | Whether the branch is taken when the condition is true or false; a decompiler negates conditions to fit the structure |
| Loop bounds | The first and the last value of the counter, and the comparison that ends the loop (`<` against `<=`) |
| Memory accesses | The scale of the index (the array element size), the offset (the structure field), and the size of each access |
| Calls | The arguments, from the calling convention, and which registers survive the call |
| Return value | The width and the signedness of what is left in `rax` |
| Order | When two side effects change a value that the other reads, the order in which they happen |

**Test the listing.** Write the C that the listing says, compile it as a function with the same signature, and run it and the original on the same inputs. Differences are mistakes in one or the other. The script `examples/23_verify.py` does this for the seven functions: `23_candidates.c` has, for each function, a reading that matches (`_ok`) and a reading with a typical mistake (`_bad`).

```bash
python3 23_verify.py build/libcases.so build/libcand.so
```

```text
div10_ok       matches
div10_bad      DIFFERS on 98 of 215 inputs
    first input ('0x80000000',): binary returns 214748364, reading returns 4080218932
widen_ok       matches
widen_bad      DIFFERS on 977 of 1225 inputs
    first input (2, '0x80000000'): binary returns 4294967296, reading returns 0
sat_add_ok     matches
sat_add_bad    DIFFERS on 168 of 841 inputs
    first input (-2147483648, -2147483648): binary returns -2147483648, reading returns 0
is_pow2_ok     matches
is_pow2_bad    DIFFERS on 3 of 215 inputs
    first input (2,): binary returns 1, reading returns 0
sum_to_ok      matches
sum_to_bad     DIFFERS on 14 of 50 inputs
    first input ('0x10001',): binary returns 2147516416, reading returns 32768
sign_byte_ok   matches
sign_byte_bad  DIFFERS on 128 of 256 inputs
    first input (b'\x80',): binary returns -128, reading returns 128
count_lt_ok    matches
count_lt_bad   DIFFERS on 233 of 300 inputs
    first input ([-2, 2, 2147483646, 1], 0): binary returns 1, reading returns 0
```

The readings that match produce no differences on any input. Each wrong reading differs on some: `div10_bad` treats the number as signed and fails for every value from 2 to the power 31 on, `widen_bad` multiplies in 32 bits, `sat_add_bad` handles only one side of the overflow, `sum_to_bad` overflows before the division, `sign_byte_bad` zero-extends, and `count_lt_bad` compares as unsigned and is wrong whenever an element or the threshold is negative. The case that deserves attention is `is_pow2_bad`: it differs on only 3 of the 215 inputs, because the test set has few powers of two, and 212 inputs agree with the wrong reading. A test is only as good as its inputs. Include the edges (0, 1, the largest and smallest values, the values at the boundaries of the widths) and the special cases that the function's logic singles out (here, powers of two).

**Check the listing's claims about the running program.** For the lines that decide the answer, break at the function in the debugger (module 11), feed it known arguments, and compare the registers and memory with what the listing says. This is the check that works even when the code is hand-written, obfuscated or too complicated to test in isolation.

### When to trust the disassembly instead

Prefer the listing of instructions to the decompiled code when:

- The function is small and every instruction matters (a checksum, a comparison routine, a cipher round, a bit manipulation). The semantics are in the exact operations, widths and flags.
- The decompiler gives up, times out or prints a function of a few lines that cannot be right for the size of the code.
- The code was written in assembly, or uses instructions without a counterpart in C (carry chains with `adc` and `sbb`, `rep movs` and `rep stos`, vector instructions, `rdtsc`, `cpuid`, system calls).
- The control flow is unusual: jump tables that were not found, computed jumps, code that jumps into the middle of another function, a virtual machine of module 21.
- The decompiled code contains many `goto` statements or variables that change type or meaning between uses; the structure is a guess.
- A result is surprising: a function that should do something does nothing, or a constant that does not make sense. Look at the instructions.

Prefer the decompiler when you need the overall shape of a large function, the logic around a call, or a quick map of what a program does. A good practice is to use both together: read the decompiled function for the structure, and read the instructions behind each line that decides something.

### A working order

1. **Read the decompiled function once** for its shape: the arguments, the loops, the calls, the return value.
2. **Mark the lines that decide something**: comparisons, constants, sizes, indexes, the return value.
3. **Check each marked line** against the disassembly with the table of points above.
4. **Rename and retype** as you learn, in the decompiler, so that the next reading is closer to the truth; then check again, because a changed type changes the listing.
5. **Test the claim** when it matters: write the equivalent C, run both on edge inputs and random inputs.
6. **Write down what you could not verify**, and report it as such.

## Try it

Run the lab commands from `modules/23-decompilers-limits/examples`.

1. Build with `make` and run `make run`. For each of the seven functions, read the disassembly (`objdump -d -M intel --no-show-raw-insn build/libcases.so`) and write the C you think it is. Run `23_lift.py` on the function and compare.
2. Run `23_lift.py` with `--stage cfg`, `lift`, `prune` and `struct` on `sat_add`, and describe in a sentence what each stage changed.
3. Open `build/libcases.so` in Ghidra on the host and compare its decompiled output for the seven functions with the output of `23_lift.py` and with the source. For each difference, decide whether it is a matter of names and style or of meaning.
4. Add an eighth function to `23_cases.c`, for example an average of two unsigned numbers written as `(a + b) / 2` and as `a + (b - a) / 2`. Rebuild, read the disassembly of both, and say which one the compiler can turn into a shorter sequence and why they differ for large inputs.
5. Write a wrong reading of your own for one of the functions in `23_candidates.c`, add it to the cases in `23_verify.py`, and check that the test finds it. Then make the wrong reading differ on very few inputs, and see how many random inputs the test needs to find it.
6. Change the argument to `23_lift.py` to a function from a program of an earlier module (a function from a library you built before). Where does the tool stop? What instruction does it not know?

## Common pitfalls

- **Believing a listing because it reads well.** Readable output is not correct output. Check the lines that decide something.
- **Trusting the types.** A variable declared `int` may be unsigned in the code, and a pointer may be printed as a number. Read the instructions that use the value.
- **Missing a negated condition.** The decompiler may print the opposite of the branch, to fit its structure. Check which side is taken.
- **Simplifying the output and changing the meaning.** Removing parentheses or casts changes what C means, as the `is_pow2` reading shows.
- **Testing with random inputs only.** Many errors show up at boundaries and at special values. Include them.
- **Fixing the decompiler's guess with another guess.** When you retype a variable, check the result against the instructions again.
- **Ignoring what the decompiler could not structure.** A `goto` in the output is a sign that the compiler reshaped the code, and it deserves a look at the graph.

## Summary

- A decompiler builds a plausible program from machine code in stages: graph, lifting, data-flow analysis, type recovery, structuring and printing; each stage can introduce errors that the printed text hides.
- Width, signedness and the position of casts are the most frequent sources of wrong readings, followed by flags turned into conditions, reshaped loops and misplaced parentheses.
- Verify a listing by reading it against the disassembly for every line that decides something, by testing a C reading against the binary with edge and random inputs, and by checking claims in a debugger.
- A test is only as strong as its inputs; include boundaries and the special cases of the logic.
- Use the instructions rather than the decompiled code for small exact routines, hand-written assembly, unusual control flow, and wherever the decompiler fails or looks wrong.
- The decompiler gives the shape; the disassembly gives the truth; use both.

## Next steps

- Take the [quiz](quiz.md).
- Continue with exercise 20, the capstone, which combines compiled code (module 13), recovering data (module 15) and virtual machines (module 21).
