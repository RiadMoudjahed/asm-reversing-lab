/* A function with locals and a call, to compare compiler output with and without
 * optimization.
 * Build: gcc -O0 -fcf-protection=none -c 08_compiled.c   (and again with -O2)
 *        objdump -d -M intel --no-show-raw-insn 08_compiled.o */
long helper(long x);
long with_locals(long a, long b)
{
    long arr[4];
    arr[0] = a;
    arr[1] = b;
    arr[2] = a + b;
    arr[3] = a * b;
    return helper(arr[2] + arr[3]) + arr[1];
}
