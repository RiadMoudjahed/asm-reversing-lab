/* Purpose : call the assembly functions in 06_control.asm and print the results.
 * Target  : Linux x86-64, C11
 * Build   : see the Makefile
 */
#include <stdint.h>
#include <stdio.h>

long max_signed(long a, long b);
unsigned long max_unsigned(unsigned long a, unsigned long b);
long sign_of(long x);
long sign_branchless(long x);
long sum_to(long n);
long count_digits(unsigned long x);
long sum_array(const int32_t *a, long n);
long is_power_of_two(unsigned long x);

int main(void)
{
    int32_t numbers[] = { 4, -9, 20, 1 };

    printf("max_signed(-1, 1)        = %ld\n", max_signed(-1, 1));
    printf("max_unsigned(-1, 1)      = %lu\n", max_unsigned((unsigned long)-1, 1));
    printf("sign_of(-7), (0), (12)   = %ld %ld %ld\n", sign_of(-7), sign_of(0), sign_of(12));
    printf("sign_branchless same     = %ld %ld %ld\n",
           sign_branchless(-7), sign_branchless(0), sign_branchless(12));
    printf("sum_to(10), sum_to(0)    = %ld %ld\n", sum_to(10), sum_to(0));
    printf("count_digits(0), (9), (12345) = %ld %ld %ld\n",
           count_digits(0), count_digits(9), count_digits(12345));
    printf("sum_array({4,-9,20,1}, 4) = %ld\n", sum_array(numbers, 4));
    printf("is_power_of_two(64), (96), (0) = %ld %ld %ld\n",
           is_power_of_two(64), is_power_of_two(96), is_power_of_two(0));
    return 0;
}
