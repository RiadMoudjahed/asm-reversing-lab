; Purpose : reference solution for exercise 14, Part 2. Builds objects of the recovered classes
;           by hand and calls their virtual functions through the vtable.
; Target  : Linux x86-64, System V AMD64
; Build   : nasm -f elf64 solution.asm -o solution.o
;
; Object layout recovered from bin/gadgets (all sizes in bytes):
;   offset 0   vptr, set to gadget_vptr(kind)
;   offset 8   id         (every class)
;   offset 12  range      (class 1) or lumens (class 3)
;   offset 16  pixels     (class 2)

bits 64
default rel

extern gadget_vptr
global gadget_init
global gadget_call

section .text

; void gadget_init(void *mem, int kind, int a, int b, int c)
;   rdi = mem, esi = kind, edx = a, ecx = b, r8d = c
gadget_init:
    push    rbx
    push    r12
    push    r13
    push    r14
    push    r15                         ; five pushes + the return address keep rsp aligned
    mov     rbx, rdi                    ; mem
    mov     r12d, esi                   ; kind
    mov     r13d, edx                   ; a = id
    mov     r14d, ecx                   ; b
    mov     r15d, r8d                   ; c
    mov     edi, esi
    call    gadget_vptr wrt ..plt
    mov     [rbx], rax                  ; the vptr
    mov     [rbx + 8], r13d             ; id
    cmp     r12d, 1                     ; Sensor: range
    je      .field12
    cmp     r12d, 2                     ; Camera: range and pixels
    je      .camera
    cmp     r12d, 3                     ; Lamp: lumens
    je      .field12
    jmp     .done                       ; Device: id only
.camera:
    mov     [rbx + 16], r15d            ; pixels
.field12:
    mov     [rbx + 12], r14d            ; range (Sensor, Camera) or lumens (Lamp)
.done:
    pop     r15
    pop     r14
    pop     r13
    pop     r12
    pop     rbx
    ret

; int gadget_call(const void *obj, int slot, int arg)
;   rdi = obj, esi = slot, edx = arg. Calls the function in the given vtable slot with the
;   object as the first argument and arg as the second.
gadget_call:
    sub     rsp, 8                      ; align the stack for the call
    mov     rax, [rdi]                  ; vptr
    movsxd  rcx, esi
    mov     esi, edx                    ; arg becomes the second argument
    call    [rax + rcx*8]               ; rdi still holds the object
    add     rsp, 8
    ret

section .note.GNU-stack noalloc noexec nowrite progbits
