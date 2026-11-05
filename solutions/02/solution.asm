; Purpose : reference solution for exercise 02, Part 2. The 64-bit port of the
;           legacy program: same text, same three calls, same exit status.
; Target  : Linux x86-64, ELF, no libc
; Build   : nasm -f elf64 solution.asm -o solution.o && ld solution.o -o solution
; Output  : "legacy ready" on stdout, exit status 5

bits 64
default rel

SYS_WRITE   equ 1
SYS_SYNC    equ 162
SYS_EXIT    equ 60

section .rodata
message:    db "legacy ready", 10
message_len equ $ - message

section .text
global _start

_start:
    mov     eax, SYS_WRITE      ; write(1, message, message_len)
    mov     edi, 1
    lea     rsi, [message]
    mov     edx, message_len
    syscall

    mov     eax, SYS_SYNC       ; sync()
    syscall

    mov     eax, SYS_EXIT       ; exit(5)
    mov     edi, 5
    syscall
