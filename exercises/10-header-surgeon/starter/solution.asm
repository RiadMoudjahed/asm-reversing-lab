; Exercise 10, Part 2: write a complete Linux executable, byte by byte.
; Target  : Linux x86-64, ELF
; Build   : nasm -f bin solution.asm -o solution   (or: make check EX=10)
;
; The file you produce must contain an ELF header, program headers, code and data, and
; nothing else is added for you. The README lists the requirements.
;
; Useful directives: org (sets the address that labels and $$ refer to), db, dw, dd, dq,
; equ, times. Use "$" for the current position and "$$" for the start of the file.

bits 64

; TODO: choose the image base, then write the ELF header, the program headers, the code
; and the message.
