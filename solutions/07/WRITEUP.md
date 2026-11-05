[English](WRITEUP.md) | [العربية](WRITEUP.ar.md) | [Deutsch](WRITEUP.de.md) | [Español](WRITEUP.es.md) | [Français](WRITEUP.fr.md)

# Write-up: exercise 07. String sweeper

The flag is `ARL{sweeper_2_5_1_3_6_4}`: `fn2` is `strlen`, `fn5` is `memchr`, `fn1` is `memcpy`, `fn3` is `memmove`, `fn6` is `memset` and `fn4` is `memcmp`. Each routine is identified by its string instruction, its prefix and the registers it sets up before it.

## Part 1: Reverse

### The six routines

```text
fn1:  mov rax,rdi            fn2:  xor eax,eax
      mov rcx,rdx                  mov rcx,-1
      shr rcx,3                    repnz scas al,[rdi]
      rep movs QWORD               not rcx
      mov ecx,edx                  lea rax,[rcx-1]
      and ecx,7                    ret
      rep movs BYTE
      ret

fn3:  mov rax,rdi            fn4:  mov rcx,rdx
      mov rcx,rdx                  xor eax,eax
      cmp rdi,rsi                  repz cmps [rsi],[rdi]
      jbe .forward                 je .same
      lea r8,[rsi+rdx]             movzx eax,[rdi-1]
      cmp rdi,r8                   movzx ecx,[rsi-1]
      jae .forward                 sub eax,ecx
      std                          ret
      lea rsi,[rsi+rdx-1]
      lea rdi,[rdi+rdx-1]    fn5:  mov eax,esi
      rep movs BYTE                mov rcx,rdx
      cld                          test rcx,rcx
      ret                          je .none
.forward: rep movs BYTE            repnz scas al,[rdi]
      ret                          jne .none
                                   lea rax,[rdi-1]
fn6:  mov r8,rdi                   ret
      movzx eax,sil
      mov rcx,rdx
      rep stos BYTE
      mov rax,r8
      ret
```

### Identifying each one

| Routine | Evidence | Function |
|---|---|---|
| `fn1` | `rep movs` copies `rcx` quadwords and then the remaining bytes; no direction handling; returns `rdi` | `memcpy` |
| `fn2` | `al` is zero and `rcx` is -1; `repnz scas` scans until the zero byte; `not rcx` and subtracting one turn the count into a length | `strlen` |
| `fn3` | Compares the addresses, and when the destination is above the source and overlaps it, executes `std`, copies from the last byte and restores `cld` | `memmove` |
| `fn4` | `repz cmps` stops at the first difference; the two differing bytes are subtracted to give the result | `memcmp` |
| `fn5` | `al` comes from the second argument and `rcx` from the third; `repnz scas` stops at a match; returns `rdi - 1` or zero | `memchr` |
| `fn6` | `al` takes the byte from the second argument and `rep stos` stores it `rcx` times | `memset` |

The two routines that use `repnz scas` differ in what they compare with and what they return: `fn2` looks for a zero and returns a count, `fn5` looks for a byte given by the caller and returns a pointer. The two copies differ in direction handling: only `fn3` has `std` and `cld`. In the order strlen, memchr, memcpy, memmove, memset, memcmp, the numbers are 2, 5, 1, 3, 6 and 4.

The flag is `ARL{sweeper_2_5_1_3_6_4}`.

## Part 2: Assemble

The reference solution is `solution.asm`, with the same shapes as `fn2`, `fn5`, `fn1` (simplified to a byte copy) and `fn3`. The points that matter:

- **`my_strlen`.** `al` is 0, `rcx` is -1, `repnz scasb`, then `not rcx` and subtract one.
- **`my_memchr`.** Check for `n` equal to 0 before the scan, because with `rcx` at 0 the scan does nothing and the zero flag keeps whatever value it had. After the scan, the zero flag says whether a match was found.
- **`my_memcpy`.** Save `rdi` for the return value, put `n` in `rcx`, and run `rep movsb`.
- **`my_memmove`.** A forward copy is safe when the destination is not above the source, or when it starts at or after the end of the source. Otherwise copy backward with `std` and clear the flag before returning.

## Mistakes that cost time

- **Always copying forward in `my_memmove`.** The overlapping cases come out wrong; 8 of the 74 test cases fail.
- **Leaving the direction flag set.** The checker reports `BAD direction flag left set`. A caller's next string instruction would run backward.
- **Copying too many bytes.** Writing past the end of the block runs into the inaccessible page and the test crashes with signal 11.
- **Skipping the `n` equal to 0 check in `my_memchr`.** The test calls the routine with the zero flag set and `n` equal to 0; a routine that trusts the stale flag returns a bad pointer and fails that case.
- **Returning the pointer one byte too far.** After a match `rdi` is already one past the matching byte; 11 of the 74 cases fail.
- **Counting the terminator in `my_strlen`.** The result is one too large; 16 cases fail, starting with the empty string.
- **Calling another routine.** A `call` is forbidden; keep each routine self-contained.
