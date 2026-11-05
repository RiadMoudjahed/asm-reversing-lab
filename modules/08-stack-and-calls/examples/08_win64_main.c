/* Purpose : call the Microsoft x64 functions from a Windows program built with MinGW-w64.
 * Build   : see the Makefile
 */
#include <stdio.h>

long ms_sum4(long a, long b, long c, long d);
long ms_call_sum4(void);

int main(void)
{
    printf("ms_sum4(9, 8, 7, 6)  = %ld\n", ms_sum4(9, 8, 7, 6));
    printf("ms_call_sum4()       = %ld\n", ms_call_sum4());
    return 0;
}
