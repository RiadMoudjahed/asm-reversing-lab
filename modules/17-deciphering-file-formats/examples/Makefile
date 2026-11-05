# Builds the module 17 target program and creates the sample files.
BUILD := build
CFLAGS := -std=c11 -O1 -fno-pie -no-pie -fno-asynchronous-unwind-tables -fcf-protection=none -fno-stack-protector -Wall -Wextra

all: $(BUILD)/17_reader $(BUILD)/sample.lg

$(BUILD)/17_reader: 17_reader.c
	mkdir -p $(BUILD)
	gcc $(CFLAGS) $< -o $@
	strip $@

$(BUILD)/sample.lg: 17_make.py
	mkdir -p $(BUILD)
	python3 17_make.py $@

run: all
	$(BUILD)/17_reader $(BUILD)/sample.lg

.PHONY: all run
