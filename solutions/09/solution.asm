; Purpose : reference solution for exercise 09, Part 2. A recursive function that builds a
;           three-entry table per level, reports it to note(), and keeps the stack aligned
;           and its footprint small.
; Target  : Linux x86-64, System V AMD64
; Build   : nasm -f elf64 solution.asm -o solution.o

bits 64
default rel

extern note
global chain_asm

section .text

; long chain_asm(long depth, long seed)
;   table[0] = seed ^ (depth * 7)
;   table[1] = seed + depth
;   table[2] = table[0] - table[1]
;   note(depth, table)
;   depth == 0: return seed & 0xff
;   otherwise : return ((chain_asm(depth - 1, seed * 3 + 1) ^ table[0]) * 3) + (table[2] >> 1)
;
; Frame: return address (8) + rbx and rbp (16) + table (24) = 48 bytes per level.
; At entry rsp is 8 modulo 16; after two pushes it is 8 modulo 16 again, so the 24-byte
; table brings it to 0 modulo 16 before each call.
chain_asm:
    push    rbx                         ; rbx = depth, kept across both calls
    push    rbp                         ; rbp = seed, kept across both calls
    sub     rsp, 24                     ; table[0..2] at [rsp], [rsp + 8], [rsp + 16]
    mov     rbx, rdi
    mov     rbp, rsi
    lea     rax, [rdi*8]
    sub     rax, rdi                    ; depth * 7
    xor     rax, rsi                    ; seed ^ (depth * 7)
    mov     [rsp], rax                  ; table[0]
    lea     rdx, [rdi + rsi]
    mov     [rsp + 8], rdx              ; table[1] = seed + depth
    sub     rax, rdx
    mov     [rsp + 16], rax             ; table[2] = table[0] - table[1]
    mov     rsi, rsp                    ; second argument: the table (rdi still holds depth)
    call    note
    test    rbx, rbx
    jne     .recurse
    movzx   eax, bpl                    ; base case: seed & 0xff
    jmp     .done
.recurse:
    lea     rdi, [rbx - 1]
    lea     rsi, [rbp + rbp*2 + 1]      ; seed * 3 + 1
    call    chain_asm
    xor     rax, [rsp]                  ; below ^ table[0]
    lea     rax, [rax + rax*2]          ; * 3
    mov     rdx, [rsp + 16]
    sar     rdx, 1
    add     rax, rdx                    ; + (table[2] >> 1)
.done:
    add     rsp, 24
    pop     rbp
    pop     rbx
    ret

section .note.GNU-stack noalloc noexec nowrite progbits
