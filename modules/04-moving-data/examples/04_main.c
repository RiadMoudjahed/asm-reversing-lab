/* Purpose : call the assembly functions in 04_addressing.asm and print the results.
 * Target  : Linux x86-64, C11
 * Build   : see the Makefile
 */
#include <stdint.h>
#include <stdio.h>

struct item { int32_t id; int32_t qty; int64_t price; };

long get_dword(const int32_t *a, long i);
unsigned load_u8(const uint8_t *p);
int load_s8(const uint8_t *p);
unsigned load_le16(const uint8_t *p);
long times5plus7(long x);
long item_price(const struct item *items, long i);
void set_second(int32_t *a);
void swap_ints(int32_t *a, int32_t *b);
long table_second(void);

int main(void)
{
    int32_t numbers[] = { 10, -20, 30, -40 };
    uint8_t bytes[]   = { 0xF6, 0x34, 0x12 };
    struct item items[] = { { 1, 5, 250 }, { 2, 7, 975 } };
    int32_t x = 1, y = 2;

    printf("get_dword(numbers, 3)    = %ld\n", get_dword(numbers, 3));
    printf("load_u8(0xF6)            = %u\n", load_u8(bytes));
    printf("load_s8(0xF6)            = %d\n", load_s8(bytes));
    printf("load_le16(34 12)         = 0x%04X\n", load_le16(bytes + 1));
    printf("times5plus7(6)           = %ld\n", times5plus7(6));
    printf("item_price(items, 1)     = %ld\n", item_price(items, 1));
    set_second(numbers);
    printf("after set_second         = %d %d %d %d\n", numbers[0], numbers[1], numbers[2], numbers[3]);
    swap_ints(&x, &y);
    printf("after swap_ints          = %d %d\n", x, y);
    printf("table_second()           = %ld\n", table_second());
    return 0;
}
