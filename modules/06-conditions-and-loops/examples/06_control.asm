; Purpose : small functions that each show one control-flow construct: comparisons,
;           conditional jumps, setcc, cmov and the common loop shapes.
;           They are called from 06_main.c.
; Target  : Linux x86-64, ELF, System V calling convention
;           (arguments in rdi, rsi, rdx; result in rax)
; Build   : nasm -f elf64 06_control.asm -o 06_control.o
; Run     : see the Makefile (links with 06_main.c)

bits 64
default rel

section .text
global max_signed, max_unsigned, sign_of, sign_branchless, sum_to, count_digits
global sum_array, is_power_of_two

; long max_signed(long a, long b)
max_signed:
    mov     rax, rdi
    cmp     rdi, rsi                    ; computes rdi - rsi, keeps only the flags
    cmovl   rax, rsi                    ; if rdi < rsi (signed), take rsi
    ret

; unsigned long max_unsigned(unsigned long a, unsigned long b)
max_unsigned:
    mov     rax, rdi
    cmp     rdi, rsi
    cmovb   rax, rsi                    ; if rdi < rsi (unsigned, "below"), take rsi
    ret

; long sign_of(long x)                  returns -1, 0 or 1, using branches (if / else if / else)
sign_of:
    test    rdi, rdi                    ; sets SF and ZF from rdi itself
    js      .negative                   ; sign flag set: x < 0
    jz      .zero                       ; zero flag set: x == 0
    mov     eax, 1                      ; otherwise positive
    ret
.negative:
    mov     rax, -1
    ret
.zero:
    xor     eax, eax
    ret

; long sign_branchless(long x)          the same result without any jump: (x > 0) - (x < 0)
sign_branchless:
    xor     eax, eax
    test    rdi, rdi
    setg    al                          ; al = 1 if x > 0 (signed greater)
    mov     edx, 0
    setl    dl                          ; dl = 1 if x < 0 (flags are still those of the test)
    sub     eax, edx
    movsxd  rax, eax
    ret

; long sum_to(long n)                   1 + 2 + ... + n using a while loop (0 for n <= 0)
sum_to:
    xor     eax, eax                    ; total = 0
    mov     rcx, 1                      ; i = 1
.while:
    cmp     rcx, rdi                    ; condition tested before each pass
    jg      .done                       ; i > n (signed): stop
    add     rax, rcx                    ; total += i
    inc     rcx                         ; i++
    jmp     .while
.done:
    ret

; long count_digits(unsigned long x)    number of decimal digits; 0 has one (do-while loop)
count_digits:
    mov     rax, rdi                    ; value to divide
    xor     ecx, ecx                    ; digits = 0
    mov     r8d, 10
.again:
    xor     edx, edx                    ; unsigned divide: clear the high half
    div     r8                          ; rax = rax / 10
    inc     ecx                         ; one more digit (body runs at least once)
    test    rax, rax
    jnz     .again                      ; condition tested after each pass
    mov     eax, ecx
    ret

; long sum_array(const int32_t *a, long n)    for (i = 0; i < n; i++) total += a[i]
sum_array:
    xor     eax, eax                    ; total = 0
    xor     ecx, ecx                    ; i = 0
.for:
    cmp     rcx, rsi
    jge     .end                        ; i >= n: stop
    movsxd  rdx, dword [rdi + rcx*4]    ; a[i], sign-extended
    add     rax, rdx
    inc     rcx
    jmp     .for
.end:
    ret

; long is_power_of_two(unsigned long x)       1 if x has exactly one bit set
is_power_of_two:
    xor     eax, eax
    test    rdi, rdi
    jz      .no                         ; zero has no bit set
    lea     rcx, [rdi - 1]              ; x - 1 flips the lowest set bit and everything below it
    test    rdi, rcx                    ; AND without storing: zero only if one bit was set
    setz    al
.no:
    ret

section .note.GNU-stack noalloc noexec nowrite progbits
