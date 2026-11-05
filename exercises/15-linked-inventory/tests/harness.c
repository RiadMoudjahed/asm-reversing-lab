/* Test harness for exercise 15. Each input line starts with an empty tree and holds commands
 * separated by semicolons:
 *   i KEY     call your inv_insert for KEY
 *   r KEY N   call your inv_insert for KEY, N times
 *   f KEY     call your inv_find for KEY and print "count total status", or "none status"
 *   d         print the whole tree as text, then the status
 * The output line holds one result per command, separated by " ; ".
 * The tree text is produced by the test library from the nodes your code built, so it shows
 * what the layout really contains.
 *
 * The status is "ok" or a comma-separated list of:
 *   find     inv_find returned a different node than the library finds
 *   alloc    inv_alloc was called a different number of times than the new keys require
 *   overrun  your code wrote beyond the 32 bytes of a node, or into the 6 bytes after count
 *   align    inv_alloc was called with the stack misaligned
 *   regs     a callee-saved register was changed
 *
 * inv_dump(), inv_lookup() and inv_info() come from bin/inventorylib.o. inv_alloc() is defined
 * here: it returns a zeroed 32-byte block.
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void inv_dump(const void *root, char *out, size_t cap);
void *inv_lookup(void *root, int key);
void inv_info(const void *node, int *count, int *total);
void *inv_insert(void *root, int key);
void *inv_find(void *root, int key);
void *inv_alloc(void);

uint64_t call_checked(void *fn, const uint64_t *args, uint64_t *saved);

#define BLOCKS 4096
static unsigned char arena[BLOCKS][48];
static unsigned char spare[48];
static int used, alloc_calls, misaligned;

void *inv_alloc_c(void)
{
    unsigned char *b = used < BLOCKS ? arena[used++] : spare;
    memset(b, 0, 32);
    memset(b + 32, 0xA5, 16);
    alloc_calls++;
    return b;
}

__asm__(
    ".text\n"
    ".globl inv_alloc\n"
    "inv_alloc:\n"
    "    lea 8(%rsp), %rax\n"               /* rsp before the call must be 16-byte aligned */
    "    test $15, %al\n"
    "    jz 1f\n"
    "    movl $1, misaligned(%rip)\n"
    "1:  sub $8, %rsp\n"
    "    call inv_alloc_c\n"
    "    add $8, %rsp\n"
    "    ret\n"
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

static int seen[BLOCKS + 1], nseen;
static int regs_bad, find_bad, alloc_bad, overrun_bad;

static int is_new_key(int key)
{
    for (int i = 0; i < nseen; i++)
        if (seen[i] == key)
            return 0;
    if (nseen < BLOCKS)
        seen[nseen++] = key;
    return 1;
}

static uint64_t run(void *fn, uint64_t a, uint64_t b)
{
    uint64_t saved[6], args[2] = {a, b};
    uint64_t r = call_checked(fn, args, saved);
    for (int i = 0; i < 6; i++)
        regs_bad |= saved[i] != sentinel[i];
    return r;
}

static void check_guards(void)
{
    for (int i = 0; i < used; i++) {
        for (int j = 26; j < 32; j++)
            overrun_bad |= arena[i][j] != 0;        /* the padding after count stays zero */
        for (int j = 32; j < 48; j++)
            overrun_bad |= arena[i][j] != 0xA5;
    }
}

static void print_status(void)
{
    char st[48] = "";
    int len = 0;
    if (find_bad)    len += snprintf(st + len, sizeof st - len, "%sfind", len ? "," : "");
    if (alloc_bad)   len += snprintf(st + len, sizeof st - len, "%salloc", len ? "," : "");
    if (overrun_bad) len += snprintf(st + len, sizeof st - len, "%soverrun", len ? "," : "");
    if (misaligned)  len += snprintf(st + len, sizeof st - len, "%salign", len ? "," : "");
    if (regs_bad)    len += snprintf(st + len, sizeof st - len, "%sregs", len ? "," : "");
    printf("%s", len ? st : "ok");
    find_bad = alloc_bad = overrun_bad = misaligned = regs_bad = 0;
}

int main(void)
{
    static char text[1 << 18], line[1 << 15];
    setvbuf(stdout, NULL, _IONBF, 0);
    while (fgets(line, sizeof line, stdin)) {
        void *root = NULL;
        int first = 1;
        used = alloc_calls = misaligned = nseen = 0;
        regs_bad = find_bad = alloc_bad = overrun_bad = 0;
        for (char *cmd = strtok(line, ";\n"); cmd; cmd = strtok(NULL, ";\n")) {
            int key = 0;
            long n = 1;
            char op = 0;
            if (sscanf(cmd, " %c %d %ld", &op, &key, &n) < 1)
                continue;
            if (!first)
                printf(" ; ");
            first = 0;
            if (op == 'd') {
                check_guards();
                inv_dump(root, text, sizeof text);
                printf("%s ", text);
            } else if (op == 'f') {
                void *want = inv_lookup(root, key);
                void *got = (void *)run((void *)inv_find, (uint64_t)(uintptr_t)root,
                                        (uint32_t)key);
                find_bad |= got != want;
                check_guards();
                if (want && got == want) {
                    int count, total;
                    inv_info(got, &count, &total);
                    printf("%d %d ", count, total);
                } else {
                    printf("none ");
                }
            } else {
                int before = alloc_calls, fresh = is_new_key(key);
                if (op == 'i')
                    n = 1;
                for (long k = 0; k < n; k++)
                    root = (void *)run((void *)inv_insert, (uint64_t)(uintptr_t)root,
                                       (uint32_t)key);
                alloc_bad |= alloc_calls - before != fresh;
                check_guards();
            }
            print_status();
        }
        printf("\n");
    }
    return 0;
}
