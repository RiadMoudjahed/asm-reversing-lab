[English](quiz.md) | [العربية](quiz.ar.md) | [Deutsch](quiz.de.md) | [Español](quiz.es.md) | [Français](quiz.fr.md)

# Quiz: 15. Recovering data

Answer first, then open the answer.

**1. The code has `mov eax, DWORD PTR [rdi+rsi*4]`. What do you know about `rdi` and `rsi`?**

<details>
<summary>Answer</summary>

`rdi` is the base of an array of 4-byte elements, probably `int`, and `rsi` is the index (here as a 64-bit value). The scale 4 and the 4-byte load agree on the element size.

</details>

**2. What does `[rdi+4+rsi*8]` tell you that `[rdi+rsi*8]` would not?**

<details>
<summary>Answer</summary>

The stride 8 is the size of an element of an array of structures, and the 4 is the offset of one field inside each element. The code reads the field at offset 4 of element `rsi`.

</details>

**3. A function reads offsets 0 (4 bytes), 8 (8 bytes) and 16 (1 byte) from `rdi`. What can you say about the layout, and about offsets 4 and 9 to 15?**

<details>
<summary>Answer</summary>

It is a structure with at least an int-sized field at 0, an 8-byte field at 8 and a byte at 16. Offsets 4 to 7 are probably padding because an 8-byte field is aligned to 8, and 9 to 15 are padding or fields this function does not use. The total size is probably 24, rounded up to a multiple of 8.

</details>

**4. A loop does `mov rdi, [rdi+8]` and tests `rdi` against zero. What data structure is this?**

<details>
<summary>Answer</summary>

A linked list: the base register is reloaded from the node it points into, and null ends the loop. The pointer to the next node is at offset 8.

</details>

**5. How do you tell a binary tree search from a list traversal in the machine code?**

<details>
<summary>Answer</summary>

A tree search loads one of two pointer fields (for example at offsets 8 and 16), chosen by a comparison of the searched key with the node's key. A list traversal always loads the same field.

</details>

**6. What is the first field of a C++ object that has virtual functions, and how does a virtual call use it?**

<details>
<summary>Answer</summary>

The vptr, at offset 0. A virtual call loads the vptr from the object (`mov rax, [rdi]`), then calls an address stored in the vtable at a fixed slot (`call [rax+8*slot]`), passing the object pointer as the first argument.

</details>

**7. Two classes have vtables that differ only in one slot. What does that tell you?**

<details>
<summary>Answer</summary>

One class is derived from the other, and the differing slot is a virtual function that the derived class overrides. All other slots hold the same function addresses because they are inherited.

</details>
