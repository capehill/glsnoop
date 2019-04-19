#include "ogles2_module.h"
#include "warp3dnova_module.h"
#include "common.h"
#include "gui.h"

#include <proto/exec.h>
#include <proto/dos.h>

#include <stdio.h>

struct Params {
    LONG ogles2;
    LONG nova;
    LONG gui;
};

static struct Params params = { 0 };

static void parse_args(void)
{
    struct RDArgs *result = IDOS->ReadArgs("OGLES2/S,NOVA/S,GUI/S", (int32 *)&params, NULL);

    if (result) {
        IDOS->FreeArgs(result);
    }

    if (!params.ogles2 && !params.nova) {
        // If user gives nothing, assume everything
        params.ogles2 = params.nova = TRUE;
    }

    printf("OGLES2 tracing: [%s]\n", params.ogles2 ? "enabled" : "disabled");
    printf("WARP3DNOVA tracing: [%s]\n", params.nova ? "enabled" : "disabled");
    printf("GUI: [%s]\n", params.gui ? "enabled" : "disabled");
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

int main(int argc, char* argv[])
{
    logLine("glSnoop started");

    parse_args();
    install_patches();

    puts("System patched. Press Control-C to quit...");

    if (params.gui) {
        run_gui();
    } else {
        IExec->Wait(SIGBREAKF_CTRL_C);
    }

    remove_patches();

    puts("Patches removed. glSnoop terminating");
    logLine("glSnoop exiting");

    return 0;
}
