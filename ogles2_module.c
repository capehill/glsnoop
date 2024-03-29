#include "ogles2_module.h"
#include "common.h"
#include "filter.h"
#include "timer.h"
#include "profiling.h"
#include "logger.h"

#include <proto/exec.h>
#include <proto/ogles2.h>
#include <proto/utility.h>

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

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
    CreateContext_AVOID,
    CreateContext2,
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
    DestroyContext,
    DetachShader,
    Disable,
    DisableVertexAttribArray,
    DrawArrays,
    DrawElements,
    DrawElementsBaseVertexOES,
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
    GetBufferParameterivOES,
    GetBufferPointervOES,
    GetError,
    GetFloatv,
    GetFramebufferAttachmentParameteriv,
    GetIntegerv,
    GetProcAddress,
    GetProgramBinaryOES,
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
    MakeCurrent,
    MapBufferOES,
    PixelStorei,
    PolygonMode,
    PolygonOffset,
    ProgramBinaryOES,
    ProvokingVertex,
    ReadPixels,
    ReleaseShaderCompiler,
    RenderbufferStorage,
    SampleCoverage,
    Scissor,
    SetBitmap,
    SetParams_AVOID,
    SetParams2,
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
    UnmapBufferOES,
    UseProgram,
    ValidateProgram,
    VertexAttrib1f,
    VertexAttrib1fv,
    VertexAttrib2f,
    VertexAttrib2fv,
    VertexAttrib3f,
    VertexAttrib3fv,
    VertexAttrib4f,
    VertexAttrib4fv,
    VertexAttribPointer,
    Viewport,
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
        MAP_ENUM(CreateContext_AVOID)
        MAP_ENUM(CreateContext2)
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
        MAP_ENUM(DestroyContext)
        MAP_ENUM(DetachShader)
        MAP_ENUM(Disable)
        MAP_ENUM(DisableVertexAttribArray)
        MAP_ENUM(DrawArrays)
        MAP_ENUM(DrawElements)
        MAP_ENUM(DrawElementsBaseVertexOES)
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
        MAP_ENUM(GetBufferParameterivOES)
        MAP_ENUM(GetBufferPointervOES)
        MAP_ENUM(GetError)
        MAP_ENUM(GetFloatv)
        MAP_ENUM(GetFramebufferAttachmentParameteriv)
        MAP_ENUM(GetIntegerv)
        MAP_ENUM(GetProcAddress)
        MAP_ENUM(GetProgramBinaryOES)
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
        MAP_ENUM(MakeCurrent)
        MAP_ENUM(MapBufferOES)
        MAP_ENUM(PixelStorei)
        MAP_ENUM(PolygonMode)
        MAP_ENUM(PolygonOffset)
        MAP_ENUM(ProgramBinaryOES)
        MAP_ENUM(ProvokingVertex)
        MAP_ENUM(ReadPixels)
        MAP_ENUM(ReleaseShaderCompiler)
        MAP_ENUM(RenderbufferStorage)
        MAP_ENUM(SampleCoverage)
        MAP_ENUM(Scissor)
        MAP_ENUM(SetBitmap)
        MAP_ENUM(SetParams_AVOID)
        MAP_ENUM(SetParams2)
        MAP_ENUM(ShaderBinary)
        MAP_ENUM(ShaderSource)
        MAP_ENUM(StencilFunc)
        MAP_ENUM(StencilFuncSeparate)
        MAP_ENUM(StencilMask)
        MAP_ENUM(StencilMaskSeparate)
        MAP_ENUM(StencilOp)
        MAP_ENUM(StencilOpSeparate)
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
        MAP_ENUM(UnmapBufferOES)
        MAP_ENUM(UseProgram)
        MAP_ENUM(ValidateProgram)
        MAP_ENUM(VertexAttrib1f)
        MAP_ENUM(VertexAttrib1fv)
        MAP_ENUM(VertexAttrib2f)
        MAP_ENUM(VertexAttrib2fv)
        MAP_ENUM(VertexAttrib3f)
        MAP_ENUM(VertexAttrib3fv)
        MAP_ENUM(VertexAttrib4f)
        MAP_ENUM(VertexAttrib4fv)
        MAP_ENUM(VertexAttribPointer)
        MAP_ENUM(Viewport)

        case Ogles2FunctionCount: break;
    }

    #undef MAP_ENUM

    return "Unknown";
}

static const char* mapOgles2Error(const GLenum code)
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

static const char* decodeTexture(const GLenum texture)
{
    #define MAP_ENUM(x) case x: return #x;

    switch (texture) {
        MAP_ENUM(GL_TEXTURE0)
        MAP_ENUM(GL_TEXTURE1)
        MAP_ENUM(GL_TEXTURE2)
        MAP_ENUM(GL_TEXTURE3)
        MAP_ENUM(GL_TEXTURE4)
        MAP_ENUM(GL_TEXTURE5)
        MAP_ENUM(GL_TEXTURE6)
        MAP_ENUM(GL_TEXTURE7)
        MAP_ENUM(GL_TEXTURE8)
        MAP_ENUM(GL_TEXTURE9)
        MAP_ENUM(GL_TEXTURE10)
        MAP_ENUM(GL_TEXTURE11)
        MAP_ENUM(GL_TEXTURE12)
        MAP_ENUM(GL_TEXTURE13)
        MAP_ENUM(GL_TEXTURE14)
        MAP_ENUM(GL_TEXTURE15)
        MAP_ENUM(GL_TEXTURE16)
        MAP_ENUM(GL_TEXTURE17)
        MAP_ENUM(GL_TEXTURE18)
        MAP_ENUM(GL_TEXTURE19)
        MAP_ENUM(GL_TEXTURE20)
        MAP_ENUM(GL_TEXTURE21)
        MAP_ENUM(GL_TEXTURE22)
        MAP_ENUM(GL_TEXTURE23)
        MAP_ENUM(GL_TEXTURE24)
        MAP_ENUM(GL_TEXTURE25)
        MAP_ENUM(GL_TEXTURE26)
        MAP_ENUM(GL_TEXTURE27)
        MAP_ENUM(GL_TEXTURE28)
        MAP_ENUM(GL_TEXTURE29)
        MAP_ENUM(GL_TEXTURE30)
        MAP_ENUM(GL_TEXTURE31)
    }

    #undef MAP_ENUM

    return "Unknown texture";
}

