; Purpose : challenge program for exercise 08 (32-bit half). Four functions, each using
;           one calling convention, and a self-test that calls each the right way.
; Target  : Linux x86 (32-bit), ELF, no libc
; Build   : make OUTDIR=<dir>

bits 32

section .rodata
ok_text:    db "callcard32: ok", 10
ok_len      equ $ - ok_text
bad_text:   db "callcard32: failed", 10
bad_len     equ $ - bad_text

section .text
global _start
global f1, f2, f3, f4

f1:
    mov     eax, [esp + 4]
    lea     eax, [eax + eax*2]          ; 3 * a
    mov     edx, [esp + 8]
    lea     eax, [eax + edx*2]          ; + 2 * b
    add     eax, [esp + 12]             ; + c
    ret     12

f2:
    mov     eax, [esp + 12]
    imul    eax, eax, 100               ; 100 * c
    mov     edx, [esp + 8]
    lea     edx, [edx + edx*4]
    lea     eax, [eax + edx*2]          ; + 10 * b
    add     eax, [esp + 4]              ; + a
    ret

f3:
    lea     eax, [ecx + ecx*4]          ; 5 * a
    sub     eax, edx                    ; - b
    add     eax, [esp + 4]              ; + c
    ret     4

f4:
    mov     eax, [esp + 4]
    imul    eax, [esp + 8]              ; a * b
    ret

_start:
    ; f1(1, 2, 3) = 3 + 4 + 3 = 10    (the callee removes its arguments)
    push    3
    push    2
    push    1
    call    f1
    cmp     eax, 10
    jne     fail

    ; f2(1, 2, 3) = 1 + 20 + 300 = 321    (the caller removes its arguments)
    push    3
    push    2
    push    1
    call    f2
    add     esp, 12
    cmp     eax, 321
    jne     fail

    ; f3(1, 2, 3) = 5 - 2 + 3 = 6
    push    3
    mov     ecx, 1
    mov     edx, 2
    call    f3
    cmp     eax, 6
    jne     fail

    ; f4(6, 7) = 42
    push    7
    push    6
    call    f4
    add     esp, 8
    cmp     eax, 42
    jne     fail

    mov     eax, 4
    mov     ebx, 1
    mov     ecx, ok_text
    mov     edx, ok_len
    int     0x80
    mov     eax, 1
    xor     ebx, ebx
    int     0x80

fail:
    mov     eax, 4
    mov     ebx, 1
    mov     ecx, bad_text
    mov     edx, bad_len
    int     0x80
    mov     eax, 1
    mov     ebx, 1
    int     0x80
