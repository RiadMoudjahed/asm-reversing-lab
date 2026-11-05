/* Reference implementation of the Part 2 routine in C. Used to generate the test vectors. */
#include <stddef.h>

static inline long ref_decode(const unsigned char *rec, size_t avail, char *out, size_t cap)
{
    if (avail < 2)
        return -1;
    size_t len = rec[0];
    if (avail < 2 + len)
        return -1;
    if (cap < len + 1)
        return -2;
    unsigned char k = rec[1];
    for (size_t i = 0; i < len; i++) {
        k = (unsigned char)((unsigned char)((k << 3) | (k >> 5)) + (unsigned char)i + 0x5d);
        out[i] = (char)(rec[2 + i] ^ k);
    }
    out[len] = 0;
    return (long)len;
}
