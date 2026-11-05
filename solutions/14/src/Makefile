# Builds the exercise 14 binaries reproducibly.
# Usage: make OUTDIR=<dir>   (default: build)
OUTDIR ?= build
CXXFLAGS = -std=c++17 -O1 -fno-pie -fcf-protection=none -fno-asynchronous-unwind-tables \
           -fno-stack-protector -fno-rtti -fno-exceptions -ffile-prefix-map=$(CURDIR)=.
LDFLAGS = -no-pie -s -Wl,--build-id=none

all: $(OUTDIR)/gadgets $(OUTDIR)/gadgetlib.o

$(OUTDIR)/gadgets: gadgets.cpp classes.h
	mkdir -p $(OUTDIR)
	SOURCE_DATE_EPOCH=0 g++ $(CXXFLAGS) $(LDFLAGS) gadgets.cpp -o $@

$(OUTDIR)/gadgetlib.o: gadgetlib.cpp classes.h
	mkdir -p $(OUTDIR)
	SOURCE_DATE_EPOCH=0 g++ $(CXXFLAGS) -fvisibility=hidden -c gadgetlib.cpp -o $@
	strip --strip-unneeded $@

.PHONY: all
