; Exercise 15, Part 2: insert into and search the tree you recovered in Part 1.
; Target  : Linux x86-64, System V AMD64
; Build   : nasm -f elf64 solution.asm -o solution.o   (or: make check EX=15)
;
;   struct node *inv_insert(struct node *root, int key);   returns the root
;   struct node *inv_find(struct node *root, int key);     returns the node, or 0
;
; The tests provide new nodes: struct node *inv_alloc(void) returns a zeroed block of the node
; size you recovered. Call it with the stack aligned like any C function.

bits 64
default rel

extern inv_alloc
global inv_insert
global inv_find

section .text

inv_insert:
    ; TODO
    mov     rax, rdi
    ret

inv_find:
    ; TODO
    xor     eax, eax
    ret

section .note.GNU-stack noalloc noexec nowrite progbits
