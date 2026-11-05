; Purpose : reference solution for exercise 08, Part 2. The three functions from Part 1
;           that use stdcall, cdecl and fastcall, each in its own calling convention.
; Target  : Linux x86 (32-bit), ELF
; Build   : nasm -f elf32 solution.asm -o solution.o

bits 32

section .text
global cc_f1, cc_f2, cc_f3

; stdcall: int cc_f1(int a, int b, int c) = 3a + 2b + c; the callee removes 12 bytes
cc_f1:
    mov     eax, [esp + 4]
    lea     eax, [eax + eax*2]          ; 3 * a
    mov     edx, [esp + 8]
    lea     eax, [eax + edx*2]          ; + 2 * b
    add     eax, [esp + 12]             ; + c
    ret     12

; cdecl: int cc_f2(int a, int b, int c) = a + 10b + 100c; the caller removes the arguments
cc_f2:
    mov     eax, [esp + 12]
    imul    eax, eax, 100               ; 100 * c
    mov     edx, [esp + 8]
    lea     edx, [edx + edx*4]
    lea     eax, [eax + edx*2]          ; + 10 * b
    add     eax, [esp + 4]              ; + a
    ret

; fastcall: int cc_f3(int a, int b, int c) = 5a - b + c; a in ecx, b in edx, c on the
; stack; the callee removes 4 bytes
cc_f3:
    lea     eax, [ecx + ecx*4]          ; 5 * a
    sub     eax, edx                    ; - b
    add     eax, [esp + 4]              ; + c
    ret     4

section .note.GNU-stack noalloc noexec nowrite progbits
