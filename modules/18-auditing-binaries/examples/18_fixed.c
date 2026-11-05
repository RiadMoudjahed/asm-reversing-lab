/* Module 18 example: the functions of 18_bugs.c with the bugs removed. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "18_api.h"

#define MAX_TABLE (1u << 20)

int set_name(char *dst, size_t cap, const unsigned char *pkt, size_t pkt_len)
{
    if (pkt_len < 1)
        return -1;
    size_t len = pkt[0];
    if (len > pkt_len - 1 || len >= cap)
        return -1;
    memcpy(dst, pkt + 1, len);
    dst[len] = '\0';
    return 0;
}

int copy_label(char *dst, size_t cap, const char *src)
{
    size_t n = strlen(src);
    if (n >= cap)
        return -1;
    memcpy(dst, src, n + 1);
    return 0;
}

unsigned char *make_table(uint32_t count, uint32_t elem)
{
    size_t total = (size_t)count * elem;       /* 64 bits: cannot wrap for 32-bit inputs */
    if (total > MAX_TABLE)
        return NULL;
    unsigned char *t = malloc(total ? total : 1);
    if (!t)
        return NULL;
    for (uint32_t i = 0; i < count; i++)
        memset(t + (size_t)i * elem, (int)i, elem);
    return t;
}

int copy_block(char *dst, const char *src, int len)
{
    if (len < 0 || len > 64)
        return -1;
    memcpy(dst, src, (size_t)len);
    return len;
}

int greet(const char *user, char *out, size_t cap)
{
    int n = snprintf(out, cap, "hello, %s", user);
    return n < 0 || (size_t)n >= cap ? -1 : 0;
}

int finish(struct item *it, int ok)
{
    int id = it->id;
    if (!ok)
        free(it);
    return id;
}
