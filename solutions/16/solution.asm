; Purpose : reference solution for exercise 16, Part 2.
;           digest_buffer uses the library; pack_values replaces sl_pack.
; Target  : Linux x86-64, System V AMD64
; Build   : nasm -f elf64 solution.asm -o solution.o
;
; Interface recovered from bin/libsilent.so:
;   struct ctx *sl_new(unsigned seed)                       16-byte state, null if out of memory
;   void sl_feed(struct ctx *c, const unsigned char *p, size_t n)
;   unsigned sl_final(const struct ctx *c)
;   void sl_free(struct ctx *c)
;   int sl_pack(const int *vals, size_t n, unsigned char *out, size_t cap)
;       Zigzag-encodes each value (v << 1 xor v >> 31, arithmetic) and writes it as a base-128
;       number, low 7 bits first, with bit 7 set on every byte but the last. If a value does not
;       fit in the remaining capacity, returns -1 and leaves that value unwritten; earlier
;       values stay in the buffer. Otherwise returns the number of bytes written.

bits 64
default rel

extern sl_new
extern sl_feed
extern sl_final
extern sl_free
global digest_buffer
global pack_values

section .text

; unsigned digest_buffer(const unsigned char *p, size_t n, unsigned seed)
;   rdi = p, rsi = n, edx = seed
digest_buffer:
    push    rbx
    push    r12
    push    r13                         ; three pushes + the return address keep rsp aligned
    mov     rbx, rdi                    ; p
    mov     r12, rsi                    ; n
    mov     edi, edx                    ; seed
    call    sl_new wrt ..plt
    mov     r13, rax                    ; handle
    mov     rdi, r13
    mov     rsi, rbx
    mov     rdx, r12
    call    sl_feed wrt ..plt
    mov     rdi, r13
    call    sl_final wrt ..plt
    mov     ebx, eax                    ; the digest, kept across sl_free
    mov     rdi, r13
    call    sl_free wrt ..plt
    mov     eax, ebx
    pop     r13
    pop     r12
    pop     rbx
    ret

; int pack_values(const int *vals, size_t n, unsigned char *out, size_t cap)
;   rdi = vals, rsi = n, rdx = out, rcx = cap
pack_values:
    xor     r8d, r8d                    ; bytes written
    xor     r9d, r9d                    ; index
.next:
    cmp     r9, rsi
    jae     .done
    mov     eax, [rdi + r9*4]
    mov     r10d, eax
    add     eax, eax                    ; v << 1
    sar     r10d, 31                    ; sign of v: 0 or -1
    xor     eax, r10d                   ; zigzag value
    mov     r10d, 1                     ; length in bytes
    mov     r11d, eax
.len:
    cmp     r11d, 0x80
    jb      .fit
    shr     r11d, 7
    inc     r10d
    jmp     .len
.fit:
    lea     r11, [r8 + r10]             ; bytes written after this value
    cmp     r11, rcx
    ja      .fail
.emit:
    cmp     eax, 0x80
    jb      .last
    mov     r11d, eax
    or      r11b, 0x80
    mov     [rdx + r8], r11b
    inc     r8
    shr     eax, 7
    jmp     .emit
.last:
    mov     [rdx + r8], al
    inc     r8
    inc     r9
    jmp     .next
.done:
    mov     eax, r8d
    ret
.fail:
    mov     eax, -1
    ret

section .note.GNU-stack noalloc noexec nowrite progbits
