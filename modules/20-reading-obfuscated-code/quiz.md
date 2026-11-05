[English](quiz.md) | [العربية](quiz.ar.md) | [Deutsch](quiz.de.md) | [Español](quiz.es.md) | [Français](quiz.fr.md)

# Quiz: 20. Reading obfuscated code

Answer first, then open the answer.

**1. Why can obfuscated code always be understood in principle?**

<details>
<summary>Answer</summary>

Obfuscation changes how hard code is to read, not what it does. The processor still has to execute the real behavior, so the behavior can be recovered by reading, by emulating small parts, or by following the data.

</details>

**2. A program obviously contacts a server, but `strings` shows no host name. What do you look for?**

<details>
<summary>Answer</summary>

Encoded strings: a table of random-looking bytes, a small loop with an `xor`, a function called with a destination, a source, a length and a key, or a run of byte stores of printable constants (a stack string). The text cannot be found, so you find the code that produces it.

</details>

**3. What distinguishes a fixed-key decoder from a rolling-key decoder in the disassembly?**

<details>
<summary>Answer</summary>

A fixed-key decoder xors every byte with the same value. In a rolling-key decoder the key register is updated inside the loop (for example `lea ecx,[rcx+rcx*4+3]`, which is `k * 5 + 3`), so each byte uses a different key and you need the rule and the starting value.

</details>

**4. How do you get the plaintext of an encoded string without running the program?**

<details>
<summary>Answer</summary>

Find the decoder and a call to it, read the arguments (destination, encoded bytes, length, key) from the instructions before the call, and either reproduce the rule in a script or emulate the decoder with an emulator. For a single-byte key you can also xor the first encoded byte with the first letter you expect, such as the `h` of `http`.

</details>

**5. What is an opaque predicate, and how do you recognize one?**

<details>
<summary>Answer</summary>

A condition that always has the same value, though that is hard to see at a glance, so one side of the branch is dead code. It is usually an arithmetic identity, such as the product of a number and its successor being even. Test it by writing the identity down or by emulating over several inputs.

</details>

**6. What does a control-flow-flattened function look like, and how do you recover its original graph?**

<details>
<summary>Answer</summary>

One hub block (a jump table or chain of comparisons on one variable) that all other blocks jump back to, and blocks that store constants into that variable. Read each block's stored constants to find its successors, or run the function for several inputs and record the values stored to the state variable; the sequences are paths through the original graph.

</details>

**7. How do you recognize a debugger check during triage, without running the program?**

<details>
<summary>Answer</summary>

By its imports, strings and patterns: a call to a function that fails when a tracer is attached (`ptrace` with the request to be traced, `IsDebuggerPresent`), the string `/proc/self/status` with a comparison of `TracerPid` with 0, two reads of a clock around a block, or searches for analysis tools. Record the location, the test and what the program does differently.

</details>

**8. Why are static analysis and emulation less affected by analysis checks than running the program?**

<details>
<summary>Answer</summary>

The checks fire when the program runs in the normal sense under observation. Reading the code or emulating one routine does not run the program, so the checks never fire. If you must observe a run, state in the report that a check is present and that the observation may differ from a normal run.

</details>
