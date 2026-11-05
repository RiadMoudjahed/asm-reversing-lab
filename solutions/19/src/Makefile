# Builds the exercise 19 programs reproducibly, for Linux and for Windows.
# Usage: make OUTDIR=<dir> DECOYDIR=<dir>
OUTDIR ?= build
DECOYDIR ?= $(OUTDIR)/decoys
CFLAGS = -std=gnu11 -O1 -fno-pie -fcf-protection=none -fno-asynchronous-unwind-tables \
         -fno-stack-protector -ffile-prefix-map=$(CURDIR)=.
LDFLAGS = -no-pie -s -Wl,--build-id=none
WFLAGS = -std=gnu11 -O1 -Wl,--no-insert-timestamp -s -ffile-prefix-map=$(CURDIR)=.

PROGS = $(OUTDIR)/dispatch $(OUTDIR)/dispatch.exe
DECOYS = $(DECOYDIR)/notes $(DECOYDIR)/notes.exe $(DECOYDIR)/dryrun $(DECOYDIR)/dryrun.exe \
         $(DECOYDIR)/updater $(DECOYDIR)/updater.exe

all: $(PROGS) $(DECOYS)

blobs.h blobs_benign.h: encode.py
	python3 encode.py

$(OUTDIR)/dispatch: dispatch.c blobs.h decode.h
	mkdir -p $(OUTDIR)
	SOURCE_DATE_EPOCH=0 gcc $(CFLAGS) $(LDFLAGS) dispatch.c -o $@

$(OUTDIR)/dispatch.exe: dispatch.c blobs.h decode.h
	mkdir -p $(OUTDIR)
	SOURCE_DATE_EPOCH=0 x86_64-w64-mingw32-gcc $(WFLAGS) dispatch.c -o $@

$(DECOYDIR)/%: %.c blobs_benign.h decode.h
	mkdir -p $(DECOYDIR)
	SOURCE_DATE_EPOCH=0 gcc $(CFLAGS) $(LDFLAGS) $< -o $@

$(DECOYDIR)/%.exe: %.c blobs_benign.h decode.h
	mkdir -p $(DECOYDIR)
	SOURCE_DATE_EPOCH=0 x86_64-w64-mingw32-gcc $(WFLAGS) $< -o $@

.PHONY: all
