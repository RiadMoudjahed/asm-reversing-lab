# Builds the exercise 17 program reproducibly.
# Usage: make OUTDIR=<dir>   (default: build)
OUTDIR ?= build
CFLAGS = -std=gnu11 -O1 -fno-pie -fcf-protection=none -fno-asynchronous-unwind-tables \
         -fno-stack-protector -ffile-prefix-map=$(CURDIR)=.
LDFLAGS = -no-pie -s -Wl,--build-id=none

all: $(OUTDIR)/arkpack

$(OUTDIR)/arkpack: arkpack.c
	mkdir -p $(OUTDIR)
	SOURCE_DATE_EPOCH=0 gcc $(CFLAGS) $(LDFLAGS) arkpack.c -o $@

.PHONY: all
