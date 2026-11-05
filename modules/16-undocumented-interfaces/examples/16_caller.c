/* Module 16 example: a caller of the black-box library. Its compiled form shows how the
 * results of the library calls are used.
 */
#include <stddef.h>

long bb_split(const char *s, char sep, char *out, size_t cap);

int copy_key(const char *line, char *key)
{
    long n = bb_split(line, '=', key, 32);
    if (n < 0)
        return -1;
    return (int)n;
}
