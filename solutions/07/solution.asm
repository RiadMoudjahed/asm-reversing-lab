; Purpose : reference solution for exercise 07, Part 2. strlen, memchr, memcpy and
;           memmove written with the x86 string instructions.
; Target  : Linux x86-64, ELF, System V calling convention
; Build   : nasm -f elf64 solution.asm -o solution.o
;
; The direction flag is clear on entry and must be clear on return.

bits 64
default rel

section .text
global my_strlen, my_memchr, my_memcpy, my_memmove

; size_t my_strlen(const char *s)
my_strlen:
    xor     eax, eax                    ; al = 0, the byte to find
    mov     rcx, -1                     ; effectively no limit
    repne scasb                         ; scan until [rdi] equals al
    not     rcx                         ; rcx counted down from -1: ~rcx = bytes scanned
    lea     rax, [rcx - 1]              ; minus the terminator
    ret

; void *my_memchr(const void *s, int c, size_t n)
my_memchr:
    mov     eax, esi                    ; al = the byte to find
    mov     rcx, rdx                    ; at most n bytes
    test    rcx, rcx
    jz      .none                       ; with rcx = 0 the repne does nothing and ZF is stale
    repne scasb
    jne     .none                       ; ZF = 0: n bytes compared, none matched
    lea     rax, [rdi - 1]              ; rdi is one past the match
    ret
.none:
    xor     eax, eax
    ret

; void *my_memcpy(void *d, const void *s, size_t n)
my_memcpy:
    mov     rax, rdi                    ; the return value
    mov     rcx, rdx
    rep movsb
    ret

; void *my_memmove(void *d, const void *s, size_t n)
my_memmove:
    mov     rax, rdi
    mov     rcx, rdx
    cmp     rdi, rsi
    jbe     .forward                    ; destination below the source: forward is safe
    lea     r8, [rsi + rdx]
    cmp     rdi, r8
    jae     .forward                    ; destination starts after the source ends
    std                                 ; overlap with the destination above: copy backward
    lea     rsi, [rsi + rdx - 1]
    lea     rdi, [rdi + rdx - 1]
    rep movsb
    cld                                 ; leave DF clear for the caller
    ret
.forward:
    rep movsb
    ret

section .note.GNU-stack noalloc noexec nowrite progbits
