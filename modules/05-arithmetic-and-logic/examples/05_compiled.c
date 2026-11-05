/* The same ideas written in C, so you can compare with what a compiler produces.
 * Build: gcc -O1 -fcf-protection=none -c 05_compiled.c -o 05_compiled.o
 *        objdump -d -M intel --no-show-raw-insn 05_compiled.o */
long times8(long x)                  { return x * 8; }
unsigned long div8_unsigned(unsigned long x) { return x / 8; }
long shift_signed(long x)            { return x >> 2; }
long divide_vars(long a, long b)     { return a / b; }
long remainder_vars(long a, long b)  { return a % b; }
unsigned mix(unsigned x)             { return ((x & 0xF0) | 0x0F) ^ 0xFF; }
