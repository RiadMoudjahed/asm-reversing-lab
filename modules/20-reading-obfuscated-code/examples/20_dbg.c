/* Module 20 example: two common ways in which a program notices a debugger. The program is
 * harmless: it only reports what it found. The status file is read first because a successful
 * PTRACE_TRACEME makes the parent process the tracer.
 */
#include <stdio.h>
#include <string.h>
#include <sys/ptrace.h>

static int tracer_pid(void)
{
    char line[128];
    FILE *f = fopen("/proc/self/status", "r");
    int pid = 0;
    if (!f)
        return -1;
    while (fgets(line, sizeof line, f))
        if (!strncmp(line, "TracerPid:", 10))
            sscanf(line + 10, "%d", &pid);
    fclose(f);
    return pid;
}

int main(void)
{
    int b = tracer_pid() != 0;                         /* check 1: who traces this process? */
    int a = ptrace(PTRACE_TRACEME, 0, 0, 0) == -1;     /* check 2: fails if a tracer is attached */
    printf("status check: %s\n", b ? "debugger detected" : "no debugger");
    printf("ptrace check: %s\n", a ? "debugger detected" : "no debugger");
    return 0;
}
