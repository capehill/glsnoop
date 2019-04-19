#include "warp3dnova_module.h"
#include "common.h"

#include <proto/exec.h>
#include <proto/warp3dnova.h>
#include <proto/dos.h>

#include <stdio.h>
#include <string.h>

struct Library* Warp3DNovaBase;
struct Interface* IWarp3DNova;

struct NovaContext {
    struct Task* task;
    struct W3DN_Context_s* context;
    char name[NAME_LEN];

    // Store original function pointers so that they can be still called

    void (*old_Destroy)(struct W3DN_Context_s *self);

    W3DN_VertexBuffer* (*old_CreateVertexBufferObject)(struct W3DN_Context_s *self,
    		W3DN_ErrorCode *errCode, uint64 size, W3DN_BufferUsage usage, uint32 maxArrays, struct TagItem *tags);

    void (*old_DestroyVertexBufferObject)(struct W3DN_Context_s *self, W3DN_VertexBuffer *vertexBuffer);

    uint64 (*old_VBOGetAttr)(struct W3DN_Context_s *self, W3DN_VertexBuffer *vertexBuffer, W3DN_BufferAttribute attr);

    W3DN_ErrorCode (*old_VBOSetArray)(struct W3DN_Context_s *self, W3DN_VertexBuffer *buffer,
    		uint32 arrayIdx, W3DN_ElementFormat elementType, BOOL normalized, uint64 numElements,
    		uint64 stride, uint64 offset, uint64 count);

    W3DN_ErrorCode (*old_VBOGetArray)(struct W3DN_Context_s *self, W3DN_VertexBuffer *buffer,
    		uint32 arrayIdx, W3DN_ElementFormat *elementType, BOOL *normalized,
    		uint64 *numElements, uint64 *stride, uint64 *offset, uint64 *count);

    W3DN_BufferLock* (*old_VBOLock)(struct W3DN_Context_s *self, W3DN_ErrorCode *errCode,
    		W3DN_VertexBuffer *buffer, uint64 readOffset, uint64 readSize);

    W3DN_ErrorCode (*old_BufferUnlock)(struct W3DN_Context_s *self,
    		W3DN_BufferLock *bufferLock, uint64 writeOffset, uint64 writeSize);

    W3DN_ErrorCode (*old_BindVertexAttribArray)(struct W3DN_Context_s *self,
    		W3DN_RenderState *renderState, uint32 attribNum,
    		W3DN_VertexBuffer *buffer, uint32 arrayIdx);

    W3DN_ErrorCode (*old_DrawArrays)(struct W3DN_Context_s *self,
    		W3DN_RenderState *renderState, W3DN_Primitive primitive, uint32 base, uint32 count);

    W3DN_ErrorCode (*old_DrawElements)(struct W3DN_Context_s *self,
    		W3DN_RenderState *renderState, W3DN_Primitive primitive, uint32 baseVertex, uint32 count,
    		W3DN_VertexBuffer *indexBuffer, uint32 arrayIdx);
};

static struct NovaContext* contexts[MAX_CLIENTS];
static APTR mutex;

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

static void find_process_name(struct NovaContext * context)
{
    struct Node * node = (struct Node *)context->task;

    if (node->ln_Type == NT_PROCESS) {
        char buffer[NAME_LEN];

        if (IDOS->GetCliProgramName(buffer, NAME_LEN) == FALSE) {
            strncpy(context->name, node->ln_Name, NAME_LEN);
        } else {
            snprintf(context->name, NAME_LEN, "%s '%s'", node->ln_Name, buffer);
        }
    } else {
        strncpy(context->name, node->ln_Name, NAME_LEN);
    }
}

static struct NovaContext* find_context(struct W3DN_Context_s* context)
{
    size_t i;
    struct NovaContext* result = NULL;

    IExec->MutexObtain(mutex);

    for (i = 0; i < MAX_CLIENTS; i++) {
        if (contexts[i]->context == context) {
            result = contexts[i];
            break;
        }
    }

    IExec->MutexRelease(mutex);

