[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# 17. Deciphering file formats

Time: about 2.5 hours. Prerequisites: [16. Undocumented interfaces](../16-undocumented-interfaces/README.md)

## Why this matters

Programs keep their data in files whose layout nobody wrote down: a vendor's export, an old save format, a log written by a device. To read such a file, convert it, repair it or check that it is safe to open, you need the layout, and the layout is hidden in two places: the bytes of sample files and the code of the program that reads or writes them. This module teaches how to combine both, how to recognize the usual parts of a format, how to identify a checksum, and how to turn the findings into a specification that you then test with a parser of your own.

## Concepts

### The practice format

The lesson works on a small format for sensor logs. The target is the reader program `17_reader`, built stripped from `examples/17_reader.c`; the lesson uses only the compiled program and sample files made by `examples/17_make.py`.

```bash
make run
```

```text
python3 17_make.py build/sample.lg
build/17_reader build/sample.lg
channels=2 rate=8000 flags=0 samples=6 meta=1 notes=1
```

The reader prints a summary and nothing else. What the file really contains is for you to find out.

### Step 1: look at the bytes

Start with a hexadecimal dump. `xxd` shows the offset, the bytes and their printable form. `strings -a -t x` lists runs of printable characters with their file offsets.

```bash
xxd build/sample.lg
strings -a -t x build/sample.lg
```

```text
00000000: 4c47 5231 0200 0200 401f 0000 4d45 5441  LGR1....@...META
00000010: 1300 0000 756e 6974 3d6d 560a 7369 7465  ....unit=mV.site
00000020: 3d6e 6f72 7468 0a44 4154 410c 0000 0064  =north.DATA....d
00000030: 00c8 002c 0190 01f4 0158 024e 4f54 4507  ...,.....X.NOTE.
00000040: 0000 0063 6865 636b 6564 454e 4420 0400  ...checkedEND ..
00000050: 0000 0898 7769                           ....wi
      0 LGR1
      c META
     14 unit=mV
     1c site=north
     27 DATA
     3b NOTE
     43 checkedEND 
```

Read it as a set of hypotheses.

| Observation | Hypothesis |
|---|---|
| The file starts with `LGR1` | A **magic value**: a fixed identifier at offset 0 that tells a reader which format this is |
| Four-letter words `META`, `DATA`, `NOTE`, `END` appear, each followed by a small number and then content | Tagged blocks: a tag, a length, a payload. After `META` comes `13 00 00 00`, and the text that follows is 19 bytes long (0x13) |
| `0c 00 00 00` after `DATA`, then 12 bytes | The same pattern: length 12, then 12 bytes of payload, which would be 6 values of 2 bytes |
| Numbers such as `13 00 00 00` have the low byte first | **Little endian**, the byte order of x86 (module 04) |
| The last four bytes look random | A **checksum**: a value computed from the other bytes that detects damage |
| Bytes 4 to 11 are `02 00 02 00 40 1f 00 00` | A header of fields with sizes to be determined: a 1-byte value 2, a 1-byte value 0, then 2 and 8000 (0x1f40) in 2 and 4 bytes |

A **field** is a named piece of the file with a fixed position or a position given by earlier fields. Its **offset** counts bytes from the start of the file. Its **length** can be fixed, or can be stored in another field.

### Step 2: change one thing at a time

The strongest tool for locating fields is comparison. Make two samples that differ in exactly one parameter and compare the dumps. Here the program that makes samples is available; with a file you only received, compare two files you have, or change a byte yourself and see what the reader says.

```bash
python3 17_make.py /tmp/r.lg --rate 16000
diff <(xxd build/sample.lg) <(xxd /tmp/r.lg)
```

```text
1c1
< 00000000: 4c47 5231 0200 0200 401f 0000 4d45 5441  LGR1....@...META
---
> 00000000: 4c47 5231 0200 0200 803e 0000 4d45 5441  LGR1.....>..META
6c6
< 00000050: 0000 0898 7769                           ....wi
---
> 00000050: 0000 5ef2 5aa3                           ..^.Z.
```

Changing the rate from 8000 to 16000 changes bytes 8 and 9 (0x1f40 became 0x3e80), so those four bytes at offset 8 hold the rate. The last four bytes also changed: they depend on the content. The same experiment with `--channels 3` changes byte 6 and the last four bytes, which places the channel count at offset 6. With `--samples 7`, the length after `DATA` goes from 0x0c to 0x0e, a new value appears in the payload, the offsets of everything after it move by 2 and the checksum changes again. That shows that the length counts bytes (7 samples of 2 bytes is 14) and that the chunks follow each other without any index.

### Step 3: find the parser in the program

The bytes give hypotheses; the program that reads the file settles them. Look for the code that does the reading, using what is easiest to find.

- **Error messages.** `strings -a -t x build/17_reader` shows `bad magic`, `unsupported version`, `truncated chunk header`, `checksum mismatch`, `chunk too large`. A reader's messages name its checks. Find the code that refers to each string, and the condition that jumps to it is the check.
- **Imports.** The program imports `fopen`, `fread` and `memcmp`. Calls to `fread` show how many bytes are read at a time; its arguments are the size of the next field or structure.
- **Constants.** A comparison with a tag or a magic value, or a loop over a table.

In Rizin, `axt` lists the code that uses an address, and in Ghidra the same is done with cross-references (module 11). The first part of the listing below is `main` after the file is opened and the table is built. Each call to `fread` is annotated.

```text
401248:  mov    rcx,r15
40124b:  mov    edx,0xc                     ; size = 12
401250:  mov    esi,0x1                     ; one-byte items
401255:  lea    rdi,[rsp+0x34]              ; buffer
40125a:  call   fread@plt
40125f:  cmp    rax,0xc                     ; fewer than 12 bytes read: "truncated header"
401263:  jne    4012f5
401269:  mov    edx,0x4
40126e:  mov    esi,0x402047                ; a 4-byte constant in .rodata
401273:  lea    rdi,[rsp+0x34]
401278:  call   memcmp@plt                  ; compare the first 4 bytes with it
40127d:  test   eax,eax
40127f:  jne    401307                      ; different: "bad magic"
401285:  cmp    BYTE PTR [rsp+0x38],0x2     ; byte at offset 4 must be 2: "unsupported version"
40128a:  jne    401319
401290:  movzx  r14d,BYTE PTR [rsp+0x39]    ; byte at offset 5: kept for the output (flags)
```

The program reads a header of 12 bytes, as the dump suggested. The constant at address 0x402047 is in `.rodata`. In this file the offset equals the address minus the load address 0x400000, so `xxd -s 0x2047 -l 4 build/17_reader` shows `LGR1`. The byte at offset 4 is compared with 2 and the byte at offset 5 is kept. Then come the bytes at offsets 6 and 7 and at offsets 8 to 11; the code assembles them with shifts into a 16-bit and a 32-bit number, which is what little endian looks like in code:

```text
4013b4:  movzx  ecx,BYTE PTR [rsp+0x1d]     ; byte 1 of the rate
4013b9:  shl    ecx,0x8
4013bc:  movzx  eax,BYTE PTR [rsp+0x1e]     ; byte 2
4013c1:  shl    eax,0x10
4013c4:  or     ecx,eax
4013c6:  movzx  eax,BYTE PTR [rsp+0x1c]     ; byte 0
4013cb:  or     ecx,eax
...
```

The loop over the chunks follows:

```text
40143b:  mov    rcx,r15
40143e:  mov    edx,0x8                     ; read 8 bytes: a chunk header
401443:  mov    esi,0x1
401448:  lea    rdi,[rsp+0x2c]
40144d:  call   fread@plt
401452:  cmp    rax,0x8
401456:  jne    40132b                      ; "truncated chunk header"
...                                         ; bytes 4 to 7 combined into a 32-bit length in ebp
40147f:  mov    edx,0x4
401484:  mov    esi,0x402081                ; the tag "END "
401489:  lea    rdi,[rsp+0x2c]
40148e:  call   memcmp@plt                  ; first 4 bytes against "END "
401493:  mov    r13d,eax
401496:  test   eax,eax
401498:  je     40133d                      ; equal: handle the end chunk
40149e:  cmp    ebp,0x10000
4014a4:  ja     401413                      ; length above 65536: "chunk too large"
4014aa:  mov    r14d,ebp
4014ad:  mov    rcx,r15
4014b0:  mov    rdx,r14                     ; read length bytes
4014b3:  mov    esi,0x1
4014b8:  mov    edi,0x404060                ; into a static buffer
4014bd:  call   fread@plt
4014c5:  cmp    r14,rax
4014c8:  jne    401425                      ; fewer than length bytes: "truncated chunk"
```

Each chunk header is 8 bytes: a 4-byte tag followed by a 4-byte little-endian length. The payload is read by the length, with an upper limit of 0x10000 bytes. Nothing in the loop aligns or pads, which agrees with the dump. After the payload the code calls a function twice (at 0x4014db and 0x4014ea): once with the 8 header bytes and once with the payload, and the results are chained through `r12d`. It is a running value over everything read so far. That is the checksum.

After the checksum update, three more `memcmp` calls compare the tag with `DATA`, `META` and `NOTE`. Only these three tags are counted; any other tag is read and included in the checksum, but otherwise ignored. That is a fact about the format that no sample could show.

### Step 4: identify the checksum

A checksum is a function over bytes. The common ones are recognizable by their constants and structure.

| Kind | How to recognize it |
|---|---|
| Simple sum or xor of bytes | A loop with `add` or `xor` of each byte into an accumulator |
| Adler-32 | Two running sums, modulo 65521 (0xfff1) |
| CRC-32 | The constant 0xedb88320 (or 0x04c11db7), or a table of 256 values that starts with 0x00000000, 0x77073096, 0xee0e612c, 0x990951ba |

In `17_reader` the function at 0x401166 is a loop that processes one byte per round:

```text
401173:  mov    edx,eax                     ; eax holds the running value
401175:  xor    dl,BYTE PTR [rcx]           ; low byte xor the next input byte
401177:  movzx  edx,dl                      ; ... as an index from 0 to 255
40117a:  shr    eax,0x8                     ; running value shifted right by one byte
40117d:  xor    eax,DWORD PTR [rdx*4+0x414060]   ; xor the table entry for that index
401184:  add    rcx,0x1
401188:  cmp    rcx,rsi
40118b:  jne    401173
```

An earlier loop fills the table, and it contains the instruction `and eax,0xedb88320`:

```text
401231:  mov    eax,edx
401233:  and    eax,0x1
401236:  neg    eax                         ; 0 or 0xffffffff
401238:  and    eax,0xedb88320              ; the polynomial, reflected
40123d:  shr    edx,1
40123f:  xor    edx,eax
401241:  sub    ebx,0x1
401244:  jne    401231                      ; eight rounds for each table entry
```

This is the standard way to build the CRC-32 table, and the first values are the ones listed above. Three details of a checksum have to be settled because they change the result: the initial value (here `0xffffffff`, the argument `edi` of the first call), the final step (here `not r12d` before the comparison with the stored value, at 0x401377) and the range of bytes it covers. The range comes from the loop: the header (12 bytes) and every chunk before the end chunk, each chunk including its 8-byte header. The end chunk itself is not included. The stored value is read as four bytes, low byte first. Python's `zlib.crc32` implements the same CRC-32 with that initial value and final step, so the rule can be tested directly:

```bash
python3 -c "import zlib;d=open('build/sample.lg','rb').read();print(hex(zlib.crc32(d[:-12])), d[-4:].hex())"
```

```text
0x69779808 08987769
```

The computed value 0x69779808 equals the stored bytes `08 98 77 69` read little endian. When you cannot tell the range from the code, try the candidates (the whole file without the last four bytes, without the header, without the end chunk) and see which one matches the sample.

### Step 5: write the specification

The specification is the product of the work. It must be precise enough that someone else can write a reader or a writer from it, and it should state what is not known.

| Offset | Size | Field | Notes |
|---|---|---|---|
| 0 | 4 | Magic | The bytes `LGR1` |
| 4 | 1 | Version | Must be 2; other values are rejected |
| 5 | 1 | Flags | Kept and reported; the reader does not interpret it |
| 6 | 2 | Channels | Little endian; reported |
| 8 | 4 | Rate in Hz | Little endian; reported |
| 12 | | Chunks | One after another, no padding, until the end chunk |

Chunk layout: a 4-byte tag, a 4-byte little-endian length of at most 65536, then the payload. Tags: `DATA` (counted as length / 2 samples), `META` and `NOTE` (counted), any other tag (ignored, but covered by the checksum). The chunk `END` (its tag is the three letters followed by a space) has length 4 and a payload with the CRC-32 (polynomial 0xedb88320, initial value 0xffffffff, final complement) of all bytes from offset 0 to the start of the `END` chunk, stored little endian. Bytes after the end chunk are not read. Not determined: what the flags mean and how the payload of `DATA` is organized across channels, because the reader never looks at them.

State the checks in the order the reader makes them, because the order decides which error a damaged file produces: header size, magic, version, then per chunk the header size, the end chunk, the length limit, the payload size, and the checksum at the end.

### Step 6: validate with your own parser

A specification is tested by implementing it independently and comparing it with the original on files the original accepts and files it rejects. The file `examples/17_parse.py` is written only from the table above. It can dump what it decodes:

```bash
python3 17_parse.py build/sample.lg --dump
```

```text
0x0000  header: version=2 flags=0x00 channels=2 rate=8000
0x000c  META  length=19
0x0027  DATA  length=12
0x003b  NOTE  length=7
0x004a  END   length=4 crc stored=0x69779808 computed=0x69779808
channels=2 rate=8000 flags=0 samples=6 meta=1 notes=1
```

The script `examples/17_compare.py` builds damaged variants of the sample (a changed magic, version, rate or flag byte; a changed payload byte; truncation at several places; extra bytes; an impossible length) and compares the verdict of the original reader with the verdict of the parser:

```bash
python3 17_compare.py build/17_reader build/sample.lg
```

```text
same  unchanged                    channels=2 rate=8000 flags=0 samples=6 meta=1 notes=1
same  empty file                   error: truncated header
same  magic changed                error: bad magic
same  version 1                    error: unsupported version
same  version 3                    error: unsupported version
same  flags changed                error: checksum mismatch
same  rate changed                 error: checksum mismatch
same  one payload byte changed     error: checksum mismatch
same  stored checksum changed      error: checksum mismatch
same  tag changed to unknown       error: checksum mismatch
same  cut inside header            error: truncated header
same  cut inside a chunk header    error: truncated chunk header
same  cut inside a payload         error: truncated chunk
same  cut before end chunk         error: truncated chunk header
same  cut inside end chunk         error: bad end chunk
same  bytes appended               channels=2 rate=8000 flags=0 samples=6 meta=1 notes=1
same  huge length                  error: chunk too large
same  end length 5                 error: bad end chunk
18 of 18 verdicts agree
```

Every line says `same`, so the specification covers these cases. Several lines show points that a specification must record: a changed flag or rate is caught only by the checksum (the reader does not check the values), and bytes appended after the end chunk are accepted. When a line says `DIFF`, either the parser or the specification is wrong: find the first byte where the reader stops and compare it with the rule. For every doubt, run the original; it is the final reference.

## Try it

Run the lab commands from `modules/17-deciphering-file-formats/examples`.

1. Build with `make`. Dump `build/sample.lg` with `xxd` and mark the header, each chunk header and each payload on paper before you read the lesson's table.
2. Make samples with `17_make.py --flags 1`, `--samples 0` and `--samples 1`. Predict how the dump changes, then compare with `diff <(xxd a) <(xxd b)`.
3. In Rizin, run `axt` on the address of the string `checksum mismatch` (`rizin -A build/17_reader`, then `iz~checksum` for the address) and read the code that jumps there.
4. Open `build/17_reader` in Ghidra. Find the call to `memcmp` that compares four bytes with the magic value, rename the function that updates the checksum and the buffer that holds the table, and write the field table from the decompiled code.
5. Remove the last chunk of `sample.lg` with a script and fix the checksum by computing CRC-32 again. Which error does the reader give if you compute the checksum over the wrong range of bytes?
6. Extend `17_parse.py` so that it prints the contents of `META` as key and value pairs. Check it on `sample.lg`.

## Common pitfalls

- **Assuming big endian.** Read the field the way the code assembles it. A shift of the first byte by 8 or more means that byte is not the least significant.
- **Guessing from one sample.** Two samples that differ in one parameter locate a field; one sample cannot.
- **Forgetting to recompute the checksum.** After every edit of a field, compute the checksum again, or the reader rejects the file before it reaches the field you changed.
- **Missing a dependency between fields.** A length that counts payload bytes, a count that counts elements and an offset from the start of a section behave differently when you change a size.
- **Documenting only what the samples show.** The tag that the reader ignores, the limit on the length and the bytes after the end chunk appear only in the code.
- **Skipping the failure cases.** A parser that accepts everything the reader accepts, but also accepts what the reader rejects, is not equivalent. Test damaged files.

## Summary

- A format is deciphered from two sources: the bytes of samples and the code that reads or writes them. Use both.
- The usual parts are a magic value, a header of fields, lengths, counts, tags, offsets and a checksum; byte order must be settled for every number.
- Compare samples that differ in one parameter to locate fields, and read the parser to learn limits, order of checks and ignored cases.
- A checksum is identified by its constants; its initial value, final step and covered range are then determined from the code and tested with a library function.
- The specification lists fields with offsets, sizes and meanings, the rules and the order of checks, and says what is unknown. A parser written from it and compared with the original on damaged files validates it.

## Next steps

- Take the [quiz](quiz.md).
- Continue with module 18, auditing binaries for bugs.
- Exercises that use this module: [Exercise 17](../../exercises/17-archive-of-unknowns/README.md).
