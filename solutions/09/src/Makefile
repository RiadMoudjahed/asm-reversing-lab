# Builds the exercise 09 challenge binary reproducibly.
# Usage: make OUTDIR=<dir>   (default: build)
OUTDIR ?= build
CFLAGS = -std=c11 -O2 -fno-pie -fcf-protection=none -fno-stack-protector \
         -fno-asynchronous-unwind-tables -DEXPECTED=670953052L -ffile-prefix-map=$(CURDIR)=.
LDFLAGS = -no-pie -Wl,--build-id=none

all: $(OUTDIR)/stackdig

$(OUTDIR)/stackdig: stackdig.c
	mkdir -p $(OUTDIR)
	SOURCE_DATE_EPOCH=0 gcc $(CFLAGS) $(LDFLAGS) stackdig.c -o $@

.PHONY: all
