#include "ogles2_module.h"
#include "common.h"

#include <proto/exec.h>
#include <proto/ogles2.h>
#include <proto/dos.h>

#include <stdio.h>
#include <string.h>

struct Library* OGLES2Base;

// Store original function pointers so that they can be still called
struct Ogles2Context
{
    struct OGLES2IFace* interface;
    struct Task* task;
    char name[NAME_LEN];

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
};

static struct Ogles2Context* contexts[MAX_CLIENTS];
static APTR mutex;

static void patch_ogles2_functions(struct Ogles2Context *);

static void find_process_name(struct Ogles2Context * context)
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

static BOOL open_ogles2_library(void)
{
    OGLES2Base = IExec->OpenLibrary("ogles2.library", 0);
    if (OGLES2Base) {
        return TRUE;
    } else {
        printf("Failed to open ogles2.library\n");
    }

    return FALSE;
}

static void close_ogles2_library(void)
{
    if (OGLES2Base) {
        IExec->CloseLibrary(OGLES2Base);
        OGLES2Base = NULL;
    }
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
                } else {
                    patch_ogles2_functions(context);
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
            logLine("%s: dropping patched interface %p", contexts[i]->name, interface);

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
        logLine("%s: GL error %d detected %s call", context->name, err, info);
    }
}

#define PRE_CHECK check_errors("before", context);
#define POST_CHECK check_errors("after", context);

#define CHECK(x) \
PRE_CHECK \
x; \
POST_CHECK

// Wrap traced function calls

static void OGLES2_aglSwapBuffers(struct OGLES2IFace *Self)
{
    GET_CONTEXT

    logLine("%s: %s", context->name, __func__);

    if (context->old_aglSwapBuffers) {
        CHECK(context->old_aglSwapBuffers(Self))
    }
}

static void OGLES2_glCompileShader(struct OGLES2IFace *Self, GLuint shader)
{
    GET_CONTEXT

    logLine("%s: %s: shader %u", context->name, __func__,
        shader);

    if (context->old_glCompileShader) {
        CHECK(context->old_glCompileShader(Self, shader))
    }
}

static void OGLES2_glGenBuffers(struct OGLES2IFace *Self, GLsizei n, GLuint * buffers)
{
    GET_CONTEXT

    logLine("%s: %s: n %d, buffers %p", context->name, __func__,
        n, buffers);

    if (context->old_glGenBuffers) {
        CHECK(context->old_glGenBuffers(Self, n, buffers))
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
        CHECK(context->old_glBindBuffer(Self, target, buffer))
    }
}

static void OGLES2_glBufferData(struct OGLES2IFace *Self, GLenum target, GLsizeiptr size, const void * data, GLenum usage)
{
    GET_CONTEXT

    logLine("%s: %s: target %d, size %u, data %p, usage %d", context->name, __func__,
        target, size, data, usage);

    if (context->old_glBufferData) {
        CHECK(context->old_glBufferData(Self, target, size, data, usage))
    }
}

static void OGLES2_glBufferSubData(struct OGLES2IFace *Self, GLenum target, GLintptr offset, GLsizeiptr size, const void * data)
{
    GET_CONTEXT

    logLine("%s: %s: target %d, offset %u, size %u, data %p", context->name, __func__,
        target, offset, size, data);

    if (context->old_glBufferSubData) {
        CHECK(context->old_glBufferSubData(Self, target, offset, size, data))
    }
}

static void OGLES2_glDeleteBuffers(struct OGLES2IFace *Self, GLsizei n, GLuint * buffers)
{
    GET_CONTEXT

    logLine("%s: %s: n %d, buffers %p", context->name, __func__,
         n, buffers);

    size_t i;
    for (i = 0; i < n; i++) {
        logLine("Deleting buffer[%u] = %u", i, buffers[i]);
    }

    if (context->old_glDeleteBuffers) {
        CHECK(context->old_glDeleteBuffers(Self, n, buffers))
    }
}

static void OGLES2_glEnableVertexAttribArray(struct OGLES2IFace *Self, GLuint index)
{
    GET_CONTEXT

    logLine("%s: %s: index %u", context->name, __func__,
        index);

    if (context->old_glEnableVertexAttribArray) {
        CHECK(context->old_glEnableVertexAttribArray(Self, index))
    }
}

