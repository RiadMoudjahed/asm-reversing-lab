; Prints 1 + 2 + ... + 10 with the register machine.
; r0 counts down from 10, r1 is the sum, r2 holds the constant 1.
        movi r0, 10
        movi r1, 0
        movi r2, 1
loop:   add  r1, r1, r0
        sub  r0, r0, r2
        jnz  r0, loop
        out  r1
        halt
