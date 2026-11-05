; Purpose : challenge program for exercise 03. Runs a short arithmetic routine
;           with four marked points, then prints a closing message.
; Target  : Linux x86-64, ELF, no libc
; Build   : make OUTDIR=<dir>

bits 64
default rel

section .rodata
closing:    db "ledger closed", 10
closing_len equ $ - closing

section .text
global _start
global mark1, mark2, mark3, mark4

_start:
    mov     rax, 1000
    imul    rax, rax, 37

mark1:
    sub     rax, 40000

mark2:
    mov     ecx, 0xFFFFFFFF
    add     ecx, 1

mark3:
    mov     rdx, 0x7FFFFFFFFFFFFFFF
    add     rdx, 1

mark4:
    mov     eax, 1
    mov     edi, 1
    lea     rsi, [closing]
    mov     edx, closing_len
    syscall
    mov     eax, 60
    xor     edi, edi
    syscall
