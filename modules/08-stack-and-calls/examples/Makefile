# Builds and runs the module 08 examples.
BUILD := build

all: $(BUILD)/08_main $(BUILD)/08_printf $(BUILD)/08_conv32 $(BUILD)/08_win64.exe

$(BUILD)/08_frames.o: 08_frames.asm
	mkdir -p $(BUILD)
	nasm -f elf64 $< -o $@

$(BUILD)/08_macros.o: 08_macros.asm 08_frame.inc
	mkdir -p $(BUILD)
	nasm -f elf64 $< -o $@

$(BUILD)/08_main: 08_main.c $(BUILD)/08_frames.o $(BUILD)/08_macros.o
	gcc -std=c11 -Wall -Wextra -no-pie 08_main.c $(BUILD)/08_frames.o $(BUILD)/08_macros.o -o $@

$(BUILD)/08_printf.o: 08_printf.asm
	mkdir -p $(BUILD)
	nasm -f elf64 $< -o $@

$(BUILD)/08_printf: $(BUILD)/08_printf.o
	gcc -no-pie $< -o $@

$(BUILD)/08_conv32.o: 08_conv32.asm
	mkdir -p $(BUILD)
	nasm -f elf32 $< -o $@

$(BUILD)/08_conv32: 08_conv32_main.c $(BUILD)/08_conv32.o
	gcc -m32 -std=c11 -Wall -Wextra -no-pie 08_conv32_main.c $(BUILD)/08_conv32.o -o $@

$(BUILD)/08_win64.obj: 08_win64.asm
	mkdir -p $(BUILD)
	nasm -f win64 $< -o $@

$(BUILD)/08_win64.exe: 08_win64_main.c $(BUILD)/08_win64.obj
	x86_64-w64-mingw32-gcc -std=c11 -Wall -Wextra 08_win64_main.c $(BUILD)/08_win64.obj -o $@

run: all
	@echo "== System V (64-bit)"; ./$(BUILD)/08_main
	@echo "== assembly calling printf"; ./$(BUILD)/08_printf
	@echo "== 32-bit conventions"; ./$(BUILD)/08_conv32
	@echo "== Microsoft x64 under Wine"; wine ./$(BUILD)/08_win64.exe

compiled:
	mkdir -p $(BUILD)
	@for opt in -O0 -O2; do \
	  echo "== gcc $$opt"; \
	  gcc $$opt -fcf-protection=none -c 08_compiled.c -o $(BUILD)/08_compiled.o; \
	  objdump -d -M intel --no-show-raw-insn $(BUILD)/08_compiled.o | grep -vE '^$$|file format|Disassembly'; \
	done

clean:
	rm -rf $(BUILD)

.PHONY: all run compiled clean
