#include "ogles2_module.h"
#include "common.h"
#include "filter.h"
#include "timer.h"
#include "profiling.h"
#include "logger.h"

#include <proto/exec.h>
#include <proto/ogles2.h>

#include <stdio.h>
#include <string.h>

struct Library* OGLES2Base;

static unsigned errorCount;
static BOOL profilingStarted = TRUE;

typedef enum Ogles2Function {
    ActiveTexture,
    AttachShader,
    BindAttribLocation,
    BindBuffer,
    BindFramebuffer,
    BindRenderbuffer,
    BindTexture,
    BufferData,
    BufferSubData,
    CheckFramebufferStatus,
    Clear,
    CompileShader,
    DeleteBuffers,
    DeleteFramebuffers,
    DeleteTextures,
    Disable,
    DrawArrays,
    DrawElements,
    Enable,
    EnableVertexAttribArray,
    Finish,
    Flush,
    FramebufferRenderbuffer,
    FramebufferTexture2D,
    GenBuffers,
    GenerateMipmap,
    GenFramebuffers,
    GenTextures,
    GetFramebufferAttachmentParameteriv,
    ShaderSource,
    SwapBuffers,
    TexImage2D,
    TexParameterf,
    TexParameterfv,
    TexParameteri,
    TexParameteriv,
    TexSubImage2D,
    Uniform1f,
    Uniform1fv,
    Uniform1i,
    Uniform1iv,
    Uniform2f,
    Uniform2fv,
    Uniform2i,
    Uniform2iv,
    Uniform3f,
    Uniform3fv,
    Uniform3i,
    Uniform3iv,
    Uniform4f,
    Uniform4fv,
    Uniform4i,
    Uniform4iv,
    UniformMatrix2fv,
    UniformMatrix3fv,
    UniformMatrix4fv,
    UseProgram,
    VertexAttribPointer,
    // Keep last
    Ogles2FunctionCount
} Ogles2Function;

