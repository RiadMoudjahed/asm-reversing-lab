# Builds the exercise 05 challenge binary reproducibly.
# Usage: make OUTDIR=<dir>   (default: build)
OUTDIR ?= build

all: $(OUTDIR)/ticket_counter

$(OUTDIR)/ticket_counter: ticket_counter.asm
	mkdir -p $(OUTDIR)
	nasm -f elf32 ticket_counter.asm -o $(OUTDIR)/ticket_counter.o
	ld -m elf_i386 --build-id=none -z noexecstack -o $@ $(OUTDIR)/ticket_counter.o
	rm $(OUTDIR)/ticket_counter.o

.PHONY: all
