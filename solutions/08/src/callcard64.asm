; Purpose : challenge program for exercise 08 (64-bit half). Two functions that follow
;           different 64-bit calling conventions, and a self-test that calls each
;           the right way.
; Target  : Linux x86-64, ELF, no libc
; Build   : make OUTDIR=<dir>

bits 64
default rel

section .rodata
ok_text:    db "callcard64: ok", 10
ok_len      equ $ - ok_text
bad_text:   db "callcard64: failed", 10
bad_len     equ $ - bad_text

section .text
global _start
global g1, g2

g1:
    mov     [rsp + 8], rcx
    mov     [rsp + 16], rdx
    mov     rax, [rsp + 8]
    imul    rax, rax, 7                 ; 7 * a
    add     rax, [rsp + 16]             ; + b
    add     rax, r8                     ; + c
    ret

g2:
    lea     rax, [rsi + rsi]            ; 2 * b
    add     rax, rdi                    ; + a
    lea     rdx, [rdx + rdx*2]
    add     rax, rdx                    ; + 3 * c
    ret

_start:
    ; g1(1, 2, 3) = 7 + 2 + 3 = 12, called with 32 bytes of shadow space
    sub     rsp, 40
    mov     ecx, 1
    mov     edx, 2
    mov     r8d, 3
    call    g1
    add     rsp, 40
    cmp     rax, 12
    jne     fail

    ; g2(1, 2, 3) = 1 + 4 + 9 = 14
    mov     edi, 1
    mov     esi, 2
    mov     edx, 3
    call    g2
    cmp     rax, 14
    jne     fail

    mov     eax, 1
    mov     edi, 1
    lea     rsi, [ok_text]
    mov     edx, ok_len
    syscall
    mov     eax, 60
    xor     edi, edi
    syscall

fail:
    mov     eax, 1
    mov     edi, 1
    lea     rsi, [bad_text]
    mov     edx, bad_len
    syscall
    mov     eax, 60
    mov     edi, 1
    syscall
