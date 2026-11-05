/* Prints the expected output of the exercise 17 test vectors with the C reference.
 * Input lines are described in tests/harness.c. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ark_ref.h"

static void hex(const unsigned char *p, size_t n)
{
    for (size_t i = 0; i < n; i++)
        printf("%02x", p[i]);
}

int main(void)
{
    static char line[1 << 17];
    while (fgets(line, sizeof line, stdin)) {
        static unsigned char arc[1 << 16];
        size_t n = 0;
        int first = 1;
        for (char *cmd = strtok(line, ";\n"); cmd; cmd = strtok(NULL, ";\n")) {
            char op = 0;
            long idx = 0;
            unsigned long cap = 0;
            char hexs[1 << 16];
            if (!first)
                printf(" ; ");
            first = 0;
            sscanf(cmd, " %c", &op);
            if (op == 'a') {
                n = 0;
                if (sscanf(cmd, " a %65535s", hexs) == 1 && strcmp(hexs, "-"))
                    for (size_t i = 0; hexs[i]; i += 2) {
                        unsigned v;
                        sscanf(hexs + i, "%2x", &v);
                        arc[n++] = (unsigned char)v;
                    }
                printf("set %zu ok", n);
            } else if (op == 'c') {
                printf("%ld ok", ref_count(arc, n));
            } else {
                sscanf(cmd, " %c %ld %lu", &op, &idx, &cap);
                unsigned char buf[70000];
                long r;
                if (op == 'n')
                    r = ref_name(arc, n, idx, (char *)buf, cap);
                else
                    r = ref_extract(arc, n, idx, buf, cap);
                printf("%ld ", r);
                if (r >= 0)
                    hex(buf, op == 'n' ? (size_t)r + 1 : (size_t)r);
                else
                    printf("-");
                printf(" ok");
            }
        }
        printf("\n");
    }
    return 0;
}
