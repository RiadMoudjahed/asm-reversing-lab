; Purpose : a function written with the macros from 08_frame.inc, in its own source file.
;           It is assembled separately and linked with the other objects.
; Target  : Linux x86-64, ELF, System V calling convention
; Build   : nasm -f elf64 08_macros.asm -o 08_macros.o
; Run     : see the Makefile (linked with 08_main.c)

bits 64
default rel

%include "08_frame.inc"

section .text
global macro_demo                       ; make the symbol visible to the linker and to C

; long macro_demo(long x)                returns 3*x + 1, using one local variable
macro_demo:
    FUNC_ENTER 16                       ; expands to push rbp / mov rbp, rsp / sub rsp, 16
    mov     [rbp - 8], rdi              ; a local copy of x
    mov     rax, [rbp - 8]
    lea     rax, [rax + rax*2]          ; 3 * x
    inc     rax                         ; + 1
    FUNC_LEAVE                          ; expands to leave / ret

section .note.GNU-stack noalloc noexec nowrite progbits
