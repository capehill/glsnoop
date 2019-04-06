#include "ogles2_module.h"
#include "warp3dnova_module.h"

#include <proto/exec.h>
#include <dos/dos.h>

#include <stdio.h>

int main(int argc, char* argv[]) {

    IExec->DebugPrintF("glSnoop starting\n");

    ogles2_install_patches();
    warp3dnova_install_patches();

    //IExec->DebugPrintF("Control-C to quit\n");
    printf("System patched. Press Control-C to quit...\n");

    IExec->Wait(SIGBREAKF_CTRL_C);

    warp3dnova_remove_patches();
    ogles2_remove_patches();

    IExec->DebugPrintF("glSnoop quiting\n");

    return 0;
}
