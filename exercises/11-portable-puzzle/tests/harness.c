/* Test harness for exercise 11, built with the MinGW cross compiler and run under Wine.
 * It reads one case per line from standard input (all numbers in hexadecimal) and prints
 * "result status" for each:
 *
 *   R N  va vs rs ro  ...  RVA     pe_rva_to_offset on a table of N section headers
 *   E N  ENTRY  va vs rs ro  ...   pe_entry_offset on a small PE image with N sections
 *   M K                            pe_entry_offset on a damaged image (K 0: no MZ, 1: no PE)
 *
 * va, vs, rs and ro are VirtualAddress, VirtualSize, SizeOfRawData and PointerToRawData.
 * log_rva() is provided here in assembly: it checks that the stack is aligned when it is
 * entered, scribbles over its shadow space and destroys every volatile register.
 * call_checked() loads sentinels into the non-volatile registers and verifies them.
 *
 * The status is "ok" or a comma-separated list of: align, regs, calls.
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

uint32_t pe_rva_to_offset(const void *sections, uint32_t count, uint32_t rva);
uint32_t pe_entry_offset(const void *file);
uint64_t call_checked(void *fn, uint64_t a, uint64_t b, uint64_t c, uint64_t *saved);
void log_rva(uint32_t rva);

static int log_calls, log_bad_align;
static uint32_t log_arg;

void log_note(uint32_t rva, uint64_t sp)
{
    log_calls++;
    log_arg = rva;
    if (sp & 15)
        log_bad_align++;
}

__asm__(
    ".intel_syntax noprefix\n"
    ".text\n"
    ".globl log_rva\n"
    "log_rva:\n"
    "    lea  rdx, [rsp + 8]\n"          /* the stack pointer before the call instruction */
    "    push rbp\n"
    "    mov  rbp, rsp\n"
    "    sub  rsp, 32\n"
    "    and  rsp, -16\n"
    "    call log_note\n"
    "    mov  rsp, rbp\n"
    "    pop  rbp\n"
    "    mov  rax, 0x5a5a5a5a5a5a5a5a\n"  /* scribble over the shadow space of this call */
    "    mov  [rsp + 8], rax\n    mov  [rsp + 16], rax\n    mov  [rsp + 24], rax\n    mov  [rsp + 32], rax\n"
    "    mov  rcx, rax\n    mov  rdx, rax\n    mov  r8, rax\n    mov  r9, rax\n"
    "    mov  r10, rax\n    mov  r11, rax\n"
    "    ret\n"
    ".globl call_checked\n"
    "call_checked:\n"
    "    push rbx\n    push rbp\n    push rdi\n    push rsi\n"
    "    push r12\n    push r13\n    push r14\n    push r15\n"
    "    mov  r10, [rsp + 104]\n"          /* the fifth argument: where to store the registers */
    "    sub  rsp, 56\n"
    "    mov  [rsp + 32], r10\n"
    "    mov  rax, rcx\n    mov  rcx, rdx\n    mov  rdx, r8\n    mov  r8, r9\n"
    "    mov  rbx, 0x1111111111111111\n    mov  rbp, 0x2222222222222222\n"
    "    mov  rdi, 0x3333333333333333\n    mov  rsi, 0x4444444444444444\n"
    "    mov  r12, 0x5555555555555555\n    mov  r13, 0x6666666666666666\n"
    "    mov  r14, 0x7777777777777777\n    mov  r15, 0x8888888888888888\n"
    "    call rax\n"
    "    mov  r10, [rsp + 32]\n"
    "    mov  [r10], rbx\n    mov  [r10 + 8], rbp\n    mov  [r10 + 16], rdi\n    mov  [r10 + 24], rsi\n"
    "    mov  [r10 + 32], r12\n    mov  [r10 + 40], r13\n    mov  [r10 + 48], r14\n    mov  [r10 + 56], r15\n"
    "    add  rsp, 56\n"
    "    pop  r15\n    pop  r14\n    pop  r13\n    pop  r12\n"
    "    pop  rsi\n    pop  rdi\n    pop  rbp\n    pop  rbx\n"
    "    ret\n"
    ".att_syntax prefix\n");

