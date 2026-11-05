; Purpose : reference solution for exercise 19, Part 2. Decodes one record of the simulator.
; Target  : Linux x86-64, System V AMD64
; Build   : nasm -f elf64 solution.asm -o solution.o
;
;   long decode_record(const unsigned char *rec, size_t avail, char *out, size_t cap)
;
; Record layout recovered from bin/dispatch: byte 0 is the length, byte 1 is the seed, then
; length encoded bytes. Decoding: k = seed; for each position i, k = rotate_left(k, 3) + i + 0x5d
; (all in 8 bits); the plain byte is the encoded byte xor k. A zero byte ends the output.
; Returns the length, -1 if the record is not fully inside avail bytes, -2 if cap is too small
; for the text and its zero byte.

bits 64
default rel

global decode_record

section .text

decode_record:
    cmp     rsi, 2
    jb      .truncated                  ; no room for the length and the seed
    movzx   r8d, byte [rdi]             ; length
    lea     r9, [r8 + 2]
    cmp     r9, rsi
    ja      .truncated                  ; the record is cut off
    lea     r9, [r8 + 1]
    cmp     r9, rcx
    ja      .small                      ; text and zero byte do not fit
    movzx   r10d, byte [rdi + 1]        ; k = seed
    xor     r11d, r11d                  ; i = 0
.next:
    cmp     r11, r8
    jae     .done
    rol     r10b, 3
    add     r10b, r11b                  ; plus i (low byte)
    add     r10b, 0x5d
    mov     al, [rdi + r11 + 2]
    xor     al, r10b
    mov     [rdx + r11], al
    inc     r11
    jmp     .next
.done:
    mov     byte [rdx + r8], 0
    mov     rax, r8
    ret
.truncated:
    mov     rax, -1
    ret
.small:
    mov     rax, -2
    ret

section .note.GNU-stack noalloc noexec nowrite progbits
