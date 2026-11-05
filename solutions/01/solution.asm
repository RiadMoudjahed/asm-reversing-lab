; Purpose : reference solution for exercise 01, Part 2. Reproduces the behavior
;           of signal_flare using only the write and exit system calls.
; Target  : Linux x86-64, ELF, no libc
; Build   : nasm -f elf64 solution.asm -o solution.o && ld solution.o -o solution
; Output  : three lines on stdout, one line on stderr, exit status 7

bits 64
default rel

SYS_WRITE   equ 1
SYS_EXIT    equ 60
STDOUT      equ 1
STDERR      equ 2

section .rodata
banner:     db "FLARE LAUNCHED", 10
            db "COLOR: RED", 10
            db "ALTITUDE: 120 m", 10
banner_len  equ $ - banner

burn:       db "[flare] burn time 30 s", 10
burn_len    equ $ - burn

section .text
global _start

_start:
    mov     eax, SYS_WRITE      ; write(STDOUT, banner, banner_len)
    mov     edi, STDOUT
    lea     rsi, [banner]
    mov     edx, banner_len
    syscall

    mov     eax, SYS_WRITE      ; write(STDERR, burn, burn_len)
    mov     edi, STDERR
    lea     rsi, [burn]
    mov     edx, burn_len
    syscall

    mov     eax, SYS_EXIT       ; exit(7)
    mov     edi, 7
    syscall
