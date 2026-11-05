[English](WRITEUP.md) | [العربية](WRITEUP.ar.md) | [Deutsch](WRITEUP.de.md) | [Español](WRITEUP.es.md) | [Français](WRITEUP.fr.md)

# Write-up: exercise 02. Two worlds

The flag is `ARL{worlds_36_162_1_60}`. The two binaries make the same three calls (`write`, `sync`, `exit`), but with different instructions, numbers and registers. The port in Part 2 changes all three.

## Part 1: Reverse

### Identify the files

```text
$ file bin/two_worlds32 bin/two_worlds64
bin/two_worlds32: ELF 32-bit LSB executable, Intel 80386, version 1 (SYSV), statically linked, not stripped
bin/two_worlds64: ELF 64-bit LSB executable, x86-64, version 1 (SYSV), statically linked, not stripped
```

### The behavior is identical

```text
$ strace ./bin/two_worlds32
[ Process PID=... runs in 32 bit mode. ]
write(1, "two worlds\n", 11)            = 11
sync()                                  = 0
exit(0)                                 = ?

$ strace ./bin/two_worlds64
write(1, "two worlds\n", 11)            = 11
sync()                                  = 0
exit(0)                                 = ?
```

`strace` notes that the first process runs in 32-bit mode and shows the same three calls for both. The numbers are not shown, so read them in the disassembly.

### The code is different

```text
$ objdump -d -M intel bin/two_worlds32
08049000 <_start>:
 8049000:  mov  eax,0x4
 8049005:  mov  ebx,0x1
 804900a:  mov  ecx,0x804a000
 804900f:  mov  edx,0xb
 8049014:  int  0x80
 8049016:  mov  eax,0x24
 804901b:  int  0x80
 804901d:  mov  eax,0x1
 8049022:  xor  ebx,ebx
 8049024:  int  0x80

$ objdump -d -M intel bin/two_worlds64
0000000000401000 <_start>:
  401000:  mov  eax,0x1
  401005:  mov  edi,0x1
  40100a:  lea  rsi,[rip+0xfef]        # 402000 <text>
  401011:  mov  edx,0xb
  401016:  syscall
  401018:  mov  eax,0xa2
  40101d:  syscall
  40101f:  mov  eax,0x3c
  401024:  xor  edi,edi
  401026:  syscall
```

| | 32-bit | 64-bit |
|---|---|---|
| Instruction | `int 0x80` | `syscall` |
| `write` number | 0x4 = 4 | 0x1 = 1 |
| `sync` number | 0x24 = 36 | 0xa2 = 162 |
| `exit` number | 0x1 = 1 | 0x3c = 60 |
| Descriptor register | `ebx` | `rdi` |
| Address register | `ecx` | `rsi` |
| Length register | `edx` | `rdx` |
| Address of the text | absolute: `mov ecx, 0x804a000` | relative: `lea rsi, [rip+0xfef]` |

The last row is a difference you can easily miss. 32-bit code at a fixed address loads the address as a constant; 64-bit code computes it relative to `rip`.

The flag is `ARL{worlds_36_162_1_60}`.

## Part 2: Assemble

The reference solution is `solution.asm`. Compared with the starter, it changes:

- `bits 32` to `bits 64`, and adds `default rel` for the address of the text.
- `int 0x80` to `syscall` at each of the three places.
- The numbers: 4 to 1 for `write`, 36 to 162 for `sync`, 1 to 60 for `exit`.
- The argument registers: `ebx`, `ecx`, `edx` to `rdi`, `rsi`, `rdx`.
- `mov ecx, message` to `lea rsi, [message]`.

## Mistakes that cost time

- **Changing only the header.** A 32-bit program assembled as 64-bit still runs, because Linux accepts `int 0x80` in 64-bit processes and reads the 32-bit call table. That is why the checker looks for the instruction itself rather than only the output. In 64-bit code, addresses above 4 GiB cannot be passed that way, so a program that works by luck today can fail later.
- **Keeping the old numbers.** With `syscall`, call number 4 is `stat`, not `write`. The output goes missing or the call fails.
- **Reading the numbers as decimal.** `0xa2` is 162, not 2.
- **Dropping `sync`.** It has no visible effect, which is why it is easy to forget. The checker compares the sequence of calls.
