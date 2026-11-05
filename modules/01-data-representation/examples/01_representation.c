/* Purpose : show how the same bits mean different things, and how a value is
 *           laid out in memory byte by byte.
 * Target  : Linux x86-64, C11
 * Build   : gcc -std=c11 -Wall -o 01_representation 01_representation.c
 * Run     : ./01_representation
 */
#include <stdint.h>
#include <stdio.h>

static void dump_bytes(const char *label, const void *p, size_t n)
{
    const unsigned char *b = p;
    printf("%-14s", label);
    for (size_t i = 0; i < n; i++)
        printf(" %02x", b[i]);
    printf("\n");
}

int main(void)
{
    /* One bit pattern, three readings. */
    uint8_t  raw = 0xF6;
    printf("0xF6 as unsigned: %u\n", (unsigned)raw);
    printf("0xF6 as signed:   %d\n", (int)(int8_t)raw);
    printf("0xF6 as binary:   11110110\n");

    /* Two's complement: negate by inverting the bits and adding one. */
    int8_t ten = 10;
    int8_t neg = (int8_t)(~ten + 1);
    printf("-10 by hand:      %d (0x%02X)\n", neg, (unsigned)(uint8_t)neg);

    /* Sign extension versus zero extension of the same byte. */
    int32_t  sx = (int8_t)raw;
    uint32_t zx = raw;
    printf("sign-extended:    0x%08X\n", (unsigned)sx);
    printf("zero-extended:    0x%08X\n", (unsigned)zx);

    /* Byte order: x86 stores the least significant byte first. */
    uint32_t word = 0x11223344;
    dump_bytes("0x11223344 in memory:", &word, sizeof word);

    /* Text is just bytes. */
    dump_bytes("\"Hi!\" bytes:", "Hi!", 3);
    return 0;
}
