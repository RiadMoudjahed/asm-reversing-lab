; Exercise 03, Part 2: report the CPU flags of an addition and a subtraction.
; Target  : Linux x86-64, ELF, System V calling convention
; Build   : nasm -f elf64 solution.asm -o solution.o   (or: make check EX=03)
;
; Return value: a bit mask of the flags set by the operation itself
;   bit 0 = CF   bit 1 = ZF   bit 2 = SF   bit 3 = OF   (all other bits 0)

bits 64
default rel

section .text
global ledger_add, ledger_sub

; uint32_t ledger_add(int64_t a, int64_t b, int64_t *out)     *out = a + b
ledger_add:
    ; TODO
    xor     eax, eax
    ret

; uint32_t ledger_sub(int64_t a, int64_t b, int64_t *out)     *out = a - b
ledger_sub:
    ; TODO
    xor     eax, eax
    ret

section .note.GNU-stack noalloc noexec nowrite progbits
