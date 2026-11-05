; Exercise 09, Part 2: write chain_asm, a hand-written version of chain from bin/stackdig.
; Target  : Linux x86-64, System V AMD64
; Build   : nasm -f elf64 solution.asm -o solution.o   (or: make check EX=09)
;
;   long chain_asm(long depth, long seed);
;
; The function must behave like chain in the binary: build the same table, report it with
; note(depth, table) at every level, and return the same value. note is provided by the tests:
;
;   void note(long depth, const long *entries);
;
; note clobbers every caller-saved register. The tests check that rsp is 16-byte aligned at
; each call to note, that the callee-saved registers are intact after chain_asm returns, and
; that no level uses more stack than the compiled chain does.

bits 64
default rel

extern note
global chain_asm

section .text

chain_asm:
    ; TODO
    xor     eax, eax
    ret

section .note.GNU-stack noalloc noexec nowrite progbits
