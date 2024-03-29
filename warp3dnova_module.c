#include "warp3dnova_module.h"
#include "common.h"
#include "filter.h"
#include "timer.h"
#include "profiling.h"
#include "logger.h"

#include <proto/exec.h>
#include <proto/warp3dnova.h>
#include <proto/utility.h>

#include <stdio.h>
#include <string.h>

extern BYTE mainSig;
extern struct Task* mainTask;

typedef enum NovaFunction {
    BindBitMapAsTexture,
    BindShaderDataBuffer,
    BindTexture,
    BindVertexAttribArray,
    BufferUnlock,
    Clear,
    CompileShader,
    CreateDataBufferObject,
    CreateFrameBuffer,
    CreateRenderStateObject,
    CreateShaderPipeline,
    CreateTexSampler,
    CreateTexture,
    CreateTextureExtRMB,
    CreateVertexBufferObject,
    DBOGetAttr,
    DBOGetBuffer,
    DBOLock,
    DBOSetBuffer,
    Destroy,
    DestroyDataBufferObject,
    DestroyFrameBuffer,
    DestroyRenderStateObject,
    DestroyShader,
    DestroyShaderLog,
    DestroyShaderPipeline,
    DestroyTexSampler,
    DestroyTexture,
    DestroyVertexBufferObject,
    DrawArrays,
    DrawElements,
    FBBindBuffer,
    FBGetAttr,
    FBGetBufferBM,
    FBGetBufferTex,
    FBGetStatus,
    GetBitMapTexture,
    GetBlendColour,
    GetBlendEquation,
    GetBlendMode,
    GetColourMask,
    GetDepthCompareFunc,
    GetFrontFace,
    GetLineWidth,
    GetPolygonMode,
    GetPolygonOffset,
    GetProvokingVertex,
    GetRenderTarget,
    GetScissor,
    GetShaderDataBuffer,
    GetShaderPipeline,
    GetState,
    GetStencilFunc,
    GetStencilOp,
    GetStencilWriteMask,
    GetTexSampler,
    GetTexture,
    GetVertexAttribArray,
    GetViewport,
    IsDone,
    Query,
    RSOCopy,
    RSOSetMaster,
    SetBlendColour,
    SetBlendEquation,
    SetBlendEquationSeparate,
    SetBlendMode,
    SetBlendModeSeparate,
    SetColourMask,
    SetDepthCompareFunc,
    SetFrontFace,
    SetLineWidth,
    SetPolygonMode,
    SetPolygonOffset,
    SetProvokingVertex,
    SetRenderTarget,
    SetScissor,
    SetShaderPipeline,
    SetState,
    SetStencilFunc,
    SetStencilFuncSeparate,
    SetStencilOp,
    SetStencilOpSeparate,
    SetStencilWriteMask,
    SetStencilWriteMaskSeparate,
    SetViewport,
    ShaderGetCount,
    ShaderGetObjectInfo,
    ShaderGetOffset,
    ShaderGetTotalStorage,
    ShaderGetType,
    ShaderPipelineGetShader,
    Submit,
    TexGenMipMaps,
    TexGetParameters,
    TexGetProperty,
    TexGetRMBuffer,
    TexGetSubResourceLayout,
    TexSetParameters,
    TexUpdateImage,
    TexUpdateSubImage,
    TSGetParameters,
    TSSetParameters,
    VBOGetArray,
    VBOGetAttr,
    VBOLock,
    VBOSetArray,
    WaitDone,
    WaitIdle,
    // Keep last
    NovaFunctionCount
} NovaFunction;

static const char* mapNovaFunction(const NovaFunction func)
{
    #define MAP_ENUM(x) case x: return #x;

    switch (func) {
        MAP_ENUM(BindBitMapAsTexture)
        MAP_ENUM(BindShaderDataBuffer)
        MAP_ENUM(BindTexture)
        MAP_ENUM(BindVertexAttribArray)
        MAP_ENUM(BufferUnlock)
        MAP_ENUM(Clear)
        MAP_ENUM(CompileShader)
        MAP_ENUM(CreateDataBufferObject)
        MAP_ENUM(CreateFrameBuffer)
        MAP_ENUM(CreateRenderStateObject)
        MAP_ENUM(CreateShaderPipeline)
        MAP_ENUM(CreateTexSampler)
        MAP_ENUM(CreateTexture)
        MAP_ENUM(CreateTextureExtRMB)
        MAP_ENUM(CreateVertexBufferObject)
        MAP_ENUM(DBOGetAttr)
        MAP_ENUM(DBOGetBuffer)
        MAP_ENUM(DBOLock)
        MAP_ENUM(DBOSetBuffer)
        MAP_ENUM(Destroy)
        MAP_ENUM(DestroyDataBufferObject)
        MAP_ENUM(DestroyFrameBuffer)
        MAP_ENUM(DestroyRenderStateObject)
        MAP_ENUM(DestroyShader)
        MAP_ENUM(DestroyShaderLog)
        MAP_ENUM(DestroyShaderPipeline)
        MAP_ENUM(DestroyTexSampler)
        MAP_ENUM(DestroyTexture)
        MAP_ENUM(DestroyVertexBufferObject)
        MAP_ENUM(DrawArrays)
        MAP_ENUM(DrawElements)
        MAP_ENUM(FBBindBuffer)
        MAP_ENUM(FBGetAttr)
        MAP_ENUM(FBGetBufferBM)
        MAP_ENUM(FBGetBufferTex)
        MAP_ENUM(FBGetStatus)
        MAP_ENUM(GetBitMapTexture)
        MAP_ENUM(GetBlendColour)
        MAP_ENUM(GetBlendEquation)
        MAP_ENUM(GetBlendMode)
        MAP_ENUM(GetColourMask)
        MAP_ENUM(GetDepthCompareFunc)
        MAP_ENUM(GetFrontFace)
        MAP_ENUM(GetLineWidth)
        MAP_ENUM(GetPolygonMode)
        MAP_ENUM(GetPolygonOffset)
        MAP_ENUM(GetProvokingVertex)
        MAP_ENUM(GetRenderTarget)
        MAP_ENUM(GetScissor)
        MAP_ENUM(GetShaderDataBuffer)
        MAP_ENUM(GetShaderPipeline)
        MAP_ENUM(GetState)
        MAP_ENUM(GetStencilFunc)
        MAP_ENUM(GetStencilOp)
        MAP_ENUM(GetStencilWriteMask)
        MAP_ENUM(GetTexSampler)
        MAP_ENUM(GetTexture)
        MAP_ENUM(GetVertexAttribArray)
        MAP_ENUM(GetViewport)
        MAP_ENUM(IsDone)
        MAP_ENUM(Query)
        MAP_ENUM(RSOCopy)
        MAP_ENUM(RSOSetMaster)
        MAP_ENUM(SetBlendColour)
        MAP_ENUM(SetBlendEquation)
        MAP_ENUM(SetBlendEquationSeparate)
        MAP_ENUM(SetBlendMode)
        MAP_ENUM(SetBlendModeSeparate)
        MAP_ENUM(SetColourMask)
        MAP_ENUM(SetDepthCompareFunc)
        MAP_ENUM(SetFrontFace)
        MAP_ENUM(SetLineWidth)
        MAP_ENUM(SetPolygonMode)
        MAP_ENUM(SetPolygonOffset)
        MAP_ENUM(SetProvokingVertex)
        MAP_ENUM(SetRenderTarget)
        MAP_ENUM(SetScissor)
        MAP_ENUM(SetShaderPipeline)
        MAP_ENUM(SetState)
        MAP_ENUM(SetStencilFunc)
        MAP_ENUM(SetStencilFuncSeparate)
        MAP_ENUM(SetStencilOp)
        MAP_ENUM(SetStencilOpSeparate)
        MAP_ENUM(SetStencilWriteMask)
        MAP_ENUM(SetStencilWriteMaskSeparate)
        MAP_ENUM(SetViewport)
        MAP_ENUM(ShaderGetCount)
        MAP_ENUM(ShaderGetObjectInfo)
        MAP_ENUM(ShaderGetOffset)
        MAP_ENUM(ShaderGetTotalStorage)
        MAP_ENUM(ShaderGetType)
        MAP_ENUM(ShaderPipelineGetShader)
        MAP_ENUM(Submit)
        MAP_ENUM(TexGenMipMaps)
        MAP_ENUM(TexGetParameters)
        MAP_ENUM(TexGetProperty)
        MAP_ENUM(TexGetRMBuffer)
        MAP_ENUM(TexGetSubResourceLayout)
        MAP_ENUM(TexSetParameters)
        MAP_ENUM(TexUpdateImage)
        MAP_ENUM(TexUpdateSubImage)
        MAP_ENUM(TSGetParameters)
        MAP_ENUM(TSSetParameters)
        MAP_ENUM(VBOGetArray)
        MAP_ENUM(VBOGetAttr)
        MAP_ENUM(VBOLock)
        MAP_ENUM(VBOSetArray)
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

static ULONG profilerStartTime = 0;
static ULONG profilerDuration = 0;

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
        MAP_ENUM(W3DNEC_INVALIDLAYOUT)
        MAP_ENUM(W3DNEC_NOTOURBUFFER)
    }

    #undef MAP_ENUM

    return "Unknown error";
}

static const char* decodeShaderType(const W3DN_ShaderType type)
{
    #define MAP_ENUM(x) case x: return #x;

    switch (type) {
        MAP_ENUM(W3DNST_VERTEX)
        MAP_ENUM(W3DNST_FRAGMENT)
        MAP_ENUM(W3DNST_MAX)
        MAP_ENUM(W3DNST_END)
    }

    #undef MAP_ENUM

    return "Unknown shader type";
}

static const char* decodeBufferUsage(const W3DN_BufferUsage usage)
{
    #define MAP_ENUM(x) case x: return #x;

    switch (usage) {
        MAP_ENUM(W3DN_STATIC_DRAW)
        MAP_ENUM(W3DN_STATIC_READ)
        MAP_ENUM(W3DN_STATIC_COPY)
        MAP_ENUM(W3DN_DYNAMIC_DRAW)
        MAP_ENUM(W3DN_DYNAMIC_READ)
        MAP_ENUM(W3DN_DYNAMIC_COPY)
        MAP_ENUM(W3DN_STREAM_DRAW)
        MAP_ENUM(W3DN_STREAM_READ)
        MAP_ENUM(W3DN_STREAM_COPY)
        MAP_ENUM(W3DN_BUFFERUSAGE_END)
    }

    #undef MAP_ENUM

    return "Unknown buffer usage";
}

static const char* decodeTextureType(const W3DN_TextureType type)
{
    #define MAP_ENUM(x) case x: return #x;

    switch (type) {
        MAP_ENUM(W3DN_TEXTURE_1D)
        MAP_ENUM(W3DN_TEXTURE_2D)
        MAP_ENUM(W3DN_TEXTURE_3D)
        MAP_ENUM(W3DN_TEXTURE_CUBEMAP)
        MAP_ENUM(W3DN_TEXTURE_1D_ARRAY)
        MAP_ENUM(W3DN_TEXTURE_2D_ARRAY)
        MAP_ENUM(W3DN_TEXTURE_CUBEMAP_ARRAY)
        MAP_ENUM(W3DN_TEXTURE_END)
    }

    #undef MAP_ENUM

    return "Unknown texture type";
}

static const char* decodePixelFormat(const W3DN_PixelFormat format)
{
    #define MAP_ENUM(x) case x: return #x;

    switch (format) {
        MAP_ENUM(W3DNPF_DEPTH)
        MAP_ENUM(W3DNPF_DEPTH_STENCIL)
        MAP_ENUM(W3DNPF_RED)
        MAP_ENUM(W3DNPF_RG)
        MAP_ENUM(W3DNPF_RGB)
        MAP_ENUM(W3DNPF_RGBA)
        MAP_ENUM(W3DNPF_SRGB8)
        MAP_ENUM(W3DNPF_SRGB8_A8)
        MAP_ENUM(W3DNPF_END)
        MAP_ENUM(W3DNPF_NONE)
    }

    #undef MAP_ENUM

    return "Unknown pixel format";
}

static const char* decodeElementFormat(const W3DN_ElementFormat format)
{
    #define MAP_ENUM(x) case x: return #x;

    switch (format) {
        MAP_ENUM(W3DNEF_UINT8)
        MAP_ENUM(W3DNEF_SINT8)
        MAP_ENUM(W3DNEF_UINT16)
        MAP_ENUM(W3DNEF_SINT16)
        MAP_ENUM(W3DNEF_UINT32)
        MAP_ENUM(W3DNEF_SINT32)
        MAP_ENUM(W3DNEF_FLOAT)
        MAP_ENUM(W3DNEF_DOUBLE)
        MAP_ENUM(W3DNEF_UINT8_3_3_2)
        MAP_ENUM(W3DNEF_UINT8_2_3_3_REV)
        MAP_ENUM(W3DNEF_UINT16_5_6_5)
        MAP_ENUM(W3DNEF_UINT16_4_4_4_4)
        MAP_ENUM(W3DNEF_UINT16_5_5_5_1)
        MAP_ENUM(W3DNEF_UINT16_1_5_5_5_REV)
        MAP_ENUM(W3DNEF_UINT32_10_10_10_2)
        MAP_ENUM(W3DNEF_UINT32_2_10_10_10_REV)
        MAP_ENUM(W3DNEF_END)
        MAP_ENUM(W3DNEF_NONE)
    }

    #undef MAP_ENUM

    return "Unknown element format";
}

static const char* decodeBufferAttribute(const W3DN_BufferAttribute attribute)
{
    #define MAP_ENUM(x) case x: return #x;

    switch (attribute) {
        MAP_ENUM(W3DN_BUFFATTR_SIZE)
        MAP_ENUM(W3DN_BUFFATTR_USAGE)
        MAP_ENUM(W3DN_BUFFATTR_NUM_ARRAYS)
        MAP_ENUM(W3DN_BUFFATTR_NUM_BUFFERS)
        MAP_ENUM(W3DN_BUFFATTR_END)
    }

    #undef MAP_ENUM

    return "Unknown buffer attribute";
}

static const char* decodeFrameBufferAttribute(const W3DN_FrameBufferAttribute attribute)
{
    #define MAP_ENUM(x) case x: return #x;

    switch (attribute) {
        MAP_ENUM(W3DN_FBATTR_WIDTH)
        MAP_ENUM(W3DN_FBATTR_HEIGHT)
        MAP_ENUM(W3DN_FBATTR_DEPTH_FORMAT)
        MAP_ENUM(W3DN_FBATTR_STENCIL_FORMAT)
        MAP_ENUM(W3DN_FBATTR_END)
        MAP_ENUM(W3DN_FBATTR_ORIGIN_LOWER_LEFT)
    }

    #undef MAP_ENUM

    return "Unknown frame buffer attribute";
}

static const char* decodeBlendEquation(const W3DN_BlendEquation equation)
{
    #define MAP_ENUM(x) case x: return #x;

    switch (equation) {
        MAP_ENUM(W3DN_FUNC_ADD)
        MAP_ENUM(W3DN_FUNC_SUBTRACT)
        MAP_ENUM(W3DN_FUNC_REVERSE_SUBTRACT)
        MAP_ENUM(W3DN_FUNC_MIN)
        MAP_ENUM(W3DN_FUNC_MAX)
        MAP_ENUM(W3DN_BLENDEQUATION_END)
    }

    #undef MAP_ENUM

    return "Unknown blend equation";
}

static const char* decodeBlendMode(const W3DN_BlendMode mode)
{
    #define MAP_ENUM(x) case x: return #x;

    switch (mode) {
        MAP_ENUM(W3DN_ZERO)
        MAP_ENUM(W3DN_ONE)
        MAP_ENUM(W3DN_SRC_COLOUR)
        MAP_ENUM(W3DN_ONE_MINUS_SRC_COLOUR)
        MAP_ENUM(W3DN_DST_COLOUR)
        MAP_ENUM(W3DN_ONE_MINUS_DST_COLOUR)
        MAP_ENUM(W3DN_SRC_ALPHA)
        MAP_ENUM(W3DN_ONE_MINUS_SRC_ALPHA)
        MAP_ENUM(W3DN_DST_ALPHA)
        MAP_ENUM(W3DN_ONE_MINUS_DST_ALPHA)
        MAP_ENUM(W3DN_CONSTANT_COLOUR)
        MAP_ENUM(W3DN_ONE_MINUS_CONSTANT_COLOUR)
        MAP_ENUM(W3DN_CONSTANT_ALPHA)
        MAP_ENUM(W3DN_ONE_MINUS_CONSTANT_ALPHA)
        MAP_ENUM(W3DN_SRC_ALPHA_SATURATE)
        MAP_ENUM(W3DN_BLENDMODE_END)
    }

    #undef MAP_ENUM

    return "Unknown blend mode";
}

static const char* decodeFace(const W3DN_Face face)
{
    #define MAP_ENUM(x) case x: return #x;

    switch (face) {
        MAP_ENUM(W3DN_FACE_CCW)
        MAP_ENUM(W3DN_FACE_CW)
        MAP_ENUM(W3DN_FACE_END)
    }

    #undef MAP_ENUM

    return "Unknown face";
}

static const char* decodeFaceSelect(const W3DN_FaceSelect face)
{
    #define MAP_ENUM(x) case x: return #x;

    switch (face) {
        MAP_ENUM(W3DN_FRONT)
        MAP_ENUM(W3DN_BACK)
        MAP_ENUM(W3DN_FRONT_AND_BACK)
        MAP_ENUM(W3DN_FACESELECT_END)
    }

    #undef MAP_ENUM

    return "Unknown face select";
}

static const char* decodePolygonMode(const W3DN_PolygonMode mode)
{
    #define MAP_ENUM(x) case x: return #x;

    switch (mode) {
        MAP_ENUM(W3DN_POINT)
        MAP_ENUM(W3DN_LINE)
        MAP_ENUM(W3DN_FILL)
        MAP_ENUM(W3DN_POLYGONMODE_END)
    }

    #undef MAP_ENUM

    return "Unknown polygon mode";
}

static const char* decodeProvokingVertexMode(const W3DN_ProvokingVertexMode mode)
{
    #define MAP_ENUM(x) case x: return #x;

    switch (mode) {
        MAP_ENUM(W3DN_FIRST_VERTEX)
        MAP_ENUM(W3DN_LAST_VERTEX)
        MAP_ENUM(W3DN_PROVOKINGVERTEXMODE_END)
    }

    #undef MAP_ENUM

    return "Unknown provoking vertex mode";
}

static const char* decodeState(const W3DN_State state)
{
    #define MAP_ENUM(x) case x: return #x;

    switch (state) {        
        MAP_ENUM(W3DN_DISABLE)
        MAP_ENUM(W3DN_ENABLE)
        MAP_ENUM(W3DN_STATE_END)
    }

    #undef MAP_ENUM

    return "Unknown state";
}

