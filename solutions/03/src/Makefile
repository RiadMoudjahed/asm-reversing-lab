# Builds the exercise 03 challenge binary reproducibly.
# Usage: make OUTDIR=<dir>   (default: build)
OUTDIR ?= build

all: $(OUTDIR)/ledger

$(OUTDIR)/ledger: ledger.asm
	mkdir -p $(OUTDIR)
	nasm -f elf64 ledger.asm -o $(OUTDIR)/ledger.o
	ld --build-id=none -z noexecstack -o $@ $(OUTDIR)/ledger.o
	rm $(OUTDIR)/ledger.o

.PHONY: all
