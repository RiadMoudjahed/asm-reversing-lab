/* Purpose : small functions that show how a compiler writes arithmetic.
 * Build   : see the Makefile; the lesson reads the -O2 and -O0 output.
 */
#include <stdint.h>

int times5(int x)        { return x * 5; }
int times10(int x)       { return x * 10; }
int times7(int x)        { return x * 7; }
int times100(int x)      { return x * 100; }
int times_neg3(int x)    { return x * -3; }
int madd(int a, int b)   { return a * 8 + b + 3; }

unsigned udiv10(unsigned x)  { return x / 10; }
unsigned umod10(unsigned x)  { return x % 10; }
int sdiv10(int x)            { return x / 10; }
int smod10(int x)            { return x % 10; }
int sdiv2(int x)             { return x / 2; }
int sdiv8(int x)             { return x / 8; }
int smod8(int x)             { return x % 8; }
unsigned udiv8(unsigned x)   { return x / 8; }
unsigned udiv7(unsigned x)   { return x / 7; }
int sdiv_var(int a, int b)   { return a / b; }

int absval(int x)            { return x < 0 ? -x : x; }
int sign(int x)              { return (x > 0) - (x < 0); }
int is_zero(int x)           { return x == 0; }
int max2(int a, int b)       { return a > b ? a : b; }
unsigned umax2(unsigned a, unsigned b) { return a > b ? a : b; }

int less_s(int a, int b)         { return a < b; }
int less_u(unsigned a, unsigned b) { return a < b; }
unsigned udiv_var(unsigned a, unsigned b) { return a / b; }
unsigned mask_lt(unsigned a, unsigned b)  { return -(a < b); }
long extend_s(int x)         { return x; }
long extend_u(unsigned x)    { return x; }
