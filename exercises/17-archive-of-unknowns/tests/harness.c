/* Test harness for exercise 17. Each input line starts without an archive and holds commands
 * separated by semicolons:
 *   a HEX        make an archive from these bytes ("-" for an empty one); it is placed so that
 *                the byte after its last byte is on an unmapped page
 *   c            call your ark_count
 *   n IDX CAP    call your ark_name with the index and an output capacity
 *   x IDX CAP    call your ark_extract with the index and an output capacity
 * The output line holds one result per command, separated by " ; ". A result is the return
 * value, the bytes written (in hexadecimal; a name includes its zero byte) when the return
 * value is not negative, or "-" otherwise, and a status. The status is "ok" or a comma-separated
 * list of:
 *   overrun  your code wrote beyond the capacity it was given
 *   regs     a callee-saved register was changed
 * Reading beyond the end of the archive ends the program with a signal.
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

long ark_count(const unsigned char *a, size_t n);
long ark_name(const unsigned char *a, size_t n, long index, char *out, size_t cap);
long ark_extract(const unsigned char *a, size_t n, long index, unsigned char *out, size_t cap);

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

static void print_status(int overrun, int regs)
{
    char st[32] = "";
    int len = 0;
    if (overrun) len += snprintf(st + len, sizeof st - len, "%soverrun", len ? "," : "");
    if (regs)    len += snprintf(st + len, sizeof st - len, "%sregs", len ? "," : "");
    printf("%s", len ? st : "ok");
}

int main(void)
{
    static char line[1 << 17];
    long pagesz = sysconf(_SC_PAGESIZE);
    size_t region = 40 * (size_t)pagesz;
    unsigned char *base = mmap(NULL, region + pagesz, PROT_READ | PROT_WRITE,
                               MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    mprotect(base + region, pagesz, PROT_NONE);
    setvbuf(stdout, NULL, _IONBF, 0);
    while (fgets(line, sizeof line, stdin)) {
        unsigned char *arc = base + region;
        size_t n = 0;
        int first = 1;
        for (char *cmd = strtok(line, ";\n"); cmd; cmd = strtok(NULL, ";\n")) {
            char op = 0;
            long idx = 0;
            unsigned long cap = 0;
            static char hexs[1 << 16];
            if (!first)
                printf(" ; ");
            first = 0;
            sscanf(cmd, " %c", &op);
            if (op == 'a') {
                static unsigned char tmp[1 << 15];
                n = 0;
                if (sscanf(cmd, " a %65535s", hexs) == 1 && strcmp(hexs, "-"))
                    for (size_t i = 0; hexs[i]; i += 2) {
                        unsigned v;
                        sscanf(hexs + i, "%2x", &v);
                        tmp[n++] = (unsigned char)v;
                    }
                arc = base + region - n;
                memcpy(arc, tmp, n);
                printf("set %zu ok", n);
                continue;
            }
            uint64_t saved[6], args[5] = {0};
            int regs_bad = 0, overrun = 0;
            long r;
            args[0] = (uint64_t)(uintptr_t)arc; args[1] = n;
            if (op == 'c') {
                r = (long)call_checked((void *)ark_count, args, saved);
                for (int i = 0; i < 6; i++) regs_bad |= saved[i] != sentinel[i];
                printf("%ld ", r);
                print_status(0, regs_bad);
                continue;
            }
            sscanf(cmd, " %c %ld %lu", &op, &idx, &cap);
            unsigned char *buf = malloc(cap + 16);
            memset(buf, 0xCC, cap + 16);
            args[2] = (uint64_t)idx; args[3] = (uint64_t)(uintptr_t)buf; args[4] = cap;
            r = (long)call_checked(op == 'n' ? (void *)ark_name : (void *)ark_extract, args, saved);
            for (int i = 0; i < 6; i++) regs_bad |= saved[i] != sentinel[i];
            for (size_t i = cap; i < cap + 16; i++) overrun |= buf[i] != 0xCC;
            printf("%ld ", r);
            if (r >= 0) {
                size_t len = op == 'n' ? (size_t)r + 1 : (size_t)r;
                for (size_t i = 0; i < len && i < cap; i++)
                    printf("%02x", buf[i]);
            } else {
                printf("-");
            }
            printf(" ");
            print_status(overrun, regs_bad);
            free(buf);
        }
        printf("\n");
    }
    return 0;
}
