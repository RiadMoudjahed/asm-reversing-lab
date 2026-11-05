[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# 11. The reverser's toolkit

Time: about 3 hours. Prerequisites: [10. Operating system fundamentals for reversers](../10-os-for-reversers/README.md)

## Why this matters

The workflow from module 09 only works if you can carry out each step quickly. This module introduces the tools, in the order you use them: identify the file, read its structure, disassemble it, explore it interactively, run it under a debugger, and watch it from outside. It uses one small program, built from `examples/11_target.c`, so you can compare what each tool shows about the same code.

## Concepts

### The practice program

`11_target` takes one word as its argument, prints a number computed from it, and prints `access granted` for one particular word and `denied` for any other. Build and run it:

```bash
make
build/11_target hello
build/11_target open-sesame
```

```text
score 99162322
denied
score -1484336485
access granted
```

The commands in this module run from `modules/11-toolkit/examples`. The program is deliberately simple: it contains a loop (`score`), a comparison (`check`) and a library call (`strcmp`).

### Step 1: identify the file

`file` reads the headers and says what the file is:

```bash
file build/11_target
```

```text
build/11_target: ELF 64-bit LSB executable, x86-64, version 1 (SYSV), dynamically linked, interpreter /lib64/ld-linux-x86-64.so.2, BuildID[sha1]=3690f8d5a33ef0140c7ee95bff861bc16fb6e86b, for GNU/Linux 3.2.0, not stripped
```

`strings` prints runs of printable characters. `-t x` adds the file offset in hexadecimal, and `-n` sets the minimum length:

```bash
strings -t x -n 6 build/11_target | grep sesame
```

```text
   2038 open-sesame
```

`xxd` shows raw bytes. Here is the beginning of the ELF header:

```bash
xxd -l 32 build/11_target
```

```text
00000000: 7f45 4c46 0201 0100 0000 0000 0000 0000  .ELF............
00000010: 0200 3e00 0100 0000 7010 4000 0000 0000  ..>.....p.@.....
```

The first four bytes `7f 45 4c 46` are the magic number (`.ELF`). Byte 4 is `02` for 64-bit, byte 5 is `01` for little-endian. At offset 0x10 the type is `02 00` (executable) and the machine is `3e 00` (x86-64). At offset 0x18 the 8-byte entry point is `70 10 40 00 00 00 00 00`, which is the address 0x401070. Module 12 explains every field.

### Step 2: read the structure

`readelf` prints the file structure without interpreting the code:

```bash
readelf -h build/11_target | grep -E "Class|Type|Machine|Entry"
readelf -d build/11_target | grep NEEDED
readelf -S -W build/11_target | grep -E " \.(text|rodata|data|bss|got)"
readelf -s build/11_target | grep -E "FUNC.*(score|check|main)"
```

```text
  Class:                             ELF64
  Type:                              EXEC (Executable file)
  Machine:                           Advanced Micro Devices X86-64
  Entry point address:               0x401070
 0x0000000000000001 (NEEDED)             Shared library: [libc.so.6]
  [14] .text             PROGBITS        0000000000401070 001070 0001b5 00  AX  0   0 16
  [16] .rodata           PROGBITS        0000000000402000 002000 000044 00   A  0   0  8
  [22] .got              PROGBITS        0000000000403fd8 002fd8 000010 08  WA  0   0  8
  [23] .got.plt          PROGBITS        0000000000403fe8 002fe8 000038 08  WA  0   0  8
  [24] .data             PROGBITS        0000000000404020 003020 000010 00  WA  0   0  8
  [25] .bss              NOBITS          0000000000404040 003030 000010 00  WA  0   0 32
    28: 0000000000401156    46 FUNC    GLOBAL DEFAULT   14 score
    34: 00000000004011a1   132 FUNC    GLOBAL DEFAULT   14 main
    36: 0000000000401184    29 FUNC    GLOBAL DEFAULT   14 check
```

Sections tell you where code (`.text`, flags `AX`: allocated and executable), constants (`.rodata`, `A`) and data (`.data`, `.bss`, flags `WA`: writable) live. The symbol table gives each function's address and size. A stripped binary has no function names; you rediscover them yourself.

### Step 3: disassemble

`objdump -d` disassembles the executable sections. It prints AT&T syntax by default and Intel syntax with `-M intel`. This course uses Intel syntax, but compilers' output and many tools default to AT&T, so you must read both.

```bash
objdump -d -M intel --no-show-raw-insn build/11_target | awk '/<check>:/{p=1} p{print} /^$/{if(p)exit}'
objdump -d --no-show-raw-insn build/11_target | awk '/<check>:/{p=1} p{print} /^$/{if(p)exit}'
```

```text
0000000000401184 <check>:
  401184:    sub    rsp,0x8
  401188:    lea    rsi,[rip+0xea9]        # 402038 <secret>
  40118f:    call   401040 <strcmp@plt>
  401194:    test   eax,eax
  401196:    sete   al
  401199:    movzx  eax,al
  40119c:    add    rsp,0x8
  4011a0:    ret
```

```text
0000000000401184 <check>:
  401184:    sub    $0x8,%rsp
  401188:    lea    0xea9(%rip),%rsi        # 402038 <secret>
  40118f:    call   401040 <strcmp@plt>
  401194:    test   %eax,%eax
  401196:    sete   %al
  401199:    movzbl %al,%eax
  40119c:    add    $0x8,%rsp
  4011a0:    ret
```

Reading the function: it loads the address of `secret` into `rsi` (the second argument), calls `strcmp` (the first argument is already in `rdi`), then turns "result equals zero" into a 0 or 1 in `eax`. The function returns whether the two strings are equal.

#### Reading AT&T syntax

| | Intel | AT&T |
|---|---|---|
| Operand order | destination, source | source, destination |
| Registers | `eax` | `%eax` |
| Constants | `0x8` | `$0x8` |
| Memory | `[rsi + rcx*4 + 8]` | `8(%rsi,%rcx,4)` |
| Operand size | In the operand: `dword ptr [rax]` | In the mnemonic suffix: `movl`, `movq`, `movzbl` |
| Example | `movzx eax, al` | `movzbl %al, %eax` |

Convert by reversing the operands and removing the decorations. The suffix letters are `b` (byte), `w` (word), `l` (doubleword), `q` (quadword). In `movzbl` the two letters mean "from byte to long".

### Step 4: explore interactively with Rizin

Rizin is a command-line disassembler and analysis framework. `-A` analyzes the file on load, `-q` quits after the commands, and `-c` runs them.

```bash
rizin -q -e scr.color=0 -A -c "pdf @ sym.check" build/11_target
```

```text
            ; CALL XREF from main @ 0x4011cf
┌ sym.check(const char *s1);
│           ; arg const char *s1 @ rdi
│           0x00401184      sub   rsp, 0x08
│           0x00401188      lea   rsi, qword obj.secret                ; 0x402038 ; "open-sesame" ; const char *s2
│           0x0040118f      call  sym.imp.strcmp                       ; int strcmp(const char *s1, const char *s2)
│           0x00401194      test  eax, eax
│           0x00401196      setz  al
│           0x00401199      movzx eax, al
│           0x0040119c      add   rsp, 0x08
└           0x004011a0      ret
```

Rizin did more than `objdump`: it named the argument, resolved the data reference to the string, and showed the prototype of `strcmp`. Commands you will use most:

| Command | Meaning |
|---|---|
| `aaa` | Analyze everything (the `-A` option does this on load) |
| `afl` | List the functions found |
| `pdf @ sym.check` | Print the disassembly of a function |
| `axt @ sym.check` | List the cross-references to `sym.check` (who calls it) |
| `iz` | List the strings in data sections |
| `s 0x401184` | Seek to an address |
| `?` | Help for any command; add `?` to a prefix such as `a?` |

```bash
rizin -q -e scr.color=0 -A -c "axt @ sym.check; iz~sesame" build/11_target
```

```text
main 0x4011cf [CALL] call sym.check
0x00002038 0x00402038  11   12 .rodata ascii open-sesame
```

The first line says that `main` calls `check` at 0x4011cf. The second says that the string sits at file offset 0x2038 and virtual address 0x402038, matching `strings -t x`.

### Step 5: Ghidra

Ghidra is a graphical suite with a disassembler and a decompiler. You installed it on your host in module 00. This is the workflow; it is the same for every binary in the course.

1. Create a project, import the binary from `exercises/NN-name/bin/` and accept the detected format.
2. Open the file in the code browser and let the default analysis run.
3. Look at the Symbol Tree for functions and the Defined Strings window for strings. Double-click a string, then look at its cross-references to find the code that uses it.
4. Read the Listing (disassembly) and the Decompiler side by side. The decompiler is approximate, so compare it with the instructions when something looks wrong.
5. Improve the program as you learn. Rename functions and variables (default key `L`), change types (default key `Ctrl+L` on a variable), and add comments (default key `;`). Show cross-references to an item with the default key `X`.

The point of renaming and retyping is that the decompiler redraws its output with your names and types, so each step makes the next function easier to read. Keyboard shortcuts can differ between Ghidra versions and settings; the Edit and Navigation menus list the current ones.

### Step 6: run under a debugger

GDB runs the program under your control. The lab image loads GEF, a plugin that prints registers, the stack and the code around `rip` at every stop. The batch form below disables plugins (`-nx`) so the output is plain text; in the lab you will normally use the interactive form.

```bash
gdb -nx -q -batch -ex "break check" -ex "run hello" -ex "info registers rdi" \
    -ex "x/s \$rdi" -ex "x/s 0x402038" -ex "finish" -ex "print \$rax" build/11_target
```

```text
Breakpoint 1, 0x0000000000401184 in check ()
rdi            0x7fffd18f3e21      140736709213729
0x7fffd18f3e21:    "hello"
0x402038 <secret>:    "open-sesame"
0x00000000004011d4 in main ()
$1 = 0
```

At the breakpoint, `rdi` points at the argument string, which is "hello". The second `x/s` prints the string `check` compares it with. After `finish`, `check` has returned to `main` and `rax` is 0, the "denied" case. The address in `rdi` is on the stack and varies from run to run.

| Command | Meaning |
|---|---|
| `break check` or `b *0x401184` | Stop at a function or at an exact address |
| `run ARGS` | Start the program with arguments |
| `continue` | Resume |
| `stepi`, `nexti` | Run one instruction, stepping into or over calls |
| `finish` | Run until the current function returns |
| `info registers` | Show registers; add names to show only those |
| `x/NFU ADDR` | Examine memory: `N` units of format `F` and size `U`, such as `x/4gx $rsp`, `x/s $rdi`, `x/5i $pc` |
| `watch EXPR` | Stop when a value changes |
| `set disassembly-flavor intel` | Show Intel syntax (the lab image sets this) |

A second session, stepping through the loop in `score`:

```bash
gdb -nx -q -batch -ex "break score" -ex "run ab" -ex "x/i \$pc" -ex "stepi 3" \
    -ex "x/i \$pc" -ex "info registers eax ecx" build/11_target
```

```text
Breakpoint 1, 0x0000000000401156 in score ()
=> 0x401156 <score>:    movzbl (%rdi),%eax
0x000000000040115d in score ()
=> 0x40115d <score+7>:    mov    $0x0,%ecx
eax            0x61                97
ecx            0x403e00            4210176
```

The first character of "ab" is loaded: `eax` is 0x61, which is "a". The `ecx` value is whatever was in the register before the code initialized it, which happens at 0x40115d. GDB prints AT&T syntax here because plugins were disabled; the lab image's configuration switches it to Intel.

### Step 7: watch from outside

`strace` and `ltrace`, introduced in module 10, are the quickest observation tools:

```bash
strace -e trace=write build/11_target hello
ltrace build/11_target hello
```

```text
write(1, "score 99162322\ndenied\n", 22score 99162322
denied
) = 22
+++ exited with 1 +++
```

```text
__printf_chk(2, 0x402014, 0x5e918d2, 0x5e918d2)  = 15
strcmp("hello", "open-sesame")                   = -7
puts("denied")                                   = 7
score 99162322
denied
+++ exited (status 1) +++
```

`ltrace` reveals the argument of `strcmp` directly, which is often the fastest way to learn what a program compares its input with. The first call is `printf` compiled to `__printf_chk`; the second argument is a pointer to the format string, and the other two are the number printed (0x5e918d2 is 99162322).

### Choosing a tool

| Question | Start with |
|---|---|
| What kind of file is this? | `file`, `xxd` |
| What text does it contain? | `strings -t x` |
| Which sections, symbols and libraries? | `readelf` |
| What do the instructions do? | `objdump`, Rizin, Ghidra |
| Who calls this function, or uses this string? | Rizin `axt`, Ghidra cross-references |
| What are the real values at run time? | GDB |
| What does it do from outside? | `strace`, `ltrace` |

## Try it

1. Without running the program with the right word, find the word it expects using only static tools. Use `strings`, then confirm with `rizin` by finding what `check` compares with.
2. Using GDB, stop at `score` with the argument `abc` and step through the loop until `ecx` holds the final score. Compare it with the number the program prints.
3. Convert the AT&T listing of `check` back to Intel syntax on paper, then compare it with the Intel listing.
4. With `ltrace`, find out which function `main` calls to print the score.

## Common pitfalls

- **Reading AT&T as Intel.** The operand order is reversed. Check the syntax before reading.
- **Trusting `strings` as the full picture.** Strings built or decoded at run time are not in the file.
- **Believing the decompiler.** It is a reconstruction. Compare with the instructions.
- **Forgetting that analysis is a guess.** Automatic analysis can miss functions or mislabel data as code. Check function boundaries when a result looks odd.
- **Stopping at the wrong place.** A breakpoint on a function name stops at its first instruction; arguments are already in registers, but locals are not yet initialized.

## Summary

- Identify a file with `file`, `strings` and `xxd`; read its structure with `readelf`.
- Disassemble with `objdump`, Rizin or Ghidra; know both Intel and AT&T syntax.
- Rizin gives interactive analysis from the command line; Ghidra adds a decompiler and lets you rename, retype and comment.
- GDB shows real values: set breakpoints, step, and examine registers and memory.
- `strace` and `ltrace` show a program's system and library calls without reading code.

## Next steps

- Take the [quiz](quiz.md).
- Continue with module 12, executable formats.
- Cheatsheets: [GDB](../../cheatsheets/gdb.md), [Rizin](../../cheatsheets/rizin.md), [Ghidra](../../cheatsheets/ghidra.md).
