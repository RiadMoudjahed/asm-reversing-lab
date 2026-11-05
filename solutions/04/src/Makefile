# Builds the exercise 04 challenge binary reproducibly.
# Usage: make OUTDIR=<dir>   (default: build)
OUTDIR ?= build

all: $(OUTDIR)/config_probe

$(OUTDIR)/config_probe: config_probe.asm
	mkdir -p $(OUTDIR)
	nasm -f elf64 config_probe.asm -o $(OUTDIR)/config_probe.o
	ld --build-id=none -z noexecstack -o $@ $(OUTDIR)/config_probe.o
	rm $(OUTDIR)/config_probe.o

.PHONY: all
