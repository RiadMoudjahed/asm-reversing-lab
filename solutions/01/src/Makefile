# Builds the exercise 01 challenge binary reproducibly.
# Usage: make OUTDIR=<dir>   (default: build)
OUTDIR ?= build

all: $(OUTDIR)/signal_flare

$(OUTDIR)/signal_flare: signal_flare.asm
	mkdir -p $(OUTDIR)
	nasm -f elf64 signal_flare.asm -o $(OUTDIR)/signal_flare.o
	ld --build-id=none -z noexecstack -o $@ $(OUTDIR)/signal_flare.o
	rm $(OUTDIR)/signal_flare.o

.PHONY: all
