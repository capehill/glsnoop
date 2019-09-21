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
    CreateProgram,
    CreateShader,
    CullFace,
    DeleteBuffers,
    DeleteFramebuffers,
    DeleteProgram,
    DeleteRenderbuffers,
    DeleteShader,
    DeleteTextures,
    DepthFunc,
    DepthMask,
    DepthRangef,
    DetachShader,
    Disable,
    DisableVertexAttribArray,
    DrawArrays,
    DrawElements,
    Enable,
    EnableVertexAttribArray,
    Finish,
    Flush,
    FramebufferRenderbuffer,
    FramebufferTexture2D,
    FrontFace,
    GenBuffers,
    GenerateMipmap,
    GenFramebuffers,
    GenRenderbuffers,
    GenTextures,
    GetActiveAttrib,
    GetActiveUniform,
    GetAttachedShaders,
    GetAttribLocation,
    GetBooleanv,
    GetBufferParameteriv,
    GetError,
    GetFloatv,
    GetFramebufferAttachmentParameteriv,
    GetIntegerv,
    GetProgramiv,
    GetProgramInfoLog,
    GetRenderbufferParameteriv,
    GetShaderiv,
    GetShaderInfoLog,
    GetShaderPrecisionFormat,
    GetShaderSource,
    GetString,
    GetTexParameterfv,
    GetTexParameteriv,
    GetUniformfv,
    GetUniformiv,
    GetUniformLocation,
    GetVertexAttribfv,
    GetVertexAttribiv,
    GetVertexAttribPointerv,
    Hint,
    IsBuffer,
    IsEnabled,
    IsFramebuffer,
    IsProgram,
    IsRenderbuffer,
    IsShader,
    IsTexture,
    LineWidth,
    LinkProgram,
    PixelStorei,
    PolygonOffset,
    ReadPixels,
    ReleaseShaderCompiler,
    RenderbufferStorage,
    SampleCoverage,
    Scissor,
    ShaderBinary,
    ShaderSource,
    StencilFunc,
    StencilFuncSeparate,
    StencilMask,
    StencilMaskSeparate,
    StencilOp,
    StencilOpSeparate,
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
        MAP_ENUM(CreateProgram)
        MAP_ENUM(CreateShader)
        MAP_ENUM(CullFace)
        MAP_ENUM(DeleteBuffers)
        MAP_ENUM(DeleteFramebuffers)
        MAP_ENUM(DeleteProgram)
        MAP_ENUM(DeleteRenderbuffers)
        MAP_ENUM(DeleteShader)
        MAP_ENUM(DeleteTextures)
        MAP_ENUM(DepthFunc)
        MAP_ENUM(DepthMask)
        MAP_ENUM(DepthRangef)
        MAP_ENUM(DetachShader)
        MAP_ENUM(Disable)
        MAP_ENUM(DisableVertexAttribArray)
        MAP_ENUM(DrawArrays)
        MAP_ENUM(DrawElements)
        MAP_ENUM(Enable)
        MAP_ENUM(EnableVertexAttribArray)
        MAP_ENUM(Finish)
        MAP_ENUM(Flush)
        MAP_ENUM(FramebufferRenderbuffer)
        MAP_ENUM(FramebufferTexture2D)
        MAP_ENUM(FrontFace)
        MAP_ENUM(GenBuffers)
        MAP_ENUM(GenerateMipmap)
        MAP_ENUM(GenFramebuffers)
        MAP_ENUM(GenRenderbuffers)
        MAP_ENUM(GenTextures)
        MAP_ENUM(GetActiveAttrib)
        MAP_ENUM(GetActiveUniform)
        MAP_ENUM(GetAttachedShaders)
        MAP_ENUM(GetAttribLocation)
        MAP_ENUM(GetBooleanv)
        MAP_ENUM(GetBufferParameteriv)
        MAP_ENUM(GetError)
        MAP_ENUM(GetFloatv)
        MAP_ENUM(GetFramebufferAttachmentParameteriv)
        MAP_ENUM(GetIntegerv)
        MAP_ENUM(GetProgramiv)
        MAP_ENUM(GetProgramInfoLog)
        MAP_ENUM(GetRenderbufferParameteriv)
        MAP_ENUM(GetShaderiv)
        MAP_ENUM(GetShaderInfoLog)
        MAP_ENUM(GetShaderPrecisionFormat)
        MAP_ENUM(GetShaderSource)
        MAP_ENUM(GetString)
        MAP_ENUM(GetTexParameterfv)
        MAP_ENUM(GetTexParameteriv)
        MAP_ENUM(GetUniformfv)
        MAP_ENUM(GetUniformiv)
        MAP_ENUM(GetUniformLocation)
        MAP_ENUM(GetVertexAttribfv)
        MAP_ENUM(GetVertexAttribiv)
        MAP_ENUM(GetVertexAttribPointerv)
        MAP_ENUM(Hint)
        MAP_ENUM(IsBuffer)
        MAP_ENUM(IsEnabled)
        MAP_ENUM(IsFramebuffer)
        MAP_ENUM(IsProgram)
        MAP_ENUM(IsRenderbuffer)
        MAP_ENUM(IsShader)
        MAP_ENUM(IsTexture)
        MAP_ENUM(LineWidth)
        MAP_ENUM(LinkProgram)
        MAP_ENUM(PixelStorei)
        MAP_ENUM(PolygonOffset)
        MAP_ENUM(ReadPixels)
        MAP_ENUM(ReleaseShaderCompiler)
        MAP_ENUM(RenderbufferStorage)
        MAP_ENUM(SampleCoverage)
        MAP_ENUM(Scissor)
        MAP_ENUM(ShaderBinary)
        MAP_ENUM(ShaderSource)
        MAP_ENUM(SwapBuffers)
        MAP_ENUM(StencilFunc)
        MAP_ENUM(StencilFuncSeparate)
        MAP_ENUM(StencilMask)
        MAP_ENUM(StencilMaskSeparate)
        MAP_ENUM(StencilOp)
        MAP_ENUM(StencilOpSeparate)
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
    GLuint (*old_glCreateProgram)(struct OGLES2IFace *Self);
    GLuint (*old_glCreateShader)(struct OGLES2IFace *Self, GLenum type);
    void (*old_glCullFace)(struct OGLES2IFace *Self, GLenum mode);
    void (*old_glDeleteBuffers)(struct OGLES2IFace *Self, GLsizei n, GLuint * buffers);
    void (*old_glDeleteFramebuffers)(struct OGLES2IFace *Self, GLsizei n, const GLuint * framebuffers);
    void (*old_glDeleteProgram)(struct OGLES2IFace *Self, GLuint program);
    void (*old_glDeleteRenderbuffers)(struct OGLES2IFace *Self, GLsizei n, const GLuint * renderbuffers);
    void (*old_glDeleteShader)(struct OGLES2IFace *Self, GLuint shader);
    void (*old_glDeleteTextures)(struct OGLES2IFace *Self, GLsizei n, const GLuint * textures);
    void (*old_glDepthFunc)(struct OGLES2IFace *Self, GLenum func);
    void (*old_glDepthMask)(struct OGLES2IFace *Self, GLboolean flag);
    void (*old_glDepthRangef)(struct OGLES2IFace *Self, GLfloat n, GLfloat f);
    void (*old_glDetachShader)(struct OGLES2IFace *Self, GLuint program, GLuint shader);
    void (*old_glDisable)(struct OGLES2IFace *Self, GLenum cap);
    void (*old_glDisableVertexAttribArray)(struct OGLES2IFace *Self, GLuint index);
    void (*old_glDrawArrays)(struct OGLES2IFace *Self, GLenum mode, GLint first, GLsizei count);
    void (*old_glDrawElements)(struct OGLES2IFace *Self, GLenum mode, GLsizei count, GLenum type, const void * indices);
    void (*old_glEnable)(struct OGLES2IFace *Self, GLenum cap);
    void (*old_glEnableVertexAttribArray)(struct OGLES2IFace *Self, GLuint index);
    void (*old_glFinish)(struct OGLES2IFace *Self);
    void (*old_glFlush)(struct OGLES2IFace *Self);
    void (*old_glFramebufferRenderbuffer)(struct OGLES2IFace *Self, GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
    void (*old_glFramebufferTexture2D)(struct OGLES2IFace *Self, GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
    void (*old_glFrontFace)(struct OGLES2IFace *Self, GLenum mode);
    void (*old_glGenBuffers)(struct OGLES2IFace *Self, GLsizei n, GLuint * buffers);
    void (*old_glGenerateMipmap)(struct OGLES2IFace *Self, GLenum target);
    void (*old_glGenFramebuffers)(struct OGLES2IFace *Self, GLsizei n, GLuint * framebuffers);
    void (*old_glGenRenderbuffers)(struct OGLES2IFace *Self, GLsizei n, GLuint * renderbuffers);
    void (*old_glGenTextures)(struct OGLES2IFace *Self, GLsizei n, GLuint * textures);
    void (*old_glGetActiveAttrib)(struct OGLES2IFace *Self, GLuint program, GLuint index, GLsizei bufSize, GLsizei * length, GLint * size, GLenum * type, GLchar * name);
    void (*old_glGetActiveUniform)(struct OGLES2IFace *Self, GLuint program, GLuint index, GLsizei bufSize, GLsizei * length, GLint * size, GLenum * type, GLchar * name);
    void (*old_glGetAttachedShaders)(struct OGLES2IFace *Self, GLuint program, GLsizei maxCount, GLsizei * count, GLuint * shaders);
    GLint (*old_glGetAttribLocation)(struct OGLES2IFace *Self, GLuint program, const GLchar * name);
    void (*old_glGetBooleanv)(struct OGLES2IFace *Self, GLenum pname, GLboolean * data);
    void (*old_glGetBufferParameteriv)(struct OGLES2IFace *Self, GLenum target, GLenum pname, GLint * params);
    GLenum (*old_glGetError)(struct OGLES2IFace *Self);
    void (*old_glGetFloatv)(struct OGLES2IFace *Self, GLenum pname, GLfloat * data);
    void (*old_glGetFramebufferAttachmentParameteriv)(struct OGLES2IFace *Self, GLenum target, GLenum attachment, GLenum pname, GLint * params);
    void (*old_glGetIntegerv)(struct OGLES2IFace *Self, GLenum pname, GLint * data);
    void (*old_glGetProgramiv)(struct OGLES2IFace *Self, GLuint program, GLenum pname, GLint * params);
    void (*old_glGetProgramInfoLog)(struct OGLES2IFace *Self, GLuint program, GLsizei bufSize, GLsizei * length, GLchar * infoLog);
    void (*old_glGetRenderbufferParameteriv)(struct OGLES2IFace *Self, GLenum target, GLenum pname, GLint * params);
    void (*old_glGetShaderiv)(struct OGLES2IFace *Self, GLuint shader, GLenum pname, GLint * params);
    void (*old_glGetShaderInfoLog)(struct OGLES2IFace *Self, GLuint shader, GLsizei bufSize, GLsizei * length, GLchar * infoLog);
    void (*old_glGetShaderPrecisionFormat)(struct OGLES2IFace *Self, GLenum shadertype, GLenum precisiontype, GLint * range, GLint * precision);
    void (*old_glGetShaderSource)(struct OGLES2IFace *Self, GLuint shader, GLsizei bufSize, GLsizei * length, GLchar * source);
    const GLubyte * (*old_glGetString)(struct OGLES2IFace *Self, GLenum name);
    void (*old_glGetTexParameterfv)(struct OGLES2IFace *Self, GLenum target, GLenum pname, GLfloat * params);
    void (*old_glGetTexParameteriv)(struct OGLES2IFace *Self, GLenum target, GLenum pname, GLint * params);
    void (*old_glGetUniformfv)(struct OGLES2IFace *Self, GLuint program, GLint location, GLfloat * params);
    void (*old_glGetUniformiv)(struct OGLES2IFace *Self, GLuint program, GLint location, GLint * params);
    GLint (*old_glGetUniformLocation)(struct OGLES2IFace *Self, GLuint program, const GLchar * name);
    void (*old_glGetVertexAttribfv)(struct OGLES2IFace *Self, GLuint index, GLenum pname, GLfloat * params);
    void (*old_glGetVertexAttribiv)(struct OGLES2IFace *Self, GLuint index, GLenum pname, GLint * params);
    void (*old_glGetVertexAttribPointerv)(struct OGLES2IFace *Self, GLuint index, GLenum pname, void ** pointer);
    void (*old_glHint)(struct OGLES2IFace *Self, GLenum target, GLenum mode);
    GLboolean (*old_glIsBuffer)(struct OGLES2IFace *Self, GLuint buffer);
    GLboolean (*old_glIsEnabled)(struct OGLES2IFace *Self, GLenum cap);
    GLboolean (*old_glIsFramebuffer)(struct OGLES2IFace *Self, GLuint framebuffer);
    GLboolean (*old_glIsProgram)(struct OGLES2IFace *Self, GLuint program);
    GLboolean (*old_glIsRenderbuffer)(struct OGLES2IFace *Self, GLuint renderbuffer);
    GLboolean (*old_glIsShader)(struct OGLES2IFace *Self, GLuint shader);
    GLboolean (*old_glIsTexture)(struct OGLES2IFace *Self, GLuint texture);
    void (*old_glLineWidth)(struct OGLES2IFace *Self, GLfloat width);
    void (*old_glLinkProgram)(struct OGLES2IFace *Self, GLuint program);
    void (*old_glPixelStorei)(struct OGLES2IFace *Self, GLenum pname, GLint param);
    void (*old_glPolygonOffset)(struct OGLES2IFace *Self, GLfloat factor, GLfloat units);
    void (*old_glReadPixels)(struct OGLES2IFace *Self, GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void * pixels);
    void (*old_glReleaseShaderCompiler)(struct OGLES2IFace *Self);
    void (*old_glRenderbufferStorage)(struct OGLES2IFace *Self, GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
    void (*old_glSampleCoverage)(struct OGLES2IFace *Self, GLfloat value, GLboolean invert);
    void (*old_glScissor)(struct OGLES2IFace *Self, GLint x, GLint y, GLsizei width, GLsizei height);
    void (*old_glShaderBinary)(struct OGLES2IFace *Self, GLsizei count, const GLuint * shaders, GLenum binaryformat, const void * binary, GLsizei length);
    void (*old_glShaderSource)(struct OGLES2IFace *Self, GLuint shader, GLsizei count, const GLchar *const* string, const GLint * length);
    void (*old_glStencilFunc)(struct OGLES2IFace *Self, GLenum func, GLint ref, GLuint mask);
    void (*old_glStencilFuncSeparate)(struct OGLES2IFace *Self, GLenum face, GLenum func, GLint ref, GLuint mask);
    void (*old_glStencilMask)(struct OGLES2IFace *Self, GLuint mask);
    void (*old_glStencilMaskSeparate)(struct OGLES2IFace *Self, GLenum face, GLuint mask);
    void (*old_glStencilOp)(struct OGLES2IFace *Self, GLenum fail, GLenum zfail, GLenum zpass);
    void (*old_glStencilOpSeparate)(struct OGLES2IFace *Self, GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass);
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

    GLenum (*func)(struct OGLES2IFace *Self);

    // Don't use the wrapper function
    if (context->old_glGetError) {
        func = context->old_glGetError;
    } else {
        func = context->interface->glGetError;
    }

    while ((err = func(context->interface)) != GL_NO_ERROR) {
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

static GLuint OGLES2_glCreateProgram(struct OGLES2IFace *Self)
{
    GET_CONTEXT

    GLuint status = 0;

    GL_CALL_STATUS(CreateProgram)

    logLine("%s: %s: created program %u", context->name, __func__,
        status);

    return status;
}

static GLuint OGLES2_glCreateShader(struct OGLES2IFace *Self, GLenum type)
{
    GET_CONTEXT

    GLuint status = 0;

    GL_CALL_STATUS(CreateShader, type)

    logLine("%s: %s: type %u. Created shader %u", context->name, __func__,
        type, status);

    return status;
}

static void OGLES2_glCullFace(struct OGLES2IFace *Self, GLenum mode)
{
    GET_CONTEXT

    GL_CALL(CullFace, mode)

    logLine("%s: %s: mode %u", context->name, __func__,
        mode);
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

static void OGLES2_glDeleteProgram(struct OGLES2IFace *Self, GLuint program)
{
    GET_CONTEXT

    logLine("%s: %s: program %u", context->name, __func__,
        program);

    GL_CALL(DeleteProgram, program)
}

static void OGLES2_glDeleteRenderbuffers(struct OGLES2IFace *Self, GLsizei n, const GLuint * renderbuffers)
{
    GET_CONTEXT

    logLine("%s: %s: n %d, renderbuffers %p", context->name, __func__,
        n, renderbuffers);

    GLsizei i;
    for (i = 0; i < n; i++) {
        logLine("Deleting renderbuffer[%u] = %u", i, renderbuffers[i]);
    }

    GL_CALL(DeleteRenderbuffers, n, renderbuffers)
}

static void OGLES2_glDeleteShader(struct OGLES2IFace *Self, GLuint shader)
{
    GET_CONTEXT

    logLine("%s: %s: shader %u", context->name, __func__,
        shader);

    GL_CALL(DeleteShader, shader)
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

static void OGLES2_glDepthFunc(struct OGLES2IFace *Self, GLenum func)
{
    GET_CONTEXT

    logLine("%s: %s: func %u", context->name, __func__,
        func);

    GL_CALL(DepthFunc, func)
}

static void OGLES2_glDepthMask(struct OGLES2IFace *Self, GLboolean flag)
{
    GET_CONTEXT

    logLine("%s: %s: flag %d", context->name, __func__,
        flag);

    GL_CALL(DepthMask, flag)
}

static void OGLES2_glDepthRangef(struct OGLES2IFace *Self, GLfloat n, GLfloat f)
{
    GET_CONTEXT

    logLine("%s: %s: n %f f %f", context->name, __func__,
        n, f);

    GL_CALL(DepthRangef, n, f)
}

static void OGLES2_glDetachShader(struct OGLES2IFace *Self, GLuint program, GLuint shader)
{
    GET_CONTEXT

    logLine("%s: %s: program %u, shader %u", context->name, __func__,
        program, shader);

    GL_CALL(DetachShader, program, shader)
}

static void OGLES2_glDisable(struct OGLES2IFace *Self, GLenum cap)
{
    GET_CONTEXT

    logLine("%s: %s: cap %d", context->name, __func__,
        cap);

    GL_CALL(Disable, cap)
}

static void OGLES2_glDisableVertexAttribArray(struct OGLES2IFace *Self, GLuint index)
{
    GET_CONTEXT

    logLine("%s: %s: index %u", context->name, __func__,
        index);

    GL_CALL(DisableVertexAttribArray, index)
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

static void OGLES2_glFrontFace(struct OGLES2IFace *Self, GLenum mode)
{
    GET_CONTEXT

    logLine("%s: %s: mode %d", context->name, __func__,
        mode);

    GL_CALL(FrontFace, mode)
}

static void OGLES2_glGenBuffers(struct OGLES2IFace *Self, GLsizei n, GLuint * buffers)
{
    GET_CONTEXT

    logLine("%s: %s: n %d, buffers %p", context->name, __func__,
        n, buffers);

    GL_CALL(GenBuffers, n, buffers)

    GLsizei i;
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

static void OGLES2_glGenRenderbuffers(struct OGLES2IFace *Self, GLsizei n, GLuint * renderbuffers)
{
    GET_CONTEXT

    logLine("%s: %s: n %u, renderbuffers %p", context->name, __func__,
        n, renderbuffers);

    GL_CALL(GenRenderbuffers, n, renderbuffers)

    GLsizei i;
    for (i = 0; i < n; i++) {
        logLine("Renderbuffer[%u] = %u", i, renderbuffers[i]);
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

static void OGLES2_glGetActiveAttrib(struct OGLES2IFace *Self, GLuint program, GLuint index, GLsizei bufSize, GLsizei * length, GLint * size, GLenum * type, GLchar * name)
{
    GET_CONTEXT

    GLsizei tempLength = 0;

    GL_CALL(GetActiveAttrib, program, index, bufSize, &tempLength, size, type, name)

    logLine("%s: %s: program %u, index %u, bufSize %u, length %u, size %d, type %u, name '%s'", context->name, __func__,
        program, index, bufSize, tempLength, *size, *type, name);

    if (length) {
        *length = tempLength;
    }
}

static void OGLES2_glGetActiveUniform(struct OGLES2IFace *Self, GLuint program, GLuint index, GLsizei bufSize, GLsizei * length, GLint * size, GLenum * type, GLchar * name)
{
    GET_CONTEXT

    GLsizei tempLength = 0;

    GL_CALL(GetActiveUniform, program, index, bufSize, &tempLength, size, type, name)

    logLine("%s: %s: program %u, index %u, bufSize %u, length %u, size %d, type %u, name '%s'", context->name, __func__,
        program, index, bufSize, tempLength, *size, *type, name);

    if (length) {
        *length = tempLength;
    }
}

static void OGLES2_glGetAttachedShaders(struct OGLES2IFace *Self, GLuint program, GLsizei maxCount, GLsizei * count, GLuint * shaders)
{
    GET_CONTEXT

    logLine("%s: %s: program %u, maxCount %u, count %p, shaders %p", context->name, __func__,
        program, maxCount, count, shaders);

    GLsizei tempCount = 0;

    GL_CALL(GetAttachedShaders, program, maxCount, &tempCount, shaders)

    GLsizei i;
    for (i = 0; i < tempCount; i++) {
        logLine("shader[%u] = %u", i, shaders[i]);
    }

    if (count) {
        *count = tempCount;
    }
}

static GLint OGLES2_glGetAttribLocation(struct OGLES2IFace *Self, GLuint program, const GLchar * name)
{
    GET_CONTEXT

    GLint status = 0;

    GL_CALL_STATUS(GetAttribLocation, program, name)

    logLine("%s: %s: program %u, name '%s'. Location %d", context->name, __func__,
        program, name, status);

    return status;
}

static void OGLES2_glGetBooleanv(struct OGLES2IFace *Self, GLenum pname, GLboolean * data)
{
    GET_CONTEXT

    GL_CALL(GetBooleanv, pname, data)

    logLine("%s: %s: pname %u, data %d", context->name, __func__,
        pname, *data);
}

static void OGLES2_glGetBufferParameteriv(struct OGLES2IFace *Self, GLenum target, GLenum pname, GLint * params)
{
    GET_CONTEXT

    GL_CALL(GetBufferParameteriv, target, pname, params)

    logLine("%s: %s: target %u, pname %u, params %d", context->name, __func__,
        target, pname, *params);
}

// NOTE: each OpenGL call triggers error checking, so it's likely that this returns usually GL_NO_ERROR.
static GLenum OGLES2_glGetError(struct OGLES2IFace *Self)
{
    GET_CONTEXT

    GLenum status = 0;

    GL_CALL_STATUS(GetError)

    logLine("%s: %s: error %u (%s)", context->name, __func__,
        status, (status == GL_NO_ERROR) ? "GL_NO_ERROR" : mapOgles2Error(status));

    return status;
}

static void OGLES2_glGetFloatv(struct OGLES2IFace *Self, GLenum pname, GLfloat * data)
{
    GET_CONTEXT

    GL_CALL(GetFloatv, pname, data)

    logLine("%s: %s: pname %u, data %f", context->name, __func__,
        pname, *data);
}

static void OGLES2_glGetFramebufferAttachmentParameteriv(struct OGLES2IFace *Self, GLenum target, GLenum attachment, GLenum pname, GLint * params)
{
    GET_CONTEXT

    GL_CALL(GetFramebufferAttachmentParameteriv, target, attachment, pname, params)

    logLine("%s: %s: target %u, attachment %u pname %u, params %d", context->name, __func__,
        target, attachment, pname, *params);
}

static void OGLES2_glGetIntegerv(struct OGLES2IFace *Self, GLenum pname, GLint * data)
{
    GET_CONTEXT

    GL_CALL(GetIntegerv, pname, data)

    logLine("%s: %s: pname %u, data %d", context->name, __func__,
        pname, *data);
}

static void OGLES2_glGetProgramiv(struct OGLES2IFace *Self, GLuint program, GLenum pname, GLint * params)
{
    GET_CONTEXT

    GL_CALL(GetProgramiv, program, pname, params)

    logLine("%s: %s: program %u, pname %u, params %d", context->name, __func__,
        program, pname, *params);
}

static void OGLES2_glGetProgramInfoLog(struct OGLES2IFace *Self, GLuint program, GLsizei bufSize, GLsizei * length, GLchar * infoLog)
{
    GET_CONTEXT

    GLsizei tempLength = 0;

    GL_CALL(GetProgramInfoLog, program, bufSize, &tempLength, infoLog)

    logLine("%s: %s: program %u, bufSize %u, length %u, infoLog '%s'", context->name, __func__,
        program, bufSize, tempLength, infoLog);

    if (length) {
        *length = tempLength;
    }
}

static void OGLES2_glGetRenderbufferParameteriv(struct OGLES2IFace *Self, GLenum target, GLenum pname, GLint * params)
{
    GET_CONTEXT

    GL_CALL(GetRenderbufferParameteriv, target, pname, params)

    logLine("%s: %s: target %u, pname %u, params %d", context->name, __func__,
        target, pname, *params);
}

static void OGLES2_glGetShaderiv(struct OGLES2IFace *Self, GLuint shader, GLenum pname, GLint * params)
{
    GET_CONTEXT

    GL_CALL(GetShaderiv, shader, pname, params)

    logLine("%s: %s: shader %u, pname %u, params %d", context->name, __func__,
        shader, pname, *params);
}

static void OGLES2_glGetShaderInfoLog(struct OGLES2IFace *Self, GLuint shader, GLsizei bufSize, GLsizei * length, GLchar * infoLog)
{
    GET_CONTEXT

    GLsizei tempLength = 0;

    GL_CALL(GetShaderInfoLog, shader, bufSize, &tempLength, infoLog)

    logLine("%s: %s: shader %u, bufSize %u, length %u, infoLog '%s'", context->name, __func__,
        shader, bufSize, tempLength, infoLog);

    if (length) {
        *length = tempLength;
    }
}

static void OGLES2_glGetShaderPrecisionFormat(struct OGLES2IFace *Self, GLenum shadertype, GLenum precisiontype, GLint * range, GLint * precision)
{
    GET_CONTEXT

    GL_CALL(GetShaderPrecisionFormat, shadertype, precisiontype, range, precision)

    logLine("%s: %s: shadertype %u, precisiontype %u, range [%d, %d], precision %d", context->name, __func__,
        shadertype, precisiontype, range[0], range[1], *precision);
}

static void OGLES2_glGetShaderSource(struct OGLES2IFace *Self, GLuint shader, GLsizei bufSize, GLsizei * length, GLchar * source)
{
    GET_CONTEXT

    GLsizei tempLength = 0;

    GL_CALL(GetShaderSource, shader, bufSize, &tempLength, source)

    logLine("%s: %s: shader %u, bufSize %u, length %u, source '%s'", context->name, __func__,
        shader, bufSize, tempLength, source);

    if (length) {
        *length = tempLength;
    }
}

static const GLubyte * OGLES2_glGetString(struct OGLES2IFace *Self, GLenum name)
{
    GET_CONTEXT

    const GLubyte *status = NULL;

    GL_CALL_STATUS(GetString, name)

    logLine("%s: %s: name %u. String '%s'", context->name, __func__,
        name, status);

    return status;
}

static void OGLES2_glGetTexParameterfv(struct OGLES2IFace *Self, GLenum target, GLenum pname, GLfloat * params)
{
    GET_CONTEXT

    GL_CALL(GetTexParameterfv, target, pname, params)

    logLine("%s: %s: target %u, pname %u, params %f", context->name, __func__,
        target, pname, *params);
}

static void OGLES2_glGetTexParameteriv(struct OGLES2IFace *Self, GLenum target, GLenum pname, GLint * params)
{
    GET_CONTEXT

    GL_CALL(GetTexParameteriv, target, pname, params)

    logLine("%s: %s: target %u, pname %u, params %d", context->name, __func__,
        target, pname, *params);
}

static void OGLES2_glGetUniformfv(struct OGLES2IFace *Self, GLuint program, GLint location, GLfloat * params)
{
    GET_CONTEXT

    GL_CALL(GetUniformfv, program, location, params)

    logLine("%s: %s: program %u, location %u, params %f", context->name, __func__,
        program, location, *params);
}

static void OGLES2_glGetUniformiv(struct OGLES2IFace *Self, GLuint program, GLint location, GLint * params)
{
    GET_CONTEXT

    GL_CALL(GetUniformiv, program, location, params)

    logLine("%s: %s: program %u, location %u, params %d", context->name, __func__,
        program, location, *params);
}

static GLint OGLES2_glGetUniformLocation(struct OGLES2IFace *Self, GLuint program, const GLchar * name)
{
    GET_CONTEXT

    GLint status = 0;

    GL_CALL_STATUS(GetUniformLocation, program, name)

    logLine("%s: %s: program %u, name '%s'. Location %d", context->name, __func__,
        program, name, status);

    return status;
}

static void OGLES2_glGetVertexAttribfv(struct OGLES2IFace *Self, GLuint index, GLenum pname, GLfloat * params)
{
    GET_CONTEXT

    GL_CALL(GetVertexAttribfv, index, pname, params)

    logLine("%s: %s: index %u, pname %u, params %f", context->name, __func__,
        index, pname, *params);
}

static void OGLES2_glGetVertexAttribiv(struct OGLES2IFace *Self, GLuint index, GLenum pname, GLint * params)
{
    GET_CONTEXT

    GL_CALL(GetVertexAttribiv, index, pname, params)

    logLine("%s: %s: index %u, pname %u, params %d", context->name, __func__,
        index, pname, *params);
}

static void OGLES2_glGetVertexAttribPointerv(struct OGLES2IFace *Self, GLuint index, GLenum pname, void ** pointer)
{
    GET_CONTEXT

    GL_CALL(GetVertexAttribPointerv, index, pname, pointer)

    logLine("%s: %s: index %u, pname %u, pointer %p", context->name, __func__,
        index, pname, *pointer);
}

static void OGLES2_glHint(struct OGLES2IFace *Self, GLenum target, GLenum mode)
{
    GET_CONTEXT

    logLine("%s: %s: target %u, mode %u", context->name, __func__,
        target, mode);

    GL_CALL(Hint, target, mode)
}

static GLboolean OGLES2_glIsBuffer(struct OGLES2IFace *Self, GLuint buffer)
{
    GET_CONTEXT

    GLboolean status = GL_FALSE;

    GL_CALL_STATUS(IsBuffer, buffer)

    logLine("%s: %s: buffer %u. Result %d", context->name, __func__,
        buffer, status);

    return status;
}

static GLboolean OGLES2_glIsEnabled(struct OGLES2IFace *Self, GLenum cap)
{
    GET_CONTEXT

    GLboolean status = GL_FALSE;

    GL_CALL_STATUS(IsEnabled, cap)

    logLine("%s: %s: cap %u. Result %d", context->name, __func__,
        cap, status);

    return status;
}

static GLboolean OGLES2_glIsFramebuffer(struct OGLES2IFace *Self, GLuint framebuffer)
{
    GET_CONTEXT

    GLboolean status = GL_FALSE;

    GL_CALL_STATUS(IsFramebuffer, framebuffer)

    logLine("%s: %s: framebuffer %u. Result %d", context->name, __func__,
        framebuffer, status);

    return status;
}

static GLboolean OGLES2_glIsProgram(struct OGLES2IFace *Self, GLuint program)
{
    GET_CONTEXT

    GLboolean status = GL_FALSE;

    GL_CALL_STATUS(IsProgram, program)

    logLine("%s: %s: program %u. Result %d", context->name, __func__,
        program, status);

    return status;
}

static GLboolean OGLES2_glIsRenderbuffer(struct OGLES2IFace *Self, GLuint renderbuffer)
{
    GET_CONTEXT

    GLboolean status = GL_FALSE;

    GL_CALL_STATUS(IsRenderbuffer, renderbuffer)

    logLine("%s: %s: renderbuffer %u. Result %d", context->name, __func__,
        renderbuffer, status);

    return status;
}

static GLboolean OGLES2_glIsShader(struct OGLES2IFace *Self, GLuint shader)
{
    GET_CONTEXT

    GLboolean status = GL_FALSE;

    GL_CALL_STATUS(IsShader, shader)

    logLine("%s: %s: shader %u. Result %d", context->name, __func__,
        shader, status);

    return status;
}

static GLboolean OGLES2_glIsTexture(struct OGLES2IFace *Self, GLuint texture)
{
    GET_CONTEXT

    GLboolean status = GL_FALSE;

    GL_CALL_STATUS(IsTexture, texture)

    logLine("%s: %s: texture %u. Result %d", context->name, __func__,
        texture, status);

    return status;
}

static void OGLES2_glLineWidth(struct OGLES2IFace *Self, GLfloat width)
{
    GET_CONTEXT

    logLine("%s: %s: width %f", context->name, __func__,
        width);

    GL_CALL(LineWidth, width)
}

static void OGLES2_glLinkProgram(struct OGLES2IFace *Self, GLuint program)
{
    GET_CONTEXT

    logLine("%s: %s: program %u", context->name, __func__,
        program);

    GL_CALL(LinkProgram, program)
}

static void OGLES2_glPixelStorei(struct OGLES2IFace *Self, GLenum pname, GLint param)
{
    GET_CONTEXT

    logLine("%s: %s: pname %u, param %d", context->name, __func__,
        pname, param);

    GL_CALL(PixelStorei, pname, param)
}

static void OGLES2_glPolygonOffset(struct OGLES2IFace *Self, GLfloat factor, GLfloat units)
{
    GET_CONTEXT

    logLine("%s: %s: factor %f, units %f", context->name, __func__,
        factor, units);

    GL_CALL(PolygonOffset, factor, units)
}

static void OGLES2_glReadPixels(struct OGLES2IFace *Self, GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void * pixels)
{
    GET_CONTEXT

    logLine("%s: %s: x %d, y %d, width %u, height %u, format %u, type %u, pixels %p", context->name, __func__,
        x, y, width, height, format, type, pixels);

    GL_CALL(ReadPixels, x, y, width, height, format, type, pixels)
}

static void OGLES2_glReleaseShaderCompiler(struct OGLES2IFace *Self)
{
    GET_CONTEXT

    logLine("%s: %s", context->name, __func__);

    GL_CALL(ReleaseShaderCompiler)
}

static void OGLES2_glRenderbufferStorage(struct OGLES2IFace *Self, GLenum target, GLenum internalformat, GLsizei width, GLsizei height)
{
    GET_CONTEXT

    logLine("%s: %s: target %u, internalformat %u, width %u, height %u", context->name, __func__,
        target, internalformat, width, height);

    GL_CALL(RenderbufferStorage, target, internalformat, width, height)
}

static void OGLES2_glSampleCoverage(struct OGLES2IFace *Self, GLfloat value, GLboolean invert)
{
    GET_CONTEXT

    logLine("%s: %s: value %f, invert %d", context->name, __func__,
        value, invert);

    GL_CALL(SampleCoverage, value, invert)
}

static void OGLES2_glScissor(struct OGLES2IFace *Self, GLint x, GLint y, GLsizei width, GLsizei height)
{
    GET_CONTEXT

    logLine("%s: %s: x %d, y %d, width %u, height %u", context->name, __func__,
        x, y, width, height);

    GL_CALL(Scissor, x, y, width, height)
}

static void OGLES2_glShaderBinary(struct OGLES2IFace *Self, GLsizei count, const GLuint * shaders, GLenum binaryformat, const void * binary, GLsizei length)
{
    GET_CONTEXT

    logLine("%s: %s: count %u, shaders %p, binaryformat %u, binary %p, length %u", context->name, __func__,
        count, shaders, binaryformat, binary, length);

    GL_CALL(ShaderBinary, count, shaders, binaryformat, binary, length)
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

static void OGLES2_glStencilFunc(struct OGLES2IFace *Self, GLenum func, GLint ref, GLuint mask)
{
    GET_CONTEXT

    logLine("%s: %s: func %u, ref %d, mask %u", context->name, __func__,
        func, ref, mask);

    GL_CALL(StencilFunc, func, ref, mask)
}

static void OGLES2_glStencilFuncSeparate(struct OGLES2IFace *Self, GLenum face, GLenum func, GLint ref, GLuint mask)
{
    GET_CONTEXT

    logLine("%s: %s: face %u, func %u, ref %d, mask %u", context->name, __func__,
        face, func, ref, mask);

    GL_CALL(StencilFuncSeparate, face, func, ref, mask)
}

static void OGLES2_glStencilMask(struct OGLES2IFace *Self, GLuint mask)
{
    GET_CONTEXT

    logLine("%s: %s: mask %u", context->name, __func__,
        mask);

    GL_CALL(StencilMask, mask)
}

static void OGLES2_glStencilMaskSeparate(struct OGLES2IFace *Self, GLenum face, GLuint mask)
{
    GET_CONTEXT

    logLine("%s: %s: face %u, mask %u", context->name, __func__,
        face, mask);

    GL_CALL(StencilMaskSeparate, face, mask)
}

static void OGLES2_glStencilOp(struct OGLES2IFace *Self, GLenum fail, GLenum zfail, GLenum zpass)
{
    GET_CONTEXT

    logLine("%s: %s: fail %u, zfail %u, zpass %u", context->name, __func__,
        fail, zfail, zpass);

    GL_CALL(StencilOp, fail, zfail, zpass)
}

static void OGLES2_glStencilOpSeparate(struct OGLES2IFace *Self, GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass)
{
    GET_CONTEXT

    logLine("%s: %s: face %u, sfail %u, dpfail %u, dppass %u", context->name, __func__,
        face, sfail, dpfail, dppass);

    GL_CALL(StencilOpSeparate, face, sfail, dpfail, dppass)
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
GENERATE_FILTERED_PATCH(OGLES2IFace, glCreateProgram, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glCreateShader, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glCullFace, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glDeleteBuffers, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glDeleteFramebuffers, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glDeleteProgram, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glDeleteRenderbuffers, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glDeleteShader, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glDeleteTextures, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glDepthFunc, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glDepthMask, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glDepthRangef, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glDetachShader, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glDisable, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glDisableVertexAttribArray, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glDrawArrays, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glDrawElements, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glEnable, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glEnableVertexAttribArray, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glFinish, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glFlush, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glFramebufferRenderbuffer, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glFramebufferTexture2D, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glFrontFace, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glGenBuffers, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glGenerateMipmap, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glGenFramebuffers, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glGenRenderbuffers, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glGenTextures, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glGetActiveAttrib, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glGetActiveUniform, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glGetAttachedShaders, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glGetAttribLocation, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glGetBooleanv, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glGetBufferParameteriv, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glGetError, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glGetFloatv, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glGetFramebufferAttachmentParameteriv, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glGetIntegerv, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glGetProgramiv, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glGetProgramInfoLog, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glGetRenderbufferParameteriv, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glGetShaderiv, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glGetShaderInfoLog, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glGetShaderPrecisionFormat, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glGetShaderSource, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glGetString, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glGetTexParameterfv, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glGetTexParameteriv, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glGetUniformfv, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glGetUniformiv, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glGetUniformLocation, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glGetVertexAttribfv, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glGetVertexAttribiv, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glGetVertexAttribPointerv, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glHint, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glIsBuffer, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glIsEnabled, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glIsFramebuffer, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glIsProgram, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glIsRenderbuffer, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glIsShader, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glIsTexture, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glLineWidth, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glLinkProgram, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glPixelStorei, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glPolygonOffset, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glReadPixels, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glReleaseShaderCompiler, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glRenderbufferStorage, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glSampleCoverage, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glScissor, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glShaderBinary, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glShaderSource, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glStencilFunc, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glStencilFuncSeparate, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glStencilMask, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glStencilMaskSeparate, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glStencilOp, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glStencilOpSeparate, OGLES2, Ogles2Context)
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
    patch_glCreateProgram,
    patch_glCreateShader,
    patch_glCullFace,
    patch_glDeleteBuffers,
    patch_glDeleteFramebuffers,
    patch_glDeleteProgram,
    patch_glDeleteRenderbuffers,
    patch_glDeleteShader,
    patch_glDeleteTextures,
    patch_glDepthFunc,
    patch_glDepthMask,
    patch_glDepthRangef,
    patch_glDetachShader,
    patch_glDisable,
    patch_glDisableVertexAttribArray,
    patch_glDrawArrays,
    patch_glDrawElements,
    patch_glEnable,
    patch_glEnableVertexAttribArray,
    patch_glFinish,
    patch_glFlush,
    patch_glFramebufferRenderbuffer,
    patch_glFramebufferTexture2D,
    patch_glFrontFace,
    patch_glGenBuffers,
    patch_glGenerateMipmap,
    patch_glGenFramebuffers,
    patch_glGenRenderbuffers,
    patch_glGenTextures,
    patch_glGetActiveAttrib,
    patch_glGetActiveUniform,
    patch_glGetAttachedShaders,
    patch_glGetAttribLocation,
    patch_glGetBooleanv,
    patch_glGetBufferParameteriv,
    patch_glGetError,
    patch_glGetFloatv,
    patch_glGetFramebufferAttachmentParameteriv,
    patch_glGetIntegerv,
    patch_glGetProgramiv,
    patch_glGetProgramInfoLog,
    patch_glGetRenderbufferParameteriv,
    patch_glGetShaderiv,
    patch_glGetShaderInfoLog,
    patch_glGetShaderPrecisionFormat,
    patch_glGetShaderSource,
    patch_glGetString,
    patch_glGetTexParameterfv,
    patch_glGetTexParameteriv,
    patch_glGetUniformfv,
    patch_glGetUniformiv,
    patch_glGetUniformLocation,
    patch_glGetVertexAttribfv,
    patch_glGetVertexAttribiv,
    patch_glGetVertexAttribPointerv,
    patch_glHint,
    patch_glIsBuffer,
    patch_glIsEnabled,
    patch_glIsFramebuffer,
    patch_glIsProgram,
    patch_glIsRenderbuffer,
    patch_glIsShader,
    patch_glIsTexture,
    patch_glLineWidth,
    patch_glLinkProgram,
    patch_glPixelStorei,
    patch_glPolygonOffset,
    patch_glReadPixels,
    patch_glReleaseShaderCompiler,
    patch_glRenderbufferStorage,
    patch_glSampleCoverage,
    patch_glScissor,
    patch_glShaderBinary,
    patch_glShaderSource,
    patch_glStencilFunc,
    patch_glStencilFuncSeparate,
    patch_glStencilMask,
    patch_glStencilMaskSeparate,
    patch_glStencilOp,
    patch_glStencilOpSeparate,
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
