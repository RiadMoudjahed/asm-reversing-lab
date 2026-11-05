; Purpose : small functions that each show one way of moving data or addressing
;           memory. They are called from 04_main.c.
; Target  : Linux x86-64, ELF, System V calling convention
;           (first two integer arguments in rdi and rsi, result in rax)
; Build   : nasm -f elf64 04_addressing.asm -o 04_addressing.o
; Run     : see the Makefile (links with 04_main.c)

bits 64
default rel

section .data
table:      dq 111, 222, 333            ; three 8-byte values

section .text
global get_dword, load_u8, load_s8, load_le16, times5plus7
global item_price, set_second, swap_ints, table_second

; long get_dword(const int32_t *a, long i)      returns a[i]
get_dword:
    movsxd  rax, dword [rdi + rsi*4]    ; base + index*scale; sign-extend 32 to 64 bits
    ret

; unsigned load_u8(const uint8_t *p)            returns *p as an unsigned number
load_u8:
    movzx   eax, byte [rdi]             ; zero-extend 8 to 32 bits
    ret

; int load_s8(const uint8_t *p)                 returns *p as a signed number
load_s8:
    movsx   eax, byte [rdi]             ; sign-extend 8 to 32 bits
    ret

; unsigned load_le16(const uint8_t *p)          two bytes read as one little-endian value
load_le16:
    movzx   eax, word [rdi]             ; the word at p: byte 0 is the low byte
    ret

; long times5plus7(long x)                      returns 5*x + 7 without any multiply
times5plus7:
    lea     rax, [rdi + rdi*4 + 7]      ; lea computes the address expression; it reads no memory
    ret

; struct item { int32_t id; int32_t qty; int64_t price; };   16 bytes
; long item_price(const struct item *items, long i)
item_price:
    shl     rsi, 4                      ; i * 16 = offset of items[i]
    mov     rax, [rdi + rsi + 8]        ; base + index + displacement: the price field
    ret

; void set_second(int32_t *a)                   a[1] = 0
set_second:
    mov     dword [rdi + 4], 0          ; the size keyword is required: nothing else says how wide
    ret

; void swap_ints(int32_t *a, int32_t *b)        exchange *a and *b
swap_ints:
    mov     eax, [rdi]                  ; memory to register
    mov     edx, [rsi]                  ; (there is no memory-to-memory mov)
    mov     [rdi], edx                  ; register to memory
    mov     [rsi], eax
    ret

; long table_second(void)                       returns the second entry of 'table'
table_second:
    mov     rax, [table + 8]            ; with 'default rel' this is addressed relative to rip
    ret

section .note.GNU-stack noalloc noexec nowrite progbits   ; tells the linker the stack need not be executable
