# Builds and runs the module 10 examples.
BUILD := build
PROGS := $(BUILD)/10_regions $(BUILD)/10_perms $(BUILD)/10_layers

all: $(PROGS)

$(BUILD)/%: %.c
	mkdir -p $(BUILD)
	gcc -std=c11 -Wall -Wextra -no-pie $< -o $@

.PHONY: all