static const char* decodeValue(const GLenum value)
{
    #define MAP_ENUM(x) case x: return #x;

    switch (value) {
        MAP_ENUM(GL_ACTIVE_ATTRIBUTE_MAX_LENGTH)
        MAP_ENUM(GL_ACTIVE_ATTRIBUTES)
        MAP_ENUM(GL_ACTIVE_TEXTURE)
        MAP_ENUM(GL_ACTIVE_UNIFORM_MAX_LENGTH)
        MAP_ENUM(GL_ACTIVE_UNIFORMS)
        MAP_ENUM(GL_ALIASED_POINT_SIZE_RANGE)
        MAP_ENUM(GL_ALIASED_LINE_WIDTH_RANGE)
        MAP_ENUM(GL_ALPHA)
        MAP_ENUM(GL_ALPHA_BITS)
        MAP_ENUM(GL_ALWAYS)
        MAP_ENUM(GL_ARRAY_BUFFER)
        MAP_ENUM(GL_ARRAY_BUFFER_BINDING)
        MAP_ENUM(GL_ATTACHED_SHADERS)
        MAP_ENUM(GL_BACK)
        MAP_ENUM(GL_BLEND_COLOR)
        MAP_ENUM(GL_BLEND_DST_ALPHA)
        MAP_ENUM(GL_BLEND_DST_RGB)
        MAP_ENUM(GL_BLEND_EQUATION_ALPHA)
        MAP_ENUM(GL_BLEND_EQUATION_RGB)
        MAP_ENUM(GL_BLEND_SRC_ALPHA)
        MAP_ENUM(GL_BLEND_SRC_RGB)
        MAP_ENUM(GL_BLUE_BITS)
        MAP_ENUM(GL_BOOL)
        MAP_ENUM(GL_BOOL_VEC2)
        MAP_ENUM(GL_BOOL_VEC3)
        MAP_ENUM(GL_BOOL_VEC4)
        MAP_ENUM(GL_BUFFER_ACCESS)
        MAP_ENUM(GL_BUFFER_MAPPED)
        MAP_ENUM(GL_BUFFER_MAP_POINTER_OES)
        MAP_ENUM(GL_BUFFER_SIZE)
        MAP_ENUM(GL_BUFFER_USAGE)
        MAP_ENUM(GL_BYTE)
        MAP_ENUM(GL_CCW)
        MAP_ENUM(GL_CLAMP_TO_EDGE)
        MAP_ENUM(GL_COLOR_ATTACHMENT0)
        MAP_ENUM(GL_COLOR_CLEAR_VALUE)
        MAP_ENUM(GL_COLOR_WRITEMASK)
        MAP_ENUM(GL_COMPILE_STATUS)
        MAP_ENUM(GL_COMPRESSED_TEXTURE_FORMATS)
        MAP_ENUM(GL_CONSTANT_ALPHA)
        MAP_ENUM(GL_CONSTANT_COLOR)
        MAP_ENUM(GL_CULL_FACE_MODE)
        MAP_ENUM(GL_CURRENT_PROGRAM)
        MAP_ENUM(GL_CURRENT_VERTEX_ATTRIB)
        MAP_ENUM(GL_CW)
        MAP_ENUM(GL_DECR)
        MAP_ENUM(GL_DECR_WRAP)
        MAP_ENUM(GL_DELETE_STATUS)
        MAP_ENUM(GL_DEPTH_BITS)
        MAP_ENUM(GL_DEPTH_ATTACHMENT)
        MAP_ENUM(GL_DEPTH_CLEAR_VALUE)
        MAP_ENUM(GL_DEPTH_COMPONENT16)
        MAP_ENUM(GL_DEPTH_COMPONENT24_OES)
        MAP_ENUM(GL_DEPTH_COMPONENT32_OES)
        MAP_ENUM(GL_DEPTH_FUNC)
        MAP_ENUM(GL_DEPTH_WRITEMASK)
        MAP_ENUM(GL_DEPTH_RANGE)
        MAP_ENUM(GL_DEPTH24_STENCIL8_OES)
        MAP_ENUM(GL_DONT_CARE)
        MAP_ENUM(GL_DST_ALPHA)
        MAP_ENUM(GL_DST_COLOR)
        MAP_ENUM(GL_DYNAMIC_DRAW)
        MAP_ENUM(GL_ELEMENT_ARRAY_BUFFER)
        MAP_ENUM(GL_ELEMENT_ARRAY_BUFFER_BINDING)
        MAP_ENUM(GL_EQUAL)
        MAP_ENUM(GL_EXTENSIONS)
        MAP_ENUM(GL_FASTEST)
        MAP_ENUM(GL_FILL)
        MAP_ENUM(GL_FIXED)
        MAP_ENUM(GL_FLOAT)
        MAP_ENUM(GL_FLOAT_VEC2)
        MAP_ENUM(GL_FLOAT_VEC3)
        MAP_ENUM(GL_FLOAT_VEC4)
        MAP_ENUM(GL_FLOAT_MAT2)
        MAP_ENUM(GL_FLOAT_MAT3)
        MAP_ENUM(GL_FLOAT_MAT4)
        MAP_ENUM(GL_FRAGMENT_SHADER)
        MAP_ENUM(GL_FRAGMENT_SHADER_DERIVATIVE_HINT_OES)
        MAP_ENUM(GL_FRAMEBUFFER)
        MAP_ENUM(GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME)
        MAP_ENUM(GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE)
        MAP_ENUM(GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE)
        MAP_ENUM(GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL)
        MAP_ENUM(GL_FRAMEBUFFER_BINDING)
        MAP_ENUM(GL_FRAMEBUFFER_COMPLETE)
        MAP_ENUM(GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT)
        MAP_ENUM(GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT)
        MAP_ENUM(GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS)
        //MAP_ENUM(GL_FRAMEBUFFER_INCOMPLETE_FORMATS) - missing?
        MAP_ENUM(GL_FRAMEBUFFER_UNSUPPORTED)
        MAP_ENUM(GL_FRONT)
        MAP_ENUM(GL_FRONT_FACE)
        MAP_ENUM(GL_FRONT_AND_BACK)
        MAP_ENUM(GL_FUNC_ADD)
        MAP_ENUM(GL_FUNC_REVERSE_SUBTRACT)
        MAP_ENUM(GL_FUNC_SUBTRACT)
        MAP_ENUM(GL_GENERATE_MIPMAP_HINT)
        MAP_ENUM(GL_GEQUAL)
        MAP_ENUM(GL_GREATER)
        MAP_ENUM(GL_GREEN_BITS)
        MAP_ENUM(GL_HALF_FLOAT_OES)
        MAP_ENUM(GL_HIGH_FLOAT)
        MAP_ENUM(GL_HIGH_INT)
        MAP_ENUM(GL_IMPLEMENTATION_COLOR_READ_FORMAT)
        MAP_ENUM(GL_IMPLEMENTATION_COLOR_READ_TYPE)
        MAP_ENUM(GL_INCR)
        MAP_ENUM(GL_INCR_WRAP)
        MAP_ENUM(GL_INFO_LOG_LENGTH)
        MAP_ENUM(GL_INT)
        MAP_ENUM(GL_INT_VEC2)
        MAP_ENUM(GL_INT_VEC3)
        MAP_ENUM(GL_INT_VEC4)
        MAP_ENUM(GL_INVERT)
        MAP_ENUM(GL_KEEP)
        MAP_ENUM(GL_LEQUAL)
        MAP_ENUM(GL_LESS)
        MAP_ENUM(GL_LINE)
        MAP_ENUM(GL_LINEAR)
        MAP_ENUM(GL_LINEAR_MIPMAP_NEAREST)
        MAP_ENUM(GL_LINEAR_MIPMAP_LINEAR)
        MAP_ENUM(GL_LINK_STATUS)
        MAP_ENUM(GL_LOW_FLOAT)
        MAP_ENUM(GL_LOW_INT)
        MAP_ENUM(GL_LUMINANCE)
        MAP_ENUM(GL_LUMINANCE_ALPHA)
        MAP_ENUM(GL_MAX_CUBE_MAP_TEXTURE_SIZE)
        MAP_ENUM(GL_MAX_TEXTURE_SIZE)
        MAP_ENUM(GL_MAX_VIEWPORT_DIMS)
        MAP_ENUM(GL_MEDIUM_FLOAT)
        MAP_ENUM(GL_MEDIUM_INT)
        MAP_ENUM(GL_MIRRORED_REPEAT)
        MAP_ENUM(GL_NEAREST)
        MAP_ENUM(GL_NEAREST_MIPMAP_NEAREST)
        MAP_ENUM(GL_NEAREST_MIPMAP_LINEAR)
        MAP_ENUM(GL_NEVER)
        MAP_ENUM(GL_NICEST)
        // MAP_ENUM(GL_NONE)
        MAP_ENUM(GL_NOTEQUAL)
        MAP_ENUM(GL_NUM_COMPRESSED_TEXTURE_FORMATS)
        MAP_ENUM(GL_ONE)
        MAP_ENUM(GL_ONE_MINUS_CONSTANT_ALPHA)
        MAP_ENUM(GL_ONE_MINUS_CONSTANT_COLOR)
        MAP_ENUM(GL_ONE_MINUS_DST_ALPHA)
        MAP_ENUM(GL_ONE_MINUS_DST_COLOR)
        MAP_ENUM(GL_ONE_MINUS_SRC_ALPHA)
        MAP_ENUM(GL_ONE_MINUS_SRC_COLOR)
        MAP_ENUM(GL_PACK_ALIGNMENT)
        MAP_ENUM(GL_POINT)
        MAP_ENUM(GL_POLYGON_OFFSET_FACTOR)
        MAP_ENUM(GL_POLYGON_OFFSET_UNITS)
        MAP_ENUM(GL_RED_BITS)
        MAP_ENUM(GL_RENDERBUFFER)
        MAP_ENUM(GL_RENDERBUFFER_ALPHA_SIZE)
        MAP_ENUM(GL_RENDERBUFFER_BINDING)
        MAP_ENUM(GL_RENDERBUFFER_BLUE_SIZE)
        MAP_ENUM(GL_RENDERBUFFER_DEPTH_SIZE)
        MAP_ENUM(GL_RENDERBUFFER_GREEN_SIZE)
        MAP_ENUM(GL_RENDERBUFFER_HEIGHT)
        MAP_ENUM(GL_RENDERBUFFER_INTERNAL_FORMAT)
        MAP_ENUM(GL_RENDERBUFFER_RED_SIZE)
        MAP_ENUM(GL_RENDERBUFFER_STENCIL_SIZE)
        MAP_ENUM(GL_RENDERBUFFER_WIDTH)
        MAP_ENUM(GL_RENDERER)
        MAP_ENUM(GL_REPEAT)
        MAP_ENUM(GL_REPLACE)
        MAP_ENUM(GL_RGB)
        MAP_ENUM(GL_RGB5_A1)
        MAP_ENUM(GL_RGB565)
        MAP_ENUM(GL_RGB8_OES)
        MAP_ENUM(GL_RGBA)
        MAP_ENUM(GL_RGBA4)
        MAP_ENUM(GL_RGBA8_OES)
        MAP_ENUM(GL_SAMPLE_COVERAGE_VALUE)
        MAP_ENUM(GL_SAMPLE_COVERAGE_INVERT)
        MAP_ENUM(GL_SAMPLER_2D)
        MAP_ENUM(GL_SAMPLER_CUBE)
        MAP_ENUM(GL_SCISSOR_BOX)
        MAP_ENUM(GL_SHADER_SOURCE_LENGTH)
        MAP_ENUM(GL_SHADER_TYPE)
        MAP_ENUM(GL_SHADING_LANGUAGE_VERSION)
        MAP_ENUM(GL_SHORT)
        MAP_ENUM(GL_SRC_ALPHA)
        MAP_ENUM(GL_SRC_ALPHA_SATURATE)
        MAP_ENUM(GL_SRC_COLOR)
        MAP_ENUM(GL_STATIC_DRAW)
        MAP_ENUM(GL_STENCIL_ATTACHMENT)
        MAP_ENUM(GL_STENCIL_BACK_FAIL)
        MAP_ENUM(GL_STENCIL_BACK_FUNC)
        MAP_ENUM(GL_STENCIL_BACK_PASS_DEPTH_FAIL)
        MAP_ENUM(GL_STENCIL_BACK_PASS_DEPTH_PASS)
        MAP_ENUM(GL_STENCIL_BACK_REF)
        MAP_ENUM(GL_STENCIL_BACK_VALUE_MASK)
        MAP_ENUM(GL_STENCIL_BACK_WRITEMASK)
        MAP_ENUM(GL_STENCIL_BITS)
        MAP_ENUM(GL_STENCIL_CLEAR_VALUE)
        MAP_ENUM(GL_STENCIL_FAIL)
        MAP_ENUM(GL_STENCIL_FUNC)
        MAP_ENUM(GL_STENCIL_INDEX1_OES)
        MAP_ENUM(GL_STENCIL_INDEX4_OES)
        MAP_ENUM(GL_STENCIL_INDEX8)
        MAP_ENUM(GL_STENCIL_PASS_DEPTH_FAIL)
        MAP_ENUM(GL_STENCIL_REF)
        MAP_ENUM(GL_STENCIL_VALUE_MASK)
        MAP_ENUM(GL_STENCIL_WRITEMASK)
        MAP_ENUM(GL_STREAM_DRAW)
        MAP_ENUM(GL_SUBPIXEL_BITS)
        MAP_ENUM(GL_TEXTURE)
        MAP_ENUM(GL_TEXTURE_2D)
        MAP_ENUM(GL_TEXTURE_BINDING_2D)
        MAP_ENUM(GL_TEXTURE_BINDING_CUBE_MAP)
        MAP_ENUM(GL_TEXTURE_CUBE_MAP)
        MAP_ENUM(GL_TEXTURE_CUBE_MAP_NEGATIVE_X)
        MAP_ENUM(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y)
        MAP_ENUM(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z)
        MAP_ENUM(GL_TEXTURE_CUBE_MAP_POSITIVE_X)
        MAP_ENUM(GL_TEXTURE_CUBE_MAP_POSITIVE_Y)
        MAP_ENUM(GL_TEXTURE_CUBE_MAP_POSITIVE_Z)
        MAP_ENUM(GL_TEXTURE_MAG_FILTER)
        MAP_ENUM(GL_TEXTURE_MIN_FILTER)
        MAP_ENUM(GL_TEXTURE_WRAP_S)
        MAP_ENUM(GL_TEXTURE_WRAP_T)
        MAP_ENUM(GL_UNPACK_ALIGNMENT)
        MAP_ENUM(GL_UNSIGNED_BYTE)
        MAP_ENUM(GL_UNSIGNED_INT)
        MAP_ENUM(GL_UNSIGNED_SHORT)
        MAP_ENUM(GL_UNSIGNED_SHORT_4_4_4_4)
        MAP_ENUM(GL_UNSIGNED_SHORT_5_5_5_1)
        MAP_ENUM(GL_UNSIGNED_SHORT_5_6_5)
        MAP_ENUM(GL_VALIDATE_STATUS)
        MAP_ENUM(GL_VENDOR)
        MAP_ENUM(GL_VERSION)
        MAP_ENUM(GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING)
        MAP_ENUM(GL_VERTEX_ATTRIB_ARRAY_ENABLED)
        MAP_ENUM(GL_VERTEX_ATTRIB_ARRAY_NORMALIZED)
        MAP_ENUM(GL_VERTEX_ATTRIB_ARRAY_POINTER)
        MAP_ENUM(GL_VERTEX_ATTRIB_ARRAY_SIZE)
        MAP_ENUM(GL_VERTEX_ATTRIB_ARRAY_STRIDE)
        MAP_ENUM(GL_VERTEX_ATTRIB_ARRAY_TYPE)
        MAP_ENUM(GL_VERTEX_SHADER)
        MAP_ENUM(GL_VIEWPORT)
        MAP_ENUM(GL_ZERO)

        // GL_ARB_provoking_vertex
        MAP_ENUM(GL_FIRST_VERTEX_CONVENTION)
        MAP_ENUM(GL_LAST_VERTEX_CONVENTION)
        MAP_ENUM(GL_PROVOKING_VERTEX)
        MAP_ENUM(GL_QUADS_FOLLOW_PROVOKING_VERTEX_CONVENTION)

        // GL_ARB_texture_mirror_clamp_to_edge
        MAP_ENUM(GL_TEXTURE_WRAP_R_OES)
        MAP_ENUM(GL_MIRROR_CLAMP_TO_EDGE)

        // GL_ARB_texture_rectangle
        MAP_ENUM(GL_TEXTURE_RECTANGLE)
        MAP_ENUM(GL_TEXTURE_BINDING_RECTANGLE)
        MAP_ENUM(GL_PROXY_TEXTURE_RECTANGLE)
        MAP_ENUM(GL_MAX_RECTANGLE_TEXTURE_SIZE)
        MAP_ENUM(GL_SAMPLER_2D_RECT)
        // MAP_ENUM(GL_SAMPLER_2D_RECT_SHADOW) - may be added later?

        // GL_EXT_blend_minmax
        // MAP_ENUM(GL_FUNC_ADD)
        MAP_ENUM(GL_MIN)
        MAP_ENUM(GL_MAX)
        // MAP_ENUM(GL_BLEND_EQUATION)

        // GL_EXT_texture_filter_anisotropic
        MAP_ENUM(GL_TEXTURE_MAX_ANISOTROPY_EXT)
        MAP_ENUM(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT)

        // GL_EXT_texture_format_BGRA8888
        MAP_ENUM(GL_BGRA_EXT)

        // GL_EXT_texture_lod_bias
        // MAP_ENUM(GL_TEXTURE_FILTER_CONTROL_EXT) - there is no texture env
        MAP_ENUM(GL_TEXTURE_LOD_BIAS)
        MAP_ENUM(GL_MAX_TEXTURE_LOD_BIAS)

        // GL_OES_element_index_uint
        // MAP_ENUM(GL_UNSIGNED_INT)

        // GL_OES_texture_float
        // MAP_ENUM(GL_HALF_FLOAT_OES)
        // MAP_ENUM(GL_FLOAT)

        // GL_OES_get_program_binary
        MAP_ENUM(GL_PROGRAM_BINARY_LENGTH_OES)
        MAP_ENUM(GL_NUM_PROGRAM_BINARY_FORMATS_OES)
        MAP_ENUM(GL_PROGRAM_BINARY_FORMATS_OES)

        // GL_OES_mapbuffer
        MAP_ENUM(GL_WRITE_ONLY_OES)
        // MAP_ENUM(GL_BUFFER_ACCESS_OES)
        // MAP_ENUM(GL_BUFFER_MAPPED_OES)
        // MAP_ENUM(GL_BUFFER_MAP_POINTER_OES)

        // GL_OES_packed_depth_stencil
        MAP_ENUM(GL_DEPTH_STENCIL_OES)
        MAP_ENUM(GL_UNSIGNED_INT_24_8_OES)
        //MAP_ENUM(GL_DEPTH24_STENCIL8_OES)

        // GL_SGIS_texture_lod
        MAP_ENUM(GL_TEXTURE_MIN_LOD)
        MAP_ENUM(GL_TEXTURE_MAX_LOD)
        MAP_ENUM(GL_TEXTURE_BASE_LEVEL)
        MAP_ENUM(GL_TEXTURE_MAX_LEVEL)

        //MAP_ENUM(GL_)
        //MAP_ENUM(GL_)
        //MAP_ENUM(GL_)
        //MAP_ENUM(GL_)
    }

    #undef MAP_ENUM

    return "Unknown enum";
}

