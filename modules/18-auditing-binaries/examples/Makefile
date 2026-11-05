# Builds the module 18 examples: plain objects for disassembly and AddressSanitizer programs.
BUILD := build
CFLAGS := -std=gnu11 -fno-asynchronous-unwind-tables -fcf-protection=none -fno-stack-protector -Wall -Wextra
ASAN := -fsanitize=address -fno-omit-frame-pointer -g -O0

all: $(BUILD)/18_bugs.o $(BUILD)/18_fixed.o $(BUILD)/demo_bug $(BUILD)/demo_fix $(BUILD)/demo_plain

# -fno-builtin and no fortification keep the library calls visible, as in many real binaries.
$(BUILD)/18_bugs.o: 18_bugs.c 18_api.h
	mkdir -p $(BUILD)
	gcc $(CFLAGS) -fno-pie -O1 -fno-builtin -mstringop-strategy=libcall -D_FORTIFY_SOURCE=0 -c 18_bugs.c -o $@

$(BUILD)/18_fixed.o: 18_fixed.c 18_api.h
	mkdir -p $(BUILD)
	gcc $(CFLAGS) -fno-pie -O1 -fno-builtin -mstringop-strategy=libcall -D_FORTIFY_SOURCE=0 -c 18_fixed.c -o $@

# The same bugs without the sanitizer, for use with the debugger. Run it only under the debugger
# and stop before the faulty copy.
$(BUILD)/demo_plain: 18_demo_bug.c 18_bugs.c 18_api.h
	mkdir -p $(BUILD)
	gcc $(CFLAGS) -fno-pie -no-pie -O0 -g -D_FORTIFY_SOURCE=0 18_demo_bug.c 18_bugs.c -o $@

$(BUILD)/demo_bug: 18_demo_bug.c 18_bugs.c 18_api.h
	mkdir -p $(BUILD)
	gcc $(CFLAGS) $(ASAN) 18_demo_bug.c 18_bugs.c -o $@

$(BUILD)/demo_fix: 18_demo_fix.c 18_fixed.c 18_api.h
	mkdir -p $(BUILD)
	gcc $(CFLAGS) $(ASAN) 18_demo_fix.c 18_fixed.c -o $@

# Prints the first lines of each report (the addresses change from run to run).
run: all
	@for c in name label table block greet finish; do echo "== $$c"; ASAN_OPTIONS=detect_leaks=0 $(BUILD)/demo_bug $$c 2>&1 | awk 'NF==0{exit} /ERROR|of size|#[0-9]/' | grep -E "ERROR|of size|18_(bugs|demo_bug)" | sed -E 's/0x[0-9a-f]+/ADDR/g; s/==[0-9]+==//; s#/[^ ]*/(18_[a-z_]+\.c)#\1#' | head -4; done
	@echo "== fixed"; ASAN_OPTIONS=detect_leaks=0 $(BUILD)/demo_fix

.PHONY: all run
