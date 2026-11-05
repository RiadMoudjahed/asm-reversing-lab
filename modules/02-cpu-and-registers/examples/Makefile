BUILD := build
PROGS := 02_registers 02_flags

all: $(addprefix $(BUILD)/,$(PROGS))

$(BUILD)/%.o: %.asm
	mkdir -p $(BUILD)
	nasm -f elf64 -g -F dwarf $< -o $@

$(BUILD)/%: $(BUILD)/%.o
	ld $< -o $@

run: all
	@echo "Built: $(PROGS). Step through them with gdb (see the lesson)."

clean:
	rm -rf $(BUILD)

.PHONY: all run clean
