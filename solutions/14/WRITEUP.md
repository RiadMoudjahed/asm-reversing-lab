[English](WRITEUP.md) | [العربية](WRITEUP.ar.md) | [Deutsch](WRITEUP.de.md) | [Español](WRITEUP.es.md) | [Français](WRITEUP.fr.md)

# Write-up: exercise 14. Shape shifter

The flag is `ARL{shape_4_3_24_16_3_9}`: there are 4 classes, each vtable has 3 functions, the largest objects are 24 bytes with their last field at offset 16, three pairs of classes share a function in the same slot, and the four vtables together hold 9 different function addresses.

## Part 1: Reverse

### The factory and the objects

The function that handles `kind` calls `operator new` once per class and stores the vtable pointer and the fields. The four code paths, shown with the `new` argument and the stores that follow it:

```text
mov    edi,0x10                       ; new(16)
call   _Znwm@plt
mov    QWORD PTR [rax],0x402030       ; vptr
mov    DWORD PTR [rax+0x8],ebx        ; first field

mov    edi,0x10                       ; new(16)
call   _Znwm@plt
mov    DWORD PTR [rax+0x8],ebx
mov    QWORD PTR [rax],0x402058       ; vptr
mov    DWORD PTR [rax+0xc],ebp        ; second field

mov    edi,0x18                       ; new(24)
call   _Znwm@plt
mov    DWORD PTR [rax+0x8],ebx
mov    DWORD PTR [rax+0xc],ebp
mov    QWORD PTR [rax],0x402080       ; vptr
mov    DWORD PTR [rax+0x10],r12d      ; third field

mov    edi,0x10                       ; new(16)
call   _Znwm@plt
mov    DWORD PTR [rax+0x8],ebx
mov    QWORD PTR [rax],0x4020a8       ; vptr
mov    DWORD PTR [rax+0xc],ebp        ; second field
```

Four different vtable pointers are stored, so `N` is 4. Three classes have 16-byte objects and one has a 24-byte object, so `Z` is 24. The 24-byte object has fields at offsets 8, 12 and 16, and the last one is at 16, so `P` is 16. Every object starts with the vtable pointer at offset 0, and the first field is at offset 8, which leaves 4 bytes of padding in the 16-byte objects after offset 12.

The classes can be named from what they share. The class with the three fields is the most derived; the class whose vtable shares the first function with it is its parent (it has two fields); the root is the class with only one field; and the remaining class has two fields and derives from the root.

### The vtables

Each stored pointer points 16 bytes into a table in `.rodata`. The two words before it are the offset to the top of the object and the missing run-time type information, both zero. The three words from the pointer on are the virtual functions:

```text
vtable pointer   slot 0 (offset 0)   slot 1 (offset 8)   slot 2 (offset 16)
0x402030         0x401272            0x40127a            0x401280
0x402058         0x401286            0x40127a            0x401290
0x402080         0x401286            0x40129a            0x4012ac
0x4020a8         0x4012ba            0x4012c2            0x401280
```

In the main loop, three virtual calls use `[rax+0x10]`, `[rax+0x8]` and `[rax]` from the vtable pointer, so there are three slots and `S` is 3.

Comparing the rows slot by slot:

| Slot | Equal addresses | Between which tables |
|---|---|---|
| 0 | 0x401286 | the second and the third |
| 1 | 0x40127a | the first and the second |
| 2 | 0x401280 | the first and the fourth |

That is three pairs of classes with an equal address in the same slot, so `E` is 3. The nine different addresses are 0x401272, 0x40127a, 0x401280, 0x401286, 0x401290, 0x40129a, 0x4012ac, 0x4012ba and 0x4012c2, so `F` is 9.

The pattern of sharing gives the hierarchy. The first table is the base class: its second slot (0x40127a) and third slot (0x401280) are inherited by others. The second table shares slot 1 with the base and overrides slots 0 and 2. The third table shares slot 0 with the second and overrides the other two, which makes it a class derived from the second. The fourth table shares slot 2 with the base and overrides slots 0 and 1. The classes that belong to the objects are therefore: root (16 bytes), derived with one added field (16 bytes), derived again with another field (24 bytes), and a second class derived from the root (16 bytes).

The flag is `ARL{shape_4_3_24_16_3_9}`.

## Part 2: Assemble

The reference solution is `solution.asm`. The points that matter:

- **The layout.** Offset 0 holds the vtable pointer. Offset 8 holds the first field for every class. Offset 12 holds the second field, which exists for three classes. Offset 16 holds the third field, which only the largest class has.
- **The vtable pointer.** `gadget_init` calls `gadget_vptr(kind)` and stores the result in the first word. The kinds are the values that the factory function of `bin/gadgets` switches on, and `gadget_vptr` returns the matching class's pointer, which is the address 16 bytes into its table.
- **Fields.** The arguments `a`, `b` and `c` go to offsets 8, 12 and 16, and a class gets only the fields it has. Because the call to `gadget_vptr` destroys the argument registers, `gadget_init` keeps them in callee-saved registers and pushes those registers first.
- **The virtual call.** `gadget_call` loads the vtable pointer with `mov rax, [rdi]`, scales the slot number by 8, moves `arg` into `esi`, and calls `[rax + rcx*8]`. The object pointer is still in `rdi`, which is `this` for the callee. The stack is aligned with a single `sub rsp, 8` because on entry `rsp` is 8 modulo 16.
- **Registers.** Only callee-saved registers that were saved are modified, and they are restored in reverse order.

## Mistakes that cost time

- **Storing a field at the wrong offset.** The library's virtual calls read other values than the ones the binary computes, and the test results differ.
- **The vtable pointer in the wrong place.** The checker reports `vptr` before it makes any call, because a call through a wrong pointer would crash.
- **Scaling the slot by 4.** The call goes to an address inside a function or to garbage, and the program ends with signal 11.
- **Not moving `arg` into `esi`.** The callee receives whatever `esi` held, and the checker reports `call`.
- **Destroying a callee-saved register.** The results are right and the checker reports `regs`.
- **Writing past the object.** A store at offset 40, outside any of the objects, is reported as `canary`.
