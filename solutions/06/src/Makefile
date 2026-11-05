# Builds the exercise 06 challenge binary reproducibly.
# Usage: make OUTDIR=<dir>   (default: build)
OUTDIR ?= build
CFLAGS = -std=c11 -O1 -fno-pie -fcf-protection=none -fno-asynchronous-unwind-tables \
         -ffile-prefix-map=$(CURDIR)=.
LDFLAGS = -no-pie -Wl,--build-id=none

all: $(OUTDIR)/maze

$(OUTDIR)/maze: maze.c
	mkdir -p $(OUTDIR)
	SOURCE_DATE_EPOCH=0 gcc $(CFLAGS) $(LDFLAGS) maze.c -o $@

.PHONY: all
