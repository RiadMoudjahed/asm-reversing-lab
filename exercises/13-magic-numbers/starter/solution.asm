; Exercise 13, Part 2: the same function as bin/magic, without div or idiv.
; Target  : Linux x86-64, System V AMD64
; Build   : nasm -f elf64 solution.asm -o solution.o   (or: make check EX=13)
;
;   int32_t mix_asm(int32_t x);          // x in edi, result in eax
;
; mix_asm must return exactly what mix returns in bin/magic, for every 32-bit input. Use
; multiplication by constants and shifts wherever the original divides.

bits 64
default rel

global mix_asm

section .text

mix_asm:
    ; TODO
    xor     eax, eax
    ret

section .note.GNU-stack noalloc noexec nowrite progbits
