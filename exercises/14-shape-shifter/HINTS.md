[English](HINTS.md) | [العربية](HINTS.ar.md) | [Deutsch](HINTS.de.md) | [Español](HINTS.es.md) | [Français](HINTS.fr.md)

# Hints: exercise 14

Open one level at a time. Try again between levels.

## Part 1: Reverse

<details>
<summary>Level 1: nudge</summary>

Find the call to `operator new` in the function that handles each kind. Right after each call, the code stores a constant address into the first word of the new object: that address is a vtable pointer, and each class has its own.

</details>

<details>
<summary>Level 2: direction</summary>

The argument of `operator new` is the size of the object. The other stores after the call are the fields, each at an offset from the new object. The vtable pointer points 16 bytes into a table in `.rodata`; read three 8-byte addresses starting at the pointer. In the main loop, the three virtual calls use offsets 0, 8 and 16 from the vtable pointer. The compiler evaluates the arguments of the print call from right to left, so the calls appear in the order 16, 8, 0 while the program prints the results of the slots 0, 1 and 2.

</details>

<details>
<summary>Level 3: nearly the answer</summary>

Write the four tables side by side, three addresses each, and mark the equal ones. Two classes share an entry when it is the same address in the same row. The number of different addresses is the number of different entries in the whole table. The class with the largest objects is the one whose `operator new` argument is the largest, and its last field is the store with the highest offset in that constructor code. Remember that every object starts with the vtable pointer, so the first field is at offset 8.

</details>

## Part 2: Assemble

<details>
<summary>Level 1: nudge</summary>

`gadget_init` needs the vtable pointer, so it calls `gadget_vptr(kind)`; keep its other arguments in callee-saved registers across that call. Then store the fields at the offsets you found in Part 1.

</details>

<details>
<summary>Level 2: direction</summary>

The constructors you read in Part 1 show which classes store a value at offset 12 and which store one at offset 16: three classes have a second field and one class also has a third. For `gadget_call`, load the vtable pointer from the first word of the object, add 8 times the slot number, and call the address stored there. The object stays in `rdi`; move `arg` into `esi` first.

</details>

<details>
<summary>Level 3: nearly the answer</summary>

Keep the stack aligned: on entry to a function `rsp` is 8 modulo 16, so `gadget_call` must subtract 8 (or push one register) before its `call`, and `gadget_init` must push an odd number of registers. Sign-extend the slot number before you use it as an index. If the checker prints `vptr`, the first word of the object is wrong; `call` means your call does not agree with the library's virtual call (wrong slot, wrong scale or wrong argument); `canary` means you wrote too far; `regs` means a callee-saved register was changed.

</details>
