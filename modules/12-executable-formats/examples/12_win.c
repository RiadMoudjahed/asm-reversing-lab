/* Purpose : a small Windows console program to inspect with 12_pe_dump.py.
 * Build   : x86_64-w64-mingw32-gcc -O1 -s 12_win.c -o 12_win.exe
 */
#include <stdio.h>
#include <windows.h>

int main(void)
{
    puts("hello from windows");
    Sleep(1);
    return 0;
}
