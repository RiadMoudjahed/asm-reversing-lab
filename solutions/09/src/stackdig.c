/* Exercise 09 challenge program. Build with the Makefile in this directory. */
#include <stddef.h>
#include <stdio.h>

static volatile long visits;

__attribute__((noinline)) void note(long depth, const long *entries)
{
    visits += depth + entries[1];
}

/* Walks down a chain of levels. Each level owns a small table that is handed to another
 * function, and keeps values alive across that call and the recursive call, so an
 * optimizing compiler has to build a real stack frame for it. */
__attribute__((noinline)) long chain(long depth, long seed)
{
    long table[3];
    table[0] = seed ^ (depth * 7);
    table[1] = seed + depth;
    table[2] = table[0] - table[1];
    note(depth, table);
    if (depth == 0)
        return seed & 0xff;
    long below = chain(depth - 1, seed * 3 + 1);
    return (below ^ table[0]) * 3 + (table[2] >> 1);
}

int main(void)
{
    long got = chain(9, 5);
    if (got != EXPECTED)
        return puts("stackdig: mismatch"), 1;
    puts("stackdig: ok");
    return 0;
}
