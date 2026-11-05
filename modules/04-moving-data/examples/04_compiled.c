/* The same ideas written in C, so you can compare with what a compiler produces.
 * Build: gcc -O1 -fcf-protection=none -c 04_compiled.c -o 04_compiled.o ; objdump -d -M intel 04_compiled.o */
#include <stdint.h>
struct item { int32_t id; int32_t qty; int64_t price; };
long get_dword(const int32_t *a, long i)  { return a[i]; }
unsigned load_u8(const uint8_t *p)        { return *p; }
int load_s8(const int8_t *p)              { return *p; }
long times5plus7(long x)                  { return x * 5 + 7; }
long item_price(const struct item *v, long i) { return v[i].price; }