static const char* decodeCapability(const GLenum value)
{
    #define MAP_ENUM(x) case x: return #x;

    switch (value) {
        MAP_ENUM(GL_CULL_FACE)
        MAP_ENUM(GL_POLYGON_OFFSET_FILL)
        MAP_ENUM(GL_SCISSOR_TEST)
        MAP_ENUM(GL_SAMPLE_COVERAGE)
        MAP_ENUM(GL_SAMPLE_ALPHA_TO_COVERAGE)
        // TODO: MAP_ENUM(GL_SAMPLE_COVERAGE_TO_ALPHA) ? Typo?
        MAP_ENUM(GL_STENCIL_TEST)
        MAP_ENUM(GL_DEPTH_TEST)
        MAP_ENUM(GL_BLEND)
        MAP_ENUM(GL_DITHER)
        //MAP_ENUM()
    }

    #undef MAP_ENUM

    return "Unknown capability";
}

static const char* decodePrimitive(const GLenum value)
{
    #define MAP_ENUM(x) case x: return #x;

    switch (value) {
        MAP_ENUM(GL_LINES)
        MAP_ENUM(GL_LINE_LOOP)
        MAP_ENUM(GL_LINE_STRIP)
        MAP_ENUM(GL_POINTS)
        MAP_ENUM(GL_TRIANGLES)
        MAP_ENUM(GL_TRIANGLE_STRIP)
        MAP_ENUM(GL_TRIANGLE_FAN)
    }

    #undef MAP_ENUM

    return "Unknown primitive";
}

#define MAX_GL_ERRORS 10

// Store original function pointers so that they can be still called
struct Ogles2Context
{
    struct OGLES2IFace* interface;
    struct Task* task;
    char name[NAME_LEN];
    char tagBuffer[TAG_BUFFER_LEN];

    MyClock start;
    uint64 ticks;
    ProfilingItem profiling[Ogles2FunctionCount];

    PrimitiveCounter counter;

    GLenum errors[MAX_GL_ERRORS];
    size_t errorRead;
    size_t errorWritten;

