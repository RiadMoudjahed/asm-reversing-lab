; Purpose : reference solution for exercise 04, Part 2. Decodes the record from
;           Part 1 without bswap or movbe.
; Target  : Linux x86-64, ELF, System V calling convention
; Build   : nasm -f elf64 solution.asm -o solution.o
;
; struct config { uint32_t version; uint16_t port; uint16_t flags; uint32_t timeout_ms; };
; Record layout (offset, size, byte order):
;   0  4  big-endian     version
;   4  2  little-endian  port
;   6  2  big-endian     flags
;   8  4  little-endian  timeout_ms

bits 64
default rel

section .text
global load_be32, load_le16, decode_record

; uint32_t load_be32(const uint8_t *p)
load_be32:
    movzx   eax, byte [rdi]             ; most significant byte first
    shl     eax, 8
    movzx   edx, byte [rdi + 1]
    or      eax, edx
    shl     eax, 8
    movzx   edx, byte [rdi + 2]
    or      eax, edx
    shl     eax, 8
    movzx   edx, byte [rdi + 3]
    or      eax, edx
    ret

; uint16_t load_le16(const uint8_t *p)
load_le16:
    movzx   eax, word [rdi]             ; x86 is little-endian: a plain load is enough
    ret

; void decode_record(const uint8_t *rec, struct config *out)
decode_record:
    movzx   eax, byte [rdi]             ; version: big-endian 32 bits, built byte by byte
    shl     eax, 8
    movzx   ecx, byte [rdi + 1]
    or      eax, ecx
    shl     eax, 8
    movzx   ecx, byte [rdi + 2]
    or      eax, ecx
    shl     eax, 8
    movzx   ecx, byte [rdi + 3]
    or      eax, ecx
    mov     [rsi], eax

    movzx   eax, word [rdi + 4]         ; port: little-endian 16 bits
    mov     [rsi + 4], ax

    movzx   eax, byte [rdi + 6]         ; flags: big-endian 16 bits
    shl     eax, 8
    movzx   ecx, byte [rdi + 7]
    or      eax, ecx
    mov     [rsi + 6], ax

    mov     eax, [rdi + 8]              ; timeout_ms: little-endian 32 bits
    mov     [rsi + 8], eax
    ret

section .note.GNU-stack noalloc noexec nowrite progbits
