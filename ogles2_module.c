#include "ogles2_module.h"
#include "common.h"
#include "filter.h"
#include "timer.h"
#include "profiling.h"
#include "logger.h"

#include <proto/exec.h>
#include <proto/ogles2.h>

#include <stdio.h>
#include <stdlib.h>

struct Library* OGLES2Base;

static unsigned errorCount;

typedef enum Ogles2Function {
    SwapBuffers,
    CompileShader,
    GenBuffers,
    BindBuffer,
    BufferData,
    BufferSubData,
    DeleteBuffers,
    EnableVertexAttribArray,
    VertexAttribPointer,
    DrawArrays,
    DrawElements,
    ShaderSource,
    ActiveTexture,
    BindTexture,
    GenTextures,
    GenerateMipmap,
    TexParameterf,
    TexParameterfv,
    TexParameteri,
    TexParameteriv,
    TexSubImage2D,
    TexImage2D,
    DeleteTextures,
    GenFramebuffers,
    BindFramebuffer,
    CheckFramebufferStatus,
    FramebufferRenderbuffer,
    FramebufferTexture2D,
    GetFramebufferAttachmentParameteriv,
    DeleteFramebuffers,
    Clear,
    UseProgram,
    // Keep last
    Ogles2FunctionCount
} Ogles2Function;

static const char* mapOgles2Function(const Ogles2Function func)
{
    #define MAP_ENUM(x) case x: return #x;

    switch (func) {
        MAP_ENUM(SwapBuffers)
        MAP_ENUM(CompileShader)
        MAP_ENUM(GenBuffers)
        MAP_ENUM(BindBuffer)
        MAP_ENUM(BufferData)
        MAP_ENUM(BufferSubData)
        MAP_ENUM(DeleteBuffers)
        MAP_ENUM(EnableVertexAttribArray)
        MAP_ENUM(VertexAttribPointer)
        MAP_ENUM(DrawArrays)
        MAP_ENUM(DrawElements)
        MAP_ENUM(ShaderSource)
        MAP_ENUM(ActiveTexture)
        MAP_ENUM(BindTexture)
        MAP_ENUM(GenTextures)
        MAP_ENUM(GenerateMipmap)
        MAP_ENUM(TexParameterf)
        MAP_ENUM(TexParameterfv)
        MAP_ENUM(TexParameteri)
        MAP_ENUM(TexParameteriv)
        MAP_ENUM(TexSubImage2D)
        MAP_ENUM(TexImage2D)
        MAP_ENUM(DeleteTextures)
        MAP_ENUM(GenFramebuffers)
        MAP_ENUM(BindFramebuffer)
        MAP_ENUM(CheckFramebufferStatus)
        MAP_ENUM(FramebufferRenderbuffer)
        MAP_ENUM(FramebufferTexture2D)
        MAP_ENUM(GetFramebufferAttachmentParameteriv)
        MAP_ENUM(DeleteFramebuffers)
        MAP_ENUM(Clear)
        MAP_ENUM(UseProgram)
        case Ogles2FunctionCount: break;
    }

    #undef MAP_ENUM

    return "Unknown";
}

static const char* mapOgles2Error(const int code)
{
    #define MAP_ENUM(x) case x: ++errorCount; return #x;

    switch (code) {
        MAP_ENUM(GL_INVALID_ENUM)
        MAP_ENUM(GL_INVALID_VALUE)
        MAP_ENUM(GL_INVALID_OPERATION)
        MAP_ENUM(GL_OUT_OF_MEMORY)
        MAP_ENUM(GL_INVALID_FRAMEBUFFER_OPERATION)
    }

    #undef MAP_ENUM

    return "Unknown error";
}

// Store original function pointers so that they can be still called
struct Ogles2Context
{
    struct OGLES2IFace* interface;
    struct Task* task;
    char name[NAME_LEN];

    MyClock start;
    uint64 ticks;
    ProfilingItem profiling[Ogles2FunctionCount];

    PrimitiveCounter counter;

    void (*old_aglSwapBuffers)(struct OGLES2IFace *Self);
    void (*old_glCompileShader)(struct OGLES2IFace *Self, GLuint shader);

