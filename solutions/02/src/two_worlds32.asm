; Purpose : challenge program for exercise 02 (32-bit half). Prints a line,
;           flushes file system buffers, and exits.
; Target  : Linux x86 (32-bit), ELF, no libc
; Build   : make OUTDIR=<dir>

bits 32

section .rodata
text:       db "two worlds", 10
text_len    equ $ - text

section .text
global _start

_start:
    mov     eax, 4              ; write
    mov     ebx, 1
    mov     ecx, text
    mov     edx, text_len
    int     0x80

    mov     eax, 36             ; sync
    int     0x80

    mov     eax, 1              ; exit
    xor     ebx, ebx
    int     0x80
