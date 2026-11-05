# Builds the module 13 examples at three optimization levels and writes Intel listings.
BUILD := build
FLAGS := -std=c11 -fno-asynchronous-unwind-tables -fcf-protection=none -fno-pie -fno-stack-protector

all: $(BUILD)/13_O0.s $(BUILD)/13_O1.s $(BUILD)/13_O2.s

$(BUILD)/13_O0.s: 13_structs.c
	mkdir -p $(BUILD)
	gcc $(FLAGS) -Wall -Wextra -O0 -S -masm=intel $< -o $@

$(BUILD)/13_O1.s: 13_structs.c
	mkdir -p $(BUILD)
	gcc $(FLAGS) -Wall -Wextra -O1 -S -masm=intel $< -o $@

$(BUILD)/13_O2.s: 13_structs.c
	mkdir -p $(BUILD)
	gcc $(FLAGS) -Wall -Wextra -O2 -S -masm=intel $< -o $@

.PHONY: all
