#include "warp3dnova_module.h"
#include "common.h"
#include "filter.h"
#include "timer.h"
#include "profiling.h"
#include "logger.h"

#include <proto/exec.h>
#include <proto/warp3dnova.h>

#include <stdio.h>
#include <string.h>

typedef enum NovaFunction {
    Destroy,
    CreateVertexBufferObject,
    DestroyVertexBufferObject,
    VBOSetArray,
    VBOGetArray,
    VBOGetAttr,
    VBOLock,
    BufferUnlock,
    BindVertexAttribArray,
    DrawArrays,
    DrawElements,
    CreateFrameBuffer,
    DestroyFrameBuffer,
    FBBindBuffer,
    FBGetBufferBM,
    FBGetBufferTex,
    FBGetStatus,
    SetRenderTarget,
    GetRenderTarget,
    FBGetAttr,
    CompileShader,
    Clear,
    BindTexture,
    Submit,
    SetShaderPipeline,
    WaitDone,
    WaitIdle,
    // Keep last
    NovaFunctionCount
} NovaFunction;

static const char* mapNovaFunction(const NovaFunction func)
{
    #define MAP_ENUM(x) case x: return #x;

    switch (func) {
        MAP_ENUM(Destroy)
        MAP_ENUM(CreateVertexBufferObject)
        MAP_ENUM(DestroyVertexBufferObject)
        MAP_ENUM(VBOSetArray)
        MAP_ENUM(VBOGetArray)
        MAP_ENUM(VBOGetAttr)
        MAP_ENUM(VBOLock)
        MAP_ENUM(BufferUnlock)
        MAP_ENUM(BindVertexAttribArray)
        MAP_ENUM(DrawArrays)
        MAP_ENUM(DrawElements)
        MAP_ENUM(CreateFrameBuffer)
        MAP_ENUM(DestroyFrameBuffer)
        MAP_ENUM(FBBindBuffer)
        MAP_ENUM(FBGetBufferBM)
        MAP_ENUM(FBGetBufferTex)
        MAP_ENUM(FBGetStatus)
        MAP_ENUM(SetRenderTarget)
        MAP_ENUM(GetRenderTarget)
        MAP_ENUM(FBGetAttr)
        MAP_ENUM(CompileShader)
        MAP_ENUM(Clear)
        MAP_ENUM(BindTexture)
        MAP_ENUM(Submit)
        MAP_ENUM(SetShaderPipeline)
        MAP_ENUM(WaitDone)
        MAP_ENUM(WaitIdle)
        case NovaFunctionCount: break;
    }

    #undef MAP_ENUM

    return "Unknown";
}

struct Library* Warp3DNovaBase;
struct Interface* IWarp3DNova;

static unsigned errorCount;
static BOOL profilingStarted = TRUE;

static const char* mapNovaError(const W3DN_ErrorCode code)
{
    #define MAP_ENUM(x) case x: return #x;

    switch (code) {
        MAP_ENUM(W3DNEC_SUCCESS)
        MAP_ENUM(W3DNEC_ILLEGALINPUT)
        MAP_ENUM(W3DNEC_UNSUPPORTED)
        MAP_ENUM(W3DNEC_NOMEMORY)
        MAP_ENUM(W3DNEC_NOVRAM)
        MAP_ENUM(W3DNEC_NODRIVER)
        MAP_ENUM(W3DNEC_ILLEGALBITMAP)
        MAP_ENUM(W3DNEC_NOBITMAP)
        MAP_ENUM(W3DNEC_NOTEXTURE)
        MAP_ENUM(W3DNEC_UNSUPPORTEDFMT)
        MAP_ENUM(W3DNEC_NOZBUFFER)
        MAP_ENUM(W3DNEC_NOSTENCILBUFFER)
        MAP_ENUM(W3DNEC_UNKNOWNERROR)
        MAP_ENUM(W3DNEC_INCOMPLETEFRAMEBUFFER)
        MAP_ENUM(W3DNEC_TIMEOUT)
        MAP_ENUM(W3DNEC_QUEUEEMPTY)
        MAP_ENUM(W3DNEC_MISSINGVERTEXARRAYS)
        MAP_ENUM(W3DNEC_FILENOTFOUND)
        MAP_ENUM(W3DNEC_SHADERSINCOMPATIBLE)
        MAP_ENUM(W3DNEC_IOERROR)
        MAP_ENUM(W3DNEC_CORRUPTSHADER)
        MAP_ENUM(W3DNEC_INCOMPLETESHADERPIPELINE)
        MAP_ENUM(W3DNEC_NOSHADERPIPELINE)
        MAP_ENUM(W3DNEC_SHADERERRORS)
        MAP_ENUM(W3DNEC_MISSINGSHADERDATABUFFERS)
        MAP_ENUM(W3DNEC_DEPTHSTENCILPRIVATE)
        MAP_ENUM(W3DNEC_NOTFOUND)
        MAP_ENUM(W3DNEC_EXCEEDEDMAXVARYING)
        MAP_ENUM(W3DNEC_EXCEEDEDMAXTEXUNITS)
        MAP_ENUM(W3DNEC_EXCEEDEDMAXDIM)
        MAP_ENUM(W3DNEC_MAXERROR)
    }

    #undef MAP_ENUM

    return "Unknown error";
}

