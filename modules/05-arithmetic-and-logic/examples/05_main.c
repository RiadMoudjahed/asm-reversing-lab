/* Purpose : call the assembly functions in 05_arith.asm and print the results.
 * Target  : Linux x86-64, C11
 * Build   : see the Makefile
 */
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

long add_overflows(long a, long b, long *sum);
unsigned long add_carries(unsigned long a, unsigned long b);
unsigned long mul_wide(unsigned long a, unsigned long b, unsigned long *hi);
long divide(long a, long b, long *rem);
long sar_by(long x, int n);
unsigned long shr_by(unsigned long x, int n);
void add128(uint64_t a[2], const uint64_t b[2]);
long negate(long x);
unsigned combine_bits(unsigned x);

int main(void)
{
    long sum, rem;
    unsigned long hi;

    long o1 = add_overflows(INT64_MAX, 1, &sum);
    printf("add_overflows(INT64_MAX, 1)      = %ld  (sum = %ld)\n", o1, sum);
    printf("add_overflows(5, 7)              = %ld\n", add_overflows(5, 7, &sum));
    printf("add_carries(UINT64_MAX, 1)       = %lu\n", add_carries(UINT64_MAX, 1));
    printf("add_carries(INT64_MAX, 1)        = %lu\n", add_carries(INT64_MAX, 1));

    unsigned long lo = mul_wide(0x100000000UL, 0x300000000UL, &hi);
    printf("mul_wide(2^32, 3*2^32)           = hi %lu, lo %lu\n", hi, lo);

    long q = divide(-17, 5, &rem);
    printf("divide(-17, 5)                   = quotient %ld, remainder %ld\n", q, rem);

    printf("sar_by(-16, 2)                   = %ld\n", sar_by(-16, 2));
    printf("shr_by(-16, 2)                   = %lu\n", shr_by((unsigned long)-16, 2));

    uint64_t a[2] = { UINT64_MAX, 0 };
    uint64_t b[2] = { 1, 0 };
    add128(a, b);
    printf("add128(2^64 - 1, 1)              = high %" PRIu64 ", low %" PRIu64 "\n", a[1], a[0]);

    printf("negate(10)                       = %ld\n", negate(10));
    printf("combine_bits(0xAB)               = 0x%02X\n", combine_bits(0xAB));
    return 0;
}
