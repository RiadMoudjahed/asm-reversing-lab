[English](WRITEUP.md) | [العربية](WRITEUP.ar.md) | [Deutsch](WRITEUP.de.md) | [Español](WRITEUP.es.md) | [Français](WRITEUP.fr.md)

# Write-up: exercise 18. Bug hunt report

The flag is `ARL{bughunt_16_128_8_64_5}`: the name handler accepts a length of up to 16 into a 16-byte buffer, the numbers handler lets every length byte from 128 on through its signed check, the tags handler accepts a tag value of up to 8 for a table of 8 entries, the data block size wraps at 8 bits from a count of 64, and the record type 5 releases memory that is used again.

## Part 1: Reverse

### Orientation

The binary is stripped, and the parsing code was inlined into the main function. The first step is to find the structure of the program rather than to read it from the top:

- The file is read with one `fread` of up to 4096 bytes into a static buffer at 0x404080. After the call, the returned length is never used again: nothing compares a position with the size of the data. This is the first hint about the record walk.
- The records are dispatched through a jump table:

```text
401465:  mov    rbx,rdi
401468:  cmp    al,0x5                       ; type above 5: unknown, error -6
40146a:  ja     40153f
401470:  movzx  eax,al
401473:  jmp    QWORD PTR [rax*8+0x402088]   ; table for the types 1 to 5
```

The table at 0x402088 holds, for the types 1 to 5, the addresses 0x401316, 0x401362, 0x4012e4, 0x4013bc and 0x401438. Each address is the code of one record type, and each one is a sink to audit. Playing with small input files confirms what they do: type 1 sets the name, type 2 adds numbers, type 3 counts tags, type 4 holds a data block, type 5 discards it.

### The record walk (all types)

```text
401440:  add    rbp,0x1                      ; one more record
401444:  movzx  eax,BYTE PTR [rbx]           ; type
401447:  movzx  edx,BYTE PTR [rbx+0x1]       ; length
40144b:  movzx  r8d,dl
40144f:  lea    rsi,[rbx+0x2]                ; payload
401453:  movzx  ecx,dl
401456:  lea    rdi,[rsi+rcx*1]              ; the next record = payload + length
40145a:  mov    r14,rbx
40145d:  test   al,al
40145f:  je     4012b6                       ; type 0 ends the loop
401465:  mov    rbx,rdi
```

The loop reads two header bytes and computes the next position, and at no point is the position compared with the end of the data. A file whose last record declares a length that does not fit makes the code read past the end of the buffer, and a file with no record of type 0 makes it read until it meets one, wherever that is. The class is an out-of-bounds read (CWE-125).

### Names (type 1, 0x401316)

```text
401316:  pxor   xmm0,xmm0
40131a:  movaps XMMWORD PTR [rsp+0x80],xmm0  ; the 16-byte local name, cleared
401322:  cmp    r8d,0x10
401326:  ja     401356                       ; length above 16: error -2
401328:  lea    rax,[rsp+0x80]
...
401335:  rep movs BYTE PTR es:[rdi],BYTE PTR ds:[rsi]    ; copy length bytes
40133a:  mov    BYTE PTR [rsp+r8*1+0x80],0x0            ; terminator at name[length]
```

The buffer has 16 bytes, the limit is 16, and the terminator is written at index `length`. At length 16 the terminator is the seventeenth byte, one past the end: an off-by-one error that writes one zero byte outside the buffer (CWE-193, an out-of-bounds write). So `N` is 16, and the correct limit is 15.

### Numbers (type 2, 0x401362)

```text
401362:  cmp    dl,0x20
401365:  jg     4013b0                       ; signed: above 32 is error -3
401367:  lea    rax,[rsp+0x20]               ; a local array of 8 integers (32 bytes)
40136c:  movsx  rcx,dl                       ; the size, sign-extended to 64 bits
401370:  mov    rdi,rax
401373:  rep movs BYTE PTR es:[rdi],BYTE PTR ds:[rsi]
```

The comparison uses the 8-bit register `dl` and a signed jump. The length bytes 0 to 32 pass, and so do all the bytes from 128 to 255, which are negative when read as signed 8-bit numbers. The following `movsx` extends a negative value to a gigantic 64-bit count for `rep movs`. So `V` is 128. The check is also missing the test that the length is a multiple of 4. This is a signed-to-unsigned conversion error (CWE-195) that leads to an out-of-bounds write. The fix compares the unsigned length and checks the multiple of 4.

