/* Test harness for exercise 03. Reads one case per line from standard input:
 *
 *   add A B     -> "status result" for ledger_add(A, B, &out)
 *   sub A B     -> "status result" for ledger_sub(A, B, &out)
 *
 * A and B are signed decimal 64-bit integers. status is printed in decimal.
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

uint32_t ledger_add(int64_t a, int64_t b, int64_t *out);
uint32_t ledger_sub(int64_t a, int64_t b, int64_t *out);

int main(void)
{
    char op[8];
    long long a, b;
    setvbuf(stdout, NULL, _IONBF, 0);
    while (scanf("%7s %lld %lld", op, &a, &b) == 3) {
        int64_t out = 0x5555555555555555;           /* sentinel: must be overwritten */
        uint32_t status;
        if (strcmp(op, "add") == 0)
            status = ledger_add(a, b, &out);
        else if (strcmp(op, "sub") == 0)
            status = ledger_sub(a, b, &out);
        else
            return 2;
        printf("%u %lld\n", (unsigned)status, (long long)out);
    }
    return 0;
}
