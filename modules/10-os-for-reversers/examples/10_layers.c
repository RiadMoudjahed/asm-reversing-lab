/* Purpose : make the same write through two layers: the C library and a raw system call.
 * Build   : gcc -std=c11 -Wall -Wextra -no-pie 10_layers.c -o 10_layers
 * Output  : two lines; strace shows that both end in the same system call.
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

int main(void)
{
    const char *via_syscall = "through syscall()\n";
    puts("through puts()");
    fflush(stdout);
    syscall(SYS_write, 1, via_syscall, strlen(via_syscall));
    return 0;
}
