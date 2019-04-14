#include "ogles2_module.h"
#include "common.h"

#include <proto/exec.h>
#include <proto/ogles2.h>

#include <stdio.h>

struct Library* OGLES2Base;

static void patch_ogles2_functions(struct Interface* interface);
static struct Interface* patchedInterface; // TODO FIXME: can only support one OGLES2 application the moment

static STRPTR task_name()
{
    return (((struct Node *)IExec->FindTask(NULL))->ln_Name);
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

static struct Interface* (*old_GetInterface)(struct ExecIFace* Self, struct Library *, STRPTR, ULONG, struct TagItem *);

static struct Interface* EXEC_GetInterface(struct ExecIFace* Self, struct Library * library, STRPTR name, ULONG version, struct TagItem *ti)
{
    struct Interface* interface = NULL;

    if (old_GetInterface) {
        interface = old_GetInterface(Self, library, name, version, ti);

        if (library == OGLES2Base) {
            patch_ogles2_functions(interface);
        }
    }

    return interface;
}

PATCH_INTERFACE(ExecIFace, GetInterface, EXEC)

// Store original function pointers so that they can be still called

static void (*old_aglSwapBuffers)(struct OGLES2IFace *Self);
static void (*old_glCompileShader)(struct OGLES2IFace *Self, GLuint shader);

static void (*old_glGenBuffers)(struct OGLES2IFace *Self, GLsizei n, GLuint * buffers);
static void (*old_glBindBuffer)(struct OGLES2IFace *Self, GLenum target, GLuint buffer);
static void (*old_glBufferData)(struct OGLES2IFace *Self, GLenum target, GLsizeiptr size, const void * data, GLenum usage);
static void (*old_glBufferSubData)(struct OGLES2IFace *Self, GLenum target, GLintptr offset, GLsizeiptr size, const void * data);
static void (*old_glDeleteBuffers)(struct OGLES2IFace *Self, GLsizei n, GLuint * buffers);

static void (*old_glDrawElements)(struct OGLES2IFace *Self, GLenum mode, GLsizei count, GLenum type, const void * indices);
static void (*old_glDrawArrays)(struct OGLES2IFace *Self, GLenum mode, GLint first, GLsizei count);

static void (*old_glVertexAttribPointer)(struct OGLES2IFace *Self, GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void * pointer);
static void (*old_glEnableVertexAttribArray)(struct OGLES2IFace *Self, GLuint index);
//static void (*old_glVertexAttrib3fv)(struct OGLES2IFace *Self, GLuint index, const GLfloat * v);

static void (*old_glShaderSource)(struct OGLES2IFace *Self, GLuint shader, GLsizei count, const GLchar *const* string, const GLint * length);

static void (*old_glActiveTexture)(struct OGLES2IFace *Self, GLenum texture);
static void (*old_glBindTexture)(struct OGLES2IFace *Self, GLenum target, GLuint texture);
static void (*old_glGenTextures)(struct OGLES2IFace *Self, GLsizei n, GLuint * textures);
static void (*old_glGenerateMipmap)(struct OGLES2IFace *Self, GLenum target);
static void (*old_glTexParameterf)(struct OGLES2IFace *Self, GLenum target, GLenum pname, GLfloat param);
static void (*old_glTexParameterfv)(struct OGLES2IFace *Self, GLenum target, GLenum pname, const GLfloat * params);
static void (*old_glTexParameteri)(struct OGLES2IFace *Self, GLenum target, GLenum pname, GLint param);
static void (*old_glTexParameteriv)(struct OGLES2IFace *Self, GLenum target, GLenum pname, const GLint * params);
static void (*old_glTexSubImage2D)(struct OGLES2IFace *Self, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void * pixels);
static void (*old_glTexImage2D)(struct OGLES2IFace *Self, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void * pixels);
static void (*old_glDeleteTextures)(struct OGLES2IFace *Self, GLsizei n, const GLuint * textures);

// Error checking helpers

static void check_errors(const char* info)
{
    GLenum err;
    while ((err = ((struct OGLES2IFace *)patchedInterface)->glGetError()) != GL_NO_ERROR) {
        logLine("%s: GL error %d detected %s call", task_name(), err, info);
    }
}

#define PRE_CHECK check_errors("before");
#define POST_CHECK check_errors("after");

#define CHECK(x) \
PRE_CHECK \
x; \
POST_CHECK

// Wrap traced function calls

static void OGLES2_aglSwapBuffers(struct OGLES2IFace *Self)
{
    logLine("%s: %s", task_name(), __func__);

    if (old_aglSwapBuffers) {
        CHECK(old_aglSwapBuffers(Self))
    }
}

static void OGLES2_glCompileShader(struct OGLES2IFace *Self, GLuint shader)
{
    logLine("%s: %s: shader %u", task_name(), __func__,
        shader);

    if (old_glCompileShader) {
        CHECK(old_glCompileShader(Self, shader))
    }
}

static void OGLES2_glGenBuffers(struct OGLES2IFace *Self, GLsizei n, GLuint * buffers)
{
    logLine("%s: %s: n %d, buffers %p", task_name(), __func__,
        n, buffers);

    if (old_glGenBuffers) {
        CHECK(old_glGenBuffers(Self, n, buffers))
    }

    size_t i;
    for (i = 0; i < n; i++) {
        logLine("Buffer[%u] = %u", i, buffers[i]);
    }
}

static void OGLES2_glBindBuffer(struct OGLES2IFace *Self, GLenum target, GLuint buffer)
{
    logLine("%s: %s: target %d, buffer %u", task_name(), __func__,
        target, buffer);

    if (old_glBindBuffer) {
        CHECK(old_glBindBuffer(Self, target, buffer))
    }
}

static void OGLES2_glBufferData(struct OGLES2IFace *Self, GLenum target, GLsizeiptr size, const void * data, GLenum usage)
{
    logLine("%s: %s: target %d, size %u, data %p, usage %d", task_name(), __func__,
        target, size, data, usage);

    if (old_glBufferData) {
        CHECK(old_glBufferData(Self, target, size, data, usage))
    }
}

static void OGLES2_glBufferSubData(struct OGLES2IFace *Self, GLenum target, GLintptr offset, GLsizeiptr size, const void * data)
{
    logLine("%s: %s: target %d, offset %u, size %u, data %p", task_name(), __func__,
        target, offset, size, data);

    if (old_glBufferSubData) {
        CHECK(old_glBufferSubData(Self, target, offset, size, data))
    }
}

static void OGLES2_glDeleteBuffers(struct OGLES2IFace *Self, GLsizei n, GLuint * buffers)
{
    logLine("%s: %s: n %d, buffers %p", task_name(), __func__,
         n, buffers);

    size_t i;
    for (i = 0; i < n; i++) {
        logLine("Deleting buffer[%u] = %u", i, buffers[i]);
    }

    if (old_glDeleteBuffers) {
        CHECK(old_glDeleteBuffers(Self, n, buffers))
    }
}

static void OGLES2_glEnableVertexAttribArray(struct OGLES2IFace *Self, GLuint index)
{
    logLine("%s: %s: index %u", task_name(), __func__,
        index);

    if (old_glEnableVertexAttribArray) {
        CHECK(old_glEnableVertexAttribArray(Self, index))
    }
}

static void OGLES2_glVertexAttribPointer(struct OGLES2IFace *Self, GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void * pointer)
{
    logLine("%s: %s: index %u, size %d, type %d, normalized %d, stride %d, pointer %p", task_name(), __func__,
        index, size, type, normalized, stride, pointer);

    if (old_glVertexAttribPointer) {
        CHECK(old_glVertexAttribPointer(Self, index, size, type, normalized, stride, pointer))
    }
}

static void OGLES2_glDrawArrays(struct OGLES2IFace *Self, GLenum mode, GLint first, GLsizei count)
{
    logLine("%s: %s: mode %d, first %d, count %d", task_name(), __func__,
        mode, first, count);

    if (old_glDrawArrays) {
        CHECK(old_glDrawArrays(Self, mode, first, count))
    }
}

static void OGLES2_glDrawElements(struct OGLES2IFace *Self, GLenum mode, GLsizei count, GLenum type, const void * indices)
{
    logLine("%s: %s: mode %d, count %d, type %d, indices %p", task_name(), __func__,
        mode, count, type, indices);

    if (old_glDrawElements) {
        CHECK(old_glDrawElements(Self, mode, count, type, indices))
    }
}

static void OGLES2_glShaderSource(struct OGLES2IFace *Self, GLuint shader, GLsizei count, const GLchar *const* string, const GLint * length)
{
    logLine("%s: %s: shader %u, count %u, string %p length %p", task_name(), __func__,
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

    if (old_glShaderSource) {
        CHECK(old_glShaderSource(Self, shader, count, string, length))
    }
}

static void OGLES2_glActiveTexture(struct OGLES2IFace *Self, GLenum texture)
{
    logLine("%s: %s: texture %d", task_name(), __func__,
        texture);

    if (old_glActiveTexture) {
        CHECK(old_glActiveTexture(Self, texture))
    }
}

static void OGLES2_glBindTexture(struct OGLES2IFace *Self, GLenum target, GLuint texture)
{
    logLine("%s: %s: target %d, texture %d", task_name(), __func__,
        target, texture);

    if (old_glBindTexture) {
        CHECK(old_glBindTexture(Self, target, texture))
    }
}

static void OGLES2_glGenTextures(struct OGLES2IFace *Self, GLsizei n, GLuint * textures)
{
    logLine("%s: %s: n %d, textures %p", task_name(), __func__,
        n, textures);

    if (old_glGenTextures) {
        CHECK(old_glGenTextures(Self, n, textures))
    }

    size_t i;
    for (i = 0; i < n; i++) {
        logLine("Texture[%u] = %u", i, textures[i]);
    }
}

static void OGLES2_glGenerateMipmap(struct OGLES2IFace *Self, GLenum target)
{
    logLine("%s: %s: target %d", task_name(), __func__,
        target);

    if (old_glGenerateMipmap) {
        CHECK(old_glGenerateMipmap(Self, target))
    }
}

static void OGLES2_glTexParameterf(struct OGLES2IFace *Self, GLenum target, GLenum pname, GLfloat param)
{
    logLine("%s: %s: target %d, pname %d, param %f", task_name(), __func__,
        target, pname, param);

    if (old_glTexParameterf) {
        CHECK(old_glTexParameterf(Self, target, pname, param))
    }
}

static void OGLES2_glTexParameterfv(struct OGLES2IFace *Self, GLenum target, GLenum pname, const GLfloat * params)
{
    logLine("%s: %s: target %d, pname %d, params %p", task_name(), __func__,
        target, pname, params);

    if (old_glTexParameterfv) {
        CHECK(old_glTexParameterfv(Self, target, pname, params))
    }
}

static void OGLES2_glTexParameteri(struct OGLES2IFace *Self, GLenum target, GLenum pname, GLint param)
{
    logLine("%s: %s: target %d, pname %d, param %d", task_name(), __func__,
        target, pname, param);

    if (old_glTexParameteri) {
        CHECK(old_glTexParameteri(Self, target, pname, param))
    }
}

static void OGLES2_glTexParameteriv(struct OGLES2IFace *Self, GLenum target, GLenum pname, const GLint * params)
{
    logLine("%s: %s: target %d, pname %d, params %p", task_name(), __func__,
        target, pname, params);

    if (old_glTexParameteriv) {
        CHECK(old_glTexParameteriv(Self, target, pname, params))
    }
}

static void OGLES2_glTexSubImage2D(struct OGLES2IFace *Self, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void * pixels)
{
    logLine("%s: %s: target %d, level %d, xoffset %d, yoffset %d, width %u, height %u, format %d, type %d, pixels %p", task_name(), __func__,
        target, level, xoffset, yoffset, width, height, format, type, pixels);

    if (old_glTexSubImage2D) {
        CHECK(old_glTexSubImage2D(Self, target, level, xoffset, yoffset, width, height, format, type, pixels))
    }
}

static void OGLES2_glTexImage2D(struct OGLES2IFace *Self, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void * pixels)
{
    logLine("%s: %s: target %d, level %d, internalformat %d, width %u, height %u, border %d, format %d, type %d, pixels %p", task_name(), __func__,
        target, level, internalformat, width, height, border, format, type, pixels);

    if (old_glTexImage2D) {
        CHECK(old_glTexImage2D(Self, target, level, internalformat, width, height, border, format, type, pixels))
    }
}

static void OGLES2_glDeleteTextures(struct OGLES2IFace *Self, GLsizei n, const GLuint * textures)
{
    logLine("%s: %s: n %u, textures %p", task_name(), __func__,
        n, textures);

    size_t i;
    for (i = 0; i < n; i++) {
        logLine("Deleting texture[%u] = %u", i, textures[i]);
    }

    if (old_glDeleteTextures) {
        CHECK(old_glDeleteTextures(Self, n, textures))
    }
}

PATCH_INTERFACE(OGLES2IFace, aglSwapBuffers, OGLES2)
PATCH_INTERFACE(OGLES2IFace, glCompileShader, OGLES2)
PATCH_INTERFACE(OGLES2IFace, glGenBuffers, OGLES2)
PATCH_INTERFACE(OGLES2IFace, glBindBuffer, OGLES2)
PATCH_INTERFACE(OGLES2IFace, glBufferData, OGLES2)
PATCH_INTERFACE(OGLES2IFace, glBufferSubData, OGLES2)
PATCH_INTERFACE(OGLES2IFace, glDeleteBuffers, OGLES2)
PATCH_INTERFACE(OGLES2IFace, glEnableVertexAttribArray, OGLES2)
PATCH_INTERFACE(OGLES2IFace, glVertexAttribPointer, OGLES2)
PATCH_INTERFACE(OGLES2IFace, glDrawArrays, OGLES2)
PATCH_INTERFACE(OGLES2IFace, glDrawElements, OGLES2)
PATCH_INTERFACE(OGLES2IFace, glShaderSource, OGLES2)
PATCH_INTERFACE(OGLES2IFace, glActiveTexture, OGLES2)
PATCH_INTERFACE(OGLES2IFace, glBindTexture, OGLES2)
PATCH_INTERFACE(OGLES2IFace, glGenTextures, OGLES2)
PATCH_INTERFACE(OGLES2IFace, glGenerateMipmap, OGLES2)
PATCH_INTERFACE(OGLES2IFace, glTexParameterf, OGLES2)
PATCH_INTERFACE(OGLES2IFace, glTexParameterfv, OGLES2)
PATCH_INTERFACE(OGLES2IFace, glTexParameteri, OGLES2)
PATCH_INTERFACE(OGLES2IFace, glTexParameteriv, OGLES2)
PATCH_INTERFACE(OGLES2IFace, glTexSubImage2D, OGLES2)
PATCH_INTERFACE(OGLES2IFace, glTexImage2D, OGLES2)
PATCH_INTERFACE(OGLES2IFace, glDeleteTextures, OGLES2)

static void (*patches[])(BOOL, struct Interface*) = {
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
    if (open_ogles2_library()) {
        patch_GetInterface(TRUE, (struct Interface *)IExec);
    }
}

static void patch_ogles2_functions(struct Interface* interface)
{
    if (interface) {
        int i;
        for (i = 0; i < sizeof(patches) / sizeof(patches[0]); i++) {
            if (patches[i]) {
                patches[i](TRUE, interface);
            }
        }

        patchedInterface = interface;
    }
}

void ogles2_remove_patches(void)
{
    if (patchedInterface) {
        int i;
        for (i = 0; i < sizeof(patches) / sizeof(patches[0]); i++) {
            if (patches[i]) {
                patches[i](FALSE, patchedInterface);
            }
        }
    }

    patch_GetInterface(FALSE, (struct Interface *)IExec);

    close_ogles2_library();
}
