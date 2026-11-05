/* Purpose : call the assembly routines in 07_strings.asm and print the results.
 * Target  : Linux x86-64, C11
 * Build   : see the Makefile
 */
#include <stddef.h>
#include <stdio.h>
#include <string.h>

size_t my_strlen(const char *s);
void *my_memchr(const void *s, int c, size_t n);
void *my_memset(void *d, int c, size_t n);
void *my_memcpy(void *d, const void *s, size_t n);
int my_memcmp(const void *a, const void *b, size_t n);
void *my_memmove(void *d, const void *s, size_t n);

static int direction_flag(void)
{
    unsigned long flags;
    __asm__ volatile("pushfq; pop %0" : "=r"(flags));
    return (flags >> 10) & 1;
}

int main(void)
{
    const char *word = "assembly";
    char buf[16];
    void *hit;

    printf("my_strlen(\"assembly\"), (\"\")  = %zu %zu\n", my_strlen(word), my_strlen(""));

    hit = my_memchr(word, 'm', 8);
    printf("my_memchr(\"assembly\", 'm', 8) = offset %ld\n", hit ? (char *)hit - word : -1L);
    printf("my_memchr(\"assembly\", 'z', 8) = %s\n", my_memchr(word, 'z', 8) ? "found" : "NULL");

    memset(buf, '.', sizeof buf); buf[8] = '\0';
    my_memset(buf, 'x', 6);
    printf("my_memset(buf, 'x', 6)        = %s\n", buf);

    memset(buf, '.', sizeof buf); buf[8] = '\0';
    my_memcpy(buf, word, 8);
    printf("my_memcpy(buf, \"assembly\", 8) = %s\n", buf);

    int lt = my_memcmp("abc", "abd", 3), eq = my_memcmp("abc", "abc", 3), gt = my_memcmp("abd", "abc", 3);
    printf("my_memcmp: abc vs abd, abc, abd vs abc = %s %s %s\n",
           lt < 0 ? "negative" : "other", eq == 0 ? "zero" : "other", gt > 0 ? "positive" : "other");

    strcpy(buf, "abcdefgh");
    my_memmove(buf + 2, buf, 6);
    printf("memmove up by 2 (overlap)      = %s\n", buf);
    strcpy(buf, "abcdefgh");
    my_memmove(buf, buf + 2, 6);
    printf("memmove down by 2 (overlap)    = %s\n", buf);

    printf("direction flag after the calls = %d\n", direction_flag());
    return 0;
}
