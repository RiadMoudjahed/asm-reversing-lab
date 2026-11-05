; Exercise 02, Part 2: port this 32-bit program to 64-bit.
; It was written for 32-bit Linux. Change it so that it assembles as a 64-bit
; program and makes the same three system calls with the same results.
;
; Build   : make
; Run     : make run

bits 32

section .rodata
message:    db "legacy ready", 10
message_len equ $ - message

section .text
global _start

_start:
    mov     eax, 4              ; write
    mov     ebx, 1
    mov     ecx, message
    mov     edx, message_len
    int     0x80

    mov     eax, 36             ; sync
    int     0x80

    mov     eax, 1              ; exit
    mov     ebx, 5
    int     0x80
