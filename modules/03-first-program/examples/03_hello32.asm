; Purpose : print a line of text and exit, using 32-bit Linux system calls.
; Target  : Linux x86 (32-bit), ELF, no libc
; Build   : nasm -f elf32 03_hello32.asm -o 03_hello32.o && ld -m elf_i386 03_hello32.o -o 03_hello32
; Run     : ./03_hello32
; Output  : Hello, assembly!

bits 32

SYS_EXIT    equ 1               ; system call numbers (32-bit table)
SYS_WRITE   equ 4
STDOUT      equ 1

section .rodata
message:    db "Hello, assembly!", 10
message_len equ $ - message

section .text
global _start

_start:
    mov     eax, SYS_WRITE      ; eax = call number
    mov     ebx, STDOUT         ; ebx = argument 1: file descriptor
    mov     ecx, message        ; ecx = argument 2: address of the text
    mov     edx, message_len    ; edx = argument 3: number of bytes
    int     0x80                ; enter the kernel: write(1, message, len)

    mov     eax, SYS_EXIT       ; eax = call number
    xor     ebx, ebx            ; ebx = argument 1: exit status 0
    int     0x80                ; exit(0)
