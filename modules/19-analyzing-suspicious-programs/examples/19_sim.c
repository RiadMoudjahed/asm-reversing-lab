/* Module 19 example: a harmless simulator of an "updater" dropper.
 *
 * It never touches the registry, scheduled tasks, other processes, tokens or the network.
 * Every step is a function that only prints what it would do. Build: see the Makefile.
 */
#include <stdio.h>
#include <string.h>

static void say(const char *fmt, ...) __attribute__((format(printf, 1, 2)));

#include <stdarg.h>
static void say(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    fputs("[sim] would ", stdout);
    vprintf(fmt, ap);
    fputc('\n', stdout);
    va_end(ap);
}

static const char *RUN_KEY = "HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Run";
static const char *DROP_PATH = "C:\\Users\\Public\\upd\\updsvc.exe";
static const char *TASK_NAME = "GoogleUpdateTaskMachineQC";
static const char *INJECT_TARGET = "explorer.exe";
static const char *C2_HOST = "updates.contoso-cdn.test";
static const char *C2_PATH = "/v2/check?id=";

static void sim_discover(void)
{
    say("read the computer name, the user name and the OS version");
    say("list the running processes");
}

static void sim_persist_runkey(void)
{
    say("open the key %s", RUN_KEY);
    say("set the value \"Updater\" to \"%s\"", DROP_PATH);
}

static void sim_persist_task(void)
{
    say("run: schtasks /create /tn \"%s\" /tr \"%s\" /sc daily /st 09:00", TASK_NAME, DROP_PATH);
}

static void sim_inject(void)
{
    say("open the process %s with write access", INJECT_TARGET);
    say("allocate 4096 bytes of executable memory in it");
    say("write the payload into that memory");
    say("start a remote thread at the payload");
}

static void sim_token(void)
{
    say("open the access token of a system process");
    say("duplicate the token as a primary token");
    say("start %s with the duplicated token", DROP_PATH);
}

static void sim_beacon(void)
{
    say("send an HTTP GET to http://%s:8443%s<host id> every 60 seconds", C2_HOST, C2_PATH);
    say("write the reply to %s", DROP_PATH);
}

int main(void)
{
    puts("[sim] simulator: no action is performed, the steps below are only printed");
    sim_discover();
    sim_persist_runkey();
    sim_persist_task();
    sim_inject();
    sim_token();
    sim_beacon();
    return 0;
}
