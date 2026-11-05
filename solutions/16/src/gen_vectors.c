/* Prints the expected output of the exercise 16 test vectors, using the library itself.
 * Input lines:  d SEED HEX   and   p CAP N V1 ... VN   (see tests/harness.c). */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void *sl_new(unsigned seed);
void sl_feed(void *c, const unsigned char *p, size_t n);
unsigned sl_final(void *c);
void sl_free(void *c);
int sl_pack(const int *vals, size_t n, unsigned char *out, size_t cap);

int main(void)
{
    char op;
    while (scanf(" %c", &op) == 1) {
        if (op == 'd') {
            unsigned seed;
            char hex[1024];
            unsigned char buf[512];
            size_t n = 0;
            scanf("%u %1023s", &seed, hex);
            if (strcmp(hex, "-"))
                for (size_t i = 0; hex[i]; i += 2) {
                    unsigned v;
                    sscanf(hex + i, "%2x", &v);
                    buf[n++] = (unsigned char)v;
                }
            void *c = sl_new(seed);
            sl_feed(c, buf, n);
            printf("0x%08x ok\n", sl_final(c));
            sl_free(c);
        } else {
            size_t cap, n;
            int vals[64];
            unsigned char out[64];
            scanf("%zu %zu", &cap, &n);
            for (size_t i = 0; i < n; i++)
                scanf("%d", &vals[i]);
            memset(out, 0xCC, sizeof out);
            int r = sl_pack(vals, n, out, cap);
            printf("%d ", r);
            for (size_t i = 0; i < cap + 4; i++)
                printf("%02x", out[i]);
            printf(" ok\n");
        }
    }
    return 0;
}
