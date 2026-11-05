# Builds the module 23 examples.
BUILD := build
CFLAGS := -std=gnu11 -O2 -fPIC -shared -fcf-protection=none -fno-asynchronous-unwind-tables

all: $(BUILD)/libcases.so $(BUILD)/libcand.so

$(BUILD)/libcases.so: 23_cases.c
	mkdir -p $(BUILD)
	gcc $(CFLAGS) 23_cases.c -o $@

$(BUILD)/libcand.so: 23_candidates.c
	mkdir -p $(BUILD)
	gcc -std=gnu11 -O1 -fPIC -shared -fwrapv -Wno-parentheses 23_candidates.c -o $@

run: all
	python3 23_verify.py $(BUILD)/libcases.so $(BUILD)/libcand.so
	python3 23_lift.py $(BUILD)/libcases.so is_pow2 --stage struct

.PHONY: all run
