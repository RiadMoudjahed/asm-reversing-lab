# Builds and runs the module 05 examples.
BUILD := build

all: $(BUILD)/05_main $(BUILD)/05_bcd32

$(BUILD)/05_arith.o: 05_arith.asm
	mkdir -p $(BUILD)
	nasm -f elf64 $< -o $@

$(BUILD)/05_main: 05_main.c $(BUILD)/05_arith.o
	gcc -std=c11 -Wall -Wextra -no-pie 05_main.c $(BUILD)/05_arith.o -o $@

$(BUILD)/05_bcd32.o: 05_bcd32.asm
	mkdir -p $(BUILD)
	nasm -f elf32 $< -o $@

$(BUILD)/05_bcd32: $(BUILD)/05_bcd32.o
	ld -m elf_i386 $< -o $@

run: all
	./$(BUILD)/05_main
	./$(BUILD)/05_bcd32; echo "05_bcd32 exit status: $$?"

compiled:
	mkdir -p $(BUILD)
	gcc -O1 -fcf-protection=none -c 05_compiled.c -o $(BUILD)/05_compiled.o
	objdump -d -M intel --no-show-raw-insn $(BUILD)/05_compiled.o

clean:
	rm -rf $(BUILD)

.PHONY: all run compiled clean
