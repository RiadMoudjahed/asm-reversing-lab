/* Exercise 18: a record parser with memory-safety bugs. Usage: recparse FILE
 * It is a teaching target: run it only on files you made yourself.
 * Build with the Makefile in this directory.
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct out {
    char name[16];
    int64_t sum;
    uint32_t tags[8];
    uint32_t blob_sum;
    uint32_t pad;
};

static long parse(const unsigned char *buf, size_t n, struct out *o)
{
    const unsigned char *p = buf;
    unsigned char *blob = NULL;
    size_t blob_len = 0;
    uint32_t hist[8];
    long records = 0;

    (void)n;
    memset(hist, 0, sizeof hist);
    for (;;) {
        unsigned type = p[0], len = p[1];
        const unsigned char *pl = p + 2;
        p = pl + len;
        if (type == 0)
            break;
        switch (type) {
        case 1: {
            char name[16];
            memset(name, 0, sizeof name);
            if (len > 16)
                return -2;
            memcpy(name, pl, len);
            name[len] = 0;
            memcpy(o->name, name, sizeof name);
            break;
        }
        case 2: {
            int vals[8];
            signed char l8 = (signed char)len;
            if (l8 > 32)
                return -3;
            memcpy(vals, pl, l8);
            for (int i = 0; i < l8 / 4; i++)
                o->sum += vals[i];
            break;
        }
        case 3:
            for (unsigned i = 0; i < len; i++) {
                if (pl[i] > 8)
                    return -4;
                hist[pl[i]]++;
            }
            break;
        case 4: {
            unsigned count = pl[0];
            uint8_t size = (uint8_t)(count * 4);
            free(blob);
            blob = malloc(size);
            memcpy(blob, pl + 1, count * 4);
            blob_len = count * 4;
            break;
        }
        case 5:
            free(blob);
            break;
        default:
            return -6;
        }
        records++;
    }
    for (int i = 0; i < 8; i++)
        o->tags[i] = hist[i];
    for (size_t i = 0; i < blob_len; i++)
        o->blob_sum += blob[i];
    free(blob);
    return records;
}

int main(int argc, char **argv)
{
    static unsigned char buf[4096];
    struct out o;
    if (argc != 2) {
        fprintf(stderr, "usage: recparse FILE\n");
        return 2;
    }
    FILE *f = fopen(argv[1], "rb");
    if (!f) {
        fprintf(stderr, "cannot open file\n");
        return 2;
    }
    size_t n = fread(buf, 1, sizeof buf, f);
    fclose(f);
    memset(&o, 0, sizeof o);
    long r = parse(buf, n, &o);
    if (r < 0) {
        printf("rejected %ld\n", r);
        return 1;
    }
    printf("records=%ld name=%s sum=%lld tags=%u,%u,%u,%u,%u,%u,%u,%u blob=%u\n", r, o.name,
           (long long)o.sum, o.tags[0], o.tags[1], o.tags[2], o.tags[3], o.tags[4], o.tags[5],
           o.tags[6], o.tags[7], o.blob_sum);
    return 0;
}
