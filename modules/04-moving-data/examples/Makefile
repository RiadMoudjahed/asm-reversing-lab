# Builds and runs the module 04 examples.
BUILD := build

all: $(BUILD)/04_main

$(BUILD)/04_addressing.o: 04_addressing.asm
	mkdir -p $(BUILD)
	nasm -f elf64 $< -o $@

$(BUILD)/04_main: 04_main.c $(BUILD)/04_addressing.o
	gcc -std=c11 -Wall -Wextra -no-pie 04_main.c $(BUILD)/04_addressing.o -o $@

run: all
	./$(BUILD)/04_main

compiled:
	mkdir -p $(BUILD)
	gcc -O1 -fcf-protection=none -c 04_compiled.c -o $(BUILD)/04_compiled.o
	objdump -d -M intel --no-show-raw-insn $(BUILD)/04_compiled.o

clean:
	rm -rf $(BUILD)

.PHONY: all run compiled clean
