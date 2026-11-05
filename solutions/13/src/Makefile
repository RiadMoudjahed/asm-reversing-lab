# Builds the exercise 13 challenge binary reproducibly.
# Usage: make OUTDIR=<dir>   (default: build)
OUTDIR ?= build
CFLAGS = -std=c11 -O2 -fno-pie -fcf-protection=none -fno-asynchronous-unwind-tables \
         -fno-stack-protector -ffile-prefix-map=$(CURDIR)=.
LDFLAGS = -no-pie -Wl,--build-id=none

all: $(OUTDIR)/magic

$(OUTDIR)/magic: magic.c
	mkdir -p $(OUTDIR)
	SOURCE_DATE_EPOCH=0 gcc $(CFLAGS) $(LDFLAGS) magic.c -o $@

.PHONY: all
