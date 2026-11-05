; Purpose : challenge program for exercise 01. Writes a status report to
;           standard output, a burn-time notice to standard error, then exits.
; Target  : Linux x86-64, ELF, no libc
; Build   : make OUTDIR=<dir>
; Output  : see exercise 01 (the analysis is the exercise)

bits 64
default rel

section .rodata
report:     db "FLARE LAUNCHED", 10
            db "COLOR: RED", 10
            db "ALTITUDE: 120 m", 10
report_len  equ $ - report

notice:     db "[flare] burn time 30 s", 10
notice_len  equ $ - notice

section .text
global _start

_start:
    mov     eax, 1              ; write
    mov     edi, 1              ; standard output
    lea     rsi, [report]
    mov     edx, report_len
    syscall

    mov     eax, 1              ; write
    mov     edi, 2              ; standard error
    lea     rsi, [notice]
    mov     edx, notice_len
    syscall

    mov     eax, 60             ; exit
    mov     edi, 7
    syscall
