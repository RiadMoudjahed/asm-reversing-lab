; Purpose : challenge program for exercise 05. Reads two decimal numbers from standard
;           input (one per line) and prints their sum.
; Target  : Linux x86 (32-bit), ELF, no libc
; Build   : make OUTDIR=<dir>

bits 32

MAX_DIGITS  equ 12
SLOTS       equ MAX_DIGITS + 1          ; one extra digit for a carry out of the top

section .rodata
err_text:   db "ERR", 10
err_len     equ $ - err_text
ovf_text:   db "OVERFLOW", 10
ovf_len     equ $ - ovf_text
zero_text:  db "0", 10

section .bss
input:      resb 64
digits:     resb SLOTS + 1              ; the sum, one digit per byte, plus a newline
a_ptr:      resd 1
a_len:      resd 1
b_ptr:      resd 1
b_len:      resd 1

section .text
global _start

_start:
    mov     eax, 3                      ; read(0, input, 63)
    xor     ebx, ebx
    mov     ecx, input
    mov     edx, 63
    int     0x80
    test    eax, eax
    jle     fail                        ; nothing to read
    mov     byte [input + eax], 0       ; terminator, so a missing newline is detected

    mov     esi, input
    call    parse_line                  ; first number
    mov     [a_ptr], edi
    mov     [a_len], ecx
    call    parse_line                  ; second number
    mov     [b_ptr], edi
    mov     [b_len], ecx

    ; add from the right, one digit per position, with a carry
    xor     ebp, ebp                    ; ebp = position from the right, 0 first
    xor     edx, edx                    ; edx = carry
.add:
    xor     eax, eax                    ; digit of a (0 when a is shorter)
    cmp     ebp, [a_len]
    jae     .no_a
    mov     esi, [a_ptr]
    add     esi, [a_len]
    sub     esi, ebp                    ; esi - 1 is the digit a_len - 1 - position
    movzx   eax, byte [esi - 1]
    sub     eax, '0'
.no_a:
    xor     ebx, ebx                    ; digit of b (0 when b is shorter)
    cmp     ebp, [b_len]
    jae     .no_b
    mov     esi, [b_ptr]
    add     esi, [b_len]
    sub     esi, ebp                    ; esi - 1 is the digit b_len - 1 - position
    movzx   ebx, byte [esi - 1]
    sub     ebx, '0'
.no_b:
    add     eax, ebx
    add     eax, edx                    ; plus the carry
    xor     edx, edx
    cmp     eax, 10
    jb      .store
    sub     eax, 10                     ; carry out of this position
    mov     edx, 1
.store:
    add     eax, '0'
    mov     edi, SLOTS - 1
    sub     edi, ebp
    mov     [digits + edi], al
    inc     ebp
    cmp     ebp, SLOTS
    jb      .add

    mov     byte [digits + SLOTS], 10

    ; a 13-digit sum does not fit the limit
    cmp     byte [digits], '0'
    jne     overflow

    ; skip leading zeros, but keep one digit
    xor     esi, esi
.strip:
    cmp     esi, SLOTS - 1
    jae     .print
    cmp     byte [digits + esi], '0'
    jne     .print
    inc     esi
    jmp     .strip
.print:
    mov     eax, 4                      ; write(1, digits + esi, SLOTS + 1 - esi)
    mov     ebx, 1
    lea     ecx, [digits + esi]
    mov     edx, SLOTS + 1
    sub     edx, esi
    int     0x80
    xor     ebx, ebx
    jmp     finish

overflow:
    mov     eax, 4
    mov     ebx, 1
    mov     ecx, ovf_text
    mov     edx, ovf_len
    int     0x80
    mov     ebx, 2
    jmp     finish

fail:
    mov     eax, 4
    mov     ebx, 1
    mov     ecx, err_text
    mov     edx, err_len
    int     0x80
    mov     ebx, 1
finish:
    mov     eax, 1
    int     0x80

; parse_line: esi points at the first character of a line.
; Checks that the line holds 1 to MAX_DIGITS digits followed by a newline.
; Returns edi = start of the digits, ecx = number of digits, esi = start of the next line.
; Jumps to fail on any error.
parse_line:
    mov     edi, esi
    xor     ecx, ecx
.scan:
    movzx   eax, byte [esi]
    cmp     al, 10
    je      .end
    sub     al, '0'
    cmp     al, 9
    ja      fail                        ; not a digit (the terminator 0 also lands here)
    inc     esi
    inc     ecx
    jmp     .scan
.end:
    inc     esi                         ; step over the newline
    test    ecx, ecx
    jz      fail                        ; empty line
    cmp     ecx, MAX_DIGITS
    ja      fail                        ; too long
    ret
