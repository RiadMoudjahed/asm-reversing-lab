/* Purpose : call the three 32-bit conventions and check that the stack pointer is the
 *           same before and after each call.
 * Target  : Linux x86 (32-bit), C11
 * Build   : see the Makefile
 */
#include <stdio.h>

int add3_cdecl(int a, int b, int c);
int __attribute__((stdcall)) add3_stdcall(int a, int b, int c);
int __attribute__((fastcall)) add3_fastcall(int a, int b, int c);

#define SP(var) __asm__ volatile("mov %%esp, %0" : "=r"(var))

int main(void)
{
    unsigned before, after;
    int r;

    SP(before); r = add3_cdecl(1, 2, 3);    SP(after);
    printf("cdecl:    add3(1, 2, 3) = %d, stack %s\n", r, before == after ? "balanced" : "UNBALANCED");
    SP(before); r = add3_stdcall(1, 2, 3);  SP(after);
    printf("stdcall:  add3(1, 2, 3) = %d, stack %s\n", r, before == after ? "balanced" : "UNBALANCED");
    SP(before); r = add3_fastcall(1, 2, 3); SP(after);
    printf("fastcall: add3(1, 2, 3) = %d, stack %s\n", r, before == after ? "balanced" : "UNBALANCED");
    return 0;
}
