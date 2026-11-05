; Purpose : a complete 64-bit Linux executable written byte by byte: ELF header, one program
;           header and the code, in a single file. It exits with status 42.
; Target  : Linux x86-64, ELF
; Build   : nasm -f bin 12_tiny_elf.asm -o 12_tiny_elf && chmod +x 12_tiny_elf
; Run     : ./12_tiny_elf; echo $?   (prints 42)

bits 64
org 0x400000                        ; the virtual address where the file is loaded

ehdr:
    db 0x7f, 'E', 'L', 'F'          ; e_ident: magic number
    db 2                            ;   class: 64-bit
    db 1                            ;   data: little-endian
    db 1                            ;   version
    db 0                            ;   OS ABI: System V
    dq 0                            ;   padding
    dw 2                            ; e_type: executable
    dw 0x3e                         ; e_machine: x86-64
    dd 1                            ; e_version
    dq _start                       ; e_entry: virtual address of the first instruction
    dq phdr - $$                    ; e_phoff: file offset of the program header table
    dq 0                            ; e_shoff: no section header table
    dd 0                            ; e_flags
    dw ehdrsize                     ; e_ehsize
    dw phdrsize                     ; e_phentsize
    dw 1                            ; e_phnum: one program header
    dw 0                            ; e_shentsize
    dw 0                            ; e_shnum
    dw 0                            ; e_shstrndx
ehdrsize equ $ - ehdr

phdr:
    dd 1                            ; p_type: PT_LOAD
    dd 5                            ; p_flags: read + execute
    dq 0                            ; p_offset: map the file from its first byte
    dq $$                           ; p_vaddr
    dq $$                           ; p_paddr
    dq filesize                     ; p_filesz
    dq filesize                     ; p_memsz
    dq 0x1000                       ; p_align
phdrsize equ $ - phdr

_start:
    mov     edi, 42                 ; exit status
    mov     eax, 60                 ; system call number of exit
    syscall

filesize equ $ - $$
