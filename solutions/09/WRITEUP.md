[English](WRITEUP.md) | [العربية](WRITEUP.ar.md) | [Deutsch](WRITEUP.de.md) | [Español](WRITEUP.es.md) | [Français](WRITEUP.fr.md)

# Write-up: exercise 09. Stack archaeology

The flag is `ARL{stack_2_40_64_56_16}`: `chain` pushes 2 callee-saved registers, subtracts 40 bytes, occupies 64 bytes per level, has the saved return address 56 bytes above the start of its table, and leaves 16 bytes of its local area unused.

## Part 1: Reverse

### The prologue and epilogue

```text
chain:
    push   rbp
    ...
    push   rbx
    ...
    sub    rsp,0x28
    ...
    mov    rsi,rsp              ; second argument of note: the table
    mov    QWORD PTR [rsp+0x8],rax
    mov    QWORD PTR [rsp],rbx
    mov    QWORD PTR [rsp+0x10],rbp
    call   note
    ...
    call   chain
    ...
    add    rsp,0x28
    pop    rbx
    pop    rbp
    ret
```

There is no `push rbp; mov rbp,rsp`: `rbp` is saved only because the function uses it as an ordinary register, here to hold a value across the calls. The compiler also uses `rbx` that way.

| Number | Value | Evidence |
|---|---|---|
| `P` | 2 | Two `push` instructions, `rbp` and `rbx`, matched by two `pop` instructions |
| `S` | 40 | `sub rsp,0x28` in the prologue, `add rsp,0x28` in the epilogue |
| `L` | 64 | 8 (return address) + 16 (two pushes) + 40 (subtraction) |
| `O` | 56 | The table starts at `rsp`; the pushed registers are at `rsp + 0x28` and `rsp + 0x30`; the return address is at `rsp + 0x38` |
| `U` | 16 | The table uses `[rsp]` to `[rsp + 0x17]`, which is 24 of the 40 bytes |

### The frame

Higher addresses are at the top. Offsets are from `rsp` while `chain` is running.

```text
higher addresses
  [rsp + 0x38]  return address into the caller           8 bytes
  [rsp + 0x30]  saved rbp                                8 bytes
  [rsp + 0x28]  saved rbx                                8 bytes
  [rsp + 0x20]  unused                                   8 bytes
  [rsp + 0x18]  unused                                   8 bytes
  [rsp + 0x10]  table[2]                                 8 bytes
  [rsp + 0x08]  table[1]                                 8 bytes
  [rsp]         table[0]    <- rsp during the calls      8 bytes
lower addresses
```

At entry, `rsp` is 8 modulo 16, because the caller's `call` pushed the return address on a 16-byte boundary. The two pushes keep it at 8 modulo 16, so the subtraction has to be 8 modulo 16 as well to leave `rsp` at a multiple of 16 before each `call`. Smaller values that satisfy this are 24, 40 and so on. The table needs 24 bytes; the compiler reserved 40 and left 16 of them unused. The listing does not say why, and this write-up does not guess.

### Checking with a debugger

Stopping at `note` three times and printing `rsp` gives these values, which differ by 0x40 (64) each time:

```text
$1 = 0x7ffe23da3e08
$3 = 0x7ffe23da3dc8
$4 = 0x7ffe23da3d88
```

At the third stop, the table is at `rsp + 8`, the value of `rsi`, and the saved return address (`0x4011e3`, inside `chain` after the recursive call) is 0x38 (56) bytes above the table. Addresses vary from run to run; the differences do not.

The flag is `ARL{stack_2_40_64_56_16}`.

## Part 2: Assemble

The reference solution is `solution.asm`. The points that matter:

- **Values that must survive `note`.** `note` destroys `rdi` and `rsi`, so `depth` goes in `rbx` and `seed` in `rbp`. These registers are callee-saved: they are pushed on entry and popped in reverse order on exit.
- **The table.** `table[0] = seed ^ (depth * 7)`, `table[1] = seed + depth`, `table[2] = table[0] - table[1]`. It stays on the stack across both calls, and the code reads `table[0]` and `table[2]` back after the recursive call.
- **Alignment.** Two pushes plus a 24-byte subtraction leave `rsp` a multiple of 16 at both calls.
- **Footprint.** 8 + 16 + 24 = 48 bytes per level, 16 fewer than the compiled function, because there is no unused space.
- **The result.** At depth 0 the function returns `seed & 0xff`. Otherwise it returns `((below ^ table[0]) * 3) + (table[2] >> 1)` where `below` is the recursive result, and the shift is arithmetic.

## Mistakes that cost time

- **A frame of 32 bytes.** The footprint looks smaller than 64, but `rsp` is 8 modulo 16 at the calls; the checker reports `align` in every test.
- **A frame of 56 bytes.** Alignment is right, but each level uses 80 bytes; the checker reports `stack` for every depth above 0.
- **Using `rbx` without saving it.** The results are correct, yet the caller's `rbx` is destroyed; the checker reports `regs`.
- **Reading `rdi` after `note`.** `note` overwrites it with a fixed pattern; the results are wrong from the first test.
- **A logical shift for `table[2] >> 1`.** The table entry is often negative; 7 of the 18 vectors fail.
- **Storing `table[1]` into `table[2]`.** The checker reports `table`.
