/* Exercise 19: a harmless simulator. It never touches services, files, the network, logs or
 * other programs. Every step only prints what it would do. Its configuration is stored encoded
 * and decoded when the program starts.
 */
#include <stdio.h>
#include "blobs.h"
#include "decode.h"

static char cfg[CONFIG_COUNT][64];

static void load_config(void)
{
    const unsigned char *p = CONFIG;
    for (int i = 0; i < CONFIG_COUNT; i++) {
        size_t n = decode_record(p, cfg[i]);
        p += 2 + n;
    }
}

static void step_discover(void)
{
    puts("[sim] would query the list of running processes");
}

static void step_service(void)
{
    printf("[sim] would create the service <cfg %d> that runs <cfg %d>\n", 0, 1);
}

static void step_sideload(void)
{
    printf("[sim] would place the library <cfg %d> next to the program <cfg %d>\n", 2, 3);
}

static void step_dns(void)
{
    printf("[sim] would resolve names under <cfg %d> and carry data in the labels\n", 4);
}

static void step_cleanup(void)
{
    printf("[sim] would clear the event log <cfg %d>\n", 5);
}

int main(void)
{
    puts("[sim] simulator: no action is performed, the steps below are only printed");
    load_config();
    step_discover();
    step_service();
    step_sideload();
    step_dns();
    step_cleanup();
    return 0;
}
