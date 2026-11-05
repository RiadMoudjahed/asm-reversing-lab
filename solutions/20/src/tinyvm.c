/* Exercise 20: a small virtual machine. Usage: tinyvm program.tvm
 * File format: "TVM1", code length (u16), data length (u16), the code, the data.
 * The data is copied to the start of the machine's 256 bytes of memory.
 * On success the number of executed instructions is written to standard error.
 */
#include <stdlib.h>
#include <string.h>

#include "ref.h"

enum { BAD, I_MOVI, I_MOV, I_ADD, I_SUB, I_MUL, I_XOR, I_AND, I_SHL, I_SHR, I_CMP, I_JMP, I_JZ,
       I_JNZ, I_CALL, I_RET, I_LDB, I_STB, I_OUT, I_OUTC, I_HALT };

static const uint8_t decode[256] = {
    [0x5B] = I_MOVI, [0xC2] = I_MOV,  [0x17] = I_ADD, [0x8E] = I_SUB,  [0x34] = I_MUL,
    [0xE9] = I_XOR,  [0x61] = I_AND,  [0xA5] = I_SHL, [0x0D] = I_SHR,  [0x7A] = I_CMP,
    [0xF3] = I_JMP,  [0x42] = I_JZ,   [0xB8] = I_JNZ, [0x29] = I_CALL, [0xD6] = I_RET,
    [0x93] = I_LDB,  [0x6C] = I_STB,  [0xFC] = I_OUT, [0x05] = I_OUTC, [0xA0] = I_HALT,
};

static const uint8_t length[21] = {0, 4, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 1, 2, 2, 2, 2, 1};

int main(int argc, char **argv)
{
    uint8_t file[1024], *code;
    struct vm m;
    size_t steps = 0, code_len, data_len;

    if (argc != 2) {
        fprintf(stderr, "usage: %s program.tvm\n", argv[0]);
        return 2;
    }
    FILE *f = fopen(argv[1], "rb");
    if (!f) {
        perror(argv[1]);
        return 2;
    }
    size_t n = fread(file, 1, sizeof file, f);
    fclose(f);
    if (n < 8 || memcmp(file, "TVM1", 4)) {
        fprintf(stderr, "not a TVM1 file\n");
        return 2;
    }
    code_len = file[4] | (file[5] << 8);
    data_len = file[6] | (file[7] << 8);
    if (code_len > 512 || data_len > 256 || 8 + code_len + data_len > n) {
        fprintf(stderr, "bad header\n");
        return 2;
    }
    code = file + 8;
    memset(&m, 0, sizeof m);
    memcpy(m.mem, code + code_len, data_len);

    while (!m.halted) {
        if (m.pc >= code_len || ++steps > 100000) {
            m.fault = 5;
            break;
        }
        const uint8_t *ins = code + m.pc;
        uint8_t idx = decode[ins[0]];
        if (!idx || m.pc + length[idx] > code_len) {
            m.fault = idx ? 6 : 1;
            break;
        }
        m.pc += length[idx];
        switch (idx) {
        case I_MOVI: ref_movi(&m, ins); break;
        case I_MOV: ref_mov(&m, ins); break;
        case I_ADD: ref_add(&m, ins); break;
        case I_SUB: ref_sub(&m, ins); break;
        case I_MUL: ref_mul(&m, ins); break;
        case I_XOR: ref_xor(&m, ins); break;
        case I_AND: ref_and(&m, ins); break;
        case I_SHL: ref_shl(&m, ins); break;
        case I_SHR: ref_shr(&m, ins); break;
        case I_CMP: ref_cmp(&m, ins); break;
        case I_JMP: ref_jmp(&m, ins); break;
        case I_JZ: ref_jz(&m, ins); break;
        case I_JNZ: ref_jnz(&m, ins); break;
        case I_CALL: ref_call(&m, ins); break;
        case I_RET: ref_ret(&m, ins); break;
        case I_LDB: ref_ldb(&m, ins); break;
        case I_STB: ref_stb(&m, ins); break;
        case I_OUT: ref_out(&m, ins); break;
        case I_OUTC: ref_outc(&m, ins); break;
        case I_HALT: ref_halt(&m, ins); break;
        }
    }
    fflush(stdout);
    if (m.fault) {
        fprintf(stderr, "fault %u at offset %u\n", m.fault, m.pc);
        return 1;
    }
    fprintf(stderr, "halt after %zu steps\n", steps);
    return 0;
}
