# Builds the exercise 16 library reproducibly.
# Usage: make OUTDIR=<dir>   (default: build)
OUTDIR ?= build
CFLAGS = -std=gnu11 -O1 -fPIC -fvisibility=hidden -fcf-protection=none -fno-asynchronous-unwind-tables \
         -fno-stack-protector -ffile-prefix-map=$(CURDIR)=.

all: $(OUTDIR)/libsilent.so

$(OUTDIR)/libsilent.so: silent.c
	mkdir -p $(OUTDIR)
	SOURCE_DATE_EPOCH=0 gcc $(CFLAGS) -shared -Wl,--build-id=none -Wl,-soname,libsilent.so silent.c -o $@
	strip --strip-unneeded $@

# Not part of the exercise files: prints the expected output for the test vectors.
$(OUTDIR)/gen_vectors: gen_vectors.c $(OUTDIR)/libsilent.so
	gcc -std=gnu11 -O0 gen_vectors.c $(OUTDIR)/libsilent.so -Wl,-rpath,'$$ORIGIN' -o $@

.PHONY: all
