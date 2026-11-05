# Builds and runs the module 03 examples.
BUILD := build
PROGS64 := 03_hello64 03_data_bss
PROGS32 := 03_hello32

all: $(addprefix $(BUILD)/,$(PROGS64) $(PROGS32))

$(BUILD)/%.o: %.asm
	mkdir -p $(BUILD)
	nasm -f $(if $(filter $*,$(PROGS32)),elf32,elf64) -g -F dwarf $< -o $@

$(addprefix $(BUILD)/,$(PROGS64)): $(BUILD)/%: $(BUILD)/%.o
	ld $< -o $@

$(addprefix $(BUILD)/,$(PROGS32)): $(BUILD)/%: $(BUILD)/%.o
	ld -m elf_i386 $< -o $@

run: all
	@for p in $(PROGS64) $(PROGS32); do echo "== $$p"; ./$(BUILD)/$$p; done

clean:
	rm -rf $(BUILD)

.PHONY: all run clean
