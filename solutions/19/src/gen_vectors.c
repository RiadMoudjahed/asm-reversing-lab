/* Prints the expected output of the exercise 19 test vectors with the C reference.
 * Input lines: CAP HEX (see tests/harness.c). */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ref.h"

int main(void)
{
    static char line[1 << 12];
    while (fgets(line, sizeof line, stdin)) {
        unsigned char rec[600];
        size_t n = 0;
        char *s = strtok(line, " \n");
        unsigned long cap = strtoul(s, NULL, 10);
        s = strtok(NULL, " \n");
        if (s && strcmp(s, "-"))
            for (size_t i = 0; s[i]; i += 2) {
                unsigned v;
                sscanf(s + i, "%2x", &v);
                rec[n++] = (unsigned char)v;
            }
        char *out = calloc(cap + 16, 1);
        long r = ref_decode(rec, n, out, cap);
        printf("%ld ", r);
        if (r >= 0)
            for (long i = 0; i <= r && (unsigned long)i < cap; i++)
                printf("%02x", (unsigned char)out[i]);
        else
            printf("-");
        printf(" ok\n");
        free(out);
    }
    return 0;
}
