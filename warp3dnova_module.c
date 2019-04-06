#include "warp3dnova_module.h"

#include <proto/exec.h>
#include <proto/warp3dnova.h>

#include <stdio.h>

struct Library* Warp3DNovaBase;
struct Interface* IWarp3DNova;

static BOOL open_warp3dnova_library()
{
    Warp3DNovaBase = IExec->OpenLibrary("Warp3DNova.library", 0);
    if (Warp3DNovaBase) {
        IWarp3DNova = IExec->GetInterface(Warp3DNovaBase, "main", 1, NULL);

        if (IWarp3DNova) {
            return TRUE;
        }

        printf("Couldn't get Warp3DNova interface\n");

    } else {
        printf("Couldn't open Warp3DNova.library\n");
    }

    return FALSE;
}

static void close_warp3dnova_library()
{
    if (IWarp3DNova) {
        IExec->DropInterface(IWarp3DNova);
        IWarp3DNova = NULL;
    }

    if (Warp3DNovaBase) {
        IExec->CloseLibrary(Warp3DNovaBase);
        Warp3DNovaBase = NULL;
    }
}

static STRPTR task_name()
{
    return (((struct Node *)IExec->FindTask(NULL))->ln_Name);
}

static W3DN_ErrorCode (*old_DrawArrays)(struct W3DN_Context_s *self,
		W3DN_RenderState *renderState, W3DN_Primitive primitive, uint32 base, uint32 count);

static W3DN_ErrorCode (*old_DrawElements)(struct W3DN_Context_s *self,
		W3DN_RenderState *renderState, W3DN_Primitive primitive, uint32 baseVertex, uint32 count,
		W3DN_VertexBuffer *indexBuffer, uint32 arrayIdx);


static W3DN_ErrorCode my_DrawArrays(struct W3DN_Context_s *self,
		W3DN_RenderState *renderState, W3DN_Primitive primitive, uint32 base, uint32 count)
{
    IExec->DebugPrintF("%s: %s: renderState %p, primitive %d, base %u, count %u\n",
        task_name(), __func__, renderState, primitive, base, count);

    return old_DrawArrays(self, renderState, primitive, base, count);
}

static W3DN_ErrorCode my_DrawElements(struct W3DN_Context_s *self,
		W3DN_RenderState *renderState, W3DN_Primitive primitive, uint32 baseVertex, uint32 count,
		W3DN_VertexBuffer *indexBuffer, uint32 arrayIdx)
{
    IExec->DebugPrintF("%s: %s: renderState %p, primitive %d, baseVertex %u, count %u, indexBuffer %p, arrayIdx %u\n",
        task_name(), __func__,
        renderState, primitive, baseVertex, count, indexBuffer, arrayIdx);

    return old_DrawElements(self, renderState, primitive, baseVertex, count, indexBuffer, arrayIdx);
}

#define PATCH_W3DN_CONTEXT(function) \
old_##function = context->function; \
context->function = my_##function; \
IExec->DebugPrintF("Patched W3DN context function " #function "\n"); \


static void patch_context_functions(W3DN_Context* context)
{
    PATCH_W3DN_CONTEXT(DrawArrays)
    PATCH_W3DN_CONTEXT(DrawElements)
}

static W3DN_Context* (*old_W3DN_CreateContext)(struct Warp3DNovaIFace *Self, W3DN_ErrorCode * errCode, struct TagItem * tags);

static W3DN_Context* my_W3DN_CreateContext(struct Warp3DNovaIFace *Self, W3DN_ErrorCode * errCode, struct TagItem * tags)
{
    W3DN_Context* context = NULL;

    if (old_W3DN_CreateContext) {
        context = old_W3DN_CreateContext(Self, errCode, tags);

        patch_context_functions(context);
    }

    return context;
}

static void patch_W3DN_CreateContext(BOOL patching)
{
    IExec->Forbid();

    if (patching) {
        old_W3DN_CreateContext = IExec->SetMethod(IWarp3DNova, offsetof(struct Warp3DNovaIFace, W3DN_CreateContext), my_W3DN_CreateContext);
        if (old_W3DN_CreateContext) {
            IExec->DebugPrintF("%s: Patched W3DN_CreateContext %p with %p\n", task_name(), old_W3DN_CreateContext, my_W3DN_CreateContext);
        }
    } else {
        if (old_W3DN_CreateContext) {
            IExec->SetMethod(IWarp3DNova, offsetof(struct Warp3DNovaIFace, W3DN_CreateContext), old_W3DN_CreateContext);
            IExec->DebugPrintF("%s: Restored W3DN_CreateContext %p\n", task_name(), old_W3DN_CreateContext);
            old_W3DN_CreateContext = NULL;
        }
    }

    IExec->Permit();
}

void warp3dnova_install_patches()
{
    open_warp3dnova_library();

    patch_W3DN_CreateContext(TRUE);
}

void warp3dnova_remove_patches()
{
    patch_W3DN_CreateContext(FALSE);

    close_warp3dnova_library();
}
