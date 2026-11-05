[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# 03. Your first program

Time: about 2 hours. Prerequisites: [01. Data representation](../01-data-representation/README.md), [02. The CPU and its registers](../02-cpu-and-registers/README.md)

## Why this matters

A system call is the only way a program without libraries can do anything visible: print, read a file, end. Once you can issue system calls from assembly, you can write real programs, and you can read the same calls in any binary you analyze. Reverse engineers start many investigations by asking which system calls a program makes.

## Concepts

### Anatomy of a statement

Every line of NASM source has the same optional parts:

```text
label:    mnemonic   operands        ; comment
```

- A **label** names an address. `_start:` and `message:` are labels.
- A **mnemonic** names an instruction, such as `mov` or `syscall`.
- **Operands** are the values the instruction works on, separated by commas. The destination comes first: `mov eax, 1` puts 1 in `eax`.
- A **comment** starts with `;` and runs to the end of the line. The assembler ignores it.

Directives such as `section`, `global`, `db` and `equ` look similar but are instructions to the assembler, not to the processor. They produce no machine code by themselves.

### Sections

A program is stored in sections that have different purposes and different permissions:

| Section | Holds | Permissions | Takes space in the file |
|---|---|---|---|
| `.text` | Code | read, execute | yes |
| `.rodata` | Constants | read | yes |
| `.data` | Initialized variables | read, write | yes |
| `.bss` | Variables that start as zero | read, write | no |

`.bss` takes no space in the file because it holds only a size; the loader provides the zeroed memory. You can see these properties in the section table of the example program:

```text
$ readelf -S -W build/03_data_bss
  [ 1] .text    PROGBITS  0000000000401000 001000 000039 00  AX  0   0 16
  [ 2] .rodata  PROGBITS  0000000000402000 002000 000001 00   A  0   0  4
  [ 3] .data    PROGBITS  0000000000403004 002004 000001 00  WA  0   0  4
  [ 4] .bss     NOBITS    0000000000403008 002005 000008 00  WA  0   0  4
```

The `Flg` column shows `A` (allocated), `X` (executable) and `W` (writable). `PROGBITS` means the section has bytes in the file; `NOBITS` means it does not.

### Defining data

| Directive | Meaning |
|---|---|
| `db`, `dw`, `dd`, `dq` | Place bytes, words (2 bytes), double words (4) or quad words (8) in the file |
| `resb`, `resw`, `resd`, `resq` | Reserve space without placing bytes (used in `.bss`) |
| `name equ expression` | Define a constant; no memory is used |
| `$` | The address of the current position |

The pattern `message_len equ $ - message`, written right after a string, makes the assembler compute the string's length. Use it instead of counting by hand. Strings in assembly are plain bytes. They have no terminator unless you add one, so `10` (a newline) is just another byte you place yourself.

### The entry point

The program starts at the label `_start`. The `global _start` directive makes the label visible to the linker, which records its address as the entry point.

### Assemble and link

Two tools turn source into a program:

```bash
nasm -f elf64 prog.asm -o prog.o      # assemble: source to object file
ld prog.o -o prog                     # link: object file to executable
```

For 32-bit code, change both commands:

```bash
nasm -f elf32 prog.asm -o prog.o
ld -m elf_i386 prog.o -o prog
```

### System calls

A system call asks the operating system kernel to do something on your behalf. The convention on Linux depends on the mode:

| | 64-bit | 32-bit |
|---|---|---|
| Instruction | `syscall` | `int 0x80` |
| Call number | `rax` | `eax` |
| Arguments 1 to 6 | `rdi`, `rsi`, `rdx`, `r10`, `r8`, `r9` | `ebx`, `ecx`, `edx`, `esi`, `edi`, `ebp` |
| Result | `rax` | `eax` |

The call numbers differ between modes too. `write` is 1 in 64-bit code and 4 in 32-bit code. `exit` is 60 and 1. The full table is in the [Linux system call cheatsheet](../../cheatsheets/syscalls-linux.md).

`write(fd, buffer, length)` sends `length` bytes starting at `buffer` to the file descriptor `fd`. Descriptor 1 is standard output and 2 is standard error. It returns the number of bytes written.

### The complete program

This is [03_hello64.asm](examples/03_hello64.asm). Every line is commented because this is your first program.

```nasm
bits 64
default rel

SYS_WRITE   equ 1               ; system call numbers (64-bit table)
SYS_EXIT    equ 60
STDOUT      equ 1               ; file descriptor of standard output

section .rodata
message:    db "Hello, assembly!", 10       ; 10 is the newline byte
message_len equ $ - message                 ; bytes from 'message' to here

section .text
global _start                   ; make the entry point visible to the linker

_start:
    mov     eax, SYS_WRITE      ; rax = call number
    mov     edi, STDOUT         ; rdi = argument 1: file descriptor
    lea     rsi, [message]      ; rsi = argument 2: address of the text
    mov     edx, message_len    ; rdx = argument 3: number of bytes
    syscall                     ; enter the kernel: write(1, message, len)

    mov     eax, SYS_EXIT       ; rax = call number
    xor     edi, edi            ; rdi = argument 1: exit status 0
    syscall                     ; exit(0)
```

`lea rsi, [message]` loads the address of `message`; it does not read the text. Module 04 covers `lea` and addressing in full. `xor edi, edi` is the usual way to set a register to zero.

[03_hello32.asm](examples/03_hello32.asm) does the same in 32-bit code with `int 0x80`. [03_data_bss.asm](examples/03_data_bss.asm) shows all four sections in one program.

### The reversing view

Assembly keeps your structure but not your names. Open the program you just built:

```text
$ objdump -d -M intel build/03_hello64
0000000000401000 <_start>:
  401000:  mov  eax,0x1
  401005:  mov  edi,0x1
  40100a:  lea  rsi,[rip+0xfef]        # 402000 <message>
  401011:  mov  edx,0x11
  401016:  syscall
  401018:  mov  eax,0x3c
  40101d:  xor  edi,edi
  40101f:  syscall
```

What survived: the instructions and the labels `_start` and `message` (the symbol table keeps them). What did not: the constants `SYS_WRITE`, `STDOUT` and `message_len`, which became the numbers 1, 1 and 0x11 (17), and every comment. This is what you will see in programs without source: numbers where the author had names. Recognizing that `mov eax, 1` before `syscall` means `write` is the first reversing skill in this course.

## Try it

Build the examples and run them:

```bash
cd modules/03-first-program/examples
make run
```

Expected output:

```text
== 03_hello64
Hello, assembly!
== 03_data_bss
A
== 03_hello32
Hello, assembly!
```

Now watch the registers change in the debugger:

```bash
gdb -q build/03_hello64
```

At the `gef>` prompt, type these commands one at a time:

```text
break _start
run
stepi 4
info registers rax rdi rdx rip
x/s $rsi
```

GEF prints a large panel after every stop. Look for the register lines. After four `stepi` commands the first four instructions have run, and you see:

```text
rax            0x1                 0x1
rdi            0x1                 0x1
rdx            0x11                0x11
rip            0x401016            0x401016 <_start+22>
0x402000 <message>: "Hello, assembly!\n,"
```

The registers hold the three arguments and the call number, and `rip` points at the `syscall` instruction. The string shows a stray `,` after the newline: `x/s` prints until it finds a zero byte, and `message` has none, so it reads one byte of whatever comes next. Now run `stepi` once more. The text appears on your screen and the registers change:

- `rax` is now `0x11`: the result of `write`, the 17 bytes written.
- `rcx` holds `0x401018`, the address of the next instruction, and `r11` holds `0x302`. The `syscall` instruction overwrites both. Never keep a value in `rcx` or `r11` across a `syscall`.

Finally, see the same program from the outside:

```bash
strace ./build/03_hello64
```

```text
execve("./build/03_hello64", ["./build/03_hello64"], ...) = 0
write(1, "Hello, assembly!\n", 17Hello, assembly!
)      = 17
exit(0)                                 = ?
+++ exited with 0 +++
```

Running `strace ./build/03_hello32` shows the same calls and adds the line `[ Process PID=... runs in 32 bit mode. ]`.

## Common pitfalls

- **Forgetting `global _start`.** The linker warns `cannot find entry symbol _start; defaulting to ...` and picks the start of `.text`. The program may still run, by luck, and break when you add code before `_start`. Treat the warning as an error.
- **Mixing formats.** `nasm -f elf32` with a plain `ld` produces a link error or a wrong program. Use `-f elf32` with `ld -m elf_i386`, and `-f elf64` with plain `ld`.
- **Forgetting `exit`.** After your last instruction the processor runs whatever bytes follow, and the program ends with a segmentation fault.
- **Using `int 0x80` in 64-bit code.** It still works on Linux, but it uses the 32-bit call table and 32-bit arguments, so addresses above 4 GiB fail. In 64-bit code use `syscall`.
- **Miscounting a string.** The newline you add with `10` is one more byte. Use `$ - label`.
- **Assuming strings end with zero.** They do not. Pass the length to `write`, and place a `0` yourself when a later routine expects one.

## Summary

- A statement is `label: mnemonic operands ; comment`; directives instruct the assembler and produce no code by themselves.
- Code lives in `.text`, constants in `.rodata`, variables in `.data` and `.bss`.
- 64-bit Linux uses `syscall` with the number in `rax` and arguments in `rdi`, `rsi`, `rdx`; 32-bit uses `int 0x80` with `eax`, `ebx`, `ecx`, `edx`.
- Constants and comments disappear when you assemble; numbers remain.
- `$ - label` computes a length; `exit` must end every program.

## Next steps

- Take the [quiz](quiz.md).
- Continue with module 04, moving data and addressing modes.
- Exercises that use this module: [Exercise 01](../../exercises/01-signal-flare/README.md), [Exercise 02](../../exercises/02-two-worlds/README.md), [Exercise 05](../../exercises/05-ticket-counter/README.md).
