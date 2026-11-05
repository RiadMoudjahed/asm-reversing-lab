# Builds the exercise 08 challenge binaries reproducibly.
# Usage: make OUTDIR=<dir>   (default: build)
OUTDIR ?= build

all: $(OUTDIR)/callcard32 $(OUTDIR)/callcard64

$(OUTDIR)/callcard32: callcard32.asm
	mkdir -p $(OUTDIR)
	nasm -f elf32 callcard32.asm -o $(OUTDIR)/callcard32.o
	ld -m elf_i386 --build-id=none -z noexecstack -o $@ $(OUTDIR)/callcard32.o
	rm $(OUTDIR)/callcard32.o

$(OUTDIR)/callcard64: callcard64.asm
	mkdir -p $(OUTDIR)
	nasm -f elf64 callcard64.asm -o $(OUTDIR)/callcard64.o
	ld --build-id=none -z noexecstack -o $@ $(OUTDIR)/callcard64.o
	rm $(OUTDIR)/callcard64.o

.PHONY: all
