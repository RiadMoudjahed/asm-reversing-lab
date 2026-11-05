/* Test harness for exercise 12. Reads "cmd acc" pairs from standard input and prints
 * run_cmd(cmd, acc) as a signed decimal number, one per line.
 */
#include <stdint.h>
#include <stdio.h>

int32_t run_cmd(int32_t cmd, uint32_t acc);

int main(void)
{
    int cmd;
    unsigned acc;
    setvbuf(stdout, NULL, _IONBF, 0);
    while (scanf("%d %u", &cmd, &acc) == 2)
        printf("%d\n", (int)run_cmd(cmd, acc));
    return 0;
}