static const char* mapNovaErrorPointerToString(const W3DN_ErrorCode* const pointer)
{
    if (pointer) {
        return mapNovaError(*pointer);
    }

    return "ignored (NULL pointer)";
}

static W3DN_ErrorCode mapNovaErrorPointerToCode(const W3DN_ErrorCode* const pointer)
{
    if (pointer) {
        return *pointer;
    }

    return W3DNEC_SUCCESS;
}

struct NovaContext {
    struct Task* task;
    struct W3DN_Context_s* context;
    char name[NAME_LEN];

    MyClock start;
    uint64 ticks;
    ProfilingItem profiling[NovaFunctionCount];

    PrimitiveCounter counter;

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

    W3DN_FrameBuffer* (*old_CreateFrameBuffer)(struct W3DN_Context_s *self, W3DN_ErrorCode *errCode);

    void (*old_DestroyFrameBuffer)(struct W3DN_Context_s *self, W3DN_FrameBuffer *frameBuffer);

    W3DN_ErrorCode (*old_FBBindBuffer)(struct W3DN_Context_s *self,
    	W3DN_FrameBuffer *frameBuffer, int32 attachmentPt, struct TagItem *tags);

    struct BitMap* (*old_FBGetBufferBM)(struct W3DN_Context_s *self,
    	W3DN_FrameBuffer *frameBuffer, uint32 attachmentPt, W3DN_ErrorCode *errCode);

    W3DN_Texture*  (*old_FBGetBufferTex)(struct W3DN_Context_s *self,
    	W3DN_FrameBuffer *frameBuffer, uint32 attachmentPt, W3DN_ErrorCode *errCode);

    W3DN_ErrorCode (*old_FBGetStatus)(struct W3DN_Context_s *self, W3DN_FrameBuffer *frameBuffer);

    W3DN_ErrorCode (*old_SetRenderTarget)(struct W3DN_Context_s *self,
    	W3DN_RenderState *renderState, W3DN_FrameBuffer *frameBuffer);

    W3DN_FrameBuffer* (*old_GetRenderTarget)(
        struct W3DN_Context_s *self, W3DN_RenderState *renderState);

    uint64 (*old_FBGetAttr)(struct W3DN_Context_s *self,
    	W3DN_FrameBuffer *frameBuffer, W3DN_FrameBufferAttribute attrib);

    W3DN_Shader* (*old_CompileShader)(struct W3DN_Context_s *self,
        W3DN_ErrorCode *errCode, struct TagItem *tags);

    W3DN_ErrorCode (*old_Clear)(struct W3DN_Context_s *self, W3DN_RenderState *renderState,
        const float *colour, const double *depth, const uint32* stencil);

    W3DN_ErrorCode (*old_BindTexture)(struct W3DN_Context_s *self, W3DN_RenderState *renderState,
        uint32 texUnit, W3DN_Texture *texture, W3DN_TextureSampler *texSampler);

    uint32 (*old_Submit)(struct W3DN_Context_s *self, W3DN_ErrorCode *errCode);

    W3DN_ErrorCode (*old_SetShaderPipeline)(struct W3DN_Context_s *self, W3DN_RenderState *renderState,
        W3DN_ShaderPipeline *shaderPipeline);

    W3DN_ErrorCode (*old_WaitDone)(struct W3DN_Context_s *self, uint32 submitID, uint32 timeout);

    W3DN_ErrorCode (*old_WaitIdle)(struct W3DN_Context_s *self, uint32 timeout);
};

static struct NovaContext* contexts[MAX_CLIENTS];
static APTR mutex;

