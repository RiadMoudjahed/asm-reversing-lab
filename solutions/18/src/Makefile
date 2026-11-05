# Builds the exercise 18 program reproducibly.
# Usage: make OUTDIR=<dir>   (default: build)
OUTDIR ?= build
CFLAGS = -std=gnu11 -O1 -fno-pie -fcf-protection=none -fno-asynchronous-unwind-tables \
         -fstack-protector-strong -D_FORTIFY_SOURCE=0 -ffile-prefix-map=$(CURDIR)=.
LDFLAGS = -no-pie -s -Wl,--build-id=none

all: $(OUTDIR)/recparse

$(OUTDIR)/recparse: recparse.c
	mkdir -p $(OUTDIR)
	SOURCE_DATE_EPOCH=0 gcc $(CFLAGS) $(LDFLAGS) recparse.c -o $@

.PHONY: all
