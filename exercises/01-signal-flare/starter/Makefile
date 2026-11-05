# Builds and runs your solution.
all: solution

solution: solution.asm
	nasm -f elf64 solution.asm -o solution.o
	ld solution.o -o solution

run: solution
	./solution; echo "exit status: $$?"

clean:
	rm -f solution solution.o

.PHONY: all run clean
