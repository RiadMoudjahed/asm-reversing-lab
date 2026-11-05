; Purpose : the classic memory and string routines written with the x86 string
;           instructions (movs, stos, scas, cmps) and the rep prefixes.
;           They are called from 07_main.c.
; Target  : Linux x86-64, ELF, System V calling convention
;           (arguments in rdi, rsi, rdx; result in rax)
; Build   : nasm -f elf64 07_strings.asm -o 07_strings.o
; Run     : see the Makefile (links with 07_main.c)
;
; The string instructions use fixed registers: rsi is the source address, rdi the
; destination, rcx the repeat count, and al/ax/eax/rax the value. They move rsi and
; rdi forward when the direction flag (DF) is clear, backward when it is set.
; The System V ABI requires DF to be clear on entry and on return.

bits 64
default rel

section .text
global my_strlen, my_memchr, my_memset, my_memcpy, my_memcmp, my_memmove

; size_t my_strlen(const char *s)
my_strlen:
    xor     eax, eax                    ; al = 0, the byte to look for
    mov     rcx, -1                     ; repeat "forever" (as many as 2^64 - 1 times)
    repne scasb                         ; compare al with [rdi], rdi++, rcx--, until equal
    not     rcx                         ; rcx counted down from -1: ~rcx = bytes scanned
    lea     rax, [rcx - 1]              ; minus the terminator itself
    ret

; void *my_memchr(const void *s, int c, size_t n)    first byte equal to c, or NULL
my_memchr:
    mov     eax, esi                    ; al = the byte to look for
    mov     rcx, rdx                    ; at most n bytes
    test    rcx, rcx
    jz      .none                       ; repne does nothing when rcx is 0 and leaves ZF alone
    repne scasb                         ; stops after a match (ZF = 1) or when rcx reaches 0
    jne     .none                       ; ZF = 0: all n bytes compared without a match
    lea     rax, [rdi - 1]              ; rdi is one past the matching byte
    ret
.none:
    xor     eax, eax
    ret

; void *my_memset(void *d, int c, size_t n)          fill n bytes with c, return d
my_memset:
    mov     r8, rdi                     ; keep d for the return value
    mov     eax, esi                    ; al = the byte to store
    mov     rcx, rdx
    rep stosb                           ; [rdi] = al, rdi++, rcx--, repeated rcx times
    mov     rax, r8
    ret

; void *my_memcpy(void *d, const void *s, size_t n)  copy n bytes forward, return d
my_memcpy:
    mov     rax, rdi
    mov     rcx, rdx
    rep movsb                           ; [rdi] = [rsi], both advance, repeated rcx times
    ret

; int my_memcmp(const void *a, const void *b, size_t n)    <0, 0 or >0 like memcmp
my_memcmp:
    mov     rcx, rdx
    xor     eax, eax                    ; also sets ZF, so n == 0 compares as equal
    repe cmpsb                          ; compare [rsi] with [rdi] while equal and rcx > 0
    je      .equal                      ; ZF = 1: no difference found
    movzx   eax, byte [rdi - 1]         ; the bytes that differed: both pointers moved past them
    movzx   ecx, byte [rsi - 1]
    sub     eax, ecx                    ; a's byte minus b's byte, as unsigned values
.equal:
    ret

; void *my_memmove(void *d, const void *s, size_t n)  copy n bytes, overlap allowed
my_memmove:
    mov     rax, rdi
    mov     rcx, rdx
    cmp     rdi, rsi
    jbe     .forward                    ; destination below source: a forward copy is safe
    lea     r8, [rsi + rdx]
    cmp     rdi, r8
    jae     .forward                    ; destination starts after the source ends: no overlap
    std                                 ; DF = 1: copy backward, so the overlap is read first
    lea     rsi, [rsi + rdx - 1]        ; start from the last byte of both blocks
    lea     rdi, [rdi + rdx - 1]
    rep movsb
    cld                                 ; restore DF = 0 before returning
    ret
.forward:
    rep movsb
    ret

section .note.GNU-stack noalloc noexec nowrite progbits
