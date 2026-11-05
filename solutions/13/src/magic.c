/* Exercise 13 challenge program: reads integers from standard input and prints mix(x) for
 * each. Build with the Makefile in this directory.
 */
#include <stdint.h>
#include <stdio.h>

__attribute__((noinline)) int32_t mix(int32_t x)
{
    uint32_t u = (uint32_t)x;
    uint32_t a = u / 12;
    uint32_t b = u % 9;
    int32_t c = x / 60;
    int32_t d = x % 11;
    return (int32_t)(a * 5 + b * 3) - c + d * 4;
}

int main(void)
{
    long x;
    while (scanf("%ld", &x) == 1)
        printf("%d\n", mix((int32_t)x));
    return 0;
}
