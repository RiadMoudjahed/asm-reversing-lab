[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# Exercise 19. Triage desk

Level 5. Time: about 5 hours. Architecture: x86-64 Linux and Windows. Modules: [19. Analyzing suspicious programs](../../modules/19-analyzing-suspicious-programs/README.md), [20. Reading obfuscated code](../../modules/20-reading-obfuscated-code/README.md)

## Scenario

The security team of a small company received a program, `dispatch`, from an alert on a workstation, together with a Windows build of the same program, `dispatch.exe`. Nobody knows what it does. It must be triaged before the end of the day: what does it do, how can the team find other copies of it, and how can the team read its configuration without running anything risky? The program in this exercise is a simulator written for the course. It only prints what it would do and performs no action. You write the triage notes and the detection rule, and then you write the routine that decodes its configuration.

## Objectives

- Triage an unknown program in a fixed order: identification, strings, imports, behavior, and decoding of hidden data.
- Find an encoded configuration, recover the decoding rule from the code, and decode every entry.
- Map observed behavior to ATT&CK technique identifiers, and state what the evidence supports.
- Write a YARA rule that finds the sample and does not fire on programs that only look similar.
- Re-implement the decoder in assembly so that it can be applied to configurations from other samples.

## Part 1: Reverse

The files are `bin/dispatch` (Linux) and `bin/dispatch.exe` (Windows). They print a few log lines and exit. Running them is safe, and it is where you should start, but the log lines contain only placeholders for the interesting data: the data itself is stored in encoded form and is decoded in memory only.

```bash
bin/dispatch
file bin/dispatch bin/dispatch.exe
strings -n 6 bin/dispatch
```

Work through the triage of module 19 and the decoding methods of module 20. Find the table that holds the encoded entries, recover the decoding rule from the code, and decode all of them. Then prepare the following values and files:

1. `N`: the number of entries in the encoded configuration, in decimal.
2. `L`: the length in bytes of the longest decoded entry, in decimal.
3. The ATT&CK technique identifiers of three behaviors, each written as the full identifier of the technique or sub-technique (a `T`, four digits, a dot and three digits), but with the `T` and the dot left out in the flag: the creation of a service, the placement of a library next to a trusted program so that the program loads it, and the use of DNS names as a carrier for data.
4. A YARA rule in `starter/rule.yar`. It must match both `bin/dispatch` and `bin/dispatch.exe`, and it must not match the other programs that the checker tries: three programs that mention the same topics in a harmless way (a help text, a dry run, an updater that uses the same decoding routine for harmless texts) in both formats, and a system program. A rule that fires on any of those would flood the analysts with false alarms.

The flag combines the first three values in this order: `N`, `L`, then the three identifiers (service, side-loading, DNS) with the `T` and the dot removed.

```text
ARL{triage_<N>_<L>_<service>_<sideload>_<dns>}
```

For example, the values 3, 20 and the identifiers T1234.567, T1234.568 and T1234.569 would give `ARL{triage_3_20_1234567_1234568_1234569}`.

The log lines also describe two more behaviors that you should map in your notes, although the flag does not ask for them: how the program learns about the system around it, and what it does to the traces it leaves. Writing the notes is part of the exercise. The checker does not read them; the write-up in `solutions/19/` contains a model for you to compare with afterwards.

## Part 2: Assemble

Edit `starter/solution.asm` to implement one function in the System V calling convention:

```c-fragment
long decode_record(const unsigned char *rec, size_t avail, char *out, size_t cap);
```

`rec` points at one encoded entry in the layout that you found in Part 1, and `avail` is the number of bytes that you may read starting at `rec`. The function writes the decoded text to `out` followed by a zero byte, and returns the length of the text. The rules, in the order they are applied:

- If `avail` is less than 2, or less than 2 plus the length stored in the first byte of the entry, the entry is not fully inside the bytes you may read: return -1 and write nothing.
- If `cap` is smaller than the length of the text plus one, the output does not fit: return -2 and write nothing.
- Otherwise decode the entry exactly as the simulator does, write the text and the terminating zero byte, and return the length of the text. An entry of length 0 gives an empty text, the zero byte and the return value 0.

The tests include the entries of the sample, the same entries cut at every position, entries with edge-case lengths and seeds, and random entries. They compare the return value and the output with a reference.

## Rules

- Never read a byte at or beyond `rec + avail`. The tests place the entry just before an unmapped page, and a read beyond the end ends the test run with a signal.
- Never write beyond `cap` bytes of `out`, and write nothing when you return an error.
- Preserve the callee-saved registers `rbx`, `rbp` and `r12` to `r15`.
- Keep the stack aligned at every `call`.

## Check your work

```bash
make check EX=19 ARGS='--flag ARL{your_answer_here}'
```

The checker reports Part 1 and Part 2 separately. Part 1 passes only when the flag is correct and the YARA rule in `starter/rule.yar` is correct; the output of the checker names each program on which the rule gives the wrong answer. Without `--flag`, it checks Part 2 only.

## Hints

Open [HINTS.md](HINTS.md) one level at a time.