static const char* decodeStateFlag(const W3DN_StateFlag flag)
{
    #define MAP_ENUM(x) case x: return #x;

    switch (flag) {
        MAP_ENUM(W3DN_DEPTHTEST)
        MAP_ENUM(W3DN_DEPTHWRITE)
        MAP_ENUM(W3DN_STENCILTEST)
        MAP_ENUM(W3DN_CULLFRONT)
        MAP_ENUM(W3DN_CULLBACK)
        MAP_ENUM(W3DN_BLEND)
        MAP_ENUM(W3DN_STATEFLAG_END)
    }

    #undef MAP_ENUM

    return "Unknown state flag";
}

static const char* decodeCompareFunc(const W3DN_CompareFunc func)
{
    #define MAP_ENUM(x) case x: return #x;

    switch (func) {
        MAP_ENUM(W3DN_NEVER)
        MAP_ENUM(W3DN_LESS)
        MAP_ENUM(W3DN_GEQUAL)
        MAP_ENUM(W3DN_LEQUAL)
        MAP_ENUM(W3DN_GREATER)
        MAP_ENUM(W3DN_NOTEQUAL)
        MAP_ENUM(W3DN_EQUAL)
        MAP_ENUM(W3DN_ALWAYS)
        MAP_ENUM(W3DN_COMPAREFUNC_END)
    }

    #undef MAP_ENUM

    return "Unknown compare func";
}

static const char* decodeStencilOp(const W3DN_StencilOp op)
{
    #define MAP_ENUM(x) case x: return #x;

    switch (op) {
        MAP_ENUM(W3DN_ST_KEEP)
        MAP_ENUM(W3DN_ST_ZERO)
        MAP_ENUM(W3DN_ST_REPLACE)
        MAP_ENUM(W3DN_ST_INCR)
        MAP_ENUM(W3DN_ST_DECR)
        MAP_ENUM(W3DN_ST_INVERT)
        MAP_ENUM(W3DN_ST_INCR_WRAP)
        MAP_ENUM(W3DN_ST_DECR_WRAP)
        MAP_ENUM(W3DN_STENCILOP_END)
    }

    #undef MAP_ENUM

    return "Unknown stencil op";
}

static const char* decodePrimitive(const W3DN_Primitive primitive)
{
    #define MAP_ENUM(x) case x: return #x;

    switch (primitive) {
        MAP_ENUM(W3DN_PRIM_TRIANGLES)
        MAP_ENUM(W3DN_PRIM_TRIFAN)
        MAP_ENUM(W3DN_PRIM_TRISTRIP)
        MAP_ENUM(W3DN_PRIM_POINTS)
        MAP_ENUM(W3DN_PRIM_LINES)
        MAP_ENUM(W3DN_PRIM_LINELOOP)
        MAP_ENUM(W3DN_PRIM_LINESTRIP)
        MAP_ENUM(W3DN_PRIM_END)
    }

    #undef MAP_ENUM

    return "Unknown primitive";
}

static const char* decodeCapQuery(const W3DN_CapQuery query)
{
    #define MAP_ENUM(x) case x: return #x;

    switch (query) {
        MAP_ENUM(W3DN_Q_MAXTEXUNITS)
        MAP_ENUM(W3DN_Q_MAXTEXWIDTH)
        MAP_ENUM(W3DN_Q_MAXTEXHEIGHT)
        MAP_ENUM(W3DN_Q_ANISOTROPICFILTER)
        MAP_ENUM(W3DN_Q_MAXANISOTROPY)
        MAP_ENUM(W3DN_Q_RENDERTOTEXTURE)
        MAP_ENUM(W3DN_Q_BITMAPASTEXTURE)
        MAP_ENUM(W3DN_Q_DEPTHTEXTURE)
        MAP_ENUM(W3DN_Q_TEXTURE_1D)
        MAP_ENUM(W3DN_Q_TEXTURE_3D)
        MAP_ENUM(W3DN_Q_TEXTURE_CUBEMAP)
        MAP_ENUM(W3DN_Q_MAXCOLOURBUFFERS)
        MAP_ENUM(W3DN_Q_MAXRENDERWIDTH)
        MAP_ENUM(W3DN_Q_MAXRENDERHEIGHT)
        MAP_ENUM(W3DN_Q_MAXVERTEXATTRIBS)
        MAP_ENUM(W3DN_Q_MIPMAPPING)
        MAP_ENUM(W3DN_Q_MIPMAPGENERATION)
        MAP_ENUM(W3DN_Q_MAXTEXDEPTH)
        MAP_ENUM(W3DN_Q_NPOT_MIPMAPPING)
        MAP_ENUM(W3DN_Q_STENCIL)
        MAP_ENUM(W3DN_Q_VERTEX_TEXTUREFETCH)
        MAP_ENUM(W3DN_Q_MAXVARYINGVECTORS)
        MAP_ENUM(W3DN_Q_MAXTEXCUBEMAPSIZE)
        MAP_ENUM(W3DN_Q_MAXLINEWIDTH)
        MAP_ENUM(W3DN_Q_MAXPOINTSIZE)
        MAP_ENUM(W3DN_Q_POLYGONOFFSET)
        MAP_ENUM(W3DN_Q_POLYGONMODE)
        MAP_ENUM(W3DN_Q_FLATSHADE)
        MAP_ENUM(W3DN_Q_TEXTUREEXTSHARE)
        MAP_ENUM(W3DN_Q_TEXTURE_1D_ARRAY)
        MAP_ENUM(W3DN_Q_TEXTURE_2D_ARRAY)
        MAP_ENUM(W3DN_Q_TEXTURE_CUBEMAP_ARRAY)
        MAP_ENUM(W3DN_Q_GPUENDIANNESS)
        MAP_ENUM(W3DN_Q_TEXTURE_RENDER_BGRA)
        MAP_ENUM(W3DN_Q_END)
    }

    #undef MAP_ENUM

    return "Unknown cap query";
}

static const char* decodeShaderObjectType(const W3DN_ShaderObjectType type)
{
    #define MAP_ENUM(x) case x: return #x;

    switch (type) {        
        MAP_ENUM(W3DNSOT_INPUT)
        MAP_ENUM(W3DNSOT_OUTPUT)
        MAP_ENUM(W3DNSOT_IMAGE)
        MAP_ENUM(W3DNSOT_UNIFORM)
        MAP_ENUM(W3DNSOT_END)
    }

    #undef MAP_ENUM

    return "Unknown shader object type";
}

static const char* decodeTextureProperty(const W3DN_TextureProperty property)
{
    #define MAP_ENUM(x) case x: return #x;

    switch (property) {
        MAP_ENUM(W3DN_TP_TEXTURETYPE)
        MAP_ENUM(W3DN_TP_PIXELFORMAT)
        MAP_ENUM(W3DN_TP_ELEMENTFORMAT)
        MAP_ENUM(W3DN_TP_WIDTH)
        MAP_ENUM(W3DN_TP_HEIGHT)
        MAP_ENUM(W3DN_TP_DEPTH)
        MAP_ENUM(W3DN_TP_MIPMAPPED)
        MAP_ENUM(W3DN_TP_USAGE)
        MAP_ENUM(W3DN_TP_END)
    }

    #undef MAP_ENUM

    return "Unknown texture property";
}

static const char* decodeTexAspect(const W3DN_TexAspect aspect)
{
    #define MAP_ENUM(x) case x: return #x;

    switch (aspect) {
        MAP_ENUM(W3DN_TA_COLOUR)
        MAP_ENUM(W3DN_TA_DEPTH)
        MAP_ENUM(W3DN_TA_STENCIL)
        MAP_ENUM(W3DN_TEXASPECT_END)
    }

    #undef MAP_ENUM

    return "Unknown texture aspect";
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
    char tagBuffer[TAG_BUFFER_LEN];

    MyClock start;
    uint64 ticks;
    ProfilingItem profiling[NovaFunctionCount];

    PrimitiveCounter counter;

    // Store original function pointers so that they can be still called

    W3DN_ErrorCode (*old_BindBitMapAsTexture)(struct W3DN_Context_s *self, W3DN_RenderState *renderState,
        uint32 texUnit, struct BitMap *bitMap, W3DN_TextureSampler *texSampler);

    W3DN_ErrorCode (*old_BindShaderDataBuffer)(struct W3DN_Context_s *self, W3DN_RenderState *renderState,
        W3DN_ShaderType shaderType, W3DN_DataBuffer *buffer, uint32 bufferIdx);

    W3DN_ErrorCode (*old_BindTexture)(struct W3DN_Context_s *self, W3DN_RenderState *renderState,
        uint32 texUnit, W3DN_Texture *texture, W3DN_TextureSampler *texSampler);

    W3DN_ErrorCode (*old_BindVertexAttribArray)(struct W3DN_Context_s *self,
    		W3DN_RenderState *renderState, uint32 attribNum,
    		W3DN_VertexBuffer *buffer, uint32 arrayIdx);

    W3DN_ErrorCode (*old_BufferUnlock)(struct W3DN_Context_s *self,
    		W3DN_BufferLock *bufferLock, uint64 writeOffset, uint64 writeSize);

    W3DN_ErrorCode (*old_Clear)(struct W3DN_Context_s *self, W3DN_RenderState *renderState,
        const float *colour, const double *depth, const uint32* stencil);

    W3DN_Shader* (*old_CompileShader)(struct W3DN_Context_s *self,
        W3DN_ErrorCode *errCode, struct TagItem *tags);

    W3DN_DataBuffer* (*old_CreateDataBufferObject)(struct W3DN_Context_s *self, W3DN_ErrorCode *errCode,
        uint64 size, W3DN_BufferUsage usage, uint32 maxBuffers, struct TagItem *tags);

    W3DN_FrameBuffer* (*old_CreateFrameBuffer)(struct W3DN_Context_s *self, W3DN_ErrorCode *errCode);

    W3DN_RenderState* (*old_CreateRenderStateObject)(struct W3DN_Context_s *self, W3DN_ErrorCode *errCode);

    W3DN_ShaderPipeline* (*old_CreateShaderPipeline)(struct W3DN_Context_s *self, W3DN_ErrorCode *errCode, struct TagItem *tags);

    W3DN_TextureSampler* (*old_CreateTexSampler)(struct W3DN_Context_s *self, W3DN_ErrorCode *errCode);

    W3DN_Texture* (*old_CreateTexture)(struct W3DN_Context_s *self, W3DN_ErrorCode *errCode, W3DN_TextureType texType,
        W3DN_PixelFormat pixelFormat, W3DN_ElementFormat elementFormat, uint32 width, uint32 height, uint32 depth,
        BOOL mipmapped, W3DN_BufferUsage usage);

    W3DN_Texture* (*old_CreateTextureExtRMB)(struct W3DN_Context_s *self, W3DN_ErrorCode *errCode, void *rmBuffer, W3DN_ResourceLayout * layout,
        W3DN_TextureType texType, W3DN_PixelFormat pixelFormat, W3DN_ElementFormat elementFormat, uint32 width, uint32 height, uint32 depth, BOOL mipmapped);

    W3DN_VertexBuffer* (*old_CreateVertexBufferObject)(struct W3DN_Context_s *self,
    		W3DN_ErrorCode *errCode, uint64 size, W3DN_BufferUsage usage, uint32 maxArrays, struct TagItem *tags);

    uint64 (*old_DBOGetAttr)(struct W3DN_Context_s *self, W3DN_DataBuffer *dataBuffer, W3DN_BufferAttribute attr);

    W3DN_ErrorCode (*old_DBOGetBuffer)(struct W3DN_Context_s *self, W3DN_DataBuffer *dataBuffer, uint32 bufferIdx,
        uint64 *offset, uint64 *size, W3DN_Shader **targetShader, struct TagItem *tags);

    W3DN_BufferLock* (*old_DBOLock)(struct W3DN_Context_s *self, W3DN_ErrorCode *errCode, W3DN_DataBuffer *buffer, uint64 readOffset, uint64 readSize);

    W3DN_ErrorCode (*old_DBOSetBuffer)(struct W3DN_Context_s *self, W3DN_DataBuffer *dataBuffer, uint32 bufferIdx,
        uint64 offset, uint64 size, W3DN_Shader *targetShader, struct TagItem *tags);

    void (*old_Destroy)(struct W3DN_Context_s *self);

    void (*old_DestroyDataBufferObject)(struct W3DN_Context_s *self, W3DN_DataBuffer *dataBuffer);

    void (*old_DestroyFrameBuffer)(struct W3DN_Context_s *self, W3DN_FrameBuffer *frameBuffer);

    void (*old_DestroyRenderStateObject)(struct W3DN_Context_s *self, W3DN_RenderState *renderState);

    void (*old_DestroyShader)(struct W3DN_Context_s *self, W3DN_Shader *shader);

    void (*old_DestroyShaderLog)(struct W3DN_Context_s *self, const char *shaderLog);

    void (*old_DestroyShaderPipeline)(struct W3DN_Context_s *self, W3DN_ShaderPipeline *shaderPipeline);

    void (*old_DestroyTexSampler)(struct W3DN_Context_s *self, W3DN_TextureSampler *texSampler);

    void (*old_DestroyTexture)(struct W3DN_Context_s *self, W3DN_Texture *texture);

    void (*old_DestroyVertexBufferObject)(struct W3DN_Context_s *self, W3DN_VertexBuffer *vertexBuffer);

    W3DN_ErrorCode (*old_DrawArrays)(struct W3DN_Context_s *self,
    		W3DN_RenderState *renderState, W3DN_Primitive primitive, uint32 base, uint32 count);

    W3DN_ErrorCode (*old_DrawElements)(struct W3DN_Context_s *self,
    		W3DN_RenderState *renderState, W3DN_Primitive primitive, uint32 baseVertex, uint32 count,
    		W3DN_VertexBuffer *indexBuffer, uint32 arrayIdx);

    W3DN_ErrorCode (*old_FBBindBuffer)(struct W3DN_Context_s *self,
    	W3DN_FrameBuffer *frameBuffer, int32 attachmentPt, struct TagItem *tags);

    uint64 (*old_FBGetAttr)(struct W3DN_Context_s *self,
    	W3DN_FrameBuffer *frameBuffer, W3DN_FrameBufferAttribute attrib);

    struct BitMap* (*old_FBGetBufferBM)(struct W3DN_Context_s *self,
    	W3DN_FrameBuffer *frameBuffer, uint32 attachmentPt, W3DN_ErrorCode *errCode);

    W3DN_Texture* (*old_FBGetBufferTex)(struct W3DN_Context_s *self,
    	W3DN_FrameBuffer *frameBuffer, uint32 attachmentPt, W3DN_ErrorCode *errCode);

    W3DN_ErrorCode (*old_FBGetStatus)(struct W3DN_Context_s *self, W3DN_FrameBuffer *frameBuffer);

    struct BitMap* (*old_GetBitMapTexture)(struct W3DN_Context_s *self, W3DN_RenderState *renderState, uint32 texUnit);

    W3DN_ErrorCode (*old_GetBlendColour)(struct W3DN_Context_s *self, W3DN_RenderState *renderState,
        float *red, float *green, float *blue, float *alpha);

    W3DN_ErrorCode (*old_GetBlendEquation)(struct W3DN_Context_s *self, W3DN_RenderState *renderState,
        uint32 buffIdx, W3DN_BlendEquation *colEquation, W3DN_BlendEquation *alphaEquation);

    W3DN_ErrorCode (*old_GetBlendMode)(struct W3DN_Context_s *self, W3DN_RenderState *renderState, uint32 buffIdx,
        W3DN_BlendMode *colSrc, W3DN_BlendMode *colDst, W3DN_BlendMode *alphaSrc, W3DN_BlendMode *alphaDst);

    uint8 (*old_GetColourMask)(struct W3DN_Context_s *self, W3DN_RenderState *renderState, uint32 index);

    W3DN_CompareFunc (*old_GetDepthCompareFunc)(struct W3DN_Context_s *self, W3DN_RenderState *renderState);

    W3DN_Face (*old_GetFrontFace)(struct W3DN_Context_s *self, W3DN_RenderState *renderState);

    float (*old_GetLineWidth)(struct W3DN_Context_s *self, W3DN_RenderState *renderState);

    W3DN_PolygonMode (*old_GetPolygonMode)(struct W3DN_Context_s *self, W3DN_RenderState *renderState, W3DN_FaceSelect face);

    W3DN_ErrorCode (*old_GetPolygonOffset)(struct W3DN_Context_s *self, W3DN_RenderState *renderState,
        float *factor, float *units, float *clamp);

    W3DN_ProvokingVertexMode (*old_GetProvokingVertex)(struct W3DN_Context_s *self, W3DN_RenderState *renderState);

    W3DN_FrameBuffer* (*old_GetRenderTarget)(
        struct W3DN_Context_s *self, W3DN_RenderState *renderState);

    W3DN_ErrorCode (*old_GetScissor)(struct W3DN_Context_s *self, W3DN_RenderState *renderState,
        uint32 *x, uint32 *y, uint32 *width, uint32 *height);

    W3DN_ErrorCode (*old_GetShaderDataBuffer)(struct W3DN_Context_s *self, W3DN_RenderState *renderState,
        W3DN_ShaderType shaderType, W3DN_DataBuffer **buffer, uint32 *bufferIdx);

    W3DN_ShaderPipeline* (*old_GetShaderPipeline)(struct W3DN_Context_s *self, W3DN_RenderState *renderState);

    W3DN_State (*old_GetState)(struct W3DN_Context_s *self, W3DN_RenderState *renderState, W3DN_StateFlag stateFlag);

    W3DN_ErrorCode (*old_GetStencilFunc)(struct W3DN_Context_s *self, W3DN_RenderState *renderState,
        W3DN_FaceSelect face, W3DN_CompareFunc *func, uint32 *ref, uint32 *mask);

    W3DN_ErrorCode (*old_GetStencilOp)(struct W3DN_Context_s *self, W3DN_RenderState *renderState,
        W3DN_FaceSelect face, W3DN_StencilOp *sFail, W3DN_StencilOp *dpFail, W3DN_StencilOp *dpPass);

    uint32 (*old_GetStencilWriteMask)(struct W3DN_Context_s *self, W3DN_RenderState *renderState,
        W3DN_FaceSelect face, W3DN_ErrorCode *errCode);

    W3DN_TextureSampler* (*old_GetTexSampler)(struct W3DN_Context_s *self, W3DN_RenderState *renderState, uint32 texUnit);

    W3DN_Texture* (*old_GetTexture)(struct W3DN_Context_s *self, W3DN_RenderState *renderState, uint32 texUnit);

    W3DN_ErrorCode (*old_GetVertexAttribArray)(struct W3DN_Context_s *self, W3DN_RenderState *renderState,
        uint32 attribNum, W3DN_VertexBuffer **buffer, uint32 *arrayIdx);

    W3DN_ErrorCode (*old_GetViewport)(struct W3DN_Context_s *self, W3DN_RenderState *renderState,
        double *x, double *y, double *width, double *height, double *zNear, double *zFar);

    BOOL (*old_IsDone)(struct W3DN_Context_s *self, uint32 submitID);

    uint32 (*old_Query)(struct W3DN_Context_s *self, W3DN_CapQuery query);

    W3DN_ErrorCode (*old_RSOCopy)(struct W3DN_Context_s *self, W3DN_RenderState *dest, const W3DN_RenderState *src);

    W3DN_ErrorCode (*old_RSOSetMaster)(struct W3DN_Context_s *self, W3DN_RenderState *renderState, W3DN_RenderState *master);

    W3DN_ErrorCode (*old_SetBlendColour)(struct W3DN_Context_s *self, W3DN_RenderState *renderState, float red, float green, float blue, float alpha);

    W3DN_ErrorCode (*old_SetBlendEquation)(struct W3DN_Context_s *self, W3DN_RenderState *renderState, uint32 buffIdx, W3DN_BlendEquation equation);

    W3DN_ErrorCode (*old_SetBlendEquationSeparate)(struct W3DN_Context_s *self, W3DN_RenderState *renderState, uint32 buffIdx,
        W3DN_BlendEquation colEquation, W3DN_BlendEquation alphaEquation);

    W3DN_ErrorCode (*old_SetBlendMode)(struct W3DN_Context_s *self, W3DN_RenderState *renderState, uint32 buffIdx, W3DN_BlendMode src, W3DN_BlendMode dst);

    W3DN_ErrorCode (*old_SetBlendModeSeparate)(struct W3DN_Context_s *self, W3DN_RenderState *renderState, uint32 buffIdx,
        W3DN_BlendMode colSrc, W3DN_BlendMode colDst, W3DN_BlendMode alphaSrc, W3DN_BlendMode alphaDst);

    W3DN_ErrorCode (*old_SetColourMask)(struct W3DN_Context_s *self, W3DN_RenderState *renderState, uint32 index, uint8 mask);

    W3DN_ErrorCode (*old_SetDepthCompareFunc)(struct W3DN_Context_s *self, W3DN_RenderState *renderState, W3DN_CompareFunc func);

    W3DN_ErrorCode (*old_SetFrontFace)(struct W3DN_Context_s *self, W3DN_RenderState *renderState, W3DN_Face face);

    W3DN_ErrorCode (*old_SetLineWidth)(struct W3DN_Context_s *self, W3DN_RenderState *renderState, float width);

    W3DN_ErrorCode (*old_SetPolygonMode)(struct W3DN_Context_s *self, W3DN_RenderState *renderState, W3DN_FaceSelect face, W3DN_PolygonMode mode);

    W3DN_ErrorCode (*old_SetPolygonOffset)(struct W3DN_Context_s *self, W3DN_RenderState *renderState, float factor, float units, float clamp);

    W3DN_ErrorCode (*old_SetProvokingVertex)(struct W3DN_Context_s *self, W3DN_RenderState *renderState, W3DN_ProvokingVertexMode mode);

    W3DN_ErrorCode (*old_SetRenderTarget)(struct W3DN_Context_s *self,
    	W3DN_RenderState *renderState, W3DN_FrameBuffer *frameBuffer);

    W3DN_ErrorCode (*old_SetScissor)(struct W3DN_Context_s *self, W3DN_RenderState *renderState,
        uint32 x, uint32 y, uint32 width, uint32 height);

    W3DN_ErrorCode (*old_SetShaderPipeline)(struct W3DN_Context_s *self, W3DN_RenderState *renderState,
        W3DN_ShaderPipeline *shaderPipeline);

    W3DN_ErrorCode (*old_SetState)(struct W3DN_Context_s *self, W3DN_RenderState *renderState,
        W3DN_StateFlag stateFlag, W3DN_State value);

    W3DN_ErrorCode (*old_SetStencilFunc)(struct W3DN_Context_s *self, W3DN_RenderState *renderState,
        W3DN_CompareFunc func, uint32 ref, uint32 mask);

    W3DN_ErrorCode (*old_SetStencilFuncSeparate)(struct W3DN_Context_s *self, W3DN_RenderState *renderState,
        W3DN_FaceSelect face, W3DN_CompareFunc func, uint32 ref, uint32 mask);

    W3DN_ErrorCode (*old_SetStencilOp)(struct W3DN_Context_s *self, W3DN_RenderState *renderState,
        W3DN_StencilOp sFail, W3DN_StencilOp dpFail, W3DN_StencilOp dpPass);

    W3DN_ErrorCode (*old_SetStencilOpSeparate)(struct W3DN_Context_s *self, W3DN_RenderState *renderState,
        W3DN_FaceSelect face, W3DN_StencilOp sFail, W3DN_StencilOp dpFail, W3DN_StencilOp dpPass);

    W3DN_ErrorCode (*old_SetStencilWriteMask)(struct W3DN_Context_s *self, W3DN_RenderState *renderState, uint32 mask);

    W3DN_ErrorCode (*old_SetStencilWriteMaskSeparate)(struct W3DN_Context_s *self, W3DN_RenderState *renderState,
        W3DN_FaceSelect face, uint32 mask);

    W3DN_ErrorCode (*old_SetViewport)(struct W3DN_Context_s *self, W3DN_RenderState *renderState,
        double x, double y, double width, double height, double zNear, double zFar);

    uint32 (*old_ShaderGetCount)(struct W3DN_Context_s *self, W3DN_ErrorCode *errCode, W3DN_Shader *shader,
        W3DN_ShaderObjectType objectType);

    W3DN_ErrorCode (*old_ShaderGetObjectInfo)(struct W3DN_Context_s *self, W3DN_Shader *shader,
        W3DN_ShaderObjectType objectType, uint32 index, struct TagItem *tags);

    uint32 (*old_ShaderGetOffset)(struct W3DN_Context_s *self, W3DN_ErrorCode *errCode, W3DN_Shader *shader,
        W3DN_ShaderObjectType objectType, const char *name);

    uint64 (*old_ShaderGetTotalStorage)(struct W3DN_Context_s *self, W3DN_Shader *shader);

    W3DN_ShaderType (*old_ShaderGetType)(struct W3DN_Context_s *self, W3DN_Shader *shader);

    W3DN_Shader* (*old_ShaderPipelineGetShader)(struct W3DN_Context_s *self, W3DN_ShaderPipeline *shaderPipeline,
        W3DN_ShaderType shaderType);

    uint32 (*old_Submit)(struct W3DN_Context_s *self, W3DN_ErrorCode *errCode);

    W3DN_ErrorCode (*old_TexGenMipMaps)(struct W3DN_Context_s *self, W3DN_Texture *texture, uint32 base, uint32 last);

    W3DN_ErrorCode (*old_TexGetParameters)(struct W3DN_Context_s *self, W3DN_Texture *texture, struct TagItem *tags);

    W3DN_ErrorCode (*old_TexGetProperty)(struct W3DN_Context_s *self, W3DN_Texture *texture, W3DN_TextureProperty texProp, void *buffer);

    void* (*old_TexGetRMBuffer)(struct W3DN_Context_s *self, W3DN_Texture *texture);

    W3DN_ErrorCode (*old_TexGetSubResourceLayout)(struct W3DN_Context_s *self, W3DN_Texture *texture, W3DN_TexAspect aspect,
         uint32 mipLevel, uint32 arrayIdx, W3DN_ResourceLayout *layout);

    W3DN_ErrorCode (*old_TexSetParameters)(struct W3DN_Context_s *self, W3DN_Texture *texture, struct TagItem *tags);

    W3DN_ErrorCode (*old_TexUpdateImage)(struct W3DN_Context_s *self, W3DN_Texture *texture, void *source,
        uint32 level, uint32 arrayIdx, uint32 srcBytesPerRow, uint32 srcRowsPerLayer);

    W3DN_ErrorCode (*old_TexUpdateSubImage)(struct W3DN_Context_s *self, W3DN_Texture *texture, void *source,
        uint32 level, uint32 arrayIdx, uint32 srcBytesPerRow, uint32 srcRowsPerLayer,
        uint32 dstX, uint32 dstY, uint32 dstLayer, uint32 width, uint32 height, uint32 depth);

    W3DN_ErrorCode (*old_TSGetParameters)(struct W3DN_Context_s *self, W3DN_TextureSampler *texSampler, struct TagItem *tags);

    W3DN_ErrorCode (*old_TSSetParameters)(struct W3DN_Context_s *self, W3DN_TextureSampler *texSampler, struct TagItem *tags);

    W3DN_ErrorCode (*old_VBOGetArray)(struct W3DN_Context_s *self, W3DN_VertexBuffer *buffer,
    		uint32 arrayIdx, W3DN_ElementFormat *elementType, BOOL *normalized,
    		uint64 *numElements, uint64 *stride, uint64 *offset, uint64 *count);

    uint64 (*old_VBOGetAttr)(struct W3DN_Context_s *self, W3DN_VertexBuffer *vertexBuffer, W3DN_BufferAttribute attr);

    W3DN_BufferLock* (*old_VBOLock)(struct W3DN_Context_s *self, W3DN_ErrorCode *errCode,
    		W3DN_VertexBuffer *buffer, uint64 readOffset, uint64 readSize);

    W3DN_ErrorCode (*old_VBOSetArray)(struct W3DN_Context_s *self, W3DN_VertexBuffer *buffer,
    		uint32 arrayIdx, W3DN_ElementFormat elementType, BOOL normalized, uint64 numElements,
    		uint64 stride, uint64 offset, uint64 count);

    W3DN_ErrorCode (*old_WaitDone)(struct W3DN_Context_s *self, uint32 submitID, uint32 timeout);

    W3DN_ErrorCode (*old_WaitIdle)(struct W3DN_Context_s *self, uint32 timeout);
};

