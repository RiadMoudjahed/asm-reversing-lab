/* Reference implementation of the Part 2 functions in C. Used to generate the test vectors. */
#include <stddef.h>
#include <stdint.h>

static inline uint32_t rd32(const unsigned char *p) { return p[0] | p[1] << 8 | p[2] << 16 | (uint32_t)p[3] << 24; }

static inline long ref_count(const unsigned char *a, size_t n)
{
    if (n < 16 || a[0] != 'A' || a[1] != 'R' || a[2] != 'K' || a[3] != '2')
        return -1;
    uint64_t count = a[4] | a[5] << 8, table = rd32(a + 8), names = rd32(a + 12);
    if (table < 16 || table + 16 * count > n || names > n)
        return -1;
    return (long)count;
}

static inline long ref_name(const unsigned char *a, size_t n, long idx, char *out, size_t cap)
{
    long count = ref_count(a, n);
    if (count < 0 || (unsigned long)idx >= (unsigned long)count)
        return -1;
    const unsigned char *e = a + rd32(a + 8) + 16 * (size_t)idx;
    uint64_t p = (uint64_t)rd32(a + 12) + rd32(e);
    if (p >= n)
        return -2;
    size_t len = 0;
    while (p + len < n && a[p + len])
        len++;
    if (p + len >= n)
        return -2;
    if (len + 1 > cap)
        return -3;
    for (size_t i = 0; i <= len; i++)
        out[i] = (char)a[p + i];
    return (long)len;
}

static inline long ref_extract(const unsigned char *a, size_t n, long idx, unsigned char *out, size_t cap)
{
    long count = ref_count(a, n);
    if (count < 0 || (unsigned long)idx >= (unsigned long)count)
        return -1;
    const unsigned char *e = a + rd32(a + 8) + 16 * (size_t)idx;
    uint64_t off = rd32(e + 4), size = rd32(e + 8);
    uint32_t stored = rd32(e + 12);
    if (off < 16 || off + size > n)
        return -2;
    if (size > cap)
        return -3;
    uint32_t c = 0x13579bdfu;
    for (uint64_t i = 0; i < size; i++) {
        unsigned char b = a[off + i] ^ (unsigned char)(a[6] + i);
        out[i] = b;
        c = ((c << 7) | (c >> 25)) + b;
    }
    if ((c ^ (uint32_t)size) != stored)
        return -4;
    return (long)size;
}
