; Exercise 05, Part 2: add two decimal numbers given as text.
; Target  : Linux x86 (32-bit), ELF, no libc
; Build   : make
; Run     : make run    (type two numbers, one per line)
;
; Behave exactly like bin/ticket_counter: same output, same exit status, for every input.
; System calls (int 0x80): eax = 3 read, 4 write, 1 exit; arguments in ebx, ecx, edx.

bits 32

section .bss
; TODO: reserve buffers for the input and for the digits of the sum.

section .rodata
; TODO: define the texts you need to print.

section .text
global _start

_start:
    ; TODO: read the input from standard input (descriptor 0).
    ; TODO: check both lines, adding up digit by digit from the right with a carry.
    ; TODO: print the result, or the error text, and exit with the right status.
