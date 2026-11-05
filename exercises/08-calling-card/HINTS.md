[English](HINTS.md) | [العربية](HINTS.ar.md) | [Deutsch](HINTS.de.md) | [Español](HINTS.es.md) | [Français](HINTS.fr.md)

# Hints: exercise 08

Open one level at a time. Try again between levels.

## Part 1: Reverse

<details>
<summary>Level 1: nudge</summary>

Look at the last instruction of each function and at where it reads its arguments. In the 32-bit binary, also look at what the caller does right after each `call`.

</details>

<details>
<summary>Level 2: direction</summary>

A `ret` with a number removes that many bytes of arguments, so the function is not cdecl. Without a number, the caller must adjust `esp` after the call, or the function takes no stack arguments. A function that reads `ecx` or `edx` before writing them takes register arguments. In the 64-bit binary, compare the registers read by `g1` and `g2`, and look at the `sub rsp` in the caller before `g1`.

</details>

<details>
<summary>Level 3: nearly the answer</summary>

Two of the 32-bit functions end in `ret` with a number: the one that removes twelve bytes takes three stack arguments (stdcall), the one that removes four takes a single stack argument because two others come in registers (fastcall). The remaining two end in plain `ret` and their callers clean up (cdecl). In the 64-bit binary, the function that saves `rcx` and `rdx` into the area above its return address is using shadow space (Microsoft x64); the other reads `rdi`, `rsi` and `rdx` (System V).

</details>

## Part 2: Assemble

<details>
<summary>Level 1: nudge</summary>

Write the formula of each function on paper first. Then decide, for each convention, where the three arguments are at entry.

</details>

<details>
<summary>Level 2: direction</summary>

At entry, `[esp]` holds the return address. For the two stack conventions the arguments start at `[esp + 4]`. For fastcall, `a` is in `ecx` and `b` is in `edx`, and only `c` is at `[esp + 4]`. Use `ret N` where the callee must remove bytes.

</details>

<details>
<summary>Level 3: nearly the answer</summary>

The stdcall function removes 12 bytes (three arguments) and the fastcall function removes 4 bytes (one argument). The cdecl function ends in a plain `ret`. If the checker prints `stack` instead of `ok`, the byte count of one `ret` is wrong, or a plain `ret` is used where a number is needed. If the program crashes, the function removed too many bytes and the caller returned to a wrong address.

</details>
