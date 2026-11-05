/* Exercise 20: the state of the machine as the handlers see it.
 * Offsets, for NASM:  r 0, pc 32, zf 36, cf 40, fault 44, halted 48, depth 52, stack 56, mem 88
 * Total size 344 bytes. A handler is called as  void h_xxx(struct vm *m, const unsigned char *ins)
 * with m in rdi and ins in rsi. The program counter has already been moved past the
 * instruction when the handler runs.
 */
#include <stdint.h>

struct vm {
    uint32_t r[8];        /* registers r0 to r7 */
    uint32_t pc;          /* offset of the next instruction in the code */
    uint32_t zf;          /* zero flag: 0 or 1 */
    uint32_t cf;          /* carry flag: 0 or 1 */
    uint32_t fault;       /* 0, or the code of the error that stopped the machine */
    uint32_t halted;      /* 1 when the machine has stopped */
    uint32_t depth;       /* number of return addresses on the call stack */
    uint16_t stack[16];   /* return addresses */
    uint8_t  mem[256];    /* data memory */
};