    return result;
}

#define GET_CONTEXT struct NovaContext* context = find_context(self);

// Wrap traced calls

static W3DN_VertexBuffer* W3DN_CreateVertexBufferObject(struct W3DN_Context_s *self,
		W3DN_ErrorCode *errCode, uint64 size, W3DN_BufferUsage usage, uint32 maxArrays, struct TagItem *tags)
{
    GET_CONTEXT

    W3DN_VertexBuffer* result = context->old_CreateVertexBufferObject(self, errCode, size, usage, maxArrays, tags);

    logLine("%s: %s: size %u, usage %d, maxArrays %u, tags %p. Buffer address %p, errCode %d", context->name, __func__,
        (unsigned)size, usage, (unsigned)maxArrays, tags, result, *errCode);

    return result;
}

static void W3DN_DestroyVertexBufferObject(struct W3DN_Context_s *self, W3DN_VertexBuffer *vertexBuffer)
{
    GET_CONTEXT

    logLine("%s: %s: vertexBuffer %p", context->name, __func__,
        vertexBuffer);

    context->old_DestroyVertexBufferObject(self, vertexBuffer);
}

static uint64 W3DN_VBOGetAttr(struct W3DN_Context_s *self, W3DN_VertexBuffer *vertexBuffer, W3DN_BufferAttribute attr)
{
    GET_CONTEXT

    const uint64 result = context->old_VBOGetAttr(self, vertexBuffer, attr);

    logLine("%s: %s: vertexBuffer %p, attr %d. Result %u", context->name, __func__,
        vertexBuffer, attr, (unsigned)result);

    return result;
}

static W3DN_ErrorCode W3DN_VBOSetArray(struct W3DN_Context_s *self, W3DN_VertexBuffer *buffer,
		uint32 arrayIdx, W3DN_ElementFormat elementType, BOOL normalized, uint64 numElements,
		uint64 stride, uint64 offset, uint64 count)
{
    GET_CONTEXT

    const W3DN_ErrorCode result = context->old_VBOSetArray(self, buffer, arrayIdx, elementType, normalized, numElements, stride, offset, count);

    logLine("%s: %s: buffer %p, arrayIdx %u, elementType %d, normalized %d, numElements %u, stride %u, offset %u, count %u. Result %d",
        context->name, __func__,
        buffer, (unsigned)arrayIdx, elementType, normalized, (unsigned)numElements, (unsigned)stride, (unsigned)offset, (unsigned)count, result);

    return result;
}

static W3DN_ErrorCode W3DN_VBOGetArray(struct W3DN_Context_s *self, W3DN_VertexBuffer *buffer,
		uint32 arrayIdx, W3DN_ElementFormat *elementType, BOOL *normalized,
		uint64 *numElements, uint64 *stride, uint64 *offset, uint64 *count)
{
    GET_CONTEXT

    const W3DN_ErrorCode result = context->old_VBOGetArray(self, buffer, arrayIdx, elementType, normalized, numElements, stride, offset, count);

    logLine("%s: %s: buffer %p, arrayIdx %u, elementType %d, normalized %d, numElements %u, stride %u, offset %u, count %u. Result %d",
        context->name, __func__,
        buffer, (unsigned)arrayIdx, *elementType, *normalized, (unsigned)*numElements, (unsigned)*stride, (unsigned)*offset, (unsigned)*count, result);

    return result;
}

static W3DN_BufferLock* W3DN_VBOLock(struct W3DN_Context_s *self, W3DN_ErrorCode *errCode,
		W3DN_VertexBuffer *buffer, uint64 readOffset, uint64 readSize)
{
    GET_CONTEXT

    W3DN_BufferLock* result = context->old_VBOLock(self, errCode, buffer, readOffset, readSize);

    logLine("%s: %s: buffer %p, readOffset %u, readSize %u. Lock address %p, errCode %d", context->name, __func__,
        buffer, (unsigned)readOffset, (unsigned)readSize, result, *errCode);

    return result;
}