static const char* mapOgles2Function(const Ogles2Function func)
{
    #define MAP_ENUM(x) case x: return #x;

    switch (func) {
        MAP_ENUM(ActiveTexture)
        MAP_ENUM(AttachShader)
        MAP_ENUM(BindAttribLocation)
        MAP_ENUM(BindBuffer)
        MAP_ENUM(BindFramebuffer)
        MAP_ENUM(BindRenderbuffer)
        MAP_ENUM(BindTexture)
        MAP_ENUM(BufferData)
        MAP_ENUM(BufferSubData)
        MAP_ENUM(CheckFramebufferStatus)
        MAP_ENUM(Clear)
        MAP_ENUM(CompileShader)
        MAP_ENUM(DeleteBuffers)
        MAP_ENUM(DeleteFramebuffers)
        MAP_ENUM(DeleteTextures)
        MAP_ENUM(Disable)
        MAP_ENUM(DrawArrays)
        MAP_ENUM(DrawElements)
        MAP_ENUM(Enable)
        MAP_ENUM(EnableVertexAttribArray)
        MAP_ENUM(Finish)
        MAP_ENUM(Flush)
        MAP_ENUM(FramebufferRenderbuffer)
        MAP_ENUM(FramebufferTexture2D)
        MAP_ENUM(GenBuffers)
        MAP_ENUM(GenerateMipmap)
        MAP_ENUM(GenFramebuffers)
        MAP_ENUM(GenTextures)
        MAP_ENUM(GetFramebufferAttachmentParameteriv)
        MAP_ENUM(ShaderSource)
        MAP_ENUM(SwapBuffers)
        MAP_ENUM(TexImage2D)
        MAP_ENUM(TexParameterf)
        MAP_ENUM(TexParameterfv)
        MAP_ENUM(TexParameteri)
        MAP_ENUM(TexParameteriv)
        MAP_ENUM(TexSubImage2D)
        MAP_ENUM(Uniform1f)
        MAP_ENUM(Uniform1fv)
        MAP_ENUM(Uniform1i)
        MAP_ENUM(Uniform1iv)
        MAP_ENUM(Uniform2f)
        MAP_ENUM(Uniform2fv)
        MAP_ENUM(Uniform2i)
        MAP_ENUM(Uniform2iv)
        MAP_ENUM(Uniform3f)
        MAP_ENUM(Uniform3fv)
        MAP_ENUM(Uniform3i)
        MAP_ENUM(Uniform3iv)
        MAP_ENUM(Uniform4f)
        MAP_ENUM(Uniform4fv)
        MAP_ENUM(Uniform4i)
        MAP_ENUM(Uniform4iv)
        MAP_ENUM(UniformMatrix2fv)
        MAP_ENUM(UniformMatrix3fv)
        MAP_ENUM(UniformMatrix4fv)
        MAP_ENUM(UseProgram)
        MAP_ENUM(VertexAttribPointer)

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
    void (*old_glActiveTexture)(struct OGLES2IFace *Self, GLenum texture);
    void (*old_glAttachShader)(struct OGLES2IFace *Self, GLuint program, GLuint shader);
    void (*old_glBindAttribLocation)(struct OGLES2IFace *Self, GLuint program, GLuint index, const GLchar * name);
    void (*old_glBindBuffer)(struct OGLES2IFace *Self, GLenum target, GLuint buffer);
    void (*old_glBindFramebuffer)(struct OGLES2IFace *Self, GLenum target, GLuint framebuffer);
    void (*old_glBindRenderbuffer)(struct OGLES2IFace *Self, GLenum target, GLuint renderbuffer);
    void (*old_glBindTexture)(struct OGLES2IFace *Self, GLenum target, GLuint texture);
    void (*old_glBufferData)(struct OGLES2IFace *Self, GLenum target, GLsizeiptr size, const void * data, GLenum usage);
    void (*old_glBufferSubData)(struct OGLES2IFace *Self, GLenum target, GLintptr offset, GLsizeiptr size, const void * data);
    GLenum (*old_glCheckFramebufferStatus)(struct OGLES2IFace *Self, GLenum target);
    void (*old_glClear)(struct OGLES2IFace *Self, GLbitfield mask);
    void (*old_glCompileShader)(struct OGLES2IFace *Self, GLuint shader);
    void (*old_glDeleteBuffers)(struct OGLES2IFace *Self, GLsizei n, GLuint * buffers);
    void (*old_glDeleteFramebuffers)(struct OGLES2IFace *Self, GLsizei n, const GLuint * framebuffers);
    void (*old_glDeleteTextures)(struct OGLES2IFace *Self, GLsizei n, const GLuint * textures);
    void (*old_glDisable)(struct OGLES2IFace *Self, GLenum cap);
    void (*old_glDrawArrays)(struct OGLES2IFace *Self, GLenum mode, GLint first, GLsizei count);
    void (*old_glDrawElements)(struct OGLES2IFace *Self, GLenum mode, GLsizei count, GLenum type, const void * indices);
    void (*old_glEnable)(struct OGLES2IFace *Self, GLenum cap);
    void (*old_glEnableVertexAttribArray)(struct OGLES2IFace *Self, GLuint index);
    void (*old_glFinish)(struct OGLES2IFace *Self);
    void (*old_glFlush)(struct OGLES2IFace *Self);
    void (*old_glFramebufferRenderbuffer)(struct OGLES2IFace *Self, GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
    void (*old_glFramebufferTexture2D)(struct OGLES2IFace *Self, GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
    void (*old_glGenBuffers)(struct OGLES2IFace *Self, GLsizei n, GLuint * buffers);
    void (*old_glGenerateMipmap)(struct OGLES2IFace *Self, GLenum target);
    void (*old_glGenFramebuffers)(struct OGLES2IFace *Self, GLsizei n, GLuint * framebuffers);
    void (*old_glGenTextures)(struct OGLES2IFace *Self, GLsizei n, GLuint * textures);
    void (*old_glGetFramebufferAttachmentParameteriv)(struct OGLES2IFace *Self, GLenum target, GLenum attachment, GLenum pname, GLint * params);
    void (*old_glShaderSource)(struct OGLES2IFace *Self, GLuint shader, GLsizei count, const GLchar *const* string, const GLint * length);
    void (*old_glTexImage2D)(struct OGLES2IFace *Self, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void * pixels);
    void (*old_glTexParameterf)(struct OGLES2IFace *Self, GLenum target, GLenum pname, GLfloat param);
    void (*old_glTexParameterfv)(struct OGLES2IFace *Self, GLenum target, GLenum pname, const GLfloat * params);
    void (*old_glTexParameteri)(struct OGLES2IFace *Self, GLenum target, GLenum pname, GLint param);
    void (*old_glTexParameteriv)(struct OGLES2IFace *Self, GLenum target, GLenum pname, const GLint * params);
    void (*old_glTexSubImage2D)(struct OGLES2IFace *Self, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void * pixels);
    void (*old_glUniform1f)(struct OGLES2IFace *Self, GLint location, GLfloat v0);
    void (*old_glUniform1fv)(struct OGLES2IFace *Self, GLint location, GLsizei count, const GLfloat * value);
    void (*old_glUniform1i)(struct OGLES2IFace *Self, GLint location, GLint v0);
    void (*old_glUniform1iv)(struct OGLES2IFace *Self, GLint location, GLsizei count, const GLint * value);
    void (*old_glUniform2f)(struct OGLES2IFace *Self, GLint location, GLfloat v0, GLfloat v1);
    void (*old_glUniform2fv)(struct OGLES2IFace *Self, GLint location, GLsizei count, const GLfloat * value);
    void (*old_glUniform2i)(struct OGLES2IFace *Self, GLint location, GLint v0, GLint v1);
    void (*old_glUniform2iv)(struct OGLES2IFace *Self, GLint location, GLsizei count, const GLint * value);
    void (*old_glUniform3f)(struct OGLES2IFace *Self, GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
    void (*old_glUniform3fv)(struct OGLES2IFace *Self, GLint location, GLsizei count, const GLfloat * value);
    void (*old_glUniform3i)(struct OGLES2IFace *Self, GLint location, GLint v0, GLint v1, GLint v2);
    void (*old_glUniform3iv)(struct OGLES2IFace *Self, GLint location, GLsizei count, const GLint * value);
    void (*old_glUniform4f)(struct OGLES2IFace *Self, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
    void (*old_glUniform4fv)(struct OGLES2IFace *Self, GLint location, GLsizei count, const GLfloat * value);
    void (*old_glUniform4i)(struct OGLES2IFace *Self, GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
    void (*old_glUniform4iv)(struct OGLES2IFace *Self, GLint location, GLsizei count, const GLint * value);
    void (*old_glUniformMatrix2fv)(struct OGLES2IFace *Self, GLint location, GLsizei count, GLboolean transpose, const GLfloat * value);
    void (*old_glUniformMatrix3fv)(struct OGLES2IFace *Self, GLint location, GLsizei count, GLboolean transpose, const GLfloat * value);
    void (*old_glUniformMatrix4fv)(struct OGLES2IFace *Self, GLint location, GLsizei count, GLboolean transpose, const GLfloat * value);
    void (*old_glUseProgram)(struct OGLES2IFace *Self, GLuint program);
    // void (*old_glVertexAttrib3fv)(struct OGLES2IFace *Self, GLuint index, const GLfloat * v);
    void (*old_glVertexAttribPointer)(struct OGLES2IFace *Self, GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void * pointer);
};

static struct Ogles2Context* contexts[MAX_CLIENTS];
static APTR mutex;

static void patch_ogles2_functions(struct Ogles2Context *);

static void find_process_name(struct Ogles2Context * context)
{
    find_process_name2((struct Node *)context->task, context->name);
}

static char versionBuffer[64] = "ogles2.library version unknown";

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
    static char errorBuffer[32];

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

static void profileResults(struct Ogles2Context* const context)
{
    if (!profilingStarted) {
        logAlways("OGLES2 profiling not started, skip summary");
        return;
    }

    PROF_FINISH_CONTEXT

    const double drawcalls = context->profiling[DrawElements].callCount + context->profiling[DrawArrays].callCount;
    const uint64 swaps = context->profiling[SwapBuffers].callCount;

    // Copy items, otherwise sorthing will ruin the further profiling
    ProfilingItem stats[Ogles2FunctionCount];
    memcpy(stats, context->profiling, Ogles2FunctionCount * sizeof(ProfilingItem));

    sort(stats, Ogles2FunctionCount);

    logAlways("\nOpenGL ES 2.0 profiling results for %s:", context->name);

    PROF_PRINT_TOTAL

    if (swaps > 0) {
        logAlways("  Draw calls/frame %.1f. Draw calls/s %.1f", drawcalls / swaps, drawcalls / seconds);
    }

    logAlways("  Frames/s %.1f", swaps / seconds);

    logAlways("%30s | %10s | %10s | %20s | %20s | %24s | %20s",
        "function", "call count", "errors", "duration (ms)", "avg. call dur. (us)", timeUsedBuffer, "% of CPU time");

    for (int i = 0; i < Ogles2FunctionCount; i++) {
        if (stats[i].callCount > 0) {
            logAlways("%30s | %10llu | %10llu | %20.6f | %20.3f | %24.2f | %20.2f",
                mapOgles2Function(stats[i].index),
                stats[i].callCount,
                stats[i].errors,
                timer_ticks_to_ms(stats[i].ticks),
                timer_ticks_to_us(stats[i].ticks) / stats[i].callCount,
                (double)stats[i].ticks * 100.0 / context->ticks,
                (double)stats[i].ticks * 100.0 / totalTicks);
        }
    }

    logAlways("  *) Please note that the above time measurements include time spent inside Warp3D Nova functions");

    primitiveStats(&context->counter, seconds, drawcalls);
}

void ogles2_start_profiling(void)
{
    profilingStarted = TRUE;

    if (mutex) {
        IExec->MutexObtain(mutex);

        for (size_t c = 0; c < MAX_CLIENTS; c++) {
            if (contexts[c]) {
                // TODO: concurrency issues?
                PROF_INIT(contexts[c], Ogles2FunctionCount)
            }
        }

        IExec->MutexRelease(mutex);
    }
}

void ogles2_finish_profiling(void)
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

static void checkErrors(struct Ogles2Context * context, const Ogles2Function id)
{
    GLenum err;
    while ((err = context->interface->glGetError()) != GL_NO_ERROR) {
        logLine("%s: GL error %d (%s) detected", context->name, err, mapOgles2Error(err));
        context->profiling[id].errors++;
    }
}

#define CHECK(x, id) \
PROF_START \
x; \
PROF_FINISH(id) \
checkErrors(context, id);

#define CHECK_STATUS(x, id) \
PROF_START \
status = x; \
PROF_FINISH(id) \
checkErrors(context, id);

// Wrap traced function calls

static void OGLES2_aglSwapBuffers(struct OGLES2IFace *Self)
{
    GET_CONTEXT

    logLine("%s: %s", context->name, __func__);

    if (context->old_aglSwapBuffers) {
        CHECK(context->old_aglSwapBuffers(Self), SwapBuffers)
    }
}

static void OGLES2_glActiveTexture(struct OGLES2IFace *Self, GLenum texture)
{
    GET_CONTEXT

    logLine("%s: %s: texture %d", context->name, __func__,
        texture);

    if (context->old_glActiveTexture) {
        CHECK(context->old_glActiveTexture(Self, texture), ActiveTexture)
    }
}

static void OGLES2_glAttachShader(struct OGLES2IFace *Self, GLuint program, GLuint shader)
{
    GET_CONTEXT

    logLine("%s: %s: program %u, shader %u", context->name, __func__,
        program, shader);

    if (context->old_glAttachShader) {
        CHECK(context->old_glAttachShader(Self, program, shader), AttachShader)
    }
}

static void OGLES2_glBindAttribLocation(struct OGLES2IFace *Self, GLuint program, GLuint index, const GLchar * name)
{
    GET_CONTEXT

    logLine("%s: %s: program %u, index %u, name '%s'", context->name, __func__,
        program, index, name);

    if (context->old_glBindAttribLocation) {
        CHECK(context->old_glBindAttribLocation(Self, program, index, name), BindAttribLocation)
    }
}

static void OGLES2_glBindBuffer(struct OGLES2IFace *Self, GLenum target, GLuint buffer)
{
    GET_CONTEXT

    logLine("%s: %s: target %d, buffer %u", context->name, __func__,
        target, buffer);

    if (context->old_glBindBuffer) {
        CHECK(context->old_glBindBuffer(Self, target, buffer), BindBuffer)
    }
}

static void OGLES2_glBindFramebuffer(struct OGLES2IFace *Self, GLenum target, GLuint framebuffer)
{
    GET_CONTEXT

    logLine("%s: %s: target %u, framebuffer %u", context->name, __func__,
        target, framebuffer);

    if (context->old_glBindFramebuffer) {
        CHECK(context->old_glBindFramebuffer(Self, target, framebuffer), BindFramebuffer)
    }
}

static void OGLES2_glBindRenderbuffer(struct OGLES2IFace *Self, GLenum target, GLuint renderbuffer)
{
    GET_CONTEXT

    logLine("%s: %s: target %u, renderbuffer %u", context->name, __func__,
        target, renderbuffer);

    if (context->old_glBindRenderbuffer) {
        CHECK(context->old_glBindRenderbuffer(Self, target, renderbuffer), BindRenderbuffer)
    }
}

static void OGLES2_glBindTexture(struct OGLES2IFace *Self, GLenum target, GLuint texture)
{
    GET_CONTEXT

    logLine("%s: %s: target %d, texture %d", context->name, __func__,
        target, texture);

    if (context->old_glBindTexture) {
        CHECK(context->old_glBindTexture(Self, target, texture), BindTexture)
    }
}

static void OGLES2_glBufferData(struct OGLES2IFace *Self, GLenum target, GLsizeiptr size, const void * data, GLenum usage)
{
    GET_CONTEXT

    logLine("%s: %s: target %d, size %u, data %p, usage %d", context->name, __func__,
        target, size, data, usage);

    if (context->old_glBufferData) {
        CHECK(context->old_glBufferData(Self, target, size, data, usage), BufferData)
    }
}

static void OGLES2_glBufferSubData(struct OGLES2IFace *Self, GLenum target, GLintptr offset, GLsizeiptr size, const void * data)
{
    GET_CONTEXT

    logLine("%s: %s: target %d, offset %u, size %u, data %p", context->name, __func__,
        target, offset, size, data);

    if (context->old_glBufferSubData) {
        CHECK(context->old_glBufferSubData(Self, target, offset, size, data), BufferSubData)
    }
}

static GLenum OGLES2_glCheckFramebufferStatus(struct OGLES2IFace *Self, GLenum target)
{
    GET_CONTEXT

    GLenum status = 0;

    if (context->old_glCheckFramebufferStatus) {
        CHECK_STATUS(context->old_glCheckFramebufferStatus(Self, target), CheckFramebufferStatus)
    }

    logLine("%s: %s: status %u", context->name, __func__,
        status);

    return status;
}

static void OGLES2_glClear(struct OGLES2IFace *Self, GLbitfield mask)
{
    GET_CONTEXT

    logLine("%s: %s mask 0x%X", context->name, __func__, mask);

    if (context->old_glClear) {
        CHECK(context->old_glClear(Self, mask), Clear)
    }
}

static void OGLES2_glCompileShader(struct OGLES2IFace *Self, GLuint shader)
{
    GET_CONTEXT

    logLine("%s: %s: shader %u", context->name, __func__,
        shader);

    if (context->old_glCompileShader) {
        CHECK(context->old_glCompileShader(Self, shader), CompileShader)
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
        CHECK(context->old_glDeleteBuffers(Self, n, buffers), DeleteBuffers)
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
        CHECK(context->old_glDeleteFramebuffers(Self, n, framebuffers), DeleteFramebuffers)
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
        CHECK(context->old_glDeleteTextures(Self, n, textures), DeleteTextures)
    }
}

static void OGLES2_glDisable(struct OGLES2IFace *Self, GLenum cap)
{
    GET_CONTEXT

    logLine("%s: %s: cap %d", context->name, __func__,
        cap);

    if (context->old_glDisable) {
        CHECK(context->old_glDisable(Self, cap), Disable)
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
        CHECK(context->old_glDrawArrays(Self, mode, first, count), DrawArrays)

        countPrimitive(&context->counter, mode, count);
    }
}

static void OGLES2_glDrawElements(struct OGLES2IFace *Self, GLenum mode, GLsizei count, GLenum type, const void * indices)
{
    GET_CONTEXT

    logLine("%s: %s: mode %d, count %d, type %d, indices %p", context->name, __func__,
        mode, count, type, indices);

    if (context->old_glDrawElements) {
        CHECK(context->old_glDrawElements(Self, mode, count, type, indices), DrawElements)

        countPrimitive(&context->counter, mode, count);
    }
}

static void OGLES2_glEnable(struct OGLES2IFace *Self, GLenum cap)
{
    GET_CONTEXT

    logLine("%s: %s: cap %d", context->name, __func__,
        cap);

    if (context->old_glEnable) {
        CHECK(context->old_glEnable(Self, cap), Enable)
    }
}

static void OGLES2_glEnableVertexAttribArray(struct OGLES2IFace *Self, GLuint index)
{
    GET_CONTEXT

    logLine("%s: %s: index %u", context->name, __func__,
        index);

    if (context->old_glEnableVertexAttribArray) {
        CHECK(context->old_glEnableVertexAttribArray(Self, index), EnableVertexAttribArray)
    }
}

static void OGLES2_glFinish(struct OGLES2IFace *Self)
{
    GET_CONTEXT

    logLine("%s: %s", context->name, __func__);

    if (context->old_glFinish) {
        CHECK(context->old_glFinish(Self), Finish)
    }
}

static void OGLES2_glFlush(struct OGLES2IFace *Self)
{
    GET_CONTEXT

    logLine("%s: %s", context->name, __func__);

    if (context->old_glFlush) {
        CHECK(context->old_glFlush(Self), Flush)
    }
}

static void OGLES2_glFramebufferRenderbuffer(struct OGLES2IFace *Self, GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)
{
    GET_CONTEXT

    logLine("%s: %s: target %u, attachment %u, renderbuffertarget %u, renderbuffer %u", context->name, __func__,
        target, attachment, renderbuffertarget, renderbuffer);

    if (context->old_glFramebufferRenderbuffer) {
        CHECK(context->old_glFramebufferRenderbuffer(Self, target, attachment, renderbuffertarget, renderbuffer), FramebufferRenderbuffer)
    }
}

static void OGLES2_glFramebufferTexture2D(struct OGLES2IFace *Self, GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
    GET_CONTEXT

    logLine("%s: %s: target %u, attachment %u, textarget %u, texture %u, level %d", context->name, __func__,
        target, attachment, textarget, texture, level);

    if (context->old_glFramebufferTexture2D) {
        CHECK(context->old_glFramebufferTexture2D(Self, target, attachment, textarget, texture, level), FramebufferTexture2D)
    }
}

static void OGLES2_glGenBuffers(struct OGLES2IFace *Self, GLsizei n, GLuint * buffers)
{
    GET_CONTEXT

    logLine("%s: %s: n %d, buffers %p", context->name, __func__,
        n, buffers);

    if (context->old_glGenBuffers) {
        CHECK(context->old_glGenBuffers(Self, n, buffers), GenBuffers)
    }

    ssize_t i;
    for (i = 0; i < n; i++) {
        logLine("Buffer[%u] = %u", i, buffers[i]);
    }
}

static void OGLES2_glGenerateMipmap(struct OGLES2IFace *Self, GLenum target)
{
    GET_CONTEXT

    logLine("%s: %s: target %d", context->name, __func__,
        target);

    if (context->old_glGenerateMipmap) {
        CHECK(context->old_glGenerateMipmap(Self, target), GenerateMipmap)
    }
}

static void OGLES2_glGenFramebuffers(struct OGLES2IFace *Self, GLsizei n, GLuint * framebuffers)
{
    GET_CONTEXT

    logLine("%s: %s: n %u, framebuffers %p", context->name, __func__,
        n, framebuffers);

    if (context->old_glGenFramebuffers) {
        CHECK(context->old_glGenFramebuffers(Self, n, framebuffers), GenFramebuffers)
    }

    GLsizei i;
    for (i = 0; i < n; i++) {
        logLine("Framebuffer[%u] = %u", i, framebuffers[i]);
    }
}

static void OGLES2_glGenTextures(struct OGLES2IFace *Self, GLsizei n, GLuint * textures)
{
    GET_CONTEXT

    logLine("%s: %s: n %d, textures %p", context->name, __func__,
        n, textures);

    if (context->old_glGenTextures) {
        CHECK(context->old_glGenTextures(Self, n, textures), GenTextures)
    }

    GLsizei i;
    for (i = 0; i < n; i++) {
        logLine("Texture[%u] = %u", i, textures[i]);
    }
}

static void OGLES2_glGetFramebufferAttachmentParameteriv(struct OGLES2IFace *Self, GLenum target, GLenum attachment, GLenum pname, GLint * params)
{
    GET_CONTEXT

    logLine("%s: %s: target %u, attachment %u pname %u, params %p", context->name, __func__,
        target, attachment, pname, params);

    if (context->old_glGetFramebufferAttachmentParameteriv) {
        CHECK(context->old_glGetFramebufferAttachmentParameteriv(Self, target, attachment, pname, params), GetFramebufferAttachmentParameteriv)
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
        CHECK(context->old_glShaderSource(Self, shader, count, string, length), ShaderSource)
    }
}

static void OGLES2_glTexImage2D(struct OGLES2IFace *Self, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void * pixels)
{
    GET_CONTEXT

    logLine("%s: %s: target %d, level %d, internalformat %d, width %u, height %u, border %d, format %d, type %d, pixels %p", context->name, __func__,
        target, level, internalformat, width, height, border, format, type, pixels);

    if (context->old_glTexImage2D) {
        CHECK(context->old_glTexImage2D(Self, target, level, internalformat, width, height, border, format, type, pixels), TexImage2D)
    }
}

static void OGLES2_glTexParameterf(struct OGLES2IFace *Self, GLenum target, GLenum pname, GLfloat param)
{
    GET_CONTEXT

    logLine("%s: %s: target %d, pname %d, param %f", context->name, __func__,
        target, pname, param);

    if (context->old_glTexParameterf) {
        CHECK(context->old_glTexParameterf(Self, target, pname, param), TexParameterf)
    }
}

static void OGLES2_glTexParameterfv(struct OGLES2IFace *Self, GLenum target, GLenum pname, const GLfloat * params)
{
    GET_CONTEXT

    logLine("%s: %s: target %d, pname %d, params %p", context->name, __func__,
        target, pname, params);

    if (context->old_glTexParameterfv) {
        CHECK(context->old_glTexParameterfv(Self, target, pname, params), TexParameterfv)
    }
}

static void OGLES2_glTexParameteri(struct OGLES2IFace *Self, GLenum target, GLenum pname, GLint param)
{
    GET_CONTEXT

    logLine("%s: %s: target %d, pname %d, param %d", context->name, __func__,
        target, pname, param);

    if (context->old_glTexParameteri) {
        CHECK(context->old_glTexParameteri(Self, target, pname, param), TexParameteri)
    }
}

static void OGLES2_glTexParameteriv(struct OGLES2IFace *Self, GLenum target, GLenum pname, const GLint * params)
{
    GET_CONTEXT

    logLine("%s: %s: target %d, pname %d, params %p", context->name, __func__,
        target, pname, params);

    if (context->old_glTexParameteriv) {
        CHECK(context->old_glTexParameteriv(Self, target, pname, params), TexParameteriv)
    }
}

static void OGLES2_glTexSubImage2D(struct OGLES2IFace *Self, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void * pixels)
{
    GET_CONTEXT

    logLine("%s: %s: target %d, level %d, xoffset %d, yoffset %d, width %u, height %u, format %d, type %d, pixels %p", context->name, __func__,
        target, level, xoffset, yoffset, width, height, format, type, pixels);

    if (context->old_glTexSubImage2D) {
        CHECK(context->old_glTexSubImage2D(Self, target, level, xoffset, yoffset, width, height, format, type, pixels), TexSubImage2D)
    }
}

static void OGLES2_glUniform1f(struct OGLES2IFace *Self, GLint location, GLfloat v0)
{
    GET_CONTEXT

    logLine("%s: %s: location %d, v0 %f", context->name, __func__,
        location, v0);

    if (context->old_glUniform1f) {
        CHECK(context->old_glUniform1f(Self, location, v0), Uniform1f)
    }
}

static void OGLES2_glUniform1fv(struct OGLES2IFace *Self, GLint location, GLsizei count, const GLfloat * value)
{
    GET_CONTEXT

    logLine("%s: %s: location %d, count %d", context->name, __func__,
        location, count);

    for (GLsizei i = 0; i < count; i++) {
        logLine("v%d { %f }", i, value[i]);
    }

    if (context->old_glUniform1fv) {
        CHECK(context->old_glUniform1fv(Self, location, count, value), Uniform1fv)
    }
}

static void OGLES2_glUniform1i(struct OGLES2IFace *Self, GLint location, GLint v0)
{
    GET_CONTEXT

    logLine("%s: %s: location %d, v0 %d", context->name, __func__,
        location, v0);

    if (context->old_glUniform1i) {
        CHECK(context->old_glUniform1i(Self, location, v0), Uniform1i)
    }
}

static void OGLES2_glUniform1iv(struct OGLES2IFace *Self, GLint location, GLsizei count, const GLint * value)
{
    GET_CONTEXT

    logLine("%s: %s: location %d, count %d", context->name, __func__,
        location, count);

    for (GLsizei i = 0; i < count; i++) {
        logLine("v%d { %d }", i, value[i]);
    }

    if (context->old_glUniform1iv) {
        CHECK(context->old_glUniform1iv(Self, location, count, value), Uniform1iv)
    }
}

static void OGLES2_glUniform2f(struct OGLES2IFace *Self, GLint location, GLfloat v0, GLfloat v1)
{
    GET_CONTEXT

    logLine("%s: %s: location %d, v0 %f, v1 %f", context->name, __func__,
        location, v0, v1);

    if (context->old_glUniform2f) {
        CHECK(context->old_glUniform2f(Self, location, v0, v1), Uniform2f)
    }
}

static void OGLES2_glUniform2fv(struct OGLES2IFace *Self, GLint location, GLsizei count, const GLfloat * value)
{
    GET_CONTEXT

    logLine("%s: %s: location %d, count %d", context->name, __func__,
        location, count);

    for (GLsizei i = 0; i < count; i++) {
        const GLsizei index = 2 * i;
        logLine("v%d {%f, %f}", i, value[index], value[index + 1]);
    }

    if (context->old_glUniform2fv) {
        CHECK(context->old_glUniform2fv(Self, location, count, value), Uniform2fv)
    }
}

static void OGLES2_glUniform2i(struct OGLES2IFace *Self, GLint location, GLint v0, GLint v1)
{
    GET_CONTEXT

    logLine("%s: %s: location %d, v0 %d, v1 %d", context->name, __func__,
        location, v0, v1);

    if (context->old_glUniform2i) {
        CHECK(context->old_glUniform2i(Self, location, v0, v1), Uniform2i)
    }
}

static void OGLES2_glUniform2iv(struct OGLES2IFace *Self, GLint location, GLsizei count, const GLint * value)
{
    GET_CONTEXT

    logLine("%s: %s: location %d, count %d", context->name, __func__,
        location, count);

    for (GLsizei i = 0; i < count; i++) {
        const GLsizei index = 2 * i;
        logLine("v%d {%d, %d}", i, value[index], value[index + 1]);
    }

    if (context->old_glUniform2iv) {
        CHECK(context->old_glUniform2iv(Self, location, count, value), Uniform2iv)
    }
}

static void OGLES2_glUniform3f(struct OGLES2IFace *Self, GLint location, GLfloat v0, GLfloat v1, GLfloat v2)
{
    GET_CONTEXT

    logLine("%s: %s: location %d, v0 %f, v1 %f, v2 %f", context->name, __func__,
        location, v0, v1, v2);

    if (context->old_glUniform3f) {
        CHECK(context->old_glUniform3f(Self, location, v0, v1, v2), Uniform3f)
    }
}

static void OGLES2_glUniform3fv(struct OGLES2IFace *Self, GLint location, GLsizei count, const GLfloat * value)
{
    GET_CONTEXT

    logLine("%s: %s: location %d, count %d", context->name, __func__,
        location, count);

    for (GLsizei i = 0; i < count; i++) {
        const GLsizei index = 3 * i;
        logLine("v%d {%f, %f, %f}", i, value[index], value[index + 1], value[index + 2]);
    }

    if (context->old_glUniform3fv) {
        CHECK(context->old_glUniform3fv(Self, location, count, value), Uniform3fv)
    }
}

static void OGLES2_glUniform3i(struct OGLES2IFace *Self, GLint location, GLint v0, GLint v1, GLint v2)
{
    GET_CONTEXT

    logLine("%s: %s: location %d, v0 %d, v1 %d, v2 %d", context->name, __func__,
        location, v0, v1, v2);

    if (context->old_glUniform3i) {
        CHECK(context->old_glUniform3i(Self, location, v0, v1, v2), Uniform3i)
    }
}

static void OGLES2_glUniform3iv(struct OGLES2IFace *Self, GLint location, GLsizei count, const GLint * value)
{
    GET_CONTEXT

    logLine("%s: %s: location %d, count %d", context->name, __func__,
        location, count);

    for (GLsizei i = 0; i < count; i++) {
        const GLsizei index = 3 * i;
        logLine("v%d {%d, %d, %d}", i, value[index], value[index + 1], value[index + 2]);
    }

    if (context->old_glUniform3iv) {
        CHECK(context->old_glUniform3iv(Self, location, count, value), Uniform3iv)
    }
}

static void OGLES2_glUniform4f(struct OGLES2IFace *Self, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)
{
    GET_CONTEXT

    logLine("%s: %s: location %d, v0 %f, v1 %f, v2 %f, v3 %f", context->name, __func__,
        location, v0, v1, v2, v3);

    if (context->old_glUniform4f) {
        CHECK(context->old_glUniform4f(Self, location, v0, v1, v2, v3), Uniform4f)
    }
}

static void OGLES2_glUniform4fv(struct OGLES2IFace *Self, GLint location, GLsizei count, const GLfloat * value)
{
    GET_CONTEXT

    logLine("%s: %s: location %d, count %d", context->name, __func__,
        location, count);

    for (GLsizei i = 0; i < count; i++) {
        const GLsizei index = 4 * i;
        logLine("v%d {%f, %f, %f, %f}", i, value[index], value[index + 1], value[index + 2], value[index + 3]);
    }

    if (context->old_glUniform4fv) {
        CHECK(context->old_glUniform4fv(Self, location, count, value), Uniform4fv)
    }
}

static void OGLES2_glUniform4i(struct OGLES2IFace *Self, GLint location, GLint v0, GLint v1, GLint v2, GLint v3)
{
    GET_CONTEXT

    logLine("%s: %s: location %d, v0 %d, v1 %d, v2 %d, v3 %d", context->name, __func__,
        location, v0, v1, v2, v3);

    if (context->old_glUniform4i) {
        CHECK(context->old_glUniform4i(Self, location, v0, v1, v2, v3), Uniform4i)
    }
}

static void OGLES2_glUniform4iv(struct OGLES2IFace *Self, GLint location, GLsizei count, const GLint * value)
{
    GET_CONTEXT

    logLine("%s: %s: location %d, count %d", context->name, __func__,
        location, count);

    for (GLsizei i = 0; i < count; i++) {
        const GLsizei index = 4 * i;
        logLine("v%d {%d, %d, %d, %d}", i, value[index], value[index + 1], value[index + 2], value[index + 3]);
    }

    if (context->old_glUniform4iv) {
        CHECK(context->old_glUniform4iv(Self, location, count, value), Uniform4iv)
    }
}

static void OGLES2_glUniformMatrix2fv(struct OGLES2IFace *Self, GLint location, GLsizei count, GLboolean transpose, const GLfloat * value)
{
    GET_CONTEXT

    logLine("%s: %s: location %d, count %d, transpose %d", context->name, __func__,
        location, count, transpose);

    for (GLsizei i = 0; i < count; i++) {
        const GLsizei index = 4 * i;
        logLine("matrix #%d {%f, %f, %f, %f}", i,
            value[index    ], value[index + 1],
            value[index + 2], value[index + 3]);
    }

    if (context->old_glUniformMatrix2fv) {
        CHECK(context->old_glUniformMatrix2fv(Self, location, count, transpose, value), UniformMatrix2fv)
    }
}

static void OGLES2_glUniformMatrix3fv(struct OGLES2IFace *Self, GLint location, GLsizei count, GLboolean transpose, const GLfloat * value)
{
    GET_CONTEXT

    logLine("%s: %s: location %d, count %d, transpose %d", context->name, __func__,
        location, count, transpose);

    for (GLsizei i = 0; i < count; i++) {
        const GLsizei index = 9 * i;
        logLine("matrix #%d {%f, %f, %f, %f, %f, %f, %f, %f, %f}", i,
            value[index    ], value[index + 1], value[index + 2],
            value[index + 3], value[index + 4], value[index + 5],
            value[index + 6], value[index + 7], value[index + 8]);
    }

    if (context->old_glUniformMatrix3fv) {
        CHECK(context->old_glUniformMatrix3fv(Self, location, count, transpose, value), UniformMatrix3fv)
    }
}

static void OGLES2_glUniformMatrix4fv(struct OGLES2IFace *Self, GLint location, GLsizei count, GLboolean transpose, const GLfloat * value)
{
    GET_CONTEXT

    logLine("%s: %s: location %d, count %d, transpose %d", context->name, __func__,
        location, count, transpose);

    for (GLsizei i = 0; i < count; i++) {
        const GLsizei index = 16 * i;
        logLine("matrix #%d {%f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f}", i,
            value[index     ], value[index +  1], value[index +  2], value[index +  3],
            value[index +  4], value[index +  5], value[index +  6], value[index +  7],
            value[index +  8], value[index +  9], value[index + 10], value[index + 11],
            value[index + 12], value[index + 13], value[index + 14], value[index + 15]);
    }

    if (context->old_glUniformMatrix4fv) {
        CHECK(context->old_glUniformMatrix4fv(Self, location, count, transpose, value), UniformMatrix4fv)
    }
}

static void OGLES2_glUseProgram(struct OGLES2IFace *Self, GLuint program)
{
    GET_CONTEXT

    logLine("%s: %s program %u", context->name, __func__, program);

    CHECK(context->old_glUseProgram(Self, program), UseProgram)
}

static void OGLES2_glVertexAttribPointer(struct OGLES2IFace *Self, GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void * pointer)
{
    GET_CONTEXT

    logLine("%s: %s: index %u, size %d, type %d, normalized %d, stride %d, pointer %p", context->name, __func__,
        index, size, type, normalized, stride, pointer);

    if (context->old_glVertexAttribPointer) {
        CHECK(context->old_glVertexAttribPointer(Self, index, size, type, normalized, stride, pointer), VertexAttribPointer)
    }
}


GENERATE_FILTERED_PATCH(OGLES2IFace, aglSwapBuffers, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glActiveTexture, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glAttachShader, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glBindAttribLocation, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glBindBuffer, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glBindFramebuffer, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glBindRenderbuffer, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glBindTexture, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glBufferData, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glBufferSubData, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glClear, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glCheckFramebufferStatus, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glCompileShader, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glDeleteBuffers, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glDeleteFramebuffers, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glDeleteTextures, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glDisable, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glDrawArrays, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glDrawElements, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glEnable, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glEnableVertexAttribArray, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glFinish, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glFlush, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glFramebufferRenderbuffer, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glFramebufferTexture2D, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glGenBuffers, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glGenerateMipmap, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glGenFramebuffers, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glGenTextures, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glGetFramebufferAttachmentParameteriv, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glShaderSource, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glTexImage2D, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glTexParameterf, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glTexParameterfv, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glTexParameteri, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glTexParameteriv, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glTexSubImage2D, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glUniform1f, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glUniform1fv, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glUniform1i, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glUniform1iv, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glUniform2f, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glUniform2fv, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glUniform2i, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glUniform2iv, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glUniform3f, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glUniform3fv, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glUniform3i, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glUniform3iv, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glUniform4f, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glUniform4fv, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glUniform4i, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glUniform4iv, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glUniformMatrix2fv, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glUniformMatrix3fv, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glUniformMatrix4fv, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glUseProgram, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glVertexAttribPointer, OGLES2, Ogles2Context)

static void (*patches[])(BOOL, struct Ogles2Context *) = {
    patch_aglSwapBuffers,
    patch_glActiveTexture,
    patch_glAttachShader,
    patch_glBindAttribLocation,
    patch_glBindBuffer,
    patch_glBindFramebuffer,
    patch_glBindRenderbuffer,
    patch_glBindTexture,
    patch_glBufferData,
    patch_glBufferSubData,
    patch_glCheckFramebufferStatus,
    patch_glClear,
    patch_glCompileShader,
    patch_glDeleteBuffers,
    patch_glDeleteFramebuffers,
    patch_glDeleteTextures,
    patch_glDisable,
    patch_glDrawArrays,
    patch_glDrawElements,
    patch_glEnable,
    patch_glEnableVertexAttribArray,
    patch_glFinish,
    patch_glFlush,
    patch_glFramebufferRenderbuffer,
    patch_glFramebufferTexture2D,
    patch_glGenBuffers,
    patch_glGenerateMipmap,
    patch_glGenFramebuffers,
    patch_glGenTextures,
    patch_glGetFramebufferAttachmentParameteriv,
    patch_glShaderSource,
    patch_glTexImage2D,
    patch_glTexParameterf,
    patch_glTexParameterfv,
    patch_glTexParameteri,
    patch_glTexParameteriv,
    patch_glTexSubImage2D,
    patch_glUniform1f,
    patch_glUniform1fv,
    patch_glUniform1i,
    patch_glUniform1iv,
    patch_glUniform2f,
    patch_glUniform2fv,
    patch_glUniform2i,
    patch_glUniform2iv,
    patch_glUniform3f,
    patch_glUniform3fv,
    patch_glUniform3i,
    patch_glUniform3iv,
    patch_glUniform4f,
    patch_glUniform4fv,
    patch_glUniform4i,
    patch_glUniform4iv,
    patch_glUniformMatrix2fv,
    patch_glUniformMatrix3fv,
    patch_glUniformMatrix4fv,
    patch_glUseProgram,
    patch_glVertexAttribPointer,
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