static void OGLES2_glVertexAttribPointer(struct OGLES2IFace *Self, GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void * pointer)
{
    GET_CONTEXT

    logLine("%s: %s: index %u, size %d, type %d, normalized %d, stride %d, pointer %p", context->name, __func__,
        index, size, type, normalized, stride, pointer);

    if (context->old_glVertexAttribPointer) {
        CHECK(context->old_glVertexAttribPointer(Self, index, size, type, normalized, stride, pointer))
    }
}

static void OGLES2_glDrawArrays(struct OGLES2IFace *Self, GLenum mode, GLint first, GLsizei count)
{
    GET_CONTEXT

    logLine("%s: %s: mode %d, first %d, count %d", context->name, __func__,
        mode, first, count);

    if (context->old_glDrawArrays) {
        CHECK(context->old_glDrawArrays(Self, mode, first, count))
    }
}

static void OGLES2_glDrawElements(struct OGLES2IFace *Self, GLenum mode, GLsizei count, GLenum type, const void * indices)
{
    GET_CONTEXT

    logLine("%s: %s: mode %d, count %d, type %d, indices %p", context->name, __func__,
        mode, count, type, indices);

    if (context->old_glDrawElements) {
        CHECK(context->old_glDrawElements(Self, mode, count, type, indices))
    }
}

static void OGLES2_glShaderSource(struct OGLES2IFace *Self, GLuint shader, GLsizei count, const GLchar *const* string, const GLint * length)
{
    GET_CONTEXT

    logLine("%s: %s: shader %u, count %u, string %p length %p", context->name, __func__,
        shader, count, string, length);

    size_t i;

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
        CHECK(context->old_glShaderSource(Self, shader, count, string, length))
    }
}

static void OGLES2_glActiveTexture(struct OGLES2IFace *Self, GLenum texture)
{
    GET_CONTEXT

    logLine("%s: %s: texture %d", context->name, __func__,
        texture);

    if (context->old_glActiveTexture) {
        CHECK(context->old_glActiveTexture(Self, texture))
    }
}

static void OGLES2_glBindTexture(struct OGLES2IFace *Self, GLenum target, GLuint texture)
{
    GET_CONTEXT

    logLine("%s: %s: target %d, texture %d", context->name, __func__,
        target, texture);

    if (context->old_glBindTexture) {
        CHECK(context->old_glBindTexture(Self, target, texture))
    }
}

static void OGLES2_glGenTextures(struct OGLES2IFace *Self, GLsizei n, GLuint * textures)
{
    GET_CONTEXT

    logLine("%s: %s: n %d, textures %p", context->name, __func__,
        n, textures);

    if (context->old_glGenTextures) {
        CHECK(context->old_glGenTextures(Self, n, textures))
    }

    size_t i;
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
        CHECK(context->old_glGenerateMipmap(Self, target))
    }
}

static void OGLES2_glTexParameterf(struct OGLES2IFace *Self, GLenum target, GLenum pname, GLfloat param)
{
    GET_CONTEXT

    logLine("%s: %s: target %d, pname %d, param %f", context->name, __func__,
        target, pname, param);

    if (context->old_glTexParameterf) {
        CHECK(context->old_glTexParameterf(Self, target, pname, param))
    }
}

static void OGLES2_glTexParameterfv(struct OGLES2IFace *Self, GLenum target, GLenum pname, const GLfloat * params)
{
    GET_CONTEXT

    logLine("%s: %s: target %d, pname %d, params %p", context->name, __func__,
        target, pname, params);

    if (context->old_glTexParameterfv) {
        CHECK(context->old_glTexParameterfv(Self, target, pname, params))
    }
}

static void OGLES2_glTexParameteri(struct OGLES2IFace *Self, GLenum target, GLenum pname, GLint param)
{
    GET_CONTEXT

    logLine("%s: %s: target %d, pname %d, param %d", context->name, __func__,
        target, pname, param);

    if (context->old_glTexParameteri) {
        CHECK(context->old_glTexParameteri(Self, target, pname, param))
    }
}

static void OGLES2_glTexParameteriv(struct OGLES2IFace *Self, GLenum target, GLenum pname, const GLint * params)
{
    GET_CONTEXT

    logLine("%s: %s: target %d, pname %d, params %p", context->name, __func__,
        target, pname, params);

    if (context->old_glTexParameteriv) {
        CHECK(context->old_glTexParameteriv(Self, target, pname, params))
    }
}

