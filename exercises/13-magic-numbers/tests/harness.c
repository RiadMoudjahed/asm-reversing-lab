/* Test harness for exercise 13. Reads integers from standard input and prints
 * mix_asm(x) for each, one per line.
 */
#include <stdint.h>
#include <stdio.h>

int32_t mix_asm(int32_t x);

int main(void)
{
    long x;
    setvbuf(stdout, NULL, _IONBF, 0);
    while (scanf("%ld", &x) == 1)
        printf("%d\n", (int)mix_asm((int32_t)x));
    return 0;
}
