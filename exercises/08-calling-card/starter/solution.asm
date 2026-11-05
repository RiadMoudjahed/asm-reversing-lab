; Exercise 08, Part 2: re-implement three functions from Part 1.
; Target  : Linux x86 (32-bit), ELF
; Build   : nasm -f elf32 solution.asm -o solution.o   (or: make check EX=08)
;
; cc_f1, cc_f2 and cc_f3 must behave like f1, f2 and f3 of bin/callcard32, and use the
; same calling convention as those functions. The tests call them through C prototypes
; written for those conventions, and they check that the stack pointer is the same
; before and after each call.
;
;   int cc_f1(int a, int b, int c);
;   int cc_f2(int a, int b, int c);
;   int cc_f3(int a, int b, int c);
;
; Registers ebx, esi, edi and ebp must be preserved.

bits 32

section .text
global cc_f1, cc_f2, cc_f3

cc_f1:
    ; TODO
    xor     eax, eax
    ret

cc_f2:
    ; TODO
    xor     eax, eax
    ret

cc_f3:
    ; TODO
    xor     eax, eax
    ret

section .note.GNU-stack noalloc noexec nowrite progbits
