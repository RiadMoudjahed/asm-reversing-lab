; Purpose : reference solution for exercise 18, Part 2. The record parser with the bugs removed.
; Target  : Linux x86-64, System V AMD64
; Build   : nasm -f elf64 solution.asm -o solution.o
;
;   long parse_records(const unsigned char *buf, size_t n, struct out *o)
;
; Records are type (1 byte), length (1 byte), payload; the type 0 record ends the list.
;   1 NAME    length at most 15; copied into o->name, the rest of the 16 bytes zeroed
;   2 VALUES  length a multiple of 4 and at most 32; signed 32-bit values added to o->sum
;   3 TAGS    every byte at most 7; o->tags[byte] is incremented
;   4 BLOB    length = 1 + 4 * (first payload byte); o->blob_sum = sum of the bytes after it
;   5 RESET   length 0; o->blob_sum = 0
; Layout of struct out: name 0 (16), sum 16 (8), tags 24 (8 x 4), blob_sum 56 (4), pad 60 (4).
; Returns the number of records before END, or a negative error code:
;   -1 truncated, -2 NAME, -3 VALUES, -4 TAGS, -5 BLOB or RESET, -6 unknown type.

bits 64
default rel

global parse_records

section .text

parse_records:
    xor     r8d, r8d                    ; pos
    xor     r9d, r9d                    ; number of records
.next:
    mov     rax, rsi
    sub     rax, r8                     ; bytes left; pos never exceeds n
    cmp     rax, 2
    jb      .trunc                      ; no room for type and length
    movzx   r10d, byte [rdi + r8]       ; type
    movzx   r11d, byte [rdi + r8 + 1]   ; length
    add     r8, 2
    sub     rax, 2                      ; bytes left after the two header bytes
    cmp     rax, r11
    jb      .trunc                      ; the payload would run past the end
    lea     rcx, [rdi + r8]             ; payload
    add     r8, r11                     ; the next record starts after the payload
    test    r10d, r10d
    jz      .done
    cmp     r10d, 1
    je      .name
    cmp     r10d, 2
    je      .values
    cmp     r10d, 3
    je      .tags
    cmp     r10d, 4
    je      .blob
    cmp     r10d, 5
    je      .reset
    mov     rax, -6
    ret

.name:
    cmp     r11d, 15
    ja      .e2
    pxor    xmm0, xmm0
    movdqu  [rdx], xmm0                 ; clear all 16 bytes first
    xor     eax, eax
.name_copy:
    cmp     eax, r11d
    jae     .record_done
    mov     r10b, [rcx + rax]
    mov     [rdx + rax], r10b
    inc     eax
    jmp     .name_copy

.values:
    test    r11b, 3
    jnz     .e3
    cmp     r11d, 32
    ja      .e3
    xor     eax, eax
.values_loop:
    cmp     eax, r11d
    jae     .record_done
    movsxd  r10, dword [rcx + rax]
    add     [rdx + 16], r10
    add     eax, 4
    jmp     .values_loop

.tags:
    xor     eax, eax
.tags_loop:
    cmp     eax, r11d
    jae     .record_done
    movzx   r10d, byte [rcx + rax]
    cmp     r10d, 7
    ja      .e4
    inc     dword [rdx + 24 + r10*4]
    inc     eax
    jmp     .tags_loop

.blob:
    test    r11d, r11d
    jz      .e5                         ; the count byte is missing
    movzx   r10d, byte [rcx]
    lea     r10d, [r10*4 + 1]
    cmp     r10d, r11d
    jne     .e5                         ; the length does not match the count
    mov     dword [rdx + 56], 0
    mov     eax, 1
.blob_loop:
    cmp     eax, r11d
    jae     .record_done
    movzx   r10d, byte [rcx + rax]
    add     [rdx + 56], r10d
    inc     eax
    jmp     .blob_loop

.reset:
    test    r11d, r11d
    jnz     .e5
    mov     dword [rdx + 56], 0

.record_done:
    inc     r9
    jmp     .next

.done:
    mov     rax, r9
    ret
.trunc:
    mov     rax, -1
    ret
.e2:
    mov     rax, -2
    ret
.e3:
    mov     rax, -3
    ret
.e4:
    mov     rax, -4
    ret
.e5:
    mov     rax, -5
    ret

section .note.GNU-stack noalloc noexec nowrite progbits
