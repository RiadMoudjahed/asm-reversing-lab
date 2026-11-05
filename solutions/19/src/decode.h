/* Exercise 19: the decoding routine shared by the simulator and one benign decoy. */
#ifndef DECODE_H
#define DECODE_H
#include <stddef.h>

__attribute__((noinline)) static size_t decode_record(const unsigned char *rec, char *out)
{
    size_t len = rec[0];
    unsigned char k = rec[1];
    for (size_t i = 0; i < len; i++) {
        k = (unsigned char)((unsigned char)((k << 3) | (k >> 5)) + i + 0x5d);
        out[i] = (char)(rec[2 + i] ^ k);
    }
    out[len] = '\0';
    return len;
}
#endif
