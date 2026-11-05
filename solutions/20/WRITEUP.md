[English](WRITEUP.md) | [العربية](WRITEUP.ar.md) | [Deutsch](WRITEUP.de.md) | [Español](WRITEUP.es.md) | [Français](WRITEUP.fr.md)

# Write-up: exercise 20. Capstone: Tiny VM

The flag is `ARL{tinyvm_20_41_24}`: the machine has 20 valid opcodes, `call` is the opcode byte 0x29 (41 in decimal), and the sample program has 24 instructions.

## Part 1: Reverse

### Orientation

`tinyvm` is stripped and small. `strings` shows the usage message and the two diagnostics (`halt after %zu steps`, `fault %u at offset %u`), and nothing else readable. The program reads its argument as a file, checks the four bytes `TVM1` at the start, and then runs a loop. The loop is the interpreter.

### The dispatch

The loop reads one byte of the code, then does two table lookups before it jumps:

```text
  401305: movzx  eax,BYTE PTR [rcx]            ; the opcode byte
  401308: movzx  eax,BYTE PTR [rax+0x402160]   ; first table: 256 bytes
  40130f: test   al,al
  401311: je     401403                        ; 0: not an instruction
  401317: movzx  edx,al
  40131a: movzx  edx,BYTE PTR [rdx+0x402130]   ; second table: one byte per instruction
  401321: add    edx,esi                       ; position + length
  401325: cmp    rbx,rsi                       ; against the length of the code
  401328: jb     4013fc                        ; the instruction does not fit
  40132e: mov    DWORD PTR [rsp+0x20],edx      ; new program counter
  401332: cmp    al,0x14
  401334: ja     4012d9
  401336: movzx  eax,al
  401339: jmp    QWORD PTR [rax*8+0x402080]    ; third table: 21 code addresses
```

Three tables, then:

- The table at 0x402160 has 256 bytes and a non-zero entry for exactly 20 positions. Those positions are the valid opcodes, so `N` is 20. The entry is a small index from 1 to 20.
- The table at 0x402130 is indexed by that index and gives the length of the instruction: 4 for one instruction, 3 for four, 1 for two, and 2 for the other thirteen.
- The table at 0x402080 holds 21 code addresses, one per index (index 0 goes back to the top of the loop). The comparison with 0x14 is the bound check.

The program counter is stored at `[rsp+0x20]` and is updated before the handler runs: that is the convention behind the handlers of Part 2. The state is a structure on the stack: the eight registers from `[rsp]`, then the program counter (0x20), the zero flag (0x24), the carry flag (0x28), the fault code (0x2c), the halted indicator (0x30), the depth of the call stack (0x34), the sixteen return addresses (0x38) and the 256 bytes of memory (0x58).

### The handlers

Each of the twenty addresses is the code of one instruction. Reading them gives the behavior; three examples:

```text
; index 3
  401448: movzx  edx,BYTE PTR [rcx+0x1]        ; the byte after the opcode
  40144c: mov    ecx,edx
  40144e: shr    cl,0x4
  401451: and    ecx,0x7                       ; first register: high nibble, modulo 8
  401454: mov    eax,DWORD PTR [rsp+rcx*4]
  401457: and    edx,0x7                       ; second register: low nibble, modulo 8
  40145a: mov    edx,DWORD PTR [rsp+rdx*4]
  40145d: lea    esi,[rax+rdx*1]
  401460: mov    DWORD PTR [rsp+rcx*4],esi     ; r[a] = r[a] + r[b]
  401463: mov    eax,eax
  401465: mov    edx,edx
  401467: add    rax,rdx                       ; the sum again, in 64 bits
  40146a: shr    rax,0x20
  40146e: mov    DWORD PTR [rsp+0x28],eax      ; carry flag = carry out
  401472: test   esi,esi
  401474: sete   al
  401477: movzx  eax,al
  40147a: mov    DWORD PTR [rsp+0x24],eax      ; zero flag = result is 0
  40147e: jmp    4012d9
```

This is `add`: it sets both flags. The handlers for `sub`, `xor` and the others are read the same way. The shift handler is where the details are:

```text
; index 8
  401550: movzx  eax,BYTE PTR [rcx+0x1]
  401554: mov    esi,eax
  401556: and    esi,0xf                       ; the count: the whole low nibble
  401559: mov    edi,eax
  40155b: and    edi,0xf
  40155e: je     4012d9                        ; count 0: nothing changes, flags included
  401564: shr    al,0x4
  401567: and    eax,0x7                       ; the register
  40156a: mov    edx,DWORD PTR [rsp+rax*4]
  40156d: mov    ecx,0x20
  401572: sub    ecx,edi                       ; 32 - count
  401574: mov    edi,edx
  401576: shr    edi,cl                        ; value >> (32 - count)
  401578: mov    ecx,edi
  40157a: and    ecx,0x1
  40157d: mov    DWORD PTR [rsp+0x28],ecx      ; carry flag = last bit shifted out at the top
  401581: mov    ecx,esi
  401583: shl    edx,cl
  401585: mov    DWORD PTR [rsp+rax*4],edx
  401588: test   edx,edx
  40158a: sete   al
  40158d: movzx  eax,al
  401590: mov    DWORD PTR [rsp+0x24],eax      ; zero flag
  401594: jmp    4012d9
```

The count is the whole low nibble (0 to 15) while the register uses three bits of the high nibble. A count of zero returns before any flag is touched. `shr` is the mirror image, with the carry flag taken from bit `count - 1`.

```text
; index 14 (call)
  401666: mov    eax,DWORD PTR [rsp+0x34]      ; depth
  40166a: cmp    eax,0x10
  40166d: je     401693                        ; full: fault 3
  40166f: lea    esi,[rax+0x1]
  401672: mov    DWORD PTR [rsp+0x34],esi      ; depth + 1
  401676: mov    eax,eax
  401678: mov    WORD PTR [rsp+rax*2+0x38],dx  ; push the program counter (already advanced)
  40167d: movzx  eax,BYTE PTR [rcx+0x2]
  401681: shl    eax,0x8
  401684: movzx  edx,BYTE PTR [rcx+0x1]
  401688: or     eax,edx                       ; target: byte 1 + 256 * byte 2 (low byte first)
  40168a: mov    DWORD PTR [rsp+0x20],eax
  40168e: jmp    4012d9
  401693: mov    DWORD PTR [rsp+0x2c],0x3      ; fault code 3
  40169b: jmp    4016c4                        ; then halt
```

### The instruction set

Putting all twenty handlers together gives the specification (the names are from the exercise):

| Opcode | Name | Length | Effect |
|---|---|---|---|
| 0x5b | `movi` | 4 | `r[a] = number` (the 16-bit number is in bytes 2 and 3, low byte first); flags unchanged |
| 0xc2 | `mov` | 2 | `r[a] = r[b]`; flags unchanged |
| 0x17 | `add` | 2 | `r[a] += r[b]`; `cf` = carry out, `zf` = result is 0 |
| 0x8e | `sub` | 2 | `r[a] -= r[b]`; `cf` = a < b unsigned, `zf` = result is 0 |
| 0x34 | `mul` | 2 | `r[a] *= r[b]` (low 32 bits); `cf` = 0, `zf` = result is 0 |
| 0xe9 | `xor` | 2 | `r[a] ^= r[b]`; `cf` = 0, `zf` = result is 0 |
| 0x61 | `and` | 2 | `r[a] &= r[b]`; `cf` = 0, `zf` = result is 0 |
| 0xa5 | `shl` | 2 | shift left by the low nibble; count 0 changes nothing; `cf` = last bit out at the top, `zf` = result is 0 |
| 0x0d | `shr` | 2 | shift right by the low nibble; count 0 changes nothing; `cf` = last bit out at the bottom, `zf` = result is 0 |
| 0x7a | `cmp` | 2 | flags as for `sub`, nothing stored |
| 0xf3 | `jmp` | 3 | `pc` = the 16-bit target in bytes 1 and 2 |
| 0x42 | `jz` | 3 | jump if `zf` is 1 |
| 0xb8 | `jnz` | 3 | jump if `zf` is 0 |
| 0x29 | `call` | 3 | push `pc` (already advanced), then jump; fault 3 and halt if 16 addresses are on the stack |
| 0xd6 | `ret` | 1 | pop `pc`; fault 2 and halt if the stack is empty |
| 0x93 | `ldb` | 2 | `r[a] = mem[r[b] & 255]` |
| 0x6c | `stb` | 2 | `mem[r[a] & 255] = r[b] & 255` |
| 0xfc | `out` | 2 | print `r[a]` as an unsigned decimal number and a new line |
| 0x05 | `outc` | 2 | print the character `r[a] & 255` |
| 0xa0 | `halt` | 1 | stop |

