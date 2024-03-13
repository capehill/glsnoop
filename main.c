#include "ogles2_module.h"
#include "warp3dnova_module.h"
#include "logger.h"
#include "gui.h"
#include "filter.h"
#include "timer.h"
#include "version.h"

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/icon.h>

#include <stdio.h>
#include <string.H>
#include <stdlib.h>

struct Params {
    LONG ogles2;
    LONG nova;
    LONG gui;
    LONG profiling;
    LONG *startTime;
    LONG *duration;
    char *filter;
};

static const char* const version __attribute__((used)) = "$VER: " VERSION_STRING DATE_STRING "\0";
static const char* const portName = "glSnoop port";
static char* filterFile;
static struct Params params = { 0, 0, 0, 0, NULL, NULL, NULL };

static struct MsgPort* port;

struct Task* mainTask;
BYTE mainSig = -1;

static ULONG startTime;
static ULONG duration;

static BOOL running = TRUE;

static BOOL already_running(void)
{
    IExec->Forbid();
    struct MsgPort* p = IExec->FindPort(portName);
    IExec->Permit();

    return p != NULL;
}

static void create_port()
{
    port = IExec->AllocSysObjectTags(ASOT_PORT,
        ASOPORT_Name, portName,
        ASOPORT_Pri, 1,
        TAG_DONE);

    if (!port) {
        puts("Failed to create port");
        // Not a big deal, port is only used to guard against multiple glSnoop instances
    }
}

static void remove_port()
{
    if (port) {
        IExec->FreeSysObject(ASOT_PORT, port);
        port = NULL;
    }
}

static void sanitiseParams(void)
{
    if (params.gui) {
        // Some of these limitations may be lifted in the future
        if (startTime) {
            puts("Using STARTTIME with GUI is not possible yet");
            startTime = 0;
        }

        if (duration) {
            puts("Using DURATION with GUI is not possible yet");
            duration = 0;
        }
    }

    if (!params.profiling) {
        if (startTime) {
            puts("STARTTIME is meant to be used with PROFILE");
            startTime = 0;
        }
        if (duration) {
            puts("DURATION is meant to be used with PROFILE");
            duration = 0;
        }
    }

    if (!params.ogles2 && !params.nova) {
        // If user gives nothing, assume everything
        params.ogles2 = params.nova = TRUE;
    }
}

static BOOL parse_args(void)
{
    const char* const enabled = "enabled";
    const char* const disabled = "disabled";
    const char* const pattern = "OGLES2/S,NOVA/S,GUI/S,PROFILE/S,STARTTIME/N,DURATION/N,FILTER/K";

    // how-to handle both tooltypes and args?

    struct RDArgs *result = IDOS->ReadArgs(pattern, (int32 *)&params, NULL);

    if (result) {
        if (params.filter) {
            filterFile = strdup(params.filter);
        }

        if (params.startTime) {
            startTime = (ULONG)*params.startTime;
        }

        if (params.duration) {
            duration = (ULONG)*params.duration;
        }

        IDOS->FreeArgs(result);
    } else {
        printf("Error when reading command-line arguments. Known parameters are: %s\n", pattern);
        return FALSE;
    }

    sanitiseParams();

    puts("--- Configuration ---");
    printf("  OGLES2 module: [%s]\n", params.ogles2 ? enabled : disabled);
    printf("  WARP3DNOVA module: [%s]\n", params.nova ? enabled : disabled);
    printf("  GUI: [%s]\n", params.gui ? enabled : disabled);
    printf("  Tracing mode: [%s]\n", params.profiling ? disabled : enabled);
    printf("  Filter file name: [%s]\n", filterFile ? filterFile : disabled);
    printf("  Start time: [%lu] seconds %s\n", startTime, !startTime ? "- immediate" : "");
    printf("  Duration: [%lu] seconds %s\n", duration, !duration ? "- unlimited" : "");
    puts("---------------------");

    return TRUE;
}