static struct NovaContext* contexts[MAX_CLIENTS];
static APTR mutex;

static const char* decodeTags(struct TagItem* tags, struct NovaContext* context)
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
            // W3DN_CreateContext
            TAG_U32(W3DNTag_BoardNum)
            TAG_HEX(W3DNTag_ModeID)
            TAG_HEX(W3DNTag_Screen)
            TAG_U32(W3DNTag_ReqFeature)
            TAG_U32(W3DNTag_Min)
            TAG_U32(W3DNTag_DesiredScreenWidth)
            TAG_U32(W3DNTag_DesiredScreenHeight)
            TAG_U32(W3DNTag_MinScreenWidth)
            TAG_U32(W3DNTag_MinScreenHeight)
            TAG_U32(W3DNTag_MaxScreenWidth)
            TAG_U32(W3DNTag_MaxScreenHeight)
            TAG_HEX(W3DNTag_GPU)
            TAG_U32(W3DNTag_DriverType)
            // FBBindBuffer
            TAG_HEX(W3DNTag_BitMap)
            TAG_HEX(W3DNTag_Texture)
            TAG_U32(W3DNTag_AllocDepthStencil)
            TAG_U32(W3DNTag_TextureMipLevel)
            TAG_U32(W3DNTag_ChannelOrder)
            // CompileShader
            TAG_HEX(W3DNTag_FileName)
            TAG_HEX(W3DNTag_DataBuffer)
            TAG_U32(W3DNTag_DataSize)
            TAG_HEX(W3DNTag_Log)
            TAG_U32(W3DNTag_LogLevel)
            // CreateShaderPipeline
            //TAG_HEX(W3DNTag_Shader) - TAG collision
            // ShaderGetObjectInfo
            TAG_HEX(W3DNTag_Offset)
            TAG_HEX(W3DNTag_SizeBytes)
            TAG_HEX(W3DNTag_Name)
            TAG_HEX(W3DNTag_Location)
            TAG_HEX(W3DNTag_Type)
            TAG_HEX(W3DNTag_ElementType)
            TAG_U32(W3DNTag_NumSubFields)
            TAG_U32(W3DNTag_SubFieldIdx)
            TAG_HEX(W3DNTag_ArrayLength)
            TAG_HEX(W3DNTag_ArrayStride)
            TAG_HEX(W3DNTag_MatrixStride)
            TAG_HEX(W3DNTag_IsRowMajor)
            TAG_HEX(W3DNTag_ArrayDims)
            TAG_U32(W3DNTag_ArrayDimIdx)
            // CreateVertexBufferObject / CreateDataBufferObject
            TAG_U32(W3DNTag_EndianConversion)
            TAG_U32(W3DNTag_OriginLowerLeft)
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

static const char* decodeShaderGetObjectInfoTags(struct TagItem* tags, struct NovaContext* context)
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
            // ShaderGetObjectInfo
            TAG_HEX(W3DNTag_Offset)
            TAG_HEX(W3DNTag_SizeBytes)
            TAG_HEX(W3DNTag_Name)
            TAG_HEX(W3DNTag_Location)
            TAG_HEX(W3DNTag_Type)
            TAG_HEX(W3DNTag_ElementType)
            TAG_HEX(W3DNTag_NumSubFields)
            TAG_U32(W3DNTag_SubFieldIdx)
            TAG_HEX(W3DNTag_ArrayLength)
            TAG_HEX(W3DNTag_ArrayStride)
            TAG_HEX(W3DNTag_MatrixStride)
            TAG_HEX(W3DNTag_IsRowMajor)
            TAG_HEX(W3DNTag_ArrayDims)
            TAG_U32(W3DNTag_ArrayDimIdx)
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

static const char* decodeTextureParameterTags(struct TagItem* tags, struct NovaContext* context)
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
            // SetTextureParameter, GetTextureParameter
            TAG_U32(W3DN_TEXTURE_BASE_LEVEL)
            TAG_HEX(W3DN_TEXTURE_MAX_LEVEL)
            TAG_HEX(W3DN_TEXTURE_DEPTH_STENCIL_MODE)
            TAG_HEX(W3DN_TEXTURE_SWIZZLE_R)
            TAG_HEX(W3DN_TEXTURE_SWIZZLE_G)
            TAG_HEX(W3DN_TEXTURE_SWIZZLE_B)
            TAG_HEX(W3DN_TEXTURE_SWIZZLE_A)
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

