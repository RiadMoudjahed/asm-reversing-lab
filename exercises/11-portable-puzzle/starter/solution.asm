; Exercise 11, Part 2: two PE helpers in the Microsoft x64 calling convention.
; Target  : Windows x86-64 (nasm -f win64); the checker links it with a test program and runs
;           it under Wine.
; Build   : nasm -f win64 solution.asm -o solution.obj   (or: make check EX=11)
;
;   uint32_t pe_rva_to_offset(const void *sections, uint32_t count, uint32_t rva);
;   uint32_t pe_entry_offset(const void *file);
;
; log_rva(uint32_t rva) is provided by the tests:  void log_rva(uint32_t rva);
; It destroys every volatile register and writes into its shadow space.

bits 64
default rel

extern log_rva
global pe_rva_to_offset
global pe_entry_offset

section .text

pe_rva_to_offset:
    ; TODO
    mov     eax, 0xFFFFFFFF
    ret

pe_entry_offset:
    ; TODO
    mov     eax, 0xFFFFFFFF
    ret
