# Builds the exercise 20 machine and its sample program reproducibly.
# Usage: make OUTDIR=<dir>   (default: build)
OUTDIR ?= build
CFLAGS = -std=gnu11 -O1 -fno-pie -fcf-protection=none -fno-asynchronous-unwind-tables \
         -fno-stack-protector -D_FORTIFY_SOURCE=0 -ffile-prefix-map=$(CURDIR)=.
LDFLAGS = -no-pie -s -Wl,--build-id=none
ASM = ../../../exercises/20-capstone-tiny-vm/starter/tvasm.py
ISA = ../isa.json

all: $(OUTDIR)/tinyvm $(OUTDIR)/sample.tvm

$(OUTDIR)/tinyvm: tinyvm.c ref.h
	mkdir -p $(OUTDIR)
	SOURCE_DATE_EPOCH=0 gcc $(CFLAGS) $(LDFLAGS) tinyvm.c -o $@

$(OUTDIR)/sample.tvm: ../sample.tvs $(ISA) $(ASM)
	mkdir -p $(OUTDIR)
	python3 $(ASM) --isa $(ISA) ../sample.tvs $@

.PHONY: all
