# Builds the exercise 12 challenge binary reproducibly.
# Usage: make OUTDIR=<dir>   (default: build)
OUTDIR ?= build
CFLAGS = -std=c11 -O2 -fno-pie -fcf-protection=none -fno-asynchronous-unwind-tables \
         -fno-stack-protector -ffile-prefix-map=$(CURDIR)=.
LDFLAGS = -no-pie -Wl,--build-id=none

all: $(OUTDIR)/switchboard

$(OUTDIR)/switchboard: switchboard.c
	mkdir -p $(OUTDIR)
	SOURCE_DATE_EPOCH=0 gcc $(CFLAGS) $(LDFLAGS) switchboard.c -o $@

.PHONY: all
