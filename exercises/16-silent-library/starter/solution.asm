; Exercise 16, Part 2: use the library you recovered, and replace one of its functions.
; Target  : Linux x86-64, System V AMD64
; Build   : nasm -f elf64 solution.asm -o solution.o   (or: make check EX=16)
;
;   unsigned digest_buffer(const unsigned char *p, size_t n, unsigned seed);
;   int      pack_values(const int *vals, size_t n, unsigned char *out, size_t cap);
;
; digest_buffer must compute the digest of the n bytes at p by calling the library (the
; functions you recovered for creating a state, feeding bytes, reading the result and releasing
; the state), and return it. pack_values must behave exactly like the library's packing
; function, without calling it.

bits 64
default rel

; extern <the library functions you need>

global digest_buffer
global pack_values

section .text

digest_buffer:
    ; TODO
    xor     eax, eax
    ret

pack_values:
    ; TODO
    xor     eax, eax
    ret

section .note.GNU-stack noalloc noexec nowrite progbits
