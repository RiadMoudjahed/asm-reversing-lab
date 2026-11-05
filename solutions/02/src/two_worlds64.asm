; Purpose : challenge program for exercise 02 (64-bit half). Same behavior as
;           the 32-bit program, written for the 64-bit system call interface.
; Target  : Linux x86-64, ELF, no libc
; Build   : make OUTDIR=<dir>

bits 64
default rel

section .rodata
text:       db "two worlds", 10
text_len    equ $ - text

section .text
global _start

_start:
    mov     eax, 1              ; write
    mov     edi, 1
    lea     rsi, [text]
    mov     edx, text_len
    syscall

    mov     eax, 162            ; sync
    syscall

    mov     eax, 60             ; exit
    xor     edi, edi
    syscall
