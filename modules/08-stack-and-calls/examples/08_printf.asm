; Purpose : an assembly program that calls the C library function printf.
; Target  : Linux x86-64, ELF, linked with gcc so the C library is available
; Build   : nasm -f elf64 08_printf.asm -o 08_printf.o && gcc -no-pie 08_printf.o -o 08_printf
; Run     : ./08_printf
; Output  : The answer is 42

bits 64
default rel

extern printf

section .rodata
fmt:        db "The answer is %d", 10, 0

section .text
global main

main:
    sub     rsp, 8                      ; main was called with rsp + 8 aligned: realign for printf
    lea     rdi, [fmt]                  ; argument 1: the format string
    mov     esi, 42                     ; argument 2: the value for %d
    xor     eax, eax                    ; printf takes a variable number of arguments: al = number
                                        ; of vector registers used, here none
    call    printf wrt ..plt
    add     rsp, 8
    xor     eax, eax                    ; main returns 0
    ret

section .note.GNU-stack noalloc noexec nowrite progbits
