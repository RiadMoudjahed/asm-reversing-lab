/* Module 21: a small register machine whose opcodes are dispatched through a table.
 * Build: gcc -std=gnu11 -O1 -fno-pie -no-pie 21_regvm.c -o 21_regvm
 * Run  : 21_regvm program.bin
 *
 * Every instruction is 4 bytes: opcode, a, b, c. There are 4 registers, r0 to r3.
 *   10 MOVI a, n   r[a] = n (n is the 16-bit value b + 256 * c)
 *   11 ADD a,b,c   r[a] = r[b] + r[c]
 *   12 SUB a,b,c   r[a] = r[b] - r[c]
 *   13 MUL a,b,c   r[a] = r[b] * r[c]
 *   14 JNZ a, t    if r[a] != 0, continue at instruction t
 *   15 OUT a       print r[a] as a decimal number
 *   FF HALT        stop
 */
#include <stdint.h>
#include <stdio.h>

struct vm {
    int32_t r[4];
    size_t pc;
    int halted;
    int fault;
};

typedef void (*handler)(struct vm *, const uint8_t *);

static void op_movi(struct vm *m, const uint8_t *i) { m->r[i[1] & 3] = i[2] + 256 * i[3]; }
static void op_add(struct vm *m, const uint8_t *i) { m->r[i[1] & 3] = m->r[i[2] & 3] + m->r[i[3] & 3]; }
static void op_sub(struct vm *m, const uint8_t *i) { m->r[i[1] & 3] = m->r[i[2] & 3] - m->r[i[3] & 3]; }
static void op_mul(struct vm *m, const uint8_t *i) { m->r[i[1] & 3] = m->r[i[2] & 3] * m->r[i[3] & 3]; }
static void op_jnz(struct vm *m, const uint8_t *i) { if (m->r[i[1] & 3] != 0) m->pc = i[2]; }
static void op_out(struct vm *m, const uint8_t *i) { printf("%d\n", m->r[i[1] & 3]); }
static void op_halt(struct vm *m, const uint8_t *i) { (void)i; m->halted = 1; }

static const handler table[256] = {
    [0x10] = op_movi, [0x11] = op_add, [0x12] = op_sub, [0x13] = op_mul,
    [0x14] = op_jnz,  [0x15] = op_out, [0xff] = op_halt,
};

int main(int argc, char **argv)
{
    if (argc != 2) { fprintf(stderr, "usage: %s program.bin\n", argv[0]); return 2; }
    FILE *f = fopen(argv[1], "rb");
    if (!f) { perror(argv[1]); return 2; }
    uint8_t code[256];
    size_t n = fread(code, 1, sizeof code, f) / 4;
    fclose(f);

    struct vm m = {{0}, 0, 0, 0};
    size_t steps = 0;
    while (!m.halted && m.pc < n && steps < 100000) {
        const uint8_t *ins = code + 4 * m.pc++;
        handler h = table[ins[0]];
        if (!h) { fprintf(stderr, "bad opcode 0x%02x at instruction %zu\n", ins[0], m.pc - 1); return 1; }
        h(&m, ins);
        steps++;
    }
    if (!m.halted) { fprintf(stderr, "stopped without halt after %zu steps\n", steps); return 1; }
    printf("halt after %zu steps\n", steps);
    return 0;
}
