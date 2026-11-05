/* Module 18 example: the same six situations against 18_fixed.c. Usage: demo_fix */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "18_api.h"

int set_name(char *dst, size_t cap, const unsigned char *pkt, size_t pkt_len);
int greet(const char *user, char *out, size_t cap);

int main(void)
{
    char dst[16], out[64], src[64] = {0};
    unsigned char pkt[64];
    pkt[0] = 40;
    memset(pkt + 1, 'A', 40);
    printf("name:   %d\n", set_name(dst, sizeof dst, pkt, 41));
    pkt[0] = 5;
    memcpy(pkt + 1, "alice", 5);
    printf("name:   %d (%s)\n", set_name(dst, sizeof dst, pkt, 6), dst);
    printf("label:  %d\n", copy_label(dst, 8, "ABCDEFGH"));
    printf("label:  %d\n", copy_label(dst, 8, "ABCDEFG"));
    unsigned char *t = make_table(0x10000001u, 16);
    printf("table:  %s\n", t ? "allocated" : "refused");
    free(t);
    printf("block:  %d\n", copy_block(dst, src, -1));
    printf("greet:  %d\n", greet("0123456789012345678901234567", out, 24));
    printf("greet:  %d (%s)\n", greet("bob", out, 24), out);
    struct item *it = malloc(sizeof *it);
    it->id = 7;
    printf("finish: %d\n", finish(it, 0));
    return 0;
}
