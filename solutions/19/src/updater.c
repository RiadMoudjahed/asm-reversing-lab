/* Exercise 19 decoy: a benign program that uses the same decoding routine for its own texts. */
#include <stdio.h>
#include "blobs_benign.h"
#include "decode.h"

int main(void)
{
    char text[64];
    const unsigned char *p = TEXTS;
    for (int i = 0; i < TEXTS_COUNT; i++) {
        size_t n = decode_record(p, text);
        puts(text);
        p += 2 + n;
    }
    return 0;
}
