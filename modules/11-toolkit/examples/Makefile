# Builds the module 11 practice program.
BUILD := build

all: $(BUILD)/11_target

$(BUILD)/11_target: 11_target.c
	mkdir -p $(BUILD)
	gcc -std=c11 -Wall -Wextra -O1 -no-pie -fcf-protection=none 11_target.c -o $@

.PHONY: all
