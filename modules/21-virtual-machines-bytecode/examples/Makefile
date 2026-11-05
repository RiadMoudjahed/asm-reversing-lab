# Builds the module 21 examples.
BUILD := build
CFLAGS := -std=gnu11 -O1 -fno-pie -no-pie -fno-stack-protector -fcf-protection=none -Wall -Wextra

all: $(BUILD)/21_stackvm $(BUILD)/21_stackvm_stripped $(BUILD)/21_regvm $(BUILD)/21_regvm_stripped \
     $(BUILD)/fact.bin $(BUILD)/sum.bin

$(BUILD)/21_stackvm: 21_stackvm.c
	mkdir -p $(BUILD)
	gcc $(CFLAGS) 21_stackvm.c -o $@

$(BUILD)/21_regvm: 21_regvm.c
	mkdir -p $(BUILD)
	gcc $(CFLAGS) 21_regvm.c -o $@

$(BUILD)/%_stripped: $(BUILD)/%
	strip -o $@ $<

$(BUILD)/fact.bin: programs/fact.asm 21_asm.py
	mkdir -p $(BUILD)
	python3 21_asm.py stack programs/fact.asm $@

$(BUILD)/sum.bin: programs/sum.asm 21_asm.py
	mkdir -p $(BUILD)
	python3 21_asm.py reg programs/sum.asm $@

run: all
	$(BUILD)/21_stackvm $(BUILD)/fact.bin
	$(BUILD)/21_regvm $(BUILD)/sum.bin

.PHONY: all run
