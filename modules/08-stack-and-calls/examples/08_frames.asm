; Purpose : functions that show stack frames, local variables, callee-saved registers,
;           recursion, stack arguments, function pointers and calls into the C library.
;           They are called from 08_main.c.
; Target  : Linux x86-64, ELF, System V calling convention
;           (integer arguments in rdi, rsi, rdx, rcx, r8, r9, then on the stack)
; Build   : nasm -f elf64 08_frames.asm -o 08_frames.o
; Run     : see the Makefile (links with 08_main.c)
;
; At the first instruction of a function, rsp + 8 is a multiple of 16: the caller kept
; rsp aligned to 16 before the call, and the call pushed an 8-byte return address.

bits 64
default rel

extern strlen

section .text
global sum3, sum_locals, factorial, weighted8, apply_twice, length_of

; long sum3(long a, long b, long c)         a leaf function: no calls, no frame needed
sum3:
    lea     rax, [rdi + rsi]
    add     rax, rdx
    ret

; long sum_locals(long x)                   uses four local variables: x + 2x + 3x + 4x
sum_locals:
    push    rbp                         ; save the caller's frame pointer
    mov     rbp, rsp                    ; this function's frame pointer
    sub     rsp, 32                     ; room for four 8-byte locals
    mov     [rbp - 8], rdi              ; local 1 = x
    lea     rax, [rdi + rdi]
    mov     [rbp - 16], rax             ; local 2 = 2x
    lea     rax, [rdi + rdi*2]
    mov     [rbp - 24], rax             ; local 3 = 3x
    lea     rax, [rdi*4]
    mov     [rbp - 32], rax             ; local 4 = 4x
    mov     rax, [rbp - 8]
    add     rax, [rbp - 16]
    add     rax, [rbp - 24]
    add     rax, [rbp - 32]
    leave                               ; mov rsp, rbp ; pop rbp
    ret

; long factorial(long n)                    recursive; keeps n in the callee-saved rbx
factorial:
    cmp     rdi, 1
    jbe     .base
    push    rbx                         ; rbx is callee-saved: save it before using it
    mov     rbx, rdi                    ; remember n across the recursive call
    lea     rdi, [rdi - 1]
    call    factorial                   ; rsp is aligned here because of the push
    imul    rax, rbx                    ; n * factorial(n - 1)
    pop     rbx                         ; restore the caller's rbx
    ret
.base:
    mov     eax, 1
    ret

; long weighted8(long a, long b, long c, long d, long e, long f, long g, long h)
; returns a + 2b + 3c + 4d + 5e + 6f + 7g + 8h. The seventh and eighth arguments are
; on the stack: [rsp + 8] and [rsp + 16] at entry (above the return address).
weighted8:
    mov     rax, rdi                    ; 1 * a
    lea     rax, [rax + rsi*2]          ; + 2 * b
    lea     rdx, [rdx + rdx*2]
    add     rax, rdx                    ; + 3 * c
    lea     rax, [rax + rcx*4]          ; + 4 * d
    lea     r8, [r8 + r8*4]
    add     rax, r8                     ; + 5 * e
    lea     r9, [r9 + r9*2]
    lea     rax, [rax + r9*2]           ; + 6 * f
    mov     r10, [rsp + 8]              ; g
    lea     rdx, [r10*8]
    sub     rdx, r10
    add     rax, rdx                    ; + 7 * g
    mov     r10, [rsp + 16]             ; h
    lea     rax, [rax + r10*8]          ; + 8 * h
    ret

; long apply_twice(long (*f)(long), long x)     returns f(f(x)) through a function pointer
apply_twice:
    push    rbx                         ; callee-saved, and it also aligns rsp for the calls
    mov     rbx, rdi                    ; keep the function pointer
    mov     rdi, rsi                    ; first call: f(x)
    call    rbx
    mov     rdi, rax                    ; second call: f(result)
    call    rbx
    pop     rbx
    ret

; size_t length_of(const char *s)           calls the C library function strlen
length_of:
    sub     rsp, 8                      ; no push available: pad the stack to realign it
    call    strlen wrt ..plt
    add     rsp, 8
    ret

section .note.GNU-stack noalloc noexec nowrite progbits
