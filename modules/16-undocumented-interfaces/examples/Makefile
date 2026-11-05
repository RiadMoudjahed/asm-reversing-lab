# Builds the module 16 black-box library and the probe program.
BUILD := build
CFLAGS := -std=c11 -O1 -fno-asynchronous-unwind-tables -fcf-protection=none -fno-stack-protector -Wall -Wextra

all: $(BUILD)/libblackbox.so $(BUILD)/16_probe $(BUILD)/16_caller.s

$(BUILD)/libblackbox.so: 16_blackbox.c
	mkdir -p $(BUILD)
	gcc $(CFLAGS) -fPIC -fvisibility=hidden -shared $< -o $@
	strip --strip-unneeded $@

$(BUILD)/16_probe: 16_probe.c $(BUILD)/libblackbox.so
	gcc -std=c11 -O0 -Wall -Wextra $< -L$(BUILD) -lblackbox -Wl,-rpath,'$$ORIGIN' -o $@

$(BUILD)/16_caller.s: 16_caller.c
	mkdir -p $(BUILD)
	gcc -std=c11 $(CFLAGS) -S -masm=intel $< -o $@

run: all
	$(BUILD)/16_probe

.PHONY: all run
