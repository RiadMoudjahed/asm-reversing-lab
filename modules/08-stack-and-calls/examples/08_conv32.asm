; Purpose : the same function written for the three common 32-bit calling conventions.
;           Each returns a*100 + b*10 + c. They are called from 08_conv32_main.c.
; Target  : Linux x86 (32-bit), ELF
; Build   : nasm -f elf32 08_conv32.asm -o 08_conv32.o
; Run     : see the Makefile (links with 08_conv32_main.c using gcc -m32)
;
; cdecl     : arguments on the stack, pushed right to left; the CALLER removes them.
; stdcall   : arguments on the stack; the CALLEE removes them with "ret N".
; fastcall  : first two arguments in ecx and edx, the rest on the stack; the callee
;             removes the stack part with "ret N".
; All three return the result in eax. eax, ecx and edx may be changed freely;
; ebx, esi, edi and ebp must be preserved.

bits 32

section .text
global add3_cdecl, add3_stdcall, add3_fastcall

; int add3_cdecl(int a, int b, int c)
add3_cdecl:
    mov     eax, [esp + 4]              ; a (above the return address)
    imul    eax, eax, 100
    mov     edx, [esp + 8]              ; b
    lea     edx, [edx + edx*4]
    lea     eax, [eax + edx*2]          ; + b * 10
    add     eax, [esp + 12]             ; + c
    ret                                 ; the caller pops the three arguments

; int __attribute__((stdcall)) add3_stdcall(int a, int b, int c)
add3_stdcall:
    mov     eax, [esp + 4]
    imul    eax, eax, 100
    mov     edx, [esp + 8]
    lea     edx, [edx + edx*4]
    lea     eax, [eax + edx*2]
    add     eax, [esp + 12]
    ret     12                          ; the callee pops 12 bytes of arguments

; int __attribute__((fastcall)) add3_fastcall(int a, int b, int c)
add3_fastcall:                          ; a in ecx, b in edx, c on the stack
    imul    eax, ecx, 100
    lea     edx, [edx + edx*4]
    lea     eax, [eax + edx*2]
    add     eax, [esp + 4]              ; c
    ret     4                           ; the callee pops its 4 bytes of stack argument

section .note.GNU-stack noalloc noexec nowrite progbits