    void* (*old_aglCreateContext_AVOID)(struct OGLES2IFace *Self, ULONG * errcode, struct TagItem * tags);
    void* (*old_aglCreateContext2)(struct OGLES2IFace *Self, ULONG * errcode, struct TagItem * tags);
    void (*old_aglDestroyContext)(struct OGLES2IFace *Self, void* context);
    void* (*old_aglGetProcAddress)(struct OGLES2IFace *Self, const char *name);
    void (*old_aglMakeCurrent)(struct OGLES2IFace *Self, void* context);
    void (*old_aglSetBitmap)(struct OGLES2IFace *Self, struct BitMap *bitmap);
    void (*old_aglSetParams_AVOID)(struct OGLES2IFace *Self, struct TagItem * tags);
    void (*old_aglSetParams2)(struct OGLES2IFace *Self, struct TagItem * tags);
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
    void (*old_glDrawElementsBaseVertexOES)(struct OGLES2IFace *Self, GLenum mode, GLsizei count, GLenum type, const void * indices, GLint basevertex);
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
    void (*old_glGetBufferParameterivOES)(struct OGLES2IFace *Self, GLenum target, GLenum value, GLint *data);
    void (*old_glGetBufferPointervOES)(struct OGLES2IFace *Self, GLenum target, GLenum pname, void **params);
    GLenum (*old_glGetError)(struct OGLES2IFace *Self);
    void (*old_glGetFloatv)(struct OGLES2IFace *Self, GLenum pname, GLfloat * data);
    void (*old_glGetFramebufferAttachmentParameteriv)(struct OGLES2IFace *Self, GLenum target, GLenum attachment, GLenum pname, GLint * params);
    void (*old_glGetIntegerv)(struct OGLES2IFace *Self, GLenum pname, GLint * data);
    void (*old_glGetProgramBinaryOES)(struct OGLES2IFace *Self, GLuint program, GLsizei bufSize, GLsizei *length, GLenum *binaryFormat, void *binary);
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
    void* (*old_glMapBufferOES)(struct OGLES2IFace *Self, GLenum target, GLenum access);
    void (*old_glPixelStorei)(struct OGLES2IFace *Self, GLenum pname, GLint param);
    void (*old_glPolygonMode)(struct OGLES2IFace *Self, GLenum face, GLenum mode);
    void (*old_glPolygonOffset)(struct OGLES2IFace *Self, GLfloat factor, GLfloat units);
    void (*old_glProgramBinaryOES)(struct OGLES2IFace *Self, GLuint program, GLenum binaryFormat, const void *binary, GLint length);
    void (*old_glProvokingVertex)(struct OGLES2IFace *Self, GLenum provokeMode);
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
    GLboolean (*old_glUnmapBufferOES)(struct OGLES2IFace *Self, GLenum target);
    void (*old_glUseProgram)(struct OGLES2IFace *Self, GLuint program);
    void (*old_glValidateProgram)(struct OGLES2IFace *Self, GLuint program);
    void (*old_glVertexAttrib1f)(struct OGLES2IFace *Self, GLuint index, GLfloat x);
    void (*old_glVertexAttrib1fv)(struct OGLES2IFace *Self, GLuint index, const GLfloat * v);
    void (*old_glVertexAttrib2f)(struct OGLES2IFace *Self, GLuint index, GLfloat x, GLfloat y);
    void (*old_glVertexAttrib2fv)(struct OGLES2IFace *Self, GLuint index, const GLfloat * v);
    void (*old_glVertexAttrib3f)(struct OGLES2IFace *Self, GLuint index, GLfloat x, GLfloat y, GLfloat z);
    void (*old_glVertexAttrib3fv)(struct OGLES2IFace *Self, GLuint index, const GLfloat * v);
    void (*old_glVertexAttrib4f)(struct OGLES2IFace *Self, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
    void (*old_glVertexAttrib4fv)(struct OGLES2IFace *Self, GLuint index, const GLfloat * v);
    void (*old_glVertexAttribPointer)(struct OGLES2IFace *Self, GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void * pointer);
    void (*old_glViewport)(struct OGLES2IFace *Self, GLint x, GLint y, GLsizei width, GLsizei height);
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

static const char* decodeTags(struct TagItem* tags, struct Ogles2Context* context)
{
    struct TagItem* iter = tags;
    struct TagItem* tag;

    char* dest = context->tagBuffer;
    size_t left = sizeof(context->tagBuffer);

    char temp[64];

    dest[0] = '\0';

    #define TAG_U32(x) case x: snprintf(temp, sizeof(temp), "[" #x ": %lu]", tag->ti_Data); break;
    #define TAG_HEX(x) case x: snprintf(temp, sizeof(temp), "[" #x ": 0x%lX]", tag->ti_Data); break;

    while ((tag = IUtility->NextTagItem(&iter))) {
        switch (tag->ti_Tag) {
            TAG_HEX(OGLES2_CCT_MIN)
            TAG_HEX(OGLES2_CCT_WINDOW)
            TAG_HEX(OGLES2_CCT_MODEID)
            TAG_U32(OGLES2_CCT_DEPTH)
            TAG_U32(OGLES2_CCT_STENCIL)
            TAG_U32(OGLES2_CCT_VSYNC)
            TAG_U32(OGLES2_CCT_SINGLE_GET_ERROR_MODE)
            TAG_HEX(OGLES2_CCT_GET_WIDTH)
            TAG_HEX(OGLES2_CCT_GET_HEIGHT)
            TAG_HEX(OGLES2_CCT_BITMAP)
            TAG_U32(OGLES2_CCT_SHADER_COMPAT_PATCH)
            TAG_HEX(OGLES2_CCT_CONTEXT_FOR_MODEID)
            TAG_U32(OGLES2_CCT_RESIZE_VIEWPORT)
            TAG_U32(OGLES2_CCT_DEBUG_SHADER_LOG)
            TAG_U32(OGLES2_CCT_DETECT_UNINITIALIZED_GLSL_VARS)
            TAG_U32(OGLES2_CCT_SPIRV_OPLINES)
            TAG_U32(OGLES2_CCT_SPIRV_OPLINES_OFFSET)
            TAG_U32(OGLES2_CCT_SPIRV_OPTIMIZE)
            TAG_U32(OGLES2_CCT_SHARE_WITH)
            default:
                snprintf(temp, sizeof(temp), "[Unknown tag %lu]", tag->ti_Tag);
                break;
        }

        snprintf(dest, left, "%s", temp);

        const size_t len = strlen(temp);
        left -= len;
        dest += len;
    }

    #undef TAG_U32
    #undef TAG_HEX

    return context->tagBuffer;
}

static void checkPointer(struct Ogles2Context* context, const Ogles2Function id, const void* ptr)
{
    if (!ptr) {
        logLine("%s: Warning: NULL pointer detected", context->name);
        context->profiling[id].errors++;
        errorCount++;
    }
}

static void profileResults(struct Ogles2Context* const context)
{
    if (!profilingStarted) {
        logAlways("OGLES2 profiling not started, skip summary");
        return;
    }

    PROF_FINISH_CONTEXT

    const double drawcalls = (double)(context->profiling[DrawElements].callCount + context->profiling[DrawArrays].callCount +
        context->profiling[DrawElementsBaseVertexOES].callCount);

    const double swaps = (double)context->profiling[SwapBuffers].callCount;

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
                timer_ticks_to_us(stats[i].ticks) / (double)stats[i].callCount,
                (double)stats[i].ticks * 100.0 / (double)context->ticks,
                (double)stats[i].ticks * 100.0 / (double)totalTicks);
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
                        logAlways("[%u] Patching task %s OGLES2IFace %p", i, context->name, interface);
                        contexts[i] = context;
                        break;
                    }
                }

                IExec->MutexRelease(mutex);

                if (i == MAX_CLIENTS) {
                    logAlways("glSnoop: too many clients, cannot patch");
                    IExec->FreeVec(context);
                } else {
                    patch_ogles2_functions(context);
                    PROF_INIT(context, Ogles2FunctionCount)
                }
            } else {
                logAlways("Cannot allocate memory for OGLES2 context data: cannot patch");
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

            logAlways("%s: dropping patched OGLES2 interface %p [%u]", contexts[i]->name, interface, i);

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

static struct Ogles2Context* find_context(const struct OGLES2IFace * const interface)
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

static void checkErrors(struct Ogles2Context * context, const Ogles2Function id, const char* const name)
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
        const size_t next = (context->errorWritten + 1) % MAX_GL_ERRORS;
        if (next == context->errorRead) {
            logLine("%s: GL error buffer overflow after %s", context->name, name);
        } else {
            context->errors[next] = err;
            context->errorWritten = next;
        }

        logLine("%s: GL error %d (%s) detected after %s", context->name, err, mapOgles2Error(err), name);
        context->profiling[id].errors++;
    }
}

