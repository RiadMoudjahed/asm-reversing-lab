; Purpose : print a line of text and exit, using 64-bit Linux system calls.
; Target  : Linux x86-64, ELF, no libc
; Build   : nasm -f elf64 03_hello64.asm -o 03_hello64.o && ld 03_hello64.o -o 03_hello64
; Run     : ./03_hello64
; Output  : Hello, assembly!

bits 64
default rel

SYS_WRITE   equ 1               ; system call numbers (64-bit table)
SYS_EXIT    equ 60
STDOUT      equ 1               ; file descriptor of standard output

section .rodata
message:    db "Hello, assembly!", 10       ; 10 is the newline byte
message_len equ $ - message                 ; bytes from 'message' to here

section .text
global _start                   ; make the entry point visible to the linker

_start:
    mov     eax, SYS_WRITE      ; rax = call number
    mov     edi, STDOUT         ; rdi = argument 1: file descriptor
    lea     rsi, [message]      ; rsi = argument 2: address of the text
    mov     edx, message_len    ; rdx = argument 3: number of bytes
    syscall                     ; enter the kernel: write(1, message, len)

    mov     eax, SYS_EXIT       ; rax = call number
    xor     edi, edi            ; rdi = argument 1: exit status 0
    syscall                     ; exit(0)
