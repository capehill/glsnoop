#include "warp3dnova_module.h"
#include "common.h"

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

        if (context) {
            patch_context_functions(context);
        }
    }

    return context;
}

PATCH_INTERFACE(Warp3DNovaIFace, W3DN_CreateContext)

void warp3dnova_install_patches()
{
    if (open_warp3dnova_library()) {
        patch_W3DN_CreateContext(TRUE, IWarp3DNova);
    }
}

void warp3dnova_remove_patches()
{
    if (IWarp3DNova) {
        patch_W3DN_CreateContext(FALSE, IWarp3DNova);
    }

    close_warp3dnova_library();
}
