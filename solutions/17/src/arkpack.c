/* Exercise 17: the program that creates the archives. Usage: arkpack OUTPUT FILE...
 * Build with the Makefile in this directory.
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_FILES 48
#define MAX_NAME 31
#define MAX_SIZE 65536

static void put16(unsigned char *p, unsigned v) { p[0] = v; p[1] = v >> 8; }
static void put32(unsigned char *p, uint32_t v) { for (int i = 0; i < 4; i++) p[i] = v >> (8 * i); }

static uint32_t rotl(uint32_t x, int k) { return (x << k) | (x >> (32 - k)); }

static uint32_t checksum(const unsigned char *p, size_t n)
{
    uint32_t c = 0x13579bdfu;
    for (size_t i = 0; i < n; i++)
        c = rotl(c, 7) + p[i];
    return c ^ (uint32_t)n;
}

static int die(const char *msg)
{
    fprintf(stderr, "arkpack: %s\n", msg);
    return 1;
}

int main(int argc, char **argv)
{
    static unsigned char out[1 << 22], data[MAX_SIZE + 1];
    int count = argc - 2;
    if (count < 1)
        return die("usage: arkpack OUTPUT FILE...");
    if (count > MAX_FILES)
        return die("too many files");
    unsigned key = (unsigned)(count * 37 + 11) & 0xff;
    size_t pos = 16;
    uint32_t data_off[MAX_FILES], size[MAX_FILES], check[MAX_FILES];
    for (int i = 0; i < count; i++) {
        FILE *f = fopen(argv[i + 2], "rb");
        if (!f)
            return die("cannot open input file");
        size_t n = fread(data, 1, sizeof data, f);
        fclose(f);
        if (n > MAX_SIZE)
            return die("file too large");
        pos = (pos + 3) & ~(size_t)3;
        data_off[i] = (uint32_t)pos;
        size[i] = (uint32_t)n;
        check[i] = checksum(data, n);
        for (size_t k = 0; k < n; k++)
            out[pos + k] = data[k] ^ (unsigned char)(key + k);
        pos += n;
    }
    size_t table_off = (pos + 3) & ~(size_t)3;
    size_t names_off = table_off + 16 * (size_t)count;
    size_t np = 0;
    for (int i = 0; i < count; i++) {
        const char *name = strrchr(argv[i + 2], '/');
        name = name ? name + 1 : argv[i + 2];
        size_t len = strlen(name);
        if (len > MAX_NAME)
            return die("name too long");
        unsigned char *e = out + table_off + 16 * (size_t)i;
        put32(e, (uint32_t)np);
        put32(e + 4, data_off[i]);
        put32(e + 8, size[i]);
        put32(e + 12, check[i]);
        memcpy(out + names_off + np, name, len + 1);
        np += len + 1;
    }
    memcpy(out, "ARK2", 4);
    put16(out + 4, (unsigned)count);
    out[6] = (unsigned char)key;
    out[7] = 0;
    put32(out + 8, (uint32_t)table_off);
    put32(out + 12, (uint32_t)names_off);
    FILE *f = fopen(argv[1], "wb");
    if (!f)
        return die("cannot create output file");
    fwrite(out, 1, names_off + np, f);
    fclose(f);
    return 0;
}
