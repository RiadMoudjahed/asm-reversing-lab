/* The same ideas written in C, so you can compare with what a compiler produces.
 * Build: gcc -O1 -fcf-protection=none -c 06_compiled.c -o 06_compiled.o
 *        objdump -d -M intel --no-show-raw-insn 06_compiled.o */
long max_signed(long a, long b)              { return a > b ? a : b; }
unsigned long max_unsigned(unsigned long a, unsigned long b) { return a > b ? a : b; }
long sum_to(long n)
{
    long total = 0;
    for (long i = 1; i <= n; i++)
        total += i;
    return total;
}
long sum_array(const int *a, long n)
{
    long total = 0;
    for (long i = 0; i < n; i++)
        total += a[i];
    return total;
}
long classify(long x)
{
    if (x < 0)
        return -1;
    else if (x == 0)
        return 0;
    return 1;
}