In every instruction with a register byte, the first register is the high nibble and the second the low nibble, each taken modulo 8; 16-bit numbers are stored low byte first. `isa.json` in `solutions/20/` holds this table in the format of the exercise. Faults 1 (bad opcode), 5 (ran off the end or too many steps) and 6 (an instruction cut off by the end of the code) come from the interpreter loop and are not part of the handlers.

### The sample program

With the table, a disassembler is a small script (`solutions/20/tvdis.py`). The listing of `bin/sample.tvm`:

```text
      0000  5b 00 05 15   movi r0, 5381
      0004  5b 10 00 00   movi r1, 0
      0008  5b 20 06 00   movi r2, 6
      000c  5b 30 01 00   movi r3, 1
L010: 0010  93 41         ldb r4, [r1]
      0012  29 34 00      call L034
      0015  17 13         add r1, r3
      0017  7a 12         cmp r1, r2
      0019  b8 10 00      jnz L010
      001c  fc 00         out r0
      001e  5b 10 00 00   movi r1, 0
L022: 0022  93 41         ldb r4, [r1]
      0024  05 40         outc r4
      0026  17 13         add r1, r3
      0028  7a 12         cmp r1, r2
      002a  b8 22 00      jnz L022
      002d  5b 50 0a 00   movi r5, 10
      0031  05 50         outc r5
      0033  a0            halt
L034: 0034  c2 60         mov r6, r0
      0036  a5 65         shl r6, 5
      0038  17 06         add r0, r6
      003a  e9 04         xor r0, r4
      003c  d6            ret
24 instructions; data segment: b'vmcore'
```

It hashes the six data bytes with `h = h * 33 xor c` (the subroutine at 0x34 computes `h + (h << 5)`, which is `h * 33`), prints the hash and then prints the bytes as text. The listing has 24 instructions, so `I` is 24, and `call` is 0x29, which is 41. The flag follows.

## Part 2: Assemble

### The handlers

The reference handlers are in `solutions/20/solution.asm`. All of them start by taking the register byte apart. The checks that matter, and the mistakes that cost time:

- **The masks.** The register fields are nibbles, 0 to 15, taken modulo 8 (`and 7`). The shift count is the whole low nibble (`and 15`) and is not reduced.
- **Which flags.** `mul` clears the carry flag; `shl` and `shr` set the carry flag from the last bit that leaves and do nothing at all for a count of zero. A handler that clears or recomputes the flags for a zero count fails the tests that start with random flag values.
- **The carry of `shl`.** It is bit `32 - count` of the old value, which is not the same bit as for `shr` (bit `count - 1`).
- **`cmp`.** The carry flag is an unsigned comparison; a signed comparison fails when the high bit of either value is set.
- **`call` and `ret`.** The address pushed is the program counter that was already advanced. The fault cases change only the fault code and the halted indicator.
- **`ldb` and `stb`.** Only the low byte of the address register is used, and `stb` stores only the low byte of the value.
- **Preserving registers.** The handlers use only registers that a function may change, so there is nothing to save.

### The program

The reference program is `solutions/20/program.tvs`. A loop stores the current number at the address given by a counter and moves the pair forward; a subroutine counts down from 12, loads each byte and prints it. The register that is never set serves as zero for the end test of the loop. It runs in 165 steps.

```text
fill:   stb  [r2], r0
        mov  r4, r0
        add  r4, r1
        mov  r0, r1
        mov  r1, r4
        add  r2, r3
        cmp  r2, r5
        jnz  fill
```

## Mistakes that cost time

- Taking the register fields in the wrong order, or from the wrong nibble. A wrong `register_a` is caught by the check of `isa.json` and by the program, because the assembler places registers by your notes.
- Reading `sub` with the operands reversed. The first register is the destination and the left operand.
- Missing that some instructions leave the flags alone (`movi`, `mov`, `ldb`, `stb`).
- Counting the data segment or the header in the instruction count.
- Writing a handler that works for registers 0 to 7 and fails for the nibbles 8 to 15, which wrap.
