[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# 18. Auditing binaries for bugs

Time: about 2.5 hours. Prerequisites: [17. Deciphering file formats](../17-deciphering-file-formats/README.md)

## Why this matters

Most serious security flaws in native code are memory-safety bugs: a program copies, indexes or frees memory in a way its author did not intend. A defender who can read compiled code can find such a bug in a component that has no source, write a report that the owner can act on, and propose a fix that can be tested. This module teaches that work, from the defender's side. It ends at diagnosis and repair. It does not teach how to turn a bug into an attack, and the course never asks you to. Audit only code you own or are authorized to examine, and read [the ethics statement](../../ETHICS.md) before you start.

## Concepts

### The method: from sources to sinks

A memory-safety bug needs two things: a value that the program does not control (read from a file, a network packet, an argument, a field of a record) and a place where that value decides how much memory is touched. The first is a **source**, the second is a **sink**. The audit follows each source to each sink and looks at the checks in between.

| Sink | Question to ask |
|---|---|
| A copy (`memcpy`, `strcpy`, a loop that stores bytes) | What bounds the number of bytes? Is the bound compared with the size of the destination, and with the right size? |
| An index into an array | Is it compared with the length first, and is the comparison signed or unsigned? |
| An allocation (`malloc`, `calloc`, `realloc`) | How is the size computed? Can the computation wrap around? Does later code use the same numbers? |
| A call to `free` | Is the pointer used afterwards on any path? Is it freed twice? |
| A call to a library function with a format or a string | Does it take a size? Is the size correct? |

In a compiled program, find the sinks first: search the disassembly for calls to the copy, allocation and free functions, and for scaled-index stores. Then walk backwards from the size operand (`rdx` in a call to `memcpy`, `rdi` in a call to `malloc`) to the place where the value came from and read every comparison on the way.

### The practice code

The file `examples/18_bugs.c` has six small functions, each with one bug of a different class. `examples/18_fixed.c` has the same functions repaired. The listings in this lesson come from a build of `18_bugs.c` with `-O1`, `-fno-builtin`, `-mstringop-strategy=libcall` and `-D_FORTIFY_SOURCE=0`, which keeps the library calls visible the way they are in many real binaries. For confirmation, `make` also builds the demo programs with AddressSanitizer, a compiler feature that stops a program at the first bad memory access and prints a report. You can use it when you have source code; for a binary that you cannot rebuild, use the debugger.

```bash
make
make run
```

### Bug 1: a length that nobody checks

```text
0000000000000000 <set_name>:
   0:  push   rbp
   1:  push   rbx
   2:  sub    rsp,0x8
   6:  mov    rbx,rdi                      ; rbx = dst
   9:  movzx  ebp,BYTE PTR [rsi]           ; length = first byte of the packet, 0 to 255
   c:  add    rsi,0x1                      ; source = packet + 1
  10:  mov    rdx,rbp                      ; size for memcpy
  13:  call   memcpy
  18:  mov    BYTE PTR [rbx+rbp*1],0x0    ; terminator written at dst[length]
  1c:  add    rsp,0x8
  20:  pop    rbx
  21:  pop    rbp
  22:  ret
```

The size passed to `memcpy` is a byte read from the input and zero-extended, and there is no comparison between the load and the call. The function does not take the size of the destination at all, so no correct check is possible inside it: the function itself is the flaw, because its interface lacks the information it needs. The terminator store at `dst[length]` is a second write with the same problem.

The caller in the demo has a 16-byte buffer and a length of 40. In the debugger you can see the situation before the copy happens:

```text
Breakpoint 1, set_name (dst=0x7ffdef7e5010 "", pkt=0x14a982a0 "(", 'A' <repeats 40 times>) at 18_bugs.c:13
13        unsigned len = pkt[0];
0x14a982a0:    40
#1  0x0000000000401244 in main (argc=2, argv=0x7ffdef7e5168) at 18_demo_bug.c:20
20        set_name(dst, pkt);
$1 = 16
$2 = 40 '('
```

The commands were `break set_name`, `run name`, `x/ub $rsi`, `up`, `print sizeof dst` and `print pkt[0]`, and then `kill`. The program was stopped before the faulty copy, and the numbers are enough for the report: the destination holds 16 bytes and the length is 40. With the sanitizer build, the report names the same line:

```text
ERROR: AddressSanitizer: stack-buffer-overflow on address ADDR at pc ADDR bp ADDR sp ADDR
WRITE of size 40 at ADDR thread T0
    #1 ADDR in set_name 18_bugs.c:14
    #2 ADDR in main 18_demo_bug.c:20
```

Fix: the function must receive the capacity of the destination and the length of the packet, and refuse input that does not fit. `18_fixed.c` has `set_name(dst, cap, pkt, pkt_len)`, which rejects a length larger than the remaining packet or not smaller than the capacity (the last byte is for the terminator).

### Bug 2: off by one

```text
0000000000000023 <copy_label>:
  ...
  30:  mov    rdi,rdx                      ; rdi = src
  33:  call   strlen                       ; rax = n
  38:  cmp    r12,rax                      ; cap against n
  3b:  jb     56 <copy_label+0x33>         ; cap < n: return -1
  3d:  lea    rdx,[rax+0x1]                ; size = n + 1
  41:  mov    rsi,rbx
  44:  mov    rdi,rbp
  47:  call   memcpy                       ; copies n + 1 bytes
```

The code rejects the string only when it is longer than the capacity. A string of exactly `cap` characters passes, and the copy then writes `cap + 1` bytes: the characters and the terminator. The giveaway is the difference between the compared quantity (`n`) and the copied quantity (`n + 1`, the `lea rdx,[rax+0x1]`). Whenever a copy size has an added constant, the check before it must include the same constant. The sanitizer reports a write of size 9 into an 8-byte buffer. In the repaired version the branch after `cmp rax,r12` is `jae`: reject when `n >= cap`.

### Bug 3: integer overflow before an allocation

```text
000000000000005d <make_table>:
  65:  mov    r13d,edi                     ; count
  68:  mov    r12d,esi                     ; elem
  6b:  imul   edi,esi                      ; total = count * elem, in 32 bits
  6e:  call   malloc                       ; malloc(total), rdi zero-extended
  ...
  8b:  mov    rdx,r12                      ; the loop uses elem ...
  93:  call   memset
  98:  add    ebx,0x1
  9e:  cmp    r13d,ebx                     ; ... for count iterations
  a1:  jne    8b
```

The multiplication is a 32-bit `imul` of two values that come from the input, and the result goes straight to `malloc`. The loop that fills the table uses `count` and `elem` separately and never looks at the allocated size. If the product wraps around (a count of 0x10000001 with an element size of 16 gives 16), the allocation is tiny and the loop writes far beyond it. The signature to look for is an arithmetic instruction on input-derived values whose result is an allocation size, with the same inputs used again later for a loop or an index. The sanitizer report is a write of size 16 past the end of a 16-byte heap region. The repair computes the size in 64 bits, which cannot wrap for 32-bit inputs, and applies a policy limit before allocating. `__builtin_mul_overflow` does the same job when the operands are already 64-bit.

### Bug 4: signed and unsigned

```text
00000000000000af <copy_block>:
  af:  cmp    edx,0x40                     ; len against 64
  b2:  jg     c3 <copy_block+0x14>         ; signed: greater than 64 is refused
  b4:  push   rbx
  b5:  mov    ebx,edx
  b7:  movsxd rdx,edx                      ; sign-extend len to 64 bits
  ba:  call   memcpy                       ; size = rdx
```

Two instructions together make the signature: a signed comparison (`jg`) that guards only the upper end, and a sign extension (`movsxd`) when the value is widened to the 64-bit size. A negative `len` passes the comparison, is sign-extended to a huge unsigned number and reaches `memcpy` as its size. The sanitizer calls this `negative-size-param`. In the repaired version the compiler emits a single unsigned comparison (`cmp edx,0x40` then `ja`), because the source rejects `len < 0` as well and a negative value reads as a large unsigned one. Remember the rule: a size is unsigned, and a check for an upper bound on a signed value is incomplete without a check for the lower bound.

### Bug 5: unsafe library calls

```text
00000000000000c9 <greet>:
  c9:  push   rbx
  ca:  sub    rsp,0x20                     ; a 32-byte frame; the buffer is 24 bytes of it
  ce:  mov    rdx,rdi                      ; the user string
  d1:  mov    rbx,rsi
  d4:  mov    esi,0x0                      ; the format "hello, %s" (a relocation)
  d9:  mov    rdi,rsp                      ; destination: a buffer on the stack
  dc:  mov    eax,0x0
  e1:  call   sprintf                      ; no size anywhere
  e6:  mov    rsi,rsp
  e9:  mov    rdi,rbx
  ec:  call   strcpy                       ; and the copy to the caller's buffer is unbounded too
```

Some library functions cannot be used safely because they do not take the size of the destination: `strcpy`, `strcat`, `sprintf`, `vsprintf`, `gets`, and `scanf` with a `%s` conversion that has no width. Others take a size and are still misused: `strncpy` does not add a terminator when the source is as long as the limit, and `memcpy` takes a size that must be checked by the caller. A call is unsafe when the destination is a fixed-size buffer (a stack frame area or a global, as `rdi = rsp` here), the source depends on the input, and no comparison with the destination size lies between them. Many distributions compile with fortification, which replaces such calls by `__sprintf_chk` and similar functions that receive the destination size as an extra argument (the `-O1` build with default flags shows `__sprintf_chk` with the size 0x18). That turns the silent overwrite into an abort, so the effect is milder, but the bug is still there and still deserves a report. Seeing a `_chk` function in a binary tells you the code was compiled with the protection; seeing the plain function tells you it was not. The repair uses `snprintf` with the capacity and checks the result.

### Bug 6: use after free

```text
00000000000000f7 <finish>:
  f7:  push   rbx
  f8:  mov    rbx,rdi                      ; rbx = the item
  fb:  test   esi,esi
  fd:  je     103 <finish+0xc>             ; ok == 0: go to the free
  ff:  mov    eax,DWORD PTR [rbx]          ; read it->id
 101:  pop    rbx
 102:  ret
 103:  call   free                          ; free(rbx)
 108:  jmp    ff <finish+0x8>              ; and then continue at the read through rbx
```

The signature of a use after free in disassembly is a path that goes through `call free` on a pointer and then reaches an instruction that dereferences the same pointer again, here by the jump back to `ff`. Follow the register or stack slot that held the argument of `free`: if it is read after the call, on any path, the code uses freed memory. A second `call free` on the same value is a double free. The sanitizer reports `heap-use-after-free` with a read of size 4. In the repaired version `18_fixed.c` reads the field before the free. Another good habit is to store a null pointer after freeing, so that a later use fails at once.

### Confirming a finding without causing damage

A finding is more useful with evidence. The safe forms of evidence are:

- Run your own build of the code under a sanitizer, with an input that triggers the path. This is the best evidence when you have the source.
- In a debugger, stop at the sink before it executes and read the numbers: the size and the capacity. This needs no crash and no oversized write.
- For the binary alone, show the path in the disassembly: the source, the missing or wrong comparison, and the sink.

Do not build inputs whose purpose is to take control of the program. For the audit it does not matter what an attacker could do next; it matters that the program touches memory it should not, and that the fix removes the cause.

### Writing the report

A report is read by the person who must fix the bug. It has to be exact and short. Use the same fields every time.

```text
Title:      Unchecked packet length in set_name
Class:      CWE-787 out-of-bounds write (CWE-120, copy without checking the size)
Location:   set_name, call to memcpy at offset 0x13 (source line 14)
Severity:   High. The length and the bytes both come from the input, and the
            destination is a fixed buffer.
Cause:      set_name copies the number of bytes given by the first byte of the
            packet (0 to 255) into a destination whose size it does not know.
            No comparison separates the load of the length from the call.
Evidence:   Debugger, stopped before the copy: destination 16 bytes, length 40.
            Sanitizer build: stack-buffer-overflow, WRITE of size 40 at
            18_bugs.c:14.
Impact:     Memory outside the destination is overwritten with bytes from the
            packet. This corrupts adjacent data and can crash the program. What
            is next to the buffer depends on the caller; that was not assessed.
Fix:        Pass the capacity and the packet length. Reject a length larger than
            the rest of the packet or not smaller than the capacity. See
            set_name in 18_fixed.c.
Test:       Packets with lengths 0, 15, 16, 17 and 255 and a one-byte packet.
            The first two succeed, the rest are rejected without a write. Run
            the tests under the sanitizer.
```

Choose the class from the public weakness list (CWE) that matches the cause, not the symptom: 787 for an out-of-bounds write, 125 for a read, 193 for an off-by-one error, 190 for integer overflow or wraparound, 195 for a signed-to-unsigned conversion, 416 for use after free, 415 for a double free and 676 for the use of a dangerous function. Give severity by reasoning: who controls the input, what is written and where. State what you did not assess. If you found the bug in software that belongs to someone else, report it privately to its owner and give them time to fix it before saying anything in public.

### Checking the fix

A fix is verified like the bug. Run the same input again and see that the report is gone. Run the boundary values: the largest accepted value, the smallest rejected value, zero, negative, and the value that would wrap. The program `demo_fix` does this for the six repaired functions:

```text
name:   -1
name:   0 (alice)
label:  -1
label:  0
table:  refused
block:  -1
greet:  -1
greet:  0 (hello, bob)
finish: 7
```

Each dangerous input is refused without any memory access, and the ordinary input still works. A fix that only moves the problem (for example, a bigger buffer) is not a fix.

## Try it

Run the lab commands from `modules/18-auditing-binaries/examples`.

1. Build with `make` and run `make run`. For each of the six cases, write down the class of the bug and the line of the report before you read the lesson's explanation.
2. Disassemble `build/18_bugs.o` with `objdump -dr -M intel`. Find the call to `memcpy` in `copy_block` and trace the origin of `rdx`. Which instruction produced it from `edx`?
3. Compare `build/18_bugs.o` and `build/18_fixed.o` with the same command. For `copy_label` and `copy_block`, list the instructions that changed.
4. Run `gdb -nx -q build/demo_plain` and use the commands from the lesson to stop `set_name` before its copy and read the numbers. Do not continue past the breakpoint with the oversized input.
5. Write a report in the format above for the bug in `finish`. Include the class, the location, the cause, the evidence and the fix.
6. Search a program of your own (a small one that you wrote) for calls to `strcpy`, `sprintf` and `memcpy` and decide for each whether the destination size is checked.

## Common pitfalls

- **Auditing the symptom.** A crash at one line often comes from an earlier mistake, such as an allocation size computed wrongly. Follow the value back to its source.
- **Comparing the wrong quantity.** The check must use the quantity that is copied, including terminators and header bytes.
- **Trusting one type.** A size read as `int` and later passed as `size_t` changes meaning when it is negative. Find where the type changes.
- **Treating the fortified call as safe.** A `_chk` function turns corruption into an abort, but a program that aborts on valid-looking input still has a bug.
- **Fixing one site.** The same pattern usually appears elsewhere. Search for it after you fix the first case.
- **Proving too much.** The report needs the cause and the evidence, not a demonstration of what an attacker could do.

## Summary

- Follow input values from sources to sinks and read every comparison between them.
- Six patterns cover most findings: an unchecked length, an off-by-one comparison, an arithmetic wrap before an allocation, a signed value used as a size, an unsafe library call, and a use after free.
- Each pattern has a signature in disassembly: a size operand with no comparison, a compared quantity that differs from the copied one, an `imul` on input values before `malloc`, `jg` with `movsxd`, a call without a size, and a dereference after `free`.
- Confirm findings with a sanitizer build or a debugger stopped before the sink, never with a harmful input.
- A report states the class, the location, the cause, the evidence, the impact, the fix and the test; a fix is verified with the boundary values.

## Next steps

- Take the [quiz](quiz.md).
- Continue with module 19, analyzing suspicious programs.
- Exercises that use this module: [Exercise 18](../../exercises/18-bug-hunt-report/README.md).