static void OGLES2_glTexSubImage2D(struct OGLES2IFace *Self, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void * pixels)
{
    GET_CONTEXT

    logLine("%s: %s: target %d, level %d, xoffset %d, yoffset %d, width %u, height %u, format %d, type %d, pixels %p", context->name, __func__,
        target, level, xoffset, yoffset, width, height, format, type, pixels);

    if (context->old_glTexSubImage2D) {
        CHECK(context->old_glTexSubImage2D(Self, target, level, xoffset, yoffset, width, height, format, type, pixels))
    }
}

static void OGLES2_glTexImage2D(struct OGLES2IFace *Self, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void * pixels)
{
    GET_CONTEXT

    logLine("%s: %s: target %d, level %d, internalformat %d, width %u, height %u, border %d, format %d, type %d, pixels %p", context->name, __func__,
        target, level, internalformat, width, height, border, format, type, pixels);

    if (context->old_glTexImage2D) {
        CHECK(context->old_glTexImage2D(Self, target, level, internalformat, width, height, border, format, type, pixels))
    }
}

static void OGLES2_glDeleteTextures(struct OGLES2IFace *Self, GLsizei n, const GLuint * textures)
{
    GET_CONTEXT

    logLine("%s: %s: n %u, textures %p", context->name, __func__,
        n, textures);

    size_t i;
    for (i = 0; i < n; i++) {
        logLine("Deleting texture[%u] = %u", i, textures[i]);
    }

    if (context->old_glDeleteTextures) {
        CHECK(context->old_glDeleteTextures(Self, n, textures))
    }
}

GENERATE_PATCH(OGLES2IFace, aglSwapBuffers, OGLES2, Ogles2Context)
GENERATE_PATCH(OGLES2IFace, glCompileShader, OGLES2, Ogles2Context)
GENERATE_PATCH(OGLES2IFace, glGenBuffers, OGLES2, Ogles2Context)
GENERATE_PATCH(OGLES2IFace, glBindBuffer, OGLES2, Ogles2Context)
GENERATE_PATCH(OGLES2IFace, glBufferData, OGLES2, Ogles2Context)
GENERATE_PATCH(OGLES2IFace, glBufferSubData, OGLES2, Ogles2Context)
GENERATE_PATCH(OGLES2IFace, glDeleteBuffers, OGLES2, Ogles2Context)
GENERATE_PATCH(OGLES2IFace, glEnableVertexAttribArray, OGLES2, Ogles2Context)
GENERATE_PATCH(OGLES2IFace, glVertexAttribPointer, OGLES2, Ogles2Context)
GENERATE_PATCH(OGLES2IFace, glDrawArrays, OGLES2, Ogles2Context)
GENERATE_PATCH(OGLES2IFace, glDrawElements, OGLES2, Ogles2Context)
GENERATE_PATCH(OGLES2IFace, glShaderSource, OGLES2, Ogles2Context)
GENERATE_PATCH(OGLES2IFace, glActiveTexture, OGLES2, Ogles2Context)
GENERATE_PATCH(OGLES2IFace, glBindTexture, OGLES2, Ogles2Context)
GENERATE_PATCH(OGLES2IFace, glGenTextures, OGLES2, Ogles2Context)
GENERATE_PATCH(OGLES2IFace, glGenerateMipmap, OGLES2, Ogles2Context)
GENERATE_PATCH(OGLES2IFace, glTexParameterf, OGLES2, Ogles2Context)
GENERATE_PATCH(OGLES2IFace, glTexParameterfv, OGLES2, Ogles2Context)
GENERATE_PATCH(OGLES2IFace, glTexParameteri, OGLES2, Ogles2Context)
GENERATE_PATCH(OGLES2IFace, glTexParameteriv, OGLES2, Ogles2Context)
GENERATE_PATCH(OGLES2IFace, glTexSubImage2D, OGLES2, Ogles2Context)
GENERATE_PATCH(OGLES2IFace, glTexImage2D, OGLES2, Ogles2Context)
GENERATE_PATCH(OGLES2IFace, glDeleteTextures, OGLES2, Ogles2Context)

static void (*patches[])(BOOL, struct Ogles2Context *) = {
    //patch_aglSwapBuffers,
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
    NULL
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
            if (patches[i]) {
                patches[i](TRUE, ctx);
            }
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
                size_t p;
                for (p = 0; p < sizeof(patches) / sizeof(patches[0]); p++) {
                    if (patches[p]) {
                        patches[p](FALSE, contexts[i]);
                    }
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