static void put32(uint8_t *p, uint32_t v) { memcpy(p, &v, 4); }
static void put16(uint8_t *p, uint16_t v) { memcpy(p, &v, 2); }

#define MAXSEC 16

static int read_sections(uint8_t *table, unsigned n)
{
    for (unsigned i = 0; i < n; i++) {
        unsigned va, vs, rs, ro;
        if (scanf("%x %x %x %x", &va, &vs, &rs, &ro) != 4)
            return 0;
        uint8_t *s = table + 40 * i;
        memset(s, 0, 40);
        memcpy(s, ".sec", 4);
        put32(s + 8, vs);
        put32(s + 12, va);
        put32(s + 16, rs);
        put32(s + 20, ro);
    }
    return 1;
}

static int status_text(char *out, size_t size, int bad_align, int regs_bad, int calls_bad)
{
    int len = 0;
    out[0] = 0;
    if (bad_align) len += snprintf(out + len, size - len, "%salign", len ? "," : "");
    if (regs_bad)  len += snprintf(out + len, size - len, "%sregs", len ? "," : "");
    if (calls_bad) len += snprintf(out + len, size - len, "%scalls", len ? "," : "");
    if (!len)
        snprintf(out, size, "ok");
    return len;
}

int main(void)
{
    static const uint64_t sentinel[8] = {0x1111111111111111ULL, 0x2222222222222222ULL,
        0x3333333333333333ULL, 0x4444444444444444ULL, 0x5555555555555555ULL,
        0x6666666666666666ULL, 0x7777777777777777ULL, 0x8888888888888888ULL};
    char kind;
    setvbuf(stdout, NULL, _IONBF, 0);
    while (scanf(" %c", &kind) == 1) {
        uint8_t image[4096];
        uint64_t saved[8] = {0};
        uint64_t raw = 0;
        int expect_calls = 0;
        uint32_t expect_arg = 0;
        log_calls = log_bad_align = 0;
        if (kind == 'R') {
            unsigned n, rva;
            uint8_t table[40 * MAXSEC];
            if (scanf("%x", &n) != 1 || n > MAXSEC || !read_sections(table, n) || scanf("%x", &rva) != 1)
                return 2;
            raw = call_checked((void *)pe_rva_to_offset, (uint64_t)(uintptr_t)table, n, rva, saved);
        } else if (kind == 'E') {
            unsigned n, entry;
            if (scanf("%x %x", &n, &entry) != 2 || n > MAXSEC)
                return 2;
            memset(image, 0, sizeof image);
            memcpy(image, "MZ", 2);
            put32(image + 0x3c, 0x80);
            memcpy(image + 0x80, "PE\0\0", 4);
            put16(image + 0x84, 0x8664);
            put16(image + 0x86, (uint16_t)n);
            put16(image + 0x94, 240);
            put16(image + 0x98, 0x20b);
            put32(image + 0x98 + 16, entry);
            if (!read_sections(image + 0x98 + 240, n))
                return 2;
            expect_calls = 1;
            expect_arg = entry;
            raw = call_checked((void *)pe_entry_offset, (uint64_t)(uintptr_t)image, 0, 0, saved);
        } else if (kind == 'M') {
            unsigned k;
            if (scanf("%x", &k) != 1)
                return 2;
            memset(image, 0, sizeof image);
            memcpy(image, "MZ", 2);
            put32(image + 0x3c, 0x80);
            memcpy(image + 0x80, "PE\0\0", 4);
            put16(image + 0x86, 1);
            put16(image + 0x94, 240);
            if (k == 0) image[0] = 'X';
            if (k == 1) image[0x80] = 'X';
            raw = call_checked((void *)pe_entry_offset, (uint64_t)(uintptr_t)image, 0, 0, saved);
        } else {
            return 2;
        }
        int regs_bad = 0;
        for (int i = 0; i < 8; i++)
            regs_bad |= saved[i] != sentinel[i];
        int calls_bad = log_calls != expect_calls || (expect_calls && log_arg != expect_arg);
        char status[48];
        status_text(status, sizeof status, log_bad_align, regs_bad, calls_bad);
        printf("%08x %s\n", (uint32_t)raw, status);
    }
    return 0;
}
