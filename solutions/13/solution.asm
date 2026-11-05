; Purpose : reference solution for exercise 13, Part 2. The same function as the challenge
;           binary, with every division and remainder done by multiplication and shifts.
; Target  : Linux x86-64, System V AMD64
; Build   : nasm -f elf64 solution.asm -o solution.o

bits 64
default rel

global mix_asm

section .text

; int32_t mix_asm(int32_t x)        edi = x, result in eax
;   u = x as unsigned
;   mix = (u / 12) * 5 + (u % 9) * 3 - (x / 60) + (x % 11) * 4
;   with unsigned division for the first two terms and signed (truncating) division for the
;   last two. Every division by d is (x * M) >> k with M = ceil(2**k / d).
mix_asm:
    ; a = u / 12 = (u * 0xAAAAAAAB) >> 35, then a * 5
    mov     eax, edi
    mov     edx, 0xAAAAAAAB
    imul    rax, rdx
    shr     rax, 35
    lea     r8d, [rax + rax*4]

    ; b = u % 9: q = (u * 0x38E38E39) >> 33, b = u - q * 9, then b * 3
    mov     eax, edi
    imul    rax, rax, 0x38E38E39
    shr     rax, 33
    lea     eax, [rax + rax*8]          ; q * 9
    mov     edx, edi
    sub     edx, eax                    ; b
    lea     edx, [rdx + rdx*2]          ; b * 3
    add     r8d, edx                    ; a * 5 + b * 3

    ; c = x / 60 (signed): (x * 0x88888889) >> 37 with the constant read as positive,
    ; plus 1 if x is negative
    movsxd  rax, edi
    mov     edx, 0x88888889             ; zero-extended: the positive value 2290649225
    imul    rdx, rax
    sar     rdx, 37
    mov     ecx, edi
    sar     ecx, 31                     ; -1 if x is negative, otherwise 0
    sub     edx, ecx                    ; c
    sub     r8d, edx

    ; d = x % 11 (signed): q = (x * 0x2E8BA2E9) >> 33 (+ 1 if negative), d = x - q * 11
    imul    rdx, rax, 0x2E8BA2E9
    sar     rdx, 33
    sub     edx, ecx
    lea     eax, [rdx + rdx*4]          ; q * 5
    lea     eax, [rdx + rax*2]          ; q * 11
    mov     edx, edi
    sub     edx, eax                    ; d
    lea     eax, [r8 + rdx*4]           ; (a * 5 + b * 3 - c) + d * 4
    ret

section .note.GNU-stack noalloc noexec nowrite progbits
