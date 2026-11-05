[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# 20. Reading obfuscated code

Time: about 2.5 hours. Prerequisites: [19. Analyzing suspicious programs](../19-analyzing-suspicious-programs/README.md)

## Why this matters

Some programs are written to be hard to read. Their strings are encoded so that a search finds nothing, their logic is cut into pieces and shuffled, their code is padded with instructions that do nothing, and some of them notice when they are being watched. Defenders meet this in malware, and everybody meets it in protected software. Obfuscation never changes what a program does; it only changes how hard it is to see. The processor must still execute the real behavior, which means the behavior can always be recovered. This module teaches the common techniques, how to recognize each one in a disassembly, and how to choose the cheapest way to see through it. All examples here are harmless programs written for the lesson. The module is about reading code to understand it; it does not cover removing the protections of commercial software.

## Concepts

### What obfuscation does and what to do about it

Programs are obfuscated to protect an idea, to slip past scanners that look for known strings and patterns, and to waste the time of the person who analyzes them. A technique that makes reading harder is not a barrier to understanding, because every obfuscated program still has to do its work. Your task is to find the cheapest route to the behavior. The three routes used in this module are:

| Route | Idea | Good for |
|---|---|---|
| Reading the code | Recognize the technique and undo it by hand or with a small script | Encoded strings, junk code, a flattened function that is small |
| Emulation | Run only a small routine in an emulator, with no access to the system | Decoding routines, hash and key derivation, any self-contained computation |
| Data flow | Start from the result you care about and follow only what contributes to it | Code padded with junk, large functions, anything where most instructions are noise |

Running the whole program is the last choice, not the first, and it is done only in the isolated lab of module 19.

### Encoded strings

Strings are the most useful evidence in triage, so they are the first thing to hide. The program keeps the text in an encoded form and decodes it when it needs it. The signs:

- `strings` finds nothing where the program obviously uses a path, a host or a message. Compare what you know the program does with what the string list shows.
- A table of bytes in the data section whose values look random, next to a small function that loops over them.
- A function called with a destination, a source, a length and a small constant, before each use of a string.
- A loop with `xor` of a byte from memory with a register or a constant.

The example `examples/20_strings.c` hides three strings in three ways. The program prints them:

```bash
make
build/20_strings
strings -a build/20_strings_stripped | grep -iE "http|temp|status|example"
```

```text
https://config.example.test/a
C:\Temp\out.dat
status=ok
```

The second command, run on the stripped program, prints nothing: none of the three strings exists in the file.

**Scheme 1: a fixed key.** The decoder is the shortest possible loop:

```text
401126:  test   rdx,rdx
401129:  je     401144
40112b:  mov    eax,0x0
401130:  mov    r8d,ecx                      ; the key, from the fourth argument
401133:  xor    r8b,BYTE PTR [rsi+rax*1]     ; key xor the encoded byte
401137:  mov    BYTE PTR [rdi+rax*1],r8b     ; store the decoded byte
40113b:  add    rax,0x1
40113f:  cmp    rdx,rax                      ; length in rdx
401142:  jne    401130
401144:  mov    BYTE PTR [rdi+rdx*1],0x0     ; terminator
401148:  ret
```

The arguments are the destination (`rdi`), the encoded bytes (`rsi`), the length (`rdx`) and the key (`ecx`). The caller passes the key as a constant (`mov ecx,0x5a`), so the key is visible in the code. A single-byte key can also be found without the code: the first bytes of a URL or a path are known (`http`, `C:\`), and xor-ing the first encoded byte with the known letter gives the key. The script `examples/20_decode.py` goes further and tries all 256 keys, keeping the results that look like a URL or path:

```bash
python3 20_decode.py
```

```text
blob A, every key that gives plausible text:
  key 0x5a: https://config.example.test/a
blob B with the rolling rule, seed 0x21:
  C:\Temp\out.dat
```

**Scheme 2: a rolling key.** The key changes with every byte, so one xor of the whole blob does not work:

```text
401149:  test   rdx,rdx
40114c:  je     40116b
40114e:  mov    eax,0x0
401153:  lea    ecx,[rcx+rcx*4+0x3]          ; k = k * 5 + 3
401157:  mov    r8d,ecx
40115a:  xor    r8b,BYTE PTR [rsi+rax*1]     ; xor with the low byte of k
40115e:  mov    BYTE PTR [rdi+rax*1],r8b
401162:  add    rax,0x1
401166:  cmp    rdx,rax
401169:  jne    401153
40116b:  mov    BYTE PTR [rdi+rdx*1],0x0
40116f:  ret
```

The signature is an update of the key register inside the loop, here `lea ecx,[rcx+rcx*4+3]`, which computes `k * 5 + 3`. To decode you need the rule and the seed (the starting value passed by the caller, 0x21). When the rule is simple, a few lines of script reproduce it, as in `20_decode.py`. When it is not, emulate the function.

**Scheme 3: a stack string.** The text never exists as one piece in the file. The code builds it byte by byte from constants in the instructions:

```text
401170:  mov    BYTE PTR [rdi],0x73          ; s
401173:  mov    BYTE PTR [rdi+0x1],0x74      ; t
401177:  mov    BYTE PTR [rdi+0x2],0x61      ; a
40117b:  mov    BYTE PTR [rdi+0x3],0x74      ; t
40117f:  mov    BYTE PTR [rdi+0x4],0x75      ; u
401183:  mov    BYTE PTR [rdi+0x5],0x73      ; s
401187:  mov    BYTE PTR [rdi+0x6],0x3d      ; =
40118b:  mov    BYTE PTR [rdi+0x7],0x6f      ; o
40118f:  mov    BYTE PTR [rdi+0x8],0x6b      ; k
401193:  mov    BYTE PTR [rdi+0x9],0x0
401197:  ret
```

A run of stores of immediate values to consecutive offsets is a stack string, and the immediates are printable ASCII. A disassembly listing shows the pieces, but a decompiler or a short script that reads the immediates in order gives the whole string. Compilers may merge the stores into wider ones (`mov DWORD PTR`, `movabs`), so look for the same pattern with 4-byte and 8-byte constants whose bytes are ASCII.

### Emulating a small routine

Decoding by hand works for simple schemes. For anything else, run the routine. An emulator executes the instructions of one function in a model of the processor, and gives you the result without running the program, without system calls and without any effect outside its own memory. The script `examples/20_emulate.py` uses the Unicorn emulator: it maps the file's loadable segments, sets up a stack, a return address and the arguments, runs from the address of the function until it returns, and reads the output buffer.

```bash
python3 20_emulate.py build/20_strings_stripped 401126 402020 29 5a
python3 20_emulate.py build/20_strings_stripped 401149 402010 15 21
```

```text
179 instructions emulated
https://config.example.test/a
110 instructions emulated
C:\Temp\out.dat
```

The arguments are the file, the address of the decoder, the address of the encoded data, its length and the key. All four numbers come from the caller: the stripped `main` loads them into `ecx`, `edx` and `esi` just before each `call` (`mov ecx,0x5a`, `mov edx,0x1d`, `mov esi,0x402020`, `call 401126`). That is the method: find the decoder by its shape, find a call to it, read its arguments from the instructions before the call, and emulate. The same approach reads whole tables of encoded strings: loop over the call sites and emulate each one.

Emulation has limits. A routine that makes system calls or calls library functions needs stubs for them, or has to be cut at that point. A routine that reads memory the emulation does not have (a global the program set up earlier) needs that memory filled in first.

### Junk code and opaque predicates

**Junk code** is code that has no effect on the result: computations whose values are never used, calls to functions that do nothing, jumps that go to the next instruction. It makes functions longer and hides the real structure. An **opaque predicate** is a condition that always has the same value, but whose value is hard to see at a glance. The branch is then either always taken or never taken, so one side is dead code; the point is to make the control-flow graph larger and to add work.

The function `score_flat` in `examples/20_flat.c` has both. Its junk is the update of `g_junk` (nothing reads it except the dead branch), and its opaque predicate is `(n * (n + 1)) % 2 == 1`. The product of a number and its successor is always even, so the predicate is false for every `n`:

```text
4011d2:  lea    eax,[rdx+0x1]                ; n + 1
4011d5:  imul   eax,edx                      ; n * (n + 1)
4011d8:  test   al,0x1                       ; low bit: the product is odd?
4011da:  je     4011e2                       ; the product is always even, so this jump is always taken
4011dc:  mov    ecx,DWORD PTR [rip+0x2e3a]   ; dead: r = g_junk
4011e2:  ...
```

To find opaque predicates, ask of every conditional branch whether the condition can depend on anything that changes. Arithmetic identities (a product of consecutive numbers is even; `x * x` is never 2; `(x | 1)` is never 0) are the usual source. Test your suspicion by emulating over several inputs, or by writing the identity down. When a branch is always one way, treat the dead side as removed.

### Control-flow flattening

A normal function is a graph of blocks that follow its logic. **Control-flow flattening** replaces the graph by a loop. A variable holds the number of the block to run next. A dispatcher at the top of the loop reads the variable and jumps to the matching block. Every block ends by storing the number of its successor and jumping back to the dispatcher. The structure of the program moves out of the shape of the code and into the values of the variable.

Compare the plain version of the function with the flattened one (the compiler produced both from the source in `20_flat.c`):

```text
0000000000401126 <score_plain>:
  401126:  mov    eax,edi
  401128:  mov    edx,0xaaaaaaab
  40112d:  imul   rax,rdx
  401131:  shr    rax,0x21                     ; n / 3 (module 14)
  401135:  lea    eax,[rax+rax*2]
  401138:  cmp    edi,eax                      ; n == (n / 3) * 3 ?
  40113a:  je     40115c
  ...
  40115b:  ret
```

```text
0000000000401160 <score_flat>:
  401162:  mov    DWORD PTR [rip+0x2eb4],0x0   ; g_state = 0
  401171:  mov    eax,DWORD PTR [rip+0x2ea9]   ; the dispatcher: load the state
  401177:  cmp    eax,0x4
  40117a:  ja     401230                       ; above 4: leave the loop
  401182:  jmp    QWORD PTR [rax*8+0x402008]   ; jump table indexed by the state
  ...                                          ; the blocks
  4011c6:  mov    DWORD PTR [rip+0x2e50],0x5   ; ... each one stores the next state
  4011d0:  jmp    401171                       ; ... and goes back to the dispatcher
```

The signs of flattening: one hub block with a jump table or a chain of comparisons on a single variable; many blocks that all end in a jump back to the hub; assignments of constants to that variable; and a function whose graph looks like a star. Two ideas help.

1. Each block's assignments tell you its successors. A block that stores one constant has one successor. A block that stores one of two values depending on a comparison is a branch, and the comparison is the original condition.
2. You can recover the graph by tracing. Run the function for a handful of inputs and record every value stored to the state variable; the sequences are paths through the original graph. The script `examples/20_trace_states.py` does this in the emulator, hooking the writes to the state variable's address (in a real sample it is a local variable or a register, and the idea is the same):

```bash
python3 20_trace_states.py build/20_flat 401160 404020
```

```text
n= 0: states 0 -> 1 -> 5  result 0
n= 1: states 0 -> 2 -> 4 -> 5  result 84
n= 2: states 0 -> 2 -> 4 -> 5  result 87
n= 3: states 0 -> 1 -> 5  result 6
n= 4: states 0 -> 2 -> 4 -> 5  result 81
n= 5: states 0 -> 2 -> 3 -> 5  result 12
...
n=14: states 0 -> 2 -> 4 -> 5  result 91
edges of the recovered graph:
  0 -> 1, 2
  1 -> 5
  2 -> 3, 4
  3 -> 5
  4 -> 5
```

The recovered graph is a tree with the shape of the original `if / else if / else`: state 0 tests divisibility by 3 (the inputs 0, 3, 6, 9, 12 take state 1, whose block doubles the input), state 2 tests divisibility by 5 (5 and 10 take state 3, which adds 7) and the rest take state 4 (an xor with 0x55). That is exactly `score_plain`. Notice that the opaque predicate in state 2 never changed the path.

### Recognizing debugger and analysis checks

Some programs look for signs that they are being analyzed and change their behavior when they find them, for example by exiting, by taking a harmless path, or by decoding something different. You need to recognize the checks during triage, because a program that behaves differently under observation can make a dynamic analysis misleading. The usual checks:

| Check | How it looks |
|---|---|
| A system call that fails if a tracer is attached (Linux: `ptrace` with the request that asks to be traced; Windows: `IsDebuggerPresent`, `CheckRemoteDebuggerPresent`, a read of the debug flag in the process block) | An import or a call to these functions, followed by a test of the result and a branch |
| Reading the process status for a tracer (Linux: the field `TracerPid` in `/proc/self/status`) | The string `/proc/self/status` and a comparison of the field with 0 |
| Timing | Two reads of a clock (`rdtsc`, a tick count) around a block, then a comparison with a limit |
| Breakpoint and exception tricks | Code that raises an exception or contains an interrupt instruction on purpose and expects a handler to run |
| Looking for analysis tools or virtual machines | Searches of the process list, window names, registry keys, device names and hardware identifiers |

The example `examples/20_dbg.c` makes two checks and only reports what it found. A plain run, a run under a system-call tracer and a run under the debugger give different answers:

```bash
build/20_dbg
strace -f -o /dev/null build/20_dbg
gdb -nx -q -batch -ex run build/20_dbg
```

```text
status check: no debugger
ptrace check: no debugger
status check: debugger detected
ptrace check: debugger detected
status check: debugger detected
ptrace check: debugger detected
```

(The first pair is the plain run, the second the run under `strace`, the third the run under `gdb`.) Static triage finds both checks without running anything: the program imports `ptrace` and `fopen`, and it contains the strings `/proc/self/status` and `TracerPid:`. When you find a check, write down where it is, what it tests, and what the program does differently when it fires. Static analysis and emulation are not affected by these checks, because the program is not executing in the normal sense, which is one more reason to prefer them. Where an observation has to be made while the program runs, state in the report that a check is present and that the observation may not represent a normal run.

### Scripting the analysis

When the same job repeats (many encoded strings, many call sites), write a script. In Ghidra, a script can read bytes at an address, decode them and put the result in a comment so that it appears next to the code. The sketch below shows the pattern for the scheme with a fixed key. It uses the Ghidra scripting interface, so it runs inside Ghidra on the host, not in the lab image, and the lab checks do not run it; check the names against the script manager's help for your version of Ghidra.

```text
# Decode an xor-encoded string and write it as a comment (Ghidra Script Manager, Python)
from ghidra.program.model.listing import CodeUnit
addr = toAddr(0x402020)                       # the address of the encoded bytes
data = getBytes(addr, 29)                     # 29 bytes
text = "".join(chr((b & 0xff) ^ 0x5a) for b in data)
currentProgram.getListing().setComment(addr, CodeUnit.EOL_COMMENT, text)
```

For many strings, collect the call sites of the decoder (the cross-references to its address), read the arguments from the instructions before each call, and apply the same decoding to each.

### A working order

1. **Triage.** Run the checks of module 19: strings, entropy, imports. Mark what is missing (no strings where you expect them) and what is odd (random-looking tables, a tiny import list).
2. **Find the decoders.** Look for small loops with an `xor`, for functions that are called many times with a constant and a length, and for runs of byte stores.
3. **Decode.** By hand or with a script for simple schemes, by emulation for the others. Put the results in comments or in a table.
4. **Find the structure.** Look for a hub block with many back edges. Trace the state variable and rebuild the graph.
5. **Slice by data flow.** Start from what the function returns or from the arguments of an interesting call and follow only the instructions that contribute. Everything else is noise.
6. **Note the checks.** List each analysis check you found, with its location and effect.

## Try it

Run the lab commands from `modules/20-reading-obfuscated-code/examples`.

1. Build with `make`. Search the stripped program for the three strings with `strings`, then print them by running the program. Which of the three schemes would you find with a search for `xor` in the disassembly?
2. Disassemble `build/20_strings_stripped`. Find the two decoders and the stack-string builder. In `main`, read the arguments of each call.
3. Use `20_emulate.py` to decode the second string. Change the seed by one and see what happens to the result.
4. Write a decoder for a third scheme of your own (for example, add the position of the byte to the key), encode a string with it, and check that `20_emulate.py` decodes it when you compile the decoder.
5. Disassemble `score_flat` and draw its blocks and the edges between them from the stores to the state variable. Compare the drawing with the output of `20_trace_states.py`.
6. Run `20_dbg` plainly, under `strace` and under `gdb`. Find the two checks with `strings` and `objdump` without running the program.

## Common pitfalls

- **Searching for the string instead of the decoder.** An encoded string cannot be found by its text. Find the code that produces it.
- **Assuming the key is a constant.** A key that changes in the loop (rolling) or depends on the position needs the rule, not a single value.
- **Emulating without checking the result.** Verify the output looks like what it should: printable text, a plausible path. A wrong argument gives noise, which looks like a failed decoder.
- **Following the dispatcher instead of the data.** In a flattened function, the order of the blocks in memory means nothing. Follow the state variable.
- **Treating a dead branch as real.** A branch whose condition is an identity is not behavior. Check it before you document it.
- **Forgetting that the program may watch you.** A result obtained under a debugger may differ from a normal run. Say so in the report.

## Summary

- Obfuscation changes how hard code is to read, not what it does, so the behavior can always be recovered.
- Encoded strings show up as a missing string, a table of random-looking bytes and a small decoding loop; the schemes are a fixed key, a rolling key and a stack string.
- Emulating a small routine gives its result without running the program; find the routine, read its arguments from the call sites and run it.
- Junk code and opaque predicates add noise; a condition that is an arithmetic identity is always the same and its dead side can be removed.
- Control-flow flattening replaces the graph with a dispatcher and a state variable; the assignments or a trace of the variable give the original graph back.
- Debugger and analysis checks are recognized in triage by their imports, strings and compare-and-branch patterns, and they are noted in the report.
- A working order, from triage to decoding to structure to data flow, keeps the work cheap.

## Next steps

- Take the [quiz](quiz.md).
- Continue with module 21, virtual machines and bytecode.
- Exercises that use this module: [Exercise 19](../../exercises/19-triage-desk/README.md).
