/* Exercise 19 decoy: a benign deployment helper with a dry-run mode, whose messages also say
 * "would". */
#include <stdio.h>

int main(void)
{
    puts("[dry-run] would copy the release to the staging folder");
    puts("[dry-run] would restart the web service");
    puts("[dry-run] would write the log");
    return 0;
}
