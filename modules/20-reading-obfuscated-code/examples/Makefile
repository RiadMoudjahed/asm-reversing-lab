# Builds the module 20 examples.
BUILD := build
CFLAGS := -std=gnu11 -O1 -fno-pie -no-pie -fno-stack-protector -fcf-protection=none -Wall -Wextra

all: $(BUILD)/20_strings $(BUILD)/20_strings_stripped $(BUILD)/20_flat $(BUILD)/20_dbg

20_blobs.h: 20_encode.py
	python3 20_encode.py

$(BUILD)/20_strings: 20_strings.c 20_blobs.h
	mkdir -p $(BUILD)
	gcc $(CFLAGS) 20_strings.c -o $@

$(BUILD)/20_strings_stripped: $(BUILD)/20_strings
	strip -o $@ $<

$(BUILD)/20_flat: 20_flat.c
	mkdir -p $(BUILD)
	gcc $(CFLAGS) 20_flat.c -o $@

$(BUILD)/20_dbg: 20_dbg.c
	mkdir -p $(BUILD)
	gcc $(CFLAGS) 20_dbg.c -o $@

run: all
	$(BUILD)/20_strings
	$(BUILD)/20_flat
	$(BUILD)/20_dbg

.PHONY: all run
