; Exercise 12, Part 2: a command dispatcher built on a jump table.
; Target  : Linux x86-64, System V AMD64
; Build   : nasm -f elf64 solution.asm -o solution.o   (or: make check EX=12)
;
;   int32_t run_cmd(int32_t cmd, uint32_t acc);   // edi = cmd, esi = acc, result in eax
;
; run_cmd must return exactly what dispatch() returns in bin/switchboard, for every command
; value and every acc value. See the README for the rules about the jump table.

bits 64
default rel

global run_cmd

section .text

run_cmd:
    ; TODO
    xor     eax, eax
    ret

section .note.GNU-stack noalloc noexec nowrite progbits
