[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# 07. Strings and string instructions

Time: about 2 hours. Prerequisites: [06. Conditions, branches and loops](../06-conditions-and-loops/README.md)

## Why this matters

Copying, filling, comparing and searching blocks of memory are among the most common operations in any program, and x86 has dedicated instructions for them. You will meet these instructions in hand-written assembly, in size-optimized compiled code and in older software, and recognizing which library function a `rep` instruction stands for is a basic reversing skill. Writing `strlen`, `memcpy` and `memmove` yourself also shows how the processor treats memory one element at a time.

## Concepts

### Three ways to store a string

| Kind | Layout | Length |
|---|---|---|
| Terminated (C) | Characters followed by a zero byte | Found by scanning for the zero |
| Length-prefixed | One length byte, then the characters | Read from the first byte |
| Pointer and length | The address and the count are kept separately | Always known |

Assembly stores whatever you write; the convention is yours. [07_prefixed.asm](examples/07_prefixed.asm) reads a length-prefixed string with `lodsb` and prints it. A zero-terminated string can be scanned with `scasb`, and a block with a known length with `movsb`, `stosb` or `cmpsb`.

### The string instructions

| Instruction | Does | Registers used |
|---|---|---|
| `movs` | Copies the element at `[rsi]` to `[rdi]` | `rsi`, `rdi` |
| `lods` | Loads the element at `[rsi]` into `al`, `ax`, `eax` or `rax` | `rsi`, the accumulator |
| `stos` | Stores `al`, `ax`, `eax` or `rax` at `[rdi]` | `rdi`, the accumulator |
| `scas` | Compares the accumulator with the element at `[rdi]` | `rdi`, the accumulator |
| `cmps` | Compares the element at `[rsi]` with the element at `[rdi]` | `rsi`, `rdi` |

Each exists in four sizes: a `b`, `w`, `d` or `q` suffix selects 1, 2, 4 or 8 bytes (`movsb`, `movsw`, `movsd`, `movsq`). After each execution the pointers it used move by the element size. The registers are fixed: you do not name them, you load them first.

### The direction flag

Whether the pointers move up or down depends on the **direction flag** (DF). `cld` clears it so they move to higher addresses; `std` sets it so they move to lower ones. The System V ABI requires DF to be clear when a function is entered and when it returns, so a routine that sets it with `std` must restore it with `cld` before `ret`.

### Repeating: the `rep` prefixes

A prefix in front of a string instruction makes it repeat with the count in `rcx`:

| Prefix | Repeats | Typical partner |
|---|---|---|
| `rep` | `rcx` times | `movs`, `stos`, `lods` |
| `repe` or `repz` | While `rcx` > 0 and the last comparison was equal (ZF = 1) | `cmps`, `scas` |
| `repne` or `repnz` | While `rcx` > 0 and the last comparison was not equal (ZF = 0) | `scas`, `cmps` |

Each pass decrements `rcx`. If `rcx` is 0 on entry, the instruction does nothing and leaves the flags as they were, so test for a zero count first when the result depends on the flags.

### The classic routines

Complete versions are in [07_strings.asm](examples/07_strings.asm). Their shape:

```nasm-fragment
my_strlen:
    xor     eax, eax                    ; al = 0, the byte to find
    mov     rcx, -1                     ; effectively no limit
    repne scasb                         ; scan until [rdi] equals al
    not     rcx                         ; rcx counted down from -1
    lea     rax, [rcx - 1]              ; bytes scanned, minus the terminator
    ret

my_memcpy:
    mov     rax, rdi                    ; the return value
    mov     rcx, rdx                    ; the count
    rep movsb                           ; copy rcx bytes from [rsi] to [rdi]
    ret
```

Filling is `rep stosb` with the byte in `al`, comparing is `repe cmpsb`, and searching is `repne scasb` with the wanted byte in `al`.

### Overlapping copies

A forward copy goes wrong when the destination lies inside the source and above it: the copy overwrites bytes it has not read yet. `memmove` handles this by checking the direction first. If the destination is below the source, or starts after the source ends, it copies forward. Otherwise it sets `std`, starts at the last byte of both blocks, copies backward, and clears the flag again. With the buffer `abcdefgh`, moving 6 bytes up by 2 gives `ababcdef`; moving them down by 2 gives `cdefghgh`.

### The reversing view

Do not expect compilers to emit `rep` everywhere. Run `make compiled` in the examples folder to see both optimization levels for the same C functions. The important lines:

```text
== gcc -O1
<clear_struct>:   ...  rep stos QWORD PTR es:[rdi],rax
<copy_struct>:    sixteen movdqu/movups pairs (SSE)
<copy_var>:       call to memcpy
<length>:         call to strlen

== gcc -Os
<clear_struct>:   mov  ecx,0x40
                  xor  eax,eax
                  rep stos DWORD PTR es:[rdi],eax
<copy_struct>:    mov  ecx,0x40
                  rep movs DWORD PTR es:[rdi],DWORD PTR ds:[rsi]
<copy_var>:       mov  rcx,rdx
                  rep movs BYTE PTR es:[rdi],BYTE PTR ds:[rsi]
```

What to read from this:

- **Optimized for speed (`-O1`).** Variable-size operations become calls to the C library, and small fixed copies become SSE moves. A `call` to `memcpy`, `strlen` or `memset` is the compiled form of those C functions.
- **Optimized for size (`-Os`).** The compiler uses `rep` instructions because they are short.
- **Count times element size.** `mov ecx, 0x40` with `rep stos DWORD` clears 0x40 times 4 = 256 bytes. The element size in the instruction and the value of `ecx` together give the block size.
- **What the registers say.** `rep stos` with `eax` or `rax` equal to zero is `memset(p, 0, n)`. `rep movs` is `memcpy`. `repne scas` with `al` equal to zero is `strlen`. `repe cmps` is `memcmp`.
- **`std` before `rep movs`.** A backward copy: an overlapping `memmove`, or a routine that works from the end of a buffer.

## Try it

Build and run the examples:

```bash
cd modules/07-strings/examples
make run
```

Expected output:

```text
my_strlen("assembly"), ("")  = 8 0
my_memchr("assembly", 'm', 8) = offset 4
my_memchr("assembly", 'z', 8) = NULL
my_memset(buf, 'x', 6)        = xxxxxx..
my_memcpy(buf, "assembly", 8) = assembly
my_memcmp: abc vs abd, abc, abd vs abc = negative zero positive
memmove up by 2 (overlap)      = ababcdef
memmove down by 2 (overlap)    = cdefghgh
direction flag after the calls = 0
Hello
```

The last line comes from the length-prefixed example. The two `memmove` lines move the same bytes up by 2 and down by 2 inside one buffer; only the first needs a backward copy. The direction flag is 0 after the calls, as the ABI requires.

Then change something and predict the result before you rebuild:

1. In `my_memmove`, remove the `std` and the `cld` and the two `lea` lines for the backward case, so every call copies forward. Which of the two `memmove` lines changes, and why?
2. In `my_strlen`, replace `mov rcx, -1` with `mov rcx, 4`. What does `my_strlen("assembly")` return?

## Common pitfalls

- **Forgetting `cld` after `std`.** The caller's next string instruction then runs backward. The direction flag must be clear when you return.
- **Reading ZF after a zero count.** With `rcx` equal to 0, a `repe` or `repne` instruction does nothing and the flags keep their old values, so a found/not-found test is meaningless. Check for a zero count first.
- **Swapping source and destination.** `movs` copies from `[rsi]` to `[rdi]`; `cmps` compares `[rsi]` with `[rdi]`. For `memcmp(a, b)` the pointer `a` is in `rdi`, so the byte difference to return is `[rdi]` minus `[rsi]`. Getting it the other way round flips the sign of every result.
- **Off by one in `strlen`.** After `repne scasb` the pointer is one past the zero byte, and `not rcx` counts the zero too. Subtract one.
- **Copying forward over an overlap.** A forward `rep movsb` where the destination starts inside the source duplicates a pattern instead of moving the data.
- **Reading past the end.** A routine that uses `scasq` or `movsq` to go faster reads whole words and can touch memory beyond the string.

## Summary

- The string instructions work through `rsi`, `rdi`, `rcx` and the accumulator, and move the pointers by the element size.
- `rep`, `repe` and `repne` repeat them; a count of zero does nothing and leaves the flags untouched.
- The direction flag chooses forward or backward; clear it again with `cld` before you return.
- `strlen`, `memchr`, `memset`, `memcpy`, `memcmp` and `memmove` are each a few instructions; `memmove` needs a backward copy for overlaps.
- In compiled code, `rep` instructions mostly appear in size-optimized builds; a `call` to the library is the common form otherwise.

## Next steps

- Take the [quiz](quiz.md).
- Continue with module 08, the stack, procedures and calling conventions.
- Exercises that use this module: [Exercise 07](../../exercises/07-string-sweeper/README.md).
