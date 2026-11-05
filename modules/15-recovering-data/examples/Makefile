# Builds the module 15 examples and writes Intel listings.
BUILD := build
CFLAGS := -fno-asynchronous-unwind-tables -fcf-protection=none -fno-pie -fno-stack-protector

all: $(BUILD)/15_data.s $(BUILD)/15_data.o $(BUILD)/15_cpp.s $(BUILD)/15_cpp.o

$(BUILD)/15_data.s: 15_data.c
	mkdir -p $(BUILD)
	gcc -std=c11 $(CFLAGS) -Wall -Wextra -O1 -S -masm=intel $< -o $@

$(BUILD)/15_data.o: 15_data.c
	mkdir -p $(BUILD)
	gcc -std=c11 $(CFLAGS) -O1 -c $< -o $@

$(BUILD)/15_cpp.s: 15_cpp.cpp
	mkdir -p $(BUILD)
	g++ -std=c++17 $(CFLAGS) -fno-rtti -Wall -Wextra -O1 -S -masm=intel $< -o $@

$(BUILD)/15_cpp.o: 15_cpp.cpp
	mkdir -p $(BUILD)
	g++ -std=c++17 $(CFLAGS) -fno-rtti -O1 -c $< -o $@

.PHONY: all
