/* Module 21: a small stack machine. It reads a bytecode file and runs it.
 * Build: gcc -std=gnu11 -O1 -fno-pie -no-pie 21_stackvm.c -o 21_stackvm
 * Run  : 21_stackvm program.bin
 *
 * Instruction set (one opcode byte, then the operands shown):
 *   00 HALT        stop
 *   01 PUSH n      push the byte n
 *   02 ADD         pop b, pop a, push a + b
 *   03 SUB         pop b, pop a, push a - b
 *   04 MUL         pop b, pop a, push a * b
 *   05 DUP         push a copy of the top value
 *   06 LOAD s      push variable s
 *   07 STORE s     pop into variable s
 *   08 JMP t       continue at offset t
 *   09 JZ t        pop; continue at offset t if it was zero
 *   0A PRINT       pop and print as a decimal number
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define STACK_SIZE 32
#define VARS 8

static int run(const uint8_t *code, size_t size)
{
    int32_t stack[STACK_SIZE];
    int32_t vars[VARS] = {0};
    size_t pc = 0, sp = 0, steps = 0;

    while (pc < size) {
        uint8_t op = code[pc++];
        steps++;
        switch (op) {
        case 0x00:
            printf("halt after %zu steps\n", steps);
            return 0;
        case 0x01:
            if (sp == STACK_SIZE || pc >= size) goto fault;
            stack[sp++] = code[pc++];
            break;
        case 0x02: case 0x03: case 0x04: {
            if (sp < 2) goto fault;
            int32_t b = stack[--sp], a = stack[--sp];
            stack[sp++] = op == 0x02 ? a + b : op == 0x03 ? a - b : a * b;
            break;
        }
        case 0x05:
            if (sp == 0 || sp == STACK_SIZE) goto fault;
            stack[sp] = stack[sp - 1];
            sp++;
            break;
        case 0x06:
            if (sp == STACK_SIZE || pc >= size || code[pc] >= VARS) goto fault;
            stack[sp++] = vars[code[pc++]];
            break;
        case 0x07:
            if (sp == 0 || pc >= size || code[pc] >= VARS) goto fault;
            vars[code[pc++]] = stack[--sp];
            break;
        case 0x08:
            if (pc >= size) goto fault;
            pc = code[pc];
            break;
        case 0x09:
            if (sp == 0 || pc >= size) goto fault;
            if (stack[--sp] == 0) pc = code[pc]; else pc++;
            break;
        case 0x0a:
            if (sp == 0) goto fault;
            printf("%d\n", stack[--sp]);
            break;
        default:
            fprintf(stderr, "bad opcode 0x%02x at offset %zu\n", op, pc - 1);
            return 1;
        }
    }
fault:
    fprintf(stderr, "fault at offset %zu\n", pc);
    return 1;
}

int main(int argc, char **argv)
{
    if (argc != 2) { fprintf(stderr, "usage: %s program.bin\n", argv[0]); return 2; }
    FILE *f = fopen(argv[1], "rb");
    if (!f) { perror(argv[1]); return 2; }
    uint8_t code[256];
    size_t n = fread(code, 1, sizeof code, f);
    fclose(f);
    return run(code, n);
}
