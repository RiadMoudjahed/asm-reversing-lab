[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# 15. Recovering data

Time: about 3 hours. Prerequisites: [14. Compiled arithmetic](../14-compiled-arithmetic/README.md)

## Why this matters

Machine code has no types. There are no arrays, no structures, no classes, only addresses, offsets and widths. Yet the most useful result of a reverse engineering session is often a description of the data: what a record contains, how a list is linked, which table drives a decision. This module teaches you to rebuild those types from the way the code uses memory: the scale of an index gives the size of an element, a constant added to a pointer gives a field, a load from the start of an object gives a function table. It ends with C++ objects, which add one hidden field to everything.

## Concepts

### How the listings in this module were made

The C source is `examples/15_data.c` and the C++ source is `examples/15_cpp.cpp`. The `Makefile` compiles them with GCC and G++ at `-O1` and writes Intel-syntax assembly to `build/15_data.s` and `build/15_cpp.s`. The listings are those files with the assembler directives removed. All code is x86-64 System V; the first argument is in `rdi`, the second in `rsi`, and the result in `eax` or `rax`.

```bash
make
```

### Globals, locals and the heap

Where a variable lives decides how the code reaches it.

**A global** has a fixed address. In this example, built without position-independent code, the compiler writes the address into the instruction. With position-independent code the same access is relative to `rip`:

```text
bump:
    mov    eax, DWORD PTR counter[rip]
    add    eax, 1
    mov    DWORD PTR counter[rip], eax
    ret
```

`counter[rip]` is a `rip`-relative reference to the variable `counter`, which lives in the `.data` section because it has an initial value. Uninitialized globals live in `.bss`, constants in `.rodata`. A read-modify-write sequence on the same address (load, add, store) is `counter += 1`.

**A local** lives on the stack or in a register. A local array always has an address, so it lives in memory:

```text
local_sum:
    mov    DWORD PTR [rsp-24], edi
    lea    edx, [rdi+1]
    mov    DWORD PTR [rsp-20], edx
    lea    esi, [rdi+2]
    mov    DWORD PTR [rsp-16], esi
    lea    ecx, [rdi+3]
    mov    DWORD PTR [rsp-12], ecx
    lea    eax, [rdi+rdx]
    add    eax, esi
    add    eax, ecx
    and    edi, 3
    add    eax, DWORD PTR [rsp-24+rdi*4]
    ret
```

The function has no frame pointer and no `sub rsp`: it stores the four elements of `buf` at `[rsp-24]` through `[rsp-12]`, below the stack pointer, in the **red zone** from module 08. The loop over `i` was fully unrolled, so no counter exists; the elements are stored with consecutive offsets 4 bytes apart. The final access `[rsp-24+rdi*4]` is the indexed read `buf[n & 3]`.

**The heap** is memory from the allocator. A call to `malloc` with a constant tells you the size of the object; the code that follows stores into it through the returned pointer:

```text
push:
    push    rbp
    push    rbx
    sub    rsp, 8
    mov    rbx, rdi
    mov    ebp, esi
    mov    edi, 16
    call    malloc
    test    rax, rax
    je    .L30
    mov    DWORD PTR [rax], ebp
    mov    QWORD PTR [rax+8], rbx
.L28:
    add    rsp, 8
    pop    rbx
    pop    rbp
    ret
.L30:
    mov    rax, rbx
    jmp    .L28
```

`mov edi, 16` is `sizeof(struct node)`. The returned pointer is in `rax`. The two stores that follow, `[rax]` (4 bytes) and `[rax+8]` (8 bytes), are the fields `value` and `next`. The `test rax, rax / je` is the check for allocation failure, which returns the old `head`. A `call malloc` (or `operator new` in C++) followed by stores through the result means an object is being initialized, and the argument is its size.

### Arrays and pointer arithmetic

An array element is at `base + index * element_size`. The x86 addressing mode `[base + index*scale]` expresses this directly, and the **scale** is the element size when it is 1, 2, 4 or 8.

```text
get:
    movsx    rdi, edi
    mov    eax, DWORD PTR table[0+rdi*4]
    ret
```

```text
get_local:
    movsx    rsi, esi
    mov    eax, DWORD PTR [rdi+rsi*4]
    ret
```

```text
get_long:
    movsx    rsi, esi
    mov    rax, QWORD PTR [rdi+rsi*8]
    ret
```

```text
get_char:
    movsx    rsi, esi
    movzx    eax, BYTE PTR [rdi+rsi]
    ret
```

All four functions read `p[i]`, and only the element type changes the instruction. `get` accesses the global array `table` at a fixed address with scale 4 (it is an `int` array). `get_local` takes the base in `rdi` and scales by 4. `get_long` scales by 8 and loads 8 bytes into `rax`. `get_char` has no scale (scale 1) and loads one byte with `movzx`. The `movsx rsi, esi` at the start sign-extends the 32-bit index to 64 bits. The size of the load (`BYTE`, `WORD`, `DWORD`, `QWORD`) and the scale together give you the element type.

| Evidence | Meaning |
|---|---|
| `[base+index*4]` with a 4-byte load | An array of `int` (or `float`, `unsigned`) |
| `[base+index*8]` with an 8-byte load | An array of `long`, pointers or `double` |
| `[base+index]` with a byte load | An array of `char` or bytes |
| A base that is a fixed address | A global array; its size appears in the symbol table or the section |

For a **two-dimensional array** or an array of structures whose size is not 1, 2, 4 or 8, the index must be multiplied first:

```text
second_row:
    movsx    rsi, esi
    sal    rsi, 4
    add    rdi, rsi
    movsx    rdx, edx
    mov    eax, DWORD PTR [rdi+rdx*4]
    ret
```

`sal rsi, 4` multiplies the row index by 16: each row of `int m[][4]` is 4 elements of 4 bytes. The row is added to the base, and the column is then scaled by 4. The row size 16 is the width of the inner dimension; the code never mentions the number 4 as a dimension. An array of structures works the same way, with the size of the structure as the stride:

```text
point_array:
    movsx    rsi, esi
    mov    eax, DWORD PTR [rdi+4+rsi*8]
    ret
```

`[rdi+4+rsi*8]` reads field `y` (offset 4) of element `i` of an array of 8-byte `struct point` values. The scale 8 is the size of the whole structure and the displacement 4 is the position of the field inside it. The two constants carry different information.

Pointer subtraction divides by the element size:

```text
pointer_diff:
    sub    rsi, rdi
    sar    rsi, 2
    mov    eax, esi
    ret
```

`sub rsi, rdi` computes the byte distance, and `sar rsi, 2` divides it by 4: the result is a count of `int` elements. A shift after a subtraction of two pointers reveals the element size.

### Structures

A structure is a block of memory in which each field has a fixed offset. Code accesses a field as `[pointer + offset]`; no instruction mentions the field name. You recover the layout by collecting every `[reg + constant]` used with the same base pointer, together with the width of each access.

```text
point_sum:
    mov    eax, DWORD PTR [rdi]
    add    eax, DWORD PTR [rdi+4]
    ret
```

`point_sum` reads offsets 0 and 4, both 4 bytes: a structure with two `int` fields. Compilers place fields at addresses that suit their size, which leaves **padding** between some fields. A larger example:

```text
record_id:
    mov    eax, DWORD PTR [rdi+4]
    ret
```

```text
record_flags:
    movzx    eax, WORD PTR [rdi+8]
    ret
```

```text
record_total:
    mov    rax, QWORD PTR [rdi+16]
    ret
```

```text
record_name:
    movsx    rsi, esi
    movzx    eax, BYTE PTR [rsi+24+rdi]
    ret
```

Collecting what these accessors show for `struct record`:

| Offset | Width | Seen in | Field |
|---|---|---|---|
| 0 | 1 | (not read here) | `char tag` |
| 4 | 4 | `[rdi+4]`, `DWORD` | `int id` |
| 8 | 2 | `[rdi+8]`, `WORD` | `short flags` |
| 16 | 8 | `[rdi+16]`, `QWORD` | `long total` |
| 24 | 1 per element | `[rsi+24+rdi]`, `BYTE` | `char name[6]` |

The gaps are padding. `id` is at 4 and not 1 because a 4-byte field is aligned to 4. `total` is at 16 and not 10 because an 8-byte field is aligned to 8. The structure ends at 30 and is rounded up to a multiple of 8: its size is 32. The compiler's own report agrees (`offsetof` and `sizeof` for this structure give `tag 0 id 4 flags 8 total 16 name 24 size 32`). The total size is visible elsewhere in the program: as the argument to `malloc`, as the stride of an array of such structures, or as the amount subtracted from `rsp` for a local.

Rules for reading a structure:

- Every constant added to the same base pointer is a field offset.
- The load or store width, and the choice of `movzx` or `movsx`, give the field type.
- A gap between offsets is padding or a field that this code does not use.
- An access with an index register at `[base + const + idx*scale]` is an array inside the structure (`name`) when the scale is the element size.
- Two functions that use different offsets from the same pointer are looking at the same object; merge their fields.

### Linked lists

A list node holds a value and a pointer to the next node. The traversal is a loop that follows the pointer until it is null.

```text
list_sum:
    test    rdi, rdi
    je    .L16
    mov    eax, 0
.L15:
    add    eax, DWORD PTR [rdi]
    mov    rdi, QWORD PTR [rdi+8]
    test    rdi, rdi
    jne    .L15
    ret
.L16:
    mov    eax, 0
    ret
```

The loop adds `[rdi]` (a 4-byte `value`) to the sum and replaces `rdi` with `[rdi+8]`, the 8-byte `next` pointer. It repeats while `rdi` is not zero. The signature of a linked list is a loop in which the register used as the base is **reloaded from memory it points into**: `mov rdi, [rdi+8]`. The structure is `{ int value; struct node *next; }`, with 4 bytes of padding after `value`. The `test rdi, rdi / je` before the loop is the check for an empty list.

### Trees

A binary tree node has two child pointers. A search follows one of them depending on a comparison.

```text
tree_has:
    test    rdi, rdi
    jne    .L22
    mov    eax, 0
    ret
.L20:
    mov    rdi, QWORD PTR [rdi+16]
.L21:
    test    rdi, rdi
    je    .L26
.L22:
    mov    eax, DWORD PTR [rdi]
    cmp    eax, esi
    je    .L24
    jle    .L20
    mov    rdi, QWORD PTR [rdi+8]
    jmp    .L21
.L26:
    mov    eax, 0
    ret
.L24:
    mov    eax, 1
    ret
```

The node pointer is in `rdi`. The node holds `key` at offset 0, and two pointers at offsets 8 and 16. The code compares `[rdi]` with the searched key `esi`: equal means found; `jle` (the node key is below or equal to the searched key, so the search key is greater) loads `[rdi+16]`, the right child; otherwise it loads `[rdi+8]`, the left child. Two pointer fields loaded alternatively, selected by a comparison with the key, are a binary search tree. The left child is the one taken when the searched key is smaller than the node's key. Reading which offset is taken on which side of the comparison tells you which pointer is `left` and which is `right`.

### C++ objects

A C++ object with virtual functions begins with a hidden field: the **virtual table pointer** (vptr), which points to a table of function addresses, the **vtable**. The class's data fields follow, base class first.

The example `examples/15_cpp.cpp` has three classes: `Shape` with virtual `area` and `sides`, `Rect` derived from `Shape`, and `Square` derived from `Rect`. The constructors are inlined into the function that creates the objects:

```text
_Z16make_and_measurev:
    push    rbx
    sub    rsp, 64
    mov    DWORD PTR [rsp+40], 1
    mov    QWORD PTR [rsp+32], OFFSET FLAT:_ZTV4Rect+16
    mov    DWORD PTR [rsp+44], 3
    mov    DWORD PTR [rsp+48], 4
    mov    DWORD PTR [rsp+8], 2
    mov    DWORD PTR [rsp+12], 5
    mov    DWORD PTR [rsp+16], 5
    mov    QWORD PTR [rsp], OFFSET FLAT:_ZTV6Square+16
    lea    rdi, [rsp+32]
    call    _Z10total_areaPK5Shape
    mov    ebx, eax
    mov    rdi, rsp
    call    _Z10total_areaPK5Shape
    add    eax, ebx
    add    rsp, 64
    pop    rbx
    ret
```

Look at the object at `[rsp+32]`. It starts with a store of an address, `OFFSET _ZTV4Rect+16`, at offset 0: this is the vptr, pointing 16 bytes into the vtable of `Rect`. The next stores are `1` at offset 8, `3` at offset 12 and `4` at offset 16. The layout of `Rect` is therefore:

| Offset | Field |
|---|---|
| 0 | vptr, hidden |
| 8 | `int id`, from `Shape` |
| 12 | `int w` |
| 16 | `int h` |
| 20 | padding to a multiple of 8; the size is 24 |

The size 24 is confirmed by the deleting destructor, which passes it to `operator delete`:

```text
_ZN4RectD0Ev:
    sub    rsp, 8
    mov    esi, 24
    call    _ZdlPvm
    add    rsp, 8
    ret
```

The second object, at `[rsp]`, is a `Square`: the same layout, with the vptr pointing into the vtable of `Square` and the width and height both 5.

The vtable itself is data, in `.rodata`. Using the symbols and `c++filt` to demangle names, the two tables are:

```text
vtable for Rect (_ZTV4Rect)             vtable for Square (_ZTV6Square)
    0                                       0
    0                                       0
    Rect::~Rect()      (complete)           Square::~Square()  (complete)
    Rect::~Rect()      (deleting)           Square::~Square()  (deleting)
    Rect::area() const                      Rect::area() const
    Rect::sides() const                     Square::sides() const
```

Every vtable begins with two words, the offset to the top of the object and a pointer to type information (zero here, because the example was built without run-time type information, which a real binary usually has: the pointer then leads to a structure that names the class). The vptr points just past them, so the first entry that the vptr addresses is the first virtual function: slot 0 is the complete destructor, slot 1 the deleting destructor, slot 2 `area`, slot 3 `sides`. **`Square` inherits `area` from `Rect`**, so slot 2 holds the same address in both tables; **`Square` overrides `sides`**, so slot 3 differs.

A virtual call loads the vptr from the object, loads a function address from the table, and calls it with the object pointer as the first argument:

```text
_Z10total_areaPK5Shape:
    push    rbp
    push    rbx
    sub    rsp, 8
    mov    rbx, rdi
    mov    rax, QWORD PTR [rdi]
    call    [QWORD PTR [rax+16]]
    mov    ebp, eax
    mov    rax, QWORD PTR [rbx]
    mov    rdi, rbx
    call    [QWORD PTR [rax+24]]
    add    eax, ebp
    add    rsp, 8
    pop    rbx
    pop    rbp
    ret
```

`mov rax, [rdi]` loads the vptr, and `call [rax+16]` calls the function in slot 2 (offset 16 is 2 slots of 8 bytes): `area`. The second call, `call [rax+24]`, is slot 3: `sides`. The object pointer `this` stays in `rdi` (it is saved in `rbx` across the first call and put back). Which concrete function runs depends on the vtable the object has. A call through a register that was loaded from `[object]` plus a small constant is a virtual call; the constant divided by 8 is the slot number.

And a non-virtual member function is an ordinary call with `this` as the first argument:

```text
_ZNK4Rect4areaEv:
    mov    eax, DWORD PTR [rdi+12]
    imul    eax, DWORD PTR [rdi+16]
    ret
```

`Rect::area` reads `w` at offset 12 and `h` at offset 16 from `this` in `rdi` and multiplies them. Nothing in the code says it is a member function except the mangled name and the offsets from the pointer.

| Evidence | Meaning |
|---|---|
| A store of a code-section address into `[obj]` at construction | The vptr; the address is a vtable plus 16 |
| `mov rax, [rdi]` then `call [rax+N]` | A virtual call to slot `N / 8` |
| A table of function addresses in `.rodata`, preceded by two words | A vtable |
| Two tables that share most entries | A base class and a derived class; the differences are the overrides |
| The first argument `rdi` used as a base in every function of a group | A set of member functions of one class |

### Building types in Ghidra

Once you know a layout, record it so that the decompiler can use it.

1. Open the Data Type Manager and create a **structure**. Add the fields with their offsets, sizes and names. Use "undefined" padding fields to reach the right offsets.
2. In the decompiler, select a variable that holds a pointer to the object and retype it (default key `Ctrl+L`) to a pointer to your structure. The decompiler rewrites `*(int *)(p + 4)` as `p->id`.
3. For a class with virtual functions, create a structure for the vtable whose fields are function pointers, then make the first field of the object a pointer to it. The decompiler then shows virtual calls as `obj->vtable->area(obj)`.
4. Rename the functions as you learn what they do, and set their first parameter to the class pointer.

Types propagate: once one function uses the structure, every function that receives the same pointer becomes more readable.

## Try it

1. Run `make` and open `build/15_data.s`. For `record_name`, work out what the displacement 24 and the lack of a scale mean.
2. Use `readelf -S -W build/15_data.o` and `nm build/15_data.o` to find which section holds `table` and `counter`, and how large `table` is.
3. Add a field `char pad` after `flags` in `struct record` and predict which offsets change before you rebuild. Then rebuild and check.
4. Write a C function that returns the right-most key of a `struct tree` and read the loop in the compiled output.
5. Run `nm -C build/15_cpp.o` and `objdump -d -M intel -C build/15_cpp.o`. Find `Rect::area() const` and the two tables. Add a new virtual function to `Shape`, rebuild, and see which slot numbers change.

## Common pitfalls

- **Reading padding as a field.** A gap between offsets may be alignment, or a field this function does not use. Wait for another function to confirm.
- **Confusing the scale with the structure size.** A scale of 8 may be an array of 8-byte structures or an array of `long`. The displacement and the width of the access tell the difference.
- **Missing the hidden vptr.** In a C++ class with virtual functions, offset 0 is the vptr and the first member is at offset 8.
- **Forgetting that `this` is the first argument.** In a member function, `rdi` is the object pointer, and the visible arguments start at `rsi`.
- **Treating every pointer chase as a list.** A loop that reloads its base register from the object is a list, but one that reloads from `[rdi+8]` or `[rdi+16]` depending on a comparison is a tree.

## Summary

- The scale of an index and the width of a load give the type of an array element; a constant added to a pointer is a field.
- A structure is a set of offsets and widths collected from every access, with padding between them; its size is visible in allocations, strides and stack space.
- A linked list is a loop that reloads its base register from the node it points into; a tree is the same with two pointers selected by a comparison.
- A C++ object with virtual functions starts with a vptr; a virtual call is `mov rax, [obj]` then `call [rax+8*slot]`.
- Writing the recovered layout down as a type makes the rest of the analysis faster.

## Next steps

- Take the [quiz](quiz.md).
- Continue with module 16.
- Cheatsheets: [Compiler idioms](../../cheatsheets/compiler-idioms.md).
- Exercises that use this module: [Exercise 14](../../exercises/14-shape-shifter/README.md), [Exercise 15](../../exercises/15-linked-inventory/README.md), [Exercise 20](../../exercises/20-capstone-tiny-vm/README.md).
