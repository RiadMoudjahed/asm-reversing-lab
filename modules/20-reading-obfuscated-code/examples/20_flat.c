/* Module 20 example: one function written in a plain way and in a flattened way.
 * Both compute the same result. The flattened version keeps its state in a global variable so
 * that the lesson can watch it; real obfuscators use a local variable or a register.
 */
#include <stdio.h>

unsigned score_plain(unsigned n)
{
    if (n % 3 == 0)
        return n * 2;
    if (n % 5 == 0)
        return n + 7;
    return n ^ 0x55;
}

volatile int g_state;
volatile unsigned g_junk;

unsigned score_flat(unsigned n)
{
    unsigned r = 0;
    g_state = 0;
    for (;;) {
        switch (g_state) {
        case 0:
            g_junk = g_junk * 31 + n;                 /* junk: the value is never used */
            g_state = (n % 3 == 0) ? 1 : 2;
            break;
        case 1:
            r = n * 2;
            g_state = 5;
            break;
        case 2:
            if ((n * (n + 1)) % 2 == 1)               /* opaque predicate: never true */
                r = g_junk;
            g_state = (n % 5 == 0) ? 3 : 4;
            break;
        case 3:
            r = n + 7;
            g_state = 5;
            break;
        case 4:
            r = n ^ 0x55;
            g_state = 5;
            break;
        default:
            return r;
        }
    }
}

int main(void)
{
    for (unsigned n = 0; n < 12; n++)
        printf("%2u: %3u %3u\n", n, score_plain(n), score_flat(n));
    return 0;
}
