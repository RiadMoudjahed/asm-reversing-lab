[English](HINTS.md) | [العربية](HINTS.ar.md) | [Deutsch](HINTS.de.md) | [Español](HINTS.es.md) | [Français](HINTS.fr.md)

# Hints: exercise 19

Open one level at a time. Try again between levels.

## Part 1: Reverse

<details>
<summary>Level 1: nudge</summary>

Run the program and read the log lines, then compare them with what `strings` finds: the lines contain placeholders where the real data should be. Find the function that is called once for every placeholder, and the table in the read-only data that it reads from. The table is not text. Look for the loop in the function: it is short, and every operation in it is one you know from module 20.

</details>

<details>
<summary>Level 2: direction</summary>

The loop keeps one byte of state that changes at every position, and the plain byte is the encoded byte combined with that state. Write down the operations that change the state: one rotation, one addition of the position, one addition of a constant. The first two bytes of every entry are not encoded text: one of them is the number of bytes that follow, and the other starts the state. For the techniques, decide what each decoded text is (a name, a path, a file name, a domain) and look up the behavior in the ATT&CK matrix for Windows: services, library search order, application layer protocols. For the rule, use what is really in the file and not what it says: compare the plain text in the sample with the decoys, and choose strings that only the sample has.

</details>

<details>
<summary>Level 3: nearly the answer</summary>

Decode by hand the first entry with a script that follows the loop exactly, using 8-bit arithmetic: rotate the state left by three, add the position, add the constant, and combine with xor. When the first decoded entry reads as a service name, the rule is right; the same script gives all the others. Count the entries from the table, or from the number of times the function is called; take the lengths from the first byte of each entry. For the rule, the decoys contain the words of the topic but not the log lines of the behaviors, and the updater contains the same decoder but different data: a rule built on the decoder alone matches the updater, and a rule built on one common word matches the dry run. Combine several of the sample's own log templates, require the format of the executable, and add the first bytes of the first encoded entry.

</details>

## Part 2: Assemble

<details>
<summary>Level 1: nudge</summary>

Do the two size checks first, in the order of the specification, before you read or write any byte of the entry. Then write the loop of the decoder that you found in Part 1: the state is one byte, so work in an 8-bit register.

</details>

<details>
<summary>Level 2: direction</summary>

Read the length from the first byte of the entry and compute the bytes needed as the length plus 2; compare that with `avail`. The room needed in `out` is the length plus 1; compare that with `cap`. In the loop, keep the position in a 64-bit register, use its low byte for the addition to the state, and read the encoded byte at the entry address plus the position plus 2. After the loop, store the zero byte and return the length. Registers `rax`, `rcx`, `rdx`, `rsi`, `rdi` and `r8` to `r11` can be changed freely.

</details>

<details>
<summary>Level 3: nearly the answer</summary>

The loop body has four steps: rotate the state byte left by 3 (`rol` on an 8-bit register), add the low byte of the position, add the constant you found, and xor the result into the encoded byte. The first value of the state is the second byte of the entry. If a test with a cut entry ends with a signal, the check on `avail` is wrong or comes too late; if a test with a small `cap` fails, check that the two error codes are returned in the order of the specification and that nothing is written before the checks.

</details>
