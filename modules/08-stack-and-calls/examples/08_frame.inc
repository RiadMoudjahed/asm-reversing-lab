; Purpose : NASM macros that write the standard function prologue and epilogue.
; Use     : %include "08_frame.inc" at the top of an assembly file.

; FUNC_ENTER n : set up a frame with n bytes of local variables (n a multiple of 16
; keeps rsp aligned when the function makes calls)
%macro FUNC_ENTER 1
    push    rbp
    mov     rbp, rsp
    sub     rsp, %1
%endmacro

; FUNC_LEAVE : tear the frame down and return
%macro FUNC_LEAVE 0
    leave
    ret
%endmacro
