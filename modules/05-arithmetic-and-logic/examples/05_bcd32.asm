; Purpose : add two packed BCD numbers (38 + 45) with the decimal adjust instruction.
;           This only works in 32-bit code: daa is not available in 64-bit mode.
; Target  : Linux x86 (32-bit), ELF, no libc
; Build   : nasm -f elf32 05_bcd32.asm -o 05_bcd32.o && ld -m elf_i386 05_bcd32.o -o 05_bcd32
; Run     : ./05_bcd32 ; echo $?
; Output  : 131   (which is 0x83, the packed BCD digits "83")

bits 32

section .text
global _start

_start:
    mov     al, 0x38                    ; packed BCD for 38
    add     al, 0x45                    ; binary add: 0x38 + 0x45 = 0x7D, not a valid BCD byte
    daa                                 ; decimal adjust after addition: 0x7D becomes 0x83

    movzx   ebx, al                     ; exit status = the adjusted byte
    mov     eax, 1                      ; exit
    int     0x80
