[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# 16. Undocumented interfaces

Time: about 2 hours. Prerequisites: [15. Recovering data](../15-recovering-data/README.md)

## Why this matters

Much of the code you meet comes as a library without a header or documentation: a vendor binary, an old component whose source is lost, a plug-in interface. To call it, replace it or check what it does, you need its interface: the names, the arguments, the return value and the rules of use. The binary contains all of it. This module teaches a method to recover an interface from the compiled code, and to confirm the result with a test program before you rely on it.

## Concepts

### What you can see from outside

A shared library lists the functions it offers to others in its dynamic symbol table. That list is the **export** list, and it is the first thing to read. The lab image builds the practice library `libblackbox.so` from `examples/16_blackbox.c`; the lesson works from the binary only.

```bash
make
nm -D build/libblackbox.so
```

```text
                 w _ITM_deregisterTMCloneTable
                 w _ITM_registerTMCloneTable
                 w __cxa_finalize@GLIBC_2.2.5
                 w __gmon_start__
0000000000001126 T bb_checksum
0000000000001119 T bb_clamp
000000000000121f T bb_close
00000000000011dc T bb_lerp
00000000000011a5 T bb_minmax
0000000000001213 T bb_next
00000000000011e9 T bb_open
0000000000001145 T bb_split
                 U calloc@GLIBC_2.2.5
                 U free@GLIBC_2.2.5
```

`T` marks a function the library defines and `U` one it needs from elsewhere. The names are already a clue: `bb_open`, `bb_next` and `bb_close` look like a set, and `calloc` and `free` appear in the undefined list. Names can mislead, so treat each one as a hypothesis, never as a fact. The imports are good evidence too: a library that imports `calloc` allocates memory somewhere.

### The signature you are looking for

For a C library the **signature** of a function is its return type, its name and the types of its parameters. The processor does not store types, so you rebuild them from how the instructions treat each register. On x86-64 System V the first integer or pointer arguments arrive in `rdi`, `rsi`, `rdx`, `rcx`, `r8` and `r9`, floating-point arguments in `xmm0` to `xmm7`, and the result leaves in `rax` or `xmm0`. The method asks five questions of each function.

| Question | Evidence to look for |
|---|---|
| How many arguments? | Argument registers that are read before they are written. The highest one read is the least number of arguments |
| How wide is each? | `edi` is 32 bits, `rdi` is 64 bits, `dil`, `sil` or `r8b` is 8 bits. Operand width follows the C type |
| Is it a pointer? | The register is used inside brackets, `[rdi]`, or added to another register as an address |
| What does it point to? | The width of the access: `BYTE PTR`, `DWORD PTR`, `QWORD PTR`; the scale of an index; `movzx` or `movsx` after a load |
| Signed or unsigned? | `jl`, `jg`, `cmovg`, `movsx`, `sar` for signed; `jb`, `ja`, `movzx`, `shr` for unsigned |
| What does it return? | The last write to `eax`, `rax` or `xmm0` before `ret`, and its width. If no path writes the register, the function returns nothing |

Treat the result as a hypothesis until a test confirms it. A void function can leave an unrelated value in `rax`, and an argument that the function ignores leaves no trace at all.

### Example 1: a function with simple values

```bash
objdump -d -M intel --no-show-raw-insn build/libblackbox.so | sed -n '/ 1119:/,/ 1125:/p'
```

```text
    1119:    mov    eax,esi
    111b:    cmp    edi,edx
    111d:    cmovle edx,edi
    1120:    cmp    edi,esi
    1122:    cmovge eax,edx
    1125:    ret
```

The function reads `edi`, `esi` and `edx` and writes `eax`: three 32-bit arguments and a 32-bit result. All comparisons are signed (`cmovle`, `cmovge`), so the values are `int`. Follow the logic. The result starts as `esi`. The first `cmp` compares `edi` with `edx`, and when `edi` is less than or equal to `edx` it replaces `edx` by `edi`; so `edx` becomes the smaller of `edi` and `edx`. If `edi` is greater than or equal to `esi`, the result becomes that limited value; otherwise it stays `esi`. The conclusion: `esi` is a lower bound, `edx` is an upper bound, and `edi` is the value that is limited.

```c
int bb_clamp(int v, int lo, int hi);
```

The names `v`, `lo` and `hi` are yours. Choosing names that describe the role is part of the work.

### Example 2: a buffer and a length

```text
0000000000001126 <bb_checksum>:
    1126:    mov    eax,edx
    1128:    test   rsi,rsi
    112b:    je     1144
    112d:    mov    rdx,rdi
    1130:    add    rdi,rsi
    1133:    rol    eax,0x5
    1136:    movzx  ecx,BYTE PTR [rdx]
    1139:    xor    eax,ecx
    113b:    add    rdx,0x1
    113f:    cmp    rdx,rdi
    1142:    jne    1133
    1144:    ret
```

- `rdi` is used as an address and `rsi` is added to it to get an end address: `rdi` is a pointer and `rsi` is a count. The count is tested as a 64-bit value (`test rsi,rsi`), so it is `size_t`, not `int`.
- Each round loads one byte with `movzx ecx,BYTE PTR [rdx]` and zero-extends it. The zero extension says the element type is `unsigned char`. The code only reads the buffer, so the pointer is `const`.
- `eax` starts as `edx`: the third argument is a 32-bit starting value. Each round rotates `eax` left by 5 and xors the byte in. The result is in `eax`.

```c-fragment
unsigned bb_checksum(const unsigned char *p, size_t n, unsigned seed);
```

The behavior is a hypothesis you can state exactly: start with `seed`, then for each byte rotate left by 5 and xor the byte. Later you will test it with an independent program.

### Example 3: a return value that is an error code

```text
0000000000001145 <bb_split>:
    1145:    mov    r9d,esi
    1148:    test   rcx,rcx
    114b:    je     1195
    114d:    movzx  r8d,BYTE PTR [rdi]
    1151:    cmp    r8b,sil
    1154:    je     118b
    1156:    test   r8b,r8b
    1159:    je     118b
    115b:    cmp    rcx,0x1
    115f:    jbe    119d
    1161:    mov    eax,0x1
    1166:    mov    BYTE PTR [rdx+rax*1-0x1],r8b
    116b:    movzx  r8d,BYTE PTR [rdi+rax*1]
    1170:    cmp    r8b,r9b
    1173:    je     1190
    1175:    test   r8b,r8b
    1178:    je     1190
    117a:    add    rax,0x1
    117e:    cmp    rcx,rax
    1181:    jne    1166
    1183:    mov    rax,0xffffffffffffffff
    118a:    ret
    118b:    mov    eax,0x0
    1190:    mov    BYTE PTR [rdx+rax*1],0x0
    1194:    ret
    1195:    mov    rax,0xffffffffffffffff
    119c:    ret
    119d:    mov    rax,0xffffffffffffffff
    11a4:    ret
```

- Four arguments: `rdi`, `sil` (the low byte of `esi`), `rdx` and `rcx` are read. `sil` is compared with bytes (`cmp r8b,sil`), so the second argument is a `char`.
- `rdi` is read byte by byte until a byte equals `sil` or is zero: a source string and a separator.
- `rdx` is written byte by byte (`mov BYTE PTR [rdx+rax*1-0x1],r8b`) and ends with a zero byte (`mov BYTE PTR [rdx+rax*1],0x0`): a destination buffer that receives a string.
- `rcx` is compared with the position and bounds the loop: the capacity of the destination, as `size_t`.
- The result is set with `mov rax,0xffffffffffffffff` on three paths. This is 64-bit `-1`, so the return type is `long` (or `ssize_t`), and the three paths are the error cases: zero capacity, a capacity of one with a field that is not empty, and a source field that does not fit. On success the result is the number of characters copied.

```c-fragment
long bb_split(const char *s, char sep, char *out, size_t cap);
```

Two details come only from careful reading: the capacity includes the terminating zero, and on failure the function may already have written part of the output. A test will check both.

### Example 4: output parameters

```text
00000000000011a5 <bb_minmax>:
    11a5:    mov    r9d,DWORD PTR [rdi]
    11a8:    cmp    rsi,0x1
    11ac:    jbe    11d7
    11ae:    lea    r8,[rdi+0x4]
    11b2:    lea    rdi,[rdi+rsi*4]
    11b6:    mov    esi,r9d
    11b9:    mov    eax,DWORD PTR [r8]
    11bc:    cmp    r9d,eax
    11bf:    cmovg  r9d,eax
    11c3:    cmp    esi,eax
    11c5:    cmovl  esi,eax
    11c8:    add    r8,0x4
    11cc:    cmp    r8,rdi
    11cf:    jne    11b9
    11d1:    mov    DWORD PTR [rdx],r9d
    11d4:    mov    DWORD PTR [rcx],esi
    11d6:    ret
    11d7:    mov    esi,r9d
    11da:    jmp    11d1
```

- `rdi` points to 4-byte elements, and `rsi` is the element count (the end address is `rdi+rsi*4`): `const int *a, size_t n`.
- `rdx` and `rcx` are not read; the last instructions store into `[rdx]` and `[rcx]`. A pointer that the function only writes through is an **output parameter**: the caller passes the address of a variable and gets the value there.
- No path writes `eax` as a result, so the function returns nothing: `void`. (`eax` is used as a scratch register inside the loop.)
- The first instruction reads `[rdi]` before the count is checked. The function therefore needs `n` of at least 1, which is a rule of use that no type shows.

```c-fragment
void bb_minmax(const int *a, size_t n, int *lo, int *hi);
```

### Example 5: floating-point values

```text
00000000000011dc <bb_lerp>:
    11dc:    subsd  xmm1,xmm0
    11e0:    mulsd  xmm1,xmm2
    11e4:    addsd  xmm0,xmm1
    11e8:    ret
```

The `sd` suffix means scalar double. The function reads `xmm0`, `xmm1` and `xmm2` and returns in `xmm0`. With `a` in `xmm0`, `b` in `xmm1` and `t` in `xmm2`, the code computes `a + (b - a) * t`. Integer registers do not take part, so the signature has no integer arguments. A suffix `ss` would mean `float`.

```c
double bb_lerp(double a, double b, double t);
```

### Example 6: a handle with a lifetime

Three functions work together.

```text
00000000000011e9 <bb_open>:
    11e9:    push   rbp
    11ea:    push   rbx
    11eb:    sub    rsp,0x8
    11ef:    mov    ebp,edi
    11f1:    mov    ebx,esi
    11f3:    mov    esi,0xc
    11f8:    mov    edi,0x1
    11fd:    call   1040 <calloc@plt>
    1202:    test   rax,rax
    1205:    je     120c
    1207:    mov    DWORD PTR [rax],ebp
    1209:    mov    DWORD PTR [rax+0x4],ebx
    120c:    add    rsp,0x8
    1210:    pop    rbx
    1211:    pop    rbp
    1212:    ret

0000000000001213 <bb_next>:
    1213:    add    DWORD PTR [rdi+0x8],0x1
    1217:    mov    eax,DWORD PTR [rdi+0x4]
    121a:    add    eax,DWORD PTR [rdi]
    121c:    mov    DWORD PTR [rdi],eax
    121e:    ret

000000000000121f <bb_close>:
    121f:    sub    rsp,0x8
    1223:    call   1030 <free@plt>
    1228:    add    rsp,0x8
    122c:    ret
```

- `bb_open` takes two 32-bit values, calls `calloc(1, 12)` and stores the two values into the new block at offsets 0 and 4. It returns the block, or null when the allocation fails (the `test rax,rax` branch skips the stores). The block is 12 bytes.
- `bb_next` takes the block, increments the 32-bit field at offset 8, adds the field at offset 4 to the field at offset 0, and returns the new value of the field at offset 0.
- `bb_close` hands its argument to `free`.

The block is an **opaque handle**: the caller receives a pointer, passes it back to other functions, and never needs to know the layout. From outside, its type is `void *`. Inside, you recovered the layout: a current value at 0, a step at 4 and a call counter at 8. The rules of use come from the set: every handle from `bb_open` must go to `bb_close` exactly once, a handle must not be used after it is closed, and a null result from `bb_open` must be checked.

```c
void *bb_open(int start, int step);
int bb_next(void *c);
void bb_close(void *c);
```

### Call sites are evidence

When a program uses the library, the code around each call shows how the result is used. The file `examples/16_caller.c` calls `bb_split` and reports failure. Compiled at `-O1`:

```text
copy_key:
    sub    rsp, 8
    mov    rdx, rsi
    mov    ecx, 32
    mov    esi, 61
    call    bb_split@PLT
    test    rax, rax
    mov    rdx, -1
    cmovs    rax, rdx
    add    rsp, 8
    ret
```

The caller passes its second parameter (`rsi`) on as the third argument, the constant 32 as the capacity, and the character 61, which is `=`, as the separator. After the call, `test rax,rax` and `cmovs` look at the sign of all 64 bits of `rax`: the result is a signed 64-bit value, and a negative one means failure. If you have a program that uses a library, search it for the calls first. The constants, the tests after each call and the order of the calls tell you what the author expected.

### Confirming with a test program

A hypothesis becomes knowledge when a program written from it behaves as predicted. Write the prototypes you recovered, call the functions with normal values, then with the boundaries, and compare with what you predicted before you ran it. The file `examples/16_probe.c` does this for the practice library:

```bash
make run
```

```text
clamp(5, 0, 3) = 3
clamp(-5, 0, 3) = 0
clamp(2, 0, 3) = 2
checksum("abc", 3, 0) = 0x00018823
checksum("abc", 3, 1) = 0x00010823
checksum("abc", 2, 0) = 0x00000c42
split("key=value", '=', out, 8) = 3, out = "key"
split("abcdefghij", ',', out, 8) = -1
split("abcdefg", ',', out, 8) = 7
minmax = -3 12
lerp(10.0, 20.0, 0.25) = 12.50
next = 105
next = 110
next = 115
counter bytes: 73 00 00 00 05 00 00 00 03 00 00 00 
```

Read the results as tests of the hypotheses. `clamp` limits on both sides. The `split` call with capacity 8 fails for a 10-character field and succeeds for a 7-character one, so a field of 7 characters plus the terminator fits exactly into 8 bytes; that confirms the capacity includes the zero byte. The last line dumps the handle: the value is 0x73 (115), the step is 5, and the counter is 3, the layout recovered from `bb_open` and `bb_next`.

For the checksum, the hypothesis is precise enough to test with an independent implementation in another language. The script `examples/16_checksum.py` follows the rule "rotate left by 5, then xor the byte":

```bash
python3 16_checksum.py abc 3 0
python3 16_checksum.py abc 3 1
python3 16_checksum.py abc 2 0
```

```text
0x00018823
0x00010823
0x00000c42
```

All three match the library. Agreement on one input is weak evidence; agreement on many inputs including the boundaries (an empty buffer, a seed of zero, long buffers) is strong.

One trap in the probe: C does not define the order in which the arguments of a call are evaluated. A line such as `printf("%d %d", bb_next(c), bb_next(c))` can print the values in either order. The probe calls the function in a loop, one call per statement.

### Writing the header

The product of the work is a header that another programmer can use. It holds the prototypes, a short comment for each rule of use, and nothing else. It also states which statements are confirmed and which are not.

```c
#include <stddef.h>

/* Limits v to the range lo..hi. Requires lo <= hi. */
int bb_clamp(int v, int lo, int hi);

/* Rotate-and-xor checksum of n bytes, starting from seed. */
unsigned bb_checksum(const unsigned char *p, size_t n, unsigned seed);

/* Copies the text of s up to the first sep into out and ends it with a zero byte.
   cap is the size of out including the zero byte. Returns the number of characters
   copied, or -1 when the field does not fit. */
long bb_split(const char *s, char sep, char *out, size_t cap);

/* Smallest and largest of n values; n must be at least 1. */
void bb_minmax(const int *a, size_t n, int *lo, int *hi);

/* a + (b - a) * t */
double bb_lerp(double a, double b, double t);

/* Counter handle. bb_open returns null when memory runs out; every handle must be passed
   to bb_close exactly once and not used afterwards. bb_next adds step to the value and
   returns it. */
void *bb_open(int start, int step);
int bb_next(void *c);
void bb_close(void *c);
```

### Where the method needs more care

- **Arguments that are never read on some paths.** A parameter used only in a rarely taken branch can look unused. Read every path.
- **Structures passed or returned by value.** A structure larger than 16 bytes is returned through a hidden pointer that arrives in `rdi`, so every visible argument is shifted by one. Small structures travel in registers split by type.
- **Variadic functions.** `al` holds the number of vector registers used. A function that reads `al` early takes a variable argument list.
- **Callbacks.** An argument that the function calls (`call rdx`, `call rax`) is a function pointer. Its signature is determined by how the function sets up the registers before that call.
- **Output that depends on hidden state.** A function that reads a global variable gives different answers from the same arguments. Run the probe twice in different states.

## Try it

Run the lab commands from `modules/16-undocumented-interfaces/examples`.

1. Build with `make`. List the exports with `nm -D build/libblackbox.so`. Before you read any code, write down what you expect from each name.
2. Disassemble `bb_clamp` and `bb_lerp`. Write each signature from the table in this lesson, then check it against the lesson.
3. Run the library under GDB and look at one call to `bb_split` as it happens (command and output below the list).
4. Change the probe to call `bb_minmax` with `n` equal to 0. Predict what happens, then run it.
5. Write the header for a library of your own: compile a small C file with three functions to a shared library, delete the source from your mind, and recover the prototypes from the binary alone.

For step 3:

```bash
gdb -nx -q -batch -ex "break bb_split" -ex "run" -ex "info registers rdi rsi rdx rcx" \
    -ex "x/s \$rdi" -ex "print/c \$rsi" -ex "finish" -ex "print \$rax" -ex "x/s \$rdx" build/16_probe
```

```text
Breakpoint 1, 0x00007ed57bbc4145 in bb_split () from /lab/modules/16-undocumented-interfaces/examples/build/libblackbox.so
rdi            0x598f749bb0a8      98472671555752
rsi            0x3d                61
rdx            0x7ffe9bb3f5a0      140731510683040
rcx            0x8                 8
0x598f749bb0a8:    "key=value"
$1 = 61 '='
0x0000598f749ba3cd in main ()
$2 = 3
0x7ffe9bb3f5a0:    "key"
```

The four registers hold the source string, the separator `=` (61), the destination buffer and the capacity 8. After `finish`, `rax` is 3 and the destination holds `key`. The addresses vary from run to run.

## Common pitfalls

- **Trusting the names.** A function called `bb_next` could do anything. Read the code and test it.
- **Reading the return type from `eax` alone.** If `rax` is set with a 64-bit constant such as -1, the return type is 64 bits wide. A 32-bit write is a 32-bit result.
- **Missing a parameter that is only passed on.** An argument forwarded to another function is read once and looks unused. Follow each argument register until its last use.
- **Calling with the wrong width.** A prototype with `int` where the library reads a 64-bit register leaves the upper half of the register undefined. The call may work by luck.
- **Forgetting the rules of use.** A correct signature is not enough: a count that must not be zero, a buffer size that includes the terminator and a handle that must be closed once are all part of the interface.
- **Concluding from one test.** One matching input confirms nothing. Test the boundaries and unusual values.

## Summary

- The export list is the first view of an undocumented library, and names are hypotheses.
- A signature is rebuilt from which argument registers are read, the width of each use, how pointers are dereferenced and what is last written to the result register.
- A pointer that is only written through is an output parameter; a pointer from one function passed to others is a handle with a lifetime.
- Call sites, constants and the tests after a call show how the interface is meant to be used.
- A test program confirms each hypothesis, including the boundaries, and the result is a header with the prototypes and the rules of use.

## Next steps

- Take the [quiz](quiz.md).
- Continue with module 17, deciphering file formats.
- Exercises that use this module: [Exercise 16](../../exercises/16-silent-library/README.md).
