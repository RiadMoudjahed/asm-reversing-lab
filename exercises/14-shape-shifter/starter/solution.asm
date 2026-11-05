; Exercise 14, Part 2: build objects of the recovered classes by hand, and call their virtual
; functions through the vtable.
; Target  : Linux x86-64, System V AMD64
; Build   : nasm -f elf64 solution.asm -o solution.o   (or: make check EX=14)
;
;   void gadget_init(void *mem, int kind, int a, int b, int c);
;   int  gadget_call(const void *obj, int slot, int arg);
;
; gadget_vptr(kind) is provided by the tests and returns the value that the first word of an
; object of class `kind` must hold:  const void *gadget_vptr(int kind);

bits 64
default rel

extern gadget_vptr
global gadget_init
global gadget_call

section .text

gadget_init:
    ; TODO
    ret

gadget_call:
    ; TODO
    xor     eax, eax
    ret

section .note.GNU-stack noalloc noexec nowrite progbits
