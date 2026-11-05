# Builds and runs the module 00 smoke test.
BUILD := build

all: $(BUILD)/00_exit42

$(BUILD)/%.o: %.asm
	mkdir -p $(BUILD)
	nasm -f elf64 $< -o $@

$(BUILD)/%: $(BUILD)/%.o
	ld $< -o $@

run: all
	./$(BUILD)/00_exit42; echo "exit status: $$?"
	bash check_tools.sh

clean:
	rm -rf $(BUILD)

.PHONY: all run clean
