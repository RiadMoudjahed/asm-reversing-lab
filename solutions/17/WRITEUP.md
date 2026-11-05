[English](WRITEUP.md) | [العربية](WRITEUP.ar.md) | [Deutsch](WRITEUP.de.md) | [Español](WRITEUP.es.md) | [Français](WRITEUP.fr.md)

# Write-up: exercise 17. Archive of unknowns

The flag is `ARL{ark_16_4_7_122_48}`: a table entry is 16 bytes, data blocks are aligned to 4 bytes, the checksum is rotated by 7 bits per byte, the key for 3 files is 122, and the tool accepts at most 48 files.

## Part 1: Reverse

### Samples first

An archive of three small files gives this dump (`a.txt` holds `AAA`, `hello.txt` holds `hello world` and a newline, `empty.bin` is empty):

```text
00000000: 4152 4b32 0300 7a00 2000 0000 5000 0000  ARK2..z. ...P...
00000010: 3b3a 3d00 121e 1011 115f f7ee f0ef e08f  ;:=......_......
00000020: 0000 0000 1000 0000 0300 0000 b7cb f27b  ...............{
00000030: 0600 0000 1400 0000 0c00 0000 b504 3829  ..............8)
00000040: 1000 0000 2000 0000 0000 0000 df9b 5713  .... .........W.
00000050: 612e 7478 7400 6865 6c6c 6f2e 7478 7400  a.txt.hello.txt.
00000060: 656d 7074 792e 6269 6e00                 empty.bin.
```

The first four bytes are the magic `ARK2`. Then comes `03 00`, which is the number of files. The next byte is `7a`, which is 122, and the one after it is zero. Then come two 32-bit numbers, 0x20 and 0x50, which are offsets: at 0x20 there is a table of three groups of 16 bytes, and at 0x50 there are the names `a.txt`, `hello.txt` and `empty.bin`, each ended by a zero byte. The data blocks sit between the header and the table: the file `AAA` was stored as `3b 3a 3d`, and `hello world` as 12 bytes that do not look like text. The three `41` bytes became three different values, so the data is transformed with something that changes along the block. The values 0x3b, 0x3a and 0x3d are 0x41 xor 0x7a, 0x41 xor 0x7b and 0x41 xor 0x7c: the key 0x7a plus the position of the byte, with only the low byte kept. So byte i of a block is stored xor the low byte of key + i.

Each table entry is four 32-bit numbers. For the first entry they are 0, 0x10, 3 and 0x7bf2cbb7: the offset of the name in the name area, the offset of the data (0x10, right after the header), the size, and a value that depends on the contents. The second entry has the name offset 6 (after `a.txt` and its zero byte) and the data offset 0x14, which is the end of the first block (0x10 + 3) rounded up to a multiple of 4. The third entry has size 0 and the value 0x13579bdf: the starting value of the checksum, unchanged by an empty block.

### The program

The code confirms the samples and adds the limits. The header is built with these instructions:

```text
40134d:  mov    DWORD PTR [out],0x324b5241   ; "ARK2"
401357:  mov    eax,DWORD PTR [rsp+0x14]     ; number of files
40135b:  mov    BYTE PTR [out+4],al          ; count, low byte
401361:  mov    BYTE PTR [out+5],0x0         ; count, high byte
401368:  imul   eax,eax,0x25                 ; count * 37
40136b:  add    eax,0xb                      ; + 11
40136e:  mov    BYTE PTR [out+6],al          ; the key
401374:  mov    BYTE PTR [out+7],0x0
```

The key is `count * 37 + 11` truncated to a byte. For 3 files that is 3 * 37 + 11 = 122, so `K` is 122. The limits sit in front of the loop:

```text
4011fb:  cmp    DWORD PTR [rsp+0x14],0x30    ; more than 48 files: "too many files"
```

The value 0x30 is 48, so `N` is 48. A comparison `cmp rax,0x1f` guards the length of a name (31 characters at most), and `cmp rbx,0x10000` the size of a file (65536 bytes at most). The data offsets are rounded with `add rbp,0x3` and `and rbp,0xfffffffffffffffc`: add 3 and clear the two low bits, so the alignment `A` is 4. The entries are addressed with `shl rax,0x4`, a factor of 16, so `E` is 16. The checksum loop starts at 0x13579bdf:

```text
40147c:  mov    edx,0x13579bdf
401481:  rol    edx,0x7                      ; rotate left by 7
...
401497:  jne    401481                       ; for every byte
```

so `R` is 7. After the loop the value is combined with the size by an exclusive or. The empty entry cannot show it, because the size is 0, but the entry for `AAA` can: the plain rolling sum of the three bytes is 0x7bf2cbb4, and the stored value 0x7bf2cbb7 is that sum xor 3.

The complete format, with all numbers little endian:

| Part | Offset | Size | Content |
|---|---|---|---|
| Header | 0 | 4 | Magic `ARK2` |
| | 4 | 2 | Number of files |
| | 6 | 1 | Key: `count * 37 + 11`, low byte |
| | 7 | 1 | Zero |
| | 8 | 4 | Offset of the entry table |
| | 12 | 4 | Offset of the name area |
| Data blocks | multiples of 4 from 16 | size | Byte i is stored xor the low byte of `key + i`; padding with zero bytes |
| Entry table | table offset | 16 per file | Name offset (relative to the name area), data offset, size, checksum |
| Name area | name area offset | | Zero-terminated names, one after another |

The checksum of an entry is computed over the original bytes: start with 0x13579bdf, for each byte rotate left by 7 and add the byte, and in the end xor the size.

## Part 2: Assemble

The reference solution is `solution.asm`. The points that matter:

- **`ark_count` does the validation.** It checks the length, the magic (a single 32-bit comparison with 0x324b5241), the table (its offset at least 16 and its end, computed in 64 bits as offset plus 16 times the count, not beyond `n`) and the name area offset (not beyond `n`). The other two functions call it, so the header is checked in one place. Because it only clobbers `rax`, `rcx` and `rdx`, the callers keep their arguments in the other registers.
- **The index.** An unsigned comparison with the count rejects negative indexes and indexes that need more than 32 bits at once. A 32-bit comparison would let the index 4294967296 through as 0.
- **Names.** The name offset is added to the name area offset in 64 bits, must be inside the archive, and the zero byte is searched with a bound at `n`. Only then is the capacity compared with length plus one, and the name copied with its zero byte.
- **Extraction.** The data offset and size are zero-extended 32-bit values. The sum is computed in 64 bits, so an offset of 0xfffffff0 with a size of 0x20 is rejected and does not wrap around to a small number. The capacity is checked next, then each byte is decoded with the low byte of `key + i`, written, and added to the checksum after a rotation by 7. The checksum is finished with the size and compared with the stored value.
- **Registers.** Nothing outside the caller-saved registers is used, so there is no save or restore. The internal `call` is preceded by `sub rsp, 8` to keep the stack aligned.

## Mistakes that cost time

- **Adding the offset and the size in 32 bits.** A damaged archive with a huge offset passes the bounds check and the code reads far outside the archive.
- **A signed comparison of the index.** A negative index is accepted and the table entry is computed from a negative number.
- **Leaving the position out of the key.** Only the first byte of a block decodes correctly.
- **Forgetting the size in the checksum.** Every archive with a non-empty block is reported as damaged, but an empty block passes, which hides the error from a hasty test.
- **Rotating by 8.** The first byte decodes correctly and the checksum never matches.
- **Scanning for the zero byte without a bound.** An archive whose last name has no terminator makes the code read past the end.
- **Writing the output before checking the capacity.** The reference allows any content in `out` after an error, but never beyond `cap`.
