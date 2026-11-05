; Exercise 07, Part 2: write four memory and string routines with the string instructions.
; Target  : Linux x86-64, ELF, System V calling convention
; Build   : nasm -f elf64 solution.asm -o solution.o   (or: make check EX=07)
;
; Arguments arrive in rdi, rsi, rdx; the result goes back in rax.
; The direction flag is clear on entry and must be clear when you return.

bits 64
default rel

section .text
global my_strlen, my_memchr, my_memcpy, my_memmove

; size_t my_strlen(const char *s)                          bytes before the first zero
my_strlen:
    ; TODO
    xor     eax, eax
    ret

; void *my_memchr(const void *s, int c, size_t n)          first byte equal to (unsigned char) c,
;                                                          or NULL when it is not among the n bytes
my_memchr:
    ; TODO
    xor     eax, eax
    ret

; void *my_memcpy(void *d, const void *s, size_t n)        copy n bytes (no overlap), return d
my_memcpy:
    ; TODO
    mov     rax, rdi
    ret

; void *my_memmove(void *d, const void *s, size_t n)      copy n bytes, overlap allowed, return d
my_memmove:
    ; TODO
    mov     rax, rdi
    ret

section .note.GNU-stack noalloc noexec nowrite progbits
