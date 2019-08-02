#include "ogles2_module.h"
#include "warp3dnova_module.h"
#include "logger.h"
#include "gui.h"
#include "filter.h"
#include "timer.h"
#include "version.h"

#include <proto/exec.h>
#include <proto/dos.h>

#include <stdio.h>
#include <string.H>
#include <stdlib.h>

struct Params {
    LONG ogles2;
    LONG nova;
    LONG gui;
    LONG profiling;
    char *filter;
};

static const char* const version __attribute__((used)) = "$VER: " VERSION_STRING "\0";
static const char* const portName = "glSnoop port";
static char* filterFile;
static struct Params params = { 0, 0, 0, 0, NULL };

static struct MsgPort* port;

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

static BOOL parse_args(void)
{
    const char* const enabled = "enabled";
    const char* const disabled = "disabled";
    const char* const pattern = "OGLES2/S,NOVA/S,GUI/S,PROFILE/S,FILTER/K";

    struct RDArgs *result = IDOS->ReadArgs(pattern, (int32 *)&params, NULL);

    if (result) {
        if (params.filter) {
            filterFile = strdup(params.filter);
        }

        IDOS->FreeArgs(result);
    } else {
        printf("Error when reading command-line arguments. Known parameters are: %s\n", pattern);
        return FALSE;
    }

    if (!params.ogles2 && !params.nova) {
        // If user gives nothing, assume everything
        params.ogles2 = params.nova = TRUE;
    }

    puts("--- Configuration ---");
    printf("  OGLES2 tracing: [%s]\n", params.ogles2 ? enabled : disabled);
    printf("  WARP3DNOVA tracing: [%s]\n", params.nova ? enabled : disabled);
    printf("  GUI: [%s]\n", params.gui ? enabled : disabled);
    printf("  Tracing mode: [%s]\n", params.profiling ? disabled : enabled);
    printf("  Filter file name: [%s]\n", filterFile ? filterFile : disabled);
    puts("---------------------");

    return TRUE;
}

static void install_patches(void)
{
    if (params.ogles2) {
        ogles2_install_patches();
    }

    if (params.nova) {
        warp3dnova_install_patches();
    }
}

static void patch_cooldown(void)
{
    logLine("Wait before quit");

    // It's possible that some patched application is still running inside glSnoop wrappers.
    // Give it time to exit before process memory goes out.
    IDOS->Delay(50);
}

static void remove_patches(void)
{
    // It may be useful to see the cleanup in serial
    resume_log();

    warp3dnova_remove_patches();
    ogles2_remove_patches();

    patch_cooldown();

    warp3dnova_free();
    ogles2_free();
}

static void run(void)
{
    if (params.gui) {
        run_gui(params.profiling);
    } else {
        IExec->Wait(SIGBREAKF_CTRL_C);
    }
}

int main(int argc __attribute__((unused)), char* argv[] __attribute__((unused)))
{
    logLine("*** glSnoop started. Built date: %s ***", __DATE__);

    if (!parse_args()) {
        goto out;
    }

    if (already_running()) {
        puts("glSnoop is already running");
        goto out;
    }

    if (!timer_init()) {
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
    remove_port();

    free_filters();
    free(filterFile);

    timer_quit();

    logLine("glSnoop exiting");

    return 0;
}