static void profileResults(struct NovaContext* const context)
{
    if (!profilingStarted) {
        logAlways("Warp3D Nova profiling not started, skip summary");
        return;
    }

    PROF_FINISH_CONTEXT

    const double drawcalls = context->profiling[DrawElements].callCount + context->profiling[DrawArrays].callCount;

    // Copy items, otherwise sorthing will ruin the further profiling
    ProfilingItem stats[NovaFunctionCount];
    memcpy(stats, context->profiling, NovaFunctionCount * sizeof(ProfilingItem));

    sort(stats, NovaFunctionCount);

    logAlways("\nWarp3D Nova profiling results for %s:", context->name);

    PROF_PRINT_TOTAL

    logAlways("  Draw calls/s %.1f", drawcalls / seconds);

    logAlways("%30s | %10s | %10s | %20s | %20s | %24s | %20s",
        "function", "call count", "errors", "duration (ms)", "avg. call dur. (us)", timeUsedBuffer, "% of CPU time");

    for (int i = 0; i < NovaFunctionCount; i++) {
        if (stats[i].callCount > 0) {
            logAlways("%30s | %10llu | %10llu | %20.6f | %20.3f | %24.2f | %20.2f",
                mapNovaFunction(stats[i].index),
                stats[i].callCount,
                stats[i].errors,
                timer_ticks_to_ms(stats[i].ticks),
                timer_ticks_to_us(stats[i].ticks) / stats[i].callCount,
                (double)stats[i].ticks * 100.0 / context->ticks,
                (double)stats[i].ticks * 100.0 / totalTicks);
        }
    }

    primitiveStats(&context->counter, seconds, drawcalls);
}

void warp3dnova_start_profiling(void)
{
    profilingStarted = TRUE;

    if (mutex) {
        IExec->MutexObtain(mutex);

        for (size_t c = 0; c < MAX_CLIENTS; c++) {
            if (contexts[c]) {
                // TODO: concurrency issues?
                PROF_INIT(contexts[c], NovaFunctionCount)
            }
        }

        IExec->MutexRelease(mutex);
    }
}

void warp3dnova_finish_profiling(void)
{
    if (mutex) {
        IExec->MutexObtain(mutex);

        for (size_t c = 0; c < MAX_CLIENTS; c++) {
            if (contexts[c]) {
                profileResults(contexts[c]);
            }
        }

        IExec->MutexRelease(mutex);
    }

    profilingStarted = FALSE;
}

static char versionBuffer[64] = "Warp3DNova.library version unknown";

