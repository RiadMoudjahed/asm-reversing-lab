; Exercise 20, Part 2: the handlers of the machine you reverse engineered.
; Target  : Linux x86-64, System V AMD64
; Build   : nasm -f elf64 solution.asm -o solution.o   (or: make check EX=20)
;
;   void h_xxx(struct vm *m, const unsigned char *ins);    m in rdi, ins in rsi
;
; See vm.h for the layout of struct vm and the README for what each handler has to do. The
; program counter has already been moved past the instruction when a handler runs.
; Preserve rbx, rbp and r12 to r15.

bits 64
default rel

global h_mul, h_shl, h_shr, h_cmp, h_call, h_ret, h_ldb, h_stb

section .text

h_mul:
h_shl:
h_shr:
h_cmp:
h_call:
h_ret:
h_ldb:
h_stb:
    ; TODO: write one block per handler; for now they all do nothing
    ret

section .note.GNU-stack noalloc noexec nowrite progbits
