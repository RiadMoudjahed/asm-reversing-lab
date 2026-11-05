; Purpose : reference solution for exercise 10, Part 2. A complete 64-bit Linux executable
;           written byte by byte: ELF header, three program headers, code and data.
; Target  : Linux x86-64, ELF
; Build   : nasm -f bin solution.asm -o solution && chmod +x solution
; Output  : "header surgery ok" followed by a newline, exit status 0

bits 64
BASE    equ 0x10000000                  ; image base found in Part 1
DATAOFF equ 0x200                       ; file offset of the data segment
DATAVA  equ BASE + 0x1000 + DATAOFF     ; same offset modulo 0x1000, one page higher

org BASE

ehdr:
    db 0x7f, 'E', 'L', 'F'              ; e_ident: magic
    db 2, 1, 1, 0                       ;   64-bit, little-endian, version 1, System V
    dq 0                                ;   padding
    dw 2                                ; e_type: executable
    dw 0x3e                             ; e_machine: x86-64
    dd 1                                ; e_version
    dq _start                           ; e_entry
    dq phdr - $$                        ; e_phoff
    dq 0                                ; e_shoff: no section headers
    dd 0                                ; e_flags
    dw ehdr_size                        ; e_ehsize
    dw phdr_size                        ; e_phentsize
    dw 3                                ; e_phnum
    dw 0                                ; e_shentsize
    dw 0                                ; e_shnum
    dw 0                                ; e_shstrndx
ehdr_size equ $ - ehdr

phdr:
    ; segment 1: the headers and the code, read + execute
    dd 1                                ; p_type: PT_LOAD
    dd 5                                ; p_flags: R + X
    dq 0                                ; p_offset
    dq BASE                             ; p_vaddr
    dq BASE                             ; p_paddr
    dq code_end - $$                    ; p_filesz
    dq code_end - $$                    ; p_memsz
    dq 0x1000                           ; p_align
phdr_size equ $ - phdr
    ; segment 2: the message, read + write
    dd 1                                ; p_type: PT_LOAD
    dd 6                                ; p_flags: R + W
    dq DATAOFF                          ; p_offset
    dq DATAVA                           ; p_vaddr
    dq DATAVA                           ; p_paddr
    dq msg_len                          ; p_filesz
    dq msg_len                          ; p_memsz
    dq 0x1000                           ; p_align
    ; segment 3: the stack is read + write, not executable
    dd 0x6474e551                       ; p_type: PT_GNU_STACK
    dd 6                                ; p_flags: R + W
    dq 0, 0, 0, 0, 0                    ; offset, vaddr, paddr, filesz, memsz
    dq 0x10                             ; p_align

_start:
    mov     eax, 1                      ; write
    mov     edi, 1                      ; standard output
    mov     esi, DATAVA                 ; the message, in the writable segment
    mov     edx, msg_len
    syscall
    mov     eax, 60                     ; exit
    xor     edi, edi
    syscall
code_end:

    times DATAOFF - ($ - $$) db 0       ; pad the file up to the data offset
msg:
    db "header surgery ok", 10
msg_len equ $ - msg
