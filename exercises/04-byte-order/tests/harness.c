/* Test harness for exercise 04. Reads one case per line from standard input.
 *
 *   be32 b0 b1 b2 b3             -> load_be32 result in decimal
 *   le16 b0 b1                   -> load_le16 result in decimal
 *   rec  b0 ... b11              -> decode_record: "version port flags timeout_ms"
 *
 * Bytes are hexadecimal. Each input is copied to the end of a mapped page that is
 * followed by an inaccessible page, so reading past the last byte crashes the test.
 */
#define _GNU_SOURCE
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

struct config { uint32_t version; uint16_t port; uint16_t flags; uint32_t timeout_ms; };

uint32_t load_be32(const uint8_t *p);
uint16_t load_le16(const uint8_t *p);
void decode_record(const uint8_t *rec, struct config *out);

static const uint8_t *guarded(const uint8_t *src, size_t n)
{
    long page = sysconf(_SC_PAGESIZE);
    uint8_t *base = mmap(NULL, 2 * page, PROT_READ | PROT_WRITE,
                         MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (base == MAP_FAILED || mprotect(base + page, page, PROT_NONE) != 0)
        return NULL;
    uint8_t *p = base + page - n;
    memcpy(p, src, n);
    return p;
}

static int read_bytes(uint8_t *buf, int n)
{
    for (int i = 0; i < n; i++) {
        unsigned v;
        if (scanf("%x", &v) != 1)
            return 0;
        buf[i] = (uint8_t)v;
    }
    return 1;
}

int main(void)
{
    char cmd[16];
    uint8_t raw[12];
    setvbuf(stdout, NULL, _IONBF, 0);
    while (scanf("%15s", cmd) == 1) {
        if (strcmp(cmd, "be32") == 0 && read_bytes(raw, 4)) {
            printf("%u\n", (unsigned)load_be32(guarded(raw, 4)));
        } else if (strcmp(cmd, "le16") == 0 && read_bytes(raw, 2)) {
            printf("%u\n", (unsigned)(uint16_t)load_le16(guarded(raw, 2)));
        } else if (strcmp(cmd, "rec") == 0 && read_bytes(raw, 12)) {
            struct config out;
            memset(&out, 0xAA, sizeof out);
            decode_record(guarded(raw, 12), &out);
            printf("%u %u %u %u\n", (unsigned)out.version, (unsigned)out.port,
                   (unsigned)out.flags, (unsigned)out.timeout_ms);
        } else {
            return 2;
        }
    }
    return 0;
}
