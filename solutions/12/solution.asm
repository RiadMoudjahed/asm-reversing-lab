; Purpose : reference solution for exercise 12, Part 2. A command dispatcher built on a jump
;           table instead of a chain of comparisons.
; Target  : Linux x86-64, System V AMD64
; Build   : nasm -f elf64 solution.asm -o solution.o

bits 64
default rel

global run_cmd

section .text

; int32_t run_cmd(int32_t cmd, uint32_t acc)       edi = cmd, esi = acc, result in eax
;   The commands 3 to 13 are handled through a table indexed by cmd - 3. Gaps in the table,
;   and every command outside the range, go to the default case.
run_cmd:
    mov     eax, edi
    sub     eax, 3                      ; index = cmd - 3
    cmp     eax, 10                     ; unsigned: negative indexes become huge
    ja      .default
    lea     rdx, [table]
    jmp     [rdx + rax*8]               ; indirect jump through the table

.c3:                                    ; acc += 5, then the code of command 4
    add     esi, 5
.c4:                                    ; acc *= 3, then the common exit
    lea     eax, [rsi + rsi*2 + 1]
    ret
.c5:                                    ; acc -= 2, returned without the final + 1
    lea     eax, [rsi - 2]
    ret
.c6:                                    ; commands 6 and 10: acc ^= 0x55
    xor     esi, 0x55
    lea     eax, [rsi + 1]
    ret
.c7:                                    ; acc <<= 2, then the code of command 8
    shl     esi, 2
.c8:                                    ; acc += 100
    lea     eax, [rsi + 101]
    ret
.c12:                                   ; acc = -acc
    mov     eax, 1
    sub     eax, esi
    ret
.c13:                                   ; acc |= 1
    or      esi, 1
    lea     eax, [rsi + 1]
    ret
.default:                               ; acc = 0xFFFFFFFF, plus 1
    xor     eax, eax
    ret

section .rodata
align 8
table:
    dq run_cmd.c3                              ; 3
    dq run_cmd.c4                              ; 4
    dq run_cmd.c5                              ; 5
    dq run_cmd.c6                              ; 6
    dq run_cmd.c7                              ; 7
    dq run_cmd.c8                              ; 8
    dq run_cmd.default                         ; 9: no case
    dq run_cmd.c6                              ; 10: shares the code of 6
    dq run_cmd.default                         ; 11: no case
    dq run_cmd.c12                             ; 12
    dq run_cmd.c13                             ; 13

section .note.GNU-stack noalloc noexec nowrite progbits
