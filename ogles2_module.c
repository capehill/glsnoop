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
    BlendColor,
    BlendEquation,
    BlendEquationSeparate,
    BlendFunc,
    BlendFuncSeparate,
    BufferData,
    BufferSubData,
    CheckFramebufferStatus,
    Clear,
    ClearColor,
    ClearDepthf,
    ClearStencil,
    ColorMask,
    CompileShader,
    CompressedTexImage2D,
    CompressedTexSubImage2D,
    CopyTexImage2D,
    CopyTexSubImage2D,
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
        MAP_ENUM(BlendColor)
        MAP_ENUM(BlendEquation)
        MAP_ENUM(BlendEquationSeparate)
        MAP_ENUM(BlendFunc)
        MAP_ENUM(BlendFuncSeparate)
        MAP_ENUM(BufferData)
        MAP_ENUM(BufferSubData)
        MAP_ENUM(CheckFramebufferStatus)
        MAP_ENUM(Clear)
        MAP_ENUM(ClearColor)
        MAP_ENUM(ClearDepthf)
        MAP_ENUM(ClearStencil)
        MAP_ENUM(ColorMask)
        MAP_ENUM(CompileShader)
        MAP_ENUM(CompressedTexImage2D)
        MAP_ENUM(CompressedTexSubImage2D)
        MAP_ENUM(CopyTexImage2D)
        MAP_ENUM(CopyTexSubImage2D)
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
    void (*old_glBlendColor)(struct OGLES2IFace *Self, GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
    void (*old_glBlendEquation)(struct OGLES2IFace *Self, GLenum mode);
    void (*old_glBlendEquationSeparate)(struct OGLES2IFace *Self, GLenum modeRGB, GLenum modeAlpha);
    void (*old_glBlendFunc)(struct OGLES2IFace *Self, GLenum sfactor, GLenum dfactor);
    void (*old_glBlendFuncSeparate)(struct OGLES2IFace *Self, GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha);
    void (*old_glBufferData)(struct OGLES2IFace *Self, GLenum target, GLsizeiptr size, const void * data, GLenum usage);
    void (*old_glBufferSubData)(struct OGLES2IFace *Self, GLenum target, GLintptr offset, GLsizeiptr size, const void * data);
    GLenum (*old_glCheckFramebufferStatus)(struct OGLES2IFace *Self, GLenum target);
    void (*old_glClear)(struct OGLES2IFace *Self, GLbitfield mask);
    void (*old_glClearColor)(struct OGLES2IFace *Self, GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
    void (*old_glClearDepthf)(struct OGLES2IFace *Self, GLfloat d);
    void (*old_glClearStencil)(struct OGLES2IFace *Self, GLint s);
    void (*old_glColorMask)(struct OGLES2IFace *Self, GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
    void (*old_glCompileShader)(struct OGLES2IFace *Self, GLuint shader);
    void (*old_glCompressedTexImage2D)(struct OGLES2IFace *Self, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void * data);
    void (*old_glCompressedTexSubImage2D)(struct OGLES2IFace *Self, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void * data);
    void (*old_glCopyTexImage2D)(struct OGLES2IFace *Self, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
    void (*old_glCopyTexSubImage2D)(struct OGLES2IFace *Self, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
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

#define GL_CALL(id, ...) \
if (context->old_gl ## id) { \
    PROF_START \
    context->old_gl ## id(Self, ##__VA_ARGS__); \
    PROF_FINISH(id) \
} \
checkErrors(context, id);

#define GL_CALL_STATUS(id, ...) \
if (context->old_gl ## id) { \
    PROF_START \
    status = context->old_gl ## id(Self, ##__VA_ARGS__); \
    PROF_FINISH(id) \
} \
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

    GL_CALL(ActiveTexture, texture)
}

static void OGLES2_glAttachShader(struct OGLES2IFace *Self, GLuint program, GLuint shader)
{
    GET_CONTEXT

    logLine("%s: %s: program %u, shader %u", context->name, __func__,
        program, shader);

    GL_CALL(AttachShader, program, shader)
}

static void OGLES2_glBindAttribLocation(struct OGLES2IFace *Self, GLuint program, GLuint index, const GLchar * name)
{
    GET_CONTEXT

    logLine("%s: %s: program %u, index %u, name '%s'", context->name, __func__,
        program, index, name);

    GL_CALL(BindAttribLocation, program, index, name)
}

static void OGLES2_glBindBuffer(struct OGLES2IFace *Self, GLenum target, GLuint buffer)
{
    GET_CONTEXT

    logLine("%s: %s: target %d, buffer %u", context->name, __func__,
        target, buffer);

    GL_CALL(BindBuffer, target, buffer)
}

static void OGLES2_glBindFramebuffer(struct OGLES2IFace *Self, GLenum target, GLuint framebuffer)
{
    GET_CONTEXT

    logLine("%s: %s: target %u, framebuffer %u", context->name, __func__,
        target, framebuffer);

    GL_CALL(BindFramebuffer, target, framebuffer)
}

static void OGLES2_glBindRenderbuffer(struct OGLES2IFace *Self, GLenum target, GLuint renderbuffer)
{
    GET_CONTEXT

    logLine("%s: %s: target %u, renderbuffer %u", context->name, __func__,
        target, renderbuffer);

    GL_CALL(BindRenderbuffer, target, renderbuffer)
}

static void OGLES2_glBindTexture(struct OGLES2IFace *Self, GLenum target, GLuint texture)
{
    GET_CONTEXT

    logLine("%s: %s: target %d, texture %d", context->name, __func__,
        target, texture);

    GL_CALL(BindTexture, target, texture)
}

static void OGLES2_glBlendColor(struct OGLES2IFace *Self, GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
    GET_CONTEXT

    logLine("%s: %s: red %f, green %f, blue %f, alpha %f", context->name, __func__,
        red, green, blue, alpha);

    GL_CALL(BlendColor, red, green, blue, alpha)
}

static void OGLES2_glBlendEquation(struct OGLES2IFace *Self, GLenum mode)
{
    GET_CONTEXT

    logLine("%s: %s: mode %d", context->name, __func__,
        mode);

    GL_CALL(BlendEquation, mode)
}

static void OGLES2_glBlendEquationSeparate(struct OGLES2IFace *Self, GLenum modeRGB, GLenum modeAlpha)
{
    GET_CONTEXT

    logLine("%s: %s: modeRGB %d, modeAlpha %d", context->name, __func__,
        modeRGB, modeAlpha);

    GL_CALL(BlendEquationSeparate, modeRGB, modeAlpha)
}

static void OGLES2_glBlendFunc(struct OGLES2IFace *Self, GLenum sfactor, GLenum dfactor)
{
    GET_CONTEXT

    logLine("%s: %s: sfactor %d, dfactor %d", context->name, __func__,
        sfactor, dfactor);

    GL_CALL(BlendFunc, sfactor, dfactor)
}

static void OGLES2_glBlendFuncSeparate(struct OGLES2IFace *Self, GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha)
{
    GET_CONTEXT

    logLine("%s: %s: sfactorRGB %d, dfactorRGB %d, sfactorAlpha %d, dfactorAlpha %d", context->name, __func__,
        sfactorRGB, dfactorRGB, sfactorAlpha, dfactorAlpha);

    GL_CALL(BlendFuncSeparate, sfactorRGB, dfactorRGB, sfactorAlpha, dfactorAlpha)
}

static void OGLES2_glBufferData(struct OGLES2IFace *Self, GLenum target, GLsizeiptr size, const void * data, GLenum usage)
{
    GET_CONTEXT

    logLine("%s: %s: target %d, size %u, data %p, usage %d", context->name, __func__,
        target, size, data, usage);

    GL_CALL(BufferData, target, size, data, usage)
}

static void OGLES2_glBufferSubData(struct OGLES2IFace *Self, GLenum target, GLintptr offset, GLsizeiptr size, const void * data)
{
    GET_CONTEXT

    logLine("%s: %s: target %d, offset %u, size %u, data %p", context->name, __func__,
        target, offset, size, data);

    GL_CALL(BufferSubData, target, offset, size, data)
}

static GLenum OGLES2_glCheckFramebufferStatus(struct OGLES2IFace *Self, GLenum target)
{
    GET_CONTEXT

    GLenum status = 0;

    GL_CALL_STATUS(CheckFramebufferStatus, target)

    logLine("%s: %s: status %u", context->name, __func__,
        status);

    return status;
}

static void OGLES2_glClear(struct OGLES2IFace *Self, GLbitfield mask)
{
    GET_CONTEXT

    logLine("%s: %s: mask 0x%X", context->name, __func__, mask);

    GL_CALL(Clear, mask)
}

static void OGLES2_glClearColor(struct OGLES2IFace *Self, GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
    GET_CONTEXT

    logLine("%s: %s: red %f, green %f, blue %f, alpha %f", context->name, __func__,
        red, green, blue, alpha);

    GL_CALL(ClearColor, red, green, blue, alpha)
}

static void OGLES2_glClearDepthf(struct OGLES2IFace *Self, GLfloat d)
{
    GET_CONTEXT

    logLine("%s: %s: d %f", context->name, __func__,
        d);

    GL_CALL(ClearDepthf, d)
}

static void OGLES2_glClearStencil(struct OGLES2IFace *Self, GLint s)
{
    GET_CONTEXT

    logLine("%s: %s: s %d", context->name, __func__,
        s);

    GL_CALL(ClearStencil, s)
}

static void OGLES2_glColorMask(struct OGLES2IFace *Self, GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
{
    GET_CONTEXT

    logLine("%s: %s: red %d, green %d, blue %d, alpha %d", context->name, __func__,
        red, green, blue, alpha);

    GL_CALL(ColorMask, red, green, blue, alpha)
}

static void OGLES2_glCompileShader(struct OGLES2IFace *Self, GLuint shader)
{
    GET_CONTEXT

    logLine("%s: %s: shader %u", context->name, __func__,
        shader);

    GL_CALL(CompileShader, shader)
}

static void OGLES2_glCompressedTexImage2D(struct OGLES2IFace *Self, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void * data)
{
    GET_CONTEXT

    logLine("%s: %s: target %u, level %d, internalformat %u, width %d, height %d, border %d, imageSize %d, data %p", context->name, __func__,
        target, level, internalformat, width, height, border, imageSize, data);

    GL_CALL(CompressedTexImage2D, target, level, internalformat, width, height, border, imageSize, data)
}

static void OGLES2_glCompressedTexSubImage2D(struct OGLES2IFace *Self, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void * data)
{
    GET_CONTEXT

    logLine("%s: %s: target %u, level %d, xoffset %d, yoffset %d, width %d, height %d, format %u, imageSize %d, data %p", context->name, __func__,
        target, level, xoffset, yoffset, width, height, format, imageSize, data);

    GL_CALL(CompressedTexSubImage2D, target, level, xoffset, yoffset, width, height, format, imageSize, data)
}

static void OGLES2_glCopyTexImage2D(struct OGLES2IFace *Self, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border)
{
    GET_CONTEXT

    logLine("%s: %s: target %u, level %d, internalformat %u, x %d, y %d, width %d, height %d, border %d", context->name, __func__,
        target, level, internalformat, x, y, width, height, border);

    GL_CALL(CopyTexImage2D, target, level, internalformat, x, y, width, height, border)
}

static void OGLES2_glCopyTexSubImage2D(struct OGLES2IFace *Self, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
    GET_CONTEXT

    logLine("%s: %s: target %u, level %d, xoffset %d, yoffset %d, x %d, y %d, width %d, height %d", context->name, __func__,
        target, level, xoffset, yoffset, x, y, width, height);

    GL_CALL(CopyTexSubImage2D, target, level, xoffset, yoffset, x, y, width, height)
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

    GL_CALL(DeleteBuffers, n, buffers)
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

    GL_CALL(DeleteFramebuffers, n, framebuffers)
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

    GL_CALL(DeleteTextures, n, textures)
}

static void OGLES2_glDisable(struct OGLES2IFace *Self, GLenum cap)
{
    GET_CONTEXT

    logLine("%s: %s: cap %d", context->name, __func__,
        cap);

    GL_CALL(Disable, cap)
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

    GL_CALL(DrawArrays, mode, first, count)

    countPrimitive(&context->counter, mode, count);
}

static void OGLES2_glDrawElements(struct OGLES2IFace *Self, GLenum mode, GLsizei count, GLenum type, const void * indices)
{
    GET_CONTEXT

    logLine("%s: %s: mode %d, count %d, type %d, indices %p", context->name, __func__,
        mode, count, type, indices);

    GL_CALL(DrawElements, mode, count, type, indices)

    countPrimitive(&context->counter, mode, count);
}

static void OGLES2_glEnable(struct OGLES2IFace *Self, GLenum cap)
{
    GET_CONTEXT

    logLine("%s: %s: cap %d", context->name, __func__,
        cap);

    GL_CALL(Enable, cap)
}

static void OGLES2_glEnableVertexAttribArray(struct OGLES2IFace *Self, GLuint index)
{
    GET_CONTEXT

    logLine("%s: %s: index %u", context->name, __func__,
        index);

    GL_CALL(EnableVertexAttribArray, index)
}

static void OGLES2_glFinish(struct OGLES2IFace *Self)
{
    GET_CONTEXT

    logLine("%s: %s", context->name, __func__);

    GL_CALL(Finish)
}

static void OGLES2_glFlush(struct OGLES2IFace *Self)
{
    GET_CONTEXT

    logLine("%s: %s", context->name, __func__);

    GL_CALL(Flush)
}

static void OGLES2_glFramebufferRenderbuffer(struct OGLES2IFace *Self, GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)
{
    GET_CONTEXT

    logLine("%s: %s: target %u, attachment %u, renderbuffertarget %u, renderbuffer %u", context->name, __func__,
        target, attachment, renderbuffertarget, renderbuffer);

    GL_CALL(FramebufferRenderbuffer, target, attachment, renderbuffertarget, renderbuffer)
}

static void OGLES2_glFramebufferTexture2D(struct OGLES2IFace *Self, GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
    GET_CONTEXT

    logLine("%s: %s: target %u, attachment %u, textarget %u, texture %u, level %d", context->name, __func__,
        target, attachment, textarget, texture, level);

    GL_CALL(FramebufferTexture2D, target, attachment, textarget, texture, level)
}

static void OGLES2_glGenBuffers(struct OGLES2IFace *Self, GLsizei n, GLuint * buffers)
{
    GET_CONTEXT

    logLine("%s: %s: n %d, buffers %p", context->name, __func__,
        n, buffers);

    GL_CALL(GenBuffers, n, buffers)

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

    GL_CALL(GenerateMipmap, target)
}

static void OGLES2_glGenFramebuffers(struct OGLES2IFace *Self, GLsizei n, GLuint * framebuffers)
{
    GET_CONTEXT

    logLine("%s: %s: n %u, framebuffers %p", context->name, __func__,
        n, framebuffers);

    GL_CALL(GenFramebuffers, n, framebuffers)

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

    GL_CALL(GenTextures, n, textures)

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

    GL_CALL(GetFramebufferAttachmentParameteriv, target, attachment, pname, params)
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

    GL_CALL(ShaderSource, shader, count, string, length)
}

static void OGLES2_glTexImage2D(struct OGLES2IFace *Self, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void * pixels)
{
    GET_CONTEXT

    logLine("%s: %s: target %d, level %d, internalformat %d, width %u, height %u, border %d, format %d, type %d, pixels %p", context->name, __func__,
        target, level, internalformat, width, height, border, format, type, pixels);

    GL_CALL(TexImage2D, target,  level, internalformat, width, height, border, format, type, pixels)
}

static void OGLES2_glTexParameterf(struct OGLES2IFace *Self, GLenum target, GLenum pname, GLfloat param)
{
    GET_CONTEXT

    logLine("%s: %s: target %d, pname %d, param %f", context->name, __func__,
        target, pname, param);

    GL_CALL(TexParameterf, target, pname, param)
}

static void OGLES2_glTexParameterfv(struct OGLES2IFace *Self, GLenum target, GLenum pname, const GLfloat * params)
{
    GET_CONTEXT

    logLine("%s: %s: target %d, pname %d, params %p", context->name, __func__,
        target, pname, params);

    GL_CALL(TexParameterfv, target, pname, params)
}

static void OGLES2_glTexParameteri(struct OGLES2IFace *Self, GLenum target, GLenum pname, GLint param)
{
    GET_CONTEXT

    logLine("%s: %s: target %d, pname %d, param %d", context->name, __func__,
        target, pname, param);

    GL_CALL(TexParameteri, target, pname, param)
}

static void OGLES2_glTexParameteriv(struct OGLES2IFace *Self, GLenum target, GLenum pname, const GLint * params)
{
    GET_CONTEXT

    logLine("%s: %s: target %d, pname %d, params %p", context->name, __func__,
        target, pname, params);

    GL_CALL(TexParameteriv, target, pname, params)
}

static void OGLES2_glTexSubImage2D(struct OGLES2IFace *Self, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void * pixels)
{
    GET_CONTEXT

    logLine("%s: %s: target %d, level %d, xoffset %d, yoffset %d, width %u, height %u, format %d, type %d, pixels %p", context->name, __func__,
        target, level, xoffset, yoffset, width, height, format, type, pixels);

    GL_CALL(TexSubImage2D, target, level, xoffset, yoffset, width, height, format, type, pixels)
}

static void OGLES2_glUniform1f(struct OGLES2IFace *Self, GLint location, GLfloat v0)
{
    GET_CONTEXT

    logLine("%s: %s: location %d, v0 %f", context->name, __func__,
        location, v0);

    GL_CALL(Uniform1f, location, v0)
}

static void OGLES2_glUniform1fv(struct OGLES2IFace *Self, GLint location, GLsizei count, const GLfloat * value)
{
    GET_CONTEXT

    logLine("%s: %s: location %d, count %d", context->name, __func__,
        location, count);

    for (GLsizei i = 0; i < count; i++) {
        logLine("v%d { %f }", i, value[i]);
    }

    GL_CALL(Uniform1fv, location, count, value)
}

static void OGLES2_glUniform1i(struct OGLES2IFace *Self, GLint location, GLint v0)
{
    GET_CONTEXT

    logLine("%s: %s: location %d, v0 %d", context->name, __func__,
        location, v0);

    GL_CALL(Uniform1i, location, v0)
}

static void OGLES2_glUniform1iv(struct OGLES2IFace *Self, GLint location, GLsizei count, const GLint * value)
{
    GET_CONTEXT

    logLine("%s: %s: location %d, count %d", context->name, __func__,
        location, count);

    for (GLsizei i = 0; i < count; i++) {
        logLine("v%d { %d }", i, value[i]);
    }

    GL_CALL(Uniform1iv, location, count, value)
}

static void OGLES2_glUniform2f(struct OGLES2IFace *Self, GLint location, GLfloat v0, GLfloat v1)
{
    GET_CONTEXT

    logLine("%s: %s: location %d, v0 %f, v1 %f", context->name, __func__,
        location, v0, v1);

    GL_CALL(Uniform2f, location, v0, v1)
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

    GL_CALL(Uniform2fv, location, count, value)
}

static void OGLES2_glUniform2i(struct OGLES2IFace *Self, GLint location, GLint v0, GLint v1)
{
    GET_CONTEXT

    logLine("%s: %s: location %d, v0 %d, v1 %d", context->name, __func__,
        location, v0, v1);

    GL_CALL(Uniform2i, location, v0, v1)
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

    GL_CALL(Uniform2iv, location, count, value)
}

static void OGLES2_glUniform3f(struct OGLES2IFace *Self, GLint location, GLfloat v0, GLfloat v1, GLfloat v2)
{
    GET_CONTEXT

    logLine("%s: %s: location %d, v0 %f, v1 %f, v2 %f", context->name, __func__,
        location, v0, v1, v2);

    GL_CALL(Uniform3f, location, v0, v1, v2)
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

    GL_CALL(Uniform3fv, location, count, value)
}

static void OGLES2_glUniform3i(struct OGLES2IFace *Self, GLint location, GLint v0, GLint v1, GLint v2)
{
    GET_CONTEXT

    logLine("%s: %s: location %d, v0 %d, v1 %d, v2 %d", context->name, __func__,
        location, v0, v1, v2);

    GL_CALL(Uniform3i, location, v0, v1, v2)
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

    GL_CALL(Uniform3iv, location, count, value)
}

static void OGLES2_glUniform4f(struct OGLES2IFace *Self, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)
{
    GET_CONTEXT

    logLine("%s: %s: location %d, v0 %f, v1 %f, v2 %f, v3 %f", context->name, __func__,
        location, v0, v1, v2, v3);

    GL_CALL(Uniform4f, location, v0, v1, v2, v3)
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

    GL_CALL(Uniform4fv, location, count, value)
}

static void OGLES2_glUniform4i(struct OGLES2IFace *Self, GLint location, GLint v0, GLint v1, GLint v2, GLint v3)
{
    GET_CONTEXT

    logLine("%s: %s: location %d, v0 %d, v1 %d, v2 %d, v3 %d", context->name, __func__,
        location, v0, v1, v2, v3);

    GL_CALL(Uniform4i, location, v0, v1, v2, v3)
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

    GL_CALL(Uniform4iv, location, count, value)
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

    GL_CALL(UniformMatrix2fv, location, count, transpose, value)
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

    GL_CALL(UniformMatrix3fv, location, count, transpose, value)
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

    GL_CALL(UniformMatrix4fv, location, count, transpose, value)
}

static void OGLES2_glUseProgram(struct OGLES2IFace *Self, GLuint program)
{
    GET_CONTEXT

    logLine("%s: %s program %u", context->name, __func__, program);

    GL_CALL(UseProgram, program);
}

static void OGLES2_glVertexAttribPointer(struct OGLES2IFace *Self, GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void * pointer)
{
    GET_CONTEXT

    logLine("%s: %s: index %u, size %d, type %d, normalized %d, stride %d, pointer %p", context->name, __func__,
        index, size, type, normalized, stride, pointer);

    GL_CALL(VertexAttribPointer, index, size, type, normalized, stride, pointer)
}


GENERATE_FILTERED_PATCH(OGLES2IFace, aglSwapBuffers, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glActiveTexture, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glAttachShader, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glBindAttribLocation, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glBindBuffer, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glBindFramebuffer, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glBindRenderbuffer, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glBindTexture, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glBlendColor, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glBlendEquation, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glBlendEquationSeparate, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glBlendFunc, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glBlendFuncSeparate, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glBufferData, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glBufferSubData, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glClear, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glClearColor, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glClearDepthf, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glClearStencil, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glCheckFramebufferStatus, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glColorMask, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glCompileShader, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glCompressedTexImage2D, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glCompressedTexSubImage2D, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glCopyTexImage2D, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glCopyTexSubImage2D, OGLES2, Ogles2Context)
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
    patch_glBlendColor,
    patch_glBlendEquation,
    patch_glBlendEquationSeparate,
    patch_glBlendFunc,
    patch_glBlendFuncSeparate,
    patch_glBufferData,
    patch_glBufferSubData,
    patch_glCheckFramebufferStatus,
    patch_glClear,
    patch_glClearColor,
    patch_glClearDepthf,
    patch_glClearStencil,
    patch_glColorMask,
    patch_glCompileShader,
    patch_glCompressedTexImage2D,
    patch_glCompressedTexSubImage2D,
    patch_glCopyTexImage2D,
    patch_glCopyTexSubImage2D,
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