### Tags (type 3, 0x4012e4)

```text
4012f6:  movzx  eax,BYTE PTR [rdx]           ; the tag byte
4012f9:  cmp    al,0x8
4012fb:  ja     40154b                       ; above 8: error -4
401301:  movzx  eax,al
401304:  add    DWORD PTR [rsp+rax*4],0x1    ; histogram[tag]++
```

The histogram is eight 32-bit counters at the bottom of the frame (cleared by an earlier `rep stos` of 8 dwords). The check lets the value 8 through, and the increment at index 8 touches the four bytes after the table: an off-by-one error with a write (CWE-193). So `T` is 8; the correct limit is 7. In this build the neighbor is the array of the numbers handler, so the damage is silent, which is what makes this kind of bug dangerous.

### Data blocks (type 4, 0x4013bc)

```text
4013bc:  movzx  r13d,BYTE PTR [r14+0x2]      ; count = first payload byte
4013c1:  mov    rdi,r12
4013c4:  call   free                         ; the previous block
4013c9:  lea    edi,[r13*4+0x0]
4013d1:  movzx  edi,dil                      ; the size is truncated to 8 bits
4013d5:  call   malloc
4013da:  mov    r12,rax
4013dd:  movzx  eax,r13b
4013e1:  shl    eax,0x2                      ; the copy size is count * 4 in 32 bits
...                                          ; a copy of that many bytes into the block
```

The allocation size is `count * 4` truncated to 8 bits, and the copy size is `count * 4` in 32 bits. They agree while the count is below 64. From a count of 64 on, the product reaches 256 and the allocation size wraps (64 gives `malloc(0)`, 65 gives `malloc(4)`), while the copy still writes 4 times the count: a heap overflow (CWE-190 leading to CWE-122). So `B` is 64. The code also never compares the length of the record with the count, so the copy may read past the payload (CWE-125). The fix computes the size in a wide type and requires that the length equals 1 plus 4 times the count.

### Release (type 5, 0x401438)

```text
401438:  mov    rdi,r12
40143b:  call   free                         ; free the block; r12 keeps its value
401440:  ...                                 ; the loop continues
...
40147a:  movzx  ecx,BYTE PTR [r12+rax*1]     ; at the end: sum the block bytes (r13 of them)
...
401494:  mov    rdi,r12
401497:  call   free                         ; and free it again
```

The record of type 5 frees the block and leaves `r12` and the length `r13` unchanged. The code after the loop sums the bytes of the block and frees it a second time: a read after the free (CWE-416) and a double free (CWE-415). So `U` is 5. (The record of type 4 also frees the old block before it allocates a new one, which is correct only because it overwrites `r12`.) The fix sets the pointer to null and the length to 0 after the free.

### A model report

