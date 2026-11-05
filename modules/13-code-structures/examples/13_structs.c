/* Purpose : small functions that show how a compiler turns C control flow into machine code.
 * Build   : see the Makefile; the lesson compares -O0 and -O2 output.
 */
#include <stddef.h>

int sum3(int a, int b, int c)
{
    int total = a + b;
    return total + c;
}

int clamp_low(int x, int low)
{
    if (x < low)
        return low;
    return x;
}

int sign_text(int x)
{
    if (x > 0)
        return 1;
    else if (x < 0)
        return -1;
    else
        return 0;
}

int pick(int a, int b)
{
    return a > b ? a - b : b - a;
}

int in_range(int x, int lo, int hi)
{
    return x >= lo && x <= hi;
}

int either(int a, int b)
{
    return a == 0 || b == 0;
}

int count_up(int n)
{
    int total = 0;
    for (int i = 0; i < n; i++)
        total += i;
    return total;
}

int count_while(const char *s)
{
    int n = 0;
    while (*s++)
        n++;
    return n;
}

int do_once(int n)
{
    int steps = 0;
    do {
        n = n / 2;
        steps++;
    } while (n > 0);
    return steps;
}

int dense(int x)
{
    switch (x) {
    case 0: return 10;
    case 1: return 20;
    case 2: return 35;
    case 3: return 41;
    case 4: return 52;
    case 5: return 63;
    default: return -1;
    }
}

int sparse(int x)
{
    switch (x) {
    case 3: return 1;
    case 400: return 2;
    case 9000: return 3;
    default: return 0;
    }
}

static int helper(int v)
{
    return v * 3 + 1;
}

int uses_helper(int v)
{
    return helper(v) + helper(v + 1);
}

int forwards(int v);

int tail(int v)
{
    return forwards(v + 1);
}

int act_a(int v);
int act_b(int v);
int act_c(int v);
int act_d(int v);
int act_e(int v);

int dispatch(int cmd, int v)
{
    switch (cmd) {
    case 0: return act_a(v);
    case 1: return act_b(v);
    case 2: return act_c(v);
    case 3: return act_d(v);
    case 4: return act_e(v);
    default: return -1;
    }
}

int many(int v)
{
    switch (v) {
    case 3: return act_a(v);
    case 40: return act_b(v);
    case 77: return act_c(v);
    case 150: return act_d(v);
    case 400: return act_e(v);
    case 900: return act_a(v + 1);
    case 2000: return act_b(v + 1);
    case 9000: return act_c(v + 1);
    default: return 0;
    }
}

int two_calls(int v)
{
    return act_a(v) + act_b(v + 1);
}
