/* Test harness for exercise 06. Reads one case per line from standard input:
 *
 *   A B C       -> prints the value returned by maze_ok(A, B, C)
 *
 * A, B and C are signed decimal 32-bit integers.
 */
#include <stdint.h>
#include <stdio.h>

int32_t maze_ok(int32_t a, int32_t b, int32_t c);

int main(void)
{
    int a, b, c;
    setvbuf(stdout, NULL, _IONBF, 0);
    while (scanf("%d %d %d", &a, &b, &c) == 3)
        printf("%d\n", (int)maze_ok(a, b, c));
    return 0;
}