static W3DN_ErrorCode W3DN_BufferUnlock(struct W3DN_Context_s *self,
		W3DN_BufferLock *bufferLock, uint64 writeOffset, uint64 writeSize)
{
    GET_CONTEXT

    const W3DN_ErrorCode result = context->old_BufferUnlock(self, bufferLock, writeOffset, writeSize);

    logLine("%s: %s: bufferLock %p, writeOffset %u, writeSize %u. Result %d", context->name, __func__,
        bufferLock, (unsigned)writeOffset, (unsigned)writeSize, result);

    return result;
}

static W3DN_ErrorCode W3DN_BindVertexAttribArray(struct W3DN_Context_s *self,
		W3DN_RenderState *renderState, uint32 attribNum,
		W3DN_VertexBuffer *buffer, uint32 arrayIdx)
{
    GET_CONTEXT

    const W3DN_ErrorCode result = context->old_BindVertexAttribArray(self, renderState, attribNum, buffer, arrayIdx);

    logLine("%s: %s: renderState %p, attribNum %u, buffer %p, arrayIdx %u. Result %d", context->name, __func__,
        renderState, (unsigned)attribNum, buffer, (unsigned)arrayIdx, result);

    return result;
}

static W3DN_ErrorCode W3DN_DrawArrays(struct W3DN_Context_s *self,
		W3DN_RenderState *renderState, W3DN_Primitive primitive, uint32 base, uint32 count)
{
    GET_CONTEXT

    const W3DN_ErrorCode result = context->old_DrawArrays(self, renderState, primitive, base, count);

    logLine("%s: %s: renderState %p, primitive %d, base %u, count %u. Result %d", context->name, __func__,
        renderState, primitive, (unsigned)base, (unsigned)count, result);

    return result;
}

static W3DN_ErrorCode W3DN_DrawElements(struct W3DN_Context_s *self,
		W3DN_RenderState *renderState, W3DN_Primitive primitive, uint32 baseVertex, uint32 count,
		W3DN_VertexBuffer *indexBuffer, uint32 arrayIdx)
{
    GET_CONTEXT

    const W3DN_ErrorCode result = context->old_DrawElements(self, renderState, primitive, baseVertex, count, indexBuffer, arrayIdx);

    logLine("%s: %s: renderState %p, primitive %d, baseVertex %u, count %u, indexBuffer %p, arrayIdx %u. Result %d",
        context->name, __func__,
        renderState, primitive, (unsigned)baseVertex, (unsigned)count, indexBuffer, (unsigned)arrayIdx, result);

    return result;
}

static void W3DN_Destroy(struct W3DN_Context_s *self)
{
    GET_CONTEXT;

    logLine("%s: %s",
        context->name, __func__);

    context->old_Destroy(self);

    size_t i;

    IExec->MutexObtain(mutex);

    for (i = 0; i < MAX_CLIENTS; i++) {
        if (contexts[i] && contexts[i]->context == self) {
            IExec->FreeVec(contexts[i]->context);
            contexts[i]->context = NULL;
            break;
        }
    }

    IExec->MutexRelease(mutex);
}