    void (*old_glGenBuffers)(struct OGLES2IFace *Self, GLsizei n, GLuint * buffers);
    void (*old_glBindBuffer)(struct OGLES2IFace *Self, GLenum target, GLuint buffer);
    void (*old_glBufferData)(struct OGLES2IFace *Self, GLenum target, GLsizeiptr size, const void * data, GLenum usage);
    void (*old_glBufferSubData)(struct OGLES2IFace *Self, GLenum target, GLintptr offset, GLsizeiptr size, const void * data);
    void (*old_glDeleteBuffers)(struct OGLES2IFace *Self, GLsizei n, GLuint * buffers);

    void (*old_glDrawElements)(struct OGLES2IFace *Self, GLenum mode, GLsizei count, GLenum type, const void * indices);
    void (*old_glDrawArrays)(struct OGLES2IFace *Self, GLenum mode, GLint first, GLsizei count);

    void (*old_glVertexAttribPointer)(struct OGLES2IFace *Self, GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void * pointer);
    void (*old_glEnableVertexAttribArray)(struct OGLES2IFace *Self, GLuint index);
    // void (*old_glVertexAttrib3fv)(struct OGLES2IFace *Self, GLuint index, const GLfloat * v);

    void (*old_glShaderSource)(struct OGLES2IFace *Self, GLuint shader, GLsizei count, const GLchar *const* string, const GLint * length);

    void (*old_glActiveTexture)(struct OGLES2IFace *Self, GLenum texture);
    void (*old_glBindTexture)(struct OGLES2IFace *Self, GLenum target, GLuint texture);
    void (*old_glGenTextures)(struct OGLES2IFace *Self, GLsizei n, GLuint * textures);
    void (*old_glGenerateMipmap)(struct OGLES2IFace *Self, GLenum target);
    void (*old_glTexParameterf)(struct OGLES2IFace *Self, GLenum target, GLenum pname, GLfloat param);
    void (*old_glTexParameterfv)(struct OGLES2IFace *Self, GLenum target, GLenum pname, const GLfloat * params);
    void (*old_glTexParameteri)(struct OGLES2IFace *Self, GLenum target, GLenum pname, GLint param);
    void (*old_glTexParameteriv)(struct OGLES2IFace *Self, GLenum target, GLenum pname, const GLint * params);
    void (*old_glTexSubImage2D)(struct OGLES2IFace *Self, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void * pixels);
    void (*old_glTexImage2D)(struct OGLES2IFace *Self, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void * pixels);
    void (*old_glDeleteTextures)(struct OGLES2IFace *Self, GLsizei n, const GLuint * textures);

