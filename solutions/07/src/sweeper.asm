; Purpose : challenge program for exercise 07. It contains six small memory and string
;           routines and a self-test that checks each of them.
; Target  : Linux x86-64, ELF, no libc
; Build   : make OUTDIR=<dir>
;
; The routines are global symbols fn1 to fn6, all in the System V calling convention.

bits 64
default rel

section .rodata
word_text:  db "sweeper", 0
hello_text: db "HELLO WORLD", 0
zzzz_text:  db "zzzz"
mixed_text: db "ababcd"
ok_text:    db "sweeper: all routines behave", 10
ok_len      equ $ - ok_text
bad_text:   db "sweeper: a routine failed", 10
bad_len     equ $ - bad_text

section .data
scratch:    db "abcdef", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0

section .text
global _start
global fn1, fn2, fn3, fn4, fn5, fn6

fn1:
    mov     rax, rdi
    mov     rcx, rdx
    shr     rcx, 3
    rep movsq
    mov     ecx, edx
    and     ecx, 7
    rep movsb
    ret

fn2:
    xor     eax, eax
    mov     rcx, -1
    repne scasb
    not     rcx
    lea     rax, [rcx - 1]
    ret

fn3:
    mov     rax, rdi
    mov     rcx, rdx
    cmp     rdi, rsi
    jbe     .forward
    lea     r8, [rsi + rdx]
    cmp     rdi, r8
    jae     .forward
    std
    lea     rsi, [rsi + rdx - 1]
    lea     rdi, [rdi + rdx - 1]
    rep movsb
    cld
    ret
.forward:
    rep movsb
    ret

fn4:
    mov     rcx, rdx
    xor     eax, eax
    repe cmpsb
    je      .same
    movzx   eax, byte [rdi - 1]
    movzx   ecx, byte [rsi - 1]
    sub     eax, ecx
.same:
    ret

fn5:
    mov     eax, esi
    mov     rcx, rdx
    test    rcx, rcx
    jz      .none
    repne scasb
    jne     .none
    lea     rax, [rdi - 1]
    ret
.none:
    xor     eax, eax
    ret

fn6:
    mov     r8, rdi
    movzx   eax, sil
    mov     rcx, rdx
    rep stosb
    mov     rax, r8
    ret

_start:
    ; the length of "sweeper" is 7
    lea     rdi, [word_text]
    call    fn2
    cmp     rax, 7
    jne     fail

    ; 'p' sits at offset 4 of "sweeper"
    lea     rdi, [word_text]
    mov     esi, 'p'
    mov     edx, 7
    call    fn5
    lea     rcx, [word_text + 4]
    cmp     rax, rcx
    jne     fail

    ; fill four bytes with 'z' and compare with "zzzz"
    lea     rdi, [scratch + 8]
    mov     esi, 'z'
    mov     edx, 4
    call    fn6
    lea     rdi, [scratch + 8]
    lea     rsi, [zzzz_text]
    mov     edx, 4
    call    fn4
    test    eax, eax
    jnz     fail

    ; copy "HELLO WORLD" and compare
    lea     rdi, [scratch + 8]
    lea     rsi, [hello_text]
    mov     edx, 11
    call    fn1
    lea     rdi, [scratch + 8]
    lea     rsi, [hello_text]
    mov     edx, 11
    call    fn4
    test    eax, eax
    jnz     fail

    ; overlapping move: "abcdef" with bytes 0..3 moved to 2..5 gives "ababcd"
    lea     rdi, [scratch]
    mov     byte [rdi + 2], 'c'
    mov     byte [rdi + 3], 'd'
    mov     byte [rdi + 4], 'e'
    mov     byte [rdi + 5], 'f'
    lea     rdi, [scratch + 2]
    lea     rsi, [scratch]
    mov     edx, 4
    call    fn3
    lea     rdi, [scratch]
    lea     rsi, [mixed_text]
    mov     edx, 6
    call    fn4
    test    eax, eax
    jnz     fail

    mov     eax, 1
    mov     edi, 1
    lea     rsi, [ok_text]
    mov     edx, ok_len
    syscall
    mov     eax, 60
    xor     edi, edi
    syscall

fail:
    mov     eax, 1
    mov     edi, 1
    lea     rsi, [bad_text]
    mov     edx, bad_len
    syscall
    mov     eax, 60
    mov     edi, 1
    syscall
