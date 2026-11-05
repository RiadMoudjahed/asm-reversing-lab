[English](WRITEUP.md) | [العربية](WRITEUP.ar.md) | [Deutsch](WRITEUP.de.md) | [Español](WRITEUP.es.md) | [Français](WRITEUP.fr.md)

# Write-up: exercise 03. Register ledger

The flag is `ARL{ledger_37000_1010_1100_0011}`. At `mark1` the register `rax` holds 37000; at the next three marks the flags are CF and SF, then CF and ZF, then SF and OF. The reference solution captures flags with `setc`, `setz`, `sets` and `seto` and combines them with shifts and `or`.

## Part 1: Reverse

### Read the program first

```text
$ objdump -d -M intel --no-show-raw-insn bin/ledger
  mov    eax,0x3e8
  imul   rax,rax,0x25
<mark1>:
  sub    rax,0x9c40
<mark2>:
  mov    ecx,0xffffffff
  add    ecx,0x1
<mark3>:
  movabs rdx,0x7fffffffffffffff
  add    rdx,0x1
<mark4>:
```

Four short calculations, each followed by a mark. Predicting the flags before you run the program is the point of the exercise: do it for every mark, then check with the debugger. The assembler wrote `mov rax, 1000` as `mov eax, 0x3e8`, which also clears the upper half of `rax`; `movabs` is the form of `mov` that carries a 64-bit constant.

### Confirm in the debugger

```text
$ gdb -q bin/ledger
(gdb) break mark1
(gdb) break mark2
(gdb) break mark3
(gdb) break mark4
(gdb) run
(gdb) p/d $rax
(gdb) continue
(gdb) p/x $rax
(gdb) p $eflags
...
```

The values at each mark, from a real run:

| Mark | Instruction just above | Result | Flags GDB lists |
|---|---|---|---|
| `mark1` | `imul rax, rax, 37` | `rax` = 37000 (`0x9088`) | not recorded (`imul` leaves SF and ZF undefined) |
| `mark2` | `sub rax, 40000` | `rax` = `0xfffffffffffff448` = -3000 | CF PF SF |
| `mark3` | `add ecx, 1` | `ecx` = 0, `rcx` = 0 | CF PF AF ZF |
| `mark4` | `add rdx, 1` | `rdx` = `0x8000000000000000` | PF AF SF OF |

Reading the flags:

- **`mark2`:** 37000 minus 40000 needs a borrow, because 37000 is smaller than 40000 as unsigned numbers (CF). The result is negative (SF). No signed overflow (OF is clear). PF is the parity of the low byte and is not asked for.
- **`mark3`:** `0xFFFFFFFF + 1` wraps to 0 in 32 bits with a carry out (CF) and a zero result (ZF). Because the register write was 32 bits, the upper half of `rcx` is cleared too, so `rcx` is 0.
- **`mark4`:** `0x7FFFFFFFFFFFFFFF + 1` is the largest positive signed number plus one. As a signed result it wrapped to the most negative number (OF, SF). As an unsigned addition nothing carried out (CF is clear).

In the order CF, ZF, SF, OF: `mark2` is `1010`, `mark3` is `1100`, `mark4` is `0011`. The flag is `ARL{ledger_37000_1010_1100_0011}`.

## Part 2: Assemble

The reference solution is `solution.asm`. Each function performs its operation, stores the result with `mov` (which leaves flags alone), and jumps to code shared by both:

```nasm-fragment
ledger_add:
    add     rdi, rsi
    mov     [rdx], rdi
    jmp     collect_flags

collect_flags:
    setc    al                          ; all four setcc first,
    setz    cl                          ; while the flags are still those of the operation
    sets    r8b
    seto    r9b
    movzx   eax, al
    movzx   ecx, cl
    shl     ecx, 1
    or      eax, ecx
    ; ... SF at bit 2 and OF at bit 3 the same way
    ret
```

For the Part 1 subtraction, `ledger_sub(37000, 40000)` returns `5`: CF in bit 0 and SF in bit 2.

## Mistakes that cost time

- **Reading the flags after other arithmetic.** `or`, `shl` and `add` all rewrite flags. Run every `set` instruction first and combine the bytes afterwards. With a stray `add rdx, 0` before the collection, 6 of the 17 test cases fail.
- **Using `cmp` or `test` to rebuild the flags.** It looks equivalent, but the checker forbids it, and `cmp` also overwrites the flags the operation just set.
- **Using `lea` to add.** `lea rax, [rdi + rsi]` computes the sum but sets no flags at all.
- **Forgetting that `ledger_sub` borrows.** CF after `sub` means the first operand was smaller than the second as unsigned numbers.
- **Not storing the result.** The tests fill `*out` with a sentinel value first; skipping the store fails every case.
