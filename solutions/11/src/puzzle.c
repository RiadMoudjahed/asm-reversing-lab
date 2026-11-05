/* Exercise 11 challenge program: a small Windows console program that imports from several
 * libraries and has an extra section. Build with the Makefile in this directory.
 */
#include <stdio.h>
#include <windows.h>

__attribute__((section(".ledger"), used)) static const char ledger[] = "portable-ledger-0042";

static volatile int tally = 17;
static char buffer[512];

int main(void)
{
    DWORD size = 64;
    char user[64] = "anonymous";
    GetUserNameA(user, &size);
    snprintf(buffer, sizeof buffer, "%s:%d:%d", ledger, tally, GetSystemMetrics(SM_CXSCREEN) > 0);
    printf("puzzle ready: %u\n", (unsigned)strlen(buffer));
    (void)user;
    return 0;
}
