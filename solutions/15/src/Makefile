# Builds the exercise 15 binaries reproducibly.
# Usage: make OUTDIR=<dir>   (default: build)
OUTDIR ?= build
CFLAGS = -std=gnu11 -O1 -fno-pie -fcf-protection=none -fno-asynchronous-unwind-tables \
         -fno-stack-protector -ffile-prefix-map=$(CURDIR)=.
LDFLAGS = -no-pie -s -Wl,--build-id=none

all: $(OUTDIR)/inventory $(OUTDIR)/inventorylib.o

$(OUTDIR)/inventory: inventory.c tree.h
	mkdir -p $(OUTDIR)
	SOURCE_DATE_EPOCH=0 gcc $(CFLAGS) $(LDFLAGS) inventory.c -o $@

$(OUTDIR)/inventorylib.o: inventorylib.c tree.h
	mkdir -p $(OUTDIR)
	SOURCE_DATE_EPOCH=0 gcc $(CFLAGS) -fvisibility=hidden -c inventorylib.c -o $@
	strip --strip-unneeded $@

.PHONY: all
