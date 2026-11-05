/* Reference implementation of the Part 2 routine in C. Used to generate the test vectors. */
#include <stddef.h>
#include <stdint.h>
#include <string.h>

struct out {
    char name[16];
    int64_t sum;
    uint32_t tags[8];
    uint32_t blob_sum;
    uint32_t pad;
};

static inline long ref_parse(const unsigned char *buf, size_t n, struct out *o)
{
    size_t pos = 0;
    long records = 0;
    uint32_t blob_sum = 0;
    for (;;) {
        if (n - pos < 2)
            return -1;
        unsigned type = buf[pos], len = buf[pos + 1];
        pos += 2;
        if (n - pos < len)
            return -1;
        const unsigned char *pl = buf + pos;
        pos += len;
        if (type == 0)
            break;
        switch (type) {
        case 1:
            if (len > 15)
                return -2;
            memset(o->name, 0, sizeof o->name);
            memcpy(o->name, pl, len);
            break;
        case 2:
            if (len % 4 != 0 || len > 32)
                return -3;
            for (unsigned i = 0; i < len; i += 4)
                o->sum += (int32_t)(pl[i] | pl[i + 1] << 8 | pl[i + 2] << 16 | (uint32_t)pl[i + 3] << 24);
            break;
        case 3:
            for (unsigned i = 0; i < len; i++) {
                if (pl[i] > 7)
                    return -4;
                o->tags[pl[i]]++;
            }
            break;
        case 4: {
            if (len < 1 || len != 1 + 4u * pl[0])
                return -5;
            blob_sum = 0;
            for (unsigned i = 1; i < len; i++)
                blob_sum += pl[i];
            break;
        }
        case 5:
            if (len != 0)
                return -5;
            blob_sum = 0;
            break;
        default:
            return -6;
        }
        records++;
    }
    o->blob_sum = blob_sum;
    return records;
}
