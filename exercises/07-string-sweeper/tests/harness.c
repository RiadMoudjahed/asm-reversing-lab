/* Test harness for exercise 07. Reads one case per line from standard input and prints
 * "OK" or "BAD <reason>" for each.
 *
 *   strlen N            a string of N non-zero bytes, then the terminator
 *   memchr N C POS      N bytes; byte C at index POS (POS -1: C is not present)
 *   memcpy N            copy N bytes between two separate blocks
 *   memmove N DELTA     move N bytes inside one block, the destination DELTA bytes
 *                       above (DELTA > 0) or below (DELTA < 0) the source
 *
 * Every block ends right before an inaccessible page, so reading or writing past the
 * end crashes the test. The direction flag is cleared before each call and checked
 * afterwards; memchr is called with ZF set.
 */
#define _GNU_SOURCE
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

size_t my_strlen(const char *s);
void *my_memchr(const void *s, int c, size_t n);
void *my_memcpy(void *d, const void *s, size_t n);
void *my_memmove(void *d, const void *s, size_t n);

static uint8_t *guarded(size_t n)          /* n usable bytes ending at an inaccessible page */
{
    long page = sysconf(_SC_PAGESIZE);
    size_t data_pages = (n + page - 1) / page + 1;
    uint8_t *base = mmap(NULL, (data_pages + 1) * page, PROT_READ | PROT_WRITE,
                         MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (base == MAP_FAILED || mprotect(base + data_pages * page, page, PROT_NONE) != 0)
        return NULL;
    memset(base, 0xA5, data_pages * page);
    return base + data_pages * page - n;
}

static void clear_df(void) { __asm__ volatile("cld"); }

/* Reports whether DF was left set, and clears it so the C library keeps working. */
static int df_set(void)
{
    unsigned long flags;
    __asm__ volatile("pushfq; pop %0; cld" : "=r"(flags));
    return (flags >> 10) & 1;
}

/* Calls my_memchr with ZF forced to 1, the worst case for a routine that wrongly relies on
 * the flags left by an earlier instruction when n is 0. */
static void *call_memchr(const void *s, int c, size_t n)
{
    register const void *a1 __asm__("rdi") = s;
    register long a2 __asm__("rsi") = c;
    register size_t a3 __asm__("rdx") = n;
    void *ret;
    __asm__ volatile("xor %%eax, %%eax\n\tcall my_memchr"
                     : "=a"(ret), "+r"(a1), "+r"(a2), "+r"(a3)
                     :
                     : "rcx", "r8", "r9", "r10", "r11", "cc", "memory");
    return ret;
}

static void pattern(uint8_t *p, size_t n)
{
    for (size_t i = 0; i < n; i++)
        p[i] = (uint8_t)(i % 251 + 1);       /* never zero */
}

int main(void)
{
    char cmd[16];
    long n, a, b;
    setvbuf(stdout, NULL, _IONBF, 0);
    while (scanf("%15s", cmd) == 1) {
        if (strcmp(cmd, "strlen") == 0 && scanf("%ld", &n) == 1) {
            uint8_t *s = guarded(n + 1);
            pattern(s, n);
            s[n] = 0;
            clear_df();
            size_t got = my_strlen((const char *)s);
            if (df_set()) puts("BAD direction flag left set");
            else if (got != (size_t)n) printf("BAD strlen returned %zu, expected %ld\n", got, n);
            else puts("OK");
        } else if (strcmp(cmd, "memchr") == 0 && scanf("%ld %ld %ld", &n, &a, &b) == 3) {
            uint8_t *s = guarded(n ? n : 1);
            memset(s, (uint8_t)(((unsigned)a + 1) & 0xFF), n ? n : 1);
            if (b >= 0) s[b] = (uint8_t)a;
            clear_df();
            void *got = call_memchr(s, (int)a, (size_t)n);
            void *want = b >= 0 ? (void *)(s + b) : NULL;
            if (df_set()) puts("BAD direction flag left set");
            else if (got != want) {
                if (!want)
                    printf("BAD memchr returned a pointer (offset %ld) but the byte is not in the %ld bytes\n",
                           (long)((uint8_t *)got - s), n);
                else if (!got)
                    printf("BAD memchr returned NULL but the byte is at offset %ld\n", b);
                else
                    printf("BAD memchr returned offset %ld, expected %ld\n", (long)((uint8_t *)got - s), b);
            }
            else puts("OK");
        } else if (strcmp(cmd, "memcpy") == 0 && scanf("%ld", &n) == 1) {
            uint8_t *src = guarded(n ? n : 1), *dst = guarded(n ? n : 1);
            pattern(src, n);
            clear_df();
            void *got = my_memcpy(dst, src, (size_t)n);
            if (df_set()) puts("BAD direction flag left set");
            else if (got != dst) puts("BAD memcpy did not return the destination");
            else if (n && memcmp(dst, src, n) != 0) puts("BAD memcpy copied wrong data");
            else puts("OK");
        } else if (strcmp(cmd, "memmove") == 0 && scanf("%ld %ld", &n, &a) == 2) {
            size_t total = (size_t)n + (size_t)(a < 0 ? -a : a);
            uint8_t *blk = guarded(total ? total : 1);
            uint8_t *want = guarded(total ? total : 1);
            pattern(blk, total);
            memcpy(want, blk, total);
            size_t src_off = a < 0 ? (size_t)-a : 0;
            memmove(want + src_off + a, want + src_off, (size_t)n);
            clear_df();
            void *got = my_memmove(blk + src_off + a, blk + src_off, (size_t)n);
            if (df_set()) puts("BAD direction flag left set");
            else if (got != (void *)(blk + src_off + a)) puts("BAD memmove did not return the destination");
            else if (total && memcmp(blk, want, total) != 0) puts("BAD memmove produced wrong data");
            else puts("OK");
        } else {
            return 2;
        }
    }
    return 0;
}
