/* Challenge program for exercise 06. Takes three integers on the command line and
 * prints "path open" when all checks pass, "wall" otherwise.
 * Build: make OUTDIR=<dir> */
#include <stdio.h>
#include <stdlib.h>

static int wall(void)
{
    puts("wall");
    return 1;
}

int main(int argc, char **argv)
{
    if (argc != 4) {
        puts("usage: maze A B C");
        return 2;
    }
    int a = atoi(argv[1]);
    int b = atoi(argv[2]);
    int c = atoi(argv[3]);

    if ((unsigned)a - 100u > 20u)
        return wall();
    if ((a & 7) != 5)
        return wall();
    if ((unsigned)(b + 31) > 10u)
        return wall();
    if (c >= 0)
        return wall();
    if ((unsigned char)c != 200)
        return wall();
    if (c < -300)
        return wall();

    puts("path open");
    return 0;
}
