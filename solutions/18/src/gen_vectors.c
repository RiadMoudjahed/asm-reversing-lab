/* Prints the expected output of the exercise 18 test vectors with the C reference.
 * Input: one line per test, the bytes of the buffer in hexadecimal ("-" for none). */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ref.h"

int main(void)
{
    static char line[1 << 14];
    while (fgets(line, sizeof line, stdin)) {
        unsigned char buf[4096];
        size_t n = 0;
        char *s = strtok(line, " \n");
        if (s && strcmp(s, "-"))
            for (size_t i = 0; s[i]; i += 2) {
                unsigned v;
                sscanf(s + i, "%2x", &v);
                buf[n++] = (unsigned char)v;
            }
        struct out o;
        memset(&o, 0, sizeof o);
        long r = ref_parse(buf, n, &o);
        printf("%ld", r);
        if (r >= 0) {
            printf(" name=");
            for (int i = 0; i < 16; i++)
                printf("%02x", (unsigned char)o.name[i]);
            printf(" sum=%lld tags=", (long long)o.sum);
            for (int i = 0; i < 8; i++)
                printf("%s%u", i ? "," : "", o.tags[i]);
            printf(" blob=%u", o.blob_sum);
        }
        printf(" ok\n");
    }
    return 0;
}
