/* Test harness for exercise 16. Each input line is one test:
 *   d SEED HEX      call your digest_buffer on the bytes given in hexadecimal ("-" for none)
 *   p CAP N V1..VN  call your pack_values on N integers with an output capacity of CAP bytes
 * The output line for "d" is the result, then the status. For "p" it is the return value, the
 * contents of an output area of CAP + 4 bytes that was filled with cc before the call, then the
 * status. The status is "ok" or a comma-separated list of:
 *   handle   the library reports open handles after your call (a handle was not freed, or was
 *            freed twice)
 *   regs     a callee-saved register was changed
 *
 * sl_live() comes from bin/libsilent.so.
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned digest_buffer(const unsigned char *p, size_t n, unsigned seed);
int pack_values(const int *vals, size_t n, unsigned char *out, size_t cap);
int sl_live(void);

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

static void print_status(int handle_bad, int regs_bad)
{
    char st[32] = "";
    int len = 0;
    if (handle_bad) len += snprintf(st + len, sizeof st - len, "%shandle", len ? "," : "");
    if (regs_bad)   len += snprintf(st + len, sizeof st - len, "%sregs", len ? "," : "");
    printf("%s\n", len ? st : "ok");
}

int main(void)
{
    char op;
    setvbuf(stdout, NULL, _IONBF, 0);
    while (scanf(" %c", &op) == 1) {
        uint64_t saved[6], args[5] = {0};
        int regs_bad = 0;
        if (op == 'd') {
            unsigned seed;
            char hex[1024];
            static unsigned char buf[512];
            size_t n = 0;
            if (scanf("%u %1023s", &seed, hex) != 2)
                break;
            if (strcmp(hex, "-"))
                for (size_t i = 0; hex[i]; i += 2) {
                    unsigned v;
                    sscanf(hex + i, "%2x", &v);
                    buf[n++] = (unsigned char)v;
                }
            int base = sl_live();
            args[0] = (uint64_t)(uintptr_t)buf; args[1] = n; args[2] = seed;
            unsigned r = (unsigned)call_checked((void *)digest_buffer, args, saved);
            for (int i = 0; i < 6; i++) regs_bad |= saved[i] != sentinel[i];
            printf("0x%08x ", r);
            print_status(sl_live() != base, regs_bad);
        } else {
            size_t cap, n;
            int vals[64];
            unsigned char out[64];
            if (scanf("%zu %zu", &cap, &n) != 2)
                break;
            for (size_t i = 0; i < n; i++)
                if (scanf("%d", &vals[i]) != 1)
                    return 1;
            memset(out, 0xCC, sizeof out);
            args[0] = (uint64_t)(uintptr_t)vals; args[1] = n;
            args[2] = (uint64_t)(uintptr_t)out; args[3] = cap;
            int r = (int)call_checked((void *)pack_values, args, saved);
            for (int i = 0; i < 6; i++) regs_bad |= saved[i] != sentinel[i];
            printf("%d ", r);
            for (size_t i = 0; i < cap + 4; i++)
                printf("%02x", out[i]);
            printf(" ");
            print_status(0, regs_bad);
        }
    }
    return 0;
}
