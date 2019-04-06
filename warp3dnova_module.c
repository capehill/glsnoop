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

static W3DN_VertexBuffer* (*old_CreateVertexBufferObject)(struct W3DN_Context_s *self,
		W3DN_ErrorCode *errCode, uint64 size, W3DN_BufferUsage usage, uint32 maxArrays, struct TagItem *tags);

static void (*old_DestroyVertexBufferObject)(struct W3DN_Context_s *self, W3DN_VertexBuffer *vertexBuffer);

static uint64 (*old_VBOGetAttr)(struct W3DN_Context_s *self, W3DN_VertexBuffer *vertexBuffer, W3DN_BufferAttribute attr);

static W3DN_ErrorCode (*old_VBOSetArray)(struct W3DN_Context_s *self, W3DN_VertexBuffer *buffer,
		uint32 arrayIdx, W3DN_ElementFormat elementType, BOOL normalized, uint64 numElements,
		uint64 stride, uint64 offset, uint64 count);

static W3DN_ErrorCode (*old_VBOGetArray)(struct W3DN_Context_s *self, W3DN_VertexBuffer *buffer,
		uint32 arrayIdx, W3DN_ElementFormat *elementType, BOOL *normalized,
		uint64 *numElements, uint64 *stride, uint64 *offset, uint64 *count);

static W3DN_BufferLock* (*old_VBOLock)(struct W3DN_Context_s *self, W3DN_ErrorCode *errCode,
		W3DN_VertexBuffer *buffer, uint64 readOffset, uint64 readSize);

static W3DN_ErrorCode (*old_BufferUnlock)(struct W3DN_Context_s *self,
		W3DN_BufferLock *bufferLock, uint64 writeOffset, uint64 writeSize);

static W3DN_ErrorCode (*old_BindVertexAttribArray)(struct W3DN_Context_s *self,
		W3DN_RenderState *renderState, uint32 attribNum,
		W3DN_VertexBuffer *buffer, uint32 arrayIdx);

static W3DN_ErrorCode (*old_DrawArrays)(struct W3DN_Context_s *self,
		W3DN_RenderState *renderState, W3DN_Primitive primitive, uint32 base, uint32 count);

static W3DN_ErrorCode (*old_DrawElements)(struct W3DN_Context_s *self,
		W3DN_RenderState *renderState, W3DN_Primitive primitive, uint32 baseVertex, uint32 count,
		W3DN_VertexBuffer *indexBuffer, uint32 arrayIdx);

static W3DN_VertexBuffer* my_CreateVertexBufferObject(struct W3DN_Context_s *self,
		W3DN_ErrorCode *errCode, uint64 size, W3DN_BufferUsage usage, uint32 maxArrays, struct TagItem *tags)
{
    IExec->DebugPrintF("%s: %s: errCode %p, size %u, usage %d, maxArrays %u, tags %p\n", task_name(), __func__,
        errCode, (unsigned)size, usage, maxArrays, tags);

    return old_CreateVertexBufferObject(self, errCode, size, usage, maxArrays, tags);
}

static void my_DestroyVertexBufferObject(struct W3DN_Context_s *self, W3DN_VertexBuffer *vertexBuffer)
{
    IExec->DebugPrintF("%s: %s: vertexBuffer %p\n", task_name(), __func__,
        vertexBuffer);

    old_DestroyVertexBufferObject(self, vertexBuffer);
}

static uint64 my_VBOGetAttr(struct W3DN_Context_s *self, W3DN_VertexBuffer *vertexBuffer, W3DN_BufferAttribute attr)
{
    const uint64 result = old_VBOGetAttr(self, vertexBuffer, attr);

    IExec->DebugPrintF("%s: %s: vertexBuffer %p, attr %d. Result %u\n", task_name(), __func__,
        vertexBuffer, attr, (unsigned)result);

    return result;
}

