/* Purpose : show that page permissions are enforced by the processor and the kernel.
 *           A page is made read-only, and a write to it is caught as a fault.
 * Build   : gcc -std=c11 -Wall -Wextra -no-pie 10_perms.c -o 10_perms
 * Output  : the write succeeds, then fails after mprotect with a fault.
 */
#define _GNU_SOURCE
#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

static sigjmp_buf recover;

static void on_fault(int sig)
{
    (void)sig;
    siglongjmp(recover, 1);
}

int main(void)
{
    long page = sysconf(_SC_PAGESIZE);
    char *p = mmap(NULL, page, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (p == MAP_FAILED)
        return 1;

    strcpy(p, "writable");
    printf("page is read-write: wrote \"%s\"\n", p);

    mprotect(p, page, PROT_READ);
    signal(SIGSEGV, on_fault);
    if (sigsetjmp(recover, 1) == 0) {
        p[0] = 'X';
        printf("write to read-only page succeeded (unexpected)\n");
    } else {
        printf("page is read-only: the write caused a fault (SIGSEGV)\n");
    }
    printf("the contents are still \"%s\"\n", p);
    return 0;
}
