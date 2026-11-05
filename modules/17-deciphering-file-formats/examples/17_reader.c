/* Module 17 example: the reader of a small file format. The lesson treats the compiled program
 * as the thing to study. Usage: 17_reader FILE
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint32_t table[256];

static void make_table(void)
{
    for (uint32_t i = 0; i < 256; i++) {
        uint32_t c = i;
        for (int k = 0; k < 8; k++)
            c = (c & 1) ? 0xEDB88320u ^ (c >> 1) : c >> 1;
        table[i] = c;
    }
}

static uint32_t crc_update(uint32_t crc, const unsigned char *p, size_t n)
{
    for (size_t i = 0; i < n; i++)
        crc = table[(crc ^ p[i]) & 0xff] ^ (crc >> 8);
    return crc;
}

static int fail(const char *msg)
{
    fprintf(stderr, "error: %s\n", msg);
    return 1;
}

int main(int argc, char **argv)
{
    static unsigned char payload[65536];
    unsigned char h[12], ch[8];
    uint32_t crc, samples = 0, notes = 0, meta = 0;
    FILE *f;

    if (argc != 2)
        return fail("usage: reader FILE");
    f = fopen(argv[1], "rb");
    if (!f)
        return fail("cannot open file");
    make_table();
    if (fread(h, 1, 12, f) != 12)
        return fail("truncated header");
    if (memcmp(h, "LGR1", 4) != 0)
        return fail("bad magic");
    if (h[4] != 2)
        return fail("unsupported version");
    unsigned flags = h[5];
    unsigned channels = h[6] | (h[7] << 8);
    uint32_t rate = h[8] | (h[9] << 8) | (h[10] << 16) | ((uint32_t)h[11] << 24);
    crc = crc_update(0xFFFFFFFFu, h, 12);
    for (;;) {
        if (fread(ch, 1, 8, f) != 8)
            return fail("truncated chunk header");
        uint32_t len = ch[4] | (ch[5] << 8) | (ch[6] << 16) | ((uint32_t)ch[7] << 24);
        if (memcmp(ch, "END ", 4) == 0) {
            unsigned char s[4];
            if (len != 4 || fread(s, 1, 4, f) != 4)
                return fail("bad end chunk");
            uint32_t want = s[0] | (s[1] << 8) | (s[2] << 16) | ((uint32_t)s[3] << 24);
            if (~crc != want)
                return fail("checksum mismatch");
            break;
        }
        if (len > sizeof payload)
            return fail("chunk too large");
        if (fread(payload, 1, len, f) != len)
            return fail("truncated chunk");
        crc = crc_update(crc, ch, 8);
        crc = crc_update(crc, payload, len);
        if (memcmp(ch, "DATA", 4) == 0)
            samples += len / 2;
        else if (memcmp(ch, "META", 4) == 0)
            meta++;
        else if (memcmp(ch, "NOTE", 4) == 0)
            notes++;
    }
    printf("channels=%u rate=%u flags=%u samples=%u meta=%u notes=%u\n",
           channels, rate, flags, samples, meta, notes);
    return 0;
}
