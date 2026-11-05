; Purpose : show the four data sections. A letter stored in .data is copied
;           into a buffer reserved in .bss and printed with a newline.
; Target  : Linux x86-64, ELF, no libc
; Build   : nasm -f elf64 03_data_bss.asm -o 03_data_bss.o && ld 03_data_bss.o -o 03_data_bss
; Run     : ./03_data_bss
; Output  : A

bits 64
default rel

section .rodata
newline:    db 10               ; read-only constant

section .data
letter:     db "A"              ; initialized, writable

section .bss
buffer:     resb 2              ; reserved, zero at start, takes no space in the file

section .text
global _start

_start:
    mov     al, [letter]        ; al = the byte stored at 'letter'
    mov     [buffer], al        ; store it in the first byte of the buffer
    mov     al, [newline]       ; al = 10
    mov     [buffer + 1], al    ; store it in the second byte

    mov     eax, 1              ; write
    mov     edi, 1              ; standard output
    lea     rsi, [buffer]       ; address of the buffer
    mov     edx, 2              ; two bytes
    syscall

    mov     eax, 60             ; exit
    xor     edi, edi            ; status 0
    syscall
