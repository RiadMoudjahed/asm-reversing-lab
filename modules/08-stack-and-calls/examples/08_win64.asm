; Purpose : two functions that follow the Microsoft x64 calling convention.
; Target  : Windows x86-64 (PE), built with NASM and MinGW-w64, run under Wine
; Build   : nasm -f win64 08_win64.asm -o 08_win64.obj
; Run     : see the Makefile
;
; Microsoft x64: the first four integer arguments are in rcx, rdx, r8, r9; further
; arguments go on the stack. The CALLER always reserves 32 bytes of "shadow space" just
; above the return address for the callee to spill those four registers, and keeps rsp
; 16-byte aligned at the call. rbx, rbp, rdi, rsi and r12 to r15 are callee-saved.

bits 64
default rel

section .text
global ms_sum4, ms_call_sum4

; long ms_sum4(long a, long b, long c, long d)      a*1000 + b*100 + c*10 + d
ms_sum4:
    imul    eax, ecx, 1000              ; a
    imul    edx, edx, 100               ; b
    add     eax, edx
    lea     r8d, [r8 + r8*4]
    lea     eax, [rax + r8*2]           ; + c * 10
    add     eax, r9d                    ; + d
    ret

; long ms_call_sum4(void)                           calls ms_sum4(1, 2, 3, 4)
ms_call_sum4:
    sub     rsp, 40                     ; 32 bytes of shadow space + 8 to realign rsp
    mov     ecx, 1
    mov     edx, 2
    mov     r8d, 3
    mov     r9d, 4
    call    ms_sum4
    add     rsp, 40
    ret
