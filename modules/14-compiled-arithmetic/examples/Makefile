# Builds the module 14 examples at -O0 and -O2 and writes Intel listings.
BUILD := build
FLAGS := -std=c11 -fno-asynchronous-unwind-tables -fcf-protection=none -fno-pie -fno-stack-protector

all: $(BUILD)/14_O0.s $(BUILD)/14_O2.s

$(BUILD)/14_O0.s: 14_arith.c
	mkdir -p $(BUILD)
	gcc $(FLAGS) -Wall -Wextra -O0 -S -masm=intel $< -o $@

$(BUILD)/14_O2.s: 14_arith.c
	mkdir -p $(BUILD)
	gcc $(FLAGS) -Wall -Wextra -O2 -S -masm=intel $< -o $@

.PHONY: all
