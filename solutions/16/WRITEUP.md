[English](WRITEUP.md) | [العربية](WRITEUP.ar.md) | [Deutsch](WRITEUP.de.md) | [Español](WRITEUP.es.md) | [Français](WRITEUP.fr.md)

# Write-up: exercise 16. Silent library

The flag is `ARL{silent_16_8_5_13_7}`: the state block is 16 bytes, the 64-bit byte count is at offset 8, the feeding function rotates by 5 bits, the function that reads the digest rotates by 13 bits, and the packing function writes 7 bytes for the four integers.

## Part 1: Reverse

### The exports

```text
$ nm -D --defined-only bin/libsilent.so
0000000000001141 T sl_feed
0000000000001173 T sl_final
000000000000117f T sl_free
000000000000119a T sl_live
0000000000001119 T sl_new
00000000000011a1 T sl_pack
```

The names suggest a set (`sl_new`, `sl_feed`, `sl_final`, `sl_free`) and two functions outside it. The set is checked by its code, not trusted by its names.

### The state: `sl_new`

```text
1119:  push   rbx
111a:  mov    ebx,edi                       ; seed, 32 bits
111c:  mov    esi,0x10
111f:  mov    edi,0x1                       ; calloc(1, 16)
1126:  call   calloc@plt
112b:  test   rax,rax
112e:  je     113f                          ; null stays null
1130:  xor    ebx,0x811c9dc5
1136:  mov    DWORD PTR [rax],ebx           ; first field = seed xor constant
1138:  add    DWORD PTR [rip+0x2edd],0x1    ; a global counter goes up
113f:  pop    rbx
1140:  ret
```

The function takes one 32-bit value, allocates one zeroed block of 16 bytes, stores the seed xor 0x811c9dc5 in the first four bytes and increments a global. So `S` is 16. The global is read by `sl_live` (`mov eax, DWORD PTR [rip+...]`, `ret`) and decremented by `sl_free`, which also hands its argument to `free` after a null test. The two together count the open states.

### The feeding function: `sl_feed`

```text
1141:  test   rdx,rdx
1144:  je     116e                          ; no bytes: skip the loop
1146:  mov    r8,rsi
1149:  add    rsi,rdx                       ; end of the buffer
114c:  movzx  eax,BYTE PTR [r8]             ; next byte
1150:  xor    eax,DWORD PTR [rdi]           ; ... xor first field
1152:  imul   eax,eax,0x1000193
1158:  mov    DWORD PTR [rdi],eax           ; first field = (first xor byte) * 16777619
115a:  mov    ecx,DWORD PTR [rdi+0x4]
115d:  rol    ecx,0x5                       ; second field rotated left by 5
1160:  add    eax,ecx
1162:  mov    DWORD PTR [rdi+0x4],eax       ; second field = rol(second, 5) + first
1165:  add    r8,0x1
1169:  cmp    r8,rsi
116c:  jne    114c
116e:  add    QWORD PTR [rdi+0x8],rdx       ; the 64-bit count grows by n
1172:  ret
```

The signature has three arguments: the state in `rdi`, the buffer in `rsi` (read with `movzx ... BYTE PTR`) and the length in `rdx`. The state has two 32-bit fields at offsets 0 and 4 that change on every byte, and a 64-bit field at offset 8 that grows by the total length once, after the loop. So `C` is 8 and `F` is 5. The state is 4 + 4 + 8 = 16 bytes, which agrees with the allocation.

### The digest: `sl_final`

```text
1173:  mov    eax,DWORD PTR [rdi+0x4]
1176:  rol    eax,0xd                       ; second field rotated left by 13
1179:  xor    eax,DWORD PTR [rdi+0x8]       ; xor the low 32 bits of the count
117c:  xor    eax,DWORD PTR [rdi]           ; xor the first field
117e:  ret
```

The function reads the state without changing it and returns a 32-bit value: `unsigned sl_final(const struct ctx *)`. So `G` is 13. The two rotation amounts are in different functions, which is the trap in the question.

### The packing function: `sl_pack`

```text
11f0:  lea    edx,[rcx+rcx*1]               ; v + v
11f3:  sar    ecx,0x1f                      ; 0 or -1 from the sign of v
11f6:  xor    edx,ecx                       ; zigzag value
11f8:  cmp    edx,0x7f
11fb:  jbe    11d0                          ; fits in one byte
...
1218:  add    rax,0x1
121c:  mov    ecx,edx
121e:  or     ecx,0xffffff80                ; low 7 bits with bit 7 set
1221:  mov    BYTE PTR [rdi+rax*1-0x1],cl
1225:  shr    edx,0x7
1228:  cmp    edx,0x7f
122b:  ja     1218
```

Four arguments are read: the integers in `rdi` (read with a 4-byte scale), the count in `rsi`, the output buffer in `rdx` (moved to `rdi`) and the capacity in `rcx` (moved to `r11`). The failure paths set `mov eax,0xffffffff`: a 32-bit -1, so the return type is `int`. Each value is first mapped by `(v + v) xor (v >> 31)` with an arithmetic shift, so that small magnitudes of either sign give small numbers. The mapped number is then written seven bits at a time, low bits first, with bit 7 set on every byte except the last. Before writing a value the code adds its length to the bytes written so far and compares the total with the capacity (`cmp r11,rsi` / `jb` to the failure path); the check happens per value, so a value either fits entirely or is not written, and the values before it stay in the buffer.

For `B`: 0 maps to 0 (one byte), -1 maps to 1 (one byte), 300 maps to 600 (two bytes, since 600 is above 127 and below 16384), and -70000 maps to 139999 (three bytes, since it is above 16383 and below 2097152). The total is 1 + 1 + 2 + 3 = 7. A short C program that calls the library confirms it: the output area holds `00 01 d8 04 df c5 08`.

## Part 2: Assemble

The reference solution is `solution.asm`. The points that matter:

- **`digest_buffer`.** Four calls: `sl_new(seed)`, `sl_feed(handle, p, n)`, `sl_final(handle)`, `sl_free(handle)`. The handle and the buffer arguments live in callee-saved registers because they must survive calls, and the digest is moved to a callee-saved register before `sl_free` so that the result survives that call too. Three pushes and the return address keep `rsp` aligned.
- **`pack_values`.** It needs no calls and no callee-saved registers. For each value it computes the zigzag number, counts the bytes it needs, compares the total with the capacity, and either returns -1 or writes the bytes. The capacity test uses an unsigned comparison; the length is computed before anything is written, so a value that does not fit leaves the output untouched.
- **The zigzag step.** The arithmetic shift `sar r10d, 31` turns the sign into 0 or -1, and the exclusive or flips all bits of a negative doubled value.

## Mistakes that cost time

- **Forgetting `sl_free`.** The results are right and the checker reports `handle`.
- **Releasing the handle twice.** The library's counter goes negative or the program aborts in `free`.
- **A logical shift in the zigzag step.** Positive values work. Negative values encode wrongly.
- **Checking the capacity per byte.** The failing value is partly written, and the output area differs from the library's.
- **An off-by-one in the capacity test.** A value that exactly fits is rejected, or one byte too many is written.
- **Declaring the packing function as `long`.** The failure path sets `eax` to a 32-bit -1, so the result is an `int`; a wrong prototype in your probe hides this.
- **Losing the digest across `sl_free`.** `rax` is overwritten by the call; the digest must be saved first.