static BOOL open_warp3dnova_library()
{
    Warp3DNovaBase = IExec->OpenLibrary("Warp3DNova.library", 0);
    if (Warp3DNovaBase) {
        snprintf(versionBuffer, sizeof(versionBuffer), "Warp3DNova.library version %u.%u", Warp3DNovaBase->lib_Version, Warp3DNovaBase->lib_Revision);
        logLine("%s", versionBuffer);

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

const char* warp3dnova_version_string(void)
{
    return versionBuffer;
}


const char* warp3dnova_errors_string(void)
{
    static char errorBuffer[32];

    snprintf(errorBuffer, sizeof(errorBuffer), "Warp3D Nova errors: %u", errorCount);
    return errorBuffer;
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
    find_process_name2((struct Node *)context->task, context->name);
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

static void checkPointer(struct NovaContext* context, const NovaFunction id, const void* ptr)
{
    if (!ptr) {
        logLine("%s: Warning: NULL pointer detected", context->name);
        context->profiling[id].errors++;
        errorCount++;
    }
}

static void checkSuccess(struct NovaContext* context, const NovaFunction id, const W3DN_ErrorCode code)
{
    if (code != W3DNEC_SUCCESS) {
        logLine("%s: Warning: unsuccessful operation detected", context->name);
        context->profiling[id].errors++;
        errorCount++;
    }
}

#define GET_CONTEXT struct NovaContext* context = find_context(self);

// Wrap traced calls

static W3DN_VertexBuffer* W3DN_CreateVertexBufferObject(struct W3DN_Context_s *self,
		W3DN_ErrorCode *errCode, uint64 size, W3DN_BufferUsage usage, uint32 maxArrays, struct TagItem *tags)
{
    GET_CONTEXT

    PROF_START

    W3DN_VertexBuffer* result = context->old_CreateVertexBufferObject(self, errCode, size, usage, maxArrays, tags);

    PROF_FINISH(CreateVertexBufferObject)

    logLine("%s: %s: size %llu, usage %d, maxArrays %u, tags %p. Buffer address %p, errCode %d (%s)", context->name, __func__,
        size, usage, (unsigned)maxArrays, tags, result, mapNovaErrorPointerToCode(errCode), mapNovaErrorPointerToString(errCode));

    checkPointer(context, CreateVertexBufferObject, result);
    checkSuccess(context, CreateVertexBufferObject, mapNovaErrorPointerToCode(errCode));

    return result;
}

static void W3DN_DestroyVertexBufferObject(struct W3DN_Context_s *self, W3DN_VertexBuffer *vertexBuffer)
{
    GET_CONTEXT

    logLine("%s: %s: vertexBuffer %p", context->name, __func__,
        vertexBuffer);

    PROF_START

    context->old_DestroyVertexBufferObject(self, vertexBuffer);

    PROF_FINISH(DestroyVertexBufferObject)
}

static uint64 W3DN_VBOGetAttr(struct W3DN_Context_s *self, W3DN_VertexBuffer *vertexBuffer, W3DN_BufferAttribute attr)
{
    GET_CONTEXT

    PROF_START

    const uint64 result = context->old_VBOGetAttr(self, vertexBuffer, attr);

    PROF_FINISH(VBOGetAttr)

    logLine("%s: %s: vertexBuffer %p, attr %d. Result %llu", context->name, __func__,
        vertexBuffer, attr, result);

    return result;
}

static W3DN_ErrorCode W3DN_VBOSetArray(struct W3DN_Context_s *self, W3DN_VertexBuffer *buffer,
		uint32 arrayIdx, W3DN_ElementFormat elementType, BOOL normalized, uint64 numElements,
		uint64 stride, uint64 offset, uint64 count)
{
    GET_CONTEXT

    PROF_START

    const W3DN_ErrorCode result = context->old_VBOSetArray(self, buffer, arrayIdx, elementType, normalized, numElements, stride, offset, count);

    PROF_FINISH(VBOSetArray)

    logLine("%s: %s: buffer %p, arrayIdx %u, elementType %d, normalized %d, numElements %llu, stride %llu, offset %llu, count %llu. Result %d (%s)",
        context->name, __func__,
        buffer, (unsigned)arrayIdx, elementType, normalized, numElements, stride, offset, count, result, mapNovaError(result));

    checkSuccess(context, VBOSetArray, result);

    return result;
}

static W3DN_ErrorCode W3DN_VBOGetArray(struct W3DN_Context_s *self, W3DN_VertexBuffer *buffer,
		uint32 arrayIdx, W3DN_ElementFormat *elementType, BOOL *normalized,
		uint64 *numElements, uint64 *stride, uint64 *offset, uint64 *count)
{
    GET_CONTEXT

    PROF_START

    const W3DN_ErrorCode result = context->old_VBOGetArray(self, buffer, arrayIdx, elementType, normalized, numElements, stride, offset, count);

    PROF_FINISH(VBOGetArray)

    logLine("%s: %s: buffer %p, arrayIdx %u, elementType %d, normalized %d, numElements %llu, stride %llu, offset %llu, count %llu. Result %d (%s)",
        context->name, __func__,
        buffer, (unsigned)arrayIdx, *elementType, *normalized, *numElements, *stride, *offset, *count, result, mapNovaError(result));

    checkSuccess(context, VBOGetArray, result);

    return result;
}

static W3DN_BufferLock* W3DN_VBOLock(struct W3DN_Context_s *self, W3DN_ErrorCode *errCode,
		W3DN_VertexBuffer *buffer, uint64 readOffset, uint64 readSize)
{
    GET_CONTEXT

    PROF_START

    W3DN_BufferLock* result = context->old_VBOLock(self, errCode, buffer, readOffset, readSize);

    PROF_FINISH(VBOLock)

    logLine("%s: %s: buffer %p, readOffset %llu, readSize %llu. Lock address %p, errCode %u (%s)", context->name, __func__,
        buffer, readOffset, readSize, result, mapNovaErrorPointerToCode(errCode), mapNovaErrorPointerToString(errCode));

    checkPointer(context, VBOLock, result);
    checkSuccess(context, VBOLock, mapNovaErrorPointerToCode(errCode));

    return result;
}

static W3DN_ErrorCode W3DN_BufferUnlock(struct W3DN_Context_s *self,
		W3DN_BufferLock *bufferLock, uint64 writeOffset, uint64 writeSize)
{
    GET_CONTEXT

    PROF_START

    const W3DN_ErrorCode result = context->old_BufferUnlock(self, bufferLock, writeOffset, writeSize);

    PROF_FINISH(BufferUnlock)

    logLine("%s: %s: bufferLock %p, writeOffset %llu, writeSize %llu. Result %d (%s)", context->name, __func__,
        bufferLock, writeOffset, writeSize, result, mapNovaError(result));

    checkSuccess(context, BufferUnlock, result);

    return result;
}

static W3DN_ErrorCode W3DN_BindVertexAttribArray(struct W3DN_Context_s *self,
		W3DN_RenderState *renderState, uint32 attribNum,
		W3DN_VertexBuffer *buffer, uint32 arrayIdx)
{
    GET_CONTEXT

    PROF_START

    const W3DN_ErrorCode result = context->old_BindVertexAttribArray(self, renderState, attribNum, buffer, arrayIdx);

    PROF_FINISH(BindVertexAttribArray)

    logLine("%s: %s: renderState %p, attribNum %u, buffer %p, arrayIdx %u. Result %d (%s)", context->name, __func__,
        renderState, (unsigned)attribNum, buffer, (unsigned)arrayIdx, result, mapNovaError(result));

    checkSuccess(context, BindVertexAttribArray, result);

    return result;
}

static void countPrimitive(PrimitiveCounter * counter, const W3DN_Primitive primitive, const uint32 count)
{
    switch (primitive) {
        case W3DN_PRIM_TRIANGLES:
            counter->triangles += count;
            break;
        case W3DN_PRIM_TRISTRIP:
            counter->triangleStrips += count;
            break;
        case W3DN_PRIM_TRIFAN:
            counter->triangleFans += count;
            break;
        case W3DN_PRIM_LINES:
            counter->lines += count;
            break;
        case W3DN_PRIM_LINESTRIP:
            counter->lineStrips += count;
            break;
        case W3DN_PRIM_LINELOOP:
            counter->lineLoops += count;
            break;
        case W3DN_PRIM_POINTS:
            counter->points += count;
            break;
        default:
            logLine("Error - unknown primitive type %d passed to Nova", primitive);
            break;
    }
}

static W3DN_ErrorCode W3DN_DrawArrays(struct W3DN_Context_s *self,
		W3DN_RenderState *renderState, W3DN_Primitive primitive, uint32 base, uint32 count)
{
    GET_CONTEXT

    PROF_START

    const W3DN_ErrorCode result = context->old_DrawArrays(self, renderState, primitive, base, count);

    PROF_FINISH(DrawArrays)

    logLine("%s: %s: renderState %p, primitive %d, base %u, count %u. Result %d (%s)", context->name, __func__,
        renderState, primitive, (unsigned)base, (unsigned)count, result, mapNovaError(result));

    countPrimitive(&context->counter, primitive, count);
    checkSuccess(context, DrawArrays, result);

    return result;
}

static W3DN_ErrorCode W3DN_DrawElements(struct W3DN_Context_s *self,
		W3DN_RenderState *renderState, W3DN_Primitive primitive, uint32 baseVertex, uint32 count,
		W3DN_VertexBuffer *indexBuffer, uint32 arrayIdx)
{
    GET_CONTEXT

    PROF_START

    const W3DN_ErrorCode result = context->old_DrawElements(self, renderState, primitive, baseVertex, count, indexBuffer, arrayIdx);

    PROF_FINISH(DrawElements)

    logLine("%s: %s: renderState %p, primitive %d, baseVertex %u, count %u, indexBuffer %p, arrayIdx %u. Result %d (%s)",
        context->name, __func__,
        renderState, primitive, (unsigned)baseVertex, (unsigned)count, indexBuffer, (unsigned)arrayIdx, result, mapNovaError(result));

    countPrimitive(&context->counter, primitive, count);
    checkSuccess(context, DrawElements, result);

    return result;
}

static void W3DN_Destroy(struct W3DN_Context_s *self)
{
    GET_CONTEXT

    logLine("%s: %s",
        context->name, __func__);

    PROF_START

    context->old_Destroy(self);

    PROF_FINISH(Destroy)

    size_t i;

    IExec->MutexObtain(mutex);

    for (i = 0; i < MAX_CLIENTS; i++) {
        if (contexts[i] && contexts[i]->context == self) {
            profileResults(contexts[i]);

            logLine("%s: freeing patched Nova context %p", contexts[i]->name, self);

            IExec->FreeVec(contexts[i]);
            contexts[i] = NULL;
            break;
        }
    }

    IExec->MutexRelease(mutex);
}

static W3DN_FrameBuffer* W3DN_CreateFrameBuffer(struct W3DN_Context_s *self, W3DN_ErrorCode *errCode)
{
    GET_CONTEXT

    PROF_START

    W3DN_FrameBuffer* buffer = context->old_CreateFrameBuffer(self, errCode);

    PROF_FINISH(CreateFrameBuffer)

    logLine("%s: %s: Frame buffer address %p. Result %d (%s)",
        context->name, __func__,
        buffer, mapNovaErrorPointerToCode(errCode), mapNovaErrorPointerToString(errCode));

    checkPointer(context, CreateFrameBuffer, buffer);
    checkSuccess(context, CreateFrameBuffer, mapNovaErrorPointerToCode(errCode));

    return buffer;
}

static void W3DN_DestroyFrameBuffer(struct W3DN_Context_s *self, W3DN_FrameBuffer *frameBuffer)
{
    GET_CONTEXT

    logLine("%s: %s: frameBuffer %p",
        context->name, __func__,
        frameBuffer);

    PROF_START

    context->old_DestroyFrameBuffer(self, frameBuffer);

    PROF_FINISH(DestroyFrameBuffer);
}

static W3DN_ErrorCode W3DN_FBBindBuffer(struct W3DN_Context_s *self,
	W3DN_FrameBuffer *frameBuffer, int32 attachmentPt, struct TagItem *tags)
{
    GET_CONTEXT

    PROF_START

    const W3DN_ErrorCode result = context->old_FBBindBuffer(self, frameBuffer, attachmentPt, tags);

    PROF_FINISH(FBBindBuffer)

    logLine("%s: %s: frameBuffer %p, attachmentPt %d, tags %p. Result %d (%s)",
        context->name, __func__,
        frameBuffer, (int)attachmentPt, tags, result, mapNovaError(result));

    checkSuccess(context, FBBindBuffer, result);

    return result;
}

static struct BitMap* W3DN_FBGetBufferBM(struct W3DN_Context_s *self,
	W3DN_FrameBuffer *frameBuffer, uint32 attachmentPt, W3DN_ErrorCode *errCode)
{
    GET_CONTEXT

    PROF_START

    struct BitMap* bitmap = context->old_FBGetBufferBM(self, frameBuffer, attachmentPt, errCode);

    PROF_FINISH(FBGetBufferBM)

    logLine("%s: %s: frameBuffer %p, attachmentPt %u. Bitmap address %p. Result %d (%s)",
        context->name, __func__,
        frameBuffer, (unsigned)attachmentPt, bitmap, mapNovaErrorPointerToCode(errCode), mapNovaErrorPointerToString(errCode));

    checkPointer(context, FBGetBufferBM, bitmap);
    checkSuccess(context, FBGetBufferBM, mapNovaErrorPointerToCode(errCode));

    return bitmap;
}

static W3DN_Texture*  W3DN_FBGetBufferTex(struct W3DN_Context_s *self,
	W3DN_FrameBuffer *frameBuffer, uint32 attachmentPt, W3DN_ErrorCode *errCode)
{
    GET_CONTEXT

    PROF_START

    W3DN_Texture * texture = context->old_FBGetBufferTex(self, frameBuffer, attachmentPt, errCode);

    PROF_FINISH(FBGetBufferTex)

    logLine("%s: %s: frameBuffer %p, attachmentPt %u. Texture address %p. Result %d (%s)",
        context->name, __func__,
        frameBuffer, (unsigned)attachmentPt, texture, mapNovaErrorPointerToCode(errCode), mapNovaErrorPointerToString(errCode));

    checkPointer(context, FBGetBufferTex, texture);
    checkSuccess(context, FBGetBufferTex, mapNovaErrorPointerToCode(errCode));

    return texture;
}

static W3DN_ErrorCode W3DN_FBGetStatus(struct W3DN_Context_s *self, W3DN_FrameBuffer *frameBuffer)
{
    GET_CONTEXT

    PROF_START

    const W3DN_ErrorCode result = context->old_FBGetStatus(self, frameBuffer);

    PROF_FINISH(FBGetStatus)

    logLine("%s: %s: frameBuffer %p. Result %d (%s)",
        context->name, __func__,
        frameBuffer, result, mapNovaError(result));

    checkSuccess(context, FBGetStatus, result);

    return result;
}

static W3DN_ErrorCode W3DN_SetRenderTarget(struct W3DN_Context_s *self,
	W3DN_RenderState *renderState, W3DN_FrameBuffer *frameBuffer)
{
    GET_CONTEXT

    PROF_START

    const W3DN_ErrorCode result = context->old_SetRenderTarget(self, renderState, frameBuffer);

    PROF_FINISH(SetRenderTarget)

    logLine("%s: %s: renderState %p, frameBuffer %p. Result %d (%s)",
        context->name, __func__,
        renderState, frameBuffer, result, mapNovaError(result));

    checkSuccess(context, SetRenderTarget, result);

    return result;
}

static W3DN_FrameBuffer* W3DN_GetRenderTarget(
    struct W3DN_Context_s *self, W3DN_RenderState *renderState)
{
    GET_CONTEXT

    PROF_START

    W3DN_FrameBuffer* buffer = context->old_GetRenderTarget(self, renderState);

    PROF_FINISH(GetRenderTarget)

    logLine("%s: %s: renderState %p. Frame buffer address %p",
        context->name, __func__,
        renderState, buffer);

    checkPointer(context, GetRenderTarget, buffer);

    return buffer;
}

static uint64 W3DN_FBGetAttr(struct W3DN_Context_s *self,
	W3DN_FrameBuffer *frameBuffer, W3DN_FrameBufferAttribute attrib)
{
    GET_CONTEXT

    PROF_START

    const uint64 result = context->old_FBGetAttr(self, frameBuffer, attrib);

    PROF_FINISH(FBGetAttr)

    logLine("%s: %s: frameBuffer %p, attrib %d. Result %llu",
        context->name, __func__,
        frameBuffer, attrib, result);

    return result;
}

static W3DN_Shader* W3DN_CompileShader(struct W3DN_Context_s *self,
    W3DN_ErrorCode *errCode, struct TagItem *tags)
{
    GET_CONTEXT

    PROF_START

    W3DN_Shader *shader = context->old_CompileShader(self, errCode, tags);

    PROF_FINISH(CompileShader)

    logLine("%s: %s: errCode %d (%s), tags %p. Shader address %p",
        context->name, __func__,
        mapNovaErrorPointerToCode(errCode),
        mapNovaErrorPointerToString(errCode),
        tags,
        shader);

    checkPointer(context, CompileShader, shader);
    checkSuccess(context, CompileShader, mapNovaErrorPointerToCode(errCode));

    return shader;
}

static W3DN_ErrorCode W3DN_Clear(struct W3DN_Context_s *self, W3DN_RenderState *renderState,
    const float *colour, const double *depth, const uint32* stencil)
{
    GET_CONTEXT

    PROF_START

    const W3DN_ErrorCode result = context->old_Clear(self, renderState, colour, depth, stencil);

    PROF_FINISH(Clear)

    logLine("%s: %s: renderState %p, colour %p, depth %p, stencil %p. Result %d (%s)",
        context->name, __func__,
        renderState, colour, depth, stencil, result, mapNovaError(result));

    checkSuccess(context, Clear, result);

    return result;
}

static W3DN_ErrorCode W3DN_BindTexture(struct W3DN_Context_s *self, W3DN_RenderState *renderState,
    uint32 texUnit, W3DN_Texture *texture, W3DN_TextureSampler *texSampler)
{
    GET_CONTEXT

    PROF_START

    const W3DN_ErrorCode result = context->old_BindTexture(self, renderState, texUnit, texture, texSampler);

    PROF_FINISH(BindTexture)

    logLine("%s: %s: renderState %p, texUnit %lu, texture %p, texSample %p. Result %d (%s)",
        context->name, __func__,
        renderState, texUnit, texture, texSampler, result, mapNovaError(result));

    checkSuccess(context, BindTexture, result);

    return result;
}

static uint32 W3DN_Submit(struct W3DN_Context_s *self, W3DN_ErrorCode *errCode)
{
    GET_CONTEXT

    PROF_START

    W3DN_ErrorCode myErrCode = 0;

    const uint32 result = context->old_Submit(self, &myErrCode);

    PROF_FINISH(Submit)

    logLine("%s: %s: errCode %d (%s). Submit ID %lu",
        context->name, __func__,
        myErrCode,
        mapNovaError(myErrCode),
        result);

    if (result == 0 && myErrCode != W3DNEC_QUEUEEMPTY) {
        logLine("%s: Warning: W3DN_Submit() returned zero", context->name);
        context->profiling[Submit].errors++;
        errorCount++;
    }

    if (errCode) {
        *errCode = myErrCode;
    }

    return result;
}

static W3DN_ErrorCode W3DN_SetShaderPipeline(struct W3DN_Context_s *self, W3DN_RenderState *renderState,
    W3DN_ShaderPipeline *shaderPipeline)
{
    GET_CONTEXT

    PROF_START

    const W3DN_ErrorCode result = context->old_SetShaderPipeline(self, renderState, shaderPipeline);

    PROF_FINISH(SetShaderPipeline)

    logLine("%s: %s: renderState %p, shaderPipeline %p. Result %d (%s)",
        context->name, __func__,
        renderState, shaderPipeline, result, mapNovaError(result));

    checkSuccess(context, SetShaderPipeline, result);

    return result;
}

W3DN_ErrorCode W3DN_WaitDone(struct W3DN_Context_s *self, uint32 submitID, uint32 timeout)
{
    GET_CONTEXT

    PROF_START

    const W3DN_ErrorCode result = context->old_WaitDone(self, submitID, timeout);

    PROF_FINISH(WaitDone)

    logLine("%s: %s: submitID %lu, timeout %lu. Result %d (%s)",
        context->name, __func__,
        submitID, timeout, result, mapNovaError(result));

    checkSuccess(context, WaitDone, result);

    return result;
}

W3DN_ErrorCode W3DN_WaitIdle(struct W3DN_Context_s *self, uint32 timeout)
{
    GET_CONTEXT

    PROF_START

    const W3DN_ErrorCode result = context->old_WaitIdle(self, timeout);

    PROF_FINISH(WaitIdle)

    logLine("%s: %s: timeout %lu. Result %d (%s)",
        context->name, __func__,
        timeout, result, mapNovaError(result));

    checkSuccess(context, WaitIdle, result);

    return result;
}


#define GENERATE_NOVA_PATCH(function) \
static void patch_##function(BOOL patching, struct NovaContext* nova) \
{ \
    if (patching) { \
        if (match("W3DN_" #function)) { \
            nova->old_##function = nova->context->function; \
            nova->context->function = W3DN_##function; \
            logLine("Patched W3DN context function " #function); \
        } \
    } else { \
        if (nova->old_##function) { \
            nova->context->function = nova->old_##function; \
            nova->old_##function = NULL; \
        } \
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
GENERATE_NOVA_PATCH(CreateFrameBuffer)
GENERATE_NOVA_PATCH(DestroyFrameBuffer)
GENERATE_NOVA_PATCH(FBBindBuffer)
GENERATE_NOVA_PATCH(FBGetBufferBM)
GENERATE_NOVA_PATCH(FBGetBufferTex)
GENERATE_NOVA_PATCH(FBGetStatus)
GENERATE_NOVA_PATCH(SetRenderTarget)
GENERATE_NOVA_PATCH(GetRenderTarget)
GENERATE_NOVA_PATCH(FBGetAttr)
GENERATE_NOVA_PATCH(CompileShader)
GENERATE_NOVA_PATCH(Clear)
GENERATE_NOVA_PATCH(BindTexture)
GENERATE_NOVA_PATCH(Submit)
GENERATE_NOVA_PATCH(SetShaderPipeline)
GENERATE_NOVA_PATCH(WaitDone)
GENERATE_NOVA_PATCH(WaitIdle)

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
    patch_DrawElements,
    patch_CreateFrameBuffer,
    patch_DestroyFrameBuffer,
    patch_FBBindBuffer,
    patch_FBGetBufferBM,
    patch_FBGetBufferTex,
    patch_FBGetStatus,
    patch_SetRenderTarget,
    patch_GetRenderTarget,
    patch_FBGetAttr,
    patch_CompileShader,
    patch_Clear,
    patch_BindTexture,
    patch_Submit,
    patch_SetShaderPipeline,
    patch_WaitDone,
    patch_WaitIdle
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
                    IExec->FreeVec(nova);
                } else {
                    patch_context_functions(nova);
                    PROF_INIT(nova, NovaFunctionCount)
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
                profileResults(contexts[i]);
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
