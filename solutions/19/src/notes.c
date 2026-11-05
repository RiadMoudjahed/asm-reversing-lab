/* Exercise 19 decoy: a benign program. Its help text mentions the same kind of things as the
 * simulator (a registry key, a service, a library) in plain words. */
#include <stdio.h>

int main(void)
{
    puts("notes: a small notes tool.");
    puts("Autostart entries live under HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Run.");
    puts("A service is created with the sc tool. A library named version.dll is a system file.");
    puts("This tool does none of these things.");
    return 0;
}
