# Builds and runs the module 12 examples.
BUILD := build

all: $(BUILD)/12_tiny_elf $(BUILD)/12_sample $(BUILD)/12_win.exe

$(BUILD)/12_tiny_elf: 12_tiny_elf.asm
	mkdir -p $(BUILD)
	nasm -f bin $< -o $@
	chmod +x $@

$(BUILD)/12_sample: 12_sample.c
	mkdir -p $(BUILD)
	gcc -std=c11 -Wall -Wextra -O1 -no-pie -fcf-protection=none $< -o $@

$(BUILD)/12_win.exe: 12_win.c
	mkdir -p $(BUILD)
	x86_64-w64-mingw32-gcc -Wall -O1 -s $< -o $@

.PHONY: all
