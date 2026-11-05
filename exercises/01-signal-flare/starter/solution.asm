; Exercise 01, Part 2: reproduce signal_flare.
; Target  : Linux x86-64, ELF, no libc
; Build   : make
; Run     : make run

bits 64
default rel

; TODO: define named constants for the system call numbers and file descriptors.

section .rodata
; TODO: define the text that goes to standard output, with its length.
; TODO: define the text that goes to standard error, with its length.

section .text
global _start

_start:
    ; TODO: write the report to standard output.
    ; TODO: write the notice to standard error.
    ; TODO: end the program with the exit status you found in Part 1.
