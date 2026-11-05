; Prints 5! with the stack machine. Variable 0 is n, variable 1 is the product.
        push 5
        store 0
        push 1
        store 1
loop:   load 0
        jz done
        load 1
        load 0
        mul
        store 1
        load 0
        push 1
        sub
        store 0
        jmp loop
done:   load 1
        print
        halt
