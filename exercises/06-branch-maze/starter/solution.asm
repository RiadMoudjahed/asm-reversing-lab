; Exercise 06, Part 2: reproduce the maze rules in assembly.
; Target  : Linux x86-64, ELF, System V calling convention
; Build   : nasm -f elf64 solution.asm -o solution.o   (or: make check EX=06)
;
; int32_t maze_ok(int32_t a, int32_t b, int32_t c)
;   a is in edi, b in esi, c in edx. Return 1 in eax when the maze binary prints
;   "path open" for the same three numbers, otherwise 0.
; Use only cmp, test and conditional jumps for the decisions.

bits 64
default rel

section .text
global maze_ok

maze_ok:
    ; TODO
    xor     eax, eax
    ret

section .note.GNU-stack noalloc noexec nowrite progbits
