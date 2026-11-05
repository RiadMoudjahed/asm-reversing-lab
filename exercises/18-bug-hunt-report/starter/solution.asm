; Exercise 18, Part 2: the record parser, written without the bugs.
; Target  : Linux x86-64, System V AMD64
; Build   : nasm -f elf64 solution.asm -o solution.o   (or: make check EX=18)
;
;   long parse_records(const unsigned char *buf, size_t n, struct out *o);
;
; The README gives the record types, the limits, the layout of struct out and the error codes.
; The buffer is the n bytes at buf; never read outside it.

bits 64
default rel

global parse_records

section .text

parse_records:
    ; TODO
    mov     rax, -1
    ret

section .note.GNU-stack noalloc noexec nowrite progbits
