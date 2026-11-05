; Exercise 17, Part 2: read the archives that arkpack writes.
; Target  : Linux x86-64, System V AMD64
; Build   : nasm -f elf64 solution.asm -o solution.o   (or: make check EX=17)
;
;   long ark_count(const unsigned char *a, size_t n);
;   long ark_name(const unsigned char *a, size_t n, long index, char *out, size_t cap);
;   long ark_extract(const unsigned char *a, size_t n, long index, unsigned char *out, size_t cap);
;
; The archive is the n bytes at a. The README lists the return values. Nothing may be read
; beyond the n bytes, whatever the header says.

bits 64
default rel

global ark_count
global ark_name
global ark_extract

section .text

ark_count:
    ; TODO
    mov     rax, -1
    ret

ark_name:
    ; TODO
    mov     rax, -1
    ret

ark_extract:
    ; TODO
    mov     rax, -1
    ret

section .note.GNU-stack noalloc noexec nowrite progbits
