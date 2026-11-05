; Exercise 19, Part 2: decode one record of the simulator's configuration.
; Target  : Linux x86-64, System V AMD64
; Build   : nasm -f elf64 solution.asm -o solution.o   (or: make check EX=19)
;
;   long decode_record(const unsigned char *rec, size_t avail, char *out, size_t cap);
;
; rec points at one encoded record, as it is stored in the sample; avail is the number of bytes
; you may read from rec. The README gives the return values. Never read beyond avail bytes or
; write beyond cap bytes.

bits 64
default rel

global decode_record

section .text

decode_record:
    ; TODO
    mov     rax, -1
    ret

section .note.GNU-stack noalloc noexec nowrite progbits
