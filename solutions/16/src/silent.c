/* Exercise 16: the library that ships without a header. Build with the Makefile here. */
#include <stdint.h>
#include <stdlib.h>

#define API __attribute__((visibility("default")))

struct ctx {
    uint32_t a;
    uint32_t b;
    uint64_t n;
};

static int live;

static inline uint32_t rotl(uint32_t x, int k)
{
    return (x << k) | (x >> (32 - k));
}

API struct ctx *sl_new(unsigned seed)
{
    struct ctx *c = calloc(1, sizeof *c);
    if (!c)
        return NULL;
    c->a = seed ^ 0x811c9dc5u;
    live++;
    return c;
}

API void sl_feed(struct ctx *c, const unsigned char *p, size_t n)
{
    for (size_t i = 0; i < n; i++) {
        c->a = (c->a ^ p[i]) * 16777619u;
        c->b = rotl(c->b, 5) + c->a;
    }
    c->n += n;
}

API unsigned sl_final(const struct ctx *c)
{
    return c->a ^ rotl(c->b, 13) ^ (unsigned)c->n;
}

API void sl_free(struct ctx *c)
{
    if (c) {
        free(c);
        live--;
    }
}

API int sl_live(void)
{
    return live;
}

API int sl_pack(const int *vals, size_t n, unsigned char *out, size_t cap)
{
    size_t w = 0;
    for (size_t i = 0; i < n; i++) {
        uint32_t z = ((uint32_t)vals[i] << 1) ^ (uint32_t)(vals[i] >> 31);
        size_t len = 1;
        for (uint32_t t = z; t >= 0x80; t >>= 7)
            len++;
        if (w + len > cap)
            return -1;
        while (z >= 0x80) {
            out[w++] = (unsigned char)(z | 0x80);
            z >>= 7;
        }
        out[w++] = (unsigned char)z;
    }
    return (int)w;
}
