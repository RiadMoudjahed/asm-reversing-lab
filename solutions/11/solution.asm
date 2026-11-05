; Purpose : reference solution for exercise 11, Part 2. Two PE helpers in the Microsoft x64
;           calling convention: RVA to file offset, and the file offset of the entry point.
; Target  : Windows x86-64 (built with nasm -f win64, run under Wine in the checker)
; Build   : nasm -f win64 solution.asm -o solution.obj

bits 64
default rel

extern log_rva
global pe_rva_to_offset
global pe_entry_offset

section .text

; uint32_t pe_rva_to_offset(const void *sections, uint32_t count, uint32_t rva)
;   rcx = section table (40-byte entries), edx = count, r8d = rva
;   Entry fields used: VirtualAddress at +12, SizeOfRawData at +16, PointerToRawData at +20.
;   Returns the file offset, or 0xFFFFFFFF when no section holds the RVA in its raw data.
pe_rva_to_offset:
    test    edx, edx
    jz      .none
.next:
    mov     r10d, r8d
    sub     r10d, [rcx + 12]            ; rva - VirtualAddress (wraps when rva is below it)
    cmp     r10d, [rcx + 16]            ; unsigned: a wrapped value is never below SizeOfRawData
    jae     .skip
    lea     eax, [r10 + 0]
    add     eax, [rcx + 20]             ; + PointerToRawData
    ret
.skip:
    add     rcx, 40
    dec     edx
    jnz     .next
.none:
    mov     eax, 0xFFFFFFFF
    ret

; uint32_t pe_entry_offset(const void *file)
;   rcx = the start of a PE file in memory. Returns the file offset of the entry point, or
;   0xFFFFFFFF if the signatures are wrong. It calls log_rva(entry RVA) once before converting.
pe_entry_offset:
    push    rbx                         ; rbx = address of the NT headers (callee-saved)
    sub     rsp, 48                     ; 32 bytes of shadow space + 16 for a local
    cmp     word [rcx], 0x5A4D          ; "MZ"
    jne     .bad
    mov     eax, [rcx + 0x3C]           ; e_lfanew
    lea     rbx, [rcx + rax]
    cmp     dword [rbx], 0x00004550     ; "PE\0\0"
    jne     .bad
    mov     ecx, [rbx + 24 + 16]        ; AddressOfEntryPoint in the optional header
    mov     [rsp + 32], ecx
    call    log_rva
    movzx   eax, word [rbx + 20]        ; SizeOfOptionalHeader
    lea     rcx, [rbx + 24 + rax]       ; the section table follows the optional header
    movzx   edx, word [rbx + 6]         ; NumberOfSections
    mov     r8d, [rsp + 32]             ; the entry RVA, saved across the call
    call    pe_rva_to_offset
    jmp     .out
.bad:
    mov     eax, 0xFFFFFFFF
.out:
    add     rsp, 48
    pop     rbx
    ret
