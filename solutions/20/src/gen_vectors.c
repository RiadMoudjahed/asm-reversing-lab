/* Exercise 20: computes the expected output lines for the Part 2 test vectors.
 * Reads lines "NAME INS STATE" on standard input and prints "STATE ok" after the reference
 * handler has run. Build: gcc -O1 gen_vectors.c -o gen_vectors
 */
#include <stdlib.h>
#include <string.h>

#include "ref.h"

static size_t parse_hex(const char *s, uint8_t *out)
{
    size_t n = 0;
    for (size_t i = 0; s[i] && s[i + 1]; i += 2) {
        unsigned v;
        sscanf(s + i, "%2x", &v);
        out[n++] = (uint8_t)v;
    }
    return n;
}

int main(void)
{
    static char line[2048];
    while (fgets(line, sizeof line, stdin)) {
        char *name = strtok(line, " \n"), *ih = strtok(NULL, " \n"), *sh = strtok(NULL, " \n");
        uint8_t ins[8] = {0}, st[sizeof(struct vm)];
        parse_hex(ih, ins);
        parse_hex(sh, st);
        struct vm *m = (struct vm *)st;
        if (!strcmp(name, "h_mul")) ref_mul(m, ins);
        else if (!strcmp(name, "h_shl")) ref_shl(m, ins);
        else if (!strcmp(name, "h_shr")) ref_shr(m, ins);
        else if (!strcmp(name, "h_cmp")) ref_cmp(m, ins);
        else if (!strcmp(name, "h_call")) ref_call(m, ins);
        else if (!strcmp(name, "h_ret")) ref_ret(m, ins);
        else if (!strcmp(name, "h_ldb")) ref_ldb(m, ins);
        else if (!strcmp(name, "h_stb")) ref_stb(m, ins);
        for (size_t i = 0; i < sizeof st; i++)
            printf("%02x", st[i]);
        printf(" ok\n");
    }
    return 0;
}
