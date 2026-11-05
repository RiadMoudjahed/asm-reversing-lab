/* Module 20 example: three ways of hiding a string. The program is harmless: it decodes
 * three strings and prints them.
 */
#include <stdio.h>
#include <stddef.h>
#include "20_blobs.h"

/* Scheme 1: every byte is xor-ed with one fixed key. */
__attribute__((noinline)) void decode_fixed(char *out, const unsigned char *in, size_t n,
                                            unsigned char key)
{
    for (size_t i = 0; i < n; i++)
        out[i] = (char)(in[i] ^ key);
    out[n] = '\0';
}

/* Scheme 2: a rolling key. Each byte uses the next value of k = k * 5 + 3. */
__attribute__((noinline)) void decode_roll(char *out, const unsigned char *in, size_t n,
                                           unsigned char seed)
{
    unsigned char k = seed;
    for (size_t i = 0; i < n; i++) {
        k = (unsigned char)(k * 5 + 3);
        out[i] = (char)(in[i] ^ k);
    }
    out[n] = '\0';
}

/* Scheme 3: a stack string. The text never exists as one piece in the file. */
__attribute__((noinline)) void build_stack(char *out)
{
    out[0] = 's'; out[1] = 't'; out[2] = 'a'; out[3] = 't'; out[4] = 'u'; out[5] = 's';
    out[6] = '='; out[7] = 'o'; out[8] = 'k'; out[9] = '\0';
}

int main(void)
{
    char a[64], b[64], c[16];
    decode_fixed(a, BLOB_A, sizeof BLOB_A, KEY_A);
    decode_roll(b, BLOB_B, sizeof BLOB_B, SEED_B);
    build_stack(c);
    printf("%s\n%s\n%s\n", a, b, c);
    return 0;
}
