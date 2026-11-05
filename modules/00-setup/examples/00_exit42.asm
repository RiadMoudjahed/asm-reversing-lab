; Purpose : smoke test for the assembler and linker. Does nothing except
;           end the process with exit status 42.
; Target  : Linux x86-64, ELF
; Build   : nasm -f elf64 00_exit42.asm -o 00_exit42.o && ld 00_exit42.o -o 00_exit42
; Run     : ./00_exit42 ; echo $?
; Output  : 42

bits 64
default rel

section .text
global _start

_start:
    mov     eax, 60             ; system call number 60 = exit
    mov     edi, 42             ; first argument: the exit status
    syscall                     ; ask the kernel to end the process
