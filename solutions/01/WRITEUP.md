[English](WRITEUP.md) | [العربية](WRITEUP.ar.md) | [Deutsch](WRITEUP.de.md) | [Español](WRITEUP.es.md) | [Français](WRITEUP.fr.md)

# Write-up: exercise 01. Signal flare

The flag is `ARL{flare_42_23_7}`. The program writes 42 bytes to standard output, 23 bytes to standard error, and exits with status 7. This write-up shows how to get there and how to rebuild it.

## Part 1: Reverse

### Identify the file

```text
$ file bin/signal_flare
bin/signal_flare: ELF 64-bit LSB executable, x86-64, version 1 (SYSV), statically linked, not stripped
```

Statically linked means no libraries are loaded. Not stripped means the symbol names survive, which is why `_start` is visible in the disassembly.

### Watch the system calls

```text
$ strace ./bin/signal_flare
execve("./bin/signal_flare", ["./bin/signal_flare"], ...) = 0
write(1, "FLARE LAUNCHED\nCOLOR: RED\nALTITU"..., 42) = 42
write(2, "[flare] burn time 30 s\n", 23[flare] burn time 30 s
) = 23
exit(7)                                 = ?
+++ exited with 7 +++
```

`strace` prints the first 32 bytes of each buffer, so the first message is cut off, but the byte count (the last argument) and the result are complete. The second message appears inside the strace line because standard error and strace's own output share a terminal. This is normal and not a bug in the program.

The three answers are already visible: 42 bytes to descriptor 1, 23 bytes to descriptor 2, exit status 7.

### Confirm in the disassembly

`strace` shows what happened on one run. The disassembly shows what the code does, which matters when a program behaves differently on other inputs.

```text
$ objdump -d -M intel bin/signal_flare
0000000000401000 <_start>:
  401000:  b8 01 00 00 00        mov    eax,0x1
  401005:  bf 01 00 00 00        mov    edi,0x1
  40100a:  48 8d 35 ef 0f 00 00  lea    rsi,[rip+0xfef]        # 402000 <report>
  401011:  ba 2a 00 00 00        mov    edx,0x2a
  401016:  0f 05                 syscall
  401018:  b8 01 00 00 00        mov    eax,0x1
  40101d:  bf 02 00 00 00        mov    edi,0x2
  401022:  48 8d 35 01 10 00 00  lea    rsi,[rip+0x1001]       # 40202a <notice>
  401029:  ba 17 00 00 00        mov    edx,0x17
  40102e:  0f 05                 syscall
  401030:  b8 3c 00 00 00        mov    eax,0x3c
  401035:  bf 07 00 00 00        mov    edi,0x7
  40103a:  0f 05                 syscall
```

Reading it as three calls:

| Call | `eax` | `edi` | `rsi` | `edx` | Meaning |
|---|---|---|---|---|---|
| 1 | 0x1 | 0x1 | address of `report` | 0x2a = 42 | `write` 42 bytes to standard output |
| 2 | 0x1 | 0x2 | address of `notice` | 0x17 = 23 | `write` 23 bytes to standard error |
| 3 | 0x3c = 60 | 0x7 | not used | not used | `exit` with status 7 |

The flag is `ARL{flare_42_23_7}`.

## Part 2: Assemble

The reference solution is `solution.asm`. The structure follows the table above: one block per call.

```nasm-fragment
_start:
    mov     eax, SYS_WRITE      ; write(STDOUT, banner, banner_len)
    mov     edi, STDOUT
    lea     rsi, [banner]
    mov     edx, banner_len
    syscall
    ...
    mov     eax, SYS_EXIT       ; exit(7)
    mov     edi, 7
    syscall
```

Two details make the lengths right without counting by hand:

- `banner_len equ $ - banner` is placed immediately after the last `db` line of the text. `$` is the current address, so the difference is the number of bytes between the label and that point.
- The trailing `10` after each line is the newline byte, and it counts toward the length. The first text is 14 + 1, 10 + 1 and 15 + 1 bytes, which is 42.

## Mistakes that cost time

- **Counting the text by eye.** The newline bytes are easy to forget. Use `$ - label`.
- **Forgetting `exit`.** The processor keeps executing whatever bytes follow the last instruction and the program dies with a segmentation fault. The checker reports "terminated by signal 11".
- **Swapping the descriptors.** Descriptor 1 is standard output and 2 is standard error. Run `./solution 2>/dev/null` to see only standard output and confirm.
- **Reading `edx` as decimal.** `0x2a` is 42, not 2 or 20.
