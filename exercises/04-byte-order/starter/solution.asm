; Exercise 04, Part 2: decode the record you analyzed in Part 1.
; Target  : Linux x86-64, ELF, System V calling convention
; Build   : nasm -f elf64 solution.asm -o solution.o   (or: make check EX=04)
;
; struct config { uint32_t version; uint16_t port; uint16_t flags; uint32_t timeout_ms; };
; The layout of the 12-byte record (offsets, sizes, byte orders) is the one you
; recovered in Part 1. It is not repeated here.

bits 64
default rel

section .text
global load_be32, load_le16, decode_record

; uint32_t load_be32(const uint8_t *p)      read 4 bytes, most significant byte first
load_be32:
    ; TODO
    xor     eax, eax
    ret

; uint16_t load_le16(const uint8_t *p)      read 2 bytes, least significant byte first
load_le16:
    ; TODO
    xor     eax, eax
    ret

; void decode_record(const uint8_t *rec, struct config *out)
decode_record:
    ; TODO
    ret

section .note.GNU-stack noalloc noexec nowrite progbits
