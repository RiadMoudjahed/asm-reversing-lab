/* Module 18 example: triggers each bug of 18_bugs.c once. Usage: demo_bug CASE
 * Built with AddressSanitizer, which stops the program at the first bad access and reports it.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "18_api.h"

void set_name(char *dst, const unsigned char *pkt);
void greet(const char *user, char *out);

int main(int argc, char **argv)
{
    const char *c = argc > 1 ? argv[1] : "";
    if (!strcmp(c, "name")) {
        char dst[16];
        unsigned char *pkt = malloc(64);
        pkt[0] = 40;
        memset(pkt + 1, 'A', 40);
        set_name(dst, pkt);
        free(pkt);
    } else if (!strcmp(c, "label")) {
        char dst[8];
        printf("copy_label returned %d\n", copy_label(dst, sizeof dst, "ABCDEFGH"));
    } else if (!strcmp(c, "table")) {
        unsigned char *t = make_table(0x10000001u, 16);
        free(t);
    } else if (!strcmp(c, "block")) {
        char dst[64], src[64] = {0};
        copy_block(dst, src, -1);
    } else if (!strcmp(c, "greet")) {
        char out[64];
        greet("0123456789012345678901234567", out);
    } else if (!strcmp(c, "finish")) {
        struct item *it = malloc(sizeof *it);
        it->id = 7;
        printf("finish returned %d\n", finish(it, 0));
    } else {
        fprintf(stderr, "usage: demo_bug name|label|table|block|greet|finish\n");
        return 2;
    }
    return 0;
}
