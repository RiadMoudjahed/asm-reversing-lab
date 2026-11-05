[English](WRITEUP.md) | [العربية](WRITEUP.ar.md) | [Deutsch](WRITEUP.de.md) | [Español](WRITEUP.es.md) | [Français](WRITEUP.fr.md)

# Write-up: exercise 12. Switchboard

The flag is `ARL{switch_11_3_8_2_4011c0_10}`: the jump table has 11 entries, the lowest command is 3, the table reaches 8 different code addresses besides the default, 2 entries point to the default, command 7 is at 0x4011c0, and command 10 shares its code with command 6.

## Part 1: Reverse

### The dispatcher

```bash
objdump -d -M intel --no-show-raw-insn bin/switchboard
```

```text
00000000004011a0 <dispatch>:
  4011a0:    sub    edi,0x3
  4011a3:    cmp    edi,0xa
  4011a6:    ja     401208 <dispatch+0x68>
  4011a8:    jmp    QWORD PTR [rdi*8+0x402008]
  4011b0:    add    esi,0x5
  4011b3:    lea    eax,[rsi+rsi*2+0x1]
  4011b7:    ret
  4011c0:    shl    esi,0x2
  4011c3:    lea    eax,[rsi+0x65]
  4011c6:    ret
  4011d0:    xor    esi,0x55
  4011d3:    lea    eax,[rsi+0x1]
  4011d6:    ret
  4011e0:    mov    eax,0x1
  4011e5:    sub    eax,esi
  4011e7:    ret
  4011f0:    or     esi,0x1
  4011f3:    lea    eax,[rsi+0x1]
  4011f6:    ret
  401200:    lea    eax,[rsi-0x2]
  401203:    ret
  401208:    xor    eax,eax
  40120a:    ret
```

The padding `nop` instructions between the blocks are left out. The first three instructions are the usual pattern: subtract the lowest case value, compare with the highest index, jump to the default if the index is above it (unsigned). The subtraction is 3, so `L` is 3. The comparison is 10, so the highest index is 10 and the table has 11 entries, so `N` is 11. The `jmp` reads from 0x402008.

### The table

```bash
objdump -s -j .rodata bin/switchboard
```

```text
 402000 01000200 00000000 b0114000 00000000
 402010 b3114000 00000000 00124000 00000000
 402020 d0114000 00000000 c0114000 00000000
 402030 c3114000 00000000 08124000 00000000
 402040 d0114000 00000000 08124000 00000000
 402050 e0114000 00000000 f0114000 00000000
```

The first 8 bytes at 0x402000 belong to the program's data (the standard library's marker value); the table begins at 0x402008, as the `jmp` says. Each entry is 8 bytes in little-endian order, so `b0 11 40 00 00 00 00 00` is 0x4011b0. Reading 11 entries and attaching the command number (index + 3):

| Command | Address | What the code at the address does |
|---|---|---|
| 3 | 0x4011b0 | `acc + 5`, then continues into the code of command 4 |
| 4 | 0x4011b3 | `acc * 3 + 1`; this address is inside the block that starts at 0x4011b0 |
| 5 | 0x401200 | `acc - 2` |
| 6 | 0x4011d0 | `(acc ^ 0x55) + 1` |
| 7 | 0x4011c0 | `acc << 2`, then continues into the code of command 8 |
| 8 | 0x4011c3 | `acc + 100 + 1`; this address is inside the block that starts at 0x4011c0 |
| 9 | 0x401208 | the default code |
| 10 | 0x4011d0 | the same address as command 6 |
| 11 | 0x401208 | the default code |
| 12 | 0x4011e0 | `1 - acc`, which is `-acc + 1` |
| 13 | 0x4011f0 | `acc` with bit 0 set, plus 1 |

The default code at 0x401208 is `xor eax, eax`, which returns 0. In the source this is the result of setting `acc` to 0xFFFFFFFF and then adding 1.

The answers:

| Value | Reasoning |
|---|---|
| `N` | 11 entries |
| `L` | 3, from `sub edi, 3` |
| `B` | The addresses other than the default are 0x4011b0, 0x4011b3, 0x401200, 0x4011d0, 0x4011c0, 0x4011c3, 0x4011e0 and 0x4011f0, which is 8. Commands 6 and 10 share 0x4011d0 and count once |
| `G` | Commands 9 and 11 point to 0x401208, which is 2 |
| `A` | The entry for command 7 is `c0 11 40 ...`, so `4011c0` |
| `S` | The entry for command 10 equals the entry for command 6, so 10 |

The flag is `ARL{switch_11_3_8_2_4011c0_10}`.

Two details worth noticing. The compiler turned a fall-through into one block with two entry points: the entry for command 4 is the middle of command 3's code, so a command-3 call executes the addition and then the multiplication, while a command-4 call enters after the addition. The same applies to commands 7 and 8. And the commands 9 and 11 are not special in the table; they are holes in the range that go to the same default as every out-of-range command.

## Part 2: Assemble

The reference solution is `solution.asm`. The points that matter:

- **The index and the bounds check.** `eax = cmd - 3` and `cmp eax, 10` followed by `ja`. The comparison is unsigned, so every command below 3 (the subtraction wraps to a huge number) and above 13 goes to the default with a single test.
- **The table.** It sits in `.rodata`, with one `dq` address per command from 3 to 13. In NASM a local label such as `.c3` belongs to the last non-local label, so the table refers to `run_cmd.c3`.
- **Shared code.** Commands 6 and 10 have the same address. Commands 9 and 11 point to the default.
- **Fall-through.** The code for command 3 does `add esi, 5` and then flows into the label of command 4, without a jump. The same holds for command 7 and command 8.
- **The results.** Each block ends in `lea eax, [...]` that computes the final value and `ret`. The final `+ 1` of `dispatch` is folded into each `lea`, and command 5 skips it.

## Mistakes that cost time

- **A bound that is one too small.** `cmp eax, 9` makes command 13 go to the default; the checker lists every command-13 case.
- **A signed comparison.** `jg` instead of `ja` lets negative indexes through, and the indirect jump leaves the table; the program ends with signal 11.
- **A wrong entry for a shared block.** Pointing command 10 at another block gives wrong results for command 10 only.
- **Returning at the end of command 3's block.** The result for command 3 lacks the multiplication.
- **A default that returns -1.** Every command outside 3 to 13, and the commands 9 and 11, give a wrong result.
- **A chain of comparisons.** It can give right results for some commands, but the checker rejects it for more than two `cmp` instructions and for the missing indirect jump.
