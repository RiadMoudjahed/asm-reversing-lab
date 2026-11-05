BUILD := build

all: $(BUILD)/01_representation

$(BUILD)/01_representation: 01_representation.c
	mkdir -p $(BUILD)
	gcc -std=c11 -Wall -Wextra -o $@ $<

run: all
	./$(BUILD)/01_representation

clean:
	rm -rf $(BUILD)

.PHONY: all run clean
