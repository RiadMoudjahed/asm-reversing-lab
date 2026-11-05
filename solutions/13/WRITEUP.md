[English](WRITEUP.md) | [العربية](WRITEUP.ar.md) | [Deutsch](WRITEUP.de.md) | [Español](WRITEUP.es.md) | [Français](WRITEUP.fr.md)

# Write-up: exercise 13. Magic numbers

The flag is `ARL{magic_12_9_60_11_5_3}`: the score is `(u / 12) * 5 + (u % 9) * 3 - (x / 60) + (x % 11) * 4`, where `u` is `x` read as unsigned, the first two operations are unsigned and the last two are signed.

## Part 1: Reverse

### The code

```bash
objdump -d -M intel --no-show-raw-insn bin/magic
```

```text
0000000000401190 <mix>:
  401190:    mov    edx,edi
  401192:    mov    ecx,0xaaaaaaab
  401197:    mov    rax,rdx
  40119a:    imul   rcx,rdx
  40119e:    imul   rdx,rdx,0x38e38e39
  4011a5:    mov    edi,eax
  4011a7:    sar    edi,0x1f
  4011aa:    shr    rcx,0x23
  4011ae:    shr    rdx,0x21
  4011b2:    lea    esi,[rcx+rcx*4]
  4011b5:    lea    ecx,[rdx+rdx*8]
  4011b8:    mov    edx,eax
  4011ba:    sub    edx,ecx
  4011bc:    lea    edx,[rdx+rdx*2]
  4011bf:    add    esi,edx
  4011c1:    movsxd rdx,eax
  4011c4:    imul   rcx,rdx,0xffffffff88888889
  4011cb:    imul   rdx,rdx,0x2e8ba2e9
  4011d2:    shr    rcx,0x20
  4011d6:    add    ecx,eax
  4011d8:    sar    rdx,0x21
  4011dc:    sar    ecx,0x5
  4011df:    sub    edx,edi
  4011e1:    sub    ecx,edi
  4011e3:    sub    esi,ecx
  4011e5:    lea    ecx,[rdx+rdx*4]
  4011e8:    lea    edx,[rdx+rcx*2]
  4011eb:    sub    eax,edx
  4011ed:    lea    eax,[rsi+rax*4]
  4011f0:    ret
```

The compiler interleaved the four computations, so read them by following each constant.

### The four divisions

Each division is an `imul` with a large constant followed by one or more right shifts. For each, the divisor is `2^shift / constant`, rounded. The script from module 14 does the arithmetic and checks it:

```bash
python3 14_magic.py 0xaaaaaaab 35
python3 14_magic.py 0x38e38e39 33
python3 14_magic.py 0x88888889 37 signed
python3 14_magic.py 0x2e8ba2e9 33 signed
```

```text
2**35 / 2863311531 = 11.9999999986  ->  divisor 12
checked 200007 inputs, 0 differences
2**33 / 954437177 = 8.9999999990  ->  divisor 9
checked 200007 inputs, 0 differences
2**37 / 2290649225 = 59.9999999878  ->  divisor 60
checked 200007 inputs, 0 differences
2**33 / 780903145 = 10.9999999962  ->  divisor 11
checked 200007 inputs, 0 differences
```

| Division | Evidence | Divisor |
|---|---|---|
| Unsigned quotient | `0xaaaaaaab` (a 32-bit value in `ecx`, so the multiply is unsigned) and `shr rcx, 0x23`, which is 35 | 12 |
| Unsigned remainder | `0x38e38e39` and `shr rdx, 0x21`, which is 33; the quotient is then multiplied by 9 with `lea ecx,[rdx+rdx*8]` and subtracted from the input | 9 |
| Signed quotient | `0xffffffff88888889` (the 32-bit constant 0x88888889 with its top bit set, so it is shown sign-extended), `shr rcx, 0x20`, the `add ecx, eax` that adds `x` back, and `sar ecx, 5`; the total shift is 32 + 5 = 37 | 60 |
| Signed remainder | `movsxd`, `0x2e8ba2e9` and `sar rdx, 0x21`, which is 33; the quotient is multiplied by 11 with two `lea` instructions and subtracted from `x` | 11 |

The signed quotient shows a variant that the lesson does not spell out. The magic number for 60 does not fit in a positive 32-bit value, so the compiler multiplies by the 32-bit value read as negative, and adds `x` back after the high half has been taken: `(x * (M - 2^32)) >> 32` plus `x` equals `(x * M) >> 32`. The script uses `M` read as positive, which is the value that the divisor formula needs.

The `sar edi, 0x1f` near the top computes the sign of `x` once (0 or -1). Both signed quotients subtract it (`sub edx, edi` and `sub ecx, edi`), which is the correction for negative inputs. The unsigned terms start from `mov edx, edi` and `mov rax, rdx`, which zero-extend `x`; the signed terms use `movsxd`.

### The factors

After the unsigned quotient is computed in `rcx`, `lea esi, [rcx+rcx*4]` multiplies it by 5, so `K` is 5. After the remainder `u - q * 9`, `lea edx, [rdx+rdx*2]` multiplies it by 3, so `M` is 3. The two results are added (`add esi, edx`). The signed quotient is then subtracted (`sub esi, ecx`), and the signed remainder is multiplied by 4 and added in the final `lea eax, [rsi+rax*4]`.

The formula is `(u / 12) * 5 + (u % 9) * 3 - (x / 60) + (x % 11) * 4`, and the flag is `ARL{magic_12_9_60_11_5_3}`.

## Part 2: Assemble

The reference solution is `solution.asm`. The points that matter:

- **Unsigned quotient.** `mov eax, edi` zero-extends `x` into `rax`; a 64-bit `imul` by `0xAAAAAAAB` gives the full product, and `shr rax, 35` is the quotient. `lea r8d, [rax+rax*4]` multiplies it by 5.
- **Unsigned remainder.** The quotient by 9 comes from `0x38E38E39` and a shift of 33. The remainder is `u` minus quotient times 9 (`lea eax, [rax+rax*8]`), and `lea edx, [rdx+rdx*2]` multiplies it by 3.
- **Signed quotient.** `movsxd rax, edi` sign-extends. The constant `0x88888889` is loaded with `mov edx, 0x88888889`, which zero-extends it and so keeps it positive, and the 64-bit product is shifted right by 37. Because `sar` rounds toward minus infinity, 1 is added for negative inputs, which `sub edx, ecx` does with `ecx = x >> 31`.
- **Signed remainder.** The same sign correction applies to the quotient `(x * 0x2E8BA2E9) >> 33`, and `q * 11` is `q + q*5*2`, computed with two `lea` instructions.
- **Combining.** The last `lea` adds `d * 4` to the running sum, which already holds the first three terms.

## Mistakes that cost time

- **No sign correction.** Without the subtraction of `x >> 31`, a signed quotient is one too small in magnitude for negative inputs: the score is off by 1 when the correction is missing for `x / 60`, and by 44 (four times the divisor 11) when it is missing for `x % 11`.
- **A wrong shift.** `shr rax, 34` instead of 35 changes the quotient for every input of 6 and up.
- **Zero-extending the signed terms.** `mov eax, edi` instead of `movsxd` gives wrong results for every negative input.
- **Reading the constant as negative.** The 60 case needs the constant as the positive 2290649225 when you do not add `x` back.
- **Using `div`.** The checker rejects it even when the results are right.
