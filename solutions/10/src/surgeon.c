/* Exercise 10 challenge program: a small dynamically linked program with an unusual image
 * base, an extra library and a custom section. Build with the Makefile in this directory.
 */
#include <math.h>
#include <stdio.h>
#include <string.h>

__attribute__((section(".vault"), used)) static const char vault_note[] = "surgeon-table-v3";

static volatile double seed = 1521.0;
static int checksum = 4242;
static char scratch[96];

static int mix(const char *s)
{
    int h = checksum;
    for (; *s; s++)
        h = h * 33 + *s;
    return h;
}

int main(void)
{
    strcpy(scratch, vault_note);
    printf("surgeon ready: %d %.0f\n", mix(scratch), sqrt(seed));
    return 0;
}
