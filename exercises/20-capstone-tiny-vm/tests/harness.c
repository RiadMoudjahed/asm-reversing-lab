/* Test harness for exercise 20, Part 2 (the handlers). Each input line is: NAME INS STATE
 *   NAME   h_mul, h_shl, h_shr, h_cmp, h_call, h_ret, h_ldb or h_stb
 *   INS    the bytes of the instruction, in hexadecimal
 *   STATE  the 344 bytes of struct vm before the call, in hexadecimal
 * The harness calls the handler as  void NAME(struct vm *m, const unsigned char *ins)  and
 * prints the state after the call in hexadecimal, then a status: "ok" or "regs" when a
 * callee-saved register was changed.
 * The state is placed so that the byte after its last byte is on an unmapped page, and so is
 * the instruction; reading or writing beyond them ends the program with a signal.
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#define STATE_SIZE 344

void h_mul(void *m, const unsigned char *ins);
void h_shl(void *m, const unsigned char *ins);
void h_shr(void *m, const unsigned char *ins);
void h_cmp(void *m, const unsigned char *ins);
void h_call(void *m, const unsigned char *ins);
void h_ret(void *m, const unsigned char *ins);
void h_ldb(void *m, const unsigned char *ins);
void h_stb(void *m, const unsigned char *ins);

uint64_t call_checked(void *fn, const uint64_t *args, uint64_t *saved);

__asm__(
    ".text\n"
    ".globl call_checked\n"
    "call_checked:\n"
    "    push %rbx\n    push %rbp\n    push %r12\n    push %r13\n    push %r14\n    push %r15\n"
    "    sub $24, %rsp\n"
    "    mov %rsi, %r10\n"
    "    mov %rdx, 0(%rsp)\n"
    "    mov %rdi, %rax\n"
    "    mov $0x1111111111111111, %rbx\n    mov $0x2222222222222222, %rbp\n"
    "    mov $0x3333333333333333, %r12\n    mov $0x4444444444444444, %r13\n"
    "    mov $0x5555555555555555, %r14\n    mov $0x6666666666666666, %r15\n"
    "    mov 0(%r10), %rdi\n    mov 8(%r10), %rsi\n"
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

static const struct { const char *name; void *fn; } handlers[] = {
    {"h_mul", h_mul}, {"h_shl", h_shl}, {"h_shr", h_shr}, {"h_cmp", h_cmp},
    {"h_call", h_call}, {"h_ret", h_ret}, {"h_ldb", h_ldb}, {"h_stb", h_stb},
};

static unsigned char *edge_buffer(size_t size)
{
    long pagesz = sysconf(_SC_PAGESIZE);
    size_t region = 2 * (size_t)pagesz;
    unsigned char *base = mmap(NULL, region + pagesz, PROT_READ | PROT_WRITE,
                               MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    mprotect(base + region, pagesz, PROT_NONE);
    return base + region - size;
}

static size_t parse_hex(const char *s, unsigned char *out)
{
    size_t n = 0;
    for (size_t i = 0; s[i] && s[i + 1]; i += 2) {
        unsigned v;
        sscanf(s + i, "%2x", &v);
        out[n++] = (unsigned char)v;
    }
    return n;
}

int main(void)
{
    static char line[2048];
    unsigned char tmp[STATE_SIZE + 8];
    setvbuf(stdout, NULL, _IONBF, 0);
    while (fgets(line, sizeof line, stdin)) {
        char *name = strtok(line, " \n"), *ins_hex = strtok(NULL, " \n"), *state_hex = strtok(NULL, " \n");
        if (!name || !ins_hex || !state_hex)
            continue;
        void *fn = NULL;
        for (size_t i = 0; i < sizeof handlers / sizeof handlers[0]; i++)
            if (!strcmp(handlers[i].name, name))
                fn = handlers[i].fn;
        size_t n = parse_hex(ins_hex, tmp);
        unsigned char *ins = edge_buffer(n);
        memcpy(ins, tmp, n);
        unsigned char *state = edge_buffer(STATE_SIZE);
        parse_hex(state_hex, state);
        uint64_t saved[6], args[2] = {(uint64_t)(uintptr_t)state, (uint64_t)(uintptr_t)ins};
        call_checked(fn, args, saved);
        int regs_bad = 0;
        for (int i = 0; i < 6; i++)
            regs_bad |= saved[i] != sentinel[i];
        for (int i = 0; i < STATE_SIZE; i++)
            printf("%02x", state[i]);
        printf(" %s\n", regs_bad ? "regs" : "ok");
    }
    return 0;
}
