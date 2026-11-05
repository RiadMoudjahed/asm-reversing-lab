; Purpose : challenge program for exercise 04. Decodes a 12-byte configuration
;           record stored in .rodata and checks that the decoded fields add up
;           to the expected checksum.
; Target  : Linux x86-64, ELF, no libc
; Build   : make OUTDIR=<dir>

bits 64
default rel

CHECKSUM    equ 0x450CB

section .rodata
record:     db 0x00, 0x03, 0x00, 0x07           ; field A
            db 0xFB, 0x20                       ; field B
            db 0x80, 0x01                       ; field C
            db 0xC8, 0xAF, 0x00, 0x00           ; field D

good_text:  db "checksum ok", 10
good_len    equ $ - good_text
bad_text:   db "checksum mismatch", 10
bad_len     equ $ - bad_text

section .text
global _start

_start:
    mov     eax, [record]               ; field A
    bswap   eax

    movzx   ecx, word [record + 4]      ; field B

    movzx   edx, word [record + 6]      ; field C
    rol     dx, 8

    mov     ebx, [record + 8]           ; field D

    add     eax, ecx
    add     eax, edx
    add     eax, ebx
    cmp     eax, CHECKSUM
    jne     .bad

    mov     eax, 1
    mov     edi, 1
    lea     rsi, [good_text]
    mov     edx, good_len
    syscall
    mov     eax, 60
    xor     edi, edi
    syscall

.bad:
    mov     eax, 1
    mov     edi, 1
    lea     rsi, [bad_text]
    mov     edx, bad_len
    syscall
    mov     eax, 60
    mov     edi, 1
    syscall
