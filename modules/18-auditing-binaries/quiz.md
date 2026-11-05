[English](quiz.md) | [العربية](quiz.ar.md) | [Deutsch](quiz.de.md) | [Español](quiz.es.md) | [Français](quiz.fr.md)

# Quiz: 18. Auditing binaries for bugs

Answer first, then open the answer.

**1. What are a source and a sink, and how do you use them to start an audit?**

<details>
<summary>Answer</summary>

A source is a value the program does not control, such as a field read from a file or a packet. A sink is a place where a value decides how much memory is touched: a copy, an index, an allocation, a free. You find the sinks in the disassembly, trace each size or index operand back to its source, and read every comparison on the way.

</details>

**2. A function compares `n` with `cap`, rejects the input if `n > cap`, and then copies `n + 1` bytes. What is wrong?**

<details>
<summary>Answer</summary>

An off-by-one error: a string with exactly `cap` characters passes the test but the copy needs `cap + 1` bytes with the terminator. The check must compare the quantity that is copied, so the input must be rejected when `n >= cap`.

</details>

**3. What combination of instructions signals a signedness bug before a copy?**

<details>
<summary>Answer</summary>

A signed comparison that guards only the upper end (`cmp`, then `jg` or `jle`), followed by a sign extension (`movsxd`, `cdqe`) when the value becomes the 64-bit size of the copy. A negative value passes the test and turns into a huge unsigned size. A correct check is a single unsigned comparison, or two comparisons that cover both ends.

</details>

**4. What in the disassembly of an allocation suggests an integer overflow, and why does it matter that the inputs are used again later?**

<details>
<summary>Answer</summary>

An arithmetic instruction (such as a 32-bit `imul` or `add`) on values derived from the input whose result becomes the size passed to `malloc`. If the same inputs control a later loop or index, the loop assumes a size that the wrapped allocation does not have, and it writes beyond the block.

</details>

**5. Name four library functions that cannot be used safely with an input-dependent source and a fixed buffer, and one that takes a size but is still often misused.**

<details>
<summary>Answer</summary>

`strcpy`, `strcat`, `sprintf` and `gets` take no destination size (`scanf` with `%s` and no width is the same). `strncpy` takes a size but does not write a terminator when the source is as long as the limit, and `memcpy` takes a size that the caller must have checked.

</details>

**6. What does a use after free look like in disassembly?**

<details>
<summary>Answer</summary>

A path through a `call free` on a pointer that continues to an instruction which dereferences the same pointer (the register or stack slot that held the argument of `free`). A second `call free` on the same value is a double free.

</details>

**7. Why is it better to stop a debugger before the faulty copy than to let the program run past it?**

<details>
<summary>Answer</summary>

At the breakpoint you can read the size and the capacity, which is the evidence the report needs, and nothing outside the buffer is written. Letting the oversized copy run only adds damage and noise, and it is not needed for a diagnosis.

</details>

**8. What must a bug report contain, and how do you check that a fix works?**

<details>
<summary>Answer</summary>

The class of the weakness, the exact location, the cause, the evidence, the impact (stated without exploitation, including what was not assessed), the fix and a test. A fix is checked by repeating the triggering input and seeing that the report is gone, and by running boundary values: the largest accepted, the smallest rejected, zero, negative and the value that would wrap.

</details>
