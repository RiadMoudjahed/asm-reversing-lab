/* Purpose : a small program to practice the reverser's toolkit on.
 * Build   : gcc -std=c11 -O1 -no-pie -fcf-protection=none -g0 11_target.c -o 11_target
 * Usage   : 11_target WORD    prints "access granted" for the right word, otherwise "denied"
 */
#include <stdio.h>
#include <string.h>

static const char secret[] = "open-sesame";

int score(const char *s)
{
    int total = 0;
    for (; *s; s++)
        total = total * 31 + *s;
    return total;
}

int check(const char *word)
{
    return strcmp(word, secret) == 0;
}

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "usage: %s WORD\n", argv[0]);
        return 2;
    }
    printf("score %d\n", score(argv[1]));
    if (check(argv[1])) {
        puts("access granted");
        return 0;
    }
    puts("denied");
    return 1;
}
