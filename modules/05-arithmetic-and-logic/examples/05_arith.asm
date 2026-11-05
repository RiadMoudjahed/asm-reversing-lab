; Purpose : small functions that each show one arithmetic or logic instruction and how
;           it affects the flags. They are called from 05_main.c.
; Target  : Linux x86-64, ELF, System V calling convention
;           (arguments in rdi, rsi, rdx; result in rax)
; Build   : nasm -f elf64 05_arith.asm -o 05_arith.o
; Run     : see the Makefile (links with 05_main.c)

bits 64
default rel

section .text
global add_overflows, add_carries, mul_wide, divide, sar_by, shr_by
global add128, negate, combine_bits

; long add_overflows(long a, long b, long *sum)    stores a+b; returns 1 on signed overflow
add_overflows:
    add     rdi, rsi                    ; sets CF, ZF, SF, OF from the 64-bit sum
    mov     [rdx], rdi                  ; mov does not change any flag
    seto    al                          ; al = OF
    movzx   eax, al                     ; widen to the full return value
    ret

; unsigned long add_carries(unsigned long a, unsigned long b)    returns 1 on unsigned overflow
add_carries:
    add     rdi, rsi
    setc    al                          ; al = CF
    movzx   eax, al
    ret

; unsigned long mul_wide(unsigned long a, unsigned long b, unsigned long *hi)
; returns the low 64 bits of a*b and stores the high 64 bits in *hi
mul_wide:
    mov     rcx, rdx                    ; mul overwrites rdx, so save the pointer first
    mov     rax, rdi
    mul     rsi                         ; rdx:rax = rax * rsi (unsigned, 128-bit result)
    mov     [rcx], rdx                  ; high half
    ret                                 ; low half is already in rax

; long divide(long a, long b, long *rem)    returns a/b and stores a%b in *rem
divide:
    mov     rcx, rdx                    ; idiv overwrites rdx, so save the pointer first
    mov     rax, rdi
    cqo                                 ; sign-extend rax into rdx:rax
    idiv    rsi                         ; rax = quotient, rdx = remainder (signed)
    mov     [rcx], rdx
    ret

; long sar_by(long x, int n)                arithmetic shift right: the sign bit is copied in
sar_by:
    mov     rax, rdi
    mov     ecx, esi                    ; a variable shift count must be in cl
    sar     rax, cl
    ret

; unsigned long shr_by(unsigned long x, int n)    logical shift right: zeros are shifted in
shr_by:
    mov     rax, rdi
    mov     ecx, esi
    shr     rax, cl
    ret

; void add128(uint64_t a[2], const uint64_t b[2])    a += b as 128-bit numbers (low word first)
add128:
    mov     rax, [rsi]
    add     [rdi], rax                  ; low halves: may set CF
    mov     rax, [rsi + 8]
    adc     [rdi + 8], rax              ; high halves plus the carry from the low halves
    ret

; long negate(long x)                       two's complement negation
negate:
    mov     rax, rdi
    neg     rax
    ret

; unsigned combine_bits(unsigned x)         ((x AND 0xF0) OR 0x0F) XOR 0xFF
combine_bits:
    mov     eax, edi
    and     eax, 0xF0                   ; keep only the high nibble of the low byte
    or      eax, 0x0F                   ; force the low nibble to ones
    xor     eax, 0xFF                   ; invert the low byte
    ret

section .note.GNU-stack noalloc noexec nowrite progbits
