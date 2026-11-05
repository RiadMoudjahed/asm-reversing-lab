[English](WRITEUP.md) | [العربية](WRITEUP.ar.md) | [Deutsch](WRITEUP.de.md) | [Español](WRITEUP.es.md) | [Français](WRITEUP.fr.md)

# Write-up: exercise 06. Branch maze

The flag is `ARL{maze_101_m31_m56}`. The accepted inputs are A in {101, 109, 117}, B from -31 to -21, and C equal to -56, which is 33 triples in all. The smallest A is 101, the smallest B is -31, and C has one possible value.

## Part 1: Reverse

### Running the program tells you nothing

```text
$ bin/maze 1 2 3
wall
```

The program never says which check failed, so guessing does not scale. The rules are in the disassembly.

### Read the checks

After the three calls that convert the arguments to numbers, `main` holds A in `ebp`, B in `r12d` and C in `eax`. The checks, from the real disassembly:

```text
  lea    edx,[rbp-0x64]
  cmp    edx,0x14
  ja     <wall>
  and    ebp,0x7
  cmp    ebp,0x5
  jne    <wall>
  add    r12d,0x1f
  cmp    r12d,0xa
  ja     <wall>
  test   eax,eax
  jns    <wall>
  cmp    al,0xc8
  jne    <wall>
  cmp    eax,0xfffffed4
  jl     <wall>
```

Reading each pair:

| Instructions | Rule | Values allowed |
|---|---|---|
| `lea edx,[rbp-0x64]`, `cmp edx,0x14`, `ja` | A minus 100 is at most 20 as an unsigned number | A from 100 to 120 |
| `and ebp,7`, `cmp ebp,5`, `jne` | The low three bits of A are 101 | A is 5 more than a multiple of 8 |
| `add r12d,0x1f`, `cmp r12d,0xa`, `ja` | B plus 31 is at most 10 as an unsigned number | B from -31 to -21 |
| `test eax,eax`, `jns` | C is negative (jump if the sign flag is clear) | C below 0 |
| `cmp al,0xc8`, `jne` | The low byte of C is `0xC8` (200) | C is 200 modulo 256 |
| `cmp eax,0xfffffed4`, `jl` | C is at least -300 as a signed number (`0xfffffed4` is -300) | C from -300 up |

The two unsigned jumps (`ja`) with a subtraction before them are range checks. The `jl` at the end is signed because the bound is a negative number. If it had been `jb`, the bound would have been the large unsigned number `0xfffffed4`.

### Combine the rules

- A is between 100 and 120 and equals 5 modulo 8: 101, 109 or 117.
- B is between -31 and -21.
- C is negative, equals 200 modulo 256, and is at least -300. The negative numbers that are 200 modulo 256 are -56, -312, -568 and so on, and only -56 is at least -300.

The smallest triple is 101, -31, -56:

```text
$ bin/maze 101 -31 -56
path open
$ bin/maze 100 -31 -56
wall
$ bin/maze 101 -32 -56
wall
$ bin/maze 101 -31 -312
wall
```

The flag is `ARL{maze_101_m31_m56}`.

## Part 2: Assemble

The reference solution is `solution.asm`. It repeats the same checks in the same order, with every failing jump going to one label:

```nasm-fragment
maze_ok:
    lea     eax, [rdi - 100]            ; a - 100 as unsigned: one test for both bounds
    cmp     eax, 20
    ja      .no
    ; ... the AND 7 check on a, the range check on b ...
    test    edx, edx
    jns     .no                         ; c must be negative (signed)
    cmp     dl, 200                     ; the low byte of c
    jne     .no
    cmp     edx, -300
    jl      .no                         ; c >= -300 (signed)
    mov     eax, 1
    ret
.no:
    xor     eax, eax
    ret
```

## Mistakes that cost time

- **Using a signed jump where the check is unsigned, or the reverse.** Replacing the sign test on `c` with an unsigned jump rejects every input, so the 10 accepted test cases fail.
- **Off by one in a range.** Using `b + 30` and 9 instead of `b + 31` and 10 rejects `b = -31`; the tests include both boundary values.
- **Forgetting a rule.** Without the `and 7` check on `a`, 11 of the 90 test cases fail, because values such as 116 and 120 are accepted by mistake.
- **Using `setcc` or `cmov`.** They give the right answers, and the checker rejects them by rule.
- **Trusting trial and error in Part 1.** The program gives no hint about which check failed; the only way through is to read the comparisons.
