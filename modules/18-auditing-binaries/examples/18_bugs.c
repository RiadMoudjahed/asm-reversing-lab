/* Module 18 example: six functions, each with one memory-safety bug.
 * 18_fixed.c has the same functions with the bugs removed.
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "18_api.h"

/* Bug 1: the length comes from the packet and is never compared with the size of dst. */
void set_name(char *dst, const unsigned char *pkt)
{
    unsigned len = pkt[0];
    memcpy(dst, pkt + 1, len);
    dst[len] = '\0';
}

/* Bug 2: off by one. A string of exactly cap characters passes the test and its
 * terminating zero byte lands one past the end. */
int copy_label(char *dst, size_t cap, const char *src)
{
    size_t n = strlen(src);
    if (n > cap)
        return -1;
    memcpy(dst, src, n + 1);
    return 0;
}

/* Bug 3: the product is computed in 32 bits and can wrap around before malloc. */
unsigned char *make_table(uint32_t count, uint32_t elem)
{
    uint32_t total = count * elem;
    unsigned char *t = malloc(total);
    if (!t)
        return NULL;
    for (uint32_t i = 0; i < count; i++)
        memset(t + (size_t)i * elem, (int)i, elem);
    return t;
}

/* Bug 4: len is signed. A negative value passes the upper-bound test and becomes a huge
 * size_t in memcpy. */
int copy_block(char *dst, const char *src, int len)
{
    if (len > 64)
        return -1;
    memcpy(dst, src, len);
    return len;
}

/* Bug 5: sprintf with an unbounded %s into a fixed buffer. */
void greet(const char *user, char *out)
{
    char buf[24];
    sprintf(buf, "hello, %s", user);
    strcpy(out, buf);
}

/* Bug 6: the item is freed on one path and used afterwards on every path. */
int finish(struct item *it, int ok)
{
    if (!ok)
        free(it);
    return it->id;
}
