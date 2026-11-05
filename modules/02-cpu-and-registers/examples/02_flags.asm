; Purpose : show how arithmetic sets the carry, zero, sign and overflow flags.
; Target  : Linux x86-64, ELF, no libc
; Build   : nasm -f elf64 -g -F dwarf 02_flags.asm -o 02_flags.o && ld 02_flags.o -o 02_flags
; Run     : gdb -q ./02_flags   (step and inspect eflags)

bits 64
default rel

section .text
global _start

_start:
    mov     al, 255
    add     al, 1                       ; 255 + 1 = 256: wraps to 0. CF=1 ZF=1 SF=0 OF=0

    mov     al, 127
    add     al, 1                       ; 127 + 1 = 128: as signed that is -128. CF=0 ZF=0 SF=1 OF=1

    mov     al, 5
    sub     al, 5                       ; 5 - 5 = 0. ZF=1

    mov     al, 3
    cmp     al, 5                       ; computes 3 - 5, discards the result. CF=1 SF=1

    mov     eax, 60                     ; exit
    xor     edi, edi
    syscall
