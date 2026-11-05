/* Module 23: two readings of each function in 23_cases.c, as they might be written down from a
 * decompiler listing. The name ends in _ok when the reading matches the binary and in _bad
 * when it contains a typical mistake. 23_verify.py runs them against the real functions.
 * Build: gcc -std=gnu11 -O1 -fPIC -shared -fwrapv -Wno-parentheses 23_candidates.c -o libcand.so
 */
#include <limits.h>

/* The constant and the shift of the binary, written with a 64-bit product. */
unsigned div10_ok(unsigned x) { return (unsigned)(((unsigned long)x * 0xcccccccdUL) >> 35); }
/* The value read as a signed number. */
unsigned div10_bad(unsigned x) { return (unsigned)((int)x / 10); }

unsigned long widen_ok(unsigned a, unsigned b) { return (unsigned long)a * b; }
/* The multiplication is done in 32 bits and widened afterwards. */
unsigned long widen_bad(unsigned a, unsigned b) { return (unsigned long)(a * b); }

int sat_add_ok(int a, int b)
{
    int r = (int)((unsigned)a + (unsigned)b);
    if (((a ^ r) & (b ^ r)) < 0)
        return a < 0 ? INT_MIN : INT_MAX;
    return r;
}
/* Only the overflow of two positive numbers is handled. */
int sat_add_bad(int a, int b)
{
    int r = (int)((unsigned)a + (unsigned)b);
    if (a > 0 && b > 0 && r < 0)
        return INT_MAX;
    return r;
}

int is_pow2_ok(unsigned x) { return x != 0 && (x & (x - 1)) == 0; }
/* The comparison binds tighter than the bitwise and. */
int is_pow2_bad(unsigned x) { return x != 0 && x & (x - 1) == 0; }

unsigned sum_to_ok(unsigned n) { return (unsigned)(((unsigned long)n * (n - 1)) / 2); }
/* The product is computed in 32 bits and wraps before the division. */
unsigned sum_to_bad(unsigned n) { return n * (n - 1) / 2; }

int sign_byte_ok(const unsigned char *p) { return (signed char)p[0]; }
/* The byte is zero-extended instead of sign-extended. */
int sign_byte_bad(const unsigned char *p) { return p[0]; }

int count_lt_ok(const int *a, int n, int t)
{
    int c = 0;
    for (int i = 0; i < n; i++)
        c += t > a[i];
    return c;
}
/* The comparison is unsigned. */
int count_lt_bad(const int *a, int n, int t)
{
    int c = 0;
    for (int i = 0; i < n; i++)
        c += (unsigned)a[i] < (unsigned)t;
    return c;
}
