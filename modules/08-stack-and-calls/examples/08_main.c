/* Purpose : call the assembly functions in 08_frames.asm and print the results.
 * Target  : Linux x86-64, C11
 * Build   : see the Makefile
 */
#include <stdio.h>

long sum3(long a, long b, long c);
long sum_locals(long x);
long factorial(long n);
long weighted8(long a, long b, long c, long d, long e, long f, long g, long h);
long apply_twice(long (*f)(long), long x);
unsigned long length_of(const char *s);
long macro_demo(long x);

static long add_ten(long x) { return x + 10; }

int main(void)
{
    printf("sum3(1, 2, 3)                = %ld\n", sum3(1, 2, 3));
    printf("sum_locals(5)                = %ld\n", sum_locals(5));
    printf("factorial(10)                = %ld\n", factorial(10));
    printf("weighted8(1,1,1,1,1,1,1,1)   = %ld\n", weighted8(1, 1, 1, 1, 1, 1, 1, 1));
    printf("weighted8(1,2,3,4,5,6,7,8)   = %ld\n", weighted8(1, 2, 3, 4, 5, 6, 7, 8));
    printf("apply_twice(add_ten, 1)      = %ld\n", apply_twice(add_ten, 1));
    printf("length_of(\"stack frame\")     = %lu\n", length_of("stack frame"));
    printf("macro_demo(14)               = %ld\n", macro_demo(14));
    return 0;
}
