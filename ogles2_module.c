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
