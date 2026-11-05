; Purpose : reference solution for exercise 15, Part 2. Insert and find in the recovered tree.
; Target  : Linux x86-64, System V AMD64
; Build   : nasm -f elf64 solution.asm -o solution.o
;
; Node layout recovered from bin/inventory (32 bytes):
;   offset 0   greater   pointer to the subtree of larger keys
;   offset 8   lesser    pointer to the subtree of smaller keys
;   offset 16  key       signed 32-bit
;   offset 20  total     32-bit count of inserts that passed through this node
;   offset 24  count     16-bit count of inserts with exactly this key (wraps at 65536)

bits 64
default rel

extern inv_alloc
global inv_insert
global inv_find

section .text

; struct node *inv_insert(struct node *root, int key)
;   rdi = root, esi = key. Returns the root.
inv_insert:
    push    rbx
    push    r12
    push    r13
    sub     rsp, 16                     ; three pushes + 16 keep rsp aligned for the call
    mov     [rsp], rdi                  ; the root pointer lives on the stack so it has an address
    mov     r12d, esi                   ; key
    mov     r13, rsp                    ; r13 = address of the link that will be followed
.walk:
    mov     rbx, [r13]                  ; node behind the link
    test    rbx, rbx
    jz      .attach                     ; empty link: a new node goes here
    inc     dword [rbx + 20]            ; total: one more insert passed through this node
    mov     eax, [rbx + 16]
    cmp     r12d, eax
    je      .same
    lea     r13, [rbx + 8]              ; smaller or equal keys use the lesser link
    cmovg   r13, rbx                    ; greater keys use the greater link at offset 0
    jmp     .walk
.same:
    inc     word [rbx + 24]             ; count is 16 bits wide
    jmp     .done
.attach:
    call    inv_alloc wrt ..plt         ; zeroed 32-byte block
    mov     [r13], rax                  ; link the new node
    mov     [rax + 16], r12d
    mov     dword [rax + 20], 1
    mov     word [rax + 24], 1
.done:
    mov     rax, [rsp]
    add     rsp, 16
    pop     r13
    pop     r12
    pop     rbx
    ret

; struct node *inv_find(struct node *root, int key)
;   rdi = root, esi = key. Returns the node, or 0.
inv_find:
.next:
    test    rdi, rdi
    jz      .out
    cmp     esi, [rdi + 16]
    je      .out
    mov     rax, [rdi]                  ; greater
    mov     rdx, [rdi + 8]              ; lesser
    cmovle  rax, rdx                    ; key <= node key: lesser (flags still from the cmp)
    mov     rdi, rax
    jmp     .next
.out:
    mov     rax, rdi
    ret

section .note.GNU-stack noalloc noexec nowrite progbits