```text
Title:      The record walk never compares its position with the end of the data
Class:      CWE-125 out-of-bounds read
Location:   main (the parser is inlined), loop at 0x401440 to 0x401465
Severity:   Medium. The length bytes come from the file; only reads are affected.
Cause:      The next record is computed as payload + length and the position is
            never compared with the size of the data returned by fread.
Evidence:   A file with a last record of length 255 and 10 bytes of data makes
            the loop read the stale content of the static buffer.
Impact:     Bytes outside the file are interpreted as records; the output depends
            on memory that is not part of the input and the program can fault.
Fix:        Track the bytes that are left, and refuse a record when fewer than 2
            bytes remain or the payload does not fit.
Test:       Truncations at every position of a valid file; a last record of length
            255 with no data.

Title:      Names: off-by-one in the terminator
Class:      CWE-193 off-by-one (out-of-bounds write of one byte)
Location:   0x401322 (limit) and 0x40133a (terminator)
Severity:   Medium. One byte with a fixed value, written next to a stack buffer.
Cause:      The limit is 16 for a buffer of 16 bytes, and the terminator is written
            at index length.
Evidence:   A name of 16 bytes passes the check; the store at [rsp+r8+0x80] with
            r8 = 16 is outside the 16 bytes at [rsp+0x80].
Impact:     A zero byte is written to whatever follows the buffer.
Fix:        Accept lengths up to 15.
Test:       Names of 0, 15, 16 and 255 bytes.

Title:      Numbers: signed length check
Class:      CWE-195 signed to unsigned conversion (leading to an out-of-bounds write)
Location:   0x401362 to 0x401373
Severity:   High. A length byte of 128 to 255 gives a copy size of about 2^64.
Cause:      The length is tested as a signed 8-bit value and then sign-extended to
            the copy size.
Evidence:   A record with length 128 crashes the program; a debugger stopped at
            0x401370 shows rcx = 0xffffffffffffff80.
Impact:     The copy runs far beyond its 32-byte destination and ends with a fault.
Fix:        Treat the length as unsigned; require a multiple of 4 and at most 32.
Test:       Lengths 0, 4, 32, 33, 36, 127, 128, 255.

Title:      Tags: the table has 8 entries and the check accepts 8
Class:      CWE-193 off-by-one (out-of-bounds write of a counter)
Location:   0x4012f9 and 0x401304
Severity:   Medium. A counter increment on the four bytes after the table.
Cause:      The limit is "above 8" for a table whose last index is 7.
Evidence:   A tag value of 8 passes the check and the increment is at [rsp+0x20].
Impact:     The data that follows the table is changed.
Fix:        Reject tag values above 7.
Test:       Tag values 0, 7, 8, 9, 255.

Title:      Data blocks: the allocation size wraps
Class:      CWE-190 integer overflow (leading to CWE-122 heap overflow)
Location:   0x4013c9 to 0x4013e1
Severity:   High. The count and the data come from the file.
Cause:      malloc receives count * 4 truncated to 8 bits; the copy uses count * 4
            in 32 bits; the record length is not compared with the count.
Evidence:   A count of 64 gives malloc(0) and a copy of 256 bytes.
Impact:     A heap overflow of up to 768 bytes beyond a small allocation.
Fix:        Compute the size in 64 bits, require length = 1 + 4 * count, and
            allocate that size.
Test:       Counts 0, 1, 63, 64, 65, 255, each with a matching and a mismatching
            length.

Title:      Release: the freed block is used and freed again
Class:      CWE-416 use after free (and CWE-415 double free)
Location:   0x401438 and 0x40147a to 0x401497
Severity:   High. The block is read after it was freed, then freed again.
Cause:      Type 5 frees the block and keeps the pointer and the length.
Evidence:   A file with a data block followed by a record of type 5 is enough;
            in the debugger, the second call to free at 0x401497 receives the
            pointer that the first call at 0x40143b has already released.
Impact:     Reads of freed memory, and corruption of the allocator state.
Fix:        Set the pointer to null and the length to 0 after the free.
Test:       A block then a reset; a reset with no block; a reset twice; a block
            after a reset.
```

## Part 2: Assemble

The reference solution is `solution.asm`. The points that matter:

- **The loop.** The bytes that are left are computed as `n - pos` before every comparison. Position plus length could be compared with `n` too, but the subtraction cannot overflow because `pos` never exceeds `n`. The two header bytes are checked first, then the payload, and only then is the type examined, so the record of type 0 follows the same rule.
- **Names.** One 16-byte store clears the field, then the payload bytes are copied. The limit is 15.
- **Numbers.** `test r11b, 3` rejects lengths that are not a multiple of 4, and an unsigned comparison rejects those above 32. The values are loaded with `movsxd` so that a negative number is added correctly to the 64-bit sum.
- **Tags.** The byte is compared with 7 before it indexes the table.
- **Data blocks.** `lea r10d, [r10*4 + 1]` computes the expected length in a 32-bit register; a count that fits in a byte cannot overflow it. The comparison with the length decides everything, so no read ever goes beyond the payload. The sum starts at 0 for every block.
- **Registers.** Only caller-saved registers are used, so there is nothing to save, and there are no calls.

## Mistakes that cost time

- **Comparing position plus length with `n` in 32 bits.** For a buffer near the top of the address space, or with a large length, the sum wraps. Compute what is left instead.
- **Testing the record type before the truncation.** A record of type 0 with a payload that is cut off must still be rejected.
- **Using an unsigned load for the numbers.** Negative values add the wrong amount; the sum differs.
- **Forgetting to clear the name.** A shorter name that replaces a longer one leaves old bytes behind.
- **Not clearing the block sum.** A second block adds to the first.
- **Reading the count before checking that the length is at least 1.** A record of type 4 with no payload reads one byte beyond it.
- **Checking the tag against 8.** The off-by-one of the original is copied into the fix.