#define GENERATE_NOVA_PATCH(function) \
static void patch_##function(BOOL patching, struct NovaContext* nova) \
{ \
    if (patching) { \
        nova->old_##function = nova->context->function; \
        nova->context->function = W3DN_##function; \
        logLine("Patched W3DN context function " #function); \
    } else { \
        nova->context->function = nova->old_##function; \
        nova->old_##function = NULL; \
    } \
}

GENERATE_NOVA_PATCH(Destroy)
GENERATE_NOVA_PATCH(CreateVertexBufferObject)
GENERATE_NOVA_PATCH(DestroyVertexBufferObject)
GENERATE_NOVA_PATCH(VBOSetArray)
GENERATE_NOVA_PATCH(VBOGetArray)
GENERATE_NOVA_PATCH(VBOGetAttr)
GENERATE_NOVA_PATCH(VBOLock)
GENERATE_NOVA_PATCH(BufferUnlock)
GENERATE_NOVA_PATCH(BindVertexAttribArray)
GENERATE_NOVA_PATCH(DrawArrays)
GENERATE_NOVA_PATCH(DrawElements)

static void (*patches[])(BOOL, struct NovaContext *) = {
    patch_Destroy,
    patch_CreateVertexBufferObject,
    patch_DestroyVertexBufferObject,
    patch_VBOSetArray,
    patch_VBOGetArray,
    patch_VBOGetAttr,
    patch_VBOLock,
    patch_BufferUnlock,
    patch_BindVertexAttribArray,
    patch_DrawArrays,
    patch_DrawElements
};

static void patch_context_functions(struct NovaContext* nova)
{
    size_t i;
    for (i = 0; i < sizeof(patches) / sizeof(patches[0]); i++) {
        patches[i](TRUE, nova);
    }
}

static void restore_context_functions(struct NovaContext* nova)
{
    // Use Forbid because functions may be in use right now
    IExec->Forbid();

    size_t i;
    for (i = 0; i < sizeof(patches) / sizeof(patches[0]); i++) {
        patches[i](FALSE, nova);
    }

    IExec->Permit();
}

struct ContextCreation {
    struct Interface* interface;
    W3DN_Context* (*old_W3DN_CreateContext)(struct Warp3DNovaIFace *Self, W3DN_ErrorCode * errCode, struct TagItem * tags);
};

static struct ContextCreation creation;

static W3DN_Context* my_W3DN_CreateContext(struct Warp3DNovaIFace *Self, W3DN_ErrorCode * errCode, struct TagItem * tags)
{
    W3DN_Context* context = NULL;

    if (creation.old_W3DN_CreateContext) {
        context = creation.old_W3DN_CreateContext(Self, errCode, tags);

        if (context) {
            struct NovaContext * nova = IExec->AllocVecTags(sizeof(struct NovaContext), AVT_ClearValue, 0, TAG_DONE);

            if (nova) {
                nova->task = IExec->FindTask(NULL);
                nova->context = context;

                find_process_name(nova);

                IExec->MutexObtain(mutex);

                size_t i;
                for (i = 0; i < MAX_CLIENTS; i++) {
                    if (contexts[i] == NULL) {
                        contexts[i] = nova;
                        break;
                    }
                }

                IExec->MutexRelease(mutex);

                if (i == MAX_CLIENTS) {
                    logLine("glSnoop: too many clients, cannot patch");
                } else {
                    patch_context_functions(nova);
                }
            }
        }
    }

    return context;
}

GENERATE_PATCH(Warp3DNovaIFace, W3DN_CreateContext, my, ContextCreation)

void warp3dnova_install_patches()
{
    mutex = IExec->AllocSysObject(ASOT_MUTEX, TAG_DONE);

    if (!mutex) {
        logLine("Failed to allocate mutex");
        return;
    }

    if (open_warp3dnova_library()) {
        creation.interface = IWarp3DNova;

        patch_W3DN_CreateContext(TRUE, &creation);
    }
}

void warp3dnova_remove_patches(void)
{
    if (IWarp3DNova) {
        patch_W3DN_CreateContext(FALSE, &creation);
    }

    if (mutex) {
        size_t i;

        // Remove patches
        IExec->MutexObtain(mutex);

        for (i = 0; i < MAX_CLIENTS; i++) {
            if (contexts[i]) {
                restore_context_functions(contexts[i]);
            }
        }

        IExec->MutexRelease(mutex);
    }

    close_warp3dnova_library();
}

void warp3dnova_free(void)
{
    logLine("%s", __func__);

    if (mutex) {
        size_t i;

        // Remove context data
        IExec->MutexObtain(mutex);

        for (i = 0; i < MAX_CLIENTS; i++) {
            if (contexts[i]) {
                IExec->FreeVec(contexts[i]);
                contexts[i] = NULL;
            }
        }

        IExec->MutexRelease(mutex);

        IExec->FreeSysObject(ASOT_MUTEX, mutex);
        mutex = NULL;
    }
}
