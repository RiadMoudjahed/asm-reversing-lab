; Purpose : reference solution for exercise 06, Part 2. Returns 1 exactly when the
;           maze binary prints "path open" for the same three numbers.
; Target  : Linux x86-64, ELF, System V calling convention
; Build   : nasm -f elf64 solution.asm -o solution.o
;
; Rules recovered from the binary:
;   100 <= a <= 120 and (a AND 7) == 5          (a is 101, 109 or 117)
;   -31 <= b <= -21
;   c < 0, the low byte of c is 200, and c >= -300   (c is -56)

bits 64
default rel

section .text
global maze_ok

; int32_t maze_ok(int32_t a, int32_t b, int32_t c)      a = edi, b = esi, c = edx
maze_ok:
    lea     eax, [rdi - 100]            ; a - 100, compared as unsigned: one test for both bounds
    cmp     eax, 20
    ja      .no                         ; above 20 (unsigned): a < 100 or a > 120

    mov     eax, edi
    and     eax, 7
    cmp     eax, 5
    jne     .no

    lea     eax, [rsi + 31]             ; b + 31 in 0..10 means -31 <= b <= -21
    cmp     eax, 10
    ja      .no

    test    edx, edx
    jns     .no                         ; c must be negative (signed)
    cmp     dl, 200                     ; the low byte of c
    jne     .no
    cmp     edx, -300
    jl      .no                         ; c >= -300 (signed)

    mov     eax, 1
    ret
.no:
    xor     eax, eax
    ret

section .note.GNU-stack noalloc noexec nowrite progbits
