/* Test harness for exercise 14. Reads lines "kind a b c m" and prints "p c t status".
 *
 * For each line the harness asks your gadget_init to build an object in a buffer, then asks
 * your gadget_call for slots 0, 1 and 2 with the argument m. p, c and t are your three
 * results. The status is "ok" or a comma-separated list of:
 *   vptr    the first word of the object is not the value that gadget_vptr(kind) returns
 *   call    your gadget_call disagrees with a call made by the library through the same object
 *   canary  your gadget_init wrote beyond the first 32 bytes of the buffer
 *   regs    a callee-saved register was changed
 *
 * gadget_vptr() and gadget_slot() come from bin/gadgetlib.o.
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

const void *gadget_vptr(int kind);
int gadget_slot(const void *obj, int slot, int arg);
void gadget_init(void *mem, int kind, int a, int b, int c);
int gadget_call(const void *obj, int slot, int arg);

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

int main(void)
{
    static const uint64_t sentinel[6] = {0x1111111111111111ULL, 0x2222222222222222ULL,
        0x3333333333333333ULL, 0x4444444444444444ULL, 0x5555555555555555ULL,
        0x6666666666666666ULL};
    int kind, a, b, c, m;
    setvbuf(stdout, NULL, _IONBF, 0);
    while (scanf("%d %d %d %d %d", &kind, &a, &b, &c, &m) == 5) {
        _Alignas(16) unsigned char buf[64];
        uint64_t saved[6], args[5];
        int regs_bad = 0, res[3], call_bad = 0;
        memset(buf, 0xCC, sizeof buf);
        args[0] = (uint64_t)(uintptr_t)buf; args[1] = (uint32_t)kind; args[2] = (uint32_t)a;
        args[3] = (uint32_t)b; args[4] = (uint32_t)c;
        call_checked((void *)gadget_init, args, saved);
        for (int i = 0; i < 6; i++) regs_bad |= saved[i] != sentinel[i];
        int vptr_bad = memcmp(buf, (const void *[]){gadget_vptr(kind)}, 8) != 0;
        int canary_bad = 0;
        for (int i = 32; i < 64; i++) canary_bad |= buf[i] != 0xCC;
        res[0] = res[1] = res[2] = 0;
        for (int slot = 0; slot < 3 && !vptr_bad; slot++) {   /* a wrong vptr would crash the call */
            args[0] = (uint64_t)(uintptr_t)buf; args[1] = (uint32_t)slot; args[2] = (uint32_t)m;
            res[slot] = (int)call_checked((void *)gadget_call, args, saved);
            for (int i = 0; i < 6; i++) regs_bad |= saved[i] != sentinel[i];
            if (res[slot] != gadget_slot(buf, slot, m)) call_bad = 1;
        }
        char st[48] = "";
        int len = 0;
        if (vptr_bad)   len += snprintf(st + len, sizeof st - len, "%svptr", len ? "," : "");
        if (call_bad)   len += snprintf(st + len, sizeof st - len, "%scall", len ? "," : "");
        if (canary_bad) len += snprintf(st + len, sizeof st - len, "%scanary", len ? "," : "");
        if (regs_bad)   len += snprintf(st + len, sizeof st - len, "%sregs", len ? "," : "");
        printf("%d %d %d %s\n", res[0], res[1], res[2], len ? st : "ok");
    }
    return 0;
}
