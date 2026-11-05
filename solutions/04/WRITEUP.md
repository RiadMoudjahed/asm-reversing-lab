[English](WRITEUP.md) | [العربية](WRITEUP.ar.md) | [Deutsch](WRITEUP.de.md) | [Español](WRITEUP.es.md) | [Français](WRITEUP.fr.md)

# Write-up: exercise 04. Byte order

The flag is `ARL{v3_7_p8443_t45000}`. The record has a big-endian 32-bit field at offset 0, a little-endian 16-bit field at offset 4, a big-endian 16-bit field at offset 6 and a little-endian 32-bit field at offset 8. The reference solution decodes it with byte loads, shifts and `or`.

## Part 1: Reverse

### Read the code

```text
$ objdump -d -M intel --no-show-raw-insn bin/config_probe
0000000000401000 <_start>:
  mov    eax,DWORD PTR [rip+0xffa]        # 402000 <record>
  bswap  eax
  movzx  ecx,WORD PTR [rip+0xff5]         # 402004 <record+0x4>
  movzx  edx,WORD PTR [rip+0xff0]         # 402006 <record+0x6>
  rol    dx,0x8
  mov    ebx,DWORD PTR [rip+0xfe8]        # 402008 <record+0x8>
  add    eax,ecx
  add    eax,edx
  add    eax,ebx
  cmp    eax,0x450cb
```

Four loads from the record, two of them followed by a swap:

| Field | Instruction | Offset | Width | Byte order |
|---|---|---|---|---|
| A | `mov eax, [record]` then `bswap eax` | 0 | 4 bytes | big-endian |
| B | `movzx ecx, word [record+4]` | 4 | 2 bytes | little-endian |
| C | `movzx edx, word [record+6]` then `rol dx, 8` | 6 | 2 bytes | big-endian |
| D | `mov ebx, [record+8]` | 8 | 4 bytes | little-endian |

`bswap` and `rol dx, 8` both reverse the bytes of what they act on, which is how a program turns big-endian data into the native little-endian value. A load with no swap keeps the memory order, which is little-endian on x86.

### Decode the bytes

```text
$ objdump -s -j .rodata bin/config_probe
 402000 00030007 fb208001 c8af0000 ...
```

The record is `00 03 00 07 | fb 20 | 80 01 | c8 af 00 00`.

| Field | Bytes | Reversed if needed | Value |
|---|---|---|---|
| A | `00 03 00 07` | already in reading order (big-endian) | `0x00030007`: high half 3, low half 7 |
| B | `fb 20` | `20 fb` | `0x20fb` = 8443 |
| C | `80 01` | already in reading order (big-endian) | `0x8001` |
| D | `c8 af 00 00` | `00 00 af c8` | `0xafc8` = 45000 |

### Check against the checksum

The program adds the four decoded values and compares the sum with `0x450cb`:

```text
0x00030007 + 0x20fb + 0x8001 + 0xafc8 = 0x450cb
196615 + 8443 + 32769 + 45000 = 282827
```

If you had read B as big-endian (`0xfb20` = 64288) the sum would not match. The constant in the program is a built-in answer key for your decoding.

The flag is `ARL{v3_7_p8443_t45000}`.

## Part 2: Assemble

The reference solution is `solution.asm`.

`load_le16` is one instruction, because the processor's native load is little-endian:

```nasm-fragment
load_le16:
    movzx   eax, word [rdi]
    ret
```

`load_be32` builds the value one byte at a time, most significant byte first. Each step moves what is already there up by 8 bits and adds the next byte:

```nasm-fragment
    movzx   eax, byte [rdi]             ; first byte
    shl     eax, 8
    movzx   edx, byte [rdi + 1]
    or      eax, edx                    ; now two bytes
    ; ... repeat for bytes 2 and 3
```

`decode_record` does the same for `version` and `flags`, and copies `port` and `timeout_ms` with plain loads and stores of matching width.

## Mistakes that cost time

- **Loading eight bytes in `load_be32`.** `mov rax, [rdi]` is a valid way to start, but it reads four bytes past the input. The tests place the input right before an inaccessible page, so the program crashes with signal 11.
- **Writing too wide into `out`.** `mov [rsi + 4], eax` stores four bytes into a two-byte field and overwrites `flags`. The tests fill `out` with a known pattern first, so the damage shows.
- **Not clearing the register.** Loading a byte with `mov al, [rdi]` leaves the old upper bits of `eax` in place. Use `movzx` so the upper bits are zero.
- **Using `bswap` or `movbe`.** They produce the right values, and the checker rejects them by rule.
- **Reading B or D as big-endian.** A load without a swap in the original program is little-endian. The checksum catches this immediately.
