; Purpose : read a length-prefixed string with lodsb and print it. The first byte of
;           the string holds its length, so no terminator is needed.
; Target  : Linux x86-64, ELF, no libc
; Build   : nasm -f elf64 07_prefixed.asm -o 07_prefixed.o && ld 07_prefixed.o -o 07_prefixed
; Run     : ./07_prefixed
; Output  : Hello

bits 64
default rel

section .rodata
message:    db 5, "Hello"               ; one length byte, then five characters
newline:    db 10

section .text
global _start

_start:
    lea     rsi, [message]
    lodsb                               ; al = [rsi] = the length, and rsi now points at the text
    movzx   edx, al                     ; edx = number of characters to write
    mov     eax, 1                      ; write(1, text, length)
    mov     edi, 1                      ; (rsi already holds the address of the text)
    syscall

    mov     eax, 1                      ; write(1, newline, 1)
    mov     edi, 1
    lea     rsi, [newline]
    mov     edx, 1
    syscall

    mov     eax, 60                     ; exit(0)
    xor     edi, edi
    syscall
