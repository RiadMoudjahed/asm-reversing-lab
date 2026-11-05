# Builds the exercise 10 challenge binary reproducibly.
# Usage: make OUTDIR=<dir>   (default: build)
OUTDIR ?= build
CFLAGS = -std=c11 -O1 -fno-pie -fcf-protection=none -fno-stack-protector \
         -fno-asynchronous-unwind-tables -ffile-prefix-map=$(CURDIR)=.
LDFLAGS = -no-pie -Wl,--build-id=none -Wl,-Ttext-segment=0x10000000 -Wl,-z,noseparate-code

all: $(OUTDIR)/surgeon

$(OUTDIR)/surgeon: surgeon.c
	mkdir -p $(OUTDIR)
	SOURCE_DATE_EPOCH=0 gcc $(CFLAGS) surgeon.c $(LDFLAGS) -lm -o $@

.PHONY: all
