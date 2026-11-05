/* Module 16 example: a test harness that checks hypotheses about the black-box library.
 * Each prototype below is a hypothesis recovered from the disassembly; each call confirms or
 * refutes it.
 */
#include <stdio.h>
#include <string.h>

int bb_clamp(int v, int lo, int hi);
unsigned bb_checksum(const unsigned char *p, size_t n, unsigned seed);
long bb_split(const char *s, char sep, char *out, size_t cap);
void bb_minmax(const int *a, size_t n, int *lo, int *hi);
double bb_lerp(double a, double b, double t);
void *bb_open(int start, int step);
int bb_next(void *c);
void bb_close(void *c);

int main(void)
{
    printf("clamp(5, 0, 3) = %d\n", bb_clamp(5, 0, 3));
    printf("clamp(-5, 0, 3) = %d\n", bb_clamp(-5, 0, 3));
    printf("clamp(2, 0, 3) = %d\n", bb_clamp(2, 0, 3));

    const unsigned char data[] = "abc";
    printf("checksum(\"abc\", 3, 0) = 0x%08x\n", bb_checksum(data, 3, 0));
    printf("checksum(\"abc\", 3, 1) = 0x%08x\n", bb_checksum(data, 3, 1));
    printf("checksum(\"abc\", 2, 0) = 0x%08x\n", bb_checksum(data, 2, 0));

    char out[8];
    printf("split(\"key=value\", '=', out, 8) = %ld, out = \"%s\"\n",
           bb_split("key=value", '=', out, 8), out);
    printf("split(\"abcdefghij\", ',', out, 8) = %ld\n", bb_split("abcdefghij", ',', out, 8));
    printf("split(\"abcdefg\", ',', out, 8) = %ld\n", bb_split("abcdefg", ',', out, 8));

    int a[] = {7, -3, 12, 0, 5};
    int lo = 0, hi = 0;
    bb_minmax(a, 5, &lo, &hi);
    printf("minmax = %d %d\n", lo, hi);

    printf("lerp(10.0, 20.0, 0.25) = %.2f\n", bb_lerp(10.0, 20.0, 0.25));

    void *c = bb_open(100, 5);
    for (int i = 0; i < 3; i++)
        printf("next = %d\n", bb_next(c));
    printf("counter bytes: ");
    for (int i = 0; i < 12; i++)
        printf("%02x ", ((unsigned char *)c)[i]);
    printf("\n");
    bb_close(c);
    return 0;
}