static const char* decodeTextureSamplerParameterTags(struct TagItem* tags, struct NovaContext* context)
{
    struct TagItem* iter = tags;
    struct TagItem* tag;

    char* dest = context->tagBuffer;
    size_t left = sizeof(context->tagBuffer);

    char temp[64];

    dest[0] = '\0';

    #define TAG_U32(x) case x: snprintf(temp, sizeof(temp), "[" #x ": %lu]", tag->ti_Data); break;
    #define TAG_HEX(x) case x: snprintf(temp, sizeof(temp), "[" #x ": 0x%lX]", tag->ti_Data); break;
    #define TAG_F32(x) case x: snprintf(temp, sizeof(temp), "[" #x ": %f]", (float)tag->ti_Data); break;

    while ((tag = IUtility->NextTagItem(&iter))) {
        switch (tag->ti_Tag) {
            // TSSetParameters, TSGetParameters
            TAG_U32(W3DN_TEXTURE_WRAP_S)
            TAG_U32(W3DN_TEXTURE_WRAP_T)
            TAG_U32(W3DN_TEXTURE_WRAP_R)
            TAG_U32(W3DN_TEXTURE_MIN_FILTER)
            TAG_U32(W3DN_TEXTURE_MAG_FILTER)
            TAG_HEX(W3DN_TEXTURE_BORDER_COLOUR)
            TAG_F32(W3DN_TEXTURE_MIN_LOD)
            TAG_F32(W3DN_TEXTURE_MAX_LOD)
            TAG_F32(W3DN_TEXTURE_LOD_BIAS)
            TAG_U32(W3DN_TEXTURE_COMPARE_MODE)
            TAG_U32(W3DN_TEXTURE_COMPARE_FUNC)
            TAG_U32(W3DN_TEXTURE_MAX_ANISOTROPY)
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
    #undef TAG_F32

    return context->tagBuffer;
}

static void profileResults(struct NovaContext* const context)
{
    if (!profilingStarted) {
        logAlways("Warp3D Nova profiling not started, skip summary");
        return;
    }

    PROF_FINISH_CONTEXT

    const double drawcalls = (double)(context->profiling[DrawElements].callCount + context->profiling[DrawArrays].callCount);

    // Copy items, otherwise sorthing will ruin the further profiling
    ProfilingItem stats[NovaFunctionCount];
    memcpy(stats, context->profiling, NovaFunctionCount * sizeof(ProfilingItem));

    sort(stats, NovaFunctionCount);

    logAlways("\nWarp3D Nova profiling results for %s:", context->name);

    PROF_PRINT_TOTAL

    logAlways("  Draw calls/s %.1f", drawcalls / seconds);

    logAlways("%30s | %10s | %10s | %10s | %20s | %20s | %24s | %20s",
        "function", "call count", "errors", "nullptrs", "duration (ms)", "avg. call dur. (us)", timeUsedBuffer, "% of CPU time");

    for (int i = 0; i < NovaFunctionCount; i++) {
        if (stats[i].callCount > 0) {
            logAlways("%30s | %10llu | %10llu | %10llu | %20.6f | %20.3f | %24.2f | %20.2f",
                mapNovaFunction(stats[i].index),
                stats[i].callCount,
                stats[i].errors,
                stats[i].nullptrs,
                timer_ticks_to_ms(stats[i].ticks),
                timer_ticks_to_us(stats[i].ticks) / (double)stats[i].callCount,
                (double)stats[i].ticks * 100.0 / (double)context->ticks,
                (double)stats[i].ticks * 100.0 / (double)totalTicks);
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

static struct NovaContext* find_context(const struct W3DN_Context_s* const context)
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
        context->profiling[id].nullptrs++;
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

#define NOVA_CALL(id, ...) \
if (context->old_ ## id) { \
    PROF_START \
    context->old_ ## id(self, ##__VA_ARGS__); \
    PROF_FINISH(id) \
} else { \
    logDebug("%s: " #id " function pointer is NULL (call ignored)", context->name); \
}

#define NOVA_CALL_RESULT(result, id, ...) \
if (context->old_ ## id) { \
    PROF_START \
    result = context->old_ ## id(self, ##__VA_ARGS__); \
    PROF_FINISH(id) \
} else { \
    logDebug("%s: " #id " function pointer is NULL (call ignored)", context->name); \
}

// Wrap traced calls

static W3DN_ErrorCode W3DN_BindBitMapAsTexture(struct W3DN_Context_s *self, W3DN_RenderState *renderState,
    uint32 texUnit, struct BitMap *bitMap, W3DN_TextureSampler *texSampler)
{
    W3DN_ErrorCode result = W3DNEC_SUCCESS;

    GET_CONTEXT

    logLine("%s: %s: renderState %p, texUnit %lu, bitMap %p, texSampler %p",
        context->name, __func__,
        renderState, texUnit, bitMap, texSampler);

    NOVA_CALL_RESULT(result, BindBitMapAsTexture, renderState, texUnit, bitMap, texSampler)

    logLine("%s: %s: <- result %d (%s)",
        context->name, __func__,
        result, mapNovaError(result));

    checkSuccess(context, BindBitMapAsTexture, result);

    return result;
}

static W3DN_ErrorCode W3DN_BindShaderDataBuffer(struct W3DN_Context_s *self, W3DN_RenderState *renderState,
    W3DN_ShaderType shaderType, W3DN_DataBuffer *buffer, uint32 bufferIdx)
{
    W3DN_ErrorCode result = W3DNEC_SUCCESS;

    GET_CONTEXT

    logLine("%s: %s: renderState %p, shaderType %u (%s), buffer %p, bufferIdx %lu",
        context->name, __func__,
        renderState,
        shaderType, decodeShaderType(shaderType),
        buffer, bufferIdx);

    NOVA_CALL_RESULT(result, BindShaderDataBuffer, renderState, shaderType, buffer, bufferIdx)

    logLine("%s: %s: <- result %d (%s)",
        context->name, __func__,
        result, mapNovaError(result));

    checkSuccess(context, BindShaderDataBuffer, result);

    return result;
}

static W3DN_ErrorCode W3DN_BindTexture(struct W3DN_Context_s *self, W3DN_RenderState *renderState,
    uint32 texUnit, W3DN_Texture *texture, W3DN_TextureSampler *texSampler)
{
    W3DN_ErrorCode result = W3DNEC_SUCCESS;

    GET_CONTEXT

    logLine("%s: %s: renderState %p, texUnit %lu, texture %p, texSampler %p",
        context->name, __func__,
        renderState, texUnit, texture, texSampler);

    NOVA_CALL_RESULT(result, BindTexture, renderState, texUnit, texture, texSampler)

    logLine("%s: %s: <- result %d (%s)",
        context->name, __func__,
        result, mapNovaError(result));

    checkSuccess(context, BindTexture, result);

    return result;
}

static W3DN_ErrorCode W3DN_BindVertexAttribArray(struct W3DN_Context_s *self,
		W3DN_RenderState *renderState, uint32 attribNum,
		W3DN_VertexBuffer *buffer, uint32 arrayIdx)
{
    W3DN_ErrorCode result = W3DNEC_SUCCESS;

    GET_CONTEXT

    logLine("%s: %s: renderState %p, attribNum %lu, buffer %p, arrayIdx %lu", context->name, __func__,
        renderState, attribNum, buffer, arrayIdx);

    NOVA_CALL_RESULT(result, BindVertexAttribArray, renderState, attribNum, buffer, arrayIdx)

    logLine("%s: %s: <- result %d (%s)", context->name, __func__,
        result, mapNovaError(result));

    checkSuccess(context, BindVertexAttribArray, result);

    return result;
}

static W3DN_ErrorCode W3DN_BufferUnlock(struct W3DN_Context_s *self,
		W3DN_BufferLock *bufferLock, uint64 writeOffset, uint64 writeSize)
{
    W3DN_ErrorCode result = W3DNEC_SUCCESS;

#if 0
    typedef union {
        uint32 u;
        float f;
    } Conversion;

    Conversion c;

    const size_t items = bufferLock->size / 4;
    for (size_t i = 0; i < items; i++) {
        c.u = bufferLock->buffer[i];
        logLine("[%u] = %f (%lx)", i, c.f, c.u);
    }
#endif

    GET_CONTEXT

    logLine("%s: %s: bufferLock %p, writeOffset %llu, writeSize %llu", context->name, __func__,
        bufferLock, writeOffset, writeSize);

    NOVA_CALL_RESULT(result, BufferUnlock, bufferLock, writeOffset, writeSize)

    logLine("%s: %s: <- result %d (%s)", context->name, __func__,
        result, mapNovaError(result));

    checkSuccess(context, BufferUnlock, result);

    return result;
}

static W3DN_ErrorCode W3DN_Clear(struct W3DN_Context_s *self, W3DN_RenderState *renderState,
    const float *colour, const double *depth, const uint32* stencil)
{
    W3DN_ErrorCode result = W3DNEC_SUCCESS;

    GET_CONTEXT

    logLine("%s: %s: renderState %p, colour %p (%f, %f, %f, %f), depth %p (%f), stencil %p (%lu)",
        context->name, __func__,
        renderState,
        colour,
        colour ? colour[0] : 0.0f,
        colour ? colour[1] : 0.0f,
        colour ? colour[2] : 0.0f,
        colour ? colour[3] : 0.0f,
        depth, depth ? *depth : 0.0f,
        stencil, stencil ? *stencil : 0);

    NOVA_CALL_RESULT(result, Clear, renderState, colour, depth, stencil)

    logLine("%s: %s: <- result %d (%s)",
        context->name, __func__,
        result, mapNovaError(result));

    checkSuccess(context, Clear, result);

    return result;
}

static W3DN_Shader* W3DN_CompileShader(struct W3DN_Context_s *self,
    W3DN_ErrorCode *errCode, struct TagItem *tags)
{
    W3DN_Shader *shader = NULL;

    GET_CONTEXT

    logLine("%s: %s: errCode %p, tags %p (%s)",
        context->name, __func__,
        errCode,
        tags, decodeTags(tags, context));

    NOVA_CALL_RESULT(shader, CompileShader, errCode, tags)

    logLine("%s: %s: <- errCode %d (%s). Shader address %p",
        context->name, __func__,
        mapNovaErrorPointerToCode(errCode),
        mapNovaErrorPointerToString(errCode),
        shader);

    checkPointer(context, CompileShader, shader);
    checkSuccess(context, CompileShader, mapNovaErrorPointerToCode(errCode));

    return shader;
}

static W3DN_DataBuffer* W3DN_CreateDataBufferObject(struct W3DN_Context_s *self, W3DN_ErrorCode *errCode,
    uint64 size, W3DN_BufferUsage usage, uint32 maxBuffers, struct TagItem *tags)
{
    W3DN_DataBuffer *buffer = NULL;

    GET_CONTEXT

    logLine("%s: %s: errCode %p, size %llu, usage %u (%s), maxBuffers %lu, tags %p (%s)",
        context->name, __func__,
        errCode,
        size,
        usage, decodeBufferUsage(usage),
        maxBuffers,
        tags, decodeTags(tags, context));

    NOVA_CALL_RESULT(buffer, CreateDataBufferObject, errCode, size, usage, maxBuffers, tags)

    logLine("%s: %s: <- errCode %d (%s). Data buffer object address %p",
        context->name, __func__,
        mapNovaErrorPointerToCode(errCode),
        mapNovaErrorPointerToString(errCode),
        buffer);

    checkPointer(context, CreateDataBufferObject, buffer);
    checkSuccess(context, CreateDataBufferObject, mapNovaErrorPointerToCode(errCode));

    return buffer;
}

static W3DN_FrameBuffer* W3DN_CreateFrameBuffer(struct W3DN_Context_s *self, W3DN_ErrorCode *errCode)
{
    W3DN_FrameBuffer* buffer = NULL;

    GET_CONTEXT

    logLine("%s: %s: errCode %p",
        context->name, __func__,
        errCode);

    NOVA_CALL_RESULT(buffer, CreateFrameBuffer, errCode)

    logLine("%s: %s: <- errCode %d (%s). Frame buffer address %p",
        context->name, __func__,
        mapNovaErrorPointerToCode(errCode),
        mapNovaErrorPointerToString(errCode),
        buffer);

    checkPointer(context, CreateFrameBuffer, buffer);
    checkSuccess(context, CreateFrameBuffer, mapNovaErrorPointerToCode(errCode));

    return buffer;
}

static W3DN_RenderState* W3DN_CreateRenderStateObject(struct W3DN_Context_s *self, W3DN_ErrorCode *errCode)
{
    W3DN_RenderState* state = NULL;

    GET_CONTEXT

    logLine("%s: %s: errCode %p",
        context->name, __func__,
        errCode);

    NOVA_CALL_RESULT(state, CreateRenderStateObject, errCode)

    logLine("%s: %s: <- errCode %d (%s). Render state object address %p",
        context->name, __func__,
        mapNovaErrorPointerToCode(errCode),
        mapNovaErrorPointerToString(errCode),
        state);

    checkPointer(context, CreateRenderStateObject, state);
    checkSuccess(context, CreateRenderStateObject, mapNovaErrorPointerToCode(errCode));

    return state;
}

static W3DN_ShaderPipeline* W3DN_CreateShaderPipeline(struct W3DN_Context_s *self, W3DN_ErrorCode *errCode, struct TagItem *tags)
{
    W3DN_ShaderPipeline* pipeline = NULL;

    GET_CONTEXT

    logLine("%s: %s: errCode %p, tags %p (%s)",
        context->name, __func__,
        errCode, tags, decodeTags(tags, context));

    NOVA_CALL_RESULT(pipeline, CreateShaderPipeline, errCode, tags)

    logLine("%s: %s: <- errCode %d (%s). Shader pipeline address %p",
        context->name, __func__,
        mapNovaErrorPointerToCode(errCode),
        mapNovaErrorPointerToString(errCode),
        pipeline);

    checkPointer(context, CreateShaderPipeline, pipeline);
    checkSuccess(context, CreateShaderPipeline, mapNovaErrorPointerToCode(errCode));

    return pipeline;
}

static W3DN_TextureSampler* W3DN_CreateTexSampler(struct W3DN_Context_s *self, W3DN_ErrorCode *errCode)
{
    W3DN_TextureSampler* sampler = NULL;

    GET_CONTEXT

    logLine("%s: %s: errCode %p",
        context->name, __func__,
        errCode);

    NOVA_CALL_RESULT(sampler, CreateTexSampler, errCode)

    logLine("%s: %s: <- errCode %d (%s). Texture sampler address %p",
        context->name, __func__,
        mapNovaErrorPointerToCode(errCode),
        mapNovaErrorPointerToString(errCode),
        sampler);

    checkPointer(context, CreateTexSampler, sampler);
    checkSuccess(context, CreateTexSampler, mapNovaErrorPointerToCode(errCode));

    return sampler;
}

static W3DN_Texture* W3DN_CreateTexture(struct W3DN_Context_s *self, W3DN_ErrorCode *errCode, W3DN_TextureType texType,
    W3DN_PixelFormat pixelFormat, W3DN_ElementFormat elementFormat, uint32 width, uint32 height, uint32 depth,
    BOOL mipmapped, W3DN_BufferUsage usage)
{
    W3DN_Texture* texture = NULL;

    GET_CONTEXT

    logLine("%s: %s: errCode %p, texType %u (%s), pixelFormat %u (%s), elementFormat %u (%s), width %lu, height %lu, depth %lu, "
        "mipmapped %d, usage %u (%s)",
        context->name, __func__,
        errCode,
        texType, decodeTextureType(texType),
        pixelFormat, decodePixelFormat(pixelFormat),
        elementFormat, decodeElementFormat(elementFormat),
        width,
        height,
        depth,
        mipmapped,
        usage, decodeBufferUsage(usage));

    NOVA_CALL_RESULT(texture, CreateTexture, errCode, texType, pixelFormat, elementFormat,
        width, height, depth, mipmapped, usage)

    logLine("%s: %s: <- errCode %d (%s). Texture address %p",
        context->name, __func__,
        mapNovaErrorPointerToCode(errCode),
        mapNovaErrorPointerToString(errCode),
        texture);

    checkPointer(context, CreateTexture, texture);
    checkSuccess(context, CreateTexture, mapNovaErrorPointerToCode(errCode));

    return texture;
}

static W3DN_Texture* W3DN_CreateTextureExtRMB(struct W3DN_Context_s *self, W3DN_ErrorCode *errCode, void *rmBuffer,
    W3DN_ResourceLayout * layout, W3DN_TextureType texType, W3DN_PixelFormat pixelFormat, W3DN_ElementFormat elementFormat,
    uint32 width, uint32 height, uint32 depth, BOOL mipmapped)
{
    W3DN_Texture* texture = NULL;

    GET_CONTEXT

    logLine("%s: %s: errCode %p, rmBuffer %p, layout %p, texType %d (%s), pixelFormat %d (%s), elementFormat %d (%s), "
        "width %lu, height %lu, depth %lu, mipmapped %d",
        context->name, __func__,
        errCode,
        rmBuffer,
        layout,
        texType, decodeTextureType(texType),
        pixelFormat, decodePixelFormat(pixelFormat),
        elementFormat, decodeElementFormat(elementFormat),
        width,
        height,
        depth,
        mipmapped);

    NOVA_CALL_RESULT(texture, CreateTextureExtRMB, errCode, rmBuffer, layout, texType, pixelFormat, elementFormat,
        width, height, depth, mipmapped)

    logLine("%s: %s: <- errCode %d (%s). Texture address %p",
        context->name, __func__,
        mapNovaErrorPointerToCode(errCode),
        mapNovaErrorPointerToString(errCode),
        texture);

    checkPointer(context, CreateTextureExtRMB, texture);
    checkSuccess(context, CreateTextureExtRMB, mapNovaErrorPointerToCode(errCode));

    return texture;
}

static W3DN_VertexBuffer* W3DN_CreateVertexBufferObject(struct W3DN_Context_s *self,
		W3DN_ErrorCode *errCode, uint64 size, W3DN_BufferUsage usage, uint32 maxArrays, struct TagItem *tags)
{
    W3DN_VertexBuffer* result = NULL;

    GET_CONTEXT

    logLine("%s: %s: errCode %p, size %llu, usage %u (%s), maxArrays %lu, tags %p (%s)",
        context->name, __func__,
        errCode, size,
        usage, decodeBufferUsage(usage),
        maxArrays,
        tags, decodeTags(tags, context));

    NOVA_CALL_RESULT(result, CreateVertexBufferObject, errCode, size, usage, maxArrays, tags)

    logLine("%s: %s: <- errCode %d (%s). Buffer address %p",
        context->name, __func__,
        mapNovaErrorPointerToCode(errCode),
        mapNovaErrorPointerToString(errCode),
        result);

    checkPointer(context, CreateVertexBufferObject, result);
    checkSuccess(context, CreateVertexBufferObject, mapNovaErrorPointerToCode(errCode));

    return result;
}

static uint64 W3DN_DBOGetAttr(struct W3DN_Context_s *self, W3DN_DataBuffer *dataBuffer, W3DN_BufferAttribute attr)
{
    uint64 result = 0;

    GET_CONTEXT

    logLine("%s: %s: dataBuffer %p, attr %u (%s)",
        context->name, __func__,
        dataBuffer,
        attr, decodeBufferAttribute(attr));

    NOVA_CALL_RESULT(result, DBOGetAttr, dataBuffer, attr)

    logLine("%s: %s: <- Result %llu",
        context->name, __func__,
        result);

    return result;
}

static W3DN_ErrorCode W3DN_DBOGetBuffer(struct W3DN_Context_s *self, W3DN_DataBuffer *dataBuffer, uint32 bufferIdx,
    uint64 *offset, uint64 *size, W3DN_Shader **targetShader, struct TagItem *tags)
{
    W3DN_ErrorCode result = W3DNEC_SUCCESS;

    GET_CONTEXT

    logLine("%s: %s: dataBuffer %p, bufferIdx %lu, offset %p, size %p, targetShader %p, tags %p (%s)",
        context->name, __func__,
        dataBuffer, bufferIdx, offset, size, targetShader,
        tags, decodeTags(tags, context));

    NOVA_CALL_RESULT(result, DBOGetBuffer, dataBuffer, bufferIdx, offset, size, targetShader, tags)

    logLine("%s: %s: <- offset %llu, size %llu, targetShader %p. Result %d (%s)",
        context->name, __func__,
        *offset, *size, *targetShader,
        result, mapNovaError(result));

    checkSuccess(context, DBOGetBuffer, result);

    return result;
}

static W3DN_BufferLock* W3DN_DBOLock(struct W3DN_Context_s *self, W3DN_ErrorCode *errCode, W3DN_DataBuffer *buffer, uint64 readOffset, uint64 readSize)
{
    W3DN_BufferLock* lock = NULL;

    GET_CONTEXT

    logLine("%s: %s: errCode %p, buffer %p, readOffset %llu, readSize %llu",
        context->name, __func__,
        errCode, buffer, readOffset, readSize);

    NOVA_CALL_RESULT(lock, DBOLock, errCode, buffer, readOffset, readSize)

    logLine("%s: %s: <- errCode %d (%s). Buffer lock address %p",
        context->name, __func__,
        mapNovaErrorPointerToCode(errCode),
        mapNovaErrorPointerToString(errCode),
        lock);

    checkPointer(context, DBOLock, lock);
    checkSuccess(context, DBOLock, mapNovaErrorPointerToCode(errCode));

    return lock;
}

static W3DN_ErrorCode W3DN_DBOSetBuffer(struct W3DN_Context_s *self, W3DN_DataBuffer *dataBuffer, uint32 bufferIdx,
    uint64 offset, uint64 size, W3DN_Shader *targetShader, struct TagItem *tags)
{
    W3DN_ErrorCode result = W3DNEC_SUCCESS;

    GET_CONTEXT

    logLine("%s: %s: dataBuffer %p, bufferIdx %lu, offset %llu. size %llu, targetShader %p, tags %p (%s)",
        context->name, __func__,
        dataBuffer, bufferIdx, offset, size, targetShader,
        tags, decodeTags(tags, context));

    NOVA_CALL_RESULT(result, DBOSetBuffer, dataBuffer, bufferIdx, offset, size, targetShader, tags)

    logLine("%s: %s: <- Result %d (%s)",
        context->name, __func__,
        result, mapNovaError(result));

    checkSuccess(context, DBOSetBuffer, result);

    return result;
}

static void W3DN_Destroy(struct W3DN_Context_s *self)
{
    GET_CONTEXT

    logLine("%s: %s",
        context->name, __func__);

    NOVA_CALL(Destroy)

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

static void W3DN_DestroyDataBufferObject(struct W3DN_Context_s *self, W3DN_DataBuffer *dataBuffer)
{
    GET_CONTEXT

    logLine("%s: %s: dataBuffer %p",
        context->name, __func__,
        dataBuffer);

    NOVA_CALL(DestroyDataBufferObject, dataBuffer)
}

static void W3DN_DestroyFrameBuffer(struct W3DN_Context_s *self, W3DN_FrameBuffer *frameBuffer)
{
    GET_CONTEXT

    logLine("%s: %s: frameBuffer %p",
        context->name, __func__,
        frameBuffer);

    NOVA_CALL(DestroyFrameBuffer, frameBuffer)
}

static void W3DN_DestroyRenderStateObject(struct W3DN_Context_s *self, W3DN_RenderState *renderState)
{
    GET_CONTEXT

    logLine("%s: %s: renderState %p",
        context->name, __func__,
        renderState);

    NOVA_CALL(DestroyRenderStateObject, renderState)
}

static void W3DN_DestroyShader(struct W3DN_Context_s *self, W3DN_Shader *shader)
{
    GET_CONTEXT

    logLine("%s: %s: shader %p",
        context->name, __func__,
        shader);

    NOVA_CALL(DestroyShader, shader)
}

static void W3DN_DestroyShaderLog(struct W3DN_Context_s *self, const char *shaderLog)
{
    GET_CONTEXT

    logLine("%s: %s: shaderLog %p",
        context->name, __func__,
        shaderLog);

    NOVA_CALL(DestroyShaderLog, shaderLog)
}

static void W3DN_DestroyShaderPipeline(struct W3DN_Context_s *self, W3DN_ShaderPipeline *shaderPipeline)
{
    GET_CONTEXT

    logLine("%s: %s: shaderPipeline %p",
        context->name, __func__,
        shaderPipeline);

    NOVA_CALL(DestroyShaderPipeline, shaderPipeline)
}

static void W3DN_DestroyTexSampler(struct W3DN_Context_s *self, W3DN_TextureSampler *texSampler)
{
    GET_CONTEXT

    logLine("%s: %s: texSampler %p",
        context->name, __func__,
        texSampler);

    NOVA_CALL(DestroyTexSampler, texSampler)
}

static void W3DN_DestroyTexture(struct W3DN_Context_s *self, W3DN_Texture *texture)
{
    GET_CONTEXT

    logLine("%s: %s: texture %p",
        context->name, __func__,
        texture);

    NOVA_CALL(DestroyTexture, texture)
}

static void W3DN_DestroyVertexBufferObject(struct W3DN_Context_s *self, W3DN_VertexBuffer *vertexBuffer)
{
    GET_CONTEXT

    logLine("%s: %s: vertexBuffer %p", context->name, __func__,
        vertexBuffer);

    NOVA_CALL(DestroyVertexBufferObject, vertexBuffer)
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
    W3DN_ErrorCode result = W3DNEC_SUCCESS;

    GET_CONTEXT

    logLine("%s: %s: renderState %p, primitive %u (%s), base %lu, count %lu", context->name, __func__,
        renderState,
        primitive, decodePrimitive(primitive),
        base, count);

    NOVA_CALL_RESULT(result, DrawArrays, renderState, primitive, base, count)

    logLine("%s: %s: <- Result %d (%s)", context->name, __func__,
        result, mapNovaError(result));

    countPrimitive(&context->counter, primitive, count);
    checkSuccess(context, DrawArrays, result);

    return result;
}

static W3DN_ErrorCode W3DN_DrawElements(struct W3DN_Context_s *self,
		W3DN_RenderState *renderState, W3DN_Primitive primitive, uint32 baseVertex, uint32 count,
		W3DN_VertexBuffer *indexBuffer, uint32 arrayIdx)
{
    W3DN_ErrorCode result = W3DNEC_SUCCESS;

    GET_CONTEXT

    logLine("%s: %s: renderState %p, primitive %u (%s), baseVertex %lu, count %lu, indexBuffer %p, arrayIdx %lu",
        context->name, __func__,
        renderState,
        primitive, decodePrimitive(primitive),
        baseVertex, count, indexBuffer, arrayIdx);

    NOVA_CALL_RESULT(result, DrawElements, renderState, primitive, baseVertex, count, indexBuffer, arrayIdx)

    logLine("%s: %s: <- Result %d (%s)",
        context->name, __func__,
        result, mapNovaError(result));

    countPrimitive(&context->counter, primitive, count);
    checkSuccess(context, DrawElements, result);

    return result;
}

static W3DN_ErrorCode W3DN_FBBindBuffer(struct W3DN_Context_s *self,
	W3DN_FrameBuffer *frameBuffer, int32 attachmentPt, struct TagItem *tags)
{
    W3DN_ErrorCode result = W3DNEC_SUCCESS;

    GET_CONTEXT

    logLine("%s: %s: frameBuffer %p, attachmentPt %d, tags %p (%s)",
        context->name, __func__,
        frameBuffer, (int)attachmentPt,
        tags, decodeTags(tags, context));

    NOVA_CALL_RESULT(result, FBBindBuffer, frameBuffer, attachmentPt, tags);

    logLine("%s: %s: <- Result %d (%s)",
        context->name, __func__,
        result, mapNovaError(result));

    checkSuccess(context, FBBindBuffer, result);

    return result;
}

static uint64 W3DN_FBGetAttr(struct W3DN_Context_s *self,
	W3DN_FrameBuffer *frameBuffer, W3DN_FrameBufferAttribute attrib)
{
    uint64 result = 0;

    GET_CONTEXT

    logLine("%s: %s: frameBuffer %p, attrib %u (%s)",
        context->name, __func__,
        frameBuffer,
        attrib, decodeFrameBufferAttribute(attrib));

    NOVA_CALL_RESULT(result, FBGetAttr, frameBuffer, attrib)

    logLine("%s: %s: <- Result %llu",
        context->name, __func__,
        result);

    return result;
}

static struct BitMap* W3DN_FBGetBufferBM(struct W3DN_Context_s *self,
	W3DN_FrameBuffer *frameBuffer, uint32 attachmentPt, W3DN_ErrorCode *errCode)
{
    struct BitMap* bitmap = NULL;

    GET_CONTEXT

    logLine("%s: %s: frameBuffer %p, attachmentPt %lu, errCode %p",
        context->name, __func__,
        frameBuffer, attachmentPt, errCode);

    NOVA_CALL_RESULT(bitmap, FBGetBufferBM, frameBuffer, attachmentPt, errCode)

    logLine("%s: %s: <- Bitmap address %p. Result %d (%s)",
        context->name, __func__,
        bitmap,
        mapNovaErrorPointerToCode(errCode),
        mapNovaErrorPointerToString(errCode));

    checkPointer(context, FBGetBufferBM, bitmap);
    checkSuccess(context, FBGetBufferBM, mapNovaErrorPointerToCode(errCode));

    return bitmap;
}

static W3DN_Texture*  W3DN_FBGetBufferTex(struct W3DN_Context_s *self,
	W3DN_FrameBuffer *frameBuffer, uint32 attachmentPt, W3DN_ErrorCode *errCode)
{
    W3DN_Texture * texture = NULL;

    GET_CONTEXT

    logLine("%s: %s: frameBuffer %p, attachmentPt %lu, errCode %p",
        context->name, __func__,
        frameBuffer, attachmentPt, errCode);

    NOVA_CALL_RESULT(texture, FBGetBufferTex, frameBuffer, attachmentPt, errCode)

    logLine("%s: %s: <- Texture address %p. Result %d (%s)",
        context->name, __func__,
        texture,
        mapNovaErrorPointerToCode(errCode),
        mapNovaErrorPointerToString(errCode));

    checkPointer(context, FBGetBufferTex, texture);
    checkSuccess(context, FBGetBufferTex, mapNovaErrorPointerToCode(errCode));

    return texture;
}

static W3DN_ErrorCode W3DN_FBGetStatus(struct W3DN_Context_s *self, W3DN_FrameBuffer *frameBuffer)
{
    W3DN_ErrorCode result = W3DNEC_SUCCESS;

    GET_CONTEXT

    logLine("%s: %s: frameBuffer %p",
        context->name, __func__,
        frameBuffer);

    NOVA_CALL_RESULT(result, FBGetStatus, frameBuffer)

    logLine("%s: %s: <- Result %d (%s)",
        context->name, __func__,
        result,
        mapNovaError(result));

    checkSuccess(context, FBGetStatus, result);

    return result;
}

static struct BitMap* W3DN_GetBitMapTexture(struct W3DN_Context_s *self, W3DN_RenderState *renderState, uint32 texUnit)
{
    struct BitMap* bitmap = NULL;

    GET_CONTEXT

    logLine("%s: %s: renderState %p, texUnit %lu",
        context->name, __func__,
        renderState, texUnit);

    NOVA_CALL_RESULT(bitmap, GetBitMapTexture, renderState, texUnit)

    logLine("%s: %s: <- Bitmap address %p",
        context->name, __func__,
        bitmap);

    checkPointer(context, GetBitMapTexture, bitmap);

    return bitmap;
}

static W3DN_ErrorCode W3DN_GetBlendColour(struct W3DN_Context_s *self, W3DN_RenderState *renderState,
    float *red, float *green, float *blue, float *alpha)
{
    W3DN_ErrorCode result = W3DNEC_SUCCESS;

    GET_CONTEXT

    logLine("%s: %s: renderState %p, red %p, green %p, blue %p, alpha %p",
        context->name, __func__,
        renderState, red, green, blue, alpha);

    NOVA_CALL_RESULT(result, GetBlendColour, renderState, red, green, blue, alpha);

    logLine("%s: %s: <- red %f, green %f, blue %f, alpha %f. Result %d (%s)",
        context->name, __func__,
        *red, *green, *blue, *alpha,
        result,
        mapNovaError(result));

    checkSuccess(context, GetBlendColour, result);

    return result;
}

static W3DN_ErrorCode W3DN_GetBlendEquation(struct W3DN_Context_s *self, W3DN_RenderState *renderState,
    uint32 buffIdx, W3DN_BlendEquation *colEquation, W3DN_BlendEquation *alphaEquation)
{
    W3DN_ErrorCode result = W3DNEC_SUCCESS;

    GET_CONTEXT

    logLine("%s: %s: renderState %p, buffIdx %lu, colEquation %p, alphaEquation %p",
        context->name, __func__,
        renderState, buffIdx, colEquation, alphaEquation);

    NOVA_CALL_RESULT(result, GetBlendEquation, renderState, buffIdx, colEquation, alphaEquation);

    logLine("%s: %s: <- colEquation %u (%s), alphaEquation %u (%s). Result %d (%s)",
        context->name, __func__,
        *colEquation, decodeBlendEquation(*colEquation),
        *alphaEquation, decodeBlendEquation(*alphaEquation),
        result,
        mapNovaError(result));

    checkSuccess(context, GetBlendEquation, result);

    return result;
}

static W3DN_ErrorCode W3DN_GetBlendMode(struct W3DN_Context_s *self, W3DN_RenderState *renderState, uint32 buffIdx, W3DN_BlendMode *colSrc,
    W3DN_BlendMode *colDst, W3DN_BlendMode *alphaSrc, W3DN_BlendMode *alphaDst)
{
    W3DN_ErrorCode result = W3DNEC_SUCCESS;

    GET_CONTEXT

    logLine("%s: %s: renderState %p, buffIdx %lu, colSrc %p, colDst %p, alphaSrc %p, alphaDst %p",
        context->name, __func__,
        renderState, buffIdx, colSrc, colDst, alphaSrc, alphaDst);

    NOVA_CALL_RESULT(result, GetBlendMode, renderState, buffIdx, colSrc, colDst, alphaSrc, alphaDst)

    logLine("%s: %s: <- colSrc %u (%s), colDst %u (%s), alphaSrc %u (%s), alphaDst %u (%s). Result %d (%s)",
        context->name, __func__,
        *colSrc, decodeBlendMode(*colSrc),
        *colDst, decodeBlendMode(*colDst),
        *alphaSrc, decodeBlendMode(*alphaSrc),
        *alphaDst, decodeBlendMode(*alphaDst),
        result,
        mapNovaError(result));

    checkSuccess(context, GetBlendMode, result);

    return result;
}

static uint8 W3DN_GetColourMask(struct W3DN_Context_s *self, W3DN_RenderState *renderState, uint32 index)
{
    uint8 mask = 0;

    GET_CONTEXT

    logLine("%s: %s: renderState %p, index %lu",
        context->name, __func__,
        renderState, index);

    NOVA_CALL_RESULT(mask, GetColourMask, renderState, index)

    logLine("%s: %s: <- Mask value 0x%x",
        context->name, __func__,
        mask);

    return mask;
}

static W3DN_CompareFunc W3DN_GetDepthCompareFunc(struct W3DN_Context_s *self, W3DN_RenderState *renderState)
{
    W3DN_CompareFunc function = 0;

    GET_CONTEXT

    logLine("%s: %s: renderState %p",
        context->name, __func__,
        renderState);

    NOVA_CALL_RESULT(function, GetDepthCompareFunc, renderState)

    logLine("%s: %s: <- Compare function %u (%s)",
        context->name, __func__,
        function, decodeCompareFunc(function));

    return function;
}

static W3DN_Face W3DN_GetFrontFace(struct W3DN_Context_s *self, W3DN_RenderState *renderState)
{
    W3DN_Face face = 0;

    GET_CONTEXT

    logLine("%s: %s: renderState %p",
        context->name, __func__,
        renderState);

    NOVA_CALL_RESULT(face, GetFrontFace, renderState)

    logLine("%s: %s: <- Front face %u (%s)",
        context->name, __func__,
        face, decodeFace(face));

    return face;
}

static float W3DN_GetLineWidth(struct W3DN_Context_s *self, W3DN_RenderState *renderState)
{
    float width = 0.0f;

    GET_CONTEXT

    logLine("%s: %s: renderState %p",
        context->name, __func__,
        renderState);

    NOVA_CALL_RESULT(width, GetLineWidth, renderState);

    logLine("%s: %s: <- Line width %f",
        context->name, __func__,
        width);

    return width;
}

static W3DN_FrameBuffer* W3DN_GetRenderTarget(
    struct W3DN_Context_s *self, W3DN_RenderState *renderState)
{
    W3DN_FrameBuffer* buffer = NULL;

    GET_CONTEXT

    logLine("%s: %s: renderState %p",
        context->name, __func__,
        renderState);

    NOVA_CALL_RESULT(buffer, GetRenderTarget, renderState)

    logLine("%s: %s: <- Frame buffer address %p",
        context->name, __func__,
        buffer);

    checkPointer(context, GetRenderTarget, buffer);

    return buffer;
}

static W3DN_PolygonMode W3DN_GetPolygonMode(struct W3DN_Context_s *self, W3DN_RenderState *renderState,
    W3DN_FaceSelect face)
{
    W3DN_PolygonMode mode = 0;

    GET_CONTEXT

    logLine("%s: %s: renderState %p, face %u (%s)",
        context->name, __func__,
        renderState,
        face, decodeFaceSelect(face));

    NOVA_CALL_RESULT(mode, GetPolygonMode, renderState, face)

    logLine("%s: %s: <- Polygon mode %u (%s)",
        context->name, __func__,
        mode, decodePolygonMode(mode));

    return mode;
}

static W3DN_ErrorCode W3DN_GetPolygonOffset(struct W3DN_Context_s *self, W3DN_RenderState *renderState,
    float *factor, float *units, float *clamp)
{
    W3DN_ErrorCode result = W3DNEC_SUCCESS;

    GET_CONTEXT

    logLine("%s: %s: renderState %p, factor %p, units %p, clamp %p",
        context->name, __func__,
        renderState, factor, units, clamp);

    NOVA_CALL_RESULT(result, GetPolygonOffset, renderState, factor, units, clamp);

    logLine("%s: %s: <- factor %f, units %f, clamp %f. Result %d (%s)",
        context->name, __func__,
        *factor,
        *units,
        *clamp,
        result, mapNovaError(result));

    checkSuccess(context, GetPolygonOffset, result);

    return result;
}

static W3DN_ProvokingVertexMode W3DN_GetProvokingVertex(struct W3DN_Context_s *self, W3DN_RenderState *renderState)
{
    W3DN_ProvokingVertexMode mode = 0;

    GET_CONTEXT

    logLine("%s: %s: renderState %p",
        context->name, __func__,
        renderState);

    NOVA_CALL_RESULT(mode, GetProvokingVertex, renderState)

    logLine("%s: %s: <- Vertex mode %u (%s)",
        context->name, __func__,
        mode, decodeProvokingVertexMode(mode));

    return mode;
}

static W3DN_ErrorCode W3DN_GetScissor(struct W3DN_Context_s *self, W3DN_RenderState *renderState,
    uint32 *x, uint32 *y, uint32 *width, uint32 *height)
{
    W3DN_ErrorCode result = W3DNEC_SUCCESS;

    GET_CONTEXT

    logLine("%s: %s: renderState %p, x %p, y %p, width %p, height %p",
        context->name, __func__,
        renderState, x, y, width, height);

    NOVA_CALL_RESULT(result, GetScissor, renderState, x, y, width, height);

    logLine("%s: %s: <- x %lu, y %lu, width %lu, height %lu. Result %d (%s)",
        context->name, __func__,
        *x,
        *y,
        *width,
        *height,
        result, mapNovaError(result));

    checkSuccess(context, GetScissor, result);

    return result;
}

static W3DN_ErrorCode W3DN_GetShaderDataBuffer(struct W3DN_Context_s *self, W3DN_RenderState *renderState,
    W3DN_ShaderType shaderType, W3DN_DataBuffer **buffer, uint32 *bufferIdx)
{
    W3DN_ErrorCode result = W3DNEC_SUCCESS;

    GET_CONTEXT

    logLine("%s: %s: renderState %p, shaderType %u (%s), buffer %p, bufferIdx %p",
        context->name, __func__,
        renderState,
        shaderType, decodeShaderType(shaderType),
        buffer,
        bufferIdx);

    NOVA_CALL_RESULT(result, GetShaderDataBuffer, renderState, shaderType, buffer, bufferIdx)

    logLine("%s: %s: <- buffer %p, bufferIdx %lu. Result %d (%s)",
        context->name, __func__,
        *buffer,
        *bufferIdx,
        result, mapNovaError(result));

    checkSuccess(context, GetShaderDataBuffer, result);

    return result;
}

static W3DN_ShaderPipeline* W3DN_GetShaderPipeline(struct W3DN_Context_s *self, W3DN_RenderState *renderState)
{
    W3DN_ShaderPipeline* pipeline = NULL;

    GET_CONTEXT

    logLine("%s: %s: renderState %p",
        context->name, __func__,
        renderState);

    NOVA_CALL_RESULT(pipeline, GetShaderPipeline, renderState)

    logLine("%s: %s: <- Shader pipeline address %p",
        context->name, __func__,
        pipeline);

    checkPointer(context, GetShaderPipeline, pipeline); // TODO: error or not?

    return pipeline;
}

static W3DN_State W3DN_GetState(struct W3DN_Context_s *self, W3DN_RenderState *renderState, W3DN_StateFlag stateFlag)
{
    W3DN_State state = 0;

    GET_CONTEXT

    logLine("%s: %s: renderState %p, stateFlag %u (%s)",
        context->name, __func__,
        renderState,
        stateFlag, decodeStateFlag(stateFlag));

    NOVA_CALL_RESULT(state, GetState, renderState, stateFlag)

    logLine("%s: %s: <- State %u (%s)",
        context->name, __func__,
        state, decodeState(state));

    return state;
}

static W3DN_ErrorCode W3DN_GetStencilFunc(struct W3DN_Context_s *self, W3DN_RenderState *renderState,
    W3DN_FaceSelect face, W3DN_CompareFunc *func, uint32 *ref, uint32 *mask)
{
    W3DN_ErrorCode result = W3DNEC_SUCCESS;

    GET_CONTEXT

    logLine("%s: %s: renderState %p, face %u (%s), func %p, ref %p, mask %p",
        context->name, __func__,
        renderState,
        face, decodeFaceSelect(face),
        func, ref, mask);

    NOVA_CALL_RESULT(result, GetStencilFunc, renderState, face, func, ref, mask);

    logLine("%s: %s: <- func %u (%s), ref %lu, mask 0x%lx. Result %d (%s)",
        context->name, __func__,
        func ? *func : 0,
        func ? decodeCompareFunc(*func) : "",
        ref ? *ref : 0,
        mask ? *mask : 0,
        result, mapNovaError(result));

    checkSuccess(context, GetStencilFunc, result);

    return result;
}

static W3DN_ErrorCode W3DN_GetStencilOp(struct W3DN_Context_s *self, W3DN_RenderState *renderState,
    W3DN_FaceSelect face, W3DN_StencilOp *sFail, W3DN_StencilOp *dpFail, W3DN_StencilOp *dpPass)
{
    W3DN_ErrorCode result = W3DNEC_SUCCESS;

    GET_CONTEXT

    logLine("%s: %s: renderState %p, face %u (%s), sFail %p, dpFail %p, dpPass %p",
        context->name, __func__,
        renderState,
        face, decodeFaceSelect(face),
        sFail, dpFail, dpPass);

    NOVA_CALL_RESULT(result, GetStencilOp, renderState, face, sFail, dpFail, dpPass);

    logLine("%s: %s: <- sFail %u (%s), dpFail %u (%s), dpPass %u (%s). Result %d (%s)",
        context->name, __func__,
        *sFail, decodeStencilOp(*sFail),
        *dpFail, decodeStencilOp(*dpFail),
        *dpPass, decodeStencilOp(*dpPass),
        result, mapNovaError(result));

    checkSuccess(context, GetStencilOp, result);

    return result;
}

static uint32 W3DN_GetStencilWriteMask(struct W3DN_Context_s *self, W3DN_RenderState *renderState,
    W3DN_FaceSelect face, W3DN_ErrorCode *errCode)
{
    uint32 mask = 0;

    GET_CONTEXT

    logLine("%s: %s: renderState %p, face %u (%s), errCode %p",
        context->name, __func__,
        renderState,
        face, decodeFaceSelect(face), errCode);

    NOVA_CALL_RESULT(mask, GetStencilWriteMask, renderState, face, errCode)

    logLine("%s: %s: <- errCode %d (%s). Stencil write mask 0x%lx",
        context->name, __func__,
        mapNovaErrorPointerToCode(errCode),
        mapNovaErrorPointerToString(errCode),
        mask);

    checkSuccess(context, GetStencilWriteMask, mapNovaErrorPointerToCode(errCode));

    return mask;
}

static W3DN_TextureSampler* W3DN_GetTexSampler(struct W3DN_Context_s *self, W3DN_RenderState *renderState, uint32 texUnit)
{
    W3DN_TextureSampler* sampler = NULL;

    GET_CONTEXT

    logLine("%s: %s: renderState %p, texUnit %lu",
        context->name, __func__,
        renderState,
        texUnit);

    NOVA_CALL_RESULT(sampler, GetTexSampler, renderState, texUnit)

    logLine("%s: %s: <- Texture sampler address %p",
        context->name, __func__,
        sampler);

    checkPointer(context, GetTexSampler, sampler);

    return sampler;
}

static W3DN_Texture* W3DN_GetTexture(struct W3DN_Context_s *self, W3DN_RenderState *renderState, uint32 texUnit)
{
    W3DN_Texture* texture = NULL;

    GET_CONTEXT

    logLine("%s: %s: renderState %p, texUnit %lu",
        context->name, __func__,
        renderState, texUnit);

    NOVA_CALL_RESULT(texture, GetTexture, renderState, texUnit)

    logLine("%s: %s: <- Texture address %p",
        context->name, __func__,
        texture);

    checkPointer(context, GetTexture, texture);

    return texture;
}

static W3DN_ErrorCode W3DN_GetVertexAttribArray(struct W3DN_Context_s *self, W3DN_RenderState *renderState,
    uint32 attribNum, W3DN_VertexBuffer **buffer, uint32 *arrayIdx)
{
    W3DN_ErrorCode result = W3DNEC_SUCCESS;

    GET_CONTEXT

    logLine("%s: %s: renderState %p, attribNum %lu, buffer %p, arrayIdx %p",
        context->name, __func__,
        renderState, attribNum, buffer, arrayIdx);

    NOVA_CALL_RESULT(result, GetVertexAttribArray, renderState, attribNum, buffer, arrayIdx);

    logLine("%s: %s: <- buffer %p, arrayIdx %lu. Result %d (%s)",
        context->name, __func__,
        *buffer,
        *arrayIdx,
        result, mapNovaError(result));

    checkSuccess(context, GetVertexAttribArray, result);

    return result;
}

static W3DN_ErrorCode W3DN_GetViewport(struct W3DN_Context_s *self, W3DN_RenderState *renderState,
    double *x, double *y, double *width, double *height, double *zNear, double *zFar)
{
    W3DN_ErrorCode result = W3DNEC_SUCCESS;

    GET_CONTEXT

    logLine("%s: %s: renderState %p, x %p, y %p, width %p, height %p, zNear %p, zFar %p",
        context->name, __func__,
        renderState, x, y, width, height, zNear, zFar);

    NOVA_CALL_RESULT(result, GetViewport, renderState, x, y, width, height, zNear, zFar);

    logLine("%s: %s: <- x %f, y %f, width %f, height %f, zNear %f, zFar %f. Result %d (%s)",
        context->name, __func__,
        *x,
        *y,
        *width,
        *height,
        *zNear,
        *zFar,
        result,
        mapNovaError(result));

    checkSuccess(context, GetViewport, result);

    return result;
}

static BOOL W3DN_IsDone(struct W3DN_Context_s *self, uint32 submitID)
{
    BOOL result = TRUE;

    GET_CONTEXT

    logLine("%s: %s: submitID %lu",
        context->name, __func__,
        submitID);

    NOVA_CALL_RESULT(result, IsDone, submitID)

    logLine("%s: %s: <- Result %d",
        context->name, __func__,
        result);

    return result;
}

static uint32 W3DN_Query(struct W3DN_Context_s *self, W3DN_CapQuery query)
{
    uint32 result = 0;

    GET_CONTEXT

    logLine("%s: %s: query %u (%s)",
        context->name, __func__,
        query, decodeCapQuery(query));

    NOVA_CALL_RESULT(result, Query, query)

    logLine("%s: %s: <- Result %lu",
        context->name, __func__,
        result);

    return result;
}

static W3DN_ErrorCode W3DN_RSOCopy(struct W3DN_Context_s *self, W3DN_RenderState *dest, const W3DN_RenderState *src)
{
    W3DN_ErrorCode result = W3DNEC_SUCCESS;

    GET_CONTEXT

    logLine("%s: %s: dest %p, src %p",
        context->name, __func__,
        dest, src);

    NOVA_CALL_RESULT(result, RSOCopy, dest, src)

    logLine("%s: %s: <- Result %d (%s)",
        context->name, __func__,
        result, mapNovaError(result));

    checkSuccess(context, RSOCopy, result);

    return result;
}

static W3DN_ErrorCode W3DN_RSOSetMaster(struct W3DN_Context_s *self, W3DN_RenderState *renderState, W3DN_RenderState *master)
{
    W3DN_ErrorCode result = W3DNEC_SUCCESS;

    GET_CONTEXT

    logLine("%s: %s: renderState %p, master %p",
        context->name, __func__,
        renderState, master);

    NOVA_CALL_RESULT(result, RSOSetMaster, renderState, master)

    logLine("%s: %s: <- Result %d (%s)",
        context->name, __func__,
        result, mapNovaError(result));

    checkSuccess(context, RSOSetMaster, result);

    return result;
}

static W3DN_ErrorCode W3DN_SetBlendColour(struct W3DN_Context_s *self, W3DN_RenderState *renderState, float red, float green, float blue, float alpha)
{
    W3DN_ErrorCode result = W3DNEC_SUCCESS;

    GET_CONTEXT

    logLine("%s: %s: renderState %p, red %f, green %f, blue %f, alpha %f",
        context->name, __func__,
        renderState, red, green, blue, alpha);

    NOVA_CALL_RESULT(result, SetBlendColour, renderState, red, green, blue, alpha)

    logLine("%s: %s: <- Result %d (%s)",
        context->name, __func__,
        result, mapNovaError(result));

    checkSuccess(context, SetBlendColour, result);

    return result;
}

static W3DN_ErrorCode W3DN_SetBlendEquation(struct W3DN_Context_s *self, W3DN_RenderState *renderState, uint32 buffIdx, W3DN_BlendEquation equation)
{
    W3DN_ErrorCode result = W3DNEC_SUCCESS;

    GET_CONTEXT

    logLine("%s: %s: renderState %p, buffIdx %lu, equation %u (%s)",
        context->name, __func__,
        renderState, buffIdx,
        equation, decodeBlendEquation(equation));

    NOVA_CALL_RESULT(result, SetBlendEquation, renderState, buffIdx, equation)

    logLine("%s: %s: <- Result %d (%s)",
        context->name, __func__,
        result, mapNovaError(result));

    checkSuccess(context, SetBlendEquation, result);

    return result;
}

static W3DN_ErrorCode W3DN_SetBlendEquationSeparate(struct W3DN_Context_s *self, W3DN_RenderState *renderState, uint32 buffIdx,
    W3DN_BlendEquation colEquation, W3DN_BlendEquation alphaEquation)
{
    W3DN_ErrorCode result = W3DNEC_SUCCESS;

    GET_CONTEXT

    logLine("%s: %s: renderState %p, buffIdx %lu, colEquation %u (%s), alphaEquation %u (%s)",
        context->name, __func__,
        renderState, buffIdx,
        colEquation, decodeBlendEquation(colEquation),
        alphaEquation, decodeBlendEquation(alphaEquation));

    NOVA_CALL_RESULT(result, SetBlendEquationSeparate, renderState, buffIdx, colEquation, alphaEquation)

    logLine("%s: %s: <- Result %d (%s)",
        context->name, __func__,
        result, mapNovaError(result));

    checkSuccess(context, SetBlendEquationSeparate, result);

    return result;
}

static W3DN_ErrorCode W3DN_SetBlendMode(struct W3DN_Context_s *self, W3DN_RenderState *renderState, uint32 buffIdx, W3DN_BlendMode src, W3DN_BlendMode dst)
{
    W3DN_ErrorCode result = W3DNEC_SUCCESS;

    GET_CONTEXT

    logLine("%s: %s: renderState %p, buffIdx %lu, src %u (%s), dst %u (%s)",
        context->name, __func__,
        renderState, buffIdx,
        src, decodeBlendMode(src),
        dst, decodeBlendMode(dst));

    NOVA_CALL_RESULT(result, SetBlendMode, renderState, buffIdx, src, dst)

    logLine("%s: %s: <- Result %d (%s)",
        context->name, __func__,
        result, mapNovaError(result));

    checkSuccess(context, SetBlendMode, result);

    return result;
}

static W3DN_ErrorCode W3DN_SetBlendModeSeparate(struct W3DN_Context_s *self, W3DN_RenderState *renderState, uint32 buffIdx,
    W3DN_BlendMode colSrc, W3DN_BlendMode colDst, W3DN_BlendMode alphaSrc, W3DN_BlendMode alphaDst)
{
    W3DN_ErrorCode result = W3DNEC_SUCCESS;

    GET_CONTEXT

    logLine("%s: %s: renderState %p, buffIdx %lu, colSrc %u (%s), colDst %u (%s), alphaSrc %u (%s), alphaDst %u (%s)",
        context->name, __func__,
        renderState, buffIdx,
        colSrc, decodeBlendMode(colSrc),
        colDst, decodeBlendMode(colDst),
        alphaSrc, decodeBlendMode(alphaSrc),
        alphaDst, decodeBlendMode(alphaDst));

    NOVA_CALL_RESULT(result, SetBlendModeSeparate, renderState, buffIdx, colSrc, colDst, alphaSrc, alphaDst)

    logLine("%s: %s: <- Result %d (%s)",
        context->name, __func__,
        result, mapNovaError(result));

    checkSuccess(context, SetBlendModeSeparate, result);

    return result;
}

static W3DN_ErrorCode W3DN_SetColourMask(struct W3DN_Context_s *self, W3DN_RenderState *renderState, uint32 index, uint8 mask)
{
    W3DN_ErrorCode result = W3DNEC_SUCCESS;

    GET_CONTEXT

    logLine("%s: %s: renderState %p, index %lu, mask 0x%x",
        context->name, __func__,
        renderState, index, mask);

    NOVA_CALL_RESULT(result, SetColourMask, renderState, index, mask)

    logLine("%s: %s: <- Result %d (%s)",
        context->name, __func__,
        result, mapNovaError(result));

    checkSuccess(context, SetColourMask, result);

    return result;
}

static W3DN_ErrorCode W3DN_SetDepthCompareFunc(struct W3DN_Context_s *self, W3DN_RenderState *renderState, W3DN_CompareFunc func)
{
    W3DN_ErrorCode result = W3DNEC_SUCCESS;

    GET_CONTEXT

    logLine("%s: %s: renderState %p, func %u (%s)",
        context->name, __func__,
        renderState,
        func, decodeCompareFunc(func));

    NOVA_CALL_RESULT(result, SetDepthCompareFunc, renderState, func)

    logLine("%s: %s: <- Result %d (%s)",
        context->name, __func__,
        result, mapNovaError(result));

    checkSuccess(context, SetDepthCompareFunc, result);

    return result;
}

static W3DN_ErrorCode W3DN_SetFrontFace(struct W3DN_Context_s *self, W3DN_RenderState *renderState, W3DN_Face face)
{
    W3DN_ErrorCode result = W3DNEC_SUCCESS;

    GET_CONTEXT

    logLine("%s: %s: renderState %p, face %u (%s)",
        context->name, __func__,
        renderState,
        face, decodeFace(face));

    NOVA_CALL_RESULT(result, SetFrontFace, renderState, face)

    logLine("%s: %s: <- Result %d (%s)",
        context->name, __func__,
        result, mapNovaError(result));

    checkSuccess(context, SetFrontFace, result);

    return result;
}

static W3DN_ErrorCode W3DN_SetLineWidth(struct W3DN_Context_s *self, W3DN_RenderState *renderState, float width)
{
    W3DN_ErrorCode result = W3DNEC_SUCCESS;

    GET_CONTEXT

    logLine("%s: %s: renderState %p, width %f",
        context->name, __func__,
        renderState, width);

    NOVA_CALL_RESULT(result, SetLineWidth, renderState, width)

    logLine("%s: %s: <- Result %d (%s)",
        context->name, __func__,
        result, mapNovaError(result));

    checkSuccess(context, SetLineWidth, result);

    return result;
}

static W3DN_ErrorCode W3DN_SetPolygonMode(struct W3DN_Context_s *self, W3DN_RenderState *renderState, W3DN_FaceSelect face, W3DN_PolygonMode mode)
{
    W3DN_ErrorCode result = W3DNEC_SUCCESS;

    GET_CONTEXT

    logLine("%s: %s: renderState %p, face %u (%s), mode %u (%s)",
        context->name, __func__,
        renderState,
        face, decodeFaceSelect(face),
        mode, decodePolygonMode(mode));

    NOVA_CALL_RESULT(result, SetPolygonMode, renderState, face, mode)

    logLine("%s: %s: <- Result %d (%s)",
        context->name, __func__,
        result, mapNovaError(result));

    checkSuccess(context, SetPolygonMode, result);

    return result;
}

static W3DN_ErrorCode W3DN_SetPolygonOffset(struct W3DN_Context_s *self, W3DN_RenderState *renderState, float factor, float units, float clamp)
{
    W3DN_ErrorCode result = W3DNEC_SUCCESS;

    GET_CONTEXT

    logLine("%s: %s: renderState %p, factor %f, units %f, clamp %f",
        context->name, __func__,
        renderState, factor, units, clamp);

    NOVA_CALL_RESULT(result, SetPolygonOffset, renderState, factor, units, clamp)

    logLine("%s: %s: <- Result %d (%s)",
        context->name, __func__,
        result, mapNovaError(result));

    checkSuccess(context, SetPolygonOffset, result);

    return result;
}

static W3DN_ErrorCode W3DN_SetProvokingVertex(struct W3DN_Context_s *self, W3DN_RenderState *renderState, W3DN_ProvokingVertexMode mode)
{
    W3DN_ErrorCode result = W3DNEC_SUCCESS;

    GET_CONTEXT

    logLine("%s: %s: renderState %p, mode %u (%s)",
        context->name, __func__,
        renderState,
        mode, decodeProvokingVertexMode(mode));

    NOVA_CALL_RESULT(result, SetProvokingVertex, renderState, mode)

    logLine("%s: %s: <- Result %d (%s)",
        context->name, __func__,
        result, mapNovaError(result));

    checkSuccess(context, SetProvokingVertex, result);

    return result;
}

static W3DN_ErrorCode W3DN_SetRenderTarget(struct W3DN_Context_s *self,
	W3DN_RenderState *renderState, W3DN_FrameBuffer *frameBuffer)
{
    W3DN_ErrorCode result = W3DNEC_SUCCESS;

    GET_CONTEXT

    logLine("%s: %s: renderState %p, frameBuffer %p",
        context->name, __func__,
        renderState, frameBuffer);

    NOVA_CALL_RESULT(result, SetRenderTarget, renderState, frameBuffer)

    logLine("%s: %s: <- Result %d (%s)",
        context->name, __func__,
        result, mapNovaError(result));

    checkSuccess(context, SetRenderTarget, result);

    return result;
}

static W3DN_ErrorCode W3DN_SetScissor(struct W3DN_Context_s *self, W3DN_RenderState *renderState,
    uint32 x, uint32 y, uint32 width, uint32 height)
{
    W3DN_ErrorCode result = W3DNEC_SUCCESS;

    GET_CONTEXT

    logLine("%s: %s: renderState %p, x %lu, y %lu, width %lu, height %lu",
        context->name, __func__,
        renderState, x, y, width, height);

    NOVA_CALL_RESULT(result, SetScissor, renderState, x, y, width, height)

    logLine("%s: %s: <- Result %d (%s)",
        context->name, __func__,
        result, mapNovaError(result));

    checkSuccess(context, SetScissor, result);

    return result;
}

static W3DN_ErrorCode W3DN_SetShaderPipeline(struct W3DN_Context_s *self, W3DN_RenderState *renderState,
    W3DN_ShaderPipeline *shaderPipeline)
{
    W3DN_ErrorCode result = W3DNEC_SUCCESS;

    GET_CONTEXT

    logLine("%s: %s: renderState %p, shaderPipeline %p",
        context->name, __func__,
        renderState, shaderPipeline);

    NOVA_CALL_RESULT(result, SetShaderPipeline, renderState, shaderPipeline)

    logLine("%s: %s: <- Result %d (%s)",
        context->name, __func__,
        result, mapNovaError(result));

    checkSuccess(context, SetShaderPipeline, result);

    return result;
}

static W3DN_ErrorCode W3DN_SetState(struct W3DN_Context_s *self, W3DN_RenderState *renderState,
    W3DN_StateFlag stateFlag, W3DN_State value)
{
    W3DN_ErrorCode result = W3DNEC_SUCCESS;

    GET_CONTEXT

    logLine("%s: %s: renderState %p, stateFlag %u (%s), value %u (%s)",
        context->name, __func__,
        renderState,
        stateFlag, decodeStateFlag(stateFlag),
        value, decodeState(value));

    NOVA_CALL_RESULT(result, SetState, renderState, stateFlag, value)

    logLine("%s: %s: <- Result %d (%s)",
        context->name, __func__,
        result, mapNovaError(result));

    checkSuccess(context, SetState, result);

    return result;
}

static W3DN_ErrorCode W3DN_SetStencilFunc(struct W3DN_Context_s *self, W3DN_RenderState *renderState,
    W3DN_CompareFunc func, uint32 ref, uint32 mask)
{
    W3DN_ErrorCode result = W3DNEC_SUCCESS;

    GET_CONTEXT

    logLine("%s: %s: renderState %p, func %u (%s), ref %lu, mask 0x%lx",
        context->name, __func__,
        renderState,
        func, decodeCompareFunc(func),
        ref, mask);

    NOVA_CALL_RESULT(result, SetStencilFunc, renderState, func, ref, mask)

    logLine("%s: %s: <- Result %d (%s)",
        context->name, __func__,
        result, mapNovaError(result));

    checkSuccess(context, SetStencilFunc, result);

    return result;
}

static W3DN_ErrorCode W3DN_SetStencilFuncSeparate(struct W3DN_Context_s *self, W3DN_RenderState *renderState,
    W3DN_FaceSelect face, W3DN_CompareFunc func, uint32 ref, uint32 mask)
{
    W3DN_ErrorCode result = W3DNEC_SUCCESS;

    GET_CONTEXT

    logLine("%s: %s: renderState %p, face %u (%s), func %u (%s), ref %lu, mask 0x%lx",
        context->name, __func__,
        renderState,
        face, decodeFaceSelect(face),
        func, decodeCompareFunc(func),
        ref, mask);

    NOVA_CALL_RESULT(result, SetStencilFuncSeparate, renderState, face, func, ref, mask)

    logLine("%s: %s: <- Result %d (%s)",
        context->name, __func__,
        result, mapNovaError(result));

    checkSuccess(context, SetStencilFuncSeparate, result);

    return result;
}

static W3DN_ErrorCode W3DN_SetStencilOp(struct W3DN_Context_s *self, W3DN_RenderState *renderState,
    W3DN_StencilOp sFail, W3DN_StencilOp dpFail, W3DN_StencilOp dpPass)
{
    W3DN_ErrorCode result = W3DNEC_SUCCESS;

    GET_CONTEXT

    logLine("%s: %s: renderState %p, sFail %u (%s), dpFail %u (%s), dpPass %u (%s)",
        context->name, __func__,
        renderState,
        sFail, decodeStencilOp(sFail),
        dpFail, decodeStencilOp(dpFail),
        dpPass, decodeStencilOp(dpPass));

    NOVA_CALL_RESULT(result, SetStencilOp, renderState, sFail, dpFail, dpPass)

    logLine("%s: %s: <- Result %d (%s)",
        context->name, __func__,
        result, mapNovaError(result));

    checkSuccess(context, SetStencilOp, result);

    return result;
}

static W3DN_ErrorCode W3DN_SetStencilOpSeparate(struct W3DN_Context_s *self, W3DN_RenderState *renderState,
    W3DN_FaceSelect face, W3DN_StencilOp sFail, W3DN_StencilOp dpFail, W3DN_StencilOp dpPass)
{
    W3DN_ErrorCode result = W3DNEC_SUCCESS;

    GET_CONTEXT

    logLine("%s: %s: renderState %p, face %u (%s), sFail %u (%s), dpFail %u (%s), dpPass %u (%s)",
        context->name, __func__,
        renderState,
        face, decodeFaceSelect(face),
        sFail, decodeStencilOp(sFail),
        dpFail, decodeStencilOp(dpFail),
        dpPass, decodeStencilOp(dpPass));

    NOVA_CALL_RESULT(result, SetStencilOpSeparate, renderState, face, sFail, dpFail, dpPass)

    logLine("%s: %s: <- Result %d (%s)",
        context->name, __func__,
        result, mapNovaError(result));

    checkSuccess(context, SetStencilOpSeparate, result);

    return result;
}

static W3DN_ErrorCode W3DN_SetStencilWriteMask(struct W3DN_Context_s *self, W3DN_RenderState *renderState, uint32 mask)
{
    W3DN_ErrorCode result = W3DNEC_SUCCESS;

    GET_CONTEXT

    logLine("%s: %s: renderState %p, mask 0x%lx",
        context->name, __func__,
        renderState, mask);

    NOVA_CALL_RESULT(result, SetStencilWriteMask, renderState, mask)

    logLine("%s: %s: <- Result %d (%s)",
        context->name, __func__,
        result, mapNovaError(result));

    checkSuccess(context, SetStencilWriteMask, result);

    return result;
}

static W3DN_ErrorCode W3DN_SetStencilWriteMaskSeparate(struct W3DN_Context_s *self, W3DN_RenderState *renderState,
     W3DN_FaceSelect face, uint32 mask)
{
    W3DN_ErrorCode result = W3DNEC_SUCCESS;

    GET_CONTEXT

    logLine("%s: %s: renderState %p, face %u (%s), mask 0x%lx",
        context->name, __func__,
        renderState,
        face, decodeFaceSelect(face),
        mask);

    NOVA_CALL_RESULT(result, SetStencilWriteMaskSeparate, renderState, face, mask)

    logLine("%s: %s: <- Result %d (%s)",
        context->name, __func__,
        result, mapNovaError(result));

    checkSuccess(context, SetStencilWriteMaskSeparate, result);

    return result;
}

static W3DN_ErrorCode W3DN_SetViewport(struct W3DN_Context_s *self, W3DN_RenderState *renderState,
    double x, double y, double width, double height, double zNear, double zFar)
{
    W3DN_ErrorCode result = W3DNEC_SUCCESS;

    GET_CONTEXT

    logLine("%s: %s: renderState %p, x %f, y %f, width %f, height %f, zNear %f, zFar %f",
        context->name, __func__,
        renderState, x, y, width, height, zNear, zFar);

    NOVA_CALL_RESULT(result, SetViewport, renderState, x, y, width, height, zNear, zFar);

    logLine("%s: %s: <- Result %d (%s)",
        context->name, __func__,
        result, mapNovaError(result));

    checkSuccess(context, SetViewport, result);

    return result;
}

static uint32 W3DN_ShaderGetCount(struct W3DN_Context_s *self, W3DN_ErrorCode *errCode, W3DN_Shader *shader,
    W3DN_ShaderObjectType objectType)
{
    uint32 count = 0;

    GET_CONTEXT

    logLine("%s: %s: errCode %p, shader %p, objectType %u (%s)",
        context->name, __func__,
        errCode, shader,
        objectType, decodeShaderObjectType(objectType));

    NOVA_CALL_RESULT(count, ShaderGetCount, errCode, shader, objectType)

    logLine("%s: %s: <- errCode %d (%s). Shader count %lu",
        context->name, __func__,
        mapNovaErrorPointerToCode(errCode),
        mapNovaErrorPointerToString(errCode),
        count);

    checkSuccess(context, ShaderGetCount, mapNovaErrorPointerToCode(errCode));

    return count;
}

static W3DN_ErrorCode W3DN_ShaderGetObjectInfo(struct W3DN_Context_s *self, W3DN_Shader *shader,
    W3DN_ShaderObjectType objectType, uint32 index, struct TagItem *tags)
{
    W3DN_ErrorCode result = W3DNEC_SUCCESS;

    GET_CONTEXT

    logLine("%s: %s: shader %p, objectType %u (%s), index %lu, tags %p",
        context->name, __func__,
        shader,
        objectType, decodeShaderObjectType(objectType),
        index,
        tags);

    NOVA_CALL_RESULT(result, ShaderGetObjectInfo, shader, objectType, index, tags)

    logLine("%s: %s: <- tags (%s). Result %d (%s)",
        context->name, __func__,
        decodeShaderGetObjectInfoTags(tags, context),
        result, mapNovaError(result));

    checkSuccess(context, ShaderGetObjectInfo, result);

    return result;
}

static uint32 W3DN_ShaderGetOffset(struct W3DN_Context_s *self, W3DN_ErrorCode *errCode, W3DN_Shader *shader,
    W3DN_ShaderObjectType objectType, const char *name)
{
    uint32 offset = 0;

    GET_CONTEXT

    logLine("%s: %s: errCode %p, shader %p, objectType %u (%s), name '%s'",
        context->name, __func__,
        errCode, shader,
        objectType, decodeShaderObjectType(objectType),
        name);

    NOVA_CALL_RESULT(offset, ShaderGetOffset, errCode, shader, objectType, name)

    logLine("%s: %s: <- errCode %d (%s). Offset %lu",
        context->name, __func__,
        mapNovaErrorPointerToCode(errCode),
        mapNovaErrorPointerToString(errCode),
        offset);

    checkSuccess(context, ShaderGetOffset, mapNovaErrorPointerToCode(errCode));

    // TODO: check offset and flag error?

    return offset;
}

static uint64 W3DN_ShaderGetTotalStorage(struct W3DN_Context_s *self, W3DN_Shader *shader)
{
    uint64 size = 0;

    GET_CONTEXT

    logLine("%s: %s: shader %p",
        context->name, __func__,
        shader);

    NOVA_CALL_RESULT(size, ShaderGetTotalStorage, shader)

    logLine("%s: %s: <- Size %llu",
        context->name, __func__,
        size);

    return size;
}

static W3DN_ShaderType W3DN_ShaderGetType(struct W3DN_Context_s *self, W3DN_Shader *shader)
{
    W3DN_ShaderType type = 0;

    GET_CONTEXT

    logLine("%s: %s: shader %p",
        context->name, __func__,
        shader);

    NOVA_CALL_RESULT(type, ShaderGetType, shader)

    logLine("%s: %s: <- Type %u (%s)",
        context->name, __func__,
        type, decodeShaderType(type));

    return type;
}

static W3DN_Shader* W3DN_ShaderPipelineGetShader(struct W3DN_Context_s *self, W3DN_ShaderPipeline *shaderPipeline,
    W3DN_ShaderType shaderType)
{
    W3DN_Shader* shader = NULL;

    GET_CONTEXT

    logLine("%s: %s: shaderPipeline %p, shaderType %u (%s)",
        context->name, __func__,
        shaderPipeline,
        shaderType, decodeShaderType(shaderType));

    NOVA_CALL_RESULT(shader, ShaderPipelineGetShader, shaderPipeline, shaderType)

    logLine("%s: %s: <- Shader %p",
        context->name, __func__,
        shader);

    checkPointer(context, ShaderPipelineGetShader, shader);

    return shader;
}

static uint32 W3DN_Submit(struct W3DN_Context_s *self, W3DN_ErrorCode *errCode)
{
    W3DN_ErrorCode myErrCode = 0;
    uint32 result = 0;

    GET_CONTEXT

    logLine("%s: %s: errCode %p",
        context->name, __func__,
        errCode);

    NOVA_CALL_RESULT(result, Submit, &myErrCode)

    logLine("%s: %s: <- errCode %d (%s). Submit ID %lu",
        context->name, __func__,
        myErrCode, mapNovaError(myErrCode),
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

static W3DN_ErrorCode W3DN_TexGenMipMaps(struct W3DN_Context_s *self, W3DN_Texture *texture, uint32 base, uint32 last)
{
    W3DN_ErrorCode result = W3DNEC_SUCCESS;

    GET_CONTEXT

    logLine("%s: %s: texture %p, base %lu, last %lu",
        context->name, __func__,
        texture, base, last);

    NOVA_CALL_RESULT(result, TexGenMipMaps, texture, base, last)

    logLine("%s: %s: <- Result %d (%s)",
        context->name, __func__,
        result, mapNovaError(result));

    checkSuccess(context, TexGenMipMaps, result);

    return result;
}

static W3DN_ErrorCode W3DN_TexGetParameters(struct W3DN_Context_s *self, W3DN_Texture *texture, struct TagItem *tags)
{
    W3DN_ErrorCode result = W3DNEC_SUCCESS;

    GET_CONTEXT

    logLine("%s: %s: texture %p, tags %p",
        context->name, __func__,
        texture, tags);

    NOVA_CALL_RESULT(result, TexGetParameters, texture, tags)

    logLine("%s: %s: <- tags (%s). Result %d (%s)",
        context->name, __func__,
        decodeTextureParameterTags(tags, context),
        result, mapNovaError(result));

    checkSuccess(context, TexGetParameters, result);

    return result;
}

static W3DN_ErrorCode W3DN_TexGetProperty(struct W3DN_Context_s *self, W3DN_Texture *texture, W3DN_TextureProperty texProp, void *buffer)
{
    W3DN_ErrorCode result = W3DNEC_SUCCESS;

    GET_CONTEXT

    logLine("%s: %s: texture %p, texProp %u (%s), buffer %p",
        context->name, __func__,
        texture,
        texProp, decodeTextureProperty(texProp),
        buffer);

    NOVA_CALL_RESULT(result, TexGetProperty, texture, texProp, buffer)

    logLine("%s: %s: <- Result %d (%s)",
        context->name, __func__,
        result, mapNovaError(result));

    // TODO: could log *buffer

    checkSuccess(context, TexGetProperty, result);

    return result;
}

static void* W3DN_TexGetRMBuffer(struct W3DN_Context_s *self, W3DN_Texture *texture)
{
    void* result = NULL;

    GET_CONTEXT

    logLine("%s: %s: texture %p",
        context->name, __func__,
        texture);

    NOVA_CALL_RESULT(result, TexGetRMBuffer, texture)

    logLine("%s: %s: <- Result %p",
        context->name, __func__,
        result);

    checkPointer(context, TexGetRMBuffer, result);

    return result;
}

static W3DN_ErrorCode W3DN_TexGetSubResourceLayout(struct W3DN_Context_s *self, W3DN_Texture *texture, W3DN_TexAspect aspect,
    uint32 mipLevel, uint32 arrayIdx, W3DN_ResourceLayout *layout)
{
    W3DN_ErrorCode result = W3DNEC_SUCCESS;

    GET_CONTEXT

    logLine("%s: %s: texture %p, aspect %d (%s), mipLevel %lu, arrayIdx %lu, layout %p",
        context->name, __func__,
        texture,
        aspect, decodeTexAspect(aspect),
        mipLevel, arrayIdx, layout);

    NOVA_CALL_RESULT(result, TexGetSubResourceLayout, texture, aspect, mipLevel, arrayIdx, layout)

    logLine("%s: %s: <- Result %d (%s)",
        context->name, __func__,
        result, mapNovaError(result));

    // TODO: could log *layout

    checkSuccess(context, TexGetSubResourceLayout, result);

    return result;
}

static W3DN_ErrorCode W3DN_TexSetParameters(struct W3DN_Context_s *self, W3DN_Texture *texture, struct TagItem *tags)
{
    W3DN_ErrorCode result = W3DNEC_SUCCESS;

    GET_CONTEXT

    logLine("%s: %s: texture %p, tags %p (%s)",
        context->name, __func__,
        texture,
        tags, decodeTextureParameterTags(tags, context));

    NOVA_CALL_RESULT(result, TexSetParameters, texture, tags)

    logLine("%s: %s: <- Result %d (%s)",
        context->name, __func__,
        result, mapNovaError(result));

    checkSuccess(context, TexSetParameters, result);

    return result;
}

static W3DN_ErrorCode W3DN_TexUpdateImage(struct W3DN_Context_s *self, W3DN_Texture *texture, void *source,
    uint32 level, uint32 arrayIdx, uint32 srcBytesPerRow, uint32 srcRowsPerLayer)
{
    W3DN_ErrorCode result = W3DNEC_SUCCESS;

    GET_CONTEXT

    logLine("%s: %s: texture %p, source %p, level %lu, arrayIdx %lu, srcBytesPerRow %lu, srcRowsPerLayer %lu",
        context->name, __func__,
        texture, source, level, arrayIdx, srcBytesPerRow, srcRowsPerLayer);

    NOVA_CALL_RESULT(result, TexUpdateImage, texture, source, level, arrayIdx, srcBytesPerRow, srcRowsPerLayer)

    logLine("%s: %s: <- Result %d (%s)",
        context->name, __func__,
        result, mapNovaError(result));

    checkSuccess(context, TexUpdateImage, result);

    return result;
}

static W3DN_ErrorCode W3DN_TexUpdateSubImage(struct W3DN_Context_s *self, W3DN_Texture *texture, void *source,
    uint32 level, uint32 arrayIdx, uint32 srcBytesPerRow, uint32 srcRowsPerLayer,
    uint32 dstX, uint32 dstY, uint32 dstLayer, uint32 width, uint32 height, uint32 depth)
{
    W3DN_ErrorCode result = W3DNEC_SUCCESS;

    GET_CONTEXT

    logLine("%s: %s: texture %p, source %p, level %lu, arrayIdx %lu, srcBytesPerRow %lu, srcRowsPerLayer %lu, "
        "dstX %lu, dstY %lu, dstLayer %lu, width %lu, height %lu, depth %lu",
        context->name, __func__,
        texture, source,
        level, arrayIdx,
        srcBytesPerRow, srcRowsPerLayer,
        dstX, dstY, dstLayer,
        width, height, depth);

    NOVA_CALL_RESULT(result, TexUpdateSubImage, texture, source,
        level, arrayIdx, srcBytesPerRow, srcRowsPerLayer, dstX, dstY, dstLayer, width, height, depth)

    logLine("%s: %s: <- Result %d (%s)",
        context->name, __func__,
        result, mapNovaError(result));

    checkSuccess(context, TexUpdateSubImage, result);

    return result;
}

static W3DN_ErrorCode W3DN_TSGetParameters(struct W3DN_Context_s *self, W3DN_TextureSampler *texSampler, struct TagItem *tags)
{
    W3DN_ErrorCode result = W3DNEC_SUCCESS;

    GET_CONTEXT

    logLine("%s: %s: texSampler %p, tags %p",
        context->name, __func__,
        texSampler, tags);

    NOVA_CALL_RESULT(result, TSGetParameters, texSampler, tags)

    logLine("%s: %s: <- tags (%s). Result %d (%s)",
        context->name, __func__,
        decodeTextureSamplerParameterTags(tags, context),
        result, mapNovaError(result));

    checkSuccess(context, TSGetParameters, result);

    return result;
}

static W3DN_ErrorCode W3DN_TSSetParameters(struct W3DN_Context_s *self, W3DN_TextureSampler *texSampler, struct TagItem *tags)
{
    W3DN_ErrorCode result = W3DNEC_SUCCESS;

    GET_CONTEXT

    logLine("%s: %s: texSampler %p, tags %p (%s)",
        context->name, __func__,
        texSampler,
        tags, decodeTextureSamplerParameterTags(tags, context));

    NOVA_CALL_RESULT(result, TSSetParameters, texSampler, tags)

    logLine("%s: %s: <- Result %d (%s)",
        context->name, __func__,
        result, mapNovaError(result));

    checkSuccess(context, TSSetParameters, result);

    return result;
}

static W3DN_ErrorCode W3DN_VBOGetArray(struct W3DN_Context_s *self, W3DN_VertexBuffer *buffer,
		uint32 arrayIdx, W3DN_ElementFormat *elementType, BOOL *normalized,
		uint64 *numElements, uint64 *stride, uint64 *offset, uint64 *count)
{
    W3DN_ErrorCode result = W3DNEC_SUCCESS;

    GET_CONTEXT

    logLine("%s: %s: buffer %p, arrayIdx %lu, elementType %p, normalized %p, numElements %p, stride %p, offset %p, count %p",
        context->name, __func__,
        buffer, arrayIdx, elementType,
        normalized, numElements, stride, offset, count);

    NOVA_CALL_RESULT(result, VBOGetArray, buffer, arrayIdx, elementType, normalized, numElements, stride, offset, count)

    logLine("%s: %s: <- elementType %u (%s), normalized %d, numElements %llu, stride %llu, offset %llu, count %llu. Result %d (%s)",
        context->name, __func__,
        *elementType, decodeElementFormat(*elementType),
        *normalized, *numElements, *stride, *offset, *count,
        result, mapNovaError(result));

    checkSuccess(context, VBOGetArray, result);

    return result;
}

static uint64 W3DN_VBOGetAttr(struct W3DN_Context_s *self, W3DN_VertexBuffer *vertexBuffer, W3DN_BufferAttribute attr)
{
    uint64 result = 0;

    GET_CONTEXT

    logLine("%s: %s: vertexBuffer %p, attr %u (%s)", context->name, __func__,
        vertexBuffer,
        attr, decodeBufferAttribute(attr));

    NOVA_CALL_RESULT(result, VBOGetAttr, vertexBuffer, attr)

    logLine("%s: %s: <- Result %llu", context->name, __func__,
        result);

    return result;
}

static W3DN_BufferLock* W3DN_VBOLock(struct W3DN_Context_s *self, W3DN_ErrorCode *errCode,
		W3DN_VertexBuffer *buffer, uint64 readOffset, uint64 readSize)
{
    W3DN_BufferLock* result = NULL;

    GET_CONTEXT

    logLine("%s: %s: buffer %p, readOffset %llu, readSize %llu. Lock address %p, errCode %p", context->name, __func__,
        buffer, readOffset, readSize, result, errCode);

    NOVA_CALL_RESULT(result, VBOLock, errCode, buffer, readOffset, readSize)

    logLine("%s: %s: <- errCode %u (%s). Lock address %p", context->name, __func__,
        mapNovaErrorPointerToCode(errCode),
        mapNovaErrorPointerToString(errCode),
        result);

    checkPointer(context, VBOLock, result);
    checkSuccess(context, VBOLock, mapNovaErrorPointerToCode(errCode));

    return result;
}

static W3DN_ErrorCode W3DN_VBOSetArray(struct W3DN_Context_s *self, W3DN_VertexBuffer *buffer,
		uint32 arrayIdx, W3DN_ElementFormat elementType, BOOL normalized, uint64 numElements,
		uint64 stride, uint64 offset, uint64 count)
{
    W3DN_ErrorCode result = W3DNEC_SUCCESS;

    GET_CONTEXT

    logLine("%s: %s: buffer %p, arrayIdx %lu, elementType %u (%s), normalized %d, numElements %llu, stride %llu, offset %llu, count %llu",
        context->name, __func__,
        buffer, arrayIdx,
        elementType, decodeElementFormat(elementType),
        normalized, numElements, stride, offset, count);

    NOVA_CALL_RESULT(result, VBOSetArray, buffer, arrayIdx, elementType, normalized, numElements, stride, offset, count)

    logLine("%s: %s: <- Result %d (%s)",
        context->name, __func__,
        result, mapNovaError(result));

    checkSuccess(context, VBOSetArray, result);

    return result;
}

static W3DN_ErrorCode W3DN_WaitDone(struct W3DN_Context_s *self, uint32 submitID, uint32 timeout)
{
    W3DN_ErrorCode result = W3DNEC_SUCCESS;

    GET_CONTEXT

    logLine("%s: %s: submitID %lu, timeout %lu",
        context->name, __func__,
        submitID, timeout);

    NOVA_CALL_RESULT(result, WaitDone, submitID, timeout)

    logLine("%s: %s: <- Result %d (%s)",
        context->name, __func__,
        result, mapNovaError(result));

    checkSuccess(context, WaitDone, result);

    return result;
}

static W3DN_ErrorCode W3DN_WaitIdle(struct W3DN_Context_s *self, uint32 timeout)
{
    W3DN_ErrorCode result = W3DNEC_SUCCESS;

    GET_CONTEXT

    logLine("%s: %s: timeout %lu",
        context->name, __func__,
        timeout);

    NOVA_CALL_RESULT(result, WaitIdle, timeout)

    logLine("%s: %s: <- Result %d (%s)",
        context->name, __func__,
        result, mapNovaError(result));

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
            logDebug("Patched W3DN context function " #function); \
        } \
    } else { \
        IExec->Forbid(); \
        if (nova->old_##function) { \
            nova->context->function = nova->old_##function; \
            nova->old_##function = NULL; \
        } \
        IExec->Permit(); \
        logDebug("Restored W3DN context function " #function); \
    } \
}

GENERATE_NOVA_PATCH(BindBitMapAsTexture)
GENERATE_NOVA_PATCH(BindShaderDataBuffer)
GENERATE_NOVA_PATCH(BindTexture)
GENERATE_NOVA_PATCH(BindVertexAttribArray)
GENERATE_NOVA_PATCH(BufferUnlock)
GENERATE_NOVA_PATCH(Clear)
GENERATE_NOVA_PATCH(CompileShader)
GENERATE_NOVA_PATCH(CreateDataBufferObject)
GENERATE_NOVA_PATCH(CreateFrameBuffer)
GENERATE_NOVA_PATCH(CreateRenderStateObject)
GENERATE_NOVA_PATCH(CreateShaderPipeline)
GENERATE_NOVA_PATCH(CreateTexSampler)
GENERATE_NOVA_PATCH(CreateTexture)
GENERATE_NOVA_PATCH(CreateTextureExtRMB)
GENERATE_NOVA_PATCH(CreateVertexBufferObject)
GENERATE_NOVA_PATCH(DBOGetAttr)
GENERATE_NOVA_PATCH(DBOGetBuffer)
GENERATE_NOVA_PATCH(DBOLock)
GENERATE_NOVA_PATCH(DBOSetBuffer)
GENERATE_NOVA_PATCH(Destroy)
GENERATE_NOVA_PATCH(DestroyDataBufferObject)
GENERATE_NOVA_PATCH(DestroyFrameBuffer)
GENERATE_NOVA_PATCH(DestroyRenderStateObject)
GENERATE_NOVA_PATCH(DestroyShader)
GENERATE_NOVA_PATCH(DestroyShaderLog)
GENERATE_NOVA_PATCH(DestroyShaderPipeline)
GENERATE_NOVA_PATCH(DestroyTexSampler)
GENERATE_NOVA_PATCH(DestroyTexture)
GENERATE_NOVA_PATCH(DestroyVertexBufferObject)
GENERATE_NOVA_PATCH(DrawArrays)
GENERATE_NOVA_PATCH(DrawElements)
GENERATE_NOVA_PATCH(FBBindBuffer)
GENERATE_NOVA_PATCH(FBGetAttr)
GENERATE_NOVA_PATCH(FBGetBufferBM)
GENERATE_NOVA_PATCH(FBGetBufferTex)
GENERATE_NOVA_PATCH(FBGetStatus)
GENERATE_NOVA_PATCH(GetBitMapTexture)
GENERATE_NOVA_PATCH(GetBlendColour)
GENERATE_NOVA_PATCH(GetBlendEquation)
GENERATE_NOVA_PATCH(GetBlendMode)
GENERATE_NOVA_PATCH(GetColourMask)
GENERATE_NOVA_PATCH(GetDepthCompareFunc)
GENERATE_NOVA_PATCH(GetFrontFace)
GENERATE_NOVA_PATCH(GetLineWidth)
GENERATE_NOVA_PATCH(GetPolygonMode)
GENERATE_NOVA_PATCH(GetPolygonOffset)
GENERATE_NOVA_PATCH(GetProvokingVertex)
GENERATE_NOVA_PATCH(GetRenderTarget)
GENERATE_NOVA_PATCH(GetScissor)
GENERATE_NOVA_PATCH(GetShaderDataBuffer)
GENERATE_NOVA_PATCH(GetShaderPipeline)
GENERATE_NOVA_PATCH(GetState)
GENERATE_NOVA_PATCH(GetStencilFunc)
GENERATE_NOVA_PATCH(GetStencilOp)
GENERATE_NOVA_PATCH(GetStencilWriteMask)
GENERATE_NOVA_PATCH(GetTexSampler)
GENERATE_NOVA_PATCH(GetTexture)
GENERATE_NOVA_PATCH(GetVertexAttribArray)
GENERATE_NOVA_PATCH(GetViewport)
GENERATE_NOVA_PATCH(IsDone)
GENERATE_NOVA_PATCH(Query)
GENERATE_NOVA_PATCH(RSOCopy)
GENERATE_NOVA_PATCH(RSOSetMaster)
GENERATE_NOVA_PATCH(SetBlendColour)
GENERATE_NOVA_PATCH(SetBlendEquation)
GENERATE_NOVA_PATCH(SetBlendEquationSeparate)
GENERATE_NOVA_PATCH(SetBlendMode)
GENERATE_NOVA_PATCH(SetBlendModeSeparate)
GENERATE_NOVA_PATCH(SetColourMask)
GENERATE_NOVA_PATCH(SetDepthCompareFunc)
GENERATE_NOVA_PATCH(SetFrontFace)
GENERATE_NOVA_PATCH(SetLineWidth)
GENERATE_NOVA_PATCH(SetPolygonMode)
GENERATE_NOVA_PATCH(SetPolygonOffset)
GENERATE_NOVA_PATCH(SetProvokingVertex)
GENERATE_NOVA_PATCH(SetRenderTarget)
GENERATE_NOVA_PATCH(SetScissor)
GENERATE_NOVA_PATCH(SetShaderPipeline)
GENERATE_NOVA_PATCH(SetState)
GENERATE_NOVA_PATCH(SetStencilFunc)
GENERATE_NOVA_PATCH(SetStencilFuncSeparate)
GENERATE_NOVA_PATCH(SetStencilOp)
GENERATE_NOVA_PATCH(SetStencilOpSeparate)
GENERATE_NOVA_PATCH(SetStencilWriteMask)
GENERATE_NOVA_PATCH(SetStencilWriteMaskSeparate)
GENERATE_NOVA_PATCH(SetViewport)
GENERATE_NOVA_PATCH(ShaderGetCount)
GENERATE_NOVA_PATCH(ShaderGetObjectInfo)
GENERATE_NOVA_PATCH(ShaderGetOffset)
GENERATE_NOVA_PATCH(ShaderGetTotalStorage)
GENERATE_NOVA_PATCH(ShaderGetType)
GENERATE_NOVA_PATCH(ShaderPipelineGetShader)
GENERATE_NOVA_PATCH(Submit)
GENERATE_NOVA_PATCH(TexGenMipMaps)
GENERATE_NOVA_PATCH(TexGetParameters)
GENERATE_NOVA_PATCH(TexGetProperty)
GENERATE_NOVA_PATCH(TexGetRMBuffer)
GENERATE_NOVA_PATCH(TexGetSubResourceLayout)
GENERATE_NOVA_PATCH(TexSetParameters)
GENERATE_NOVA_PATCH(TexUpdateImage)
GENERATE_NOVA_PATCH(TexUpdateSubImage)
GENERATE_NOVA_PATCH(TSGetParameters)
GENERATE_NOVA_PATCH(TSSetParameters)
GENERATE_NOVA_PATCH(VBOGetArray)
GENERATE_NOVA_PATCH(VBOGetAttr)
GENERATE_NOVA_PATCH(VBOLock)
GENERATE_NOVA_PATCH(VBOSetArray)
GENERATE_NOVA_PATCH(WaitDone)
GENERATE_NOVA_PATCH(WaitIdle)

static void (*patches[])(BOOL, struct NovaContext *) = {
    patch_BindBitMapAsTexture,
    patch_BindShaderDataBuffer,
    patch_BindTexture,
    patch_BindVertexAttribArray,
    patch_BufferUnlock,
    patch_Clear,
    patch_CompileShader,
    patch_CreateDataBufferObject,
    patch_CreateFrameBuffer,
    patch_CreateRenderStateObject,
    patch_CreateShaderPipeline,
    patch_CreateTexSampler,
    patch_CreateTexture,
    patch_CreateTextureExtRMB,
    patch_CreateVertexBufferObject,
    patch_DBOGetAttr,
    patch_DBOGetBuffer,
    patch_DBOLock,
    patch_DBOSetBuffer,
    patch_Destroy,
    patch_DestroyDataBufferObject,
    patch_DestroyFrameBuffer,
    patch_DestroyRenderStateObject,
    patch_DestroyShader,
    patch_DestroyShaderLog,
    patch_DestroyShaderPipeline,
    patch_DestroyTexSampler,
    patch_DestroyTexture,
    patch_DestroyVertexBufferObject,
    patch_DrawArrays,
    patch_DrawElements,
    patch_FBBindBuffer,
    patch_FBGetAttr,
    patch_FBGetBufferBM,
    patch_FBGetBufferTex,
    patch_FBGetStatus,
    patch_GetBitMapTexture,
    patch_GetBlendColour,
    patch_GetBlendEquation,
    patch_GetBlendMode,
    patch_GetColourMask,
    patch_GetDepthCompareFunc,
    patch_GetFrontFace,
    patch_GetLineWidth,
    patch_GetPolygonMode,
    patch_GetPolygonOffset,
    patch_GetProvokingVertex,
    patch_GetRenderTarget,
    patch_GetScissor,
    patch_GetShaderDataBuffer,
    patch_GetShaderPipeline,
    patch_GetState,
    patch_GetStencilFunc,
    patch_GetStencilOp,
    patch_GetStencilWriteMask,
    patch_GetTexSampler,
    patch_GetTexture,
    patch_GetVertexAttribArray,
    patch_GetViewport,
    patch_IsDone,
    patch_Query,
    patch_RSOCopy,
    patch_RSOSetMaster,
    patch_SetBlendColour,
    patch_SetBlendEquation,
    patch_SetBlendEquationSeparate,
    patch_SetBlendMode,
    patch_SetBlendModeSeparate,
    patch_SetColourMask,
    patch_SetDepthCompareFunc,
    patch_SetFrontFace,
    patch_SetLineWidth,
    patch_SetPolygonMode,
    patch_SetPolygonOffset,
    patch_SetProvokingVertex,
    patch_SetRenderTarget,
    patch_SetScissor,
    patch_SetShaderPipeline,
    patch_SetState,
    patch_SetStencilFunc,
    patch_SetStencilFuncSeparate,
    patch_SetStencilOp,
    patch_SetStencilOpSeparate,
    patch_SetStencilWriteMask,
    patch_SetStencilWriteMaskSeparate,
    patch_SetViewport,
    patch_ShaderGetCount,
    patch_ShaderGetObjectInfo,
    patch_ShaderGetOffset,
    patch_ShaderGetTotalStorage,
    patch_ShaderGetType,
    patch_ShaderPipelineGetShader,
    patch_Submit,
    patch_TexGenMipMaps,
    patch_TexGetParameters,
    patch_TexGetProperty,
    patch_TexGetRMBuffer,
    patch_TexGetSubResourceLayout,
    patch_TexSetParameters,
    patch_TexUpdateImage,
    patch_TexUpdateSubImage,
    patch_TSGetParameters,
    patch_TSSetParameters,
    patch_VBOGetArray,
    patch_VBOGetAttr,
    patch_VBOLock,
    patch_VBOSetArray,
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
    size_t i;
    for (i = 0; i < sizeof(patches) / sizeof(patches[0]); i++) {
        patches[i](FALSE, nova);
    }
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

                logLine("%s: %s: tags %p (%s)", nova->name, __func__,
                    tags, decodeTags(tags, nova));

                IExec->MutexObtain(mutex);

                size_t i;
                for (i = 0; i < MAX_CLIENTS; i++) {
                    if (contexts[i] == NULL) {
                        contexts[i] = nova;
                        logAlways("[%u] Patching task %s NOVA context %p", i, nova->name, context);
                        break;
                    }
                }

                IExec->MutexRelease(mutex);

                if (i == MAX_CLIENTS) {
                    logAlways("glSnoop: too many clients, cannot patch");
                    IExec->FreeVec(nova);
                } else {
                    patch_context_functions(nova);
                    PROF_INIT(nova, NovaFunctionCount)

                    if (profilerStartTime) {
                        logLine("Trigger timer in %lu seconds", profilerStartTime);
                        // TODO: supports only one app. A proper implementation
                        // would need a some kind of a timer pool?
                        timer_start(&triggerTimer, profilerStartTime, 0);
                    } else {
                        if (profilerDuration) {
                            logLine("Signal glSnoop task %p with signal %d", mainTask, mainSig);
                            IExec->Signal(mainTask, 1L << mainSig);
                        }
                    }
                }
            } else {
                logAlways("Cannot allocate memory for NOVA context data: cannot patch");
            }
        }
    }

    return context;
}

GENERATE_PATCH(Warp3DNovaIFace, W3DN_CreateContext, my, ContextCreation)

void warp3dnova_install_patches(ULONG startTimeInSeconds, ULONG durationTimeInSeconds)
{
    profilerStartTime = startTimeInSeconds;
    profilerDuration = durationTimeInSeconds;

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
