# Builds and runs your 32-bit solution.
all: solution

solution: solution.asm
	nasm -f elf32 solution.asm -o solution.o
	ld -m elf_i386 solution.o -o solution

run: solution
	./solution; echo "exit status: $$?"

clean:
	rm -f solution solution.o

.PHONY: all run clean
