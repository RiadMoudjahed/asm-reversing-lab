# Builds the exercise 11 challenge binary reproducibly.
# Usage: make OUTDIR=<dir>   (default: build)
OUTDIR ?= build
CFLAGS = -std=gnu11 -O1 -ffile-prefix-map=$(CURDIR)=.
LDFLAGS = -s -Wl,--image-base=0x150000000 -Wl,--no-insert-timestamp -Wl,--file-alignment=0x200 -Wl,--section-alignment=0x2000

all: $(OUTDIR)/puzzle.exe

$(OUTDIR)/puzzle.exe: puzzle.c
	mkdir -p $(OUTDIR)
	SOURCE_DATE_EPOCH=0 x86_64-w64-mingw32-gcc $(CFLAGS) puzzle.c $(LDFLAGS) -ladvapi32 -luser32 -o $@

.PHONY: all
