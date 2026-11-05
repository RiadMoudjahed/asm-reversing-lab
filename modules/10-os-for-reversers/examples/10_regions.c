/* Purpose : show where a program's code, data, heap, stack and a library function live,
 *           and which permissions each region has.
 * Build   : gcc -std=c11 -Wall -Wextra -no-pie 10_regions.c -o 10_regions
 * Output  : one line per object, naming the region that contains it (addresses vary).
 */
#define _GNU_SOURCE
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int global_counter = 7;
static const char message[] = "constant";

static void region_of(const char *label, const void *p)
{
    FILE *f = fopen("/proc/self/maps", "r");
    char line[512];
    uintptr_t addr = (uintptr_t)p;
    while (f && fgets(line, sizeof line, f)) {
        unsigned long lo, hi;
        char perms[8], name[256] = "";
        if (sscanf(line, "%lx-%lx %7s %*s %*s %*s %255[^\n]", &lo, &hi, perms, name) >= 3 &&
            addr >= lo && addr < hi) {
            const char *shown = name[0] ? name : "(anonymous)";
            while (*shown == ' ')
                shown++;
            printf("%-22s %s  %s\n", label, perms, shown);
            break;
        }
    }
    if (f)
        fclose(f);
}

int main(void)
{
    int local = 1;
    void *heap = malloc(64);
    region_of("function main", (const void *)main);
    region_of("function printf", (const void *)printf);
    region_of("constant string", message);
    region_of("global variable", &global_counter);
    region_of("heap block", heap);
    region_of("local variable", &local);
    free(heap);
    return 0;
}
