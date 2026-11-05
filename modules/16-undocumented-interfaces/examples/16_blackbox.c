/* Module 16 example: a small library used as a black box.
 * Build: see the Makefile. The lesson works from the compiled library, not from this file.
 */
#include <stdlib.h>

#define API __attribute__((visibility("default")))

API int bb_clamp(int v, int lo, int hi)
{
    return v < lo ? lo : v > hi ? hi : v;
}

API unsigned bb_checksum(const unsigned char *p, size_t n, unsigned seed)
{
    unsigned h = seed;
    for (size_t i = 0; i < n; i++)
        h = ((h << 5) | (h >> 27)) ^ p[i];
    return h;
}

API long bb_split(const char *s, char sep, char *out, size_t cap)
{
    if (cap == 0)
        return -1;
    size_t i = 0;
    while (s[i] && s[i] != sep) {
        if (i + 1 >= cap)
            return -1;
        out[i] = s[i];
        i++;
    }
    out[i] = 0;
    return (long)i;
}

API void bb_minmax(const int *a, size_t n, int *lo, int *hi)
{
    int l = a[0], h = a[0];
    for (size_t i = 1; i < n; i++) {
        if (a[i] < l)
            l = a[i];
        if (a[i] > h)
            h = a[i];
    }
    *lo = l;
    *hi = h;
}

API double bb_lerp(double a, double b, double t)
{
    return a + (b - a) * t;
}

struct counter {
    int value;
    int step;
    unsigned calls;
};

API struct counter *bb_open(int start, int step)
{
    struct counter *c = calloc(1, sizeof *c);
    if (!c)
        return NULL;
    c->value = start;
    c->step = step;
    return c;
}

API int bb_next(struct counter *c)
{
    c->calls++;
    c->value += c->step;
    return c->value;
}

API void bb_close(struct counter *c)
{
    free(c);
}
