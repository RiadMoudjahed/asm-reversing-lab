# Builds the exercise 07 challenge binary reproducibly.
# Usage: make OUTDIR=<dir>   (default: build)
OUTDIR ?= build

all: $(OUTDIR)/sweeper

$(OUTDIR)/sweeper: sweeper.asm
	mkdir -p $(OUTDIR)
	nasm -f elf64 sweeper.asm -o $(OUTDIR)/sweeper.o
	ld --build-id=none -z noexecstack -o $@ $(OUTDIR)/sweeper.o
	rm $(OUTDIR)/sweeper.o

.PHONY: all