    void (*old_glGenFramebuffers)(struct OGLES2IFace *Self, GLsizei n, GLuint * framebuffers);
    void (*old_glBindFramebuffer)(struct OGLES2IFace *Self, GLenum target, GLuint framebuffer);
    GLenum (*old_glCheckFramebufferStatus)(struct OGLES2IFace *Self, GLenum target);
    void (*old_glFramebufferRenderbuffer)(struct OGLES2IFace *Self, GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
    void (*old_glFramebufferTexture2D)(struct OGLES2IFace *Self, GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
    void (*old_glGetFramebufferAttachmentParameteriv)(struct OGLES2IFace *Self, GLenum target, GLenum attachment, GLenum pname, GLint * params);
    void (*old_glDeleteFramebuffers)(struct OGLES2IFace *Self, GLsizei n, const GLuint * framebuffers);
    void (*old_glClear)(struct OGLES2IFace *Self, GLbitfield mask);
    void (*old_glUseProgram)(struct OGLES2IFace *Self, GLuint program);
};

static struct Ogles2Context* contexts[MAX_CLIENTS];
static APTR mutex;

static void patch_ogles2_functions(struct Ogles2Context *);

static void find_process_name(struct Ogles2Context * context)
{
    find_process_name2((struct Node *)context->task, context->name);
}

static char versionBuffer[64] = "ogles2.library version unknown";
static char errorBuffer[32];

static BOOL open_ogles2_library(void)
{
    OGLES2Base = IExec->OpenLibrary("ogles2.library", 0);
    if (OGLES2Base) {
        snprintf(versionBuffer, sizeof(versionBuffer), "ogles2.library version %u.%u", OGLES2Base->lib_Version, OGLES2Base->lib_Revision);
        logLine("%s", versionBuffer);
        return TRUE;
    } else {
        printf("Failed to open ogles2.library\n");
    }

    return FALSE;
}

const char* ogles2_version_string(void)
{
    return versionBuffer;
}

const char* ogles2_errors_string(void)
{
    snprintf(errorBuffer, sizeof(errorBuffer), "OpenGL ES 2 errors: %u", errorCount);
    return errorBuffer;
}

static void close_ogles2_library(void)
{
    if (OGLES2Base) {
        IExec->CloseLibrary(OGLES2Base);
        OGLES2Base = NULL;
    }
}

static void sort(struct Ogles2Context* const context)
{
    qsort(context->profiling, Ogles2FunctionCount, sizeof(ProfilingItem), tickComparison);
}

static void profileResults(struct Ogles2Context* const context)
{
    PROF_FINISH_CONTEXT

    const double drawcalls = context->profiling[DrawElements].callCount + context->profiling[DrawArrays].callCount;
    const double swaps = context->profiling[SwapBuffers].callCount;

    sort(context);

    resume_log();

    logLine("OpenGL ES 2.0 profiling results for %s:", context->name);
    logLine("--------------------------------------------------------");

    PROF_PRINT_TOTAL

    logLine("Draw calls (glDraw*) per frame %.6f. Draw calls per second %.6f", drawcalls / swaps, drawcalls / seconds);
    logLine("Frames (buffer swaps) per second %.6f", swaps / seconds);

    logLine("%30s | %10s | %20s | %20s | %30s",
        "function", "call count", "duration (ms)", "% of combined time", "% of CPU time (incl. driver)");

    for (int i = 0; i < Ogles2FunctionCount; i++) {
        if (context->profiling[i].callCount > 0) {
            logLine("%30s | %10llu | %20.6f | %20.2f | %30.2f",
                mapOgles2Function(context->profiling[i].index),
                context->profiling[i].callCount,
                (double)context->profiling[i].ticks / timer_frequency_ms(),
                (double)context->profiling[i].ticks * 100.0 / context->ticks,
                (double)context->profiling[i].ticks * 100.0 / totalTicks);
        }
    }

    primitiveStats(&context->counter, seconds, drawcalls);

    logLine("--------------------------------------------------------");
}

// We patch IExec->GetInterface to be able to patch later IOGLES2 interface.

struct ExecContext {
    struct ExecIFace * interface;
    struct Interface* (*old_GetInterface)(struct ExecIFace* Self, struct Library *, STRPTR, ULONG, struct TagItem *);
    void (*old_DropInterface)(struct ExecIFace* Self, struct Interface* interface);
};

static struct ExecContext execContext;

static struct Interface* EXEC_GetInterface(struct ExecIFace* Self, struct Library * library, STRPTR name, ULONG version, struct TagItem *ti)
{
    struct Interface* interface = NULL;

    if (execContext.old_GetInterface) {
        interface = execContext.old_GetInterface(Self, library, name, version, ti);

        if (library == OGLES2Base) {

            struct Ogles2Context * context = IExec->AllocVecTags(sizeof(struct Ogles2Context), AVT_ClearValue, 0, TAG_DONE);

            if (context) {
                context->task = IExec->FindTask(NULL);
                context->interface = (struct OGLES2IFace *)interface;

                find_process_name(context);

                IExec->MutexObtain(mutex);

                size_t i;
                for (i = 0; i < MAX_CLIENTS; i++) {
                    if (contexts[i] == NULL) {
                        contexts[i] = context;
                        break;
                    }
                }

                IExec->MutexRelease(mutex);

                if (i == MAX_CLIENTS) {
                    logLine("glSnoop: too many clients, cannot patch");
                    IExec->FreeVec(context);
                } else {
                    patch_ogles2_functions(context);
                    PROF_INIT(context, Ogles2FunctionCount)
                }
            } else {
                logLine("Cannot allocate memory for OGLES2 context data: cannot patch");
            }
        }
    }

    return interface;
}

static void EXEC_DropInterface(struct ExecIFace* Self, struct Interface* interface)
{
    size_t i;

    IExec->MutexObtain(mutex);

    for (i = 0; i < MAX_CLIENTS; i++) {
        if (contexts[i] && (struct Interface *)contexts[i]->interface == interface) {
            profileResults(contexts[i]);

            logLine("%s: dropping patched OGLES2 interface %p", contexts[i]->name, interface);

            // No need to remove patches because every OGLES2 applications has its own interface
            IExec->FreeVec(contexts[i]);
            contexts[i] = NULL;
            break;
        }
    }

    IExec->MutexRelease(mutex);

    if (execContext.old_DropInterface) {
        execContext.old_DropInterface(Self, interface);
    }
}

GENERATE_PATCH(ExecIFace, GetInterface, EXEC, ExecContext)
GENERATE_PATCH(ExecIFace, DropInterface, EXEC, ExecContext)

static struct Ogles2Context* find_context(struct OGLES2IFace *interface)
{
    size_t i;
    struct Ogles2Context* context = NULL;

    IExec->MutexObtain(mutex);

    for (i = 0; i < MAX_CLIENTS; i++) {
        if (contexts[i]->interface == interface) {
            context = contexts[i];
            break;
        }
    }

    IExec->MutexRelease(mutex);

    return context;
}

#define GET_CONTEXT struct Ogles2Context* context = find_context(Self);

// Error checking helpers

static void check_errors(const char* info, struct Ogles2Context * context)
{
    GLenum err;
    while ((err = context->interface->glGetError()) != GL_NO_ERROR) {
        logLine("%s: GL error %d (%s) detected %s call", context->name, err, mapOgles2Error(err), info);
    }
}

#define PRE_CHECK check_errors("before", context);
#define POST_CHECK check_errors("after", context);

#define CHECK(x) \
PRE_CHECK \
x; \
POST_CHECK

#define CHECK_STATUS(x) \
PRE_CHECK \
status = x; \
POST_CHECK

// Wrap traced function calls

static void OGLES2_aglSwapBuffers(struct OGLES2IFace *Self)
{
    GET_CONTEXT

    logLine("%s: %s", context->name, __func__);

    if (context->old_aglSwapBuffers) {
        PROF_START

        CHECK(context->old_aglSwapBuffers(Self))

        PROF_FINISH(SwapBuffers)
    }
}

static void OGLES2_glCompileShader(struct OGLES2IFace *Self, GLuint shader)
{
    GET_CONTEXT

    logLine("%s: %s: shader %u", context->name, __func__,
        shader);

    if (context->old_glCompileShader) {
        PROF_START

        CHECK(context->old_glCompileShader(Self, shader))

        PROF_FINISH(CompileShader)
    }
}

static void OGLES2_glGenBuffers(struct OGLES2IFace *Self, GLsizei n, GLuint * buffers)
{
    GET_CONTEXT

    logLine("%s: %s: n %d, buffers %p", context->name, __func__,
        n, buffers);

    if (context->old_glGenBuffers) {
        PROF_START

        CHECK(context->old_glGenBuffers(Self, n, buffers))

        PROF_FINISH(GenBuffers)
    }

    ssize_t i;
    for (i = 0; i < n; i++) {
        logLine("Buffer[%u] = %u", i, buffers[i]);
    }
}

static void OGLES2_glBindBuffer(struct OGLES2IFace *Self, GLenum target, GLuint buffer)
{
    GET_CONTEXT

    logLine("%s: %s: target %d, buffer %u", context->name, __func__,
        target, buffer);

    if (context->old_glBindBuffer) {
        PROF_START

        CHECK(context->old_glBindBuffer(Self, target, buffer))

        PROF_FINISH(BindBuffer)
    }
}

static void OGLES2_glBufferData(struct OGLES2IFace *Self, GLenum target, GLsizeiptr size, const void * data, GLenum usage)
{
    GET_CONTEXT

    logLine("%s: %s: target %d, size %u, data %p, usage %d", context->name, __func__,
        target, size, data, usage);

    if (context->old_glBufferData) {
        PROF_START

        CHECK(context->old_glBufferData(Self, target, size, data, usage))

        PROF_FINISH(BufferData)
    }
}

static void OGLES2_glBufferSubData(struct OGLES2IFace *Self, GLenum target, GLintptr offset, GLsizeiptr size, const void * data)
{
    GET_CONTEXT

    logLine("%s: %s: target %d, offset %u, size %u, data %p", context->name, __func__,
        target, offset, size, data);

    if (context->old_glBufferSubData) {
        PROF_START

        CHECK(context->old_glBufferSubData(Self, target, offset, size, data))

        PROF_FINISH(BufferSubData)
    }
}

static void OGLES2_glDeleteBuffers(struct OGLES2IFace *Self, GLsizei n, GLuint * buffers)
{
    GET_CONTEXT

    logLine("%s: %s: n %d, buffers %p", context->name, __func__,
         n, buffers);

    GLsizei i;
    for (i = 0; i < n; i++) {
        logLine("Deleting buffer[%u] = %u", i, buffers[i]);
    }

    if (context->old_glDeleteBuffers) {
        PROF_START

        CHECK(context->old_glDeleteBuffers(Self, n, buffers))

        PROF_FINISH(DeleteBuffers)
    }
}

static void OGLES2_glEnableVertexAttribArray(struct OGLES2IFace *Self, GLuint index)
{
    GET_CONTEXT

    logLine("%s: %s: index %u", context->name, __func__,
        index);

    if (context->old_glEnableVertexAttribArray) {
        PROF_START

        CHECK(context->old_glEnableVertexAttribArray(Self, index))

        PROF_FINISH(EnableVertexAttribArray)
    }
}

static void OGLES2_glVertexAttribPointer(struct OGLES2IFace *Self, GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void * pointer)
{
    GET_CONTEXT

    logLine("%s: %s: index %u, size %d, type %d, normalized %d, stride %d, pointer %p", context->name, __func__,
        index, size, type, normalized, stride, pointer);

    if (context->old_glVertexAttribPointer) {
        PROF_START

        CHECK(context->old_glVertexAttribPointer(Self, index, size, type, normalized, stride, pointer))

        PROF_FINISH(VertexAttribPointer)
    }
}

static void countPrimitive(PrimitiveCounter * counter, const GLenum type, const size_t count)
{
    switch (type) {
        case GL_TRIANGLES:
            counter->triangles += count;
            break;
        case GL_TRIANGLE_STRIP:
            counter->triangleStrips += count;
            break;
        case GL_TRIANGLE_FAN:
            counter->triangleFans += count;
            break;
        case GL_LINES:
            counter->lines += count;
            break;
        case GL_LINE_STRIP:
            counter->lineStrips += count;
            break;
        case GL_LINE_LOOP:
            counter->lineLoops += count;
            break;
        case GL_POINTS:
            counter->points += count;
            break;

        default:
            logLine("Error - unknown primitive type %d passed to OGLES2", type);
            break;
    }
}

static void OGLES2_glDrawArrays(struct OGLES2IFace *Self, GLenum mode, GLint first, GLsizei count)
{
    GET_CONTEXT

    logLine("%s: %s: mode %d, first %d, count %d", context->name, __func__,
        mode, first, count);

    if (context->old_glDrawArrays) {
        PROF_START

        CHECK(context->old_glDrawArrays(Self, mode, first, count))

        PROF_FINISH(DrawArrays)

        countPrimitive(&context->counter, mode, count);
    }
}

static void OGLES2_glDrawElements(struct OGLES2IFace *Self, GLenum mode, GLsizei count, GLenum type, const void * indices)
{
    GET_CONTEXT

    logLine("%s: %s: mode %d, count %d, type %d, indices %p", context->name, __func__,
        mode, count, type, indices);

    if (context->old_glDrawElements) {
        PROF_START

        CHECK(context->old_glDrawElements(Self, mode, count, type, indices))

        PROF_FINISH(DrawElements)

        countPrimitive(&context->counter, mode, count);
    }
}

static void OGLES2_glShaderSource(struct OGLES2IFace *Self, GLuint shader, GLsizei count, const GLchar *const* string, const GLint * length)
{
    GET_CONTEXT

    logLine("%s: %s: shader %u, count %u, string %p length %p", context->name, __func__,
        shader, count, string, length);

    GLsizei i;

    if (length) {
        for (i = 0; i < count; i++) {
            logLine("Line %u: length %d: '%s'", i, length[i], string[i]);
        }
    } else {
        for (i = 0; i < count; i++) {
            logLine("Line %u: '%s'", i, string[i]);
        }
    }

    if (context->old_glShaderSource) {
        PROF_START

        CHECK(context->old_glShaderSource(Self, shader, count, string, length))

        PROF_FINISH(ShaderSource)
    }
}

static void OGLES2_glActiveTexture(struct OGLES2IFace *Self, GLenum texture)
{
    GET_CONTEXT

    logLine("%s: %s: texture %d", context->name, __func__,
        texture);

    if (context->old_glActiveTexture) {
        PROF_START

        CHECK(context->old_glActiveTexture(Self, texture))

        PROF_FINISH(ActiveTexture)
    }
}

static void OGLES2_glBindTexture(struct OGLES2IFace *Self, GLenum target, GLuint texture)
{
    GET_CONTEXT

    logLine("%s: %s: target %d, texture %d", context->name, __func__,
        target, texture);

    if (context->old_glBindTexture) {
        PROF_START

        CHECK(context->old_glBindTexture(Self, target, texture))

        PROF_FINISH(BindTexture)
    }
}

static void OGLES2_glGenTextures(struct OGLES2IFace *Self, GLsizei n, GLuint * textures)
{
    GET_CONTEXT

    logLine("%s: %s: n %d, textures %p", context->name, __func__,
        n, textures);

    if (context->old_glGenTextures) {
        PROF_START

        CHECK(context->old_glGenTextures(Self, n, textures))

        PROF_FINISH(GenTextures)
    }

    GLsizei i;
    for (i = 0; i < n; i++) {
        logLine("Texture[%u] = %u", i, textures[i]);
    }
}

static void OGLES2_glGenerateMipmap(struct OGLES2IFace *Self, GLenum target)
{
    GET_CONTEXT

    logLine("%s: %s: target %d", context->name, __func__,
        target);

    if (context->old_glGenerateMipmap) {
        PROF_START

        CHECK(context->old_glGenerateMipmap(Self, target))

        PROF_FINISH(GenerateMipmap)
    }
}

static void OGLES2_glTexParameterf(struct OGLES2IFace *Self, GLenum target, GLenum pname, GLfloat param)
{
    GET_CONTEXT

    logLine("%s: %s: target %d, pname %d, param %f", context->name, __func__,
        target, pname, param);

    if (context->old_glTexParameterf) {
        PROF_START

        CHECK(context->old_glTexParameterf(Self, target, pname, param))

        PROF_FINISH(TexParameterf)
    }
}

static void OGLES2_glTexParameterfv(struct OGLES2IFace *Self, GLenum target, GLenum pname, const GLfloat * params)
{
    GET_CONTEXT

    logLine("%s: %s: target %d, pname %d, params %p", context->name, __func__,
        target, pname, params);

    if (context->old_glTexParameterfv) {
        PROF_START

        CHECK(context->old_glTexParameterfv(Self, target, pname, params))

        PROF_FINISH(TexParameterfv)
    }
}

static void OGLES2_glTexParameteri(struct OGLES2IFace *Self, GLenum target, GLenum pname, GLint param)
{
    GET_CONTEXT

    logLine("%s: %s: target %d, pname %d, param %d", context->name, __func__,
        target, pname, param);

    if (context->old_glTexParameteri) {
        PROF_START

        CHECK(context->old_glTexParameteri(Self, target, pname, param))

        PROF_FINISH(TexParameteri)
    }
}

static void OGLES2_glTexParameteriv(struct OGLES2IFace *Self, GLenum target, GLenum pname, const GLint * params)
{
    GET_CONTEXT

    logLine("%s: %s: target %d, pname %d, params %p", context->name, __func__,
        target, pname, params);

    if (context->old_glTexParameteriv) {
        PROF_START

        CHECK(context->old_glTexParameteriv(Self, target, pname, params))

        PROF_FINISH(TexParameteriv)
    }
}

static void OGLES2_glTexSubImage2D(struct OGLES2IFace *Self, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void * pixels)
{
    GET_CONTEXT

    logLine("%s: %s: target %d, level %d, xoffset %d, yoffset %d, width %u, height %u, format %d, type %d, pixels %p", context->name, __func__,
        target, level, xoffset, yoffset, width, height, format, type, pixels);

    if (context->old_glTexSubImage2D) {
        PROF_START

        CHECK(context->old_glTexSubImage2D(Self, target, level, xoffset, yoffset, width, height, format, type, pixels))

        PROF_FINISH(TexSubImage2D)
    }
}

static void OGLES2_glTexImage2D(struct OGLES2IFace *Self, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void * pixels)
{
    GET_CONTEXT

    logLine("%s: %s: target %d, level %d, internalformat %d, width %u, height %u, border %d, format %d, type %d, pixels %p", context->name, __func__,
        target, level, internalformat, width, height, border, format, type, pixels);

    if (context->old_glTexImage2D) {
        PROF_START

        CHECK(context->old_glTexImage2D(Self, target, level, internalformat, width, height, border, format, type, pixels))

        PROF_FINISH(TexImage2D)
    }
}

static void OGLES2_glDeleteTextures(struct OGLES2IFace *Self, GLsizei n, const GLuint * textures)
{
    GET_CONTEXT

    logLine("%s: %s: n %u, textures %p", context->name, __func__,
        n, textures);

    GLsizei i;
    for (i = 0; i < n; i++) {
        logLine("Deleting texture[%u] = %u", i, textures[i]);
    }

    if (context->old_glDeleteTextures) {
        PROF_START

        CHECK(context->old_glDeleteTextures(Self, n, textures))

        PROF_FINISH(DeleteTextures)
    }
}

static void OGLES2_glGenFramebuffers(struct OGLES2IFace *Self, GLsizei n, GLuint * framebuffers)
{
    GET_CONTEXT

    logLine("%s: %s: n %u, framebuffers %p", context->name, __func__,
        n, framebuffers);

    if (context->old_glGenFramebuffers) {
        PROF_START

        CHECK(context->old_glGenFramebuffers(Self, n, framebuffers))

        PROF_FINISH(GenFramebuffers)
    }

    GLsizei i;
    for (i = 0; i < n; i++) {
        logLine("Framebuffer[%u] = %u", i, framebuffers[i]);
    }
}

static void OGLES2_glBindFramebuffer(struct OGLES2IFace *Self, GLenum target, GLuint framebuffer)
{
    GET_CONTEXT

    logLine("%s: %s: target %u, framebuffer %u", context->name, __func__,
        target, framebuffer);

    if (context->old_glBindFramebuffer) {
        PROF_START

        CHECK(context->old_glBindFramebuffer(Self, target, framebuffer))

        PROF_FINISH(BindFramebuffer)
    }
}

static GLenum OGLES2_glCheckFramebufferStatus(struct OGLES2IFace *Self, GLenum target)
{
    GET_CONTEXT

    GLenum status = 0;

    if (context->old_glCheckFramebufferStatus) {
        PROF_START

        CHECK_STATUS(context->old_glCheckFramebufferStatus(Self, target))

        PROF_FINISH(CheckFramebufferStatus)
    }

    logLine("%s: %s: status %u", context->name, __func__,
        status);

    return status;
}

static void OGLES2_glFramebufferRenderbuffer(struct OGLES2IFace *Self, GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)
{
    GET_CONTEXT

    logLine("%s: %s: target %u, attachment %u, renderbuffertarget %u, renderbuffer %u", context->name, __func__,
        target, attachment, renderbuffertarget, renderbuffer);

    if (context->old_glFramebufferRenderbuffer) {
        PROF_START

        CHECK(context->old_glFramebufferRenderbuffer(Self, target, attachment, renderbuffertarget, renderbuffer))

        PROF_FINISH(FramebufferRenderbuffer)
    }
}

static void OGLES2_glFramebufferTexture2D(struct OGLES2IFace *Self, GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
    GET_CONTEXT

    logLine("%s: %s: target %u, attachment %u, textarget %u, texture %u, level %d", context->name, __func__,
        target, attachment, textarget, texture, level);

    if (context->old_glFramebufferTexture2D) {
        PROF_START

        CHECK(context->old_glFramebufferTexture2D(Self, target, attachment, textarget, texture, level))

        PROF_FINISH(FramebufferTexture2D)
    }
}

static void OGLES2_glGetFramebufferAttachmentParameteriv(struct OGLES2IFace *Self, GLenum target, GLenum attachment, GLenum pname, GLint * params)
{
    GET_CONTEXT

    logLine("%s: %s: target %u, attachment %u pname %u, params %p", context->name, __func__,
        target, attachment, pname, params);

    if (context->old_glGetFramebufferAttachmentParameteriv) {
        PROF_START

        CHECK(context->old_glGetFramebufferAttachmentParameteriv(Self, target, attachment, pname, params))

        PROF_FINISH(GetFramebufferAttachmentParameteriv)
    }
}

static void OGLES2_glDeleteFramebuffers(struct OGLES2IFace *Self, GLsizei n, const GLuint * framebuffers)
{
    GET_CONTEXT

    logLine("%s: %s: n %u, framebuffers %p", context->name, __func__,
        n, framebuffers);

    GLsizei i;
    for (i = 0; i < n; i++) {
        logLine("Deleting framebuffer[%u] = %u", i, framebuffers[i]);
    }

    if (context->old_glDeleteFramebuffers) {
        PROF_START

        CHECK(context->old_glDeleteFramebuffers(Self, n, framebuffers))

        PROF_FINISH(DeleteFramebuffers)
    }
}

static void OGLES2_glClear(struct OGLES2IFace *Self, GLbitfield mask)
{
    GET_CONTEXT

    logLine("%s: %s mask 0x%X", context->name, __func__, mask);

    if (context->old_glClear) {
        PROF_START

        CHECK(context->old_glClear(Self, mask))

        PROF_FINISH(Clear)
    }
}

static void OGLES2_glUseProgram(struct OGLES2IFace *Self, GLuint program)
{
    GET_CONTEXT

    logLine("%s: %s program %u", context->name, __func__, program);

    PROF_START

    CHECK(context->old_glUseProgram(Self, program))

    PROF_FINISH(UseProgram)
}

GENERATE_FILTERED_PATCH(OGLES2IFace, aglSwapBuffers, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glCompileShader, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glGenBuffers, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glBindBuffer, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glBufferData, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glBufferSubData, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glDeleteBuffers, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glEnableVertexAttribArray, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glVertexAttribPointer, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glDrawArrays, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glDrawElements, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glShaderSource, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glActiveTexture, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glBindTexture, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glGenTextures, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glGenerateMipmap, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glTexParameterf, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glTexParameterfv, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glTexParameteri, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glTexParameteriv, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glTexSubImage2D, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glTexImage2D, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glDeleteTextures, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glGenFramebuffers, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glBindFramebuffer, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glCheckFramebufferStatus, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glFramebufferRenderbuffer, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glFramebufferTexture2D, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glGetFramebufferAttachmentParameteriv, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glDeleteFramebuffers, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glClear, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glUseProgram, OGLES2, Ogles2Context)

static void (*patches[])(BOOL, struct Ogles2Context *) = {
    patch_aglSwapBuffers,
    patch_glCompileShader,
    patch_glGenBuffers,
    patch_glBindBuffer,
    patch_glBufferData,
    patch_glBufferSubData,
    patch_glDeleteBuffers,
    patch_glEnableVertexAttribArray,
    patch_glVertexAttribPointer,
    patch_glDrawArrays,
    patch_glDrawElements,
    patch_glShaderSource,
    patch_glActiveTexture,
    patch_glBindTexture,
    patch_glGenTextures,
    patch_glGenerateMipmap,
    patch_glTexParameterf,
    patch_glTexParameterfv,
    patch_glTexParameteri,
    patch_glTexParameteriv,
    patch_glTexSubImage2D,
    patch_glTexImage2D,
    patch_glDeleteTextures,
    patch_glGenFramebuffers,
    patch_glBindFramebuffer,
    patch_glCheckFramebufferStatus,
    patch_glFramebufferRenderbuffer,
    patch_glFramebufferTexture2D,
    patch_glGetFramebufferAttachmentParameteriv,
    patch_glDeleteFramebuffers,
    patch_glClear,
    patch_glUseProgram
};

void ogles2_install_patches(void)
{
    execContext.interface = IExec;

    mutex = IExec->AllocSysObject(ASOT_MUTEX, TAG_DONE);

    if (!mutex) {
        logLine("Failed to allocate mutex");
        return;
    }

    if (open_ogles2_library()) {
        patch_GetInterface(TRUE, &execContext);
        patch_DropInterface(TRUE, &execContext);
    }
}

static void patch_ogles2_functions(struct Ogles2Context * ctx)
{
    if (ctx->interface) {
        size_t i;
        for (i = 0; i < sizeof(patches) / sizeof(patches[0]); i++) {
            patches[i](TRUE, ctx);
        }
    }
}

void ogles2_remove_patches(void)
{
    patch_DropInterface(FALSE, &execContext);
    patch_GetInterface(FALSE, &execContext);

    if (mutex) {
        // Remove patches
        size_t i;

        IExec->MutexObtain(mutex);

        for (i = 0; i < MAX_CLIENTS; i++) {
            if (contexts[i]) {
                profileResults(contexts[i]);

                size_t p;
                for (p = 0; p < sizeof(patches) / sizeof(patches[0]); p++) {
                    patches[p](FALSE, contexts[i]);
                }
            }
        }

        IExec->MutexRelease(mutex);
    }

    close_ogles2_library();
}

void ogles2_free(void)
{
    logLine("%s", __func__);

    if (mutex) {
        // Remove all known context data
        size_t i;

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
