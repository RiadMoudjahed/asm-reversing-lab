/* Exercise 20: the state of the machine and the reference behavior of every instruction.
 * Used by tinyvm.c (the challenge) and by gen_vectors.c (the test vectors of Part 2).
 * Register fields: the first register is the high nibble of the byte after the opcode, the
 * second is the low nibble; register numbers are taken modulo 8.
 */
#ifndef REF_H
#define REF_H
#include <stdint.h>
#include <stdio.h>

struct vm {
    uint32_t r[8];
    uint32_t pc;
    uint32_t zf;
    uint32_t cf;
    uint32_t fault;
    uint32_t halted;
    uint32_t depth;
    uint16_t stack[16];
    uint8_t mem[256];
};

#define RA(i) (((i)[1] >> 4) & 7)
#define RB(i) ((i)[1] & 7)
#define IMM16(i) ((uint32_t)(i)[2] | ((uint32_t)(i)[3] << 8))
#define TGT16(i) ((uint32_t)(i)[1] | ((uint32_t)(i)[2] << 8))

static inline void ref_movi(struct vm *m, const uint8_t *i) { m->r[RA(i)] = IMM16(i); }
static inline void ref_mov(struct vm *m, const uint8_t *i) { m->r[RA(i)] = m->r[RB(i)]; }

static inline void ref_add(struct vm *m, const uint8_t *i)
{
    uint64_t t = (uint64_t)m->r[RA(i)] + m->r[RB(i)];
    m->r[RA(i)] = (uint32_t)t;
    m->cf = (uint32_t)(t >> 32);
    m->zf = (uint32_t)t == 0;
}

static inline void ref_sub(struct vm *m, const uint8_t *i)
{
    uint32_t a = m->r[RA(i)], b = m->r[RB(i)];
    m->r[RA(i)] = a - b;
    m->cf = a < b;
    m->zf = a - b == 0;
}

static inline void ref_cmp(struct vm *m, const uint8_t *i)
{
    uint32_t a = m->r[RA(i)], b = m->r[RB(i)];
    m->cf = a < b;
    m->zf = a == b;
}

static inline void ref_mul(struct vm *m, const uint8_t *i)
{
    uint32_t v = m->r[RA(i)] * m->r[RB(i)];
    m->r[RA(i)] = v;
    m->cf = 0;
    m->zf = v == 0;
}

static inline void ref_xor(struct vm *m, const uint8_t *i)
{
    uint32_t v = m->r[RA(i)] ^ m->r[RB(i)];
    m->r[RA(i)] = v;
    m->cf = 0;
    m->zf = v == 0;
}

static inline void ref_and(struct vm *m, const uint8_t *i)
{
    uint32_t v = m->r[RA(i)] & m->r[RB(i)];
    m->r[RA(i)] = v;
    m->cf = 0;
    m->zf = v == 0;
}

/* The shift count is the whole low nibble, 0 to 15. A count of 0 changes nothing. */
static inline void ref_shl(struct vm *m, const uint8_t *i)
{
    uint32_t n = i[1] & 15, v = m->r[RA(i)];
    if (!n)
        return;
    m->cf = (v >> (32 - n)) & 1;
    v <<= n;
    m->r[RA(i)] = v;
    m->zf = v == 0;
}

static inline void ref_shr(struct vm *m, const uint8_t *i)
{
    uint32_t n = i[1] & 15, v = m->r[RA(i)];
    if (!n)
        return;
    m->cf = (v >> (n - 1)) & 1;
    v >>= n;
    m->r[RA(i)] = v;
    m->zf = v == 0;
}

static inline void ref_jmp(struct vm *m, const uint8_t *i) { m->pc = TGT16(i); }
static inline void ref_jz(struct vm *m, const uint8_t *i) { if (m->zf) m->pc = TGT16(i); }
static inline void ref_jnz(struct vm *m, const uint8_t *i) { if (!m->zf) m->pc = TGT16(i); }

/* The program counter has already been advanced past the instruction. */
static inline void ref_call(struct vm *m, const uint8_t *i)
{
    if (m->depth == 16) {
        m->fault = 3;
        m->halted = 1;
        return;
    }
    m->stack[m->depth++] = (uint16_t)m->pc;
    m->pc = TGT16(i);
}

static inline void ref_ret(struct vm *m, const uint8_t *i)
{
    (void)i;
    if (m->depth == 0) {
        m->fault = 2;
        m->halted = 1;
        return;
    }
    m->pc = m->stack[--m->depth];
}

static inline void ref_ldb(struct vm *m, const uint8_t *i) { m->r[RA(i)] = m->mem[m->r[RB(i)] & 255]; }
static inline void ref_stb(struct vm *m, const uint8_t *i) { m->mem[m->r[RA(i)] & 255] = (uint8_t)m->r[RB(i)]; }
static inline void ref_out(struct vm *m, const uint8_t *i) { printf("%u\n", m->r[RA(i)]); }
static inline void ref_outc(struct vm *m, const uint8_t *i) { putchar((int)(m->r[RA(i)] & 255)); }
static inline void ref_halt(struct vm *m, const uint8_t *i) { (void)i; m->halted = 1; }
#endif
