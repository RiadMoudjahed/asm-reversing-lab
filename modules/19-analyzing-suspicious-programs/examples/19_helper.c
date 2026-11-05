/* Module 19 example: a harmless program that is not malicious, used to test detection rules for
 * false positives. It prints help text and mentions a registry key, as documentation tools do.
 */
#include <stdio.h>

int main(void)
{
    puts("startup-report: prints the programs configured to start at logon.");
    puts("On Windows they are listed under HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Run.");
    puts("Use 'schtasks /query' to list scheduled tasks. This tool changes nothing.");
    return 0;
}
