#include "ogles2_module.h"
#include "warp3dnova_module.h"
#include "common.h"

#include <proto/exec.h>
#include <proto/dos.h>

#include <stdio.h>

struct Params {
    LONG ogles2;
    LONG nova;
};

static struct Params params = { 0 };

static void parse_args()
{
    struct RDArgs *result = IDOS->ReadArgs("OGLES2/S,NOVA/S", (int32 *)&params, NULL);

    if (result) {
        IDOS->FreeArgs(result);
    }

    if (!params.ogles2 && !params.nova) {
        // If user gives nothing, assume everything
        params.ogles2 = params.nova = TRUE;
    }

    printf("OGLES2 tracing: [%s]\n", params.ogles2 ? "enabled" : "disabled");
    printf("WARP3DNOVA tracing: [%s]\n", params.nova ? "enabled" : "disabled");
}

static void install_patches()
{
    if (params.ogles2) {
        ogles2_install_patches();
    }

    if (params.nova) {
        warp3dnova_install_patches();
    }
}

static void remove_patches()
{
    if (params.nova) {
        warp3dnova_remove_patches();
    }

    if (params.ogles2) {
        ogles2_remove_patches();
    }
}

int main(int argc, char* argv[])
{
    logLine("glSnoop starting");

    parse_args();
    install_patches();

    puts("System patched. Press Control-C to quit...");

    IExec->Wait(SIGBREAKF_CTRL_C);

    remove_patches();

    logLine("glSnoop terminated");

    return 0;
}