#define GL_CALL(id, ...) \
if (context->old_gl ## id) { \
    PROF_START \
    context->old_gl ## id(Self, ##__VA_ARGS__); \
    PROF_FINISH(id) \
    checkErrors(context, id, #id); \
} else { \
    logDebug("%s: " #id " function pointer is NULL (call ignored)", context->name); \
}

#define AGL_CALL(id, ...) \
if (context->old_agl ## id) { \
    PROF_START \
    context->old_agl ## id(Self, ##__VA_ARGS__); \
    PROF_FINISH(id) \
    checkErrors(context, id, #id); \
} else { \
    logDebug("%s: " #id " function pointer is NULL (call ignored)", context->name); \
}

#define GL_CALL_STATUS(id, ...) \
if (context->old_gl ## id) { \
    PROF_START \
    status = context->old_gl ## id(Self, ##__VA_ARGS__); \
    PROF_FINISH(id) \
    checkErrors(context, id, #id); \
} else { \
    logDebug("%s: " #id " function pointer is NULL (call ignored)", context->name); \
}

#define AGL_CALL_STATUS(id, ...) \
if (context->old_agl ## id) { \
    PROF_START \
    status = context->old_agl ## id(Self, ##__VA_ARGS__); \
    PROF_FINISH(id) \
    checkErrors(context, id, #id); \
} else { \
    logDebug("%s: " #id " function pointer is NULL (call ignored)", context->name); \
}

// Wrap traced function calls

static void* OGLES2_aglCreateContext_AVOID(struct OGLES2IFace *Self, ULONG * errcode, struct TagItem * tags)
{
    GET_CONTEXT

    void* status = NULL;

    ULONG tempErrCode = 0;

    logLine("%s: %s: errcode pointer %p, tags %p (%s)", context->name, __func__,
        errcode, tags, decodeTags(tags, context));

    AGL_CALL_STATUS(CreateContext_AVOID, &tempErrCode, tags)

    logLine("%s: %s: <- errcode %lu. Context address %p", context->name, __func__,
        tempErrCode, status);

    if (errcode) {
        *errcode = tempErrCode;
    }

    return status;
}

static void* OGLES2_aglCreateContext2(struct OGLES2IFace *Self, ULONG * errcode, struct TagItem * tags)
{
    GET_CONTEXT

    void* status = NULL;

    ULONG tempErrCode = 0;

    logLine("%s: %s: errcode pointer %p, tags %p (%s)", context->name, __func__,
        errcode, tags, decodeTags(tags, context));

    AGL_CALL_STATUS(CreateContext2, &tempErrCode, tags)

    logLine("%s: %s: <- errcode %lu. Context address %p", context->name, __func__,
        tempErrCode, status);

    if (errcode) {
        *errcode = tempErrCode;
    }

    return status;
}

static void OGLES2_aglDestroyContext(struct OGLES2IFace *Self, void* context_)
{
    GET_CONTEXT

    logLine("%s: %s: context %p", context->name, __func__,
        context_);

    AGL_CALL(DestroyContext, context_)
}

static void* OGLES2_aglGetProcAddress(struct OGLES2IFace *Self, const char *name)
{
    GET_CONTEXT

    void* status = NULL;

    logLine("%s: %s: name '%s'", context->name, __func__,
        name);

    AGL_CALL_STATUS(GetProcAddress, name)

    logLine("%s: %s: <- address %p", context->name, __func__,
        status);

    return status;
}

static void OGLES2_aglMakeCurrent(struct OGLES2IFace *Self, void* context_)
{
    GET_CONTEXT

    logLine("%s: %s: context %p", context->name, __func__,
        context_);

    AGL_CALL(MakeCurrent, context_)
}

static void OGLES2_aglSetBitmap(struct OGLES2IFace *Self, struct BitMap *bitmap)
{
    GET_CONTEXT

    logLine("%s: %s: bitmap %p", context->name, __func__,
        bitmap);

    AGL_CALL(SetBitmap, bitmap)
}

static void OGLES2_aglSetParams_AVOID(struct OGLES2IFace *Self, struct TagItem * tags)
{
    GET_CONTEXT

    logLine("%s: %s: tags %p (%s)", context->name, __func__,
        tags, decodeTags(tags, context));

    AGL_CALL(SetParams_AVOID, tags)
}

static void OGLES2_aglSetParams2(struct OGLES2IFace *Self, struct TagItem * tags)
{
    GET_CONTEXT

    logLine("%s: %s: tags %p (%s)", context->name, __func__,
        tags, decodeTags(tags, context));

    AGL_CALL(SetParams2, tags)
}

static void OGLES2_aglSwapBuffers(struct OGLES2IFace *Self)
{
    GET_CONTEXT

    logLine("%s: %s", context->name, __func__);

    AGL_CALL(SwapBuffers)
}

static void OGLES2_glActiveTexture(struct OGLES2IFace *Self, GLenum texture)
{
    GET_CONTEXT

    logLine("%s: %s: texture 0x%X (%s)", context->name, __func__,
        texture, decodeTexture(texture));

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

    logLine("%s: %s: target 0x%X (%s), buffer %u", context->name, __func__,
        target, decodeValue(target),
        buffer);

    GL_CALL(BindBuffer, target, buffer)
}

static void OGLES2_glBindFramebuffer(struct OGLES2IFace *Self, GLenum target, GLuint framebuffer)
{
    GET_CONTEXT

    logLine("%s: %s: target 0x%X (%s), framebuffer %u", context->name, __func__,
        target, decodeValue(target),
        framebuffer);

    GL_CALL(BindFramebuffer, target, framebuffer)
}

static void OGLES2_glBindRenderbuffer(struct OGLES2IFace *Self, GLenum target, GLuint renderbuffer)
{
    GET_CONTEXT

    logLine("%s: %s: target 0x%X (%s), renderbuffer %u", context->name, __func__,
        target, decodeValue(target),
        renderbuffer);

    GL_CALL(BindRenderbuffer, target, renderbuffer)
}

static void OGLES2_glBindTexture(struct OGLES2IFace *Self, GLenum target, GLuint texture)
{
    GET_CONTEXT

    logLine("%s: %s: target 0x%X (%s), texture %d", context->name, __func__,
        target, decodeValue(target),
        texture);

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

    logLine("%s: %s: mode 0x%X (%s)", context->name, __func__,
        mode, decodeValue(mode));

    GL_CALL(BlendEquation, mode)
}

static void OGLES2_glBlendEquationSeparate(struct OGLES2IFace *Self, GLenum modeRGB, GLenum modeAlpha)
{
    GET_CONTEXT

    logLine("%s: %s: modeRGB 0x%X (%s), modeAlpha 0x%X (%s)", context->name, __func__,
        modeRGB, decodeValue(modeRGB),
        modeAlpha, decodeValue(modeAlpha));

    GL_CALL(BlendEquationSeparate, modeRGB, modeAlpha)
}

static void OGLES2_glBlendFunc(struct OGLES2IFace *Self, GLenum sfactor, GLenum dfactor)
{
    GET_CONTEXT

    logLine("%s: %s: sfactor 0x%X (%s), dfactor 0x%X (%s)", context->name, __func__,
        sfactor, decodeValue(sfactor),
        dfactor, decodeValue(dfactor));

    GL_CALL(BlendFunc, sfactor, dfactor)
}

static void OGLES2_glBlendFuncSeparate(struct OGLES2IFace *Self, GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha)
{
    GET_CONTEXT

    logLine("%s: %s: sfactorRGB 0x%X (%s), dfactorRGB 0x%X (%s), sfactorAlpha 0x%X (%s), dfactorAlpha 0x%X (%s)", context->name, __func__,
        sfactorRGB, decodeValue(sfactorRGB),
        dfactorRGB, decodeValue(dfactorRGB),
        sfactorAlpha, decodeValue(sfactorAlpha),
        dfactorAlpha, decodeValue(dfactorAlpha));

    GL_CALL(BlendFuncSeparate, sfactorRGB, dfactorRGB, sfactorAlpha, dfactorAlpha)
}

static void OGLES2_glBufferData(struct OGLES2IFace *Self, GLenum target, GLsizeiptr size, const void * data, GLenum usage)
{
    GET_CONTEXT

    logLine("%s: %s: target 0x%X (%s), size %u, data %p, usage 0x%X (%s)", context->name, __func__,
        target, decodeValue(target),
        size, data,
        usage, decodeValue(usage));

    GL_CALL(BufferData, target, size, data, usage)
}

static void OGLES2_glBufferSubData(struct OGLES2IFace *Self, GLenum target, GLintptr offset, GLsizeiptr size, const void * data)
{
    GET_CONTEXT

    logLine("%s: %s: target 0x%X (%s), offset %u, size %u, data %p", context->name, __func__,
        target, decodeValue(target),
        offset, size, data);

    GL_CALL(BufferSubData, target, offset, size, data)
}

static GLenum OGLES2_glCheckFramebufferStatus(struct OGLES2IFace *Self, GLenum target)
{
    GET_CONTEXT

    GLenum status = 0;

    logLine("%s: %s: target 0x%X (%s)", context->name, __func__,
        target, decodeValue(target));

    GL_CALL_STATUS(CheckFramebufferStatus, target)

    logLine("%s: %s: <- status 0x%X (%s)", context->name, __func__,
        status, decodeValue(status));

    return status;
}

static void OGLES2_glClear(struct OGLES2IFace *Self, GLbitfield mask)
{
    GET_CONTEXT

    logLine("%s: %s: mask 0x%X %s%s%s", context->name, __func__,
        mask,
        (mask & GL_COLOR_BUFFER_BIT) ? "[COLOR]" : "",
        (mask & GL_DEPTH_BUFFER_BIT) ? "[DEPTH]" : "",
        (mask & GL_STENCIL_BUFFER_BIT) ? "[STENCIL]" : "");

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

    logLine("%s: %s: target 0x%X (%s), level %d, internalformat 0x%X (%s), width %d, height %d, border %d, imageSize %d, data %p", context->name, __func__,
        target, decodeValue(target),
        level,
        internalformat, decodeValue(internalformat),
        width, height, border, imageSize, data);

    GL_CALL(CompressedTexImage2D, target, level, internalformat, width, height, border, imageSize, data)
}

static void OGLES2_glCompressedTexSubImage2D(struct OGLES2IFace *Self, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void * data)
{
    GET_CONTEXT

    logLine("%s: %s: target 0x%X (%s), level %d, xoffset %d, yoffset %d, width %d, height %d, format 0x%X (%s), imageSize %d, data %p", context->name, __func__,
        target, decodeValue(target),
        level, xoffset, yoffset, width, height,
        format, decodeValue(format),
        imageSize, data);

    GL_CALL(CompressedTexSubImage2D, target, level, xoffset, yoffset, width, height, format, imageSize, data)
}

static void OGLES2_glCopyTexImage2D(struct OGLES2IFace *Self, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border)
{
    GET_CONTEXT

    logLine("%s: %s: target 0x%X (%s), level %d, internalformat 0x%X (%s), x %d, y %d, width %d, height %d, border %d", context->name, __func__,
        target, decodeValue(target),
        level,
        internalformat, decodeValue(internalformat),
        x, y, width, height, border);

    GL_CALL(CopyTexImage2D, target, level, internalformat, x, y, width, height, border)
}

static void OGLES2_glCopyTexSubImage2D(struct OGLES2IFace *Self, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
    GET_CONTEXT

    logLine("%s: %s: target 0x%X (%s), level %d, xoffset %d, yoffset %d, x %d, y %d, width %d, height %d", context->name, __func__,
        target, decodeValue(target),
        level, xoffset, yoffset, x, y, width, height);

    GL_CALL(CopyTexSubImage2D, target, level, xoffset, yoffset, x, y, width, height)
}

static GLuint OGLES2_glCreateProgram(struct OGLES2IFace *Self)
{
    GET_CONTEXT

    GLuint status = 0;

    logLine("%s: %s", context->name, __func__);

    GL_CALL_STATUS(CreateProgram)

    logLine("%s: %s: <- program %u", context->name, __func__,
        status);

    return status;
}

static GLuint OGLES2_glCreateShader(struct OGLES2IFace *Self, GLenum type)
{
    GET_CONTEXT

    GLuint status = 0;

    logLine("%s: %s: type 0x%X (%s)", context->name, __func__,
        type, decodeValue(type));

    GL_CALL_STATUS(CreateShader, type)

    logLine("%s: %s: <- shader %u", context->name, __func__,
        status);

    return status;
}

static void OGLES2_glCullFace(struct OGLES2IFace *Self, GLenum mode)
{
    GET_CONTEXT

    logLine("%s: %s: mode 0x%X (%s)", context->name, __func__,
        mode, decodeValue(mode));

    GL_CALL(CullFace, mode)
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

    logLine("%s: %s: func 0x%X (%s)", context->name, __func__,
        func, decodeValue(func));

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

    logLine("%s: %s: cap 0x%X (%s)", context->name, __func__,
        cap, decodeCapability(cap));

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

    logLine("%s: %s: mode 0x%X (%s), first %d, count %u", context->name, __func__,
        mode, decodePrimitive(mode),
        first, count);

    GL_CALL(DrawArrays, mode, first, count)

    countPrimitive(&context->counter, mode, (size_t)count);
}

static void OGLES2_glDrawElements(struct OGLES2IFace *Self, GLenum mode, GLsizei count, GLenum type, const void * indices)
{
    GET_CONTEXT

    logLine("%s: %s: mode 0x%X (%s), count %u, type 0x%X (%s), indices %p", context->name, __func__,
        mode, decodePrimitive(mode),
        count,
        type, decodeValue(type),
        indices);

    GL_CALL(DrawElements, mode, count, type, indices)

    countPrimitive(&context->counter, mode, (size_t)count);
}

static void OGLES2_glDrawElementsBaseVertexOES(struct OGLES2IFace *Self, GLenum mode, GLsizei count, GLenum type, const void * indices, GLint basevertex)
{
    GET_CONTEXT

    logLine("%s: %s: mode 0x%X (%s), count %u, type 0x%X (%s), indices %p, basevertex %d", context->name, __func__,
        mode, decodePrimitive(mode),
        count,
        type, decodeValue(type),
        indices, basevertex);

    GL_CALL(DrawElementsBaseVertexOES, mode, count, type, indices, basevertex)

    countPrimitive(&context->counter, mode, (size_t)count);
}

static void OGLES2_glEnable(struct OGLES2IFace *Self, GLenum cap)
{
    GET_CONTEXT

    logLine("%s: %s: cap 0x%X (%s)", context->name, __func__,
        cap, decodeCapability(cap));

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

    logLine("%s: %s: target 0x%X (%s), attachment 0x%X (%s), renderbuffertarget 0x%X (%s), renderbuffer %u", context->name, __func__,
        target, decodeValue(target),
        attachment, decodeValue(attachment),
        renderbuffertarget, decodeValue(renderbuffertarget),
        renderbuffer);

    GL_CALL(FramebufferRenderbuffer, target, attachment, renderbuffertarget, renderbuffer)
}

static void OGLES2_glFramebufferTexture2D(struct OGLES2IFace *Self, GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
    GET_CONTEXT

    logLine("%s: %s: target 0x%X (%s), attachment 0x%X (%s), textarget 0x%X (%s), texture %u, level %d", context->name, __func__,
        target, decodeValue(target),
        attachment, decodeValue(attachment),
        textarget, decodeValue(textarget),
        texture, level);

    GL_CALL(FramebufferTexture2D, target, attachment, textarget, texture, level)
}

static void OGLES2_glFrontFace(struct OGLES2IFace *Self, GLenum mode)
{
    GET_CONTEXT

    logLine("%s: %s: mode 0x%X (%s)", context->name, __func__,
        mode, decodeValue(mode));

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

    logLine("%s: %s: target 0x%X (%s)", context->name, __func__,
        target, decodeValue(target));

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

    logLine("%s: %s: program %u, index %u, bufSize %u, length %p, size %p, type %p, name %p", context->name, __func__,
        program, index, bufSize, length, size, type, name);

    GL_CALL(GetActiveAttrib, program, index, bufSize, &tempLength, size, type, name)

    logLine("%s: %s: <- length %u, size %d, type 0x%X (%s), name '%s'", context->name, __func__,
        tempLength, *size,
        *type, decodeValue(*type),
        name);

    if (length) {
        *length = tempLength;
    }
}

static void OGLES2_glGetActiveUniform(struct OGLES2IFace *Self, GLuint program, GLuint index, GLsizei bufSize, GLsizei * length, GLint * size, GLenum * type, GLchar * name)
{
    GET_CONTEXT

    GLsizei tempLength = 0;

    logLine("%s: %s: program %u, index %u, bufSize %u, length %p, size %p, type %p, name %p", context->name, __func__,
        program, index, bufSize, length, size, type, name);

    GL_CALL(GetActiveUniform, program, index, bufSize, &tempLength, size, type, name)

    logLine("%s: %s: <- length %u, size %d, type 0x%X (%s), name '%s'", context->name, __func__,
        tempLength, *size,
        *type, decodeValue(*type),
        name);

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

    logLine("%s: %s: program %u, name '%s'", context->name, __func__,
        program, name);

    GL_CALL_STATUS(GetAttribLocation, program, name)

    logLine("%s: %s: <- location %d", context->name, __func__,
        status);

    return status;
}

static void OGLES2_glGetBooleanv(struct OGLES2IFace *Self, GLenum pname, GLboolean * data)
{
    GET_CONTEXT

    logLine("%s: %s: pname 0x%X (%s), data %p", context->name, __func__,
        pname, decodeValue(pname),
        data);

    GL_CALL(GetBooleanv, pname, data)

    logLine("%s: %s: <- data %d", context->name, __func__,
        *data);
}

static void OGLES2_glGetBufferParameteriv(struct OGLES2IFace *Self, GLenum target, GLenum pname, GLint * params)
{
    GET_CONTEXT

    logLine("%s: %s: target 0x%X (%s), pname 0x%X (%s), params %p", context->name, __func__,
        target, decodeValue(target),
        pname, decodeValue(pname),
        params);

    GL_CALL(GetBufferParameteriv, target, pname, params)

    logLine("%s: %s: <- params %d", context->name, __func__,
        *params);
}

static void OGLES2_glGetBufferParameterivOES(struct OGLES2IFace *Self, GLenum target, GLenum value, GLint *data)
{
    GET_CONTEXT

    logLine("%s: %s: target 0x%X (%s), value 0x%X (%s), data %p", context->name, __func__,
        target, decodeValue(target),
        value, decodeValue(value),
        data);

    GL_CALL(GetBufferParameterivOES, target, value, data)

    logLine("%s: %s: <- data %d", context->name, __func__,
        *data);
}

static void OGLES2_glGetBufferPointervOES(struct OGLES2IFace *Self, GLenum target, GLenum pname, void **params)
{
    GET_CONTEXT

    logLine("%s: %s: target 0x%X (%s), pname 0x%X (%s), params %p", context->name, __func__,
        target, decodeValue(target),
        pname, decodeValue(pname),
        params);

    GL_CALL(GetBufferPointervOES, target, pname, params)

    logLine("%s: %s: <- params %p", context->name, __func__,
        *params);

    checkPointer(context, GetBufferPointervOES, *params);
}

static GLenum OGLES2_glGetError(struct OGLES2IFace *Self)
{
    GET_CONTEXT

    GLenum status = GL_NO_ERROR;

    context->profiling[GetError].callCount++;

    if (context->errorRead != context->errorWritten) {
        context->errorRead = (context->errorRead + 1) % MAX_GL_ERRORS;
        status = context->errors[context->errorRead];
    }

    logLine("%s: %s: <- error 0x%X (%s)", context->name, __func__,
        status, (status == GL_NO_ERROR) ? "GL_NO_ERROR" : mapOgles2Error(status));

    return status;
}

static void OGLES2_glGetFloatv(struct OGLES2IFace *Self, GLenum pname, GLfloat * data)
{
    GET_CONTEXT

    logLine("%s: %s: pname 0x%X (%s), data %p", context->name, __func__,
        pname, decodeValue(pname),
        data);

    GL_CALL(GetFloatv, pname, data)

    logLine("%s: %s: <- data %f", context->name, __func__,
        *data);
}

static void OGLES2_glGetFramebufferAttachmentParameteriv(struct OGLES2IFace *Self, GLenum target, GLenum attachment, GLenum pname, GLint * params)
{
    GET_CONTEXT

    logLine("%s: %s: target 0x%X (%s), attachment 0x%X (%s), pname 0x%X (%s), params %p", context->name, __func__,
        target, decodeValue(target),
        attachment, decodeValue(attachment),
        pname, decodeValue(pname),
        params);

    GL_CALL(GetFramebufferAttachmentParameteriv, target, attachment, pname, params)

    logLine("%s: %s: <- params %d", context->name, __func__,
        *params);
}

static void OGLES2_glGetIntegerv(struct OGLES2IFace *Self, GLenum pname, GLint * data)
{
    GET_CONTEXT

    logLine("%s: %s: pname 0x%X (%s), data %p", context->name, __func__,
        pname, decodeValue(pname),
        data);

    GL_CALL(GetIntegerv, pname, data)

    logLine("%s: %s: <- data %d", context->name, __func__,
        *data);
}

static void OGLES2_glGetProgramBinaryOES(struct OGLES2IFace *Self, GLuint program, GLsizei bufSize, GLsizei *length, GLenum *binaryFormat, void *binary)
{
    GET_CONTEXT

    GLsizei tempLength = 0;

    logLine("%s: %s: program %u, bufSize %u, length %p, binaryFormat %p, binary %p", context->name, __func__,
        program, bufSize, length, binaryFormat, binary);

    GL_CALL(GetProgramBinaryOES, program, bufSize, &tempLength, binaryFormat, binary)

    logLine("%s: %s: <- length %u, binaryFormat 0x%X (%s)", context->name, __func__,
        tempLength,
        *binaryFormat, decodeValue(*binaryFormat));

    if (length) {
        *length = tempLength;
    }
}

static void OGLES2_glGetProgramiv(struct OGLES2IFace *Self, GLuint program, GLenum pname, GLint * params)
{
    GET_CONTEXT

    logLine("%s: %s: program %u, pname 0x%X (%s), params %p", context->name, __func__,
        program,
        pname, decodeValue(pname),
        params);

    GL_CALL(GetProgramiv, program, pname, params)

    logLine("%s: %s: <- params %d", context->name, __func__,
        *params);
}

static void OGLES2_glGetProgramInfoLog(struct OGLES2IFace *Self, GLuint program, GLsizei bufSize, GLsizei * length, GLchar * infoLog)
{
    GET_CONTEXT

    GLsizei tempLength = 0;

    logLine("%s: %s: program %u, bufSize %u, length %p, infoLog %p", context->name, __func__,
        program, bufSize, length, infoLog);

    GL_CALL(GetProgramInfoLog, program, bufSize, &tempLength, infoLog)

    logLine("%s: %s: <- length %u, infoLog '%s'", context->name, __func__,
        tempLength, infoLog);

    if (length) {
        *length = tempLength;
    }
}

static void OGLES2_glGetRenderbufferParameteriv(struct OGLES2IFace *Self, GLenum target, GLenum pname, GLint * params)
{
    GET_CONTEXT

    logLine("%s: %s: target 0x%X (%s), pname 0x%X (%s), params %p", context->name, __func__,
        target, decodeValue(target),
        pname, decodeValue(pname),
        params);

    GL_CALL(GetRenderbufferParameteriv, target, pname, params)

    logLine("%s: %s: <- params %d", context->name, __func__,
        *params);
}

static void OGLES2_glGetShaderiv(struct OGLES2IFace *Self, GLuint shader, GLenum pname, GLint * params)
{
    GET_CONTEXT

    logLine("%s: %s: shader %u, pname 0x%X (%s), params %p", context->name, __func__,
        shader,
        pname, decodeValue(pname),
        params);

    GL_CALL(GetShaderiv, shader, pname, params)

    logLine("%s: %s: <- params %d", context->name, __func__,
        *params);
}

static void OGLES2_glGetShaderInfoLog(struct OGLES2IFace *Self, GLuint shader, GLsizei bufSize, GLsizei * length, GLchar * infoLog)
{
    GET_CONTEXT

    GLsizei tempLength = 0;

    logLine("%s: %s: shader %u, bufSize %u, length %p, infoLog %p", context->name, __func__,
        shader, bufSize, length, infoLog);

    GL_CALL(GetShaderInfoLog, shader, bufSize, &tempLength, infoLog)

    logLine("%s: %s: <- length %u, infoLog '%s'", context->name, __func__,
        tempLength, infoLog);

    if (length) {
        *length = tempLength;
    }
}

static void OGLES2_glGetShaderPrecisionFormat(struct OGLES2IFace *Self, GLenum shadertype, GLenum precisiontype, GLint * range, GLint * precision)
{
    GET_CONTEXT

    logLine("%s: %s: shadertype 0x%X (%s), precisiontype 0x%X (%s), range %p, precision %p", context->name, __func__,
        shadertype, decodeValue(shadertype),
        precisiontype, decodeValue(precisiontype),
        range, precision);

    GL_CALL(GetShaderPrecisionFormat, shadertype, precisiontype, range, precision)

    logLine("%s: %s: <- range [%d, %d], precision %d", context->name, __func__,
        range[0], range[1], *precision);
}

static void OGLES2_glGetShaderSource(struct OGLES2IFace *Self, GLuint shader, GLsizei bufSize, GLsizei * length, GLchar * source)
{
    GET_CONTEXT

    GLsizei tempLength = 0;

    logLine("%s: %s: shader %u, bufSize %u, length %p, source %p", context->name, __func__,
        shader, bufSize, length, source);

    GL_CALL(GetShaderSource, shader, bufSize, &tempLength, source)

    logLine("%s: %s: <- length %u, source '%s'", context->name, __func__,
        tempLength, source);

    if (length) {
        *length = tempLength;
    }
}

static const GLubyte * OGLES2_glGetString(struct OGLES2IFace *Self, GLenum name)
{
    GET_CONTEXT

    const GLubyte *status = NULL;

    logLine("%s: %s: name 0x%X (%s)", context->name, __func__,
        name, decodeValue(name));

    GL_CALL_STATUS(GetString, name)

    logLine("%s: %s: <- string '%s'", context->name, __func__,
        status);

    return status;
}

static void OGLES2_glGetTexParameterfv(struct OGLES2IFace *Self, GLenum target, GLenum pname, GLfloat * params)
{
    GET_CONTEXT

    logLine("%s: %s: target 0x%X (%s), pname 0x%X (%s), params %p", context->name, __func__,
        target, decodeValue(target),
        pname, decodeValue(pname),
        params);

    GL_CALL(GetTexParameterfv, target, pname, params)

    logLine("%s: %s: <- params %f", context->name, __func__,
        *params);
}

static void OGLES2_glGetTexParameteriv(struct OGLES2IFace *Self, GLenum target, GLenum pname, GLint * params)
{
    GET_CONTEXT

    logLine("%s: %s: target 0x%X (%s), pname 0x%X (%s), params %p", context->name, __func__,
        target, decodeValue(target),
        pname, decodeValue(pname),
        params);

    GL_CALL(GetTexParameteriv, target, pname, params)

    logLine("%s: %s: <- params %d", context->name, __func__,
        *params);
}

static void OGLES2_glGetUniformfv(struct OGLES2IFace *Self, GLuint program, GLint location, GLfloat * params)
{
    GET_CONTEXT

    logLine("%s: %s: program %u, location %u, params %p", context->name, __func__,
        program, location, params);

    GL_CALL(GetUniformfv, program, location, params)

    logLine("%s: %s: <- params %f", context->name, __func__,
        *params);
}

static void OGLES2_glGetUniformiv(struct OGLES2IFace *Self, GLuint program, GLint location, GLint * params)
{
    GET_CONTEXT

    logLine("%s: %s: program %u, location %u, params %p", context->name, __func__,
        program, location, params);

    GL_CALL(GetUniformiv, program, location, params)

    logLine("%s: %s: <- params %d", context->name, __func__,
        *params);
}

static GLint OGLES2_glGetUniformLocation(struct OGLES2IFace *Self, GLuint program, const GLchar * name)
{
    GET_CONTEXT

    GLint status = 0;

    logLine("%s: %s: program %u, name '%s'", context->name, __func__,
        program, name);

    GL_CALL_STATUS(GetUniformLocation, program, name)

    logLine("%s: %s: <- location %d", context->name, __func__,
        status);

    return status;
}

static void OGLES2_glGetVertexAttribfv(struct OGLES2IFace *Self, GLuint index, GLenum pname, GLfloat * params)
{
    GET_CONTEXT

    logLine("%s: %s: index %u, pname 0x%X (%s), params %p", context->name, __func__,
        index,
        pname, decodeValue(pname),
        params);

    GL_CALL(GetVertexAttribfv, index, pname, params)

    logLine("%s: %s: <- params %f", context->name, __func__,
        *params);
}

static void OGLES2_glGetVertexAttribiv(struct OGLES2IFace *Self, GLuint index, GLenum pname, GLint * params)
{
    GET_CONTEXT

    logLine("%s: %s: index %u, pname 0x%X (%s), params %p", context->name, __func__,
        index,
        pname, decodeValue(pname),
        params);

    GL_CALL(GetVertexAttribiv, index, pname, params)

    logLine("%s: %s: <- params %d", context->name, __func__,
        *params);
}

static void OGLES2_glGetVertexAttribPointerv(struct OGLES2IFace *Self, GLuint index, GLenum pname, void ** pointer)
{
    GET_CONTEXT

    logLine("%s: %s: index %u, pname 0x%X (%s), pointer %p", context->name, __func__,
        index,
        pname, decodeValue(pname),
        pointer);

    GL_CALL(GetVertexAttribPointerv, index, pname, pointer)

    logLine("%s: %s: <- pointer %p", context->name, __func__,
        *pointer);
}

static void OGLES2_glHint(struct OGLES2IFace *Self, GLenum target, GLenum mode)
{
    GET_CONTEXT

    logLine("%s: %s: target 0x%X (%s), mode 0x%X (%s)", context->name, __func__,
        target, decodeValue(target),
        mode, decodeValue(mode));

    GL_CALL(Hint, target, mode)
}

static GLboolean OGLES2_glIsBuffer(struct OGLES2IFace *Self, GLuint buffer)
{
    GET_CONTEXT

    GLboolean status = GL_FALSE;

    logLine("%s: %s: buffer %u", context->name, __func__,
        buffer);

    GL_CALL_STATUS(IsBuffer, buffer)

    logLine("%s: %s: <- result %d", context->name, __func__,
        status);

    return status;
}

static GLboolean OGLES2_glIsEnabled(struct OGLES2IFace *Self, GLenum cap)
{
    GET_CONTEXT

    GLboolean status = GL_FALSE;

    logLine("%s: %s: cap 0x%X (%s)", context->name, __func__,
        cap, decodeValue(cap));

    GL_CALL_STATUS(IsEnabled, cap)

    logLine("%s: %s: <- result %d", context->name, __func__,
        status);

    return status;
}

static GLboolean OGLES2_glIsFramebuffer(struct OGLES2IFace *Self, GLuint framebuffer)
{
    GET_CONTEXT

    GLboolean status = GL_FALSE;

    logLine("%s: %s: framebuffer %u", context->name, __func__,
        framebuffer);

    GL_CALL_STATUS(IsFramebuffer, framebuffer)

    logLine("%s: %s: <- result %d", context->name, __func__,
        status);

    return status;
}

static GLboolean OGLES2_glIsProgram(struct OGLES2IFace *Self, GLuint program)
{
    GET_CONTEXT

    GLboolean status = GL_FALSE;

    logLine("%s: %s: program %u", context->name, __func__,
        program);

    GL_CALL_STATUS(IsProgram, program)

    logLine("%s: %s: <- result %d", context->name, __func__,
        status);

    return status;
}

static GLboolean OGLES2_glIsRenderbuffer(struct OGLES2IFace *Self, GLuint renderbuffer)
{
    GET_CONTEXT

    GLboolean status = GL_FALSE;

    logLine("%s: %s: renderbuffer %u", context->name, __func__,
        renderbuffer);

    GL_CALL_STATUS(IsRenderbuffer, renderbuffer)

    logLine("%s: %s: <- result %d", context->name, __func__,
        status);

    return status;
}

static GLboolean OGLES2_glIsShader(struct OGLES2IFace *Self, GLuint shader)
{
    GET_CONTEXT

    GLboolean status = GL_FALSE;

    logLine("%s: %s: shader %u", context->name, __func__,
        shader);

    GL_CALL_STATUS(IsShader, shader)

    logLine("%s: %s: <- result %d", context->name, __func__,
        status);

    return status;
}

static GLboolean OGLES2_glIsTexture(struct OGLES2IFace *Self, GLuint texture)
{
    GET_CONTEXT

    GLboolean status = GL_FALSE;

    logLine("%s: %s: texture %u", context->name, __func__,
        texture);

    GL_CALL_STATUS(IsTexture, texture)

    logLine("%s: %s: <- result %d", context->name, __func__,
        status);

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

static void* OGLES2_glMapBufferOES(struct OGLES2IFace *Self, GLenum target, GLenum access)
{
    GET_CONTEXT

    void* status = NULL;

    logLine("%s: %s: target 0x%X (%s), access 0x%X (%s)", context->name, __func__,
        target, decodeValue(target),
        access, decodeValue(access));

    GL_CALL_STATUS(MapBufferOES, target, access)

    logLine("%s: %s: <- address %p", context->name, __func__,
        status);

    checkPointer(context, MapBufferOES, status);

    return status;
}

static void OGLES2_glPixelStorei(struct OGLES2IFace *Self, GLenum pname, GLint param)
{
    GET_CONTEXT

    logLine("%s: %s: pname 0x%X (%s), param %d", context->name, __func__,
        pname, decodeValue(pname),
        param);

    GL_CALL(PixelStorei, pname, param)
}

static void OGLES2_glPolygonMode(struct OGLES2IFace *Self, GLenum face, GLenum mode)
{
    GET_CONTEXT

    logLine("%s: %s: face 0x%X (%s), mode 0x%X (%s)", context->name, __func__,
        face, decodeValue(face),
        mode, decodeValue(mode));

    GL_CALL(PolygonMode, face, mode)
}

static void OGLES2_glPolygonOffset(struct OGLES2IFace *Self, GLfloat factor, GLfloat units)
{
    GET_CONTEXT

    logLine("%s: %s: factor %f, units %f", context->name, __func__,
        factor, units);

    GL_CALL(PolygonOffset, factor, units)
}

static void OGLES2_glProgramBinaryOES(struct OGLES2IFace *Self, GLuint program, GLenum binaryFormat, const void *binary, GLint length)
{
    GET_CONTEXT

    logLine("%s: %s: program %u, binaryFormat 0x%X (%s), binary %p, length %d", context->name, __func__,
        program,
        binaryFormat, decodeValue(binaryFormat),
        binary, length);

    GL_CALL(ProgramBinaryOES, program, binaryFormat, binary, length)
}

static void OGLES2_glProvokingVertex(struct OGLES2IFace *Self, GLenum provokeMode)
{
    GET_CONTEXT

    logLine("%s: %s: provokeMode 0x%X (%s)", context->name, __func__,
        provokeMode, decodeValue(provokeMode));

    GL_CALL(ProvokingVertex, provokeMode)
}

static void OGLES2_glReadPixels(struct OGLES2IFace *Self, GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void * pixels)
{
    GET_CONTEXT

    logLine("%s: %s: x %d, y %d, width %u, height %u, format 0x%X (%s), type 0x%X (%s), pixels %p", context->name, __func__,
        x, y, width, height,
        format, decodeValue(format),
        type, decodeValue(type),
        pixels);

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

    logLine("%s: %s: target 0x%X (%s), internalformat 0x%X (%s), width %u, height %u", context->name, __func__,
        target, decodeValue(target),
        internalformat, decodeValue(internalformat),
        width, height);

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

    logLine("%s: %s: count %u, shaders %p, binaryformat 0x%X (%s), binary %p, length %u", context->name, __func__,
        count, shaders,
        binaryformat, decodeValue(binaryformat),
        binary, length);

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
            if (length[i] < 0) {
                // These should be NUL-terminated
                logLine("Line %u: '%s'", i, string[i]);
            } else if (length[i] == 0) {
                logLine("Line %u: length 0:", i);
            } else {
                // Use temporary buffer to NUL-terminate strings
                const size_t len = (size_t)length[i] + 1;

                char* temp = IExec->AllocVecTags(len, TAG_DONE);

                if (temp) {
                    snprintf(temp, len, "%s", string[i]);
                    logLine("Line %u: length %d: '%s'", i, length[i], temp);

                    IExec->FreeVec(temp);
                } else {
                    logLine("Failed to allocate %u bytes", len);
                }
            }
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

    logLine("%s: %s: func 0x%X (%s), ref %d, mask %u", context->name, __func__,
        func, decodeValue(func),
        ref, mask);

    GL_CALL(StencilFunc, func, ref, mask)
}

static void OGLES2_glStencilFuncSeparate(struct OGLES2IFace *Self, GLenum face, GLenum func, GLint ref, GLuint mask)
{
    GET_CONTEXT

    logLine("%s: %s: face 0x%X (%s), func 0x%X (%s), ref %d, mask %u", context->name, __func__,
        face, decodeValue(face),
        func, decodeValue(func),
        ref, mask);

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

    logLine("%s: %s: face 0x%X (%s), mask %u", context->name, __func__,
        face, decodeValue(face),
        mask);

    GL_CALL(StencilMaskSeparate, face, mask)
}

static void OGLES2_glStencilOp(struct OGLES2IFace *Self, GLenum fail, GLenum zfail, GLenum zpass)
{
    GET_CONTEXT

    logLine("%s: %s: fail 0x%X (%s), zfail 0x%X (%s), zpass 0x%X (%s)", context->name, __func__,
        fail, decodeValue(fail),
        zfail, decodeValue(zfail),
        zpass, decodeValue(zpass));

    GL_CALL(StencilOp, fail, zfail, zpass)
}

static void OGLES2_glStencilOpSeparate(struct OGLES2IFace *Self, GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass)
{
    GET_CONTEXT

    logLine("%s: %s: face 0x%X (%s), sfail 0x%X (%s), dpfail 0x%X (%s), dppass 0x%X (%s)", context->name, __func__,
        face, decodeValue(face),
        sfail, decodeValue(sfail),
        dpfail, decodeValue(dpfail),
        dppass, decodeValue(dppass));

    GL_CALL(StencilOpSeparate, face, sfail, dpfail, dppass)
}

static void OGLES2_glTexImage2D(struct OGLES2IFace *Self, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void * pixels)
{
    GET_CONTEXT

    logLine("%s: %s: target 0x%X (%s), level %d, internalformat 0x%X (%s), width %u, height %u, border %d, format 0x%X (%s), type 0x%X (%s), pixels %p", context->name, __func__,
        target, decodeValue(target),
        level,
        internalformat, decodeValue((GLenum)internalformat),
        width, height, border,
        format, decodeValue(format),
        type, decodeValue(type),
        pixels);

    GL_CALL(TexImage2D, target,  level, internalformat, width, height, border, format, type, pixels)
}

static void OGLES2_glTexParameterf(struct OGLES2IFace *Self, GLenum target, GLenum pname, GLfloat param)
{
    GET_CONTEXT

    logLine("%s: %s: target 0x%X (%s), pname 0x%X (%s), param %f", context->name, __func__,
        target, decodeValue(target),
        pname, decodeValue(pname),
        param);

    GL_CALL(TexParameterf, target, pname, param)
}

static void OGLES2_glTexParameterfv(struct OGLES2IFace *Self, GLenum target, GLenum pname, const GLfloat * params)
{
    GET_CONTEXT

    logLine("%s: %s: target 0x%X (%s), pname 0x%X (%s), params %p", context->name, __func__,
        target, decodeValue(target),
        pname, decodeValue(pname),
        params);

    GL_CALL(TexParameterfv, target, pname, params)
}

static void OGLES2_glTexParameteri(struct OGLES2IFace *Self, GLenum target, GLenum pname, GLint param)
{
    GET_CONTEXT

    logLine("%s: %s: target 0x%X (%s), pname 0x%X (%s), param 0x%X (%s)", context->name, __func__,
        target, decodeValue(target),
        pname, decodeValue(pname),
        param, decodeValue((GLenum)param));

    GL_CALL(TexParameteri, target, pname, param)
}

static void OGLES2_glTexParameteriv(struct OGLES2IFace *Self, GLenum target, GLenum pname, const GLint * params)
{
    GET_CONTEXT

    logLine("%s: %s: target 0x%X (%s), pname 0x%X (%s), params %p", context->name, __func__,
        target, decodeValue(target),
        pname, decodeValue(pname),
        params);

    GL_CALL(TexParameteriv, target, pname, params)
}

static void OGLES2_glTexSubImage2D(struct OGLES2IFace *Self, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void * pixels)
{
    GET_CONTEXT

    logLine("%s: %s: target 0x%X (%s, level %d, xoffset %d, yoffset %d, width %u, height %u, format 0x%X (%s), type 0x%X (%s), pixels %p", context->name, __func__,
        target, decodeValue(target),
        level, xoffset, yoffset, width, height,
        format, decodeValue(format),
        type, decodeValue(type),
        pixels);

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

static GLboolean OGLES2_glUnmapBufferOES(struct OGLES2IFace *Self, GLenum target)
{
    GET_CONTEXT

    GLboolean status = GL_FALSE;

    logLine("%s: %s: target 0x%X (%s)", context->name, __func__,
        target, decodeValue(target));

    GL_CALL_STATUS(UnmapBufferOES, target)

    logLine("%s: %s: <- return value %d", context->name, __func__,
        status);

    return status;
}

static void OGLES2_glUseProgram(struct OGLES2IFace *Self, GLuint program)
{
    GET_CONTEXT

    logLine("%s: %s program %u", context->name, __func__,
        program);

    GL_CALL(UseProgram, program);
}

static void OGLES2_glValidateProgram(struct OGLES2IFace *Self, GLuint program)
{
    GET_CONTEXT

    logLine("%s: %s program %u", context->name, __func__,
        program);

    GL_CALL(ValidateProgram, program);
}

static void OGLES2_glVertexAttrib1f(struct OGLES2IFace *Self, GLuint index, GLfloat x)
{
    GET_CONTEXT

    logLine("%s: %s index %u, x %f", context->name, __func__,
        index, x);

    GL_CALL(VertexAttrib1f, index, x);
}

static void OGLES2_glVertexAttrib1fv(struct OGLES2IFace *Self, GLuint index, const GLfloat * v)
{
    GET_CONTEXT

    logLine("%s: %s index %u, v { %f }", context->name, __func__,
        index, v[0]);

    GL_CALL(VertexAttrib1fv, index, v);
}

static void OGLES2_glVertexAttrib2f(struct OGLES2IFace *Self, GLuint index, GLfloat x, GLfloat y)
{
    GET_CONTEXT

    logLine("%s: %s index %u, x %f, y %f", context->name, __func__,
        index, x, y);

    GL_CALL(VertexAttrib2f, index, x, y);
}

static void OGLES2_glVertexAttrib2fv(struct OGLES2IFace *Self, GLuint index, const GLfloat * v)
{
    GET_CONTEXT

    logLine("%s: %s index %u, v { %f, %f }", context->name, __func__,
        index, v[0], v[1]);

    GL_CALL(VertexAttrib2fv, index, v);
}

static void OGLES2_glVertexAttrib3f(struct OGLES2IFace *Self, GLuint index, GLfloat x, GLfloat y, GLfloat z)
{
    GET_CONTEXT

    logLine("%s: %s index %u, x %f, y %f, z %f", context->name, __func__,
        index, x, y, z);

    GL_CALL(VertexAttrib3f, index, x, y, z);
}

static void OGLES2_glVertexAttrib3fv(struct OGLES2IFace *Self, GLuint index, const GLfloat * v)
{
    GET_CONTEXT

    logLine("%s: %s index %u, v { %f, %f, %f }", context->name, __func__,
        index, v[0], v[1], v[2]);

    GL_CALL(VertexAttrib3fv, index, v);
}

static void OGLES2_glVertexAttrib4f(struct OGLES2IFace *Self, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    GET_CONTEXT

    logLine("%s: %s index %u, x %f, y %f, z %f, w %f", context->name, __func__,
        index, x, y, z, w);

    GL_CALL(VertexAttrib4f, index, x, y, z, w);
}

static void OGLES2_glVertexAttrib4fv(struct OGLES2IFace *Self, GLuint index, const GLfloat * v)
{
    GET_CONTEXT

    logLine("%s: %s index %u, v { %f, %f, %f, %f }", context->name, __func__,
        index, v[0], v[1], v[2], v[3]);

    GL_CALL(VertexAttrib4fv, index, v);
}

static void OGLES2_glVertexAttribPointer(struct OGLES2IFace *Self, GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void * pointer)
{
    GET_CONTEXT

    logLine("%s: %s: index %u, size %d, type 0x%X (%s), normalized %d, stride %d, pointer %p", context->name, __func__,
        index, size,
        type, decodeValue(type),
        normalized, stride, pointer);

    GL_CALL(VertexAttribPointer, index, size, type, normalized, stride, pointer)
}

static void OGLES2_glViewport(struct OGLES2IFace *Self, GLint x, GLint y, GLsizei width, GLsizei height)
{
    GET_CONTEXT

    logLine("%s: %s: x %d, y %d, width %u, height %u", context->name, __func__,
        x, y, width, height);

    GL_CALL(Viewport, x, y, width, height)
}

GENERATE_FILTERED_PATCH(OGLES2IFace, aglCreateContext_AVOID, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, aglCreateContext2, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, aglDestroyContext, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, aglGetProcAddress, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, aglMakeCurrent, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, aglSetBitmap, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, aglSetParams_AVOID, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, aglSetParams2, OGLES2, Ogles2Context)
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
GENERATE_FILTERED_PATCH(OGLES2IFace, glDrawElementsBaseVertexOES, OGLES2, Ogles2Context)
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
GENERATE_FILTERED_PATCH(OGLES2IFace, glGetBufferParameterivOES, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glGetBufferPointervOES, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glGetError, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glGetFloatv, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glGetFramebufferAttachmentParameteriv, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glGetIntegerv, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glGetProgramBinaryOES, OGLES2, Ogles2Context)
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
GENERATE_FILTERED_PATCH(OGLES2IFace, glMapBufferOES, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glPixelStorei, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glPolygonMode, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glPolygonOffset, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glProgramBinaryOES, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glProvokingVertex, OGLES2, Ogles2Context)
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
GENERATE_FILTERED_PATCH(OGLES2IFace, glUnmapBufferOES, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glUseProgram, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glValidateProgram, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glVertexAttrib1f, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glVertexAttrib1fv, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glVertexAttrib2f, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glVertexAttrib2fv, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glVertexAttrib3f, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glVertexAttrib3fv, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glVertexAttrib4f, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glVertexAttrib4fv, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glVertexAttribPointer, OGLES2, Ogles2Context)
GENERATE_FILTERED_PATCH(OGLES2IFace, glViewport, OGLES2, Ogles2Context)

static void (*patches[])(BOOL, struct Ogles2Context *) = {
    patch_aglCreateContext_AVOID,
    patch_aglCreateContext2,
    patch_aglDestroyContext,
    patch_aglGetProcAddress,
    patch_aglMakeCurrent,
    patch_aglSetBitmap,
    patch_aglSetParams_AVOID,
    patch_aglSetParams2,
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
    patch_glDrawElementsBaseVertexOES,
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
    patch_glGetBufferParameterivOES,
    patch_glGetBufferPointervOES,
    patch_glGetError,
    patch_glGetFloatv,
    patch_glGetFramebufferAttachmentParameteriv,
    patch_glGetIntegerv,
    patch_glGetProgramBinaryOES,
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
    patch_glMapBufferOES,
    patch_glPixelStorei,
    patch_glPolygonMode,
    patch_glPolygonOffset,
    patch_glProgramBinaryOES,
    patch_glProvokingVertex,
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
    patch_glUnmapBufferOES,
    patch_glUseProgram,
    patch_glValidateProgram,
    patch_glVertexAttrib1f,
    patch_glVertexAttrib1fv,
    patch_glVertexAttrib2f,
    patch_glVertexAttrib2fv,
    patch_glVertexAttrib3f,
    patch_glVertexAttrib3fv,
    patch_glVertexAttrib4f,
    patch_glVertexAttrib4fv,
    patch_glVertexAttribPointer,
    patch_glViewport
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
