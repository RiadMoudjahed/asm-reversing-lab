/* Exercise 12 challenge program: a tiny command processor. It reads "command value" pairs
 * from standard input and prints the result of dispatch(). Build with the Makefile in this
 * directory.
 */
#include <stdint.h>
#include <stdio.h>

__attribute__((noinline)) int32_t dispatch(int32_t cmd, uint32_t acc)
{
    switch (cmd) {
    case 3:
        acc += 5;
        /* falls through */
    case 4:
        acc *= 3;
        break;
    case 5:
        acc -= 2;
        return (int32_t)acc;
    case 6:
    case 10:
        acc ^= 0x55;
        break;
    case 7:
        acc <<= 2;
        /* falls through */
    case 8:
        acc += 100;
        break;
    case 12:
        acc = -acc;
        break;
    case 13:
        acc |= 1;
        break;
    default:
        acc = 0xFFFFFFFFu;
        break;
    }
    return (int32_t)(acc + 1);
}

int main(void)
{
    int cmd;
    unsigned acc;
    while (scanf("%d %u", &cmd, &acc) == 2)
        printf("%d\n", dispatch(cmd, acc));
    return 0;
}
