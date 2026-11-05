; Purpose : reference solution for exercise 20, Part 2: the eight handlers.
; Target  : Linux x86-64, System V AMD64
; Build   : nasm -f elf64 solution.asm -o solution.o
;
;   void h_xxx(struct vm *m, const unsigned char *ins);   m in rdi, ins in rsi
;
; struct vm: r[8] at 0, pc 32, zf 36, cf 40, fault 44, halted 48, depth 52, stack[16] (u16)
; at 56, mem[256] at 88. The register fields are the nibbles of ins[1], taken modulo 8.

bits 64
default rel

global h_mul, h_shl, h_shr, h_cmp, h_call, h_ret, h_ldb, h_stb

OFF_PC      equ 32
OFF_ZF      equ 36
OFF_CF      equ 40
OFF_FAULT   equ 44
OFF_HALTED  equ 48
OFF_DEPTH   equ 52
OFF_STACK   equ 56
OFF_MEM     equ 88

section .text

; mul a, b : r[a] = r[a] * r[b] (low 32 bits); zf = result is 0; cf = 0
h_mul:
    movzx   eax, byte [rsi + 1]
    mov     ecx, eax
    shr     ecx, 4
    and     ecx, 7                      ; a
    and     eax, 7                      ; b
    mov     edx, [rdi + rcx*4]
    imul    edx, [rdi + rax*4]
    mov     [rdi + rcx*4], edx
    test    edx, edx
    sete    al
    movzx   eax, al
    mov     [rdi + OFF_ZF], eax
    mov     dword [rdi + OFF_CF], 0
    ret

; shl a, n : n is the low nibble of ins[1]; n = 0 changes nothing
h_shl:
    movzx   eax, byte [rsi + 1]
    mov     r8d, eax
    shr     r8d, 4
    and     r8d, 7                      ; a
    and     eax, 15                     ; n
    jz      .done
    mov     edx, [rdi + r8*4]           ; value
    mov     r9d, edx
    mov     ecx, 32
    sub     ecx, eax
    shr     r9d, cl
    and     r9d, 1                      ; bit shifted out last: (v >> (32 - n)) & 1
    mov     [rdi + OFF_CF], r9d
    mov     ecx, eax
    shl     edx, cl
    mov     [rdi + r8*4], edx
    test    edx, edx
    sete    al
    movzx   eax, al
    mov     [rdi + OFF_ZF], eax
.done:
    ret

; shr a, n : cf = (v >> (n - 1)) & 1
h_shr:
    movzx   eax, byte [rsi + 1]
    mov     r8d, eax
    shr     r8d, 4
    and     r8d, 7
    and     eax, 15
    jz      .done
    mov     edx, [rdi + r8*4]
    mov     r9d, edx
    lea     ecx, [rax - 1]
    shr     r9d, cl
    and     r9d, 1
    mov     [rdi + OFF_CF], r9d
    mov     ecx, eax
    shr     edx, cl
    mov     [rdi + r8*4], edx
    test    edx, edx
    sete    al
    movzx   eax, al
    mov     [rdi + OFF_ZF], eax
.done:
    ret

; cmp a, b : flags as for sub, nothing is stored
h_cmp:
    movzx   eax, byte [rsi + 1]
    mov     ecx, eax
    shr     ecx, 4
    and     ecx, 7
    and     eax, 7
    mov     edx, [rdi + rcx*4]
    cmp     edx, [rdi + rax*4]
    sete    al
    setb    cl
    movzx   eax, al
    movzx   ecx, cl
    mov     [rdi + OFF_ZF], eax
    mov     [rdi + OFF_CF], ecx
    ret

; call target : push the (already advanced) pc, jump; fault 3 and halt when the stack is full
h_call:
    mov     eax, [rdi + OFF_DEPTH]
    cmp     eax, 16
    je      .full
    mov     ecx, [rdi + OFF_PC]
    mov     [rdi + OFF_STACK + rax*2], cx
    inc     eax
    mov     [rdi + OFF_DEPTH], eax
    movzx   eax, byte [rsi + 1]
    movzx   ecx, byte [rsi + 2]
    shl     ecx, 8
    or      eax, ecx
    mov     [rdi + OFF_PC], eax
    ret
.full:
    mov     dword [rdi + OFF_FAULT], 3
    mov     dword [rdi + OFF_HALTED], 1
    ret

; ret : pop the pc; fault 2 and halt when the stack is empty
h_ret:
    mov     eax, [rdi + OFF_DEPTH]
    test    eax, eax
    jz      .empty
    dec     eax
    mov     [rdi + OFF_DEPTH], eax
    movzx   ecx, word [rdi + OFF_STACK + rax*2]
    mov     [rdi + OFF_PC], ecx
    ret
.empty:
    mov     dword [rdi + OFF_FAULT], 2
    mov     dword [rdi + OFF_HALTED], 1
    ret

; ldb a, [b] : r[a] = mem[r[b] & 255]
h_ldb:
    movzx   eax, byte [rsi + 1]
    mov     ecx, eax
    shr     ecx, 4
    and     ecx, 7
    and     eax, 7
    mov     eax, [rdi + rax*4]
    movzx   eax, al
    movzx   eax, byte [rdi + OFF_MEM + rax]
    mov     [rdi + rcx*4], eax
    ret

; stb [a], b : mem[r[a] & 255] = low byte of r[b]
h_stb:
    movzx   eax, byte [rsi + 1]
    mov     ecx, eax
    shr     ecx, 4
    and     ecx, 7
    and     eax, 7
    mov     edx, [rdi + rax*4]          ; value
    mov     ecx, [rdi + rcx*4]          ; address
    movzx   ecx, cl
    mov     [rdi + OFF_MEM + rcx], dl
    ret

section .note.GNU-stack noalloc noexec nowrite progbits