static void install_patches(void)
{
    if (params.ogles2) {
        ogles2_install_patches();
    }

    if (params.nova) {
        warp3dnova_install_patches(startTime, duration);
    }
}

static void patch_cooldown(void)
{
    const ULONG seconds = 1;

    logLine("Wait %lu s before quit...", seconds);

    // It's possible that some patched application is still running inside glSnoop wrappers.
    // Give it time to exit before process memory goes out.

    timer_delay(seconds);

    logLine("...waiting over");
}

static void remove_patches(void)
{
    warp3dnova_remove_patches();
    ogles2_remove_patches();

    patch_cooldown();

    // It may be useful to see the cleanup in serial
    resume_log();

    warp3dnova_free();
    ogles2_free();
}

// When STARTTIME > 0
static void waitForStartTimer()
{
    const uint32 timerSig = timer_signal(&triggerTimer);

    const ESignalType type = timer_wait_for_signal(timerSig, "Start");

    if (type == ESignalType_Timer) {
        puts("Timer signal - start profiling");

        timer_handle_events(&triggerTimer);

        ogles2_start_profiling();
        warp3dnova_start_profiling();
    } else if (type == ESignalType_Break) {
        running = FALSE;
    }
}

static void waitForEndTimer()
{
    if (running) {
        timer_start(&triggerTimer, duration, 0);

        const uint32 timerSig = timer_signal(&triggerTimer);

        puts("Waiting for timer...");

        if (timer_wait_for_signal(timerSig, "Stop") == ESignalType_Timer) {
            puts("Timer signal - stop profiling");
            timer_handle_events(&triggerTimer);
        }
    }
}

static void waitForSignal()
{
    if (running) {
        const uint32 sigMask = 1L << mainSig;
        const uint32 wait = IExec->Wait(sigMask | SIGBREAKF_CTRL_C);

        if (wait & sigMask) {
            puts("Start signal received");
        }

        if (wait & SIGBREAKF_CTRL_C) {
            puts("Break signal received");
            running = FALSE;
        }
    }
}

static void waitForTimers()
{
    if (startTime) {
        waitForStartTimer();
    }

    if (duration) {
        if (!startTime) {
            // NOVA module Signal()s us
            puts("Waiting for signal...");
            waitForSignal();
        }

        waitForEndTimer();
    } else {
        if (startTime) {
            waitForSignal();
        }
    }
}

static void run(void)
{
    if (params.gui) {
        run_gui(params.profiling);
    } else {
        if (startTime || duration) {
            waitForTimers();
        } else {
            waitForSignal();
        }
    }
}

int main(int argc __attribute__((unused)), char* argv[] __attribute__((unused)))
{
    if (!timer_init(&timer)) {
        goto out;
    }

    logLine("*** %s started. Built date: %s ***", VERSION_STRING, __DATE__);

    if (!parse_args()) {
        goto out;
    }

    if (already_running()) {
        puts("glSnoop is already running");
        goto out;
    }

    mainTask = IExec->FindTask(NULL);

    if (startTime || duration) {
        if (!timer_init(&triggerTimer)) {
             goto out;
        }
    }

    mainSig = IExec->AllocSignal(-1);
    if (mainSig == -1) {
        puts("Failed to allocate signal");
        goto out;
    }

    create_port();

    if (!load_filters(filterFile)) {
        goto out;
    }

    install_patches();

    if (params.profiling) {
        puts("Profiling mode - disabling most serial logging...");
        pause_log();
    }

    puts("System patched. Press Control-C to quit...");

    run();

    remove_patches();

    puts("Patches removed. glSnoop terminating");

out:
    if (mainSig != -1) {
        IExec->FreeSignal(mainSig);
        mainSig = -1;
    }

    remove_port();

    free_filters();
    free(filterFile);

    if (startTime || duration) {
        timer_stop(&triggerTimer);
        timer_quit(&triggerTimer);
    }

    logLine("glSnoop exiting");

    timer_quit(&timer);

    return 0;
}
