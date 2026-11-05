/* Test harness for exercise 19. Each input line is: CAP HEX
 *   CAP  the size of the output buffer you are given, in bytes
 *   HEX  the bytes of one encoded record, in hexadecimal
 * The harness places the record so that the byte after its last byte is on an unmapped page, so
 * the number of bytes available is exactly the length of HEX divided by two. It calls your
 * decode_record and prints the return value, the bytes of the output (the text and its zero
 * byte) when the return value is not negative, and a status.
 * The status is "ok" or a comma-separated list of:
 *   overrun  your code wrote beyond the output buffer
 *   regs     a callee-saved register was changed
 * Reading beyond the end of the record ends the program with a signal.
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

long decode_record(const unsigned char *rec, size_t avail, char *out, size_t cap);

uint64_t call_checked(void *fn, const uint64_t *args, uint64_t *saved);

__asm__(
    ".text\n"
    ".globl call_checked\n"
    "call_checked:\n"
    "    push %rbx\n    push %rbp\n    push %r12\n    push %r13\n    push %r14\n    push %r15\n"
    "    sub $24, %rsp\n"                   /* 6 pushes + 24 = 72: rsp is 16-byte aligned */
    "    mov %rsi, %r10\n"                  /* args */
    "    mov %rdx, 0(%rsp)\n"               /* saved */
    "    mov %rdi, %rax\n"
    "    mov $0x1111111111111111, %rbx\n    mov $0x2222222222222222, %rbp\n"
    "    mov $0x3333333333333333, %r12\n    mov $0x4444444444444444, %r13\n"
    "    mov $0x5555555555555555, %r14\n    mov $0x6666666666666666, %r15\n"
    "    mov 0(%r10), %rdi\n    mov 8(%r10), %rsi\n    mov 16(%r10), %rdx\n"
    "    mov 24(%r10), %rcx\n   mov 32(%r10), %r8\n"
    "    call *%rax\n"
    "    mov 0(%rsp), %r10\n"
    "    mov %rbx, 0(%r10)\n    mov %rbp, 8(%r10)\n    mov %r12, 16(%r10)\n"
    "    mov %r13, 24(%r10)\n   mov %r14, 32(%r10)\n   mov %r15, 40(%r10)\n"
    "    add $24, %rsp\n"
    "    pop %r15\n    pop %r14\n    pop %r13\n    pop %r12\n    pop %rbp\n    pop %rbx\n"
    "    ret\n");

static const uint64_t sentinel[6] = {0x1111111111111111ULL, 0x2222222222222222ULL,
    0x3333333333333333ULL, 0x4444444444444444ULL, 0x5555555555555555ULL,
    0x6666666666666666ULL};

int main(void)
{
    static char line[1 << 12];
    static unsigned char tmp[600];
    long pagesz = sysconf(_SC_PAGESIZE);
    size_t region = 2 * (size_t)pagesz;
    unsigned char *base = mmap(NULL, region + pagesz, PROT_READ | PROT_WRITE,
                               MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    mprotect(base + region, pagesz, PROT_NONE);
    setvbuf(stdout, NULL, _IONBF, 0);
    while (fgets(line, sizeof line, stdin)) {
        unsigned long cap = 0;
        char *s = strtok(line, " \n");
        if (!s)
            continue;
        cap = strtoul(s, NULL, 10);
        s = strtok(NULL, " \n");
        size_t n = 0;
        if (s && strcmp(s, "-"))
            for (size_t i = 0; s[i]; i += 2) {
                unsigned v;
                sscanf(s + i, "%2x", &v);
                tmp[n++] = (unsigned char)v;
            }
        unsigned char *rec = base + region - n;
        memcpy(rec, tmp, n);
        unsigned char *out = malloc(cap + 16);
        memset(out, 0xCC, cap + 16);
        uint64_t saved[6], args[5] = {0};
        args[0] = (uint64_t)(uintptr_t)rec; args[1] = n;
        args[2] = (uint64_t)(uintptr_t)out; args[3] = cap;
        long r = (long)call_checked((void *)decode_record, args, saved);
        int regs_bad = 0, overrun = 0;
        for (int i = 0; i < 6; i++) regs_bad |= saved[i] != sentinel[i];
        for (size_t i = cap; i < cap + 16; i++) overrun |= out[i] != 0xCC;
        printf("%ld ", r);
        if (r >= 0)
            for (long i = 0; i <= r && (unsigned long)i < cap; i++)
                printf("%02x", out[i]);
        else
            printf("-");
        printf(" ");
        if (!overrun && !regs_bad)
            printf("ok");
        else
            printf("%s%s%s", overrun ? "overrun" : "", overrun && regs_bad ? "," : "", regs_bad ? "regs" : "");
        printf("\n");
        free(out);
    }
    return 0;
}
