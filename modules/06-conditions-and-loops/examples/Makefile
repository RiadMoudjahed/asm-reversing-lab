# Builds and runs the module 06 examples.
BUILD := build

all: $(BUILD)/06_main

$(BUILD)/06_control.o: 06_control.asm
	mkdir -p $(BUILD)
	nasm -f elf64 $< -o $@

$(BUILD)/06_main: 06_main.c $(BUILD)/06_control.o
	gcc -std=c11 -Wall -Wextra -no-pie 06_main.c $(BUILD)/06_control.o -o $@

run: all
	./$(BUILD)/06_main

compiled:
	mkdir -p $(BUILD)
	gcc -O1 -fcf-protection=none -c 06_compiled.c -o $(BUILD)/06_compiled.o
	objdump -d -M intel --no-show-raw-insn $(BUILD)/06_compiled.o

clean:
	rm -rf $(BUILD)

.PHONY: all run compiled clean
