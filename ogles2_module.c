#include "ogles2_module.h"
#include "common.h"

#include <proto/exec.h>
#include <proto/ogles2.h>

#include <stdio.h>

//struct OGLES2IFace* IOGLES2;
struct Library* OGLES2Base;

static void patch_ogles2_functions(struct Interface* interface);
static struct Interface* patchedInterface;

static BOOL open_ogles2_library(void)
{
    OGLES2Base = IExec->OpenLibrary("ogles2.library", 0);
    if (OGLES2Base) {
        //IOGLES2 = (struct OGLES2IFace *)IExec->GetInterface(OGLES2Base, "main", 1, NULL);

        //if (IOGLES2) {
            return TRUE;
        //}

        //printf("Failed to get ogles2 interface\n");
    } else {
        printf("Failed to open ogles2.library\n");
    }

    return FALSE;
}

static void close_ogles2_library(void)
{
    //if (IOGLES2) {
    //    IExec->DropInterface((struct Interface *)IOGLES2);
    //    IOGLES2 = NULL;
    //}

    if (OGLES2Base) {
        IExec->CloseLibrary(OGLES2Base);
        OGLES2Base = NULL;
    }
}

// We patch IExec->GetInterface to be able to patch later IOGLES2 interface.

static struct Interface* (*old_GetInterface)(struct ExecIFace* Self, struct Library *, STRPTR, ULONG, struct TagItem *);

static struct Interface* my_GetInterface(struct ExecIFace* Self, struct Library * library, STRPTR name, ULONG version, struct TagItem *ti)
{
    struct Interface* interface = NULL;

    if (old_GetInterface) {
        interface = old_GetInterface(Self, library, name, version, ti);

        if (library == OGLES2Base) {
            // Applications wants IOGLES2, patch it
            patch_ogles2_functions(interface);
        }
    }

    return interface;
}

PATCH_INTERFACE(ExecIFace, GetInterface)

static void (*old_aglSwapBuffers)(struct OGLES2IFace *Self);
static void (*old_glCompileShader)(struct OGLES2IFace *Self, GLuint shader) = NULL;
static void (*old_glBufferData)(struct OGLES2IFace *Self, GLenum target, GLsizeiptr size, const void * data, GLenum usage);

static void my_aglSwapBuffers(struct OGLES2IFace *Self)
{
    IExec->DebugPrintF("%s: %s\n", task_name(), __func__);

    if (old_aglSwapBuffers) {
        old_aglSwapBuffers(Self);
    }
}

static void my_glCompileShader(struct OGLES2IFace *Self, GLuint shader)
{
    IExec->DebugPrintF("%s: %s: shader %u\n", task_name(), __func__, shader);

    if (old_glCompileShader) {
        old_glCompileShader(Self, shader);
    }
}

static void my_glBufferData(struct OGLES2IFace *Self, GLenum target, GLsizeiptr size, const void * data, GLenum usage)
{
    IExec->DebugPrintF("%s: %s: target %d, size %u, data %p, usage %d\n", task_name(), __func__);

    if (old_glBufferData) {
        old_glBufferData(Self, target, size, data, usage);
    }
}

PATCH_INTERFACE(OGLES2IFace, aglSwapBuffers)
PATCH_INTERFACE(OGLES2IFace, glCompileShader)
PATCH_INTERFACE(OGLES2IFace, glBufferData)

static void (*patches[])(BOOL, struct Interface*) = {
    patch_aglSwapBuffers,
    patch_glCompileShader,
    patch_glBufferData,
    NULL
};

void ogles2_install_patches(void)
{
    open_ogles2_library();

    // Actually, patches only IExec
    patch_GetInterface(TRUE, (struct Interface *)IExec);
}

static void patch_ogles2_functions(struct Interface* interface)
{
    if (interface) {
        int i;
        for (i = 0; i < sizeof(patches) / sizeof(patches[0]); i++) {
            if (patches[i]) {
                patches[i](TRUE, interface);
            }
        }

        patchedInterface = interface;
    }
}

void ogles2_remove_patches(void)
{
    if (patchedInterface) {
        int i;
        for (i = 0; i < sizeof(patches) / sizeof(patches[0]); i++) {
            if (patches[i]) {
                patches[i](FALSE, patchedInterface);
            }
        }
    }

    patch_GetInterface(FALSE, (struct Interface *)IExec);

    close_ogles2_library();
}
