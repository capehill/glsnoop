#include "ogles2_module.h"

#include <proto/exec.h>
#include <proto/ogles2.h>
//#include <proto/dos.h>

#include <stdio.h>

struct OGLES2IFace* IOGLES2; // We don't actually use IOGLES2 at the moment
struct Library* OGLES2Base;

static void patch_gl_functions(struct Interface* interface);
static struct Interface* patchedInterface;

static BOOL open_ogles2_library(void)
{
    OGLES2Base = IExec->OpenLibrary("ogles2.library", 0);
    if (OGLES2Base) {
        IOGLES2 = (struct OGLES2IFace *)IExec->GetInterface(OGLES2Base, "main", 1, NULL);

        if (IOGLES2) {
            return TRUE;
        }

        printf("Failed to get ogles2 interface\n");
    } else {
        printf("Failed to open ogles2.library\n");
    }

    return FALSE;
}

static void close_ogles2_library(void)
{
    if (IOGLES2) {
        IExec->DropInterface((struct Interface *)IOGLES2);
        IOGLES2 = NULL;
    }

    if (OGLES2Base) {
        IExec->CloseLibrary(OGLES2Base);
        OGLES2Base = NULL;
    }
}

static struct Interface* (*old_GetInterface)(struct ExecIFace* Self, struct Library *, STRPTR, ULONG, struct TagItem *);

static struct Interface* my_GetInterface(struct ExecIFace* Self, struct Library * library, STRPTR name, ULONG version, struct TagItem *ti)
{
    struct Interface* interface = NULL;

    if (old_GetInterface) {
        interface = old_GetInterface(Self, library, name, version, ti);

        if (library == OGLES2Base) {
            // Applications wants IOGLES2, patch it
            patch_gl_functions(interface);
        }
    }

    return interface;
}

static void patch_GetInterface(BOOL patching)
{
    if (patching) {
        old_GetInterface = IExec->SetMethod((struct Interface *)IExec, offsetof(struct ExecIFace, GetInterface), my_GetInterface);
        if (old_GetInterface) {
            IExec->DebugPrintF("Patched GetInterface %p with %p\n", old_GetInterface, my_GetInterface);
        }
    } else {
        if (old_GetInterface) {
            IExec->SetMethod((struct Interface *)IExec, offsetof(struct ExecIFace, GetInterface), old_GetInterface);
            IExec->DebugPrintF("Restored GetInterface %p\n", old_GetInterface);
            old_GetInterface = NULL;
        }
    }
}


static void (*old_aglSwapBuffers)(struct OGLES2IFace *Self);
static void (*old_glCompileShader)(struct OGLES2IFace *Self, GLuint shader) = NULL;
static void (*old_glBufferData)(struct OGLES2IFace *Self, GLenum target, GLsizeiptr size, const void * data, GLenum usage);

static void my_aglSwapBuffers(struct OGLES2IFace *Self)
{
    IExec->DebugPrintF("%s\n", __func__);
    if (old_aglSwapBuffers) {
        old_aglSwapBuffers(Self);
    }
}

static void my_glCompileShader(struct OGLES2IFace *Self, GLuint shader)
{
    IExec->DebugPrintF("%s: shader %u\n", __func__, shader);
    if (old_glCompileShader) {
        old_glCompileShader(Self, shader);
    }
}

static void my_glBufferData(struct OGLES2IFace *Self, GLenum target, GLsizeiptr size, const void * data, GLenum usage)
{
    IExec->DebugPrintF("%s\n", __func__);
    if (old_glBufferData) {
        old_glBufferData(Self, target, size, data, usage);
    }
}

static void patch_aglSwapBuffers(BOOL patching, struct Interface* interface)
{
    if (patching) {
        old_aglSwapBuffers = IExec->SetMethod(interface, offsetof(struct OGLES2IFace, aglSwapBuffers), my_aglSwapBuffers);
        if (old_aglSwapBuffers) {
            IExec->DebugPrintF("Patched aglSwapBuffers %p with %p\n", old_aglSwapBuffers, my_aglSwapBuffers);
        }
    } else {
        if (old_aglSwapBuffers) {
            IExec->SetMethod(interface, offsetof(struct OGLES2IFace, aglSwapBuffers), old_aglSwapBuffers);
            IExec->DebugPrintF("Restored aglSwapBuffers %p\n", old_aglSwapBuffers);
        }
    }
}

static void patch_glCompileShader(BOOL patching, struct Interface* interface)
{
    if (patching) {
        old_glCompileShader = IExec->SetMethod(interface, offsetof(struct OGLES2IFace, glCompileShader), my_glCompileShader);
        if (old_glCompileShader) {
            IExec->DebugPrintF("Patched glCompileShader %p with %p\n", old_glCompileShader, my_glCompileShader);
        }
    } else {
        if (old_glCompileShader) {
            IExec->SetMethod(interface, offsetof(struct OGLES2IFace, glCompileShader), old_glCompileShader);
            IExec->DebugPrintF("Restored glCompileShader %p\n", old_glCompileShader);
        }
    }
}

static void patch_glBufferData(BOOL patching, struct Interface* interface)
{
    if (patching) {
        old_glBufferData = IExec->SetMethod(interface, offsetof(struct OGLES2IFace, glBufferData), my_glBufferData);
        if (old_glBufferData) {
            IExec->DebugPrintF("Patched glBufferData %p with %p\n", old_glBufferData, my_glBufferData);
        }
    } else {
        if (old_glBufferData) {
            IExec->SetMethod(interface, offsetof(struct OGLES2IFace, glBufferData), old_glBufferData);
            IExec->DebugPrintF("Restored glBufferData %p\n", old_glBufferData);
        }
    }
}

static void (*patches[])(BOOL, struct Interface*) = {
    patch_aglSwapBuffers,
    patch_glCompileShader,
    patch_glBufferData,
    NULL
};

//__attribute__((constructor))
void ogles2_install_patches(void)
{
    open_ogles2_library();

    // Actually, patches only IExec
    patch_GetInterface(TRUE);
}

static void patch_gl_functions(struct Interface* interface)
{
    if (interface) {
        int i;
        IExec->Forbid();
        for (i = 0; i < sizeof(patches) / sizeof(patches[0]); i++) {
            if (patches[i]) {
                patches[i](TRUE, interface);
            }
        }
        IExec->Permit();

        patchedInterface = interface;
    }
}

//__attribute__((destructor))
void ogles2_remove_patches(void)
{
    if (patchedInterface) {
        int i;
        IExec->Forbid();
        for (i = 0; i < sizeof(patches) / sizeof(patches[0]); i++) {
            if (patches[i]) {
                patches[i](FALSE, patchedInterface);
            }
        }
        IExec->Permit();
    }

    if (old_GetInterface) {
        patch_GetInterface(FALSE);
    }

    close_ogles2_library();
}
