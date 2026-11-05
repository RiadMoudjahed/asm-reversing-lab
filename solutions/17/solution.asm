; Purpose : reference solution for exercise 17, Part 2. Reads archives made by arkpack.
; Target  : Linux x86-64, System V AMD64
; Build   : nasm -f elf64 solution.asm -o solution.o
;
; Format recovered from bin/arkpack (all numbers little endian):
;   header, 16 bytes
;     0   4  magic "ARK2"
;     4   2  entry count
;     6   1  key
;     7   1  zero
;     8   4  offset of the entry table
;     12  4  offset of the name area
;   records at offsets given by the entries, each aligned to 4; byte i of a record is stored
;     xor ((key + i) & 0xff)
;   entry table, 16 bytes per entry
;     0   4  name offset, relative to the name area
;     4   4  record offset
;     8   4  record size
;     12  4  checksum of the decoded bytes: start 0x13579bdf, for each byte
;            c = rotate_left(c, 7) + byte, finally c xor size
;   name area: zero-terminated names

bits 64
default rel

global ark_count
global ark_name
global ark_extract

section .text

; long ark_count(const unsigned char *a, size_t n)
;   rdi = a, rsi = n. Returns the entry count, or -1 if the header does not describe a
;   table and name area inside the archive. Clobbers rax, rcx, rdx.
ark_count:
    cmp     rsi, 16
    jb      .bad
    cmp     dword [rdi], 0x324b5241     ; "ARK2"
    jne     .bad
    movzx   eax, word [rdi + 4]         ; count
    mov     ecx, [rdi + 8]              ; table offset
    cmp     rcx, 16
    jb      .bad
    shl     rax, 4                      ; 16 * count, in 64 bits
    add     rax, rcx                    ; end of the table
    cmp     rax, rsi
    ja      .bad
    mov     edx, [rdi + 12]             ; name area offset
    cmp     rdx, rsi
    ja      .bad
    movzx   eax, word [rdi + 4]
    ret
.bad:
    mov     rax, -1
    ret

; long ark_name(const unsigned char *a, size_t n, long index, char *out, size_t cap)
;   rdi = a, rsi = n, rdx = index, rcx = out, r8 = cap
ark_name:
    mov     r9, rdx                     ; index
    mov     r10, rcx                    ; out
    sub     rsp, 8                      ; keep rsp aligned for the call
    call    ark_count
    add     rsp, 8
    test    rax, rax
    js      .bad_archive
    cmp     r9, rax                     ; unsigned: a negative index is too large
    jae     .bad_archive
    shl     r9, 4
    mov     eax, [rdi + 8]
    add     rax, r9                     ; entry offset
    mov     ecx, [rdi + rax]            ; name offset in the name area
    mov     edx, [rdi + 12]
    add     rcx, rdx                    ; absolute offset of the name
    cmp     rcx, rsi
    jae     .bad_bounds
    mov     rdx, rcx
.scan:
    cmp     byte [rdi + rdx], 0
    je      .found
    inc     rdx
    cmp     rdx, rsi
    jb      .scan
    jmp     .bad_bounds                 ; no zero byte before the end
.found:
    sub     rdx, rcx                    ; length
    lea     rax, [rdx + 1]
    cmp     rax, r8
    ja      .small
    lea     r11, [rdi + rcx]            ; source
    xor     eax, eax
.copy:
    mov     r9b, [r11 + rax]
    mov     [r10 + rax], r9b
    inc     rax
    cmp     rax, rdx
    jbe     .copy                       ; copies length + 1 bytes
    mov     rax, rdx
    ret
.bad_archive:
    mov     rax, -1
    ret
.bad_bounds:
    mov     rax, -2
    ret
.small:
    mov     rax, -3
    ret

; long ark_extract(const unsigned char *a, size_t n, long index, unsigned char *out, size_t cap)
;   rdi = a, rsi = n, rdx = index, rcx = out, r8 = cap
ark_extract:
    mov     r9, rdx                     ; index
    mov     r10, rcx                    ; out
    sub     rsp, 8
    call    ark_count
    add     rsp, 8
    test    rax, rax
    js      .bad_archive
    cmp     r9, rax
    jae     .bad_archive
    shl     r9, 4
    mov     eax, [rdi + 8]
    add     rax, r9                     ; entry offset
    mov     ecx, [rdi + rax + 4]        ; record offset
    mov     edx, [rdi + rax + 8]        ; size
    mov     r11d, [rdi + rax + 12]      ; stored checksum
    cmp     rcx, 16
    jb      .bad_bounds
    lea     rax, [rcx + rdx]            ; end of the record, no wrap in 64 bits
    cmp     rax, rsi
    ja      .bad_bounds
    cmp     rdx, r8
    ja      .small
    add     rcx, rdi                    ; source
    movzx   r8d, byte [rdi + 6]         ; key
    mov     r9d, 0x13579bdf             ; checksum
    xor     esi, esi                    ; index in the record
.decode:
    cmp     rsi, rdx
    jae     .done
    lea     eax, [r8 + rsi]             ; key + i, the low byte is used
    xor     al, [rcx + rsi]
    mov     [r10 + rsi], al
    rol     r9d, 7
    movzx   eax, al
    add     r9d, eax
    inc     rsi
    jmp     .decode
.done:
    xor     r9d, edx                    ; xor the size
    cmp     r9d, r11d
    jne     .mismatch
    mov     rax, rdx
    ret
.bad_archive:
    mov     rax, -1
    ret
.bad_bounds:
    mov     rax, -2
    ret
.small:
    mov     rax, -3
    ret
.mismatch:
    mov     rax, -4
    ret

section .note.GNU-stack noalloc noexec nowrite progbits
