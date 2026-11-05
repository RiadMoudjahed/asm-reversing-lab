# Builds and runs the module 07 examples.
BUILD := build

all: $(BUILD)/07_main $(BUILD)/07_prefixed

$(BUILD)/07_strings.o: 07_strings.asm
	mkdir -p $(BUILD)
	nasm -f elf64 $< -o $@

$(BUILD)/07_main: 07_main.c $(BUILD)/07_strings.o
	gcc -std=c11 -Wall -Wextra -no-pie 07_main.c $(BUILD)/07_strings.o -o $@

$(BUILD)/07_prefixed.o: 07_prefixed.asm
	mkdir -p $(BUILD)
	nasm -f elf64 $< -o $@

$(BUILD)/07_prefixed: $(BUILD)/07_prefixed.o
	ld $< -o $@

run: all
	./$(BUILD)/07_main
	./$(BUILD)/07_prefixed

compiled:
	mkdir -p $(BUILD)
	@for opt in -O1 -Os; do \
	  echo "== gcc $$opt"; \
	  gcc $$opt -fcf-protection=none -c 07_compiled.c -o $(BUILD)/07_compiled.o; \
	  objdump -d -M intel --no-show-raw-insn $(BUILD)/07_compiled.o | grep -vE '^$$|file format|Disassembly'; \
	done

clean:
	rm -rf $(BUILD)

.PHONY: all run compiled clean
