/* Test harness for exercise 08. Reads one case per line from standard input:
 *
 *   A B C    -> prints "r1 r2 r3 status"
 *
 * r1, r2 and r3 are the results of cc_f1, cc_f2 and cc_f3. The status is "ok" when the
 * stack pointer was the same before and after every call and "stack" when it was not.
 * The functions are called through prototypes that declare their calling conventions.
 */
#include <stdio.h>

int __attribute__((stdcall)) cc_f1(int a, int b, int c);
int cc_f2(int a, int b, int c);
int __attribute__((fastcall)) cc_f3(int a, int b, int c);

#define SP(var) __asm__ volatile("mov %%esp, %0" : "=r"(var))

int main(void)
{
    int a, b, c;
    setvbuf(stdout, NULL, _IONBF, 0);
    while (scanf("%d %d %d", &a, &b, &c) == 3) {
        unsigned s0, s1, s2, s3, s4, s5;
        int r1, r2, r3;
        SP(s0); r1 = cc_f1(a, b, c); SP(s1);
        SP(s2); r2 = cc_f2(a, b, c); SP(s3);
        SP(s4); r3 = cc_f3(a, b, c); SP(s5);
        int balanced = s0 == s1 && s2 == s3 && s4 == s5;
        printf("%d %d %d %s\n", r1, r2, r3, balanced ? "ok" : "stack");
    }
    return 0;
}
