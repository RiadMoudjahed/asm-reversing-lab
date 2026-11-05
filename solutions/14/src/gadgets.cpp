// Exercise 14 challenge program. It reads lines "kind a b c m" from standard input, builds one
// object of the class selected by kind, and prints power(m), cost(m) and tag(m).
// Build with the Makefile in this directory.
#include <cstdio>
#include <new>
#include "classes.h"

__attribute__((noinline)) Device *make_device(int kind, int a, int b, int c)
{
    switch (kind) {
    case 0: return new Device(a);
    case 1: return new Sensor(a, b);
    case 2: return new Camera(a, b, c);
    default: return new Lamp(a, b);
    }
}

int main()
{
    int kind, a, b, c, m;
    while (std::scanf("%d %d %d %d %d", &kind, &a, &b, &c, &m) == 5) {
        Device *d = make_device(kind & 3, a, b, c);
        std::printf("%d %d %d\n", d->power(m), d->cost(m), d->tag(m));
    }
    return 0;
}
