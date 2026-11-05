# Builds the exercise 02 challenge binaries reproducibly.
# Usage: make OUTDIR=<dir>   (default: build)
OUTDIR ?= build

all: $(OUTDIR)/two_worlds32 $(OUTDIR)/two_worlds64

$(OUTDIR)/two_worlds32: two_worlds32.asm
	mkdir -p $(OUTDIR)
	nasm -f elf32 two_worlds32.asm -o $(OUTDIR)/two_worlds32.o
	ld -m elf_i386 --build-id=none -z noexecstack -o $@ $(OUTDIR)/two_worlds32.o
	rm $(OUTDIR)/two_worlds32.o

$(OUTDIR)/two_worlds64: two_worlds64.asm
	mkdir -p $(OUTDIR)
	nasm -f elf64 two_worlds64.asm -o $(OUTDIR)/two_worlds64.o
	ld --build-id=none -z noexecstack -o $@ $(OUTDIR)/two_worlds64.o
	rm $(OUTDIR)/two_worlds64.o

.PHONY: all