static W3DN_ErrorCode my_VBOSetArray(struct W3DN_Context_s *self, W3DN_VertexBuffer *buffer,
		uint32 arrayIdx, W3DN_ElementFormat elementType, BOOL normalized, uint64 numElements,
		uint64 stride, uint64 offset, uint64 count)
{
    IExec->DebugPrintF("%s: %s: buffer %p, arrayIdx %u, elementType %d, normalized %d, numElements %u, stride %u, offset %u, count %u\n",
        task_name(), __func__,
        buffer, arrayIdx, elementType, normalized, (unsigned)numElements, (unsigned)stride, (unsigned)offset, (unsigned)count);

    return old_VBOSetArray(self, buffer, arrayIdx, elementType, normalized, numElements, stride, offset, count);
}

static W3DN_ErrorCode my_VBOGetArray(struct W3DN_Context_s *self, W3DN_VertexBuffer *buffer,
		uint32 arrayIdx, W3DN_ElementFormat *elementType, BOOL *normalized,
		uint64 *numElements, uint64 *stride, uint64 *offset, uint64 *count)
{
    const W3DN_ErrorCode result = old_VBOGetArray(self, buffer, arrayIdx, elementType, normalized, numElements, stride, offset, count);

    IExec->DebugPrintF("%s: %s: buffer %p, arrayIdx %u, elementType %d, normalized %d, numElements %u, stride %u, offset %u, count %u. Error code %d\n",
        task_name(), __func__,
        buffer, arrayIdx, *elementType, *normalized, (unsigned)*numElements, (unsigned)*stride, (unsigned)*offset, (unsigned)*count, result);

    return result;
}

static W3DN_BufferLock* my_VBOLock(struct W3DN_Context_s *self, W3DN_ErrorCode *errCode,
		W3DN_VertexBuffer *buffer, uint64 readOffset, uint64 readSize)
{
    IExec->DebugPrintF("%s: %s: errCode %p, buffer %p, readOffset %u, readSize %u\n", task_name(), __func__,
        errCode, buffer, (unsigned)readOffset, (unsigned)readSize);

    return old_VBOLock(self, errCode, buffer, readOffset, readSize);
}

static W3DN_ErrorCode my_BufferUnlock(struct W3DN_Context_s *self,
		W3DN_BufferLock *bufferLock, uint64 writeOffset, uint64 writeSize)
{
    IExec->DebugPrintF("%s: %s: bufferLock %p, writeOffset %u, writeSize %u\n", task_name(), __func__,
        bufferLock, (unsigned)writeOffset, (unsigned)writeSize);

    return old_BufferUnlock(self, bufferLock, writeOffset, writeSize);
}

static W3DN_ErrorCode my_BindVertexAttribArray(struct W3DN_Context_s *self,
		W3DN_RenderState *renderState, uint32 attribNum,
		W3DN_VertexBuffer *buffer, uint32 arrayIdx)
{
    IExec->DebugPrintF("%s: %s: renderState %p, attribNum %u, buffer %p, arrayIdx %u\n", task_name(), __func__,
        renderState, attribNum, buffer, arrayIdx);

    return old_BindVertexAttribArray(self, renderState, attribNum, buffer, arrayIdx);
}

static W3DN_ErrorCode my_DrawArrays(struct W3DN_Context_s *self,
		W3DN_RenderState *renderState, W3DN_Primitive primitive, uint32 base, uint32 count)
{
    IExec->DebugPrintF("%s: %s: renderState %p, primitive %d, base %u, count %u\n", task_name(), __func__,
        renderState, primitive, base, count);

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
    PATCH_W3DN_CONTEXT(CreateVertexBufferObject)
    PATCH_W3DN_CONTEXT(DestroyVertexBufferObject)
    PATCH_W3DN_CONTEXT(VBOSetArray)
    PATCH_W3DN_CONTEXT(VBOGetArray)
    PATCH_W3DN_CONTEXT(VBOGetAttr)
    PATCH_W3DN_CONTEXT(VBOLock)
    PATCH_W3DN_CONTEXT(BufferUnlock)
    PATCH_W3DN_CONTEXT(BindVertexAttribArray)
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
