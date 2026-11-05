/* The same operations written in C, so you can compare with what a compiler produces.
 * Build: gcc -O1 -fcf-protection=none -c 07_compiled.c   (and again with -Os)
 *        objdump -d -M intel --no-show-raw-insn 07_compiled.o */
#include <string.h>
struct big { char data[256]; };
void clear_struct(struct big *p)                  { memset(p, 0, sizeof *p); }
void copy_struct(struct big *d, const struct big *s) { *d = *s; }
void copy_var(char *d, const char *s, unsigned long n) { memcpy(d, s, n); }
unsigned long length(const char *s)               { return strlen(s); }
