# Builds the module 19 samples: a harmless simulator and a harmless helper, for Linux and Windows.
BUILD := build
CFLAGS := -std=gnu11 -O1 -fno-inline -Wall -Wextra

all: $(BUILD)/19_sim $(BUILD)/19_sim.exe $(BUILD)/19_helper $(BUILD)/19_helper.exe

$(BUILD)/19_sim: 19_sim.c
	mkdir -p $(BUILD)
	gcc $(CFLAGS) -fno-pie -no-pie $< -o $@

$(BUILD)/19_sim.exe: 19_sim.c
	mkdir -p $(BUILD)
	x86_64-w64-mingw32-gcc $(CFLAGS) $< -o $@

$(BUILD)/19_helper: 19_helper.c
	mkdir -p $(BUILD)
	gcc $(CFLAGS) -fno-pie -no-pie $< -o $@

$(BUILD)/19_helper.exe: 19_helper.c
	mkdir -p $(BUILD)
	x86_64-w64-mingw32-gcc $(CFLAGS) $< -o $@

run: all
	$(BUILD)/19_sim

.PHONY: all run
