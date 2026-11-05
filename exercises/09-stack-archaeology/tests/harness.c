/* Test harness for exercise 09. Reads one case per line from standard input:
 *
 *   DEPTH SEED    -> prints "result status"
 *
 * The harness provides note(), which chain_asm must call once per level, from the outermost
 * level down to depth 0. note records the stack pointer, checks that it is 16-byte aligned,
 * checks the table the learner built, and then clobbers every caller-saved register.
 * chain_asm is entered through call_checked, which loads sentinel values into the
 * callee-saved registers and verifies them afterwards.
 *
 * The status is "ok" or a comma-separated list of: result, align, table, calls, regs, stack.
 */
#include <stdint.h>
#include <stdio.h>

typedef uint64_t u64;

long chain_asm(long depth, long seed);
long call_checked(long depth, long seed, u64 *saved);
void note_check(long depth, const long *table, u64 sp);

#define MAXLOG 4096
#define FOOTPRINT_LIMIT 64

static u64 sp_log[MAXLOG];
static long n_log, exp_depth, exp_seed;
static int bad_align, bad_table, bad_order;

void note_check(long depth, const long *t, u64 sp)
{
    if (n_log < MAXLOG)
        sp_log[n_log] = sp;
    n_log++;
    if (sp & 15)
        bad_align++;
    if (depth != exp_depth)
        bad_order++;
    u64 t0 = (u64)exp_seed ^ ((u64)exp_depth * 7);
    u64 t1 = (u64)exp_seed + (u64)exp_depth;
    if ((u64)t[0] != t0 || (u64)t[1] != t1 || (u64)t[2] != t0 - t1)
        bad_table++;
    exp_depth--;
    exp_seed = (long)((u64)exp_seed * 3 + 1);
}

/* note: sp passed to note_check is the stack pointer before the call instruction. */
__asm__(
    ".intel_syntax noprefix\n"
    ".text\n"
    ".globl note\n"
    ".type note, @function\n"
    "note:\n"
    "    lea  rdx, [rsp + 8]\n"
    "    push rbp\n"
    "    mov  rbp, rsp\n"
    "    and  rsp, -16\n"
    "    call note_check\n"
    "    mov  rsp, rbp\n"
    "    pop  rbp\n"
    "    mov  rax, 0x5a5a5a5a5a5a5a5a\n"
    "    mov  rcx, rax\n    mov  rdx, rax\n    mov  rsi, rax\n    mov  rdi, rax\n"
    "    mov  r8, rax\n    mov  r9, rax\n    mov  r10, rax\n    mov  r11, rax\n"
    "    ret\n"
    ".size note, .-note\n"
    ".globl call_checked\n"
    ".type call_checked, @function\n"
    "call_checked:\n"
    "    push rbx\n    push rbp\n    push r12\n    push r13\n    push r14\n    push r15\n"
    "    sub  rsp, 8\n"
    "    mov  [rsp], rdx\n"
    "    mov  rbx, 0x1111111111111111\n    mov  rbp, 0x2222222222222222\n"
    "    mov  r12, 0x3333333333333333\n    mov  r13, 0x4444444444444444\n"
    "    mov  r14, 0x5555555555555555\n    mov  r15, 0x6666666666666666\n"
    "    call chain_asm\n"
    "    mov  rcx, [rsp]\n"
    "    mov  [rcx], rbx\n    mov  [rcx + 8], rbp\n    mov  [rcx + 16], r12\n"
    "    mov  [rcx + 24], r13\n    mov  [rcx + 32], r14\n    mov  [rcx + 40], r15\n"
    "    add  rsp, 8\n"
    "    pop  r15\n    pop  r14\n    pop  r13\n    pop  r12\n    pop  rbp\n    pop  rbx\n"
    "    ret\n"
    ".size call_checked, .-call_checked\n"
    ".att_syntax prefix\n"
    ".section .note.GNU-stack,\"\",@progbits\n"
    ".text\n");

static long reference(long depth, long seed)
{
    u64 t0 = (u64)seed ^ ((u64)depth * 7);
    u64 t1 = (u64)seed + (u64)depth;
    long t2 = (long)(t0 - t1);
    if (depth == 0)
        return seed & 0xff;
    long below = reference(depth - 1, (long)((u64)seed * 3 + 1));
    return (long)((((u64)below ^ t0) * 3) + (u64)(t2 >> 1));
}

int main(void)
{
    static const u64 sentinels[6] = {0x1111111111111111ULL, 0x2222222222222222ULL,
                                     0x3333333333333333ULL, 0x4444444444444444ULL,
                                     0x5555555555555555ULL, 0x6666666666666666ULL};
    long depth, seed;
    setvbuf(stdout, NULL, _IONBF, 0);
    while (scanf("%ld %ld", &depth, &seed) == 2) {
        u64 saved[6] = {0};
        n_log = 0, exp_depth = depth, exp_seed = seed;
        bad_align = bad_table = bad_order = 0;
        long got = call_checked(depth, seed, saved);
        char status[64] = "";
        int len = 0;
        #define FLAG(cond, name) \
            do { if (cond) len += snprintf(status + len, sizeof status - len, "%s%s", len ? "," : "", name); } while (0)
        FLAG(got != reference(depth, seed), "result");
        FLAG(bad_align, "align");
        FLAG(bad_table, "table");
        FLAG(n_log != depth + 1 || bad_order, "calls");
        int regs = 0;
        for (int i = 0; i < 6; i++)
            regs |= saved[i] != sentinels[i];
        FLAG(regs, "regs");
        int heavy = 0;
        for (long i = 1; i < n_log && i < MAXLOG; i++)
            if (sp_log[i - 1] - sp_log[i] > FOOTPRINT_LIMIT)
                heavy = 1;
        FLAG(heavy, "stack");
        printf("%ld %s\n", got, len ? status : "ok");
    }
    return 0;
}
