; Purpose : show that al, ax, eax and rax are views of the same register, and how
;           writing to a view affects the whole.
; Target  : Linux x86-64, ELF, no libc
; Build   : nasm -f elf64 -g -F dwarf 02_registers.asm -o 02_registers.o && ld 02_registers.o -o 02_registers
; Run     : gdb -q ./02_registers   (step with stepi and inspect rax)

bits 64
default rel

section .text
global _start

_start:
    mov     rax, 0x1122334455667788     ; fill all 64 bits
    mov     al, 0xFF                    ; replaces bits 0-7, the rest stays
    mov     ah, 0xEE                    ; replaces bits 8-15
    mov     ax, 0xDDCC                  ; replaces bits 0-15
    mov     eax, 0xAABBCCDD             ; replaces bits 0-31 and CLEARS bits 32-63
    mov     rax, -1                     ; all 64 bits set
    mov     eax, eax                    ; a 32-bit write zeroes the upper half

    mov     eax, 60                     ; exit
    xor     edi, edi
    syscall
