/* Module 18 example: shared declarations. */
#ifndef API_H
#define API_H
#include <stddef.h>
#include <stdint.h>

struct item {
    int id;
    char tag[12];
};

unsigned char *make_table(uint32_t count, uint32_t elem);
int copy_block(char *dst, const char *src, int len);
int copy_label(char *dst, size_t cap, const char *src);
int finish(struct item *it, int ok);
#endif
