; Purpose : reference solution for exercise 03, Part 2. Stores the sum or difference
;           and returns the CPU flags of that very operation as a bit mask.
; Target  : Linux x86-64, ELF, System V calling convention
; Build   : nasm -f elf64 solution.asm -o solution.o
;
; Status bits in the return value:
;   bit 0 = CF   bit 1 = ZF   bit 2 = SF   bit 3 = OF

bits 64
default rel

section .text
global ledger_add, ledger_sub

; uint32_t ledger_add(int64_t a, int64_t b, int64_t *out)
ledger_add:
    add     rdi, rsi                    ; the operation whose flags we report
    mov     [rdx], rdi                  ; mov leaves every flag untouched
    jmp     collect_flags

; uint32_t ledger_sub(int64_t a, int64_t b, int64_t *out)
ledger_sub:
    sub     rdi, rsi
    mov     [rdx], rdi
    ; falls through

; Reads the flags set by the operation above. Only setcc and movzx/shl/or run here,
; none of which changes the flags before every setcc has been executed.
collect_flags:
    setc    al
    setz    cl
    sets    r8b
    seto    r9b
    movzx   eax, al                     ; bit 0 = CF
    movzx   ecx, cl
    shl     ecx, 1
    or      eax, ecx                    ; bit 1 = ZF
    movzx   ecx, r8b
    shl     ecx, 2
    or      eax, ecx                    ; bit 2 = SF
    movzx   ecx, r9b
    shl     ecx, 3
    or      eax, ecx                    ; bit 3 = OF
    ret

section .note.GNU-stack noalloc noexec nowrite progbits
