/* Module 23: small functions whose compiled form is easy to misread.
 * Build: gcc -std=gnu11 -O2 -fPIC -shared -fcf-protection=none -fno-asynchronous-unwind-tables 23_cases.c -o libcases.so
 */
#include <limits.h>
#include <stddef.h>

/* Division by a constant becomes a multiplication and a shift. */
unsigned div10(unsigned x) { return x / 10; }

/* Both operands are 32 bits wide; the product is 64 bits wide. */
unsigned long widen(unsigned a, unsigned b) { return (unsigned long)a * b; }

/* Signed addition that saturates instead of wrapping. */
int sat_add(int a, int b)
{
    int r;
    if (__builtin_add_overflow(a, b, &r))
        return a < 0 ? INT_MIN : INT_MAX;
    return r;
}

/* True for 1, 2, 4, 8, ... */
int is_pow2(unsigned x) { return x && !(x & (x - 1)); }

/* Sum of 0 .. n-1, computed with a loop in the source. */
unsigned sum_to(unsigned n)
{
    unsigned s = 0;
    for (unsigned i = 0; i < n; i++)
        s += i;
    return s;
}

/* A byte that is read as a signed number. */
int sign_byte(const unsigned char *p) { return (signed char)p[0]; }

/* Count the elements that are smaller than t. */
int count_lt(const int *a, int n, int t)
{
    int c = 0;
    for (int i = 0; i < n; i++)
        if (a[i] < t)
            c++;
    return c;
}
