# Module 21: print one line per executed bytecode instruction of build/21_stackvm_stripped.
# The address of the dispatch jump comes from the disassembly of this exact build:
#   4012a3: jmp QWORD PTR [rax*8+0x402078]   (rax = opcode, r8 = offset of the opcode, rbx = depth)
set pagination off
break *0x4012a3
commands
  silent
  printf "offset %3d  opcode %02x  stack depth %d\n", $r8, $rax, $rbx
  continue
end
run build/fact.bin
