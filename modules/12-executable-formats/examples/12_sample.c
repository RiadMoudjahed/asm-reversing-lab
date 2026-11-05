/* Purpose : a small dynamically linked program to inspect with readelf and objdump.
 * Build   : gcc -std=c11 -O1 -no-pie -fcf-protection=none 12_sample.c -o 12_sample
 */
#include <stdio.h>

int counter = 5;
const char greeting[] = "hello";

int main(void)
{
    puts(greeting);
    printf("%d\n", counter);
    return 0;
}
