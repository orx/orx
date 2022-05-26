/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2022 Orx-Project
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 *    1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 *
 *    2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 *    3. This notice may not be removed or altered from any source
 *    distribution.
 */

/**
 * @file orxDisplay.c
 * @date 23/06/2010
 * @author iarwain@orx-project.org
 *
 * GLFW display plugin implementation
 *
 */


#include "orxPluginAPI.h"

#ifdef __orxMAC__
#define GL_GLEXT_PROTOTYPES
#endif /* __orxMAC__ */

/* No OpenGL/ES defines? */
#if !defined(__orxDISPLAY_OPENGL__) && !defined(__orxDISPLAY_OPENGL_ES__)
  /* Linux ARM/ARM64 platforms default to OpenGL ES */
  #if defined(__orxLINUX__) && (defined(__orxARM__) || defined(__orxARM64__))
#define __orxDISPLAY_OPENGL_ES__
  /* All other platforms default to OpenGL */
  #else /* __orxLINUX__ && (__orxARM__ || __orxARM64__) */
#define __orxDISPLAY_OPENGL__
  #endif /* __orxLINUX__ && (__orxARM__ || __orxARM64__) */
#endif /* !__orxDISPLAY_OPENGL__ && !__orxDISPLAY_OPENGL_ES__ */

#ifdef __orxDISPLAY_OPENGL_ES__
#define GLFW_INCLUDE_ES3
#else /* __orxDISPLAY_OPENGL_ES__ */
#define GLFW_INCLUDE_GLEXT
#endif /* __orxDISPLAY_OPENGL_ES__ */
#include "GLFW/glfw3.h"
#undef GLFW_INCLUDE_ES3
#undef GLFW_INCLUDE_GLEXT

#if !defined(__orxMSVC__) || (_MSC_VER > 1600)
  #include "webp/decode.h"
#endif /* !__orxMSVC__ || (_MSC_VER > 1600) */

#ifdef __orxGCC__
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wpragmas"
  #pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
  #pragma GCC diagnostic ignored "-Wmisleading-indentation"
#endif /* __orxGCC__ */

#ifdef __orxMSVC__
  #pragma warning(disable : 4312)
#endif /* __orxMSVC__ */
#define STBI_NO_STDIO
#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_PSD
#define STBI_NO_GIF
#define STBI_NO_HDR
#define STBI_NO_PIC
#define STBI_NO_PNM
#define STBI_MALLOC(sz)         orxMemory_Allocate((orxU32)sz, orxMEMORY_TYPE_VIDEO)
#define STBI_REALLOC(p, newsz)  orxMemory_Reallocate(p, newsz, orxMEMORY_TYPE_VIDEO)
#define STBI_FREE(p)            orxMemory_Free(p)
#include "stb_image.h"
#undef STBI_FREE
#undef STBI_REALLOC
#undef STBI_MALLOC
#undef STBI_NO_PNM
#undef STBI_NO_PIC
#undef STBI_NO_HDR
#undef STBI_NO_GIF
#undef STBI_NO_PSD
#undef STB_IMAGE_IMPLEMENTATION
#undef STBI_NO_STDIO
#ifdef __orxMSVC__
  #pragma warning(default : 4312)
#endif /* __orxMSVC__ */

#define STBI_WRITE_NO_STDIO
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBIW_MALLOC(sz)        orxMemory_Allocate(sz, orxMEMORY_TYPE_VIDEO)
#define STBIW_REALLOC(p, newsz) orxMemory_Reallocate(p, newsz, orxMEMORY_TYPE_VIDEO)
#define STBIW_FREE(p)           orxMemory_Free(p)
#define STBIW_MEMMOVE(a, b, sz) orxMemory_Move(a, b, sz)
#define STBIW_ASSERT(x)         orxASSERT(x)
#include "stb_image_write.h"
#undef STBIW_ASSERT
#undef STBIW_MEMMOVE
#undef STBIW_FREE
#undef STBIW_REALLOC
#undef STBIW_MALLOC
#undef STB_IMAGE_WRITE_IMPLEMENTATION
#undef STBI_WRITE_NO_STDIO

#define QOI_NO_STDIO
#define QOI_IMPLEMENTATION
#define QOI_MALLOC(sz)          orxMemory_Allocate(sz, orxMEMORY_TYPE_VIDEO)
#define QOI_FREE(p)             orxMemory_Free(p)
#define QOI_ZEROARR(a)          orxMemory_Zero(a, sizeof(a))
#include "qoi.h"
#undef QOI_ZEROARR
#undef QOI_FREE
#undef QOI_MALLOC
#undef QOI_IMPLEMENTATION
#undef QOI_NO_STDIO

#include "basisu.h"


#ifndef __orxEMBEDDED__
  #ifdef __orxMSVC__
    #pragma message("!!WARNING!! This plugin will only work in non-embedded mode when linked against a *DYNAMIC* version of GLFW!")
  #else /* __orxMSVC__ */
    #warning !!WARNING!! This plugin will only work in non-embedded mode when linked against a *DYNAMIC* version of GLFW!
  #endif /* __orxMSVC__ */
#endif /* __orxEMBEDDED__ */


/** Module flags
 */
#define orxDISPLAY_KU32_STATIC_FLAG_NONE        0x00000000  /**< No flags */

#define orxDISPLAY_KU32_STATIC_FLAG_READY       0x00000001  /**< Ready flag */
#define orxDISPLAY_KU32_STATIC_FLAG_VSYNC       0x00000002  /**< VSync flag */
#define orxDISPLAY_KU32_STATIC_FLAG_SHADER      0x00000004  /**< Shader support flag */
#define orxDISPLAY_KU32_STATIC_FLAG_VBO         0x00000008  /**< VBO support flag */
#define orxDISPLAY_KU32_STATIC_FLAG_FOCUS       0x00000010  /**< Focus flag */
#define orxDISPLAY_KU32_STATIC_FLAG_BACKGROUND  0x00000020  /**< Background flag */
#define orxDISPLAY_KU32_STATIC_FLAG_NPOT        0x00000040  /**< NPOT texture support flag */
#define orxDISPLAY_KU32_STATIC_FLAG_EXT_READY   0x00000080  /**< Extensions ready flag */
#define orxDISPLAY_KU32_STATIC_FLAG_FRAMEBUFFER 0x00000100  /**< Framebuffer support flag */
#define orxDISPLAY_KU32_STATIC_FLAG_DEPTHBUFFER 0x00000200  /**< Depthbuffer support flag */
#define orxDISPLAY_KU32_STATIC_FLAG_NO_RESIZE   0x00000400  /**< No resize flag */
#define orxDISPLAY_KU32_STATIC_FLAG_IGNORE_EVENT 0x00000800 /**< Ignore event flag */
#define orxDISPLAY_KU32_STATIC_FLAG_NO_DECORATION 0x00001000 /**< No decoration flag */
#define orxDISPLAY_KU32_STATIC_FLAG_FULLSCREEN  0x00002000  /**< Full screen flag */
#define orxDISPLAY_KU32_STATIC_FLAG_CUSTOM_IBO  0x00004000  /**< Custom IBO flag */
#define orxDISPLAY_KU32_STATIC_FLAG_CONTROL_TEAR 0x00008000 /**< Swap control tear support flag */
#define orxDISPLAY_KU32_STATIC_FLAG_DEBUG_OUTPUT 0x00010000 /**< Debug output support flag */
#define orxDISPLAY_KU32_STATIC_FLAG_VSYNC_FIX   0x10000000  /**< VSync fix flag */

#define orxDISPLAY_KU32_STATIC_MASK_ALL         0xFFFFFFFF  /**< All mask */

#define orxDISPLAY_KU32_BITMAP_FLAG_NONE        0x00000000  /** No flags */

#define orxDISPLAY_KU32_BITMAP_FLAG_LOADING     0x00000001  /**< Loading flag */
#define orxDISPLAY_KU32_BITMAP_FLAG_DELETE      0x00000002  /**< Delete flag */
#define orxDISPLAY_KU32_BITMAP_FLAG_CURSOR      0x00000004  /**< Cursor flag */
#define orxDISPLAY_KU32_BITMAP_FLAG_ICON        0x00000008  /**< Icon flag */

#define orxDISPLAY_KU32_BITMAP_MASK_ALL         0xFFFFFFFF  /**< All mask */

#define orxDISPLAY_KU32_BITMAP_BANK_SIZE        256
#define orxDISPLAY_KU32_SHADER_BANK_SIZE        64

#define orxDISPLAY_KU32_VERTEX_BUFFER_SIZE      (4 * 2048) /**< 2048 items batch capacity */
#define orxDISPLAY_KU32_INDEX_BUFFER_SIZE       (6 * 2048) /**< 2048 items batch capacity */
#define orxDISPLAY_KU32_SHADER_BUFFER_SIZE      131072

#define orxDISPLAY_KF_BORDER_FIX                0.1f

#define orxDISPLAY_KF_VSYNC_DELAY_FIX           0.5f

#define orxDISPLAY_KU32_CIRCLE_LINE_NUMBER      32

#define orxDISPLAY_KU32_MAX_TEXTURE_UNIT_NUMBER 32
#define orxDISPLAY_KE_DEFAULT_PRIMITIVE         GL_TRIANGLES
#define orxDISPLAY_KV_DEFAULT_DECORATED_POSITION orx2F(100.0f), orx2F(120.0f), orxFLOAT_0

#define orxDISPLAY_KU32_MAX_ICON_NUMBER         16

#define orxDISPLAY_KU32_MAX_SHADER_VERSION      410


/**  Misc defines
 */
#if defined(__orxGCC__) || defined(__orxLLVM__)

#define glUNIFORM(EXT, LOCATION, ...) do {if((LOCATION) >= 0) {glUniform##EXT(LOCATION, ##__VA_ARGS__); glASSERT();}} while(orxFALSE)

#else /* __orxGCC__ || __orxLLVM__ */

#define glUNIFORM(EXT, LOCATION, ...) do {if((LOCATION) >= 0) {glUniform##EXT(LOCATION, __VA_ARGS__); glASSERT();}} while(orxFALSE)

#endif /* __orxGCC__ || __orxLLVM__ */


#ifdef __orxDEBUG__

#define glASSERT()                                                        \
do                                                                        \
{                                                                         \
  if(sstDisplay.pstWindow != orxNULL)                                     \
  {                                                                       \
    GLenum eError = glGetError();                                         \
    orxASSERT(eError == GL_NO_ERROR && "OpenGL error code: 0x%X", eError);\
  }                                                                       \
} while(orxFALSE)

  #if defined(__orxGCC__) || defined(__orxLLVM__)

#define glUNIFORM_NO_ASSERT(EXT, LOCATION, ...) do {if((LOCATION) >= 0) {glUniform##EXT(LOCATION, ##__VA_ARGS__); (void)glGetError();}} while(orxFALSE)

  #else /* __orxGCC__ || __orxLLVM__ */

#define glUNIFORM_NO_ASSERT(EXT, LOCATION, ...) do {if((LOCATION) >= 0) {glUniform##EXT(LOCATION, __VA_ARGS__); (void)glGetError();}} while(orxFALSE)

  #endif /* __orxGCC__ || __orxLLVM__ */

#else /* __orxDEBUG__ */

#define glASSERT()

  #if defined(__orxGCC__) || defined(__orxLLVM__)

#define glUNIFORM_NO_ASSERT(EXT, LOCATION, ...) do {if((LOCATION) >= 0) {glUniform##EXT(LOCATION, ##__VA_ARGS__);}} while(orxFALSE)

  #else /* __orxGCC__ || __orxLLVM__ */

#define glUNIFORM_NO_ASSERT(EXT, LOCATION, ...) do {if((LOCATION) >= 0) {glUniform##EXT(LOCATION, __VA_ARGS__);}} while(orxFALSE)

  #endif /* __orxGCC__ || __orxLLVM__ */

#endif /* __orxDEBUG__ */


#ifdef __orxDISPLAY_OPENGL_ES__

#define GLhandleARB GLuint

#endif /* __orxDISPLAY_OPENGL_ES__ */


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Standard cursor list
 */
#define orxDISPLAY_DECLARE_CURSOR(NAME) {#NAME, GLFW_##NAME##_CURSOR}

static struct
{
  const orxSTRING zName;
  int             iShape;
} sastStandardCursorList[] =
{
  orxDISPLAY_DECLARE_CURSOR(ARROW),
  orxDISPLAY_DECLARE_CURSOR(IBEAM),
  orxDISPLAY_DECLARE_CURSOR(CROSSHAIR),
  orxDISPLAY_DECLARE_CURSOR(HAND),
  orxDISPLAY_DECLARE_CURSOR(HRESIZE),
  orxDISPLAY_DECLARE_CURSOR(VRESIZE),
  {"DEFAULT", 0}
};

#undef orxDISPLAY_DECLARE_CURSOR

/** Attribute location
 */
typedef enum __orxDISPLAY_ATTRIBUTE_LOCATION_t
{
  orxDISPLAY_ATTRIBUTE_LOCATION_VERTEX = 0,
  orxDISPLAY_ATTRIBUTE_LOCATION_TEXCOORD,
  orxDISPLAY_ATTRIBUTE_LOCATION_COLOR,

  orxDISPLAY_ATTRIBUTE_LOCATION_NUMBER,

  orxDISPLAY_ATTRIBUTE_LOCATION_NONE = orxENUM_NONE

} orxDISPLAY_ATTRIBUTE_LOCATION;

/** Internal buffer mode
 */
typedef enum __orxDISPLAY_BUFFER_MODE_t
{
  orxDISPLAY_BUFFER_MODE_INDIRECT = 0,
  orxDISPLAY_BUFFER_MODE_DIRECT,

  orxDISPLAY_BUFFER_MODE_NUMBER,

  orxDISPLAY_BUFFER_MODE_NONE = orxENUM_NONE

} orxDISPLAY_BUFFER_MODE;

/** Internal matrix structure
 */
typedef struct __orxDISPLAY_MATRIX_t
{
  orxVECTOR vX;
  orxVECTOR vY;

} orxDISPLAY_MATRIX;

/** Internal vertex structure
 */
typedef struct __orxDISPLAY_GLFW_VERTEX_t
{
  GLfloat fX, fY;
  GLfloat fU, fV;
  orxRGBA stRGBA;

} orxDISPLAY_GLFW_VERTEX;

/** Internal projection matrix structure
 */
typedef struct __orxDISPLAY_PROJ_MATRIX_t
{
  orxFLOAT aafValueList[4][4];

} orxDISPLAY_PROJ_MATRIX;

/** Internal bitmap structure
 */
struct __orxBITMAP_t
{
  GLuint                    uiTexture;
  orxBOOL                   bSmoothing;
  orxFLOAT                  fWidth, fHeight;
  orxAABOX                  stClip;
  orxU32                    u32RealWidth, u32RealHeight, u32Depth;
  orxFLOAT                  fRecRealWidth, fRecRealHeight;
  orxU32                    u32DataSize;
  const orxSTRING           zLocation;
  orxSTRINGID               stFilenameID;
  orxU32                    u32Flags;
};

/** Internal bitmap save info structure
 */
typedef struct __orxDISPLAY_SAVE_INFO_t
{
  orxU8      *pu8ImageData;
  orxHANDLE   hResource;
  orxU32      u32Width;
  orxU32      u32Height;

} orxDISPLAY_SAVE_INFO;

/** Internal bitmap load info structure
 */
typedef struct __orxDISPLAY_LOAD_INFO_t
{
  orxU8      *pu8ImageBuffer;
  orxU8      *pu8ImageSource;
  orxS64      s64Size;
  orxBITMAP  *pstBitmap;
  orxU32      u32DataSize;
  GLuint      uiWidth;
  GLuint      uiHeight;
  GLuint      uiRealWidth;
  GLuint      uiRealHeight;
  orxBOOL     bIsBasisU;

} orxDISPLAY_LOAD_INFO;

/** Internal texture info structure
 */
typedef struct __orxDISPLAY_TEXTURE_INFO_t
{
  GLint                     iLocation;
  const orxBITMAP          *pstBitmap;

} orxDISPLAY_TEXTURE_INFO;

/** Internal param info structure
 */
typedef struct __orxDISPLAY_PARAM_INFO_t
{
  GLint                     iLocation, iLocationTop, iLocationLeft, iLocationBottom, iLocationRight;

} orxDISPLAY_PARAM_INFO;

/** Internal shader structure
 */
typedef struct __orxDISPLAY_SHADER_t
{
  orxLINKLIST_NODE          stNode;
  GLhandleARB               hProgram;
  GLint                     iTextureLocation;
  GLint                     iProjectionMatrixLocation;
  GLint                     iTextureCount;
  orxS32                    s32ParamCount;
  orxBOOL                   bPending;
  orxBOOL                   bUseCustomParam;
  orxSTRING                 zCode;
  orxDISPLAY_TEXTURE_INFO  *astTextureInfoList;
  orxDISPLAY_PARAM_INFO    *astParamInfoList;

} orxDISPLAY_SHADER;

/** Static structure
 */
typedef struct __orxDISPLAY_STATIC_t
{
  orxBANK                  *pstBitmapBank;
  orxBANK                  *pstShaderBank;
  orxLINKLIST               stActiveShaderList;
  orxBOOL                   bDefaultSmoothing;
  GLFWwindow               *pstWindow;
  GLFWcursor               *pstCursor;
  orxBITMAP                *pstScreen;
  const orxBITMAP          *pstTempBitmap;
  orxVECTOR                 vContentScale;
  orxVECTOR                 vWindowPosition;
  GLFWimage                 astIconList[orxDISPLAY_KU32_MAX_ICON_NUMBER];
  orxS32                    s32IconNumber, s32PendingIconCount;
  orxRGBA                   stLastColor;
  orxU32                    u32LastClipX, u32LastClipY, u32LastClipWidth, u32LastClipHeight;
  orxDISPLAY_BLEND_MODE     eLastBlendMode;
  orxDISPLAY_BUFFER_MODE    eLastBufferMode;
  GLenum                    ePrimitive;
  orxS32                    s32PendingShaderCount;
  GLint                     iLastViewportX, iLastViewportY;
  GLsizei                   iLastViewportWidth, iLastViewportHeight;
  GLfloat                   fLastOrthoRight, fLastOrthoBottom;
  orxDISPLAY_SHADER        *pstDefaultShader;
  orxDISPLAY_SHADER        *pstNoTextureShader;
  orxFLOAT                  fClockTickSize;
  GLint                     iTextureUnitNumber;
  GLint                     iDrawBufferNumber;
  GLint                     iMaxTextureSize;
  orxU32                    u32DestinationBitmapCount;
  GLuint                    uiFrameBuffer;
  GLuint                    uiLastFrameBuffer;
  GLuint                    uiVertexBuffer;
  GLuint                    uiIndexBuffer;
  orxS32                    s32BufferIndex;
  orxS32                    s32ElementNumber;
  orxU32                    u32Flags;
  orxU32                    u32Depth;
  orxU32                    u32RefreshRate;
  orxU32                    u32DefaultWidth;
  orxU32                    u32DefaultHeight;
  orxU32                    u32DefaultDepth;
  orxU32                    u32DefaultRefreshRate;
  orxS32                    s32ActiveTextureUnit;
  stbi_io_callbacks         stSTBICallbacks;
  BasisUFormat              eBasisUFormat;
  GLenum                    aeDrawBufferList[orxDISPLAY_KU32_MAX_TEXTURE_UNIT_NUMBER];
  orxBITMAP                *apstDestinationBitmapList[orxDISPLAY_KU32_MAX_TEXTURE_UNIT_NUMBER];
  const orxBITMAP          *apstBoundBitmapList[orxDISPLAY_KU32_MAX_TEXTURE_UNIT_NUMBER];
  orxDOUBLE                 adMRUBitmapList[orxDISPLAY_KU32_MAX_TEXTURE_UNIT_NUMBER];
  orxDISPLAY_PROJ_MATRIX    mProjectionMatrix;
  orxDISPLAY_GLFW_VERTEX    astVertexList[orxDISPLAY_KU32_VERTEX_BUFFER_SIZE];
  GLushort                  au16IndexList[orxDISPLAY_KU32_INDEX_BUFFER_SIZE];
  orxCHAR                   acShaderCodeBuffer[orxDISPLAY_KU32_SHADER_BUFFER_SIZE];

} orxDISPLAY_STATIC;


/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** Static data
 */
static orxDISPLAY_STATIC sstDisplay;

#ifdef __orxDISPLAY_OPENGL_ES__

#define glCreateProgramObjectARB    glCreateProgram
#define glCreateShaderObjectARB     glCreateShader
#define glDeleteObjectARB           glDeleteShader
#define glShaderSourceARB           glShaderSource
#define glCompileShaderARB          glCompileShader
#define glAttachObjectARB           glAttachShader
#define glLinkProgramARB            glLinkProgram
#define glGetObjectParameterivARB   glGetProgramiv
#define glGetInfoLogARB             glGetProgramInfoLog
#define glUseProgramObjectARB       glUseProgram
#define glGetUniformLocationARB     glGetUniformLocation
#define glBindAttribLocationARB     glBindAttribLocation;
#define glEnableVertexAttribArrayARB glEnableVertexAttribArray;
#define glVertexAttribPointerARB    glVertexAttribPointer;
#define glUniform1fARB              glUniform1f
#define glUniform3fARB              glUniform3f
#define glUniform1iARB              glUniform1i
#define glUniformMatrix4fvARB       glUniformMatrix4fv;

#define glGenBuffersARB             glGenBuffers
#define glDeleteBuffersARB          glDeleteBuffers
#define glBindBufferARB             glBindBuffer
#define glBufferDataARB             glBufferData
#define glBufferSubDataARB          glBufferSubData
#define glDrawBuffersARB            glDrawBuffers
#define glActiveTextureARB          glActiveTexture

#define glGenFramebuffersEXT        glGenFramebuffers
#define glDeleteFramebuffersEXT     glDeleteFramebuffers
#define glBindFramebufferEXT        glBindFramebuffer
#define glCheckFramebufferStatusEXT glCheckFramebufferStatus
#define glFramebufferTexture2DEXT   glFramebufferTexture2D

#define glClearDepth                glClearDepthf

#define GL_TEXTURE0_ARB             GL_TEXTURE0
#define GL_OBJECT_COMPILE_STATUS_ARB GL_COMPILE_STATUS
#define GL_OBJECT_LINK_STATUS_ARB   GL_LINK_STATUS
#define GL_ARRAY_BUFFER_ARB         GL_ARRAY_BUFFER
#define GL_ELEMENT_ARRAY_BUFFER_ARB GL_ELEMENT_ARRAY_BUFFER
#define GL_DYNAMIC_DRAW_ARB         GL_DYNAMIC_DRAW
#define GL_STATIC_DRAW_ARB          GL_STATIC_DRAW
#define GL_STREAM_DRAW_ARB          GL_STREAM_DRAW

#define GL_FRAMEBUFFER_EXT          GL_FRAMEBUFFER
#define GL_COLOR_ATTACHMENT0_EXT    GL_COLOR_ATTACHMENT0
#define GL_FRAMEBUFFER_COMPLETE_EXT GL_FRAMEBUFFER_COMPLETE

#else /* __orxDISPLAY_OPENGL_ES__ */

#define glDeleteProgram             glDeleteObjectARB

/** Shader-related OpenGL extension functions
 */
  #ifndef __orxMAC__

PFNGLCREATEPROGRAMOBJECTARBPROC     glCreateProgramObjectARB    = NULL;
PFNGLCREATESHADEROBJECTARBPROC      glCreateShaderObjectARB     = NULL;
PFNGLDELETEOBJECTARBPROC            glDeleteObjectARB           = NULL;
PFNGLSHADERSOURCEARBPROC            glShaderSourceARB           = NULL;
PFNGLCOMPILESHADERARBPROC           glCompileShaderARB          = NULL;
PFNGLATTACHOBJECTARBPROC            glAttachObjectARB           = NULL;
PFNGLLINKPROGRAMARBPROC             glLinkProgramARB            = NULL;
PFNGLGETOBJECTPARAMETERIVARBPROC    glGetObjectParameterivARB   = NULL;
PFNGLGETINFOLOGARBPROC              glGetInfoLogARB             = NULL;
PFNGLUSEPROGRAMOBJECTARBPROC        glUseProgramObjectARB       = NULL;
PFNGLGETUNIFORMLOCATIONARBPROC      glGetUniformLocationARB     = NULL;
PFNGLBINDATTRIBLOCATIONARBPROC      glBindAttribLocationARB     = NULL;
PFNGLENABLEVERTEXATTRIBARRAYARBPROC glEnableVertexAttribArrayARB= NULL;
PFNGLVERTEXATTRIBPOINTERARBPROC     glVertexAttribPointerARB    = NULL;
PFNGLUNIFORM1FARBPROC               glUniform1fARB              = NULL;
PFNGLUNIFORM3FARBPROC               glUniform3fARB              = NULL;
PFNGLUNIFORM1IARBPROC               glUniform1iARB              = NULL;
PFNGLUNIFORMMATRIX4FVARBPROC        glUniformMatrix4fvARB       = NULL;
PFNGLDEBUGMESSAGECALLBACKARBPROC    glDebugMessageCallback      = NULL;

PFNGLGENBUFFERSARBPROC              glGenBuffersARB             = NULL;
PFNGLDELETEBUFFERSARBPROC           glDeleteBuffersARB          = NULL;
PFNGLBINDBUFFERARBPROC              glBindBufferARB             = NULL;
PFNGLBUFFERDATAARBPROC              glBufferDataARB             = NULL;
PFNGLBUFFERSUBDATAARBPROC           glBufferSubDataARB          = NULL;
PFNGLDRAWBUFFERSARBPROC             glDrawBuffersARB            = NULL;

PFNGLGENFRAMEBUFFERSEXTPROC         glGenFramebuffersEXT        = NULL;
PFNGLDELETEFRAMEBUFFERSEXTPROC      glDeleteFramebuffersEXT     = NULL;
PFNGLBINDFRAMEBUFFEREXTPROC         glBindFramebufferEXT        = NULL;
PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC  glCheckFramebufferStatusEXT = NULL;
PFNGLFRAMEBUFFERTEXTURE2DEXTPROC    glFramebufferTexture2DEXT   = NULL;


    #ifndef __orxLINUX__

PFNGLCOMPRESSEDTEXIMAGE2DPROC       glCompressedTexImage2D      = NULL;
PFNGLACTIVETEXTUREARBPROC           glActiveTextureARB          = NULL;

    #endif /* !__orxLINUX__ */

  #endif /* !__orxMAC__ */

#endif /* __orxDISPLAY_OPENGL_ES__ */

/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

/** Prototypes
 */
orxSTATUS orxFASTCALL orxDisplay_GLFW_StartShader(orxHANDLE _hShader);
orxSTATUS orxFASTCALL orxDisplay_GLFW_StopShader(orxHANDLE _hShader);
orxSTATUS orxFASTCALL orxDisplay_GLFW_SetBlendMode(orxDISPLAY_BLEND_MODE _eBlendMode);
orxSTATUS orxFASTCALL orxDisplay_GLFW_SetDestinationBitmaps(orxBITMAP **_apstBitmapList, orxU32 _u32Number);
orxSTATUS orxFASTCALL orxDisplay_GLFW_SetVideoMode(const orxDISPLAY_VIDEO_MODE *_pstVideoMode);


/** Render inhibitor
 */
static orxSTATUS orxFASTCALL orxDisplay_GLFW_RenderInhibitor(const orxEVENT *_pstEvent)
{
  /* Render stop? */
  if(_pstEvent->eID == orxRENDER_EVENT_STOP)
  {
    /* Profiles */
    orxPROFILER_PUSH_MARKER("PollEvents");

    /* Polls events */
    glfwPollEvents();

    /* Profiles */
    orxPROFILER_POP_MARKER();
  }

  /* Done! */
  return orxSTATUS_FAILURE;
}

static orxINLINE GLFWmonitor *orxDisplay_GLFW_GetMonitor()
{
  GLFWmonitor **apstMonitors;
  int           iCount;
  GLFWmonitor  *pstResult = NULL;

  /* Gets monitor list */
  apstMonitors = glfwGetMonitors(&iCount);

  /* Valid? */
  if(apstMonitors != NULL)
  {
    orxU32 u32Monitor;

    /* Pushes config section */
    orxConfig_PushSection(orxDISPLAY_KZ_CONFIG_SECTION);

    /* Gets config monitor */
    u32Monitor = orxConfig_GetU32(orxDISPLAY_KZ_CONFIG_MONITOR);

    /* Valid? */
    if(u32Monitor != 0)
    {
      /* Is index in range? */
      if(u32Monitor <= (orxU32)iCount)
      {
        /* Updates result */
        pstResult = apstMonitors[u32Monitor - 1];
      }
      else
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Invalid monitor: index [%u] out of range, max is [%d]. Reverting to current monitor.", u32Monitor, iCount);
      }
    }

    /* Not found? */
    if(pstResult == NULL)
    {
      /* Is full screen? */
      if(orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_FULLSCREEN))
      {
        /* Gets current monitor */
        pstResult = glfwGetWindowMonitor(sstDisplay.pstWindow);
      }

      /* Still no monitor? */
      if(pstResult == NULL)
      {
        /* Has window? */
        if(sstDisplay.pstWindow != orxNULL)
        {
          int iWindowX = 0, iWindowY = 0;

          /* Gets window position */
          glfwGetWindowPos(sstDisplay.pstWindow, &iWindowX, &iWindowY);

          /* Success? */
          if(glfwGetError(NULL) == GLFW_NO_ERROR)
          {
            orxS32 i;

            /* For all monitors */
            for(i = 0; i < iCount; i++)
            {
              int iX, iY;

              /* Gets its position */
              glfwGetMonitorPos(apstMonitors[i], &iX, &iY);

              /* Success? */
              if(glfwGetError(NULL) == GLFW_NO_ERROR)
              {
                const GLFWvidmode *pstMode;

                /* Gets its mode */
                pstMode = glfwGetVideoMode(apstMonitors[i]);

                /* Success? */
                if(glfwGetError(NULL) == GLFW_NO_ERROR)
                {
                  /* Inside? */
                  if((iWindowX >= iX)
                  && (iWindowY >= iY)
                  && (iWindowX - iX < pstMode->width)
                  && (iWindowY - iY < pstMode->height))
                  {
                    /* Selects it */
                    pstResult   = apstMonitors[i];
                    u32Monitor  = (orxU32)(i + 1);
                    break;
                  }
                }
              }
            }
          }
        }

        /* Still no monitor? */
        if(pstResult == NULL)
        {
          /* Defaults to primary monitor */
          pstResult   = glfwGetPrimaryMonitor();
          u32Monitor  = 1;
        }
      }
      else
      {
        orxS32 i;

        /* For all monitors */
        for(i = 0; i < iCount; i++)
        {
          /* Found? */
          if(apstMonitors[i] == pstResult)
          {
            /* Gets its ID */
            u32Monitor = (orxU32)(i + 1);
            break;
          }
        }
      }

      /* Stores it */
      orxConfig_SetU32(orxDISPLAY_KZ_CONFIG_MONITOR, u32Monitor);
    }

    /* Pops config section */
    orxConfig_PopSection();
  }

  /* Done! */
  return (pstResult != NULL) ? pstResult : orxNULL;
}

static orxINLINE void orxDisplay_GLFW_UpdateDefaultMode()
{
  GLFWmonitor *pstMonitor;

  /* Gets current monitor */
  pstMonitor = orxDisplay_GLFW_GetMonitor();

  /* Success? */
  if(pstMonitor != orxNULL)
  {
    const GLFWvidmode *pstDesktopMode;

    /* Gets desktop mode */
    pstDesktopMode = glfwGetVideoMode(pstMonitor);

    /* Updates default mode */
    sstDisplay.u32DefaultWidth  = (orxU32)pstDesktopMode->width;
    sstDisplay.u32DefaultHeight = (orxU32)pstDesktopMode->height;
    sstDisplay.u32DefaultDepth  = (orxU32)(pstDesktopMode->redBits + pstDesktopMode->greenBits + pstDesktopMode->blueBits);

    /* 24-bit? */
    if(sstDisplay.u32DefaultDepth == 24)
    {
      /* Gets 32-bit instead */
      sstDisplay.u32DefaultDepth = 32;
    }

    /* Hack: Corrects imprecise refresh rate reports for default mode */
    switch(pstDesktopMode->refreshRate)
    {
      case 59:
      case 60:
      case 61:
      {
        sstDisplay.u32DefaultRefreshRate = 60;
        break;
      }

      case 49:
      case 50:
      case 51:
      {
        sstDisplay.u32DefaultRefreshRate = 50;
        break;
      }

      default:
      {
        sstDisplay.u32DefaultRefreshRate = pstDesktopMode->refreshRate;
        break;
      }
    }
  }

  /* Done! */
  return;
}

#ifndef __orxMAC__
static void GLAPIENTRY orxDisplay_GLFW_MessageCallback(GLenum _eSource, GLenum _eType, GLuint _uID, GLenum _eSeverity, GLsizei _iLength, const GLchar *_zMessage, const void *_pContext)
{
  /* Relevant type? */
  if(_eType != GL_DEBUG_TYPE_OTHER)
  {
    const orxSTRING zType;

    /* Gets type literal */
    switch(_eType)
    {
      case GL_DEBUG_TYPE_ERROR:               zType = "ERROR";        break;
      case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: zType = "DEPRECATED";   break;
      case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  zType = "UNDEFINED";    break;
      case GL_DEBUG_TYPE_PORTABILITY:         zType = "PORTABILITY";  break;
      case GL_DEBUG_TYPE_PERFORMANCE:         zType = "PERFORMANCE";  break;
      case GL_DEBUG_TYPE_MARKER:              zType = "MARKER";       break;
      case GL_DEBUG_TYPE_PUSH_GROUP:          zType = "PUSH";         break;
      case GL_DEBUG_TYPE_POP_GROUP:           zType = "POP";          break;
      default:                                zType = "UNKNOWN";      break;
    }

    /* Logs it */
    orxLOG("[GL %s] %s", zType, _zMessage);
  }

  /* Done! */
  return;
}
#endif /* __orxMAC__ */

static void orxDisplay_GLFW_ResizeCallback(GLFWwindow *_pstWindow, int _iWidth, int _iHeight)
{
  /* Not ignoring event? */
  if(!orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_IGNORE_EVENT))
  {
    /* Valid? */
    if((_iWidth > 0) && (_iHeight > 0))
    {
      orxDISPLAY_VIDEO_MODE stVideoMode;

      /* Inits video mode */
      stVideoMode.u32Width        = (orxU32)_iWidth;
      stVideoMode.u32Height       = (orxU32)_iHeight;
      stVideoMode.u32Depth        = sstDisplay.u32Depth;
      stVideoMode.u32RefreshRate  = sstDisplay.u32RefreshRate;
      stVideoMode.bFullScreen     = orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_FULLSCREEN) ? orxTRUE : orxFALSE;

      /* Applies it */
      orxDisplay_GLFW_SetVideoMode(&stVideoMode);
    }
  }

  /* Done! */
  return;
}

static void orxDisplay_GLFW_DropCallback(GLFWwindow *_pstWindow, int _iNumber, const char **_azPaths)
{
  orxSYSTEM_EVENT_PAYLOAD stPayload;

  /* Inits payload */
  orxMemory_Zero(&stPayload, sizeof(orxSYSTEM_EVENT_PAYLOAD));
  stPayload.stDrop.azValueList  = (const orxSTRING *)_azPaths;
  stPayload.stDrop.u32Number    = (orxU32)_iNumber;

  /* Sends event */
  orxEVENT_SEND(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_DROP, orxNULL, orxNULL, &stPayload);

  /* Done! */
  return;
}

static void orxDisplay_GLFW_PosCallback(GLFWwindow *_pstWindow, int _iX, int _iY)
{
  /* Not ignoring event and not fullscreen? */
  if(!orxFLAG_TEST(sstDisplay.u32Flags, (orxDISPLAY_KU32_STATIC_FLAG_IGNORE_EVENT | orxDISPLAY_KU32_STATIC_FLAG_FULLSCREEN)))
  {
    orxVECTOR     vPosition;
    GLFWmonitor **apstMonitors;
    orxU32        u32Monitor = 1;
    int           iCount;

    /* Stores raw position */
    sstDisplay.vWindowPosition.fX = orx2F(_iX);
    sstDisplay.vWindowPosition.fY = orx2F(_iY);
    orxVector_Copy(&vPosition, &(sstDisplay.vWindowPosition));

    /* Gets monitor list */
    apstMonitors = glfwGetMonitors(&iCount);

    /* Valid? */
    if(apstMonitors != NULL)
    {
      orxS32 i;

      /* For all monitors */
      for(i = 0; i < iCount; i++)
      {
        int iX, iY;

        /* Gets its position */
        glfwGetMonitorPos(apstMonitors[i], &iX, &iY);

        /* Success? */
        if(glfwGetError(NULL) == GLFW_NO_ERROR)
        {
          const GLFWvidmode  *pstMode;

          /* Gets its mode */
          pstMode = glfwGetVideoMode(apstMonitors[i]);

          /* Success? */
          if(glfwGetError(NULL) == GLFW_NO_ERROR)
          {
            /* Inside? */
            if((_iX >= iX)
            && (_iY >= iY)
            && (_iX - iX < pstMode->width)
            && (_iY - iY < pstMode->height))
            {
              /* Updates position */
              vPosition.fX -= orx2F(iX);
              vPosition.fY -= orx2F(iY);

              /* Updates monitor */
              u32Monitor = (orxU32)(i + 1);
              break;
            }
          }
        }
      }
    }

    /* Stores position & monitor in config */
    orxConfig_PushSection(orxDISPLAY_KZ_CONFIG_SECTION);
    orxConfig_SetVector(orxDISPLAY_KZ_CONFIG_POSITION, &vPosition);
    orxConfig_SetU32(orxDISPLAY_KZ_CONFIG_MONITOR, u32Monitor);
    orxConfig_PopSection();
  }

  /* Done! */
  return;
}

static void orxDisplay_GLFW_ContentScaleCallback(GLFWwindow *_pstWindow, float _fScaleX, float _fScaleY)
{
  /* Not ignoring event? */
  if(!orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_IGNORE_EVENT))
  {
    orxDISPLAY_VIDEO_MODE stVideoMode;
    int                   iWindowX = 0, iWindowY = 0;

    /* Gets window position */
    glfwGetWindowPos(_pstWindow, &iWindowX, &iWindowY);

    /* Forces a position update to prevent window from going back to its previous monitor */
    orxDisplay_GLFW_PosCallback(_pstWindow, iWindowX, iWindowY);

    /* Retrieves current video mode */
    stVideoMode.u32Width        = orxF2U(sstDisplay.pstScreen->fWidth);
    stVideoMode.u32Height       = orxF2U(sstDisplay.pstScreen->fHeight);
    stVideoMode.u32Depth        = sstDisplay.u32Depth;
    stVideoMode.u32RefreshRate  = sstDisplay.u32RefreshRate;
    stVideoMode.bFullScreen     = orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_FULLSCREEN) ? orxTRUE : orxFALSE;

    /* Applies it */
    orxDisplay_GLFW_SetVideoMode(&stVideoMode);
  }

  /* Done! */
  return;
}

static void orxFASTCALL orxDisplay_GLFW_Update(const orxCLOCK_INFO *_pstClockInfo, void *_pContext)
{
  /* Profiles */
  orxPROFILER_PUSH_MARKER("orxDisplay_Update");

  /* Has window? */
  if(sstDisplay.pstWindow != orxNULL)
  {
    /* Foreground? */
    if(glfwGetWindowAttrib(sstDisplay.pstWindow, GLFW_ICONIFIED) == GLFW_FALSE)
    {
      /* Wasn't in the foreground before? */
      if(orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_BACKGROUND))
      {
        /* Has backup clock tick size? */
        if(sstDisplay.fClockTickSize >= orxFLOAT_0)
        {
          orxCLOCK *pstClock;

          /* Gets core clock */
          pstClock = orxClock_Get(orxCLOCK_KZ_CORE);

          /* Valid? */
          if(pstClock != orxNULL)
          {
            /* Restores its tick size */
            orxClock_SetTickSize(pstClock, sstDisplay.fClockTickSize);
          }
        }

        /* Sends foreground event */
        if(orxEvent_SendShort(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_FOREGROUND) != orxSTATUS_FAILURE)
        {
          /* Removes render inhibitor */
          orxEvent_RemoveHandler(orxEVENT_TYPE_RENDER, orxDisplay_GLFW_RenderInhibitor);
        }

        /* Updates foreground status */
        orxFLAG_SET(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_NONE, orxDISPLAY_KU32_STATIC_FLAG_BACKGROUND);
      }
    }
    /* Background */
    else
    {
      /* Wasn't in the background before? */
      if(!orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_BACKGROUND))
      {
        /* Clears backup clock tick size */
        sstDisplay.fClockTickSize = -orxFLOAT_1;

        /* Sends background event */
        if(orxEvent_SendShort(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_BACKGROUND) != orxSTATUS_FAILURE)
        {
          /* Adds render inhibitor */
          orxEvent_AddHandler(orxEVENT_TYPE_RENDER, orxDisplay_GLFW_RenderInhibitor);

          /* Is VSync on? */
          if(orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_VSYNC))
          {
            orxCLOCK *pstClock;

            /* Gets core clock */
            pstClock = orxClock_Get(orxCLOCK_KZ_CORE);

            /* Valid? */
            if(pstClock != orxNULL)
            {
              /* Backups its tick size */
              sstDisplay.fClockTickSize = orxClock_GetInfo(pstClock)->fTickSize;

              /* Sets its tick size to match the refresh rate */
              orxClock_SetTickSize(pstClock, orxFLOAT_1 / orxU2F(sstDisplay.u32RefreshRate));
            }
          }
        }

        /* Updates background status */
        orxFLAG_SET(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_BACKGROUND, orxDISPLAY_KU32_STATIC_FLAG_NONE);
      }
    }

    /* Has focus? */
    if(glfwGetWindowAttrib(sstDisplay.pstWindow, GLFW_FOCUSED) != GLFW_FALSE)
    {
      /* Didn't have focus before? */
      if(!orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_FOCUS))
      {
        /* Sends focus gained event */
        orxEvent_SendShort(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_FOCUS_GAINED);

        /* Updates focus status */
        orxFLAG_SET(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_FOCUS, orxDISPLAY_KU32_STATIC_FLAG_NONE);
      }
    }
    /* Out of focus */
    else
    {
      /* Had focus before? */
      if(orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_FOCUS))
      {
        /* Sends focus lost event */
        orxEvent_SendShort(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_FOCUS_LOST);

        /* Updates focus status */
        orxFLAG_SET(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_NONE, orxDISPLAY_KU32_STATIC_FLAG_FOCUS);
      }
    }

    /* Should close window? */
    if(glfwWindowShouldClose(sstDisplay.pstWindow) != GLFW_FALSE)
    {
      /* Sends system close event */
      if(orxEvent_SendShort(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_CLOSE) == orxSTATUS_FAILURE)
      {
        /* Resets close status */
        glfwSetWindowShouldClose(sstDisplay.pstWindow, GLFW_FALSE);
      }
    }
  }

  /* Profiles */
  orxPROFILER_POP_MARKER();

  /* Done! */
  return;
}

static orxDISPLAY_PROJ_MATRIX *orxDisplay_GLFW_OrthoProjMatrix(orxDISPLAY_PROJ_MATRIX *_pmResult, orxFLOAT _fLeft, orxFLOAT _fRight, orxFLOAT _fBottom, orxFLOAT _fTop, orxFLOAT _fNear, orxFLOAT _fFar)
{
  orxFLOAT                fDeltaX, fDeltaY, fDeltaZ;
  orxDISPLAY_PROJ_MATRIX *pmResult;

  /* Checks */
  orxASSERT(_pmResult != orxNULL);

  /* Gets deltas */
  fDeltaX = _fRight - _fLeft;
  fDeltaY = _fTop - _fBottom;
  fDeltaZ = _fFar - _fNear;

  /* Valid? */
  if((fDeltaX != orxFLOAT_0) && (fDeltaY != orxFLOAT_0) && (fDeltaZ != orxFLOAT_0))
  {
    /* Clears matrix */
    orxMemory_Zero(_pmResult, sizeof(orxDISPLAY_PROJ_MATRIX));

    /* Updates result */
    _pmResult->aafValueList[0][0] = orx2F(2.0f) / fDeltaX;
    _pmResult->aafValueList[3][0] = -(_fRight + _fLeft) / fDeltaX;
    _pmResult->aafValueList[1][1] = orx2F(2.0f) / fDeltaY;
    _pmResult->aafValueList[3][1] = -(_fTop + _fBottom) / fDeltaY;
    _pmResult->aafValueList[2][2] = orx2F(-2.0f) / fDeltaZ;
    _pmResult->aafValueList[3][2] = -(_fNear + _fFar) / fDeltaZ;
    _pmResult->aafValueList[3][3] = orxFLOAT_1;
    pmResult = _pmResult;
  }
  else
  {
    /* Updates result */
    pmResult = orxNULL;
  }

  /* Done! */
  return pmResult;
}

static orxINLINE void orxDisplay_GLFW_BindBitmap(const orxBITMAP *_pstBitmap)
{
  orxDOUBLE dBestTime;
  orxS32    i, s32BestCandidate;

  /* For all texture units */
  for(i = 0, s32BestCandidate = 0, dBestTime = orxDOUBLE_MAX; i < (orxS32)sstDisplay.iTextureUnitNumber; i++)
  {
    /* Found? */
    if(sstDisplay.apstBoundBitmapList[i] == _pstBitmap)
    {
      /* Stops */
      break;
    }
    /* Is first empty? */
    else if((dBestTime != orxDOUBLE_0) && (sstDisplay.apstBoundBitmapList[i] == orxNULL))
    {
      /* Selects it */
      s32BestCandidate = i;
      dBestTime = orxDOUBLE_0;
    }
    /* Older candidate? */
    else if(sstDisplay.adMRUBitmapList[i] < dBestTime)
    {
      /* Selects it */
      s32BestCandidate = i;
      dBestTime = sstDisplay.adMRUBitmapList[i];
    }
  }

  /* Found? */
  if(i < (orxS32)sstDisplay.iTextureUnitNumber)
  {
    /* Has shader support? */
    if(orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_SHADER))
    {
      /* Selects unit */
      glActiveTextureARB(GL_TEXTURE0_ARB + i);
      glASSERT();
    }

    /* Updates MRU timestamp */
    sstDisplay.adMRUBitmapList[i] = orxSystem_GetSystemTime();

    /* Updates active texture unit */
    sstDisplay.s32ActiveTextureUnit = i;
  }
  else
  {
    /* Has shader support? */
    if(orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_SHADER))
    {
      /* Selects unit */
      glActiveTextureARB(GL_TEXTURE0_ARB + s32BestCandidate);
      glASSERT();
    }

    /* Binds texture */
    glBindTexture(GL_TEXTURE_2D, _pstBitmap->uiTexture);
    glASSERT();

    /* Stores texture */
    sstDisplay.apstBoundBitmapList[s32BestCandidate] = _pstBitmap;

    /* Updates MRU timestamp */
    sstDisplay.adMRUBitmapList[s32BestCandidate] = orxSystem_GetSystemTime();

    /* Updates active texture unit */
    sstDisplay.s32ActiveTextureUnit = s32BestCandidate;
  }

  /* Done! */
  return;
}

static orxINLINE orxDISPLAY_MATRIX *orxDisplay_GLFW_InitMatrix(orxDISPLAY_MATRIX *_pmMatrix, orxFLOAT _fPosX, orxFLOAT _fPosY, orxFLOAT _fScaleX, orxFLOAT _fScaleY, orxFLOAT _fRotation, orxFLOAT _fPivotX, orxFLOAT _fPivotY)
{
  orxFLOAT fCos, fSin, fSCosX, fSCosY, fSSinX, fSSinY, fTX, fTY;

  /* Has rotation? */
  if(_fRotation != orxFLOAT_0)
  {
    /* Gets its cos/sin */
    fCos = orxMath_Cos(_fRotation);
    fSin = orxMath_Sin(_fRotation);
  }
  else
  {
    /* Inits cos/sin */
    fCos = orxFLOAT_1;
    fSin = orxFLOAT_0;
  }

  /* Computes values */
  fSCosX  = _fScaleX * fCos;
  fSCosY  = _fScaleY * fCos;
  fSSinX  = _fScaleX * fSin;
  fSSinY  = _fScaleY * fSin;
  fTX     = _fPosX - (_fPivotX * fSCosX) + (_fPivotY * fSSinY);
  fTY     = _fPosY - (_fPivotX * fSSinX) - (_fPivotY * fSCosY);

  /* Updates matrix */
  orxVector_Set(&(_pmMatrix->vX), fSCosX, -fSSinY, fTX);
  orxVector_Set(&(_pmMatrix->vY), fSSinX, fSCosY, fTY);

  /* Done! */
  return _pmMatrix;
}

static orxINLINE void orxDisplay_GLFW_InitExtensions()
{
#define orxDISPLAY_LOAD_EXTENSION_FUNCTION(TYPE, FN)  FN = (TYPE)glfwGetProcAddress(#FN);

  /* Not already initialized? */
  if(!orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_EXT_READY))
  {
    orxU32 i;

#ifdef __orxDISPLAY_OPENGL_ES__

    /* Updates status flags */
    orxFLAG_SET(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_FRAMEBUFFER, orxDISPLAY_KU32_STATIC_FLAG_NONE);

#else /* __orxDISPLAY_OPENGL_ES__ */

    /* Supports frame buffer? */
    if(glfwExtensionSupported("GL_EXT_framebuffer_object") != GLFW_FALSE)
    {
  #ifndef __orxMAC__

      /* Loads frame buffer extension functions */
      orxDISPLAY_LOAD_EXTENSION_FUNCTION(PFNGLGENFRAMEBUFFERSEXTPROC, glGenFramebuffersEXT);
      orxDISPLAY_LOAD_EXTENSION_FUNCTION(PFNGLDELETEFRAMEBUFFERSEXTPROC, glDeleteFramebuffersEXT);
      orxDISPLAY_LOAD_EXTENSION_FUNCTION(PFNGLBINDFRAMEBUFFEREXTPROC, glBindFramebufferEXT);
      orxDISPLAY_LOAD_EXTENSION_FUNCTION(PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC, glCheckFramebufferStatusEXT);
      orxDISPLAY_LOAD_EXTENSION_FUNCTION(PFNGLFRAMEBUFFERTEXTURE2DEXTPROC, glFramebufferTexture2DEXT);

  #endif /* !__orxMAC__ */

      /* Updates status flags */
      orxFLAG_SET(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_FRAMEBUFFER, orxDISPLAY_KU32_STATIC_FLAG_NONE);
    }
    else
    {
      /* Updates status flags */
      orxFLAG_SET(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_NONE, orxDISPLAY_KU32_STATIC_FLAG_FRAMEBUFFER);
    }

#endif /* __orxDISPLAY_OPENGL_ES__ */

    /* Gets max texture size */
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &(sstDisplay.iMaxTextureSize));
    glASSERT();

#ifdef __orxDISPLAY_OPENGL_ES__

    /* Gets number of available draw buffers */
    glGetIntegerv(GL_MAX_DRAW_BUFFERS, &sstDisplay.iDrawBufferNumber);
    glASSERT();
    sstDisplay.iDrawBufferNumber = orxMIN(sstDisplay.iDrawBufferNumber, orxDISPLAY_KU32_MAX_TEXTURE_UNIT_NUMBER);

    /* Fills the list of draw buffer symbols */
    for(i = 0; i < (orxU32)sstDisplay.iDrawBufferNumber; i++)
    {
      sstDisplay.aeDrawBufferList[i] = GL_COLOR_ATTACHMENT0 + i;
    }

    /* Updates status flags */
    orxFLAG_SET(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_NPOT, orxDISPLAY_KU32_STATIC_FLAG_NONE);

#else /* __orxDISPLAY_OPENGL_ES__ */

    /* Supports draw buffer? */
    if(glfwExtensionSupported("GL_ARB_draw_buffers") != GLFW_FALSE)
    {
  #ifndef __orxMAC__

      /* Loads draw buffers extension functions */
      orxDISPLAY_LOAD_EXTENSION_FUNCTION(PFNGLDRAWBUFFERSARBPROC, glDrawBuffersARB);

  #endif /* !__orxMAC__ */

      /* Gets number of available draw buffers */
      glGetIntegerv(GL_MAX_DRAW_BUFFERS, &sstDisplay.iDrawBufferNumber);
      glASSERT();
      sstDisplay.iDrawBufferNumber = orxMIN(sstDisplay.iDrawBufferNumber, orxDISPLAY_KU32_MAX_TEXTURE_UNIT_NUMBER);
    }
    else
    {
      /* Uses only a single draw buffer */
      sstDisplay.iDrawBufferNumber = 1;
    }

    /* Fills the list of draw buffer symbols */
    for(i = 0; i < (orxU32)sstDisplay.iDrawBufferNumber; i++)
    {
      sstDisplay.aeDrawBufferList[i] = GL_COLOR_ATTACHMENT0_EXT + i;
    }

    /* Has NPOT texture support? */
    if(glfwExtensionSupported("GL_ARB_texture_non_power_of_two") != GLFW_FALSE)
    {
      /* Updates status flags */
      orxFLAG_SET(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_NPOT, orxDISPLAY_KU32_STATIC_FLAG_NONE);
    }
    else
    {
      /* Updates status flags */
      orxFLAG_SET(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_NONE, orxDISPLAY_KU32_STATIC_FLAG_NPOT);
    }

#endif /* __orxDISPLAY_OPENGL_ES__ */

#ifdef __orxDISPLAY_OPENGL_ES__

    /* Updates status flags */
    orxFLAG_SET(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_VBO, orxDISPLAY_KU32_STATIC_FLAG_NONE);

#else /* __orxDISPLAY_OPENGL_ES__ */

    /* Can support vertex buffer objects? */
    if(glfwExtensionSupported("GL_ARB_vertex_buffer_object") != GLFW_FALSE)
    {
  #ifndef __orxMAC__

      /* Loads frame buffer extension functions */
      orxDISPLAY_LOAD_EXTENSION_FUNCTION(PFNGLGENBUFFERSARBPROC, glGenBuffersARB);
      orxDISPLAY_LOAD_EXTENSION_FUNCTION(PFNGLDELETEBUFFERSARBPROC, glDeleteBuffersARB);
      orxDISPLAY_LOAD_EXTENSION_FUNCTION(PFNGLBINDBUFFERARBPROC, glBindBufferARB);
      orxDISPLAY_LOAD_EXTENSION_FUNCTION(PFNGLBUFFERDATAARBPROC, glBufferDataARB);
      orxDISPLAY_LOAD_EXTENSION_FUNCTION(PFNGLBUFFERSUBDATAARBPROC, glBufferSubDataARB);

  #endif /* !__orxMAC__ */

      /* Updates status flags */
      orxFLAG_SET(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_VBO, orxDISPLAY_KU32_STATIC_FLAG_NONE);
    }
    else
    {
      /* Updates status flags */
      orxFLAG_SET(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_NONE, orxDISPLAY_KU32_STATIC_FLAG_VBO);
    }

#endif /* __orxDISPLAY_OPENGL_ES__ */

#ifdef __orxDISPLAY_OPENGL_ES__

    {
      orxFLOAT fShaderVersion;

      /* Gets supported GLSL version */
      if((orxString_ToFloat((const orxSTRING)glGetString(GL_SHADING_LANGUAGE_VERSION), &fShaderVersion, orxNULL) != orxSTATUS_FAILURE) && (fShaderVersion + orxMATH_KF_EPSILON >= orx2F(1.1f)))
      {
        /* Pushes config section */
        orxConfig_PushSection(orxDISPLAY_KZ_CONFIG_SECTION);

        /* Doesn't have a shader version? */
        if(orxConfig_HasValue(orxDISPLAY_KZ_CONFIG_SHADER_VERSION) == orxFALSE)
        {
          /* Stores it */
          orxConfig_SetU32(orxDISPLAY_KZ_CONFIG_SHADER_VERSION, orxF2U(orxMath_Round(orx2F(100.0f) * fShaderVersion)));
        }

        /* Pops config section */
        orxConfig_PopSection();
      }

      /* Gets max texture unit number */
      glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &(sstDisplay.iTextureUnitNumber));
      sstDisplay.iTextureUnitNumber = orxMIN(sstDisplay.iTextureUnitNumber, orxDISPLAY_KU32_MAX_TEXTURE_UNIT_NUMBER);
      glASSERT();

      /* Updates status flags */
      orxFLAG_SET(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_SHADER, orxDISPLAY_KU32_STATIC_FLAG_NONE);
    }

#else /* __orxDISPLAY_OPENGL_ES__ */

    /* Can support shader? */
    if(orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_VBO)
    && (glfwExtensionSupported("GL_ARB_shader_objects") != GLFW_FALSE)
    && (glfwExtensionSupported("GL_ARB_shading_language_100") != GLFW_FALSE)
    && (glfwExtensionSupported("GL_ARB_vertex_shader") != GLFW_FALSE)
    && (glfwExtensionSupported("GL_ARB_fragment_shader") != GLFW_FALSE))
    {
      orxFLOAT fShaderVersion;

  #ifndef __orxMAC__

      /* Loads related OpenGL extension functions */
      orxDISPLAY_LOAD_EXTENSION_FUNCTION(PFNGLCREATEPROGRAMOBJECTARBPROC, glCreateProgramObjectARB);
      orxDISPLAY_LOAD_EXTENSION_FUNCTION(PFNGLCREATESHADEROBJECTARBPROC, glCreateShaderObjectARB);
      orxDISPLAY_LOAD_EXTENSION_FUNCTION(PFNGLDELETEOBJECTARBPROC, glDeleteObjectARB);
      orxDISPLAY_LOAD_EXTENSION_FUNCTION(PFNGLSHADERSOURCEARBPROC, glShaderSourceARB);
      orxDISPLAY_LOAD_EXTENSION_FUNCTION(PFNGLCOMPILESHADERARBPROC, glCompileShaderARB);
      orxDISPLAY_LOAD_EXTENSION_FUNCTION(PFNGLATTACHOBJECTARBPROC, glAttachObjectARB);
      orxDISPLAY_LOAD_EXTENSION_FUNCTION(PFNGLLINKPROGRAMARBPROC, glLinkProgramARB);
      orxDISPLAY_LOAD_EXTENSION_FUNCTION(PFNGLGETOBJECTPARAMETERIVARBPROC, glGetObjectParameterivARB);
      orxDISPLAY_LOAD_EXTENSION_FUNCTION(PFNGLGETINFOLOGARBPROC, glGetInfoLogARB);
      orxDISPLAY_LOAD_EXTENSION_FUNCTION(PFNGLUSEPROGRAMOBJECTARBPROC, glUseProgramObjectARB);
      orxDISPLAY_LOAD_EXTENSION_FUNCTION(PFNGLGETUNIFORMLOCATIONARBPROC, glGetUniformLocationARB);
      orxDISPLAY_LOAD_EXTENSION_FUNCTION(PFNGLBINDATTRIBLOCATIONARBPROC, glBindAttribLocationARB);
      orxDISPLAY_LOAD_EXTENSION_FUNCTION(PFNGLENABLEVERTEXATTRIBARRAYARBPROC, glEnableVertexAttribArrayARB);
      orxDISPLAY_LOAD_EXTENSION_FUNCTION(PFNGLVERTEXATTRIBPOINTERARBPROC, glVertexAttribPointerARB);
      orxDISPLAY_LOAD_EXTENSION_FUNCTION(PFNGLUNIFORM1FARBPROC, glUniform1fARB);
      orxDISPLAY_LOAD_EXTENSION_FUNCTION(PFNGLUNIFORM3FARBPROC, glUniform3fARB);
      orxDISPLAY_LOAD_EXTENSION_FUNCTION(PFNGLUNIFORM1IARBPROC, glUniform1iARB);
      orxDISPLAY_LOAD_EXTENSION_FUNCTION(PFNGLUNIFORMMATRIX4FVARBPROC, glUniformMatrix4fvARB);

    #ifndef __orxLINUX__

      orxDISPLAY_LOAD_EXTENSION_FUNCTION(PFNGLACTIVETEXTUREARBPROC, glActiveTextureARB);

    #endif /* !__orxLINUX__ */

  #endif /* !__orxMAC__ */

      /* Gets supported GLSL version */
      if((orxString_ToFloat((const orxSTRING)glGetString(GL_SHADING_LANGUAGE_VERSION), &fShaderVersion, orxNULL) != orxSTATUS_FAILURE) && (fShaderVersion + orxMATH_KF_EPSILON >= orx2F(1.1f)))
      {
        /* Pushes config section */
        orxConfig_PushSection(orxDISPLAY_KZ_CONFIG_SECTION);

        /* Doesn't have a shader version? */
        if(orxConfig_HasValue(orxDISPLAY_KZ_CONFIG_SHADER_VERSION) == orxFALSE)
        {
          /* Stores it */
          orxConfig_SetU32(orxDISPLAY_KZ_CONFIG_SHADER_VERSION, orxF2U(orxMath_Round(orx2F(100.0f) * fShaderVersion)));
        }

        /* Pops config section */
        orxConfig_PopSection();
      }

      /* Gets max texture unit number */
      glGetIntegerv(GL_MAX_TEXTURE_COORDS, &(sstDisplay.iTextureUnitNumber));
      sstDisplay.iTextureUnitNumber = orxMIN(sstDisplay.iTextureUnitNumber, orxDISPLAY_KU32_MAX_TEXTURE_UNIT_NUMBER);
      glASSERT();

      /* Updates status flags */
      orxFLAG_SET(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_SHADER, orxDISPLAY_KU32_STATIC_FLAG_NONE);
    }
    else
    {
      /* Updates texture unit number */
      sstDisplay.iTextureUnitNumber = 1;

      /* Updates status flags */
      orxFLAG_SET(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_NONE, orxDISPLAY_KU32_STATIC_FLAG_SHADER);
    }

#ifndef __orxMAC__
    /* Has debug output support? */
    if(glfwExtensionSupported("GL_ARB_debug_output") != GLFW_FALSE)
    {
      /* Loads it */
      orxDISPLAY_LOAD_EXTENSION_FUNCTION(PFNGLDEBUGMESSAGECALLBACKARBPROC, glDebugMessageCallback);

      /* Updates status flags */
      orxFLAG_SET(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_DEBUG_OUTPUT, orxDISPLAY_KU32_STATIC_FLAG_NONE);
    }
    else
#endif /* __orxMAC__ */
    {
      /* Updates status flags */
      orxFLAG_SET(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_NONE, orxDISPLAY_KU32_STATIC_FLAG_DEBUG_OUTPUT);
    }

#endif /* __orxDISPLAY_OPENGL_ES__ */

    /* Swap Control Tear extension? */
#if !defined(__orxDISPLAY_OPENGL_ES__)
  #if defined(__orxWINDOWS__) || defined(__orxLINUX__)
    #ifdef __orxWINDOWS__

    if(glfwExtensionSupported("WGL_EXT_swap_control_tear") != GLFW_FALSE)

    #else /* __orxWINDOWS__ */

    if(glfwExtensionSupported("GLX_EXT_swap_control_tear") != GLFW_FALSE)

    #endif /* __orxWINDOWS__ */

    {
      /* Updates status flags */
      orxFLAG_SET(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_CONTROL_TEAR, orxDISPLAY_KU32_STATIC_FLAG_NONE);
    }
    else

  #endif /* __orxWINDOWS__ || __orxLINUX__ */
#endif /* __orxDISPLAY_OPENGL_ES__ */

    {
      /* Updates status flags */
      orxFLAG_SET(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_NONE, orxDISPLAY_KU32_STATIC_FLAG_CONTROL_TEAR);
    }

#ifdef GL_COMPRESSED_RGBA_BPTC_UNORM
    /* Has BC7 support? */
    if(glfwExtensionSupported("GL_ARB_texture_compression_bptc") != GLFW_FALSE)
    {
      /* Selects format */
      sstDisplay.eBasisUFormat = BasisUFormat_BC7;
    }
    else
#endif /* GL_COMPRESSED_RGBA_BPTC_UNORM */
#ifdef GL_COMPRESSED_RGBA_ASTC_4x4_KHR
    /* Has ASTC support? */
    if(glfwExtensionSupported("GL_KHR_texture_compression_astc_ldr") != GLFW_FALSE)
    {
      /* Selects format */
      sstDisplay.eBasisUFormat = BasisUFormat_ASTC;
    }
    else
#endif /* GL_COMPRESSED_RGBA_ASTC_4x4_KHR */
    /* Defaults to uncompressed */
    {
      /* Selects format */
      sstDisplay.eBasisUFormat = BasisUFormat_Uncompressed;
    }

#if !defined(__orxDISPLAY_OPENGL_ES__) && !defined(__orxLINUX__) && !defined(__orxMAC__)
    /* Has texture compression support? */
    if(sstDisplay.eBasisUFormat != BasisUFormat_Uncompressed)
    {
      /* Loads related extension function */
      orxDISPLAY_LOAD_EXTENSION_FUNCTION(PFNGLCOMPRESSEDTEXIMAGE2DPROC, glCompressedTexImage2D);
    }
#endif /* !__orxDISPLAY_OPENGL_ES__ && !__orxLINUX__ && !__orxMAC */

    /* Updates status flags */
    orxFLAG_SET(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_EXT_READY, orxDISPLAY_KU32_STATIC_FLAG_NONE);
  }

  /* Pushes config section */
  orxConfig_PushSection(orxDISPLAY_KZ_CONFIG_SECTION);

  /* Stores texture units, draw buffer numbers & max texture size */
  orxConfig_SetU32(orxDISPLAY_KZ_CONFIG_TEXTURE_UNIT_NUMBER, (orxU32)sstDisplay.iTextureUnitNumber);
  orxConfig_SetU32(orxDISPLAY_KZ_CONFIG_DRAW_BUFFER_NUMBER, (orxU32)sstDisplay.iDrawBufferNumber);
  orxConfig_SetU32(orxDISPLAY_KZ_CONFIG_MAX_TEXTURE_SIZE, (orxU32)sstDisplay.iMaxTextureSize);

  /* Pops config section */
  orxConfig_PopSection();

#undef orxDISPLAY_LOAD_EXTENSION_FUNCTION

  /* Done! */
  return;
}

static int orxDisplay_GLFW_ReadSTBICallback(void *_hResource, char *_pBuffer, int _iSize)
{
  /* Reads data */
  return (int)orxResource_Read((orxHANDLE)_hResource, _iSize, (orxU8 *)_pBuffer, orxNULL, orxNULL);
}

static void orxDisplay_GLFW_SkipSTBICallback(void *_hResource, int _iOffset)
{
  /* Seeks offset */
  orxResource_Seek((orxHANDLE)_hResource, _iOffset, orxSEEK_OFFSET_WHENCE_CURRENT);

  /* Done! */
  return;
}

static int orxDisplay_GLFW_EOFSTBICallback(void *_hResource)
{
  /* End of buffer? */
  return (orxResource_Tell((orxHANDLE)_hResource) == orxResource_GetSize(_hResource)) ? 1 : 0;
}

static orxSTATUS orxFASTCALL orxDisplay_GLFW_DecompressBitmapCallback(void *_pContext)
{
  orxDISPLAY_LOAD_INFO *pstInfo;
  orxSTATUS             eResult = orxSTATUS_SUCCESS;

  /* Gets load info */
  pstInfo = (orxDISPLAY_LOAD_INFO *)_pContext;

  /* Hasn't exited yet? */
  if(sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY)
  {
    /* Cursor? */
    if(orxFLAG_TEST(pstInfo->pstBitmap->u32Flags, orxDISPLAY_KU32_BITMAP_FLAG_CURSOR))
    {
      /* Successful? */
      if(pstInfo->pu8ImageBuffer != orxNULL)
      {
        GLFWimage stImage;

        /* Has cursor? */
        if(sstDisplay.pstCursor != NULL)
        {
          /* Deletes it */
          glfwDestroyCursor(sstDisplay.pstCursor);
        }

        /* Inits cursor image */
        stImage.width   = (int)(pstInfo->uiWidth);
        stImage.height  = (int)(pstInfo->uiHeight);
        stImage.pixels  = (unsigned char *)pstInfo->pu8ImageBuffer;

        /* Creates cursor */
        sstDisplay.pstCursor = glfwCreateCursor(&stImage, (int)(pstInfo->pstBitmap->fWidth), (int)(pstInfo->pstBitmap->fHeight));

        /* Success? */
        if(sstDisplay.pstCursor != NULL)
        {
          /* Sets it */
          glfwSetCursor(sstDisplay.pstWindow, sstDisplay.pstCursor);
        }

        /* Frees data */
        orxMemory_Free(pstInfo->pu8ImageBuffer);
      }

      /* Clears loading flag */
      orxFLAG_SET(pstInfo->pstBitmap->u32Flags, orxDISPLAY_KU32_BITMAP_FLAG_NONE, orxDISPLAY_KU32_BITMAP_FLAG_LOADING);

      /* Deletes bitmap */
      orxDisplay_DeleteBitmap(pstInfo->pstBitmap);
    }
    /* Icon? */
    else if(orxFLAG_TEST(pstInfo->pstBitmap->u32Flags, orxDISPLAY_KU32_BITMAP_FLAG_ICON))
    {
      /* Successful? */
      if(pstInfo->pu8ImageBuffer != orxNULL)
      {
        /* Adds icon image */
        sstDisplay.astIconList[sstDisplay.s32IconNumber].width  = (int)(pstInfo->uiWidth);
        sstDisplay.astIconList[sstDisplay.s32IconNumber].height = (int)(pstInfo->uiHeight);
        sstDisplay.astIconList[sstDisplay.s32IconNumber].pixels = (unsigned char *)pstInfo->pu8ImageBuffer;
        sstDisplay.s32IconNumber++;
      }

      /* Clears loading flag */
      orxFLAG_SET(pstInfo->pstBitmap->u32Flags, orxDISPLAY_KU32_BITMAP_FLAG_NONE, orxDISPLAY_KU32_BITMAP_FLAG_LOADING);

      /* Last icon? */
      if(sstDisplay.s32PendingIconCount == 1)
      {
        orxS32 i;

        /* Has icons? */
        if(sstDisplay.s32IconNumber > 0)
        {
          /* Sets window icons */
          glfwSetWindowIcon(sstDisplay.pstWindow, (int)sstDisplay.s32IconNumber, sstDisplay.astIconList);
        }

        /* For all icons */
        for(i = 0; i < sstDisplay.s32IconNumber; i++)
        {
          /* Frees its data */
          orxMemory_Free(sstDisplay.astIconList[i].pixels);

          /* Clears it */
          orxMemory_Zero(&(sstDisplay.astIconList[i]), sizeof(GLFWimage));
        }

        /* Resets icon list */
        sstDisplay.s32IconNumber = 0;
      }

      /* Deletes bitmap */
      orxDisplay_DeleteBitmap(pstInfo->pstBitmap);

      /* Updates pending icon count */
      sstDisplay.s32PendingIconCount--;
    }
    /* Texture */
    else
    {
      orxDISPLAY_EVENT_PAYLOAD  stPayload;
      orxU32                    i;

      /* Inits bitmap */
      pstInfo->pstBitmap->fWidth         = orxU2F(pstInfo->uiWidth);
      pstInfo->pstBitmap->fHeight        = orxU2F(pstInfo->uiHeight);
      pstInfo->pstBitmap->u32RealWidth   = (orxU32)pstInfo->uiRealWidth;
      pstInfo->pstBitmap->u32RealHeight  = (orxU32)pstInfo->uiRealHeight;
      pstInfo->pstBitmap->u32Depth       = 32;
      pstInfo->pstBitmap->fRecRealWidth  = orxFLOAT_1 / orxU2F(pstInfo->pstBitmap->u32RealWidth);
      pstInfo->pstBitmap->fRecRealHeight = orxFLOAT_1 / orxU2F(pstInfo->pstBitmap->u32RealHeight);
      pstInfo->pstBitmap->u32DataSize    = pstInfo->u32DataSize;
      orxVector_Copy(&(pstInfo->pstBitmap->stClip.vTL), &orxVECTOR_0);
      orxVector_Set(&(pstInfo->pstBitmap->stClip.vBR), pstInfo->pstBitmap->fWidth, pstInfo->pstBitmap->fHeight, orxFLOAT_0);

      /* Tracks video memory */
      orxMEMORY_TRACK(VIDEO, pstInfo->pstBitmap->u32DataSize, orxTRUE);

      /* Creates new texture */
      glGenTextures(1, &(pstInfo->pstBitmap->uiTexture));
      glASSERT();
      glBindTexture(GL_TEXTURE_2D, pstInfo->pstBitmap->uiTexture);
      glASSERT();
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glASSERT();
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glASSERT();
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (pstInfo->pstBitmap->bSmoothing != orxFALSE) ? GL_LINEAR : GL_NEAREST);
      glASSERT();
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (pstInfo->pstBitmap->bSmoothing != orxFALSE) ? GL_LINEAR : GL_NEAREST);
      glASSERT();

      /* Compressed Basis Universal? */
      if((pstInfo->bIsBasisU != orxFALSE) && (sstDisplay.eBasisUFormat != BasisUFormat_Uncompressed))
      {
        GLenum eInternalFormat;

        /* Depending on Basis Universal format */
        switch(sstDisplay.eBasisUFormat)
        {
#ifdef GL_COMPRESSED_RGBA_ASTC_4x4_KHR
          case BasisUFormat_ASTC:
          {
            /* Gets internal format */
            eInternalFormat = GL_COMPRESSED_RGBA_ASTC_4x4_KHR;
            break;
          }
#endif /* GL_COMPRESSED_RGBA_ASTC_4x4_KHR */
#ifdef GL_COMPRESSED_RGBA_BPTC_UNORM
          case BasisUFormat_BC7:
          {
            /* Gets internal format */
            eInternalFormat = GL_COMPRESSED_RGBA_BPTC_UNORM;
            break;
          }
#endif /* GL_COMPRESSED_RGBA_BPTC_UNORM */
          default:
          {
            /* Logs message */
            orxASSERT(orxFALSE && "Invalid Basis Universal format [%u] for this platform, this should *not* happen.", sstDisplay.eBasisUFormat);
            break;
          }
        }

        /* Loads compressed data */
        glCompressedTexImage2D(GL_TEXTURE_2D, 0, eInternalFormat, (GLsizei)pstInfo->pstBitmap->u32RealWidth, (GLsizei)pstInfo->pstBitmap->u32RealHeight, 0, (GLsizei)pstInfo->pstBitmap->u32DataSize, pstInfo->pu8ImageBuffer);
      }
      else
      {
        /* Loads data */
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)pstInfo->pstBitmap->u32RealWidth, (GLsizei)pstInfo->pstBitmap->u32RealHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, (pstInfo->pu8ImageBuffer != orxNULL) ? pstInfo->pu8ImageBuffer : NULL);
      }
      glASSERT();

      /* Restores previous texture */
      glBindTexture(GL_TEXTURE_2D, (sstDisplay.apstBoundBitmapList[sstDisplay.s32ActiveTextureUnit] != orxNULL) ? sstDisplay.apstBoundBitmapList[sstDisplay.s32ActiveTextureUnit]->uiTexture : 0);
      glASSERT();

      /* For all bound bitmaps */
      for(i = 0; i < (orxU32)sstDisplay.iTextureUnitNumber; i++)
      {
        /* Is decompressed bitmap? */
        if(sstDisplay.apstBoundBitmapList[i] == pstInfo->pstBitmap)
        {
          /* Resets it */
          sstDisplay.apstBoundBitmapList[i] = orxNULL;
          sstDisplay.adMRUBitmapList[i]     = orxDOUBLE_0;
        }
      }

      /* Asynchronous call? */
      if(orxFLAG_TEST(pstInfo->pstBitmap->u32Flags, orxDISPLAY_KU32_BITMAP_FLAG_LOADING))
      {
        /* Failed decompression? */
        if(pstInfo->pu8ImageBuffer == orxNULL)
        {
          /* Logs message */
          orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Couldn't process data for bitmap <%s>: an empty texture will be used instead.", pstInfo->pstBitmap->zLocation);
        }
      }

      /* Inits payload */
      stPayload.stBitmap.zLocation      = pstInfo->pstBitmap->zLocation;
      stPayload.stBitmap.stFilenameID   = pstInfo->pstBitmap->stFilenameID;
      stPayload.stBitmap.u32ID          = (pstInfo->pu8ImageBuffer != orxNULL) ? (orxU32)pstInfo->pstBitmap->uiTexture : orxU32_UNDEFINED;

      /* Frees image buffer */
      if(pstInfo->pu8ImageBuffer != pstInfo->pu8ImageSource)
      {
        orxMemory_Free(pstInfo->pu8ImageBuffer);
      }
      pstInfo->pu8ImageBuffer = orxNULL;

      /* Frees source */
      if(pstInfo->pu8ImageSource != orxNULL)
      {
        orxMemory_Free(pstInfo->pu8ImageSource);
        pstInfo->pu8ImageSource = orxNULL;
      }

      /* Clears loading flag */
      orxFLAG_SET(pstInfo->pstBitmap->u32Flags, orxDISPLAY_KU32_BITMAP_FLAG_NONE, orxDISPLAY_KU32_BITMAP_FLAG_LOADING);
      orxMEMORY_BARRIER();

      /* Sends event */
      orxEVENT_SEND(orxEVENT_TYPE_DISPLAY, orxDISPLAY_EVENT_LOAD_BITMAP, pstInfo->pstBitmap, orxNULL, &stPayload);

      /* Asked for deletion? */
      if(orxFLAG_TEST(pstInfo->pstBitmap->u32Flags, orxDISPLAY_KU32_BITMAP_FLAG_DELETE))
      {
        /* Deletes it */
        orxDisplay_DeleteBitmap(pstInfo->pstBitmap);
      }
    }
  }
  else
  {
    /* Frees image buffer */
    if(pstInfo->pu8ImageBuffer != pstInfo->pu8ImageSource)
    {
      orxMemory_Free(pstInfo->pu8ImageBuffer);
    }
    pstInfo->pu8ImageBuffer = orxNULL;

    /* Frees source */
    if(pstInfo->pu8ImageSource != orxNULL)
    {
      orxMemory_Free(pstInfo->pu8ImageSource);
      pstInfo->pu8ImageSource = orxNULL;
    }
  }

  /* Frees load info */
  orxMemory_Free(pstInfo);

  /* Done! */
  return eResult;
}

static orxSTATUS orxFASTCALL orxDisplay_GLFW_DecompressBitmap(void *_pContext)
{
  orxDISPLAY_LOAD_INFO *pstInfo;
  orxSTATUS             eResult;

  /* Profiles */
  orxPROFILER_PUSH_MARKER("orxDisplay_DecompressBitmap");

  /* Gets load info */
  pstInfo = (orxDISPLAY_LOAD_INFO *)_pContext;

  /* Hasn't exited yet? */
  if(sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY)
  {
    unsigned char  *pu8ImageData = orxNULL;
    unsigned int    uiDataSize;
    int             iIndex = 0;
    BasisUFormat    eFormat;

    /* Gets format based on image type */
    eFormat = orxFLAG_TEST(pstInfo->pstBitmap->u32Flags, orxDISPLAY_KU32_BITMAP_FLAG_CURSOR | orxDISPLAY_KU32_BITMAP_FLAG_ICON) ? BasisUFormat_Uncompressed : sstDisplay.eBasisUFormat;

    /* Is Basis Universal texture? */
    if((uiDataSize = BasisU_GetInfo(pstInfo->pu8ImageSource, (unsigned int)pstInfo->s64Size, eFormat, &(pstInfo->uiWidth), &(pstInfo->uiHeight), (unsigned int *)&(pstInfo->u32DataSize))) != 0)
    {
      /* Allocates image data */
      pu8ImageData = (orxU8 *)orxMemory_Allocate(uiDataSize, orxMEMORY_TYPE_VIDEO);

      /* Valid? */
      if(pu8ImageData != orxNULL)
      {
        /* Transcode it? */
        if(BasisU_Transcode(pstInfo->pu8ImageSource, (unsigned int)pstInfo->s64Size, eFormat, pu8ImageData, uiDataSize) != 0)
        {
          /* Updates its status */
          pstInfo->bIsBasisU = orxTRUE;
        }
        else
        {
          /* Frees image data */
          orxMemory_Free(pu8ImageData);
          pu8ImageData = orxNULL;
        }
      }
    }
    /* Is QOI? */
    else if((qoi_read_32(pstInfo->pu8ImageSource, &iIndex) == QOI_MAGIC))
    {
      qoi_desc stDesc;

      /* Decodes it */
      pu8ImageData = (unsigned char *)qoi_decode(pstInfo->pu8ImageSource, (int)pstInfo->s64Size, &stDesc, 4);

      /* Valid? */
      if(pu8ImageData != NULL)
      {
        /* Updates info */
        pstInfo->uiWidth      = stDesc.width;
        pstInfo->uiHeight     = stDesc.height;
        pstInfo->u32DataSize  = 4 * stDesc.width * stDesc.height;
      }
    }
    else
    {
      GLuint uiBytesPerPixel;

      /* Loads image */
      pu8ImageData = stbi_load_from_memory((unsigned char *)pstInfo->pu8ImageSource, (int)pstInfo->s64Size, (int *)&(pstInfo->uiWidth), (int *)&(pstInfo->uiHeight), (int *)&uiBytesPerPixel, STBI_rgb_alpha);

      /* Valid? */
      if(pu8ImageData != orxNULL)
      {
        /* Updates info size */
        pstInfo->u32DataSize = 4 * pstInfo->uiWidth * pstInfo->uiHeight;
      }
    }

    /* Valid? */
    if(pu8ImageData != NULL)
    {
      /* Has NPOT texture support, is a Basis Universal compressed texture or cursor/icon? */
      if((orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_NPOT))
      || (eFormat != BasisUFormat_Uncompressed)
      || (orxFLAG_TEST(pstInfo->pstBitmap->u32Flags, orxDISPLAY_KU32_BITMAP_FLAG_CURSOR | orxDISPLAY_KU32_BITMAP_FLAG_ICON)))
      {
        /* Uses image buffer */
        pstInfo->pu8ImageBuffer = pu8ImageData;

        /* Gets real size */
        pstInfo->uiRealWidth  = pstInfo->uiWidth;
        pstInfo->uiRealHeight = pstInfo->uiHeight;
      }
      else
      {
        GLuint i, uiSrcOffset, uiDstOffset, uiLineSize, uiRealLineSize;

        /* Gets real size */
        pstInfo->uiRealWidth  = (GLuint)orxMath_GetNextPowerOfTwo(pstInfo->uiWidth);
        pstInfo->uiRealHeight = (GLuint)orxMath_GetNextPowerOfTwo(pstInfo->uiHeight);

        /* Allocates buffer */
        pstInfo->pu8ImageBuffer = (orxU8 *)orxMemory_Allocate(pstInfo->uiRealWidth * pstInfo->uiRealHeight * 4 * sizeof(orxU8), orxMEMORY_TYPE_TEMP);

        /* Checks */
        orxASSERT(pstInfo->pu8ImageBuffer != orxNULL);

        /* Gets line sizes */
        uiLineSize      = pstInfo->uiWidth * 4 * sizeof(orxU8);
        uiRealLineSize  = pstInfo->uiRealWidth * 4 * sizeof(orxU8);

        /* Clears padding */
        orxMemory_Zero(pstInfo->pu8ImageBuffer, uiRealLineSize * (pstInfo->uiRealHeight - pstInfo->uiHeight));

        /* For all lines */
        for(i = 0, uiSrcOffset = 0, uiDstOffset = 0;
            i < pstInfo->uiHeight;
            i++, uiSrcOffset += uiLineSize, uiDstOffset += uiRealLineSize)
        {
          /* Copies data */
          orxMemory_Copy(pstInfo->pu8ImageBuffer + uiDstOffset, pu8ImageData + uiSrcOffset, uiLineSize);

          /* Adds padding */
          orxMemory_Zero(pstInfo->pu8ImageBuffer + uiDstOffset + uiLineSize, uiRealLineSize - uiLineSize);
        }
      }

      /* Frees original source from resource */
      orxMemory_Free(pstInfo->pu8ImageSource);

      /* Stores uncompressed data as new source */
      pstInfo->pu8ImageSource = pu8ImageData;
    }
    else
    {
      /* Clears info */
      pstInfo->uiWidth      =
      pstInfo->uiHeight     =
      pstInfo->uiRealWidth  =
      pstInfo->uiRealHeight = 1;

      /* Frees original source from resource */
      orxMemory_Free(pstInfo->pu8ImageSource);
      pstInfo->pu8ImageSource = orxNULL;
    }

    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }
  else
  {
    /* Frees original source from resource */
    orxMemory_Free(pstInfo->pu8ImageSource);
    pstInfo->pu8ImageSource = orxNULL;

    /* Frees load info */
    orxMemory_Free(pstInfo);

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Profiles */
  orxPROFILER_POP_MARKER();

  /* Done! */
  return eResult;
}

static void orxFASTCALL orxDisplay_GLFW_ReadResourceCallback(orxHANDLE _hResource, orxS64 _s64Size, void *_pBuffer, void *_pContext)
{
  orxDISPLAY_LOAD_INFO *pstInfo;

  /* Allocates load info */
  pstInfo = (orxDISPLAY_LOAD_INFO *)orxMemory_Allocate(sizeof(orxDISPLAY_LOAD_INFO), orxMEMORY_TYPE_TEMP);

  /* Checks */
  orxASSERT(pstInfo != orxNULL);

  /* Inits it */
  orxMemory_Zero(pstInfo, sizeof(orxDISPLAY_LOAD_INFO));
  pstInfo->pu8ImageSource = (orxU8 *)_pBuffer;
  pstInfo->s64Size        = _s64Size;
  pstInfo->pstBitmap      = (orxBITMAP *)_pContext;

  /* Asynchronous? */
  if(orxFLAG_TEST(pstInfo->pstBitmap->u32Flags, orxDISPLAY_KU32_BITMAP_FLAG_LOADING))
  {
    /* Runs asynchronous task */
    if(orxThread_RunTask(&orxDisplay_GLFW_DecompressBitmap, orxDisplay_GLFW_DecompressBitmapCallback, orxNULL, (void *)pstInfo) == orxSTATUS_FAILURE)
    {
      /* Frees load info */
      orxMemory_Free(pstInfo);
    }
  }
  else
  {
    /* Decompresses bitmap */
    if(orxDisplay_GLFW_DecompressBitmap(pstInfo) != orxSTATUS_FAILURE)
    {
      /* Upload texture */
      orxDisplay_GLFW_DecompressBitmapCallback(pstInfo);
    }
  }

  /* Closes resource */
  orxResource_Close(_hResource);
}

static void orxDisplay_GLFW_WriteResourceCallback(void *_pContext, void *_pData, int _iSize)
{
  /* Writes resource synchronously */
  orxResource_Write((orxHANDLE)_pContext, (orxS64)_iSize, _pData, orxNULL, orxNULL);
}


static orxSTATUS orxFASTCALL orxDisplay_GLFW_SaveBitmapData(void *_pContext)
{
  orxDISPLAY_SAVE_INFO *pstInfo;
  const orxSTRING       zExtension;
  orxSTATUS             eResult = orxSTATUS_FAILURE;

  /* Gets save info */
  pstInfo = (orxDISPLAY_SAVE_INFO *)_pContext;

  /* Gets extension */
  zExtension = orxString_GetExtension(orxResource_GetLocation(pstInfo->hResource));

  /* PNG? */
  if(orxString_ICompare(zExtension, "png") == 0)
  {
    /* Saves image to disk */
    eResult = stbi_write_png_to_func(&orxDisplay_GLFW_WriteResourceCallback, pstInfo->hResource, pstInfo->u32Width, pstInfo->u32Height, 4, pstInfo->pu8ImageData, 0) != 0 ? orxSTATUS_SUCCESS : orxSTATUS_FAILURE;
  }
  /* QOI? */
  else if(orxString_ICompare(zExtension, "qoi") == 0)
  {
    qoi_desc  stDesc;
    int       iSize;
    void     *pBuffer;

    /* Inits descriptor */
    orxMemory_Zero(&stDesc, sizeof(qoi_desc));
    stDesc.width      = pstInfo->u32Width;
    stDesc.height     = pstInfo->u32Height;
    stDesc.channels   = 4;
    stDesc.colorspace = 1;

    /* Encodes it */
    pBuffer = qoi_encode(pstInfo->pu8ImageData, &stDesc, &iSize);

    /* Success? */
    if(pBuffer != NULL)
    {
      /* Saves image to disk */
      if(orxResource_Write(pstInfo->hResource, (orxS64)iSize, pBuffer, orxNULL, orxNULL) == (orxS64)iSize)
      {
        /* Updates result */
        eResult = orxSTATUS_SUCCESS;
      }

      /* Deletes buffer */
      orxMemory_Free(pBuffer);
    }
  }
  /* JPG? */
  else if((orxString_ICompare(zExtension, "jpg") == 0) || (orxString_ICompare(zExtension, "jpeg") == 0))
  {
    /* Saves image to disk */
    eResult = stbi_write_jpg_to_func(&orxDisplay_GLFW_WriteResourceCallback, pstInfo->hResource, pstInfo->u32Width, pstInfo->u32Height, 4, pstInfo->pu8ImageData, 0) != 0 ? orxSTATUS_SUCCESS : orxSTATUS_FAILURE;
  }
  /* BMP? */
  else if(orxString_ICompare(zExtension, "bmp") == 0)
  {
    /* Saves image to disk */
    eResult = stbi_write_bmp_to_func(&orxDisplay_GLFW_WriteResourceCallback, pstInfo->hResource, pstInfo->u32Width, pstInfo->u32Height, 4, pstInfo->pu8ImageData) != 0 ? orxSTATUS_SUCCESS : orxSTATUS_FAILURE;
  }
  /* TGA */
  else
  {
    /* Saves image to disk */
    eResult = stbi_write_tga_to_func(&orxDisplay_GLFW_WriteResourceCallback, pstInfo->hResource, pstInfo->u32Width, pstInfo->u32Height, 4, pstInfo->pu8ImageData) != 0 ? orxSTATUS_SUCCESS : orxSTATUS_FAILURE;
  }

  /* Closes resource */
  orxResource_Close(pstInfo->hResource);

  /* Deletes data */
  orxMemory_Free(pstInfo->pu8ImageData);

  /* Deletes save info */
  orxMemory_Free(pstInfo);

  /* Done! */
  return eResult;
}


static orxSTATUS orxFASTCALL orxDisplay_GLFW_LoadBitmapData(orxBITMAP *_pstBitmap)
{
  orxHANDLE hResource;
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Opens resource */
  hResource = orxResource_Open(_pstBitmap->zLocation, orxFALSE);

  /* Success? */
  if(hResource != orxHANDLE_UNDEFINED)
  {
    orxS64  s64Size;
    orxU8  *pu8Buffer;

    /* Gets its size */
    s64Size = orxResource_GetSize(hResource);

    /* Checks */
    orxASSERT((s64Size > 0) && (s64Size < 0xFFFFFFFF));

    /* Allocates buffer */
    pu8Buffer = (orxU8 *)orxMemory_Allocate((orxU32)s64Size, orxMEMORY_TYPE_TEMP);

    /* Success? */
    if(pu8Buffer != orxNULL)
    {
      /* Asynchronous? */
      if(sstDisplay.pstTempBitmap != orxNULL)
      {
        orxU8        *pu8Header;
        unsigned int  uiHeaderSize;
        int           iWidth, iHeight, iDummy = 0;

        /* Retrieves header for Basis Universal & QOI */
        uiHeaderSize  = orxMAX(BasisU_GetHeaderSize(), QOI_HEADER_SIZE);
        pu8Header     = (orxU8 *)alloca(uiHeaderSize);
        orxResource_Read(hResource, uiHeaderSize, pu8Header, orxNULL, orxNULL);
        orxResource_Seek(hResource, 0, orxSEEK_OFFSET_WHENCE_START);

        /* Gets its info */
        if(((qoi_read_32(pu8Header, &iDummy) == QOI_MAGIC)
         && (iWidth   = qoi_read_32(pu8Header, &iDummy),
             iHeight  = qoi_read_32(pu8Header, &iDummy),
             iDummy   = (int)pu8Header[iDummy],
             (iDummy == 3)
          || (iDummy == 4)))
        || (BasisU_GetInfo(pu8Header, uiHeaderSize, sstDisplay.eBasisUFormat, (unsigned int *)&iWidth, (unsigned int*)&iHeight, (unsigned int*)&iDummy) != 0)
        || (stbi_info_from_callbacks(&(sstDisplay.stSTBICallbacks), (void *)hResource, &iWidth, &iHeight, &iDummy) != 0))
        {
          /* Resets resource cursor */
          orxResource_Seek(hResource, 0, orxSEEK_OFFSET_WHENCE_START);

          /* Updates asynchronous loading flag */
          orxFLAG_SET(_pstBitmap->u32Flags, orxDISPLAY_KU32_BITMAP_FLAG_LOADING, orxDISPLAY_KU32_BITMAP_FLAG_NONE);

          /* Loads data from resource */
          s64Size = orxResource_Read(hResource, s64Size, pu8Buffer, orxDisplay_GLFW_ReadResourceCallback, (void *)_pstBitmap);

          /* Successful asynchronous call? */
          if(s64Size < 0)
          {
            /* Not a cursor nor an icon? */
            if(!orxFLAG_TEST(_pstBitmap->u32Flags, orxDISPLAY_KU32_BITMAP_FLAG_CURSOR | orxDISPLAY_KU32_BITMAP_FLAG_ICON))
            {
              /* Inits bitmap info using temp */
              _pstBitmap->uiTexture       = sstDisplay.pstTempBitmap->uiTexture;
              _pstBitmap->fWidth          = orxS2F(iWidth);
              _pstBitmap->fHeight         = orxS2F(iHeight);
              _pstBitmap->u32RealWidth    = sstDisplay.pstTempBitmap->u32RealWidth;
              _pstBitmap->u32RealHeight   = sstDisplay.pstTempBitmap->u32RealHeight;
              _pstBitmap->u32Depth        = sstDisplay.pstTempBitmap->u32Depth;
              _pstBitmap->fRecRealWidth   = sstDisplay.pstTempBitmap->fRecRealWidth;
              _pstBitmap->fRecRealHeight  = sstDisplay.pstTempBitmap->fRecRealHeight;
              _pstBitmap->u32DataSize     = sstDisplay.pstTempBitmap->u32DataSize;
              orxVector_Copy(&(_pstBitmap->stClip.vTL), &(sstDisplay.pstTempBitmap->stClip.vTL));
              orxVector_Copy(&(_pstBitmap->stClip.vBR), &(sstDisplay.pstTempBitmap->stClip.vBR));
            }

            /* Updates result */
            eResult = orxSTATUS_SUCCESS;
          }
          else
          {
            /* Frees buffer */
            orxMemory_Free(pu8Buffer);

            /* Closes resource */
            orxResource_Close(hResource);
          }
        }
        else
        {
          /* Frees buffer */
          orxMemory_Free(pu8Buffer);

          /* Closes resource */
          orxResource_Close(hResource);
        }
      }
      else
      {
        /* Loads data from resource */
        s64Size = orxResource_Read(hResource, s64Size, pu8Buffer, orxNULL, orxNULL);

        /* Success? */
        if(s64Size != 0)
        {
          /* Processes data */
          orxDisplay_GLFW_ReadResourceCallback(hResource, s64Size, (void *)pu8Buffer, (void *)_pstBitmap);

          /* Updates result */
          eResult = orxSTATUS_SUCCESS;
        }
        else
        {
          /* Frees buffer */
          orxMemory_Free(pu8Buffer);

          /* Closes resource */
          orxResource_Close(hResource);
        }
      }
    }
    else
    {
      /* Closes resource */
      orxResource_Close(hResource);
    }
  }

  /* Done! */
  return eResult;
}

static void orxFASTCALL orxDisplay_GLFW_DeleteBitmapData(orxBITMAP *_pstBitmap)
{
  orxS32 i;

  /* For all bound bitmaps */
  for(i = 0; i < (orxS32)sstDisplay.iTextureUnitNumber; i++)
  {
    /* Is deleted bitmap? */
    if(sstDisplay.apstBoundBitmapList[i] == _pstBitmap)
    {
      /* Resets it */
      sstDisplay.apstBoundBitmapList[i] = orxNULL;
      sstDisplay.adMRUBitmapList[i]     = orxDOUBLE_0;
    }
  }

  /* Tracks video memory */
  orxMEMORY_TRACK(VIDEO, _pstBitmap->u32DataSize, orxFALSE);

  /* Deletes its texture */
  glDeleteTextures(1, &(_pstBitmap->uiTexture));
  glASSERT();

  /* Done! */
  return;
}

static orxSTATUS orxFASTCALL orxDisplay_GLFW_CompileShader(orxDISPLAY_SHADER *_pstShader)
{
  static const orxSTRING szVertexShaderSource =
#ifdef __orxDISPLAY_OPENGL_ES__
  "precision mediump float;"
#endif /* __orxDISPLAY_OPENGL_ES__ */
  "attribute vec2 _vPosition_;"
  "uniform mat4 _mProjection_;"
  "attribute vec2 _vTexCoord_;"
  "varying vec2 _gl_TexCoord0_;"
  "attribute vec4 _vColor_;"
  "varying vec4 _Color0_;"
  "void main()"
  "{"
  "  float fCoef      = 1.0 / 255.0;"
  "  gl_Position      = _mProjection_ * vec4(_vPosition_.xy, 0.0, 1.0);"
  "  _gl_TexCoord0_   = _vTexCoord_;"
  "  _Color0_         = fCoef * _vColor_;"
  "}";

  GLhandleARB hProgram, hVertexShader, hFragmentShader;
  GLint       iSuccess;
  orxSTATUS   eResult = orxSTATUS_FAILURE;

  /* Creates program */
  hProgram = glCreateProgramObjectARB();
  glASSERT();

  /* Creates vertex and fragment shaders */
  hVertexShader   = glCreateShaderObjectARB(GL_VERTEX_SHADER);
  glASSERT();
  hFragmentShader = glCreateShaderObjectARB(GL_FRAGMENT_SHADER);
  glASSERT();

  /* Compiles shader objects */
  glShaderSourceARB(hVertexShader, 1, (const GLchar **)&szVertexShaderSource, NULL);
  glASSERT();
  glShaderSourceARB(hFragmentShader, 1, (const GLchar **)&(_pstShader->zCode), NULL);
  glASSERT();
  glCompileShaderARB(hVertexShader);
  glASSERT();
  glCompileShaderARB(hFragmentShader);
  glASSERT();

  /* Gets vertex shader compiling status */
  glGetObjectParameterivARB(hVertexShader, GL_OBJECT_COMPILE_STATUS_ARB, &iSuccess);
  glASSERT();

  /* Success? */
  if(iSuccess != GL_FALSE)
  {
    /* Gets fragment shader compiling status */
    glGetObjectParameterivARB(hFragmentShader, GL_OBJECT_COMPILE_STATUS_ARB, &iSuccess);
    glASSERT();

    /* Success? */
    if(iSuccess != GL_FALSE)
    {
      /* Attaches shader objects to program */
      glAttachObjectARB(hProgram, hVertexShader);
      glASSERT();
      glAttachObjectARB(hProgram, hFragmentShader);
      glASSERT();

      /* Deletes shader objects */
      glDeleteObjectARB(hVertexShader);
      glASSERT();
      glDeleteObjectARB(hFragmentShader);
      glASSERT();

      /* Binds attributes */
      glBindAttribLocationARB(hProgram, orxDISPLAY_ATTRIBUTE_LOCATION_VERTEX, "_vPosition_");
      glASSERT();
      glBindAttribLocationARB(hProgram, orxDISPLAY_ATTRIBUTE_LOCATION_TEXCOORD, "_vTexCoord_");
      glASSERT();
      glBindAttribLocationARB(hProgram, orxDISPLAY_ATTRIBUTE_LOCATION_COLOR, "_vColor_");
      glASSERT();

      /* Links program */
      glLinkProgramARB(hProgram);
      glASSERT();

      /* Gets texture location */
      _pstShader->iTextureLocation = glGetUniformLocationARB(hProgram, "orxTexture");
      glASSERT();

      /* Gets projection matrix location */
      _pstShader->iProjectionMatrixLocation = glGetUniformLocationARB(hProgram, "_mProjection_");
      glASSERT();

      /* Gets linking status */
      glGetObjectParameterivARB(hProgram, GL_OBJECT_LINK_STATUS_ARB, &iSuccess);
      glASSERT();

      /* Success? */
      if(iSuccess != GL_FALSE)
      {
        /* Updates shader */
        _pstShader->hProgram      = hProgram;
        _pstShader->iTextureCount = 0;

        /* Updates result */
        eResult = orxSTATUS_SUCCESS;
      }
      else
      {
        orxCHAR acBuffer[4096];

        /* Gets log */
        glGetInfoLogARB(hProgram, sizeof(acBuffer) - 1, NULL, (GLchar *)acBuffer);
        glASSERT();
        acBuffer[sizeof(acBuffer) - 1] = orxCHAR_NULL;

        /* Outputs log */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Couldn't link shader program:%s%s%s", orxSTRING_EOL, acBuffer, orxSTRING_EOL);

        /* Deletes program */
        glDeleteProgram(hProgram);
        glASSERT();
      }
    }
    else
    {
      orxCHAR acBuffer[4096];

      /* Gets log */
      glGetInfoLogARB(hFragmentShader, sizeof(acBuffer) - 1, NULL, (GLchar *)acBuffer);
      glASSERT();
      acBuffer[sizeof(acBuffer) - 1] = orxCHAR_NULL;

      /* Outputs log */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Couldn't compile fragment shader:%s%s%s", orxSTRING_EOL, acBuffer, orxSTRING_EOL);

      /* Deletes shader objects & program */
      glDeleteObjectARB(hVertexShader);
      glASSERT();
      glDeleteObjectARB(hFragmentShader);
      glASSERT();
      glDeleteProgram(hProgram);
      glASSERT();
    }
  }
  else
  {
    orxCHAR acBuffer[4096];

    /* Gets log */
    glGetInfoLogARB(hVertexShader, sizeof(acBuffer) - 1, NULL, (GLchar *)acBuffer);
    glASSERT();
    acBuffer[sizeof(acBuffer) - 1] = orxCHAR_NULL;

    /* Outputs log */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Couldn't compile vertex shader:%s%s%s", orxSTRING_EOL, acBuffer, orxSTRING_EOL);

    /* Deletes shader objects & program */
    glDeleteObjectARB(hVertexShader);
    glASSERT();
    glDeleteObjectARB(hFragmentShader);
    glASSERT();
    glDeleteProgram(hProgram);
    glASSERT();
  }

  /* Done! */
  return eResult;
}

static void orxFASTCALL orxDisplay_GLFW_InitShader(orxDISPLAY_SHADER *_pstShader)
{
  /* Uses its program */
  glUseProgramObjectARB(_pstShader->hProgram);
  glASSERT();

  /* Has custom textures? */
  if(_pstShader->iTextureCount > 0)
  {
    GLint i;

    /* For all defined textures */
    for(i = 0; i < _pstShader->iTextureCount; i++)
    {
      /* Binds bitmap */
      orxDisplay_GLFW_BindBitmap(_pstShader->astTextureInfoList[i].pstBitmap);

      /* Updates shader uniform */
      glUNIFORM(1iARB, _pstShader->astTextureInfoList[i].iLocation, sstDisplay.s32ActiveTextureUnit);
    }
  }

  /* Done! */
  return;
}

static void orxFASTCALL orxDisplay_GLFW_DrawArrays()
{
  /* Has data? */
  if(sstDisplay.s32BufferIndex > 0)
  {
    GLvoid *pIndexContext;

    /* Profiles */
    orxPROFILER_PUSH_MARKER("orxDisplay_DrawArrays");

    /* Has VBO support? */
    if(orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_VBO))
    {
      /* No offset in the index list */
      pIndexContext = (GLvoid *)0;

      /* Indirect mode? */
      if(sstDisplay.eLastBufferMode == orxDISPLAY_BUFFER_MODE_INDIRECT)
      {
        /* Copies vertex buffer */
        glBufferSubDataARB(GL_ARRAY_BUFFER_ARB, 0, sstDisplay.s32BufferIndex * sizeof(orxDISPLAY_GLFW_VERTEX), sstDisplay.astVertexList);
        glASSERT();
      }
    }
    else
    {
      /* Uses client-side index list */
      pIndexContext = (GLvoid *)sstDisplay.au16IndexList;
    }

    /* Has active shaders? */
    if(orxLinkList_GetCount(&(sstDisplay.stActiveShaderList)) > 0)
    {
      orxDISPLAY_SHADER *pstShader, *pstNextShader;

      /* For all active shaders */
      for(pstShader = (orxDISPLAY_SHADER *)orxLinkList_GetFirst(&(sstDisplay.stActiveShaderList));
          pstShader != orxNULL;
          pstShader = pstNextShader)
      {
        /* Inits shader */
        orxDisplay_GLFW_InitShader(pstShader);

        /* Draws elements */
        glDrawElements(sstDisplay.ePrimitive, (GLsizei)sstDisplay.s32ElementNumber, GL_UNSIGNED_SHORT, pIndexContext);
        glASSERT();

        /* Gets next shader */
        pstNextShader = (orxDISPLAY_SHADER *)orxLinkList_GetNext(&(pstShader->stNode));

        /* Was pending removal? */
        if(pstShader->bPending != orxFALSE)
        {
          /* Clears its texture count */
          pstShader->iTextureCount = 0;

          /* Clears its texture info list */
          orxMemory_Zero(pstShader->astTextureInfoList, sstDisplay.iTextureUnitNumber * sizeof(orxDISPLAY_TEXTURE_INFO));

          /* Removes its pending status */
          pstShader->bPending = orxFALSE;

          /* Removes it from active list */
          orxLinkList_Remove(&(pstShader->stNode));

          /* Updates count */
          sstDisplay.s32PendingShaderCount--;
        }
      }

      /* Has shader support? */
      if(orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_SHADER))
      {
        /* Uses default shader */
        orxDisplay_GLFW_StopShader(orxNULL);
      }
    }
    else
    {
      /* Draws elements */
      glDrawElements(sstDisplay.ePrimitive, (GLsizei)sstDisplay.s32ElementNumber, GL_UNSIGNED_SHORT, pIndexContext);
      glASSERT();
    }

    /* Clears buffer index & element number */
    sstDisplay.s32BufferIndex   =
    sstDisplay.s32ElementNumber = 0;

    /* Profiles */
    orxPROFILER_POP_MARKER();
  }

  /* Done! */
  return;
}

static void orxFASTCALL orxDisplay_GLFW_SetBufferMode(orxDISPLAY_BUFFER_MODE _eBufferMode)
{
  /* New buffer mode? */
  if(_eBufferMode != sstDisplay.eLastBufferMode)
  {
    /* Draws remaining items */
    orxDisplay_GLFW_DrawArrays();

    /* Indirect? */
    if(_eBufferMode == orxDISPLAY_BUFFER_MODE_INDIRECT)
    {
      /* Reverts back to default primitive */
      sstDisplay.ePrimitive = orxDISPLAY_KE_DEFAULT_PRIMITIVE;

      /* Has VBO support? */
      if(orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_VBO))
      {
        /* Inits VBO */
        glBufferDataARB(GL_ARRAY_BUFFER_ARB, orxDISPLAY_KU32_VERTEX_BUFFER_SIZE * sizeof(orxDISPLAY_GLFW_VERTEX), NULL, GL_DYNAMIC_DRAW_ARB);
        glASSERT();

        /* Was using custom IBO? */
        if(orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_CUSTOM_IBO))
        {
          /* Fills IBO */
          glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, orxDISPLAY_KU32_INDEX_BUFFER_SIZE * sizeof(GLushort), sstDisplay.au16IndexList, GL_STATIC_DRAW_ARB);
          glASSERT();

          /* Updates flags */
          orxFLAG_SET(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_NONE, orxDISPLAY_KU32_STATIC_FLAG_CUSTOM_IBO);
        }
      }
    }

    /* Stores it */
    sstDisplay.eLastBufferMode = _eBufferMode;
  }

  /* Done! */
  return;
}

static void orxFASTCALL orxDisplay_GLFW_PrepareBitmap(const orxBITMAP *_pstBitmap, orxDISPLAY_SMOOTHING _eSmoothing, orxDISPLAY_BLEND_MODE _eBlendMode, orxDISPLAY_BUFFER_MODE _eBufferMode)
{
  orxBOOL bSmoothing;

  /* Checks */
  orxASSERT((_pstBitmap != orxNULL) && (_pstBitmap != sstDisplay.pstScreen));

  /* Has pending shaders? */
  if(sstDisplay.s32PendingShaderCount != 0)
  {
    /* Draws remaining items */
    orxDisplay_GLFW_DrawArrays();

    /* Checks */
    orxASSERT(sstDisplay.s32PendingShaderCount == 0);
  }

  /* New bitmap? */
  if(_pstBitmap != sstDisplay.apstBoundBitmapList[sstDisplay.s32ActiveTextureUnit])
  {
    /* Draws remaining items */
    orxDisplay_GLFW_DrawArrays();

    /* Binds source bitmap */
    orxDisplay_GLFW_BindBitmap(_pstBitmap);

    /* Has shader support? */
    if(orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_SHADER))
    {
      /* No other shader active? */
      if(orxLinkList_GetCount(&(sstDisplay.stActiveShaderList)) == 0)
      {
        /* Updates shader uniform */
        glUNIFORM(1iARB, sstDisplay.pstDefaultShader->iTextureLocation, sstDisplay.s32ActiveTextureUnit);
      }
    }
  }

  /* Depending on smoothing type */
  switch(_eSmoothing)
  {
    case orxDISPLAY_SMOOTHING_ON:
    {
      /* Applies smoothing */
      bSmoothing = orxTRUE;

      break;
    }

    case orxDISPLAY_SMOOTHING_OFF:
    {
      /* Applies no smoothing */
      bSmoothing = orxFALSE;

      break;
    }

    default:
    case orxDISPLAY_SMOOTHING_DEFAULT:
    {
      /* Applies default smoothing */
      bSmoothing = sstDisplay.bDefaultSmoothing;

      break;
    }
  }

  /* Should update smoothing? */
  if(bSmoothing != _pstBitmap->bSmoothing)
  {
    /* Draws remaining items */
    orxDisplay_GLFW_DrawArrays();

    /* Smoothing? */
    if(bSmoothing != orxFALSE)
    {
      /* Updates texture */
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glASSERT();
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glASSERT();

      /* Updates mode */
      ((orxBITMAP *)_pstBitmap)->bSmoothing = orxTRUE;
    }
    else
    {
      /* Updates texture */
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glASSERT();
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glASSERT();

      /* Updates mode */
      ((orxBITMAP *)_pstBitmap)->bSmoothing = orxFALSE;
    }
  }

  /* Sets blend mode */
  orxDisplay_GLFW_SetBlendMode(_eBlendMode);

  /* Sets buffer mode */
  orxDisplay_GLFW_SetBufferMode(_eBufferMode);

  /* Done! */
  return;
}

static orxINLINE void orxDisplay_GLFW_DrawBitmap(const orxBITMAP *_pstBitmap, const orxDISPLAY_MATRIX *_pmTransform, orxRGBA _stColor, orxDISPLAY_SMOOTHING _eSmoothing, orxDISPLAY_BLEND_MODE _eBlendMode)
{
  GLfloat fWidth, fHeight;

  /* Prepares bitmap for drawing */
  orxDisplay_GLFW_PrepareBitmap(_pstBitmap, _eSmoothing, _eBlendMode, orxDISPLAY_BUFFER_MODE_INDIRECT);

  /* Gets bitmap working size */
  fWidth  = (GLfloat)(_pstBitmap->stClip.vBR.fX - _pstBitmap->stClip.vTL.fX);
  fHeight = (GLfloat)(_pstBitmap->stClip.vBR.fY - _pstBitmap->stClip.vTL.fY);

  /* End of buffer? */
  if(sstDisplay.s32BufferIndex > orxDISPLAY_KU32_VERTEX_BUFFER_SIZE - 5)
  {
    /* Draws arrays */
    orxDisplay_GLFW_DrawArrays();
  }

  /* Fills the vertex list */
  sstDisplay.astVertexList[sstDisplay.s32BufferIndex].fX      = (_pmTransform->vX.fY * fHeight) + _pmTransform->vX.fZ;
  sstDisplay.astVertexList[sstDisplay.s32BufferIndex].fY      = (_pmTransform->vY.fY * fHeight) + _pmTransform->vY.fZ;
  sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 1].fX  = _pmTransform->vX.fZ;
  sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 1].fY  = _pmTransform->vY.fZ;
  sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 2].fX  = (_pmTransform->vX.fX * fWidth) + (_pmTransform->vX.fY * fHeight) + _pmTransform->vX.fZ;
  sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 2].fY  = (_pmTransform->vY.fX * fWidth) + (_pmTransform->vY.fY * fHeight) + _pmTransform->vY.fZ;
  sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 3].fX  = (_pmTransform->vX.fX * fWidth) + _pmTransform->vX.fZ;
  sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 3].fY  = (_pmTransform->vY.fX * fWidth) + _pmTransform->vY.fZ;

  /* Fills the texture coord list */
  sstDisplay.astVertexList[sstDisplay.s32BufferIndex].fU      =
  sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 1].fU  = (GLfloat)(_pstBitmap->fRecRealWidth * (_pstBitmap->stClip.vTL.fX + orxDISPLAY_KF_BORDER_FIX));
  sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 2].fU  =
  sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 3].fU  = (GLfloat)(_pstBitmap->fRecRealWidth * (_pstBitmap->stClip.vBR.fX - orxDISPLAY_KF_BORDER_FIX));
  sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 1].fV  =
  sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 3].fV  = (GLfloat)(_pstBitmap->fRecRealHeight * (_pstBitmap->stClip.vTL.fY + orxDISPLAY_KF_BORDER_FIX));
  sstDisplay.astVertexList[sstDisplay.s32BufferIndex].fV      =
  sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 2].fV  = (GLfloat)(_pstBitmap->fRecRealHeight * (_pstBitmap->stClip.vBR.fY - orxDISPLAY_KF_BORDER_FIX));

  /* Fills the color list */
  sstDisplay.astVertexList[sstDisplay.s32BufferIndex].stRGBA      =
  sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 1].stRGBA  =
  sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 2].stRGBA  =
  sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 3].stRGBA  = _stColor;

  /* Updates index & element number */
  sstDisplay.s32BufferIndex   += 4;
  sstDisplay.s32ElementNumber += 6;

  /* Done! */
  return;
}

static void orxFASTCALL orxDisplay_GLFW_DrawPrimitive(orxU32 _u32VertexNumber, orxRGBA _stColor, orxBOOL _bFill, orxBOOL _bOpen)
{
  /* Profiles */
  orxPROFILER_PUSH_MARKER("orxDisplay_DrawPrimitive");

  /* Sets buffer mode */
  orxDisplay_GLFW_SetBufferMode(orxDISPLAY_BUFFER_MODE_INDIRECT);

  /* Has shader support? */
  if(orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_SHADER))
  {
    /* Starts no texture shader */
    orxDisplay_GLFW_StartShader((orxHANDLE)sstDisplay.pstNoTextureShader);

    /* Inits it */
    orxDisplay_GLFW_InitShader(sstDisplay.pstNoTextureShader);
  }
  else
  {
    /* Disables texturing */
    glDisable(GL_TEXTURE_2D);
    glASSERT();
  }

  /* Has alpha? */
  if(orxRGBA_A(_stColor) != 0xFF)
  {
    /* Enables alpha blending */
    glEnable(GL_BLEND);
    glASSERT();
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glASSERT();

    /* Updates blend mode */
    sstDisplay.eLastBlendMode = orxDISPLAY_BLEND_MODE_ALPHA;
  }
  else
  {
    /* Disables alpha blending */
    glDisable(GL_BLEND);
    glASSERT();

    /* Updates blend mode */
    sstDisplay.eLastBlendMode = orxDISPLAY_BLEND_MODE_NONE;
  }

  /* Has VBO support? */
  if(orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_VBO))
  {
    /* Copies vertex buffer */
    glBufferSubDataARB(GL_ARRAY_BUFFER_ARB, 0, _u32VertexNumber * sizeof(orxDISPLAY_GLFW_VERTEX), sstDisplay.astVertexList);
    glASSERT();
  }

  /* Only 2 vertices? */
  if(_u32VertexNumber == 2)
  {
    /* Draws it */
    glDrawArrays(GL_LINES, 0, 2);
    glASSERT();
  }
  else
  {
    /* Should fill? */
    if(_bFill != orxFALSE)
    {
      /* Draws it */
      glDrawArrays(GL_TRIANGLE_FAN, 0, _u32VertexNumber);
      glASSERT();
    }
    else
    {
      /* Is open? */
      if(_bOpen != orxFALSE)
      {
        /* Draws it */
        glDrawArrays(GL_LINE_STRIP, 0, _u32VertexNumber);
        glASSERT();
      }
      else
      {
        /* Draws it */
        glDrawArrays(GL_LINE_LOOP, 0, _u32VertexNumber);
        glASSERT();
      }
    }
  }

  /* Has shader support? */
  if(orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_SHADER))
  {
    /* Bypasses the full screen rendering when stopping shader */
    sstDisplay.s32BufferIndex = -1;

    /* Stops current shader */
    orxDisplay_GLFW_StopShader((orxHANDLE)sstDisplay.pstNoTextureShader);

    /* Resets buffer index */
    sstDisplay.s32BufferIndex = 0;
  }
  else
  {
    /* Reenables texturing */
    glEnable(GL_TEXTURE_2D);
    glASSERT();
  }

  /* Profiles */
  orxPROFILER_POP_MARKER();

  /* Done! */
  return;
}

static orxINLINE GLenum orxDisplay_GLFW_GetOpenGLPrimitive(orxDISPLAY_PRIMITIVE _ePrimitive)
{
  GLenum eResult;

#define orxDISPLAY_PRIMITIVE_CASE(TYPE)   case orxDISPLAY_PRIMITIVE_##TYPE: eResult = GL_##TYPE; break

  /* Depending on mode */
  switch(_ePrimitive)
  {
    orxDISPLAY_PRIMITIVE_CASE(POINTS);
    orxDISPLAY_PRIMITIVE_CASE(LINES);
    orxDISPLAY_PRIMITIVE_CASE(LINE_LOOP);
    orxDISPLAY_PRIMITIVE_CASE(LINE_STRIP);
    orxDISPLAY_PRIMITIVE_CASE(TRIANGLES);
    orxDISPLAY_PRIMITIVE_CASE(TRIANGLE_STRIP);
    orxDISPLAY_PRIMITIVE_CASE(TRIANGLE_FAN);
    default: eResult = orxDISPLAY_KE_DEFAULT_PRIMITIVE; break;
  }

#undef orxDISPLAY_PRIMITIVE_CASE

  /* Done! */
  return eResult;
}

static orxINLINE void orxDisplay_GLFW_UpdateCursor()
{
  const orxSTRING zCursor;

  /* Gets cursor */
  zCursor = orxConfig_GetListString(orxDISPLAY_KZ_CONFIG_CURSOR, 0);

  /* Valid? */
  if(*zCursor != orxCHAR_NULL)
  {
    orxBOOL bFound;
    orxU32  i;

    /* For all internal cursors */
    for(i = 0, bFound = orxFALSE; i < orxARRAY_GET_ITEM_COUNT(sastStandardCursorList); i++)
    {
      /* Matches? */
      if(orxString_ICompare(zCursor, sastStandardCursorList[i].zName) == 0)
      {
        /* Has cursor? */
        if(sstDisplay.pstCursor != NULL)
        {
          /* Deletes it */
          glfwDestroyCursor(sstDisplay.pstCursor);
          sstDisplay.pstCursor = NULL;
        }

        /* Defined? */
        if(sastStandardCursorList[i].iShape != 0)
        {
          /* Creates cursor */
          sstDisplay.pstCursor = glfwCreateStandardCursor(sastStandardCursorList[i].iShape);

          /* Success? */
          if(sstDisplay.pstCursor != NULL)
          {
            /* Sets it */
            glfwSetCursor(sstDisplay.pstWindow, sstDisplay.pstCursor);
          }
        }

        /* Updates status */
        bFound = orxTRUE;

        break;
      }
    }

    /* Not found? */
    if(bFound == orxFALSE)
    {
      const orxSTRING zResourceLocation;

      /* Locates resource */
      zResourceLocation = orxResource_Locate(orxTEXTURE_KZ_RESOURCE_GROUP, zCursor);

      /* Success? */
      if(zResourceLocation != orxNULL)
      {
        orxBITMAP *pstBitmap;

        /* Allocates bitmap */
        pstBitmap = (orxBITMAP *)orxBank_Allocate(sstDisplay.pstBitmapBank);

        /* Valid? */
        if(pstBitmap != orxNULL)
        {
          /* Clears it */
          orxMemory_Zero(pstBitmap, sizeof(orxBITMAP));

          /* Inits it */
          pstBitmap->zLocation    = zResourceLocation;
          pstBitmap->stFilenameID = orxString_GetID(zCursor);
          pstBitmap->u32Flags     = orxDISPLAY_KU32_BITMAP_FLAG_CURSOR;

          /* Has pivot? */
          if(orxConfig_GetListCount(orxDISPLAY_KZ_CONFIG_CURSOR) > 1)
          {
            orxVECTOR vPivot;

            /* Gets it */
            if(orxConfig_GetListVector(orxDISPLAY_KZ_CONFIG_CURSOR, 1, &vPivot) != orxNULL)
            {
              /* Stores it */
              pstBitmap->fWidth   = vPivot.fX;
              pstBitmap->fHeight  = vPivot.fY;
            }
          }

          /* Loads bitmap's data */
          if(orxDisplay_GLFW_LoadBitmapData(pstBitmap) == orxSTATUS_FAILURE)
          {
            /* Deletes it */
            orxBank_Free(sstDisplay.pstBitmapBank, pstBitmap);
          }
        }
      }
    }
  }
  else
  {
    /* Has cursor? */
    if(sstDisplay.pstCursor != NULL)
    {
      /* Deletes it */
      glfwDestroyCursor(sstDisplay.pstCursor);
      sstDisplay.pstCursor = NULL;
    }
  }

  /* Done! */
  return;
}

static orxINLINE void orxDisplay_GLFW_UpdateIconList()
{
  /* Can update? */
  if(sstDisplay.s32PendingIconCount == 0)
  {
    orxS32 s32Count;

    /* Gets icon count */
    s32Count = orxConfig_GetListCount(orxDISPLAY_KZ_CONFIG_ICON_LIST);

    /* Found? */
    if(s32Count > 0)
    {
      orxS32 i;

      /* Too many? */
      if(s32Count > orxDISPLAY_KU32_MAX_ICON_NUMBER)
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Can't set <%d> icons: only the first <%u> will be used.", s32Count, orxDISPLAY_KU32_MAX_ICON_NUMBER);

        /* Updates count */
        s32Count = orxDISPLAY_KU32_MAX_ICON_NUMBER;
      }

      /* For all icons */
      for(i = 0, sstDisplay.s32IconNumber = 0; i < s32Count; i++)
      {
        const orxSTRING zIcon;
        const orxSTRING zResourceLocation;

        /* Gets it */
        zIcon = orxConfig_GetListString(orxDISPLAY_KZ_CONFIG_ICON_LIST, i);

        /* Locates its resource */
        zResourceLocation = orxResource_Locate(orxTEXTURE_KZ_RESOURCE_GROUP, zIcon);

        /* Success? */
        if(zResourceLocation != orxNULL)
        {
          orxBITMAP *pstBitmap;

          /* Allocates bitmap */
          pstBitmap = (orxBITMAP *)orxBank_Allocate(sstDisplay.pstBitmapBank);

          /* Valid? */
          if(pstBitmap != orxNULL)
          {
            /* Clears it */
            orxMemory_Zero(pstBitmap, sizeof(orxBITMAP));

            /* Inits it */
            pstBitmap->zLocation    = zResourceLocation;
            pstBitmap->stFilenameID = orxString_GetID(zIcon);
            pstBitmap->u32Flags     = orxDISPLAY_KU32_BITMAP_FLAG_ICON;

            /* Updates pending icon count */
            sstDisplay.s32PendingIconCount++;

            /* Loads bitmap's data */
            if(orxDisplay_GLFW_LoadBitmapData(pstBitmap) == orxSTATUS_FAILURE)
            {
              /* Updates pending icon count */
              sstDisplay.s32PendingIconCount--;

              /* Deletes it */
              orxBank_Free(sstDisplay.pstBitmapBank, pstBitmap);
            }
          }
        }
      }
    }
  }

  /* Done! */
  return;
}

/** Event handler
 */
static orxSTATUS orxFASTCALL orxDisplay_GLFW_EventHandler(const orxEVENT *_pstEvent)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Depending on event */
  switch(_pstEvent->eType)
  {
    case orxEVENT_TYPE_RENDER:
    {
      /* Render stop? */
      if(_pstEvent->eID == orxRENDER_EVENT_STOP)
      {
        /* Draws remaining items */
        orxDisplay_GLFW_DrawArrays();

        /* Profiles */
        orxPROFILER_PUSH_MARKER("PollEvents");

        /* Polls events */
        glfwPollEvents();

        /* Profiles */
        orxPROFILER_POP_MARKER();
      }
      break;
    }

    case orxEVENT_TYPE_SYSTEM:
    {
      orxSYSTEM_EVENT_PAYLOAD *pstPayload;

      /* Gets payload */
      pstPayload = (orxSYSTEM_EVENT_PAYLOAD *)_pstEvent->pstPayload;

      /* Valid? */
      if(pstPayload != orxNULL)
      {
        /* Clears error */
        glfwGetError(NULL);

        /* Get? */
        if(pstPayload->stClipboard.zValue == orxNULL)
        {
          /* Updates payload */
          pstPayload->stClipboard.zValue = glfwGetClipboardString(sstDisplay.pstWindow);
        }
        /* Set */
        else
        {
          /* Updates clipboard */
          glfwSetClipboardString(sstDisplay.pstWindow, pstPayload->stClipboard.zValue);
        }

        /* Updates result */
        eResult = (glfwGetError(NULL) == GLFW_NO_ERROR) ? orxSTATUS_SUCCESS : orxSTATUS_FAILURE;
      }
      else
      {
        /* Updates result */
        eResult = orxSTATUS_FAILURE;
      }
      break;
    }

    case orxEVENT_TYPE_FIRST_RESERVED:
    {
      /* Checks */
      orxASSERT(_pstEvent->pstPayload != orxNULL);

      /* Sends windows back */
      *((GLFWwindow **)(_pstEvent->pstPayload)) = sstDisplay.pstWindow;
      break;
    }

    default:
    {
      eResult = orxSTATUS_FAILURE;
      break;
    }
  }

  /* Done! */
  return eResult;
}

/** VSync fix (to prevent a busy loop/high CPU use in some graphics drivers)
 */
static void orxFASTCALL orxDisplay_GLFW_VSyncFix(const orxCLOCK_INFO *_pstInfo, void *_pContext)
{
  /* Updates status */
  orxFLAG_SET(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_VSYNC_FIX, orxDISPLAY_KU32_STATIC_FLAG_NONE);

  /* Enforces VSync */
  orxDisplay_EnableVSync(orxDisplay_IsVSyncEnabled());

  /* Done! */
  return;
}

orxBITMAP *orxFASTCALL orxDisplay_GLFW_GetScreenBitmap()
{
  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Done! */
  return sstDisplay.pstScreen;
}

orxSTATUS orxFASTCALL orxDisplay_GLFW_TransformText(const orxSTRING _zString, const orxBITMAP *_pstFont, const orxCHARACTER_MAP *_pstMap, const orxDISPLAY_TRANSFORM *_pstTransform, orxRGBA _stColor, orxDISPLAY_SMOOTHING _eSmoothing, orxDISPLAY_BLEND_MODE _eBlendMode)
{
  orxDISPLAY_MATRIX mTransform;
  const orxCHAR    *pc;
  orxU32            u32CharacterCodePoint;
  GLfloat           fX, fY, fHeight;
  orxSTATUS         eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_zString != orxNULL);
  orxASSERT(_pstFont != orxNULL);
  orxASSERT(_pstMap != orxNULL);
  orxASSERT(_pstTransform != orxNULL);

  /* Inits matrix */
  orxDisplay_GLFW_InitMatrix(&mTransform, _pstTransform->fDstX, _pstTransform->fDstY, _pstTransform->fScaleX, _pstTransform->fScaleY, _pstTransform->fRotation, _pstTransform->fSrcX, _pstTransform->fSrcY);

  /* Gets character's height */
  fHeight = _pstMap->fCharacterHeight;

  /* Prepares font for drawing */
  orxDisplay_GLFW_PrepareBitmap(_pstFont, _eSmoothing, _eBlendMode, orxDISPLAY_BUFFER_MODE_INDIRECT);

  /* For all characters */
  for(u32CharacterCodePoint = orxString_GetFirstCharacterCodePoint(_zString, &pc), fX = 0.0f, fY = 0.0f;
      u32CharacterCodePoint != orxCHAR_NULL;
      u32CharacterCodePoint = orxString_GetFirstCharacterCodePoint(pc, &pc))
  {
    /* Depending on character */
    switch(u32CharacterCodePoint)
    {
      case orxCHAR_CR:
      {
        /* Half EOL? */
        if(*pc == orxCHAR_LF)
        {
          /* Updates pointer */
          pc++;
        }

        /* Fall through */
      }

      case orxCHAR_LF:
      {
        /* Updates Y position */
        fY += fHeight;

        /* Resets X position */
        fX = 0.0f;

        break;
      }

      default:
      {
        const orxCHARACTER_GLYPH *pstGlyph;
        orxFLOAT                  fWidth;

        /* Gets glyph from UTF-8 table */
        pstGlyph = (orxCHARACTER_GLYPH *)orxHashTable_Get(_pstMap->pstCharacterTable, u32CharacterCodePoint);

        /* Valid? */
        if(pstGlyph != orxNULL)
        {
          /* Gets character width */
          fWidth = pstGlyph->fWidth;

          /* End of buffer? */
          if(sstDisplay.s32BufferIndex > orxDISPLAY_KU32_VERTEX_BUFFER_SIZE - 5)
          {
            /* Draws arrays */
            orxDisplay_GLFW_DrawArrays();
          }

          /* Outputs vertices and texture coordinates */
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex].fX      = (mTransform.vX.fX * fX) + (mTransform.vX.fY * (fY + fHeight)) + mTransform.vX.fZ;
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex].fY      = (mTransform.vY.fX * fX) + (mTransform.vY.fY * (fY + fHeight)) + mTransform.vY.fZ;
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 1].fX  = (mTransform.vX.fX * fX) + (mTransform.vX.fY * fY) + mTransform.vX.fZ;
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 1].fY  = (mTransform.vY.fX * fX) + (mTransform.vY.fY * fY) + mTransform.vY.fZ;
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 2].fX  = (mTransform.vX.fX * (fX + fWidth)) + (mTransform.vX.fY * (fY + fHeight)) + mTransform.vX.fZ;
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 2].fY  = (mTransform.vY.fX * (fX + fWidth)) + (mTransform.vY.fY * (fY + fHeight)) + mTransform.vY.fZ;
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 3].fX  = (mTransform.vX.fX * (fX + fWidth)) + (mTransform.vX.fY * fY) + mTransform.vX.fZ;
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 3].fY  = (mTransform.vY.fX * (fX + fWidth)) + (mTransform.vY.fY * fY) + mTransform.vY.fZ;

          sstDisplay.astVertexList[sstDisplay.s32BufferIndex].fU      =
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 1].fU  = (GLfloat)(_pstFont->fRecRealWidth * (pstGlyph->fX + orxDISPLAY_KF_BORDER_FIX));
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 2].fU  =
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 3].fU  = (GLfloat)(_pstFont->fRecRealWidth * (pstGlyph->fX + fWidth - orxDISPLAY_KF_BORDER_FIX));
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 1].fV  =
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 3].fV  = (GLfloat)(_pstFont->fRecRealHeight * (pstGlyph->fY + orxDISPLAY_KF_BORDER_FIX));
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex].fV      =
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 2].fV  = (GLfloat)(_pstFont->fRecRealHeight * (pstGlyph->fY + fHeight - orxDISPLAY_KF_BORDER_FIX));

          /* Fills the color list */
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex].stRGBA      =
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 1].stRGBA  =
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 2].stRGBA  =
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 3].stRGBA  = _stColor;

          /* Updates index & element number */
          sstDisplay.s32BufferIndex   += 4;
          sstDisplay.s32ElementNumber += 6;
        }
        else
        {
          /* Gets default width */
          fWidth = fHeight;
        }

        /* Updates X position */
        fX += fWidth;

        break;
      }
    }
  }

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_GLFW_DrawLine(const orxVECTOR *_pvStart, const orxVECTOR *_pvEnd, orxRGBA _stColor)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pvStart != orxNULL);
  orxASSERT(_pvEnd != orxNULL);

  /* Draws remaining items */
  orxDisplay_GLFW_DrawArrays();

  /* Copies vertices */
  sstDisplay.astVertexList[0].fX = (GLfloat)(_pvStart->fX);
  sstDisplay.astVertexList[0].fY = (GLfloat)(_pvStart->fY);
  sstDisplay.astVertexList[1].fX = (GLfloat)(_pvEnd->fX);
  sstDisplay.astVertexList[1].fY = (GLfloat)(_pvEnd->fY);

  /* Copies color */
  sstDisplay.astVertexList[0].stRGBA =
  sstDisplay.astVertexList[1].stRGBA = _stColor;

  /* Draws it */
  orxDisplay_GLFW_DrawPrimitive(2, _stColor, orxFALSE, orxTRUE);

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_GLFW_DrawPolyline(const orxVECTOR *_avVertexList, orxU32 _u32VertexNumber, orxRGBA _stColor)
{
  orxU32    i;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_avVertexList != orxNULL);
  orxASSERT(_u32VertexNumber > 0);

  /* Draws remaining items */
  orxDisplay_GLFW_DrawArrays();

  /* For all vertices */
  for(i = 0; i < _u32VertexNumber; i++)
  {
    /* Copies its coords */
    sstDisplay.astVertexList[i].fX = (GLfloat)(_avVertexList[i].fX);
    sstDisplay.astVertexList[i].fY = (GLfloat)(_avVertexList[i].fY);

    /* Copies color */
    sstDisplay.astVertexList[i].stRGBA = _stColor;
  }

  /* Draws it */
  orxDisplay_GLFW_DrawPrimitive(_u32VertexNumber, _stColor, orxFALSE, orxTRUE);

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_GLFW_DrawPolygon(const orxVECTOR *_avVertexList, orxU32 _u32VertexNumber, orxRGBA _stColor, orxBOOL _bFill)
{
  orxU32    i;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_avVertexList != orxNULL);
  orxASSERT(_u32VertexNumber > 0);

  /* Draws remaining items */
  orxDisplay_GLFW_DrawArrays();

  /* For all vertices */
  for(i = 0; i < _u32VertexNumber; i++)
  {
    /* Copies its coords */
    sstDisplay.astVertexList[i].fX = (GLfloat)(_avVertexList[i].fX);
    sstDisplay.astVertexList[i].fY = (GLfloat)(_avVertexList[i].fY);

    /* Copies color */
    sstDisplay.astVertexList[i].stRGBA = _stColor;
  }

  /* Draws it */
  orxDisplay_GLFW_DrawPrimitive(_u32VertexNumber, _stColor, _bFill, orxFALSE);

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_GLFW_DrawCircle(const orxVECTOR *_pvCenter, orxFLOAT _fRadius, orxRGBA _stColor, orxBOOL _bFill)
{
  orxU32    i;
  orxFLOAT  fAngle;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pvCenter != orxNULL);
  orxASSERT(_fRadius >= orxFLOAT_0);

  /* Draws remaining items */
  orxDisplay_GLFW_DrawArrays();

  /* For all vertices */
  for(i = 0, fAngle = orxFLOAT_0; i < orxDISPLAY_KU32_CIRCLE_LINE_NUMBER; i++, fAngle += orxMATH_KF_2_PI / orxDISPLAY_KU32_CIRCLE_LINE_NUMBER)
  {
    /* Copies its coords */
    sstDisplay.astVertexList[i].fX = (GLfloat)(_fRadius * orxMath_Cos(fAngle) + _pvCenter->fX);
    sstDisplay.astVertexList[i].fY = (GLfloat)(_fRadius * orxMath_Sin(fAngle) + _pvCenter->fY);

    /* Copies color */
    sstDisplay.astVertexList[i].stRGBA = _stColor;
  }

  /* Draws it */
  orxDisplay_GLFW_DrawPrimitive(orxDISPLAY_KU32_CIRCLE_LINE_NUMBER, _stColor, _bFill, orxFALSE);

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_GLFW_DrawOBox(const orxOBOX *_pstBox, orxRGBA _stColor, orxBOOL _bFill)
{
  orxVECTOR vOrigin;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBox != orxNULL);

  /* Draws remaining items */
  orxDisplay_GLFW_DrawArrays();

  /* Gets origin */
  orxVector_Sub(&vOrigin, &(_pstBox->vPosition), &(_pstBox->vPivot));

  /* Sets vertices */
  sstDisplay.astVertexList[0].fX = (GLfloat)(vOrigin.fX);
  sstDisplay.astVertexList[0].fY = (GLfloat)(vOrigin.fY);
  sstDisplay.astVertexList[1].fX = (GLfloat)(vOrigin.fX + _pstBox->vX.fX);
  sstDisplay.astVertexList[1].fY = (GLfloat)(vOrigin.fY + _pstBox->vX.fY);
  sstDisplay.astVertexList[2].fX = (GLfloat)(vOrigin.fX + _pstBox->vX.fX + _pstBox->vY.fX);
  sstDisplay.astVertexList[2].fY = (GLfloat)(vOrigin.fY + _pstBox->vX.fY + _pstBox->vY.fY);
  sstDisplay.astVertexList[3].fX = (GLfloat)(vOrigin.fX + _pstBox->vY.fX);
  sstDisplay.astVertexList[3].fY = (GLfloat)(vOrigin.fY + _pstBox->vY.fY);

  /* Copies color */
  sstDisplay.astVertexList[0].stRGBA =
  sstDisplay.astVertexList[1].stRGBA =
  sstDisplay.astVertexList[2].stRGBA =
  sstDisplay.astVertexList[3].stRGBA = _stColor;

  /* Draws it */
  orxDisplay_GLFW_DrawPrimitive(4, _stColor, _bFill, orxFALSE);

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_GLFW_DrawMesh(const orxDISPLAY_MESH *_pstMesh, const orxBITMAP *_pstBitmap, orxDISPLAY_SMOOTHING _eSmoothing, orxDISPLAY_BLEND_MODE _eBlendMode)
{
  const orxBITMAP  *pstBitmap;
  orxU32            u32ElementNumber;
  orxSTATUS         eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstMesh != orxNULL);
  orxASSERT(_pstMesh->u32VertexNumber > 1);
  orxASSERT((_pstMesh->au16IndexList == orxNULL) || (_pstMesh->u32IndexNumber > 1));
  orxASSERT((_pstMesh->ePrimitive < orxDISPLAY_PRIMITIVE_NUMBER) || ((_pstMesh->ePrimitive == orxDISPLAY_PRIMITIVE_NONE) && (_pstMesh->au16IndexList == orxNULL)));

  /* Gets bitmap to use */
  pstBitmap = (_pstBitmap != orxNULL) ? _pstBitmap : sstDisplay.apstBoundBitmapList[sstDisplay.s32ActiveTextureUnit];

  /* Prepares bitmap for drawing */
  orxDisplay_GLFW_PrepareBitmap(pstBitmap, _eSmoothing, _eBlendMode, orxDISPLAY_BUFFER_MODE_DIRECT);

  /* Stores primitive */
  sstDisplay.ePrimitive = orxDisplay_GLFW_GetOpenGLPrimitive(_pstMesh->ePrimitive);

  /* Gets element number */
  u32ElementNumber = ((_pstMesh->u32IndexNumber != 0) && (_pstMesh->au16IndexList != orxNULL)) ? _pstMesh->u32IndexNumber : _pstMesh->u32VertexNumber + (_pstMesh->u32VertexNumber >> 1);

  /* Has VBO support? */
  if(orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_VBO))
  {
    /* Fills VBO */
    glBufferDataARB(GL_ARRAY_BUFFER_ARB, _pstMesh->u32VertexNumber * sizeof(orxDISPLAY_GLFW_VERTEX), _pstMesh->astVertexList, GL_STREAM_DRAW_ARB);
    glASSERT();

    /* Has index buffer? */
    if((_pstMesh->au16IndexList != orxNULL)
    && (_pstMesh->u32IndexNumber > 1))
    {
      /* Fills IBO */
      glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, _pstMesh->u32IndexNumber * sizeof(GLushort), _pstMesh->au16IndexList, GL_STREAM_DRAW_ARB);
      glASSERT();

      /* Updates flags */
      orxFLAG_SET(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_CUSTOM_IBO, orxDISPLAY_KU32_STATIC_FLAG_NONE);
    }

    /* Updates buffer index */
    sstDisplay.s32BufferIndex = _pstMesh->u32VertexNumber;

    /* Updates element number */
    sstDisplay.s32ElementNumber = u32ElementNumber;

    /* Draws mesh */
    orxDisplay_GLFW_DrawArrays();
  }
#ifndef __orxDISPLAY_OPENGL_ES__

  else
  {
    /* Selects local arrays */
    glVertexPointer(2, GL_FLOAT, sizeof(orxDISPLAY_VERTEX), &(_pstMesh->astVertexList[0].fX));
    glASSERT();
    glTexCoordPointer(2, GL_FLOAT, sizeof(orxDISPLAY_VERTEX), &(_pstMesh->astVertexList[0].fU));
    glASSERT();
    glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(orxDISPLAY_VERTEX), &(_pstMesh->astVertexList[0].stRGBA));
    glASSERT();

    /* Draws mesh */
    glDrawElements(sstDisplay.ePrimitive, (GLsizei)u32ElementNumber, GL_UNSIGNED_SHORT, (GLvoid *)((_pstMesh->au16IndexList != orxNULL) ? _pstMesh->au16IndexList : sstDisplay.au16IndexList));
    glASSERT();

    /* Selects global arrays */
    glVertexPointer(2, GL_FLOAT, sizeof(orxDISPLAY_VERTEX), &(sstDisplay.astVertexList[0].fX));
    glASSERT();
    glTexCoordPointer(2, GL_FLOAT, sizeof(orxDISPLAY_VERTEX), &(sstDisplay.astVertexList[0].fU));
    glASSERT();
    glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(orxDISPLAY_VERTEX), &(sstDisplay.astVertexList[0].stRGBA));
    glASSERT();
  }

#endif /* !__orxDISPLAY_OPENGL_ES__ */

  /* Done! */
  return eResult;
}

void orxFASTCALL orxDisplay_GLFW_DeleteBitmap(orxBITMAP *_pstBitmap)
{
  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBitmap != orxNULL);

  /* Not screen? */
  if(_pstBitmap != sstDisplay.pstScreen)
  {
    /* Loading? */
    if(orxFLAG_TEST(_pstBitmap->u32Flags, orxDISPLAY_KU32_BITMAP_FLAG_LOADING))
    {
      /* Asks for deletion */
      orxFLAG_SET(_pstBitmap->u32Flags, orxDISPLAY_KU32_BITMAP_FLAG_DELETE, orxDISPLAY_KU32_BITMAP_FLAG_NONE);
    }
    else
    {
      /* Deletes its data */
      orxDisplay_GLFW_DeleteBitmapData(_pstBitmap);

      /* Is temp bitmap? */
      if(_pstBitmap == sstDisplay.pstTempBitmap)
      {
        /* Clears temp bitmap */
        sstDisplay.pstTempBitmap = orxNULL;
      }

      /* Deletes it */
      orxBank_Free(sstDisplay.pstBitmapBank, _pstBitmap);
    }
  }

  /* Done! */
  return;
}

orxBITMAP *orxFASTCALL orxDisplay_GLFW_CreateBitmap(orxU32 _u32Width, orxU32 _u32Height)
{
  orxBITMAP *pstBitmap;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Allocates bitmap */
  pstBitmap = (orxBITMAP *)orxBank_Allocate(sstDisplay.pstBitmapBank);

  /* Valid? */
  if(pstBitmap != orxNULL)
  {
    /* Inits it */
    pstBitmap->bSmoothing     = sstDisplay.bDefaultSmoothing;
    pstBitmap->fWidth         = orxU2F(_u32Width);
    pstBitmap->fHeight        = orxU2F(_u32Height);
    pstBitmap->u32RealWidth   = orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_NPOT) ? _u32Width : orxMath_GetNextPowerOfTwo(_u32Width);
    pstBitmap->u32RealHeight  = orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_NPOT) ? _u32Height : orxMath_GetNextPowerOfTwo(_u32Height);
    pstBitmap->u32Depth       = 32;
    pstBitmap->fRecRealWidth  = orxFLOAT_1 / orxU2F(pstBitmap->u32RealWidth);
    pstBitmap->fRecRealHeight = orxFLOAT_1 / orxU2F(pstBitmap->u32RealHeight);
    pstBitmap->u32DataSize    = pstBitmap->u32RealWidth * pstBitmap->u32RealHeight * 4 * sizeof(orxU8);
    pstBitmap->zLocation      = orxSTRING_EMPTY;
    pstBitmap->stFilenameID   = orxSTRINGID_UNDEFINED;
    pstBitmap->u32Flags       = orxDISPLAY_KU32_BITMAP_FLAG_NONE;
    orxVector_Copy(&(pstBitmap->stClip.vTL), &orxVECTOR_0);
    orxVector_Set(&(pstBitmap->stClip.vBR), pstBitmap->fWidth, pstBitmap->fHeight, orxFLOAT_0);

    /* Tracks video memory */
    orxMEMORY_TRACK(VIDEO, pstBitmap->u32DataSize, orxTRUE);

    /* Creates new texture */
    glGenTextures(1, &pstBitmap->uiTexture);
    glASSERT();
    glBindTexture(GL_TEXTURE_2D, pstBitmap->uiTexture);
    glASSERT();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)pstBitmap->u32RealWidth, (GLsizei)pstBitmap->u32RealHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glASSERT();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glASSERT();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glASSERT();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (pstBitmap->bSmoothing != orxFALSE) ? GL_LINEAR : GL_NEAREST);
    glASSERT();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (pstBitmap->bSmoothing != orxFALSE) ? GL_LINEAR : GL_NEAREST);
    glASSERT();

    /* Restores previous texture */
    glBindTexture(GL_TEXTURE_2D, (sstDisplay.apstBoundBitmapList[sstDisplay.s32ActiveTextureUnit] != orxNULL) ? sstDisplay.apstBoundBitmapList[sstDisplay.s32ActiveTextureUnit]->uiTexture : 0);
    glASSERT();
  }

  /* Done! */
  return pstBitmap;
}

orxSTATUS orxFASTCALL orxDisplay_GLFW_ClearBitmap(orxBITMAP *_pstBitmap, orxRGBA _stColor)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Clears current destinations? */
  if(_pstBitmap == orxNULL)
  {
    /* Different clear color? */
    if(_stColor.u32RGBA != sstDisplay.stLastColor.u32RGBA)
    {
      /* Updates it */
      glClearColor(orxCOLOR_NORMALIZER * orxU2F(orxRGBA_R(_stColor)), orxCOLOR_NORMALIZER * orxU2F(orxRGBA_G(_stColor)), orxCOLOR_NORMALIZER * orxU2F(orxRGBA_B(_stColor)), orxCOLOR_NORMALIZER * orxU2F(orxRGBA_A(_stColor)));
      glASSERT();

      /* Stores it */
      sstDisplay.stLastColor.u32RGBA = _stColor.u32RGBA;
    }

    /* Clears the color buffer with given color */
    glClear(GL_COLOR_BUFFER_BIT);
    glASSERT();
  }
  else
  {
    orxBITMAP  *apstBackupBitmap[orxDISPLAY_KU32_MAX_TEXTURE_UNIT_NUMBER];
    orxU32      u32BackupBitmapCount;

    /* Backups current destinations */
    orxMemory_Copy(apstBackupBitmap, sstDisplay.apstDestinationBitmapList, sstDisplay.u32DestinationBitmapCount * sizeof(orxBITMAP *));
    u32BackupBitmapCount = sstDisplay.u32DestinationBitmapCount;

    /* Sets new destination bitmap */
    if(orxDisplay_GLFW_SetDestinationBitmaps(&_pstBitmap, 1) != orxSTATUS_FAILURE)
    {
      /* Different clear color? */
      if(_stColor.u32RGBA != sstDisplay.stLastColor.u32RGBA)
      {
        /* Updates it */
        glClearColor(orxCOLOR_NORMALIZER * orxU2F(orxRGBA_R(_stColor)), orxCOLOR_NORMALIZER * orxU2F(orxRGBA_G(_stColor)), orxCOLOR_NORMALIZER * orxU2F(orxRGBA_B(_stColor)), orxCOLOR_NORMALIZER * orxU2F(orxRGBA_A(_stColor)));
        glASSERT();

        /* Stores it */
        sstDisplay.stLastColor.u32RGBA = _stColor.u32RGBA;
      }

      /* Clears the color buffer with given color */
      glClear(GL_COLOR_BUFFER_BIT);
      glASSERT();

      /* Is screen and has depth buffer? */
      if((_pstBitmap == sstDisplay.pstScreen) && (orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_DEPTHBUFFER)))
      {
        /* Clears depth buffer */
        glClear(GL_DEPTH_BUFFER_BIT);
        glASSERT();
      }

      /* Restores previous destinations */
      orxDisplay_GLFW_SetDestinationBitmaps(apstBackupBitmap, u32BackupBitmapCount);
    }
    /* Not screen? */
    else if(_pstBitmap != sstDisplay.pstScreen)
    {
      orxRGBA *astBuffer, *pstPixel;

      /* Allocates buffer */
      astBuffer = (orxRGBA *)orxMemory_Allocate(_pstBitmap->u32RealWidth * _pstBitmap->u32RealHeight * sizeof(orxRGBA), orxMEMORY_TYPE_TEMP);

      /* Checks */
      orxASSERT(astBuffer != orxNULL);

      /* For all pixels */
      for(pstPixel = astBuffer; pstPixel < astBuffer + (_pstBitmap->u32RealWidth * _pstBitmap->u32RealHeight); pstPixel++)
      {
        /* Sets its value */
        *pstPixel = _stColor;
      }

      /* Binds texture */
      glBindTexture(GL_TEXTURE_2D, _pstBitmap->uiTexture);
      glASSERT();

      /* Updates texture */
      glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, (GLsizei)_pstBitmap->u32RealWidth, (GLsizei)_pstBitmap->u32RealHeight, GL_RGBA, GL_UNSIGNED_BYTE, astBuffer);
      glASSERT();

      /* Restores previous texture */
      glBindTexture(GL_TEXTURE_2D, (sstDisplay.apstBoundBitmapList[sstDisplay.s32ActiveTextureUnit] != orxNULL) ? sstDisplay.apstBoundBitmapList[sstDisplay.s32ActiveTextureUnit]->uiTexture : 0);
      glASSERT();

      /* Frees buffer */
      orxMemory_Free(astBuffer);
    }
  }

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_GLFW_SetBlendMode(orxDISPLAY_BLEND_MODE _eBlendMode)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* New blend mode? */
  if(_eBlendMode != sstDisplay.eLastBlendMode)
  {
    /* Draws remaining items */
    orxDisplay_GLFW_DrawArrays();

    /* Stores it */
    sstDisplay.eLastBlendMode = _eBlendMode;

    /* Depending on blend mode */
    switch(_eBlendMode)
    {
      case orxDISPLAY_BLEND_MODE_ALPHA:
      {
        glEnable(GL_BLEND);
        glASSERT();
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glASSERT();

        break;
      }

      case orxDISPLAY_BLEND_MODE_MULTIPLY:
      {
        glEnable(GL_BLEND);
        glASSERT();
        glBlendFunc(GL_DST_COLOR, GL_ZERO);
        glASSERT();

        break;
      }

      case orxDISPLAY_BLEND_MODE_ADD:
      {
        glEnable(GL_BLEND);
        glASSERT();
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glASSERT();

        break;
      }

      case orxDISPLAY_BLEND_MODE_PREMUL:
      {
        glEnable(GL_BLEND);
        glASSERT();
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        glASSERT();

        break;
      }

      default:
      {
        glDisable(GL_BLEND);
        glASSERT();

        break;
      }
    }
  }

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_GLFW_Swap()
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Has window? */
  if(sstDisplay.pstWindow != orxNULL)
  {
    /* Checks */
    orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

    /* Draws remaining items */
    orxDisplay_GLFW_DrawArrays();

    /* Swap buffers */
    glfwSwapBuffers(sstDisplay.pstWindow);
  }

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_GLFW_SetBitmapData(orxBITMAP *_pstBitmap, const orxU8 *_au8Data, orxU32 _u32ByteNumber)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBitmap != orxNULL);
  orxASSERT(_au8Data != orxNULL);

  /* Not loading? */
  if(!orxFLAG_TEST(_pstBitmap->u32Flags, orxDISPLAY_KU32_BITMAP_FLAG_LOADING))
  {
    orxU32 u32Width, u32Height;

    /* Gets bitmap's size */
    u32Width  = orxF2U(_pstBitmap->fWidth);
    u32Height = orxF2U(_pstBitmap->fHeight);

    /* Valid? */
    if((_pstBitmap != sstDisplay.pstScreen) && (_u32ByteNumber == u32Width * u32Height * 4 * sizeof(orxU8)))
    {
      orxU8 *pu8ImageBuffer;

      /* Has NPOT texture support? */
      if(orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_NPOT))
      {
        /* Uses sources bitmap */
        pu8ImageBuffer = (orxU8 *)_au8Data;
      }
      else
      {
        orxU32 i, u32LineSize, u32RealLineSize, u32SrcOffset, u32DstOffset;

        /* Allocates buffer */
        pu8ImageBuffer = (orxU8 *)orxMemory_Allocate(_pstBitmap->u32RealWidth * _pstBitmap->u32RealHeight * 4 * sizeof(orxU8), orxMEMORY_TYPE_TEMP);

        /* Gets line sizes */
        u32LineSize     = orxF2U(_pstBitmap->fWidth) * 4 * sizeof(orxU8);
        u32RealLineSize = _pstBitmap->u32RealWidth * 4 * sizeof(orxU8);

        /* Clears padding */
        orxMemory_Zero(pu8ImageBuffer, u32RealLineSize * (_pstBitmap->u32RealHeight - orxF2U(_pstBitmap->fHeight)));

        /* For all lines */
        for(i = 0, u32SrcOffset = 0, u32DstOffset = 0;
            i < u32Height;
            i++, u32SrcOffset += u32LineSize, u32DstOffset += u32RealLineSize)
        {
          /* Copies data */
          orxMemory_Copy(pu8ImageBuffer + u32DstOffset, _au8Data + u32SrcOffset, u32LineSize);

          /* Adds padding */
          orxMemory_Zero(pu8ImageBuffer + u32DstOffset + u32LineSize, u32RealLineSize - u32LineSize);
        }
      }

      /* Binds texture */
      glBindTexture(GL_TEXTURE_2D, _pstBitmap->uiTexture);
      glASSERT();

      /* Updates its content */
      glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, (GLsizei)_pstBitmap->u32RealWidth, (GLsizei)_pstBitmap->u32RealHeight, GL_RGBA, GL_UNSIGNED_BYTE, pu8ImageBuffer);
      glASSERT();

      /* Restores previous texture */
      glBindTexture(GL_TEXTURE_2D, (sstDisplay.apstBoundBitmapList[sstDisplay.s32ActiveTextureUnit] != orxNULL) ? sstDisplay.apstBoundBitmapList[sstDisplay.s32ActiveTextureUnit]->uiTexture : 0);
      glASSERT();

      /* Needs to free buffer? */
      if(pu8ImageBuffer != _au8Data)
      {
        /* Frees it */
        orxMemory_Free(pu8ImageBuffer);
      }

      /* Updates result */
      eResult = orxSTATUS_SUCCESS;
    }
    else
    {
      /* Screen? */
      if(_pstBitmap == sstDisplay.pstScreen)
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Can't set bitmap data: can't use screen as destination bitmap.");
      }
      else
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Can't set bitmap data for [%s]: format needs to be RGBA.", _pstBitmap->zLocation);
      }

      /* Updates result */
      eResult = orxSTATUS_FAILURE;
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Can't set bitmap data for [%s]: bitmap is not done loading.", _pstBitmap->zLocation);

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_GLFW_GetBitmapData(const orxBITMAP *_pstBitmap, orxU8 *_au8Data, orxU32 _u32ByteNumber)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBitmap != orxNULL);
  orxASSERT(_au8Data != orxNULL);

  /* Not loading? */
  if(!orxFLAG_TEST(_pstBitmap->u32Flags, orxDISPLAY_KU32_BITMAP_FLAG_LOADING))
  {
    orxU32 u32BufferSize;

    /* Gets buffer size */
    u32BufferSize = orxF2U(_pstBitmap->fWidth * _pstBitmap->fHeight) * 4 * sizeof(orxU8);

    /* Is size matching? */
    if(_u32ByteNumber == u32BufferSize)
    {
      orxU32  u32LineSize, u32RealLineSize, u32SrcOffset, u32DstOffset, i;
      orxU8  *pu8ImageBuffer;

      /* Draws remaining items */
      orxDisplay_GLFW_DrawArrays();

#ifdef __orxDISPLAY_OPENGL_ES__

      {
        orxBITMAP  *apstBackupBitmap[orxDISPLAY_KU32_MAX_TEXTURE_UNIT_NUMBER];
        orxU32      u32BackupBitmapCount;

        /* Backups current destinations */
        orxMemory_Copy(apstBackupBitmap, sstDisplay.apstDestinationBitmapList, sstDisplay.u32DestinationBitmapCount * sizeof(orxBITMAP *));
        u32BackupBitmapCount = sstDisplay.u32DestinationBitmapCount;

        /* Sets new destination bitmap */
        eResult = orxDisplay_GLFW_SetDestinationBitmaps((orxBITMAP **)&_pstBitmap, 1);

        /* Checks */
        orxASSERT(eResult != orxSTATUS_FAILURE);

        /* Allocates buffer */
        pu8ImageBuffer = ((_pstBitmap != sstDisplay.pstScreen) && (orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_NPOT))) ? _au8Data : (orxU8 *)orxMemory_Allocate(_pstBitmap->u32RealWidth * _pstBitmap->u32RealHeight * 4 * sizeof(orxU8), orxMEMORY_TYPE_TEMP);

        /* Checks */
        orxASSERT(pu8ImageBuffer != orxNULL);

        /* Reads OpenGL data */
        glReadPixels(0, 0, _pstBitmap->u32RealWidth, _pstBitmap->u32RealHeight, GL_RGBA, GL_UNSIGNED_BYTE, pu8ImageBuffer);
        glASSERT();

        /* Restores previous destinations */
        orxDisplay_GLFW_SetDestinationBitmaps(apstBackupBitmap, u32BackupBitmapCount);
      }

#else /* __orxDISPLAY_OPENGL_ES__ */

      /* Allocates buffer */
      pu8ImageBuffer = ((_pstBitmap != sstDisplay.pstScreen) && (orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_NPOT))) ? _au8Data : (orxU8 *)orxMemory_Allocate(_pstBitmap->u32RealWidth * _pstBitmap->u32RealHeight * 4 * sizeof(orxU8), orxMEMORY_TYPE_TEMP);

      /* Checks */
      orxASSERT(pu8ImageBuffer != orxNULL);

      /* Binds bitmap's associated texture */
      glBindTexture(GL_TEXTURE_2D, _pstBitmap->uiTexture);
      glASSERT();

      /* Screen capture? */
      if(_pstBitmap == sstDisplay.pstScreen)
      {
        /* Tracks video memory */
        orxMEMORY_TRACK(VIDEO, sstDisplay.pstScreen->u32DataSize, orxTRUE);

        /* Creates texture for screen backup */
        glGenTextures(1, &(sstDisplay.pstScreen->uiTexture));
        glASSERT();
        glBindTexture(GL_TEXTURE_2D, sstDisplay.pstScreen->uiTexture);
        glASSERT();
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)sstDisplay.pstScreen->u32RealWidth, (GLsizei)sstDisplay.pstScreen->u32RealHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glASSERT();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glASSERT();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glASSERT();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (sstDisplay.pstScreen->bSmoothing != orxFALSE) ? GL_LINEAR : GL_NEAREST);
        glASSERT();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (sstDisplay.pstScreen->bSmoothing != orxFALSE) ? GL_LINEAR : GL_NEAREST);
        glASSERT();

        /* Copies screen content */
        glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, (GLint)(orxF2U(_pstBitmap->fHeight) - _pstBitmap->u32RealHeight), (GLsizei)orxF2U(_pstBitmap->fWidth), (GLsizei)_pstBitmap->u32RealHeight);
        glASSERT();

        /* Copies bitmap data */
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pu8ImageBuffer);
        glASSERT();

        /* Tracks video memory */
        orxMEMORY_TRACK(VIDEO, sstDisplay.pstScreen->u32DataSize, orxFALSE);

        /* Deletes screen backup texture */
        glDeleteTextures(1, &(sstDisplay.pstScreen->uiTexture));
        glASSERT();
        sstDisplay.pstScreen->uiTexture = 0;
      }
      else
      {
        /* Copies bitmap data */
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pu8ImageBuffer);
        glASSERT();
      }

      /* Restores previous texture */
      glBindTexture(GL_TEXTURE_2D, (sstDisplay.apstBoundBitmapList[sstDisplay.s32ActiveTextureUnit] != orxNULL) ? sstDisplay.apstBoundBitmapList[sstDisplay.s32ActiveTextureUnit]->uiTexture : 0);
      glASSERT();

#endif /* __orxDISPLAY_OPENGL_ES__ */

      /* Gets line sizes */
      u32LineSize     = orxF2U(_pstBitmap->fWidth) * 4 * sizeof(orxU8);
      u32RealLineSize = _pstBitmap->u32RealWidth * 4 * sizeof(orxU8);

      /* Screen? */
      if(_pstBitmap == sstDisplay.pstScreen)
      {
        orxRGBA stOpaque;

        /* Sets opaque pixel */
        stOpaque = orx2RGBA(0x00, 0x00, 0x00, 0xFF);

        /* For all lines */
        for(i = 0, u32SrcOffset = u32RealLineSize * (_pstBitmap->u32RealHeight - orxF2U(_pstBitmap->fHeight)), u32DstOffset = u32LineSize * (orxF2U(_pstBitmap->fHeight) - 1);
            i < orxF2U(_pstBitmap->fHeight);
            i++, u32SrcOffset += u32RealLineSize, u32DstOffset -= u32LineSize)
        {
          orxU32 j;

          /* For all columns */
          for(j = 0; j < orxF2U(_pstBitmap->fWidth); j++)
          {
            orxRGBA stPixel;

            /* Gets opaque pixel */
            stPixel.u32RGBA = ((orxRGBA *)(pu8ImageBuffer + u32SrcOffset))[j].u32RGBA | stOpaque.u32RGBA;

            /* Stores it */
            ((orxRGBA *)(_au8Data + u32DstOffset))[j] = stPixel;
          }
        }

        /* Deletes buffer */
        orxMemory_Free(pu8ImageBuffer);
      }
      else
      {
        /* Doesn't have NPOT texture support? */
        if(!orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_NPOT))
        {
          /* For all lines */
          for(i = 0, u32SrcOffset = 0, u32DstOffset = 0;
              i < orxF2U(_pstBitmap->fHeight);
              i++, u32SrcOffset += u32RealLineSize, u32DstOffset += u32LineSize)
          {
            /* Copies data */
            orxMemory_Copy(_au8Data + u32DstOffset, pu8ImageBuffer + u32SrcOffset, u32LineSize);
          }

          /* Deletes buffer */
          orxMemory_Free(pu8ImageBuffer);
        }
      }

      /* Updates result */
      eResult = orxSTATUS_SUCCESS;
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Can't get bitmap data for [%s] as the buffer size is <%u> when it should be <%u>.", _pstBitmap->zLocation, _u32ByteNumber, u32BufferSize);

      /* Updates result */
      eResult = orxSTATUS_FAILURE;
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Can't get bitmap data for [%s] as it's not done loading.", _pstBitmap->zLocation);

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_GLFW_SetPartialBitmapData(orxBITMAP *_pstBitmap, const orxU8 *_au8Data, orxU32 _u32X, orxU32 _u32Y, orxU32 _u32Width, orxU32 _u32Height)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBitmap != orxNULL);
  orxASSERT(_au8Data != orxNULL);

  /* Not loading? */
  if(!orxFLAG_TEST(_pstBitmap->u32Flags, orxDISPLAY_KU32_BITMAP_FLAG_LOADING))
  {
    orxU32 u32BitmapWidth, u32BitmapHeight;

    /* Gets bitmap's size */
    u32BitmapWidth  = orxF2U(_pstBitmap->fWidth);
    u32BitmapHeight = orxF2U(_pstBitmap->fHeight);

    /* Valid? */
    if((_pstBitmap != sstDisplay.pstScreen) && (_u32X + _u32Width <= u32BitmapWidth) && (_u32Y + _u32Height <= u32BitmapHeight))
    {
      orxU8 *pu8ImageBuffer;

      /* Uses sources bitmap */
      pu8ImageBuffer = (orxU8 *)_au8Data;

      /* Binds texture */
      glBindTexture(GL_TEXTURE_2D, _pstBitmap->uiTexture);
      glASSERT();

      /* Updates its content */
      glTexSubImage2D(GL_TEXTURE_2D, 0, _u32X, _u32Y, (GLsizei)_u32Width, (GLsizei)_u32Height, GL_RGBA, GL_UNSIGNED_BYTE, pu8ImageBuffer);
      glASSERT();

      /* Restores previous texture */
      glBindTexture(GL_TEXTURE_2D, (sstDisplay.apstBoundBitmapList[sstDisplay.s32ActiveTextureUnit] != orxNULL) ? sstDisplay.apstBoundBitmapList[sstDisplay.s32ActiveTextureUnit]->uiTexture : 0);
      glASSERT();

      /* Updates result */
      eResult = orxSTATUS_SUCCESS;
    }
    else
    {
      /* Screen? */
      if(_pstBitmap == sstDisplay.pstScreen)
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Can't set bitmap data: can't use screen as destination bitmap.");
      }
      else
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Can't set bitmap data for [%s]: rectangle coordinates (%u, %u) - (%u, %u) are out of bound [%ux%u].", _pstBitmap->zLocation, _u32X, _u32Y, _u32X + _u32Width, _u32Y + _u32Height, u32BitmapWidth, u32BitmapHeight);
      }

      /* Updates result */
      eResult = orxSTATUS_FAILURE;
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Can't set bitmap data for [%s]: bitmap is not done loading.", _pstBitmap->zLocation);

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_GLFW_SetDestinationBitmaps(orxBITMAP **_apstBitmapList, orxU32 _u32Number)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_apstBitmapList != orxNULL)

  /* Has framebuffer support? */
  if(orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_FRAMEBUFFER))
  {
    orxU32 u32Number;

    /* Too many destinations? */
    if(_u32Number > (orxU32)sstDisplay.iDrawBufferNumber)
    {
      /* Outputs logs */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Can only attach the first <%d> bitmaps as destinations, out of the <%u> requested.", sstDisplay.iDrawBufferNumber, _u32Number);

      /* Updates bitmap count */
      u32Number = (orxU32)sstDisplay.iDrawBufferNumber;
    }
    else
    {
      /* Gets bitmap count */
      u32Number = _u32Number;
    }

    /* Has destinations? */
    if(u32Number != 0)
    {
      orxU32  i;
      orxBOOL bDraw, bUseFrameBuffer = orxFALSE;

      /* Updates draw status */
      bDraw = ((_apstBitmapList[0] != sstDisplay.apstDestinationBitmapList[0]) || (u32Number != sstDisplay.u32DestinationBitmapCount)) ? orxTRUE : orxFALSE;

      /* Not screen? */
      if((_apstBitmapList[0] != orxNULL) && (_apstBitmapList[0] != sstDisplay.pstScreen))
      {
        orxFLOAT fWidth, fHeight;

        /* Checks */
        orxASSERT(_apstBitmapList[0] != orxNULL);

        /* Gets first destination width & height */
        fWidth  = _apstBitmapList[0]->fWidth;
        fHeight = _apstBitmapList[0]->fHeight;

        /* For all other destination bitmaps */
        for(i = 1; (i < u32Number) && (eResult != orxSTATUS_FAILURE); i++)
        {
          orxBITMAP *pstBitmap;

          /* Gets it */
          pstBitmap = _apstBitmapList[i];

          /* Checks */
          orxASSERT(pstBitmap != orxNULL);
          orxASSERT((pstBitmap != sstDisplay.pstScreen) && "Can only use screen as bitmap destination by itself.");

          /* Valid? */
          if(pstBitmap != orxNULL)
          {
            /* Same size? */
            if((pstBitmap->fWidth == fWidth) && (pstBitmap->fHeight == fHeight))
            {
              /* Different than previous? */
              if(pstBitmap != sstDisplay.apstDestinationBitmapList[i])
              {
                /* Updates draw status */
                bDraw = orxTRUE;
              }
            }
            else
            {
              /* Logs message */
              orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Can't set bitmap destinations as they have different dimensions: (%f, %f) != (%f, %f).", pstBitmap->fWidth, pstBitmap->fHeight, fWidth, fHeight);

              /* Updates result */
              eResult = orxSTATUS_FAILURE;

              break;
            }
          }
        }

        /* Updates status */
        bUseFrameBuffer = orxTRUE;
      }
      else
      {
        /* Has destination? */
        if(_apstBitmapList[0] != orxNULL)
        {
          /* Checks */
          orxASSERT((_u32Number == 1) && "Can only use screen as bitmap destination by itself.");

          /* Multiple destinations? */
          if(_u32Number != 1)
          {
            /* Updates result */
            eResult = orxSTATUS_FAILURE;
          }
        }
        else
        {
          /* Updates result */
          eResult = orxSTATUS_FAILURE;
        }
      }

      /* Success? */
      if(eResult != orxSTATUS_FAILURE)
      {
        orxU32 j;

        /* Should draw? */
        if(bDraw != orxFALSE)
        {
          /* Draws remaining items */
          orxDisplay_GLFW_DrawArrays();
        }

        /* Using framebuffer? */
        if(bUseFrameBuffer != orxFALSE)
        {
          /* Different framebuffer? */
          if(sstDisplay.uiFrameBuffer != sstDisplay.uiLastFrameBuffer)
          {
            /* Binds frame buffer */
            glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, sstDisplay.uiFrameBuffer);
            glASSERT();

            /* Updates status */
            sstDisplay.uiLastFrameBuffer = sstDisplay.uiFrameBuffer;
          }
        }

        /* For all destination bitmaps */
        for(i = 0; i < u32Number; i++)
        {
          orxBITMAP *pstBitmap;

          /* Gets it */
          pstBitmap = _apstBitmapList[i];

          /* Screen? */
          if(pstBitmap == sstDisplay.pstScreen)
          {
            /* Different destination bitmap? */
            if(pstBitmap != sstDisplay.apstDestinationBitmapList[i])
            {
              /* Different framebuffer? */
              if(sstDisplay.uiFrameBuffer != 0)
              {
                /* Binds default frame buffer */
                glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
                glASSERT();

                /* Updates status */
                sstDisplay.uiLastFrameBuffer = 0;
              }
            }
          }
          /* Valid texture? */
          else if(pstBitmap != orxNULL)
          {
            /* Links texture to it */
            glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT + i, GL_TEXTURE_2D, pstBitmap->uiTexture, 0);
            glASSERT();
          }
          else
          {
            /* Updates result */
            eResult = orxSTATUS_FAILURE;

            break;
          }

          /* Stores new destination bitmap */
          sstDisplay.apstDestinationBitmapList[i] = pstBitmap;
        }

        /* Updates count */
        sstDisplay.u32DestinationBitmapCount = i;

        /* Using framebuffer? */
        if(bUseFrameBuffer != orxFALSE)
        {
          /* For all previous destinations */
          for(j = i; j < (orxU32)sstDisplay.iDrawBufferNumber; j++)
          {
            /* Removes previous bound texture */
            glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT + j, GL_TEXTURE_2D, 0, 0);
            glASSERT();
          }

          /* Supports more than a single draw buffer? */
          if((sstDisplay.iDrawBufferNumber > 1) && (i != 0))
          {
            /* Updates draw buffers */
            glDrawBuffersARB((GLsizei)i, sstDisplay.aeDrawBufferList);
            glASSERT();
          }
        }

        /* For all previous destinations */
        for(j = i; j < (orxU32)sstDisplay.iDrawBufferNumber; j++)
        {
          /* Clears it */
          sstDisplay.apstDestinationBitmapList[j] = orxNULL;
        }

        /* Updates result */
        eResult = (glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT) == GL_FRAMEBUFFER_COMPLETE_EXT) ? orxSTATUS_SUCCESS : orxSTATUS_FAILURE;
        glASSERT();
      }
    }
  }
  else
  {
    /* Single destination as screen? */
    if((_u32Number == 1) && (_apstBitmapList[0] == sstDisplay.pstScreen))
    {
      /* Binds default frame buffer */
      glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
      glASSERT();

      /* Stores screen bitmap */
      sstDisplay.apstDestinationBitmapList[0] = sstDisplay.pstScreen;

      /* Updates result */
      eResult = (glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT) == GL_FRAMEBUFFER_COMPLETE_EXT) ? orxSTATUS_SUCCESS : orxSTATUS_FAILURE;
      glASSERT();
    }
    else
    {
      /* Has destinations? */
      if(_u32Number != 0)
      {
        /* Outputs logs */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Can only attach screen as destination as there's no framebuffer support on this machine.");

        /* Updates status */
        eResult = orxSTATUS_FAILURE;
      }
    }
  }

  /* Success? */
  if(eResult != orxSTATUS_FAILURE)
  {
    GLfloat  fOrthoRight, fOrthoBottom;
    GLint     iX = 0, iY = 0;
    GLsizei   iWidth, iHeight;

    /* Is screen? */
    if(sstDisplay.apstDestinationBitmapList[0] == sstDisplay.pstScreen)
    {
      /* Gets framebuffer size */
      glfwGetFramebufferSize(sstDisplay.pstWindow, (int *)&iWidth, (int *)&iHeight);

      /* Updates ortho info */
      fOrthoRight   = (GLfloat)sstDisplay.apstDestinationBitmapList[0]->fWidth;
      fOrthoBottom  = (GLfloat)sstDisplay.apstDestinationBitmapList[0]->fHeight;
    }
    else
    {
      /* Updates viewport info */
      iWidth  = (GLsizei)orxF2S(sstDisplay.apstDestinationBitmapList[0]->fWidth);
      iHeight = (GLsizei)orxF2S(sstDisplay.apstDestinationBitmapList[0]->fHeight);

      /* Updates ortho info */
      fOrthoRight   = (GLfloat)sstDisplay.apstDestinationBitmapList[0]->fWidth;
      fOrthoBottom  = (GLfloat)-sstDisplay.apstDestinationBitmapList[0]->fHeight;
    }

    /* Should update viewport? */
    if((iX != sstDisplay.iLastViewportX)
    || (iY != sstDisplay.iLastViewportY)
    || (iWidth != sstDisplay.iLastViewportWidth)
    || (iHeight != sstDisplay.iLastViewportHeight))
    {
      /* Inits viewport */
      glViewport(iX, iY, iWidth, iHeight);
      glASSERT();

      /* Stores its info */
      sstDisplay.iLastViewportX       = iX;
      sstDisplay.iLastViewportY       = iY;
      sstDisplay.iLastViewportWidth   = iWidth;
      sstDisplay.iLastViewportHeight  = iHeight;
    }

    /* Should update the orthogonal projection? */
    if((fOrthoRight != sstDisplay.fLastOrthoRight)
    || (fOrthoBottom != sstDisplay.fLastOrthoBottom))
    {
      /* Stores data */
      sstDisplay.fLastOrthoRight  = fOrthoRight;
      sstDisplay.fLastOrthoBottom = fOrthoBottom;

      /* Inits projection matrix */
      (sstDisplay.fLastOrthoBottom >= 0.0)
      ? orxDisplay_GLFW_OrthoProjMatrix(&(sstDisplay.mProjectionMatrix), orxFLOAT_0, fOrthoRight, fOrthoBottom, orxFLOAT_0, -orxFLOAT_1, orxFLOAT_1)
      : orxDisplay_GLFW_OrthoProjMatrix(&(sstDisplay.mProjectionMatrix), orxFLOAT_0, fOrthoRight, orxFLOAT_0, -fOrthoBottom, -orxFLOAT_1, orxFLOAT_1);

      /* Has shader support? */
      if(orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_SHADER))
      {
        /* Passes it to shader */
        glUNIFORM(Matrix4fvARB, sstDisplay.pstDefaultShader->iProjectionMatrixLocation, 1, GL_FALSE, (GLfloat *)&(sstDisplay.mProjectionMatrix.aafValueList[0][0]));
      }
    }
  }

  /* Done! */
  return eResult;
}

orxU32 orxFASTCALL orxDisplay_GLFW_GetBitmapID(const orxBITMAP *_pstBitmap)
{
  orxU32 u32Result;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT((_pstBitmap != orxNULL) && (_pstBitmap != sstDisplay.pstScreen));

  /* Updates result */
  u32Result = (orxU32)_pstBitmap->uiTexture;

  /* Done! */
  return u32Result;
}

orxSTATUS orxFASTCALL orxDisplay_GLFW_TransformBitmap(const orxBITMAP *_pstSrc, const orxDISPLAY_TRANSFORM *_pstTransform, orxRGBA _stColor, orxDISPLAY_SMOOTHING _eSmoothing, orxDISPLAY_BLEND_MODE _eBlendMode)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSrc != sstDisplay.pstScreen);
  orxASSERT((_pstSrc == orxNULL) || (_pstTransform != orxNULL));

  /* No bitmap? */
  if(_pstSrc == orxNULL)
  {
    /* Has something to display? */
    if(sstDisplay.s32BufferIndex > 0)
    {
      /* Draws arrays */
      orxDisplay_GLFW_DrawArrays();
    }

    /* Sets buffer mode */
    orxDisplay_GLFW_SetBufferMode(orxDISPLAY_BUFFER_MODE_INDIRECT);

    /* Defines the vertex list */
    sstDisplay.astVertexList[0].fX  =
    sstDisplay.astVertexList[1].fX  = sstDisplay.apstDestinationBitmapList[0]->stClip.vTL.fX;
    sstDisplay.astVertexList[2].fX  =
    sstDisplay.astVertexList[3].fX  = sstDisplay.apstDestinationBitmapList[0]->stClip.vBR.fX;
    sstDisplay.astVertexList[1].fY  =
    sstDisplay.astVertexList[3].fY  = sstDisplay.apstDestinationBitmapList[0]->stClip.vTL.fY;
    sstDisplay.astVertexList[0].fY  =
    sstDisplay.astVertexList[2].fY  = sstDisplay.apstDestinationBitmapList[0]->stClip.vBR.fY;

    /* Defines the texture coord list */
    sstDisplay.astVertexList[0].fU  =
    sstDisplay.astVertexList[1].fU  = 0.0f;
    sstDisplay.astVertexList[2].fU  =
    sstDisplay.astVertexList[3].fU  = 1.0f;
    sstDisplay.astVertexList[1].fV  =
    sstDisplay.astVertexList[3].fV  = 0.0f;
    sstDisplay.astVertexList[0].fV  =
    sstDisplay.astVertexList[2].fV  = 1.0f;

    /* Fills the color list */
    sstDisplay.astVertexList[sstDisplay.s32BufferIndex].stRGBA      =
    sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 1].stRGBA  =
    sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 2].stRGBA  =
    sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 3].stRGBA  = sstDisplay.stLastColor;

    /* Updates index & element number */
    sstDisplay.s32BufferIndex = 4;
    sstDisplay.s32ElementNumber = 6;

    /* Draws arrays */
    orxDisplay_GLFW_DrawArrays();
  }
  else
  {
    orxDISPLAY_MATRIX mTransform;

    /* Inits matrix */
    orxDisplay_GLFW_InitMatrix(&mTransform, _pstTransform->fDstX, _pstTransform->fDstY, _pstTransform->fScaleX, _pstTransform->fScaleY, _pstTransform->fRotation, _pstTransform->fSrcX, _pstTransform->fSrcY);

    /* No repeat? */
    if((_pstTransform->fRepeatX == orxFLOAT_1) && (_pstTransform->fRepeatY == orxFLOAT_1))
    {
      /* Draws it */
      orxDisplay_GLFW_DrawBitmap(_pstSrc, &mTransform, _stColor, _eSmoothing, _eBlendMode);
    }
    else
    {
      orxFLOAT  i, j, fRecRepeatX;
      GLfloat   fX, fY, fWidth, fHeight, fTop, fBottom, fLeft, fRight;

      /* Prepares bitmap for drawing */
      orxDisplay_GLFW_PrepareBitmap(_pstSrc, _eSmoothing, _eBlendMode, orxDISPLAY_BUFFER_MODE_INDIRECT);

      /* Inits bitmap height */
      fHeight = (GLfloat)((_pstSrc->stClip.vBR.fY - _pstSrc->stClip.vTL.fY) / _pstTransform->fRepeatY);

      /* Inits texture coords */
      fLeft = _pstSrc->fRecRealWidth * (_pstSrc->stClip.vTL.fX + orxDISPLAY_KF_BORDER_FIX);
      fTop  = _pstSrc->fRecRealHeight * (_pstSrc->stClip.vTL.fY + orxDISPLAY_KF_BORDER_FIX);

      /* For all lines */
      for(fY = 0.0f, i = _pstTransform->fRepeatY, fRecRepeatX = orxFLOAT_1 / _pstTransform->fRepeatX; i > orxFLOAT_0; i -= orxFLOAT_1, fY += fHeight)
      {
        /* Partial line? */
        if(i < orxFLOAT_1)
        {
          /* Updates height */
          fHeight *= (GLfloat)i;

          /* Resets texture coords */
          fRight  = (GLfloat)(_pstSrc->fRecRealWidth * (_pstSrc->stClip.vBR.fX - orxDISPLAY_KF_BORDER_FIX));
          fBottom = (GLfloat)(_pstSrc->fRecRealHeight * (_pstSrc->stClip.vTL.fY + (i * (_pstSrc->stClip.vBR.fY - _pstSrc->stClip.vTL.fY)) - orxDISPLAY_KF_BORDER_FIX));
        }
        else
        {
          /* Resets texture coords */
          fRight  = (GLfloat)(_pstSrc->fRecRealWidth * (_pstSrc->stClip.vBR.fX - orxDISPLAY_KF_BORDER_FIX));
          fBottom = (GLfloat)(_pstSrc->fRecRealHeight * (_pstSrc->stClip.vBR.fY - orxDISPLAY_KF_BORDER_FIX));
        }

        /* Resets bitmap width */
        fWidth = (GLfloat)((_pstSrc->stClip.vBR.fX - _pstSrc->stClip.vTL.fX) * fRecRepeatX);

        /* For all columns */
        for(fX = 0.0f, j = _pstTransform->fRepeatX; j > orxFLOAT_0; j -= orxFLOAT_1, fX += fWidth)
        {
          /* Partial column? */
          if(j < orxFLOAT_1)
          {
            /* Updates width */
            fWidth *= (GLfloat)j;

            /* Updates texture right coord */
            fRight = (GLfloat)(_pstSrc->fRecRealWidth * (_pstSrc->stClip.vTL.fX + (j * (_pstSrc->stClip.vBR.fX - _pstSrc->stClip.vTL.fX))));
          }

          /* End of buffer? */
          if(sstDisplay.s32BufferIndex > orxDISPLAY_KU32_VERTEX_BUFFER_SIZE - 5)
          {
            /* Draws arrays */
            orxDisplay_GLFW_DrawArrays();
          }

          /* Outputs vertices and texture coordinates */
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex].fX      = (mTransform.vX.fX * fX) + (mTransform.vX.fY * (fY + fHeight)) + mTransform.vX.fZ;
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex].fY      = (mTransform.vY.fX * fX) + (mTransform.vY.fY * (fY + fHeight)) + mTransform.vY.fZ;
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 1].fX  = (mTransform.vX.fX * fX) + (mTransform.vX.fY * fY) + mTransform.vX.fZ;
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 1].fY  = (mTransform.vY.fX * fX) + (mTransform.vY.fY * fY) + mTransform.vY.fZ;
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 2].fX  = (mTransform.vX.fX * (fX + fWidth)) + (mTransform.vX.fY * (fY + fHeight)) + mTransform.vX.fZ;
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 2].fY  = (mTransform.vY.fX * (fX + fWidth)) + (mTransform.vY.fY * (fY + fHeight)) + mTransform.vY.fZ;
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 3].fX  = (mTransform.vX.fX * (fX + fWidth)) + (mTransform.vX.fY * fY) + mTransform.vX.fZ;
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 3].fY  = (mTransform.vY.fX * (fX + fWidth)) + (mTransform.vY.fY * fY) + mTransform.vY.fZ;

          sstDisplay.astVertexList[sstDisplay.s32BufferIndex].fU      =
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 1].fU  = fLeft;
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 2].fU  =
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 3].fU  = fRight;
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 1].fV  =
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 3].fV  = fTop;
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex].fV      =
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 2].fV  = fBottom;

          /* Fills the color list */
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex].stRGBA      =
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 1].stRGBA  =
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 2].stRGBA  =
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 3].stRGBA  = _stColor;

          /* Updates index & element number */
          sstDisplay.s32BufferIndex += 4;
          sstDisplay.s32ElementNumber += 6;
        }
      }
    }
  }

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_GLFW_SaveBitmap(const orxBITMAP *_pstBitmap, const orxSTRING _zFileName)
{
  orxU32    u32BufferSize;
  orxU8    *pu8ImageData;
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBitmap != orxNULL);
  orxASSERT(_zFileName != orxNULL);

  /* Gets buffer size */
  u32BufferSize = orxF2U(_pstBitmap->fWidth * _pstBitmap->fHeight) * 4 * sizeof(orxU8);

  /* Allocates buffer */
  pu8ImageData = (orxU8 *)orxMemory_Allocate(u32BufferSize, orxMEMORY_TYPE_TEMP);

  /* Valid? */
  if(pu8ImageData != orxNULL)
  {
    orxDISPLAY_SAVE_INFO *pstInfo = orxNULL;

    /* Gets bitmap data */
    if(orxDisplay_GLFW_GetBitmapData(_pstBitmap, pu8ImageData, u32BufferSize) != orxSTATUS_FAILURE)
    {
      const orxSTRING zResourceLocation;
      orxHANDLE       hResource;

      /* Valid file to open? */
      if(((zResourceLocation = orxResource_LocateInStorage(orxTEXTURE_KZ_RESOURCE_GROUP, orxRESOURCE_KZ_DEFAULT_STORAGE, _zFileName)) != orxNULL)
      && ((hResource = orxResource_Open(zResourceLocation, orxTRUE)) != orxHANDLE_UNDEFINED))
      {
        /* Allocates save info */
        pstInfo = (orxDISPLAY_SAVE_INFO *)orxMemory_Allocate(sizeof(orxDISPLAY_SAVE_INFO), orxMEMORY_TYPE_TEMP);

        /* Valid? */
        if(pstInfo != orxNULL)
        {
          /* Inits it */
          pstInfo->pu8ImageData   = pu8ImageData;
          pstInfo->hResource      = hResource;
          pstInfo->u32Width       = orxF2U(_pstBitmap->fWidth);
          pstInfo->u32Height      = orxF2U(_pstBitmap->fHeight);

          /* Runs asynchronous task */
          eResult = orxThread_RunTask(&orxDisplay_GLFW_SaveBitmapData, orxNULL, orxNULL, (void *)pstInfo);
        }
        else
        {
          /* Closes resource */
          orxResource_Close(hResource);
        }
      }
    }

    /* Failure? */
    if(eResult == orxSTATUS_FAILURE)
    {
      /* Frees save info */
      if(pstInfo != orxNULL)
      {
        orxMemory_Free(pstInfo);
      }

      /* Frees buffer */
      orxMemory_Free(pu8ImageData);
    }
  }

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_GLFW_SetTempBitmap(const orxBITMAP *_pstBitmap)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Stores it */
  sstDisplay.pstTempBitmap = _pstBitmap;

  /* Done! */
  return eResult;
}

const orxBITMAP *orxFASTCALL orxDisplay_GLFW_GetTempBitmap()
{
  const orxBITMAP *pstResult;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Updates result */
  pstResult = sstDisplay.pstTempBitmap;

  /* Done! */
  return pstResult;
}

orxBITMAP *orxFASTCALL orxDisplay_GLFW_LoadBitmap(const orxSTRING _zFileName)
{
  const orxSTRING zResourceLocation;
  orxBITMAP      *pstResult = orxNULL;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Locates resource */
  zResourceLocation = orxResource_Locate(orxTEXTURE_KZ_RESOURCE_GROUP, _zFileName);

  /* Success? */
  if(zResourceLocation != orxNULL)
  {
    /* Allocates bitmap */
    pstResult = (orxBITMAP *)orxBank_Allocate(sstDisplay.pstBitmapBank);

    /* Valid? */
    if(pstResult != orxNULL)
    {
      /* Inits it */
      pstResult->bSmoothing     = sstDisplay.bDefaultSmoothing;
      pstResult->zLocation      = zResourceLocation;
      pstResult->stFilenameID   = orxString_GetID(_zFileName);
      pstResult->u32Flags       = orxDISPLAY_KU32_BITMAP_FLAG_NONE;

      /* Loads its data */
      if(orxDisplay_GLFW_LoadBitmapData(pstResult) == orxSTATUS_FAILURE)
      {
        /* Deletes it */
        orxBank_Free(sstDisplay.pstBitmapBank, pstResult);

        /* Updates result */
        pstResult = orxNULL;
      }
    }
  }

  /* Done! */
  return pstResult;
}

orxSTATUS orxFASTCALL orxDisplay_GLFW_GetBitmapSize(const orxBITMAP *_pstBitmap, orxFLOAT *_pfWidth, orxFLOAT *_pfHeight)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBitmap != orxNULL);
  orxASSERT(_pfWidth != orxNULL);
  orxASSERT(_pfHeight != orxNULL);

  /* Gets size */
  *_pfWidth   = _pstBitmap->fWidth;
  *_pfHeight  = _pstBitmap->fHeight;

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_GLFW_GetScreenSize(orxFLOAT *_pfWidth, orxFLOAT *_pfHeight)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pfWidth != orxNULL);
  orxASSERT(_pfHeight != orxNULL);

  /* Gets size */
  *_pfWidth   = sstDisplay.pstScreen->fWidth;
  *_pfHeight  = sstDisplay.pstScreen->fHeight;

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_GLFW_SetBitmapClipping(orxBITMAP *_pstBitmap, orxU32 _u32TLX, orxU32 _u32TLY, orxU32 _u32BRX, orxU32 _u32BRY)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* No destination bitmap? */
  if(_pstBitmap == orxNULL)
  {
    /* Defaults to first destination */
    _pstBitmap = sstDisplay.apstDestinationBitmapList[0];
  }

  /* Destination bitmap? */
  if(_pstBitmap == sstDisplay.apstDestinationBitmapList[0])
  {
    orxU32 u32ClipX, u32ClipY, u32ClipWidth, u32ClipHeight;

    /* Draws remaining items */
    orxDisplay_GLFW_DrawArrays();

    /* Sets OpenGL clipping */
    if(_pstBitmap == sstDisplay.pstScreen)
    {
      /* Gets new clipping values */
      u32ClipX      = orxF2U(orxMath_Round(sstDisplay.vContentScale.fX * orxU2F(_u32TLX)));
      u32ClipY      = orxF2U(orxMath_Round(sstDisplay.vContentScale.fY * (sstDisplay.apstDestinationBitmapList[0]->fHeight - orxU2F(_u32BRY))));
      u32ClipWidth  = orxF2U(orxMath_Round(sstDisplay.vContentScale.fX * orxU2F(_u32BRX - _u32TLX)));
      u32ClipHeight = orxF2U(orxMath_Round(sstDisplay.vContentScale.fY * orxU2F(_u32BRY - _u32TLY)));
    }
    else
    {
      /* Gets new clipping values */
      u32ClipX      = _u32TLX;
      u32ClipY      = _u32TLY;
      u32ClipWidth  = _u32BRX - _u32TLX;
      u32ClipHeight = _u32BRY - _u32TLY;
    }

    /* Different clipping? */
    if((u32ClipX != sstDisplay.u32LastClipX)
    || (u32ClipY != sstDisplay.u32LastClipY)
    || (u32ClipWidth != sstDisplay.u32LastClipWidth)
    || (u32ClipHeight != sstDisplay.u32LastClipHeight))
    {
      /* Sets OpenGL clipping */
      glScissor((GLint)u32ClipX, (GLint)u32ClipY, (GLsizei)u32ClipWidth, (GLsizei)u32ClipHeight);
      glASSERT();

      /* Stores clipping values */
      sstDisplay.u32LastClipX       = u32ClipX;
      sstDisplay.u32LastClipY       = u32ClipY;
      sstDisplay.u32LastClipWidth   = u32ClipWidth;
      sstDisplay.u32LastClipHeight  = u32ClipHeight;
    }
  }

  /* Stores clip coords */
  orxVector_Set(&(_pstBitmap->stClip.vTL), orxU2F(_u32TLX), orxU2F(_u32TLY), orxFLOAT_0);
  orxVector_Set(&(_pstBitmap->stClip.vBR), orxU2F(_u32BRX), orxU2F(_u32BRY), orxFLOAT_0);

  /* Done! */
  return eResult;
}

orxU32 orxFASTCALL orxDisplay_GLFW_GetVideoModeCount()
{
  GLFWmonitor  *pstMonitor;
  orxU32        u32Result = 0;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Gets current monitor */
  pstMonitor = orxDisplay_GLFW_GetMonitor();

  /* Success? */
  if(pstMonitor != orxNULL)
  {
    /* Gets video mode list */
    glfwGetVideoModes(pstMonitor, (int *)&u32Result);
  }

  /* Done! */
  return u32Result;
}

orxDISPLAY_VIDEO_MODE *orxFASTCALL orxDisplay_GLFW_GetVideoMode(orxU32 _u32Index, orxDISPLAY_VIDEO_MODE *_pstVideoMode)
{
  GLFWmonitor            *pstMonitor;
  orxDISPLAY_VIDEO_MODE  *pstResult = orxNULL;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstVideoMode != orxNULL);

  /* Gets current monitor */
  pstMonitor = orxDisplay_GLFW_GetMonitor();

  /* Success? */
  if(pstMonitor != orxNULL)
  {
    const GLFWvidmode  *astModeList;
    orxU32              u32Count;

    /* Updates default mode */
    orxDisplay_GLFW_UpdateDefaultMode();

    /* Gets video mode list */
    astModeList = glfwGetVideoModes(pstMonitor, (int *)&u32Count);

    /* Request the default mode? */
    if(_u32Index == orxU32_UNDEFINED)
    {
      /* Stores info */
      _pstVideoMode->u32Width       = sstDisplay.u32DefaultWidth;
      _pstVideoMode->u32Height      = sstDisplay.u32DefaultHeight;
      _pstVideoMode->u32Depth       = sstDisplay.u32DefaultDepth;
      _pstVideoMode->u32RefreshRate = sstDisplay.u32DefaultRefreshRate;
      _pstVideoMode->bFullScreen    = orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_FULLSCREEN) ? orxTRUE : orxFALSE;

      /* 24-bit? */
      if(_pstVideoMode->u32Depth == 24)
      {
        /* Gets 32-bit instead */
        _pstVideoMode->u32Depth = 32;
      }
    }
    /* Is index valid? */
    else if(_u32Index < u32Count)
    {
      /* Stores info */
      _pstVideoMode->u32Width       = astModeList[_u32Index].width;
      _pstVideoMode->u32Height      = astModeList[_u32Index].height;
      _pstVideoMode->u32Depth       = astModeList[_u32Index].redBits + astModeList[_u32Index].greenBits + astModeList[_u32Index].blueBits;
      _pstVideoMode->u32RefreshRate = (astModeList[_u32Index].refreshRate != 0) ? astModeList[_u32Index].refreshRate : sstDisplay.u32DefaultRefreshRate;
      _pstVideoMode->bFullScreen    = orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_FULLSCREEN) ? orxTRUE : orxFALSE;

      /* 24-bit? */
      if(_pstVideoMode->u32Depth == 24)
      {
        /* Gets 32-bit instead */
        _pstVideoMode->u32Depth = 32;
      }
    }
    /* Gets current mode */
    else
    {
      /* Stores info */
      _pstVideoMode->u32Width       = orxF2U(sstDisplay.pstScreen->fWidth);
      _pstVideoMode->u32Height      = orxF2U(sstDisplay.pstScreen->fHeight);
      _pstVideoMode->u32Depth       = sstDisplay.pstScreen->u32Depth;
      _pstVideoMode->u32RefreshRate = sstDisplay.u32RefreshRate;
      _pstVideoMode->bFullScreen    = orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_FULLSCREEN) ? orxTRUE : orxFALSE;
    }

    /* Updates result */
    pstResult = _pstVideoMode;
  }

  /* Done! */
  return pstResult;
}

orxBOOL orxFASTCALL orxDisplay_GLFW_IsVideoModeAvailable(const orxDISPLAY_VIDEO_MODE *_pstVideoMode)
{
  GLFWmonitor  *pstMonitor;
  orxBOOL       bResult = orxFALSE;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstVideoMode != orxNULL);

  /* Gets current monitor */
  pstMonitor = orxDisplay_GLFW_GetMonitor();

  /* Success? */
  if(pstMonitor != orxNULL)
  {
    const GLFWvidmode  *astModeList;
    orxS32              s32Count, i;

    /* Gets video mode list */
    astModeList = glfwGetVideoModes(pstMonitor, (int *)&s32Count);

    /* For all mode */
    for(i = 0; i < s32Count; i++)
    {
      /* Matches? */
      if((_pstVideoMode->u32Width == (orxU32)astModeList[i].width)
      && (_pstVideoMode->u32Height == (orxU32)astModeList[i].height)
      && ((_pstVideoMode->u32Depth == (orxU32)(astModeList[i].redBits + astModeList[i].greenBits + astModeList[i].blueBits))
       || ((_pstVideoMode->u32Depth == 32)
        && (astModeList[i].redBits + astModeList[i].greenBits + astModeList[i].blueBits == 24)))
      && ((_pstVideoMode->u32RefreshRate == (orxU32)astModeList[i].refreshRate)
       || (_pstVideoMode->u32RefreshRate == 0)))
      {
        /* Updates result */
        bResult = orxTRUE;

        break;
      }
    }
  }

  /* Done! */
  return bResult;
}

orxSTATUS orxFASTCALL orxDisplay_GLFW_EnableVSync(orxBOOL _bEnable)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Enable? */
  if(_bEnable != orxFALSE)
  {
    /* Has VSync fix already happened? (to prevent busy loop in some graphics drivers) */
    if(orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_VSYNC_FIX))
    {
      /* Updates VSync status */
      glfwSwapInterval(orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_CONTROL_TEAR) ? -1 : 1);
    }
    else
    {
      /* Updates VSync status */
      glfwSwapInterval(0);
    }

    /* Updates status */
    orxFLAG_SET(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_VSYNC, orxDISPLAY_KU32_STATIC_FLAG_NONE);
  }
  else
  {
    /* Updates VSync status */
    glfwSwapInterval(0);

    /* Updates status */
    orxFLAG_SET(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_NONE, orxDISPLAY_KU32_STATIC_FLAG_VSYNC);
  }

  /* Done! */
  return eResult;
}

orxBOOL orxFASTCALL orxDisplay_GLFW_IsVSyncEnabled()
{
  orxBOOL bResult;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Updates result */
  bResult = orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_VSYNC) ? orxTRUE : orxFALSE;

  /* Done! */
  return bResult;
}

orxSTATUS orxFASTCALL orxDisplay_GLFW_SetVideoMode(const orxDISPLAY_VIDEO_MODE *_pstVideoMode)
{
  orxS32    i;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Draws remaining items */
  orxDisplay_GLFW_DrawArrays();

  /* Pushes display section */
  orxConfig_PushSection(orxDISPLAY_KZ_CONFIG_SECTION);

  /* Has specified video mode? */
  if(_pstVideoMode != orxNULL)
  {
    int iWidth, iHeight, iDepth, iRefreshRate;
    GLFWmonitor *pstMonitor;

    /* Gets its info */
    iWidth        = (int)((_pstVideoMode->u32Width != 0) ? _pstVideoMode->u32Width : sstDisplay.u32DefaultWidth);
    iHeight       = (int)((_pstVideoMode->u32Height != 0) ? _pstVideoMode->u32Height : sstDisplay.u32DefaultHeight);
    iDepth        = (int)((_pstVideoMode->u32Depth != 0) ? _pstVideoMode->u32Depth : sstDisplay.u32DefaultDepth);
    iRefreshRate  = (int)((_pstVideoMode->u32RefreshRate != 0) ? _pstVideoMode->u32RefreshRate : sstDisplay.u32DefaultRefreshRate);

    /* Doesn't allow resize? */
    if(orxConfig_GetBool(orxDISPLAY_KZ_CONFIG_ALLOW_RESIZE) == orxFALSE)
    {
      /* Updates flags */
      orxFLAG_SET(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_NO_RESIZE, orxDISPLAY_KU32_STATIC_FLAG_NONE);
    }
    else
    {
      /* Updates flags */
      orxFLAG_SET(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_NONE, orxDISPLAY_KU32_STATIC_FLAG_NO_RESIZE);
    }

    /* No decoration? */
    if((orxConfig_HasValue(orxDISPLAY_KZ_CONFIG_DECORATION) != orxFALSE) && (orxConfig_GetBool(orxDISPLAY_KZ_CONFIG_DECORATION) == orxFALSE))
    {
      /* Updates flags */
      orxFLAG_SET(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_NO_DECORATION, orxDISPLAY_KU32_STATIC_FLAG_NONE);
    }
    else
    {
      /* Updates flags */
      orxFLAG_SET(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_NONE, orxDISPLAY_KU32_STATIC_FLAG_NO_DECORATION);
    }

    /* Depth buffer? */
    if(orxConfig_GetBool(orxDISPLAY_KZ_CONFIG_DEPTHBUFFER) != orxFALSE)
    {
      /* Updates flags */
      orxFLAG_SET(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_DEPTHBUFFER, orxDISPLAY_KU32_STATIC_FLAG_NONE);
    }
    else
    {
      /* Updates flags */
      orxFLAG_SET(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_NONE, orxDISPLAY_KU32_STATIC_FLAG_DEPTHBUFFER);
    }

    /* Depending on video depth */
    switch(iDepth)
    {
      /* 16-bit */
      case 16:
      {
        /* Updates hints */
        glfwWindowHint(GLFW_RED_BITS, 5);
        glfwWindowHint(GLFW_GREEN_BITS, 6);
        glfwWindowHint(GLFW_BLUE_BITS, 5);
        glfwWindowHint(GLFW_ALPHA_BITS, 0);

        break;
      }

      /* 24-bit */
      case 24:
      case 32:
      {
        /* Updates hints */
        glfwWindowHint(GLFW_RED_BITS, 8);
        glfwWindowHint(GLFW_GREEN_BITS, 8);
        glfwWindowHint(GLFW_BLUE_BITS, 8);
        glfwWindowHint(GLFW_ALPHA_BITS, (iDepth == 24) ? 0 : 8);

        break;
      }
    }

    /* Gets current monitor */
    pstMonitor = orxDisplay_GLFW_GetMonitor();

    /* Has a window? */
    if(sstDisplay.pstWindow != orxNULL)
    {
      /* Different depth? */
      if((orxU32)iDepth != sstDisplay.u32Depth)
      {
        GLFWwindow *pstNewWindow;

        /* Updates window hints */
        glfwWindowHint(GLFW_RESIZABLE, orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_NO_RESIZE) ? GLFW_FALSE : GLFW_TRUE);
        glfwWindowHint(GLFW_DECORATED, orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_NO_DECORATION) ? GLFW_FALSE : GLFW_TRUE);
        glfwWindowHint(GLFW_REFRESH_RATE, iRefreshRate);

        /* Creates new window sharing the context */
        pstNewWindow = glfwCreateWindow(iWidth, iHeight, orxConfig_GetString(orxDISPLAY_KZ_CONFIG_TITLE), (_pstVideoMode->bFullScreen != orxFALSE) ? pstMonitor : orxNULL, sstDisplay.pstWindow);

        /* Success? */
        if(pstNewWindow != orxNULL)
        {
          int iWindowX = 0, iWindowY = 0;

          /* Deletes previous window */
          glfwDestroyWindow(sstDisplay.pstWindow);

          /* Stores new one */
          sstDisplay.pstWindow = pstNewWindow;

          /* Makes OpenGL context current */
          glfwMakeContextCurrent(sstDisplay.pstWindow);

          /* Ignores resize event for now */
          sstDisplay.u32Flags |= orxDISPLAY_KU32_STATIC_FLAG_IGNORE_EVENT;

          /* Registers resize callback */
          glfwSetWindowSizeCallback(sstDisplay.pstWindow, orxDisplay_GLFW_ResizeCallback);

          /* Registers content scale callback */
          glfwSetWindowContentScaleCallback(sstDisplay.pstWindow, orxDisplay_GLFW_ContentScaleCallback);

          /* Registers drop callback */
          glfwSetDropCallback(sstDisplay.pstWindow, orxDisplay_GLFW_DropCallback);

          /* Registers position callback */
          glfwSetWindowPosCallback(sstDisplay.pstWindow, orxDisplay_GLFW_PosCallback);

          /* Reactivates resize event */
          sstDisplay.u32Flags &= ~orxDISPLAY_KU32_STATIC_FLAG_IGNORE_EVENT;

          /* Gets window position */
          glfwGetWindowPos(sstDisplay.pstWindow, &iWindowX, &iWindowY);

          /* Forces a position update */
          orxDisplay_GLFW_PosCallback(sstDisplay.pstWindow, iWindowX, iWindowY);
        }
        else
        {
          /* Updates result */
          eResult = orxSTATUS_FAILURE;
        }
      }
      else
      {
        /* Ignores resize event for now */
        sstDisplay.u32Flags |= orxDISPLAY_KU32_STATIC_FLAG_IGNORE_EVENT;

        /* Updates window attributes */
        glfwSetWindowAttrib(sstDisplay.pstWindow, GLFW_RESIZABLE, orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_NO_RESIZE) ? GLFW_FALSE : GLFW_TRUE);
        glfwSetWindowAttrib(sstDisplay.pstWindow, GLFW_DECORATED, orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_NO_DECORATION) ? GLFW_FALSE : GLFW_TRUE);

        /* Applies monitor status */
        glfwSetWindowMonitor(sstDisplay.pstWindow, (_pstVideoMode->bFullScreen != orxFALSE) ? pstMonitor : orxNULL, (int)sstDisplay.vWindowPosition.fX, (int)sstDisplay.vWindowPosition.fY, iWidth, iHeight, iRefreshRate);

        /* Resizes window */
        glfwSetWindowSize(sstDisplay.pstWindow, iWidth, iHeight);

        /* Reactivates resize event */
        sstDisplay.u32Flags &= ~orxDISPLAY_KU32_STATIC_FLAG_IGNORE_EVENT;
      }
    }
    else
    {
      /* Updates window hints */
      glfwWindowHint(GLFW_RESIZABLE, orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_NO_RESIZE) ? GLFW_FALSE : GLFW_TRUE);
      glfwWindowHint(GLFW_DECORATED, orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_NO_DECORATION) ? GLFW_FALSE : GLFW_TRUE);
      glfwWindowHint(GLFW_REFRESH_RATE, iRefreshRate);

      /* Creates window */
      sstDisplay.pstWindow = glfwCreateWindow(iWidth, iHeight, orxConfig_GetString(orxDISPLAY_KZ_CONFIG_TITLE), (_pstVideoMode->bFullScreen != orxFALSE) ? pstMonitor : orxNULL, orxNULL);

      /* Success? */
      if(sstDisplay.pstWindow != orxNULL)
      {
        /* Makes OpenGL context current */
        glfwMakeContextCurrent(sstDisplay.pstWindow);

        /* Inits extensions */
        orxDisplay_GLFW_InitExtensions();

        /* Has shader support? */
        if(orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_SHADER))
        {
          orxU32                  u32ShaderVersion = orxU32_UNDEFINED;
          static const orxSTRING  szFragmentShaderSource =
#ifdef __orxDISPLAY_OPENGL_ES__
          "precision mediump float;"
#endif /* __orxDISPLAY_OPENGL_ES__ */
          "varying vec2 _gl_TexCoord0_;"
          "varying vec4 _Color0_;"
          "uniform sampler2D orxTexture;"
          "void main()"
          "{"
          "  gl_FragColor = _Color0_.rgba * texture2D(orxTexture, _gl_TexCoord0_).rgba;"
          "}";
          static const orxSTRING szNoTextureFragmentShaderSource =
#ifdef __orxDISPLAY_OPENGL_ES__
          "precision mediump float;"
#endif /* __orxDISPLAY_OPENGL_ES__ */
          "varying vec2 _gl_TexCoord0_;"
          "varying vec4 _Color0_;"
          "void main()"
          "{"
          "  gl_FragColor = _Color0_;"
          "}";

          /* Has shader version value? */
          if(orxConfig_HasValue(orxDISPLAY_KZ_CONFIG_SHADER_VERSION))
          {
            /* Stores it */
            u32ShaderVersion = orxConfig_GetU32(orxDISPLAY_KZ_CONFIG_SHADER_VERSION);

            /* Clears it */
            orxConfig_SetU32(orxDISPLAY_KZ_CONFIG_SHADER_VERSION, 0);
          }

          /* Creates default & no texture shaders */
          sstDisplay.pstDefaultShader   = (orxDISPLAY_SHADER *)orxDisplay_CreateShader(&szFragmentShaderSource, 1, orxNULL, orxFALSE);
          sstDisplay.pstNoTextureShader = (orxDISPLAY_SHADER *)orxDisplay_CreateShader(&szNoTextureFragmentShaderSource, 1, orxNULL, orxTRUE);

          /* Should restore shader version? */
          if(u32ShaderVersion != orxU32_UNDEFINED)
          {
            /* Restores it */
            orxConfig_SetU32(orxDISPLAY_KZ_CONFIG_SHADER_VERSION, u32ShaderVersion);
          }

          /* Uses default shader */
          orxDisplay_GLFW_StopShader(orxNULL);
        }

        /* Has framebuffer support? */
        if(orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_FRAMEBUFFER))
        {
          /* Generates frame buffer */
          glGenFramebuffersEXT(1, &(sstDisplay.uiFrameBuffer));
          glASSERT();

          /* Clears frame buffer cache */
          sstDisplay.uiLastFrameBuffer = 0;
        }

        /* Has VBO support? */
        if(orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_VBO))
        {
          /* Generates index buffer objects (VBO/IBO) */
          glGenBuffersARB(1, &(sstDisplay.uiVertexBuffer));
          glASSERT();
          glGenBuffersARB(1, &(sstDisplay.uiIndexBuffer));
          glASSERT();

          /* Binds them */
          glBindBufferARB(GL_ARRAY_BUFFER_ARB, sstDisplay.uiVertexBuffer);
          glASSERT();
          glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, sstDisplay.uiIndexBuffer);
          glASSERT();

          /* Inits VBO */
          glBufferDataARB(GL_ARRAY_BUFFER_ARB, orxDISPLAY_KU32_VERTEX_BUFFER_SIZE * sizeof(orxDISPLAY_GLFW_VERTEX), NULL, GL_DYNAMIC_DRAW_ARB);
          glASSERT();

          /* Fills IBO */
          glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, orxDISPLAY_KU32_INDEX_BUFFER_SIZE * sizeof(GLushort), sstDisplay.au16IndexList, GL_STATIC_DRAW_ARB);
          glASSERT();
        }
      }
      else
      {
        /* Updates result */
        eResult = orxSTATUS_FAILURE;
      }
    }

    /* Success? */
    if(eResult != orxSTATUS_FAILURE)
    {
      orxDISPLAY_EVENT_PAYLOAD  stPayload;
      orxU32                    u32ClipWidth, u32ClipHeight;
      int                       iFramebufferWidth, iFramebufferHeight;

      /* Gets window size */
      glfwGetWindowSize(sstDisplay.pstWindow, (int *)&iWidth, (int *)&iHeight);

      /* Gets framebuffer size */
      glfwGetFramebufferSize(sstDisplay.pstWindow, &iFramebufferWidth, &iFramebufferHeight);

      /* Sets content scale */
      orxVector_Set(&(sstDisplay.vContentScale), orxS2F(iFramebufferWidth) / orxS2F(iWidth), orxS2F(iFramebufferHeight) / orxS2F(iHeight), orxFLOAT_1);

      /* Is fullscreen? */
      if(_pstVideoMode->bFullScreen != orxFALSE)
      {
        /* Updates status */
        orxFLAG_SET(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_FULLSCREEN, orxDISPLAY_KU32_STATIC_FLAG_NONE);
      }
      else
      {
        /* Updates status */
        orxFLAG_SET(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_NONE, orxDISPLAY_KU32_STATIC_FLAG_FULLSCREEN);
      }

      /* Updates default mode */
      orxDisplay_GLFW_UpdateDefaultMode();

      /* Inits event payload */
      orxMemory_Zero(&stPayload, sizeof(orxDISPLAY_EVENT_PAYLOAD));
      stPayload.stVideoMode.u32Width                = (orxU32)iWidth;
      stPayload.stVideoMode.u32Height               = (orxU32)iHeight;
      stPayload.stVideoMode.u32Depth                = (orxU32)iDepth;
      stPayload.stVideoMode.u32RefreshRate          = sstDisplay.u32DefaultRefreshRate;
      stPayload.stVideoMode.u32PreviousWidth        = orxF2U(sstDisplay.pstScreen->fWidth);
      stPayload.stVideoMode.u32PreviousHeight       = orxF2U(sstDisplay.pstScreen->fHeight);
      stPayload.stVideoMode.u32PreviousDepth        = sstDisplay.pstScreen->u32Depth;
      stPayload.stVideoMode.u32PreviousRefreshRate  = sstDisplay.u32RefreshRate;
      stPayload.stVideoMode.bFullScreen             = _pstVideoMode->bFullScreen;

      /* Updates screen info */
      sstDisplay.pstScreen->fWidth          = orx2F(iWidth);
      sstDisplay.pstScreen->fHeight         = orx2F(iHeight);
      sstDisplay.pstScreen->u32RealWidth    = orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_NPOT) ? (orxU32)iWidth : orxMath_GetNextPowerOfTwo((orxU32)iWidth);
      sstDisplay.pstScreen->u32RealHeight   = orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_NPOT) ? (orxU32)iHeight : orxMath_GetNextPowerOfTwo((orxU32)iHeight);
      sstDisplay.pstScreen->u32Depth        = (orxU32)iDepth;
      sstDisplay.pstScreen->bSmoothing      = sstDisplay.bDefaultSmoothing;
      sstDisplay.pstScreen->fRecRealWidth   = orxFLOAT_1 / orxU2F(sstDisplay.pstScreen->u32RealWidth);
      sstDisplay.pstScreen->fRecRealHeight  = orxFLOAT_1 / orxU2F(sstDisplay.pstScreen->u32RealHeight);
      sstDisplay.pstScreen->u32DataSize     = sstDisplay.pstScreen->u32RealWidth * sstDisplay.pstScreen->u32RealHeight * 4 * sizeof(orxU8);
      orxVector_Copy(&(sstDisplay.pstScreen->stClip.vTL), &orxVECTOR_0);
      orxVector_Set(&(sstDisplay.pstScreen->stClip.vBR), sstDisplay.pstScreen->fWidth, sstDisplay.pstScreen->fHeight, orxFLOAT_0);

      /* Inits OpenGL */
      glEnable(GL_TEXTURE_2D);
      glASSERT();
      glEnable(GL_SCISSOR_TEST);
      glASSERT();
      glDisable(GL_DITHER);
      glASSERT();
      glDisable(GL_CULL_FACE);
      glASSERT();
      glDisable(GL_STENCIL_TEST);
      glASSERT();

#ifndef __orxDISPLAY_OPENGL_ES__

      glDisable(GL_ALPHA_TEST);
      glASSERT();
      glDisable(GL_LIGHTING);
      glASSERT();
      glDisable(GL_FOG);
      glASSERT();

#endif /* !__orxDISPLAY_OPENGL_ES__ */

      /* Has depth buffer? */
      if(orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_DEPTHBUFFER))
      {
        /* Enables depth test */
        glEnable(GL_DEPTH_TEST);
        glASSERT();

        /* Sets depth function */
        glDepthFunc(GL_LEQUAL);
        glASSERT();

        /* Clears depth buffer */
        glClearDepth(1.0f);
        glASSERT();
        glClear(GL_DEPTH_BUFFER_BIT);
        glASSERT();
      }
      else
      {
        /* Disables depth test */
        glDisable(GL_DEPTH_TEST);
        glASSERT();
      }

      /* Clears destination bitmap */
      sstDisplay.apstDestinationBitmapList[0] = orxNULL;
      sstDisplay.u32DestinationBitmapCount    = 1;

      /* Updates bound texture */
      sstDisplay.apstBoundBitmapList[sstDisplay.s32ActiveTextureUnit] = orxNULL;

      /* Gets corrected clip dimensions */
      u32ClipWidth  = orxF2U(orxMath_Round(sstDisplay.pstScreen->fWidth * sstDisplay.vContentScale.fX));
      u32ClipHeight = orxF2U(orxMath_Round(sstDisplay.pstScreen->fHeight * sstDisplay.vContentScale.fY));

      /* Clears new display surface */
      glScissor(0, 0, (GLsizei)u32ClipWidth, (GLsizei)u32ClipHeight);
      glASSERT();
      glClearColor(orxCOLOR_NORMALIZER * orxU2F(orxRGBA_R(sstDisplay.stLastColor)), orxCOLOR_NORMALIZER * orxU2F(orxRGBA_G(sstDisplay.stLastColor)), orxCOLOR_NORMALIZER * orxU2F(orxRGBA_B(sstDisplay.stLastColor)), orxCOLOR_NORMALIZER * orxU2F(orxRGBA_A(sstDisplay.stLastColor)));
      glASSERT();
      glClear(GL_COLOR_BUFFER_BIT);
      glASSERT();

      /* Stores clipping values */
      sstDisplay.u32LastClipX       = 0;
      sstDisplay.u32LastClipY       = 0;
      sstDisplay.u32LastClipWidth   = u32ClipWidth;
      sstDisplay.u32LastClipHeight  = u32ClipHeight;

      /* Enforces VSync status */
      orxDisplay_GLFW_EnableVSync(orxDisplay_GLFW_IsVSyncEnabled());

      /* Stores screen depth & refresh rate */
      sstDisplay.u32Depth       = (orxU32)iDepth;
      sstDisplay.u32RefreshRate = (orxU32)iRefreshRate;

      /* Sends event */
      orxEVENT_SEND(orxEVENT_TYPE_DISPLAY, orxDISPLAY_EVENT_SET_VIDEO_MODE, orxNULL, orxNULL, &stPayload);
    }
  }

  /* Successful? */
  if(eResult != orxSTATUS_FAILURE)
  {
    orxVECTOR vFramebufferSize;
    int       iFramebufferWidth, iFramebufferHeight;

    /* Isn't fullscreen? */
    if(!orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_FULLSCREEN))
    {
      GLFWmonitor *pstMonitor;

      /* Gets current monitor */
      pstMonitor = orxDisplay_GLFW_GetMonitor();

      /* Success? */
      if(pstMonitor != orxNULL)
      {
        int iX, iY;

        /* Gets its position */
        glfwGetMonitorPos(pstMonitor, &iX, &iY);

        /* Success? */
        if(glfwGetError(NULL) == GLFW_NO_ERROR)
        {
          /* Retrieves config position */
          orxConfig_GetVector(orxDISPLAY_KZ_CONFIG_POSITION, &(sstDisplay.vWindowPosition));

          /* Updates it */
          sstDisplay.vWindowPosition.fX += orx2F(iX);
          sstDisplay.vWindowPosition.fY += orx2F(iY);

          /* Updates window's position */
          glfwSetWindowPos(sstDisplay.pstWindow, (int)sstDisplay.vWindowPosition.fX, (int)sstDisplay.vWindowPosition.fY);
        }
      }
    }

    /* Updates cursor */
    orxDisplay_GLFW_UpdateCursor();

    /* Updates icons */
    orxDisplay_GLFW_UpdateIconList();

    /* Updates its title */
    glfwSetWindowTitle(sstDisplay.pstWindow, orxConfig_GetString(orxDISPLAY_KZ_CONFIG_TITLE));

    /* Updates active texture unit */
    sstDisplay.s32ActiveTextureUnit = 0;

    /* Has shader support? */
    if(orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_SHADER))
    {
      /* For all first 8 texture units */
      for(i = 0; i < (orxS32)orxMIN(8, sstDisplay.iTextureUnitNumber); i++)
      {
        /* Selects associated texture unit */
        glActiveTextureARB(GL_TEXTURE0_ARB + i);
        glASSERT();

        /* Resets it */
        glEnable(GL_TEXTURE_2D);
        glASSERT();
      }

      /* Selects first texture unit */
      glActiveTextureARB(GL_TEXTURE0_ARB);
      glASSERT();

      /* Uses default shader */
      orxDisplay_GLFW_StopShader(orxNULL);
    }

    /* Clears cache */
    sstDisplay.stLastColor          = orx2RGBA(0x00, 0x00, 0x00, 0x00);
    sstDisplay.iLastViewportX       = 0;
    sstDisplay.iLastViewportY       = 0;
    sstDisplay.iLastViewportWidth   = 0;
    sstDisplay.iLastViewportHeight  = 0;

    /* Has VBO support? */
    if(orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_VBO))
    {
      /* Binds them */
      glBindBufferARB(GL_ARRAY_BUFFER_ARB, sstDisplay.uiVertexBuffer);
      glASSERT();
      glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, sstDisplay.uiIndexBuffer);
      glASSERT();
    }

    /* Inits orthogonal projection values */
    sstDisplay.fLastOrthoRight  = (GLfloat)(sstDisplay.apstDestinationBitmapList[0] != orxNULL) ? sstDisplay.apstDestinationBitmapList[0]->fWidth : sstDisplay.pstScreen->fWidth;
    sstDisplay.fLastOrthoBottom = (GLfloat)(sstDisplay.apstDestinationBitmapList[0] != orxNULL)
                                  ? (sstDisplay.apstDestinationBitmapList[0] == sstDisplay.pstScreen)
                                    ? sstDisplay.apstDestinationBitmapList[0]->fHeight
                                    : -sstDisplay.apstDestinationBitmapList[0]->fHeight
                                  : sstDisplay.pstScreen->fHeight;

    /* Has VBO support? */
    if(orxFLAG_TEST_ALL(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_VBO | orxDISPLAY_KU32_STATIC_FLAG_SHADER))
    {
      /* Inits projection matrix */
      (sstDisplay.fLastOrthoBottom >= 0.0)
      ? orxDisplay_GLFW_OrthoProjMatrix(&(sstDisplay.mProjectionMatrix), orxFLOAT_0, sstDisplay.fLastOrthoRight, sstDisplay.fLastOrthoBottom, orxFLOAT_0, -orxFLOAT_1, orxFLOAT_1)
      : orxDisplay_GLFW_OrthoProjMatrix(&(sstDisplay.mProjectionMatrix), orxFLOAT_0, sstDisplay.fLastOrthoRight, orxFLOAT_0, -sstDisplay.fLastOrthoBottom, -orxFLOAT_1, orxFLOAT_1);

      /* Passes it to shader */
      glUNIFORM(Matrix4fvARB, sstDisplay.pstDefaultShader->iProjectionMatrixLocation, 1, GL_FALSE, (GLfloat *)&(sstDisplay.mProjectionMatrix.aafValueList[0][0]));

      /* Enables vertex attribute arrays */
      glEnableVertexAttribArrayARB(orxDISPLAY_ATTRIBUTE_LOCATION_VERTEX);
      glASSERT();
      glEnableVertexAttribArrayARB(orxDISPLAY_ATTRIBUTE_LOCATION_TEXCOORD);
      glASSERT();
      glEnableVertexAttribArrayARB(orxDISPLAY_ATTRIBUTE_LOCATION_COLOR);
      glASSERT();

      /* Sets vertex attribute arrays */
      glVertexAttribPointerARB(orxDISPLAY_ATTRIBUTE_LOCATION_VERTEX, 2, GL_FLOAT, GL_FALSE, sizeof(orxDISPLAY_VERTEX), (GLvoid *)offsetof(orxDISPLAY_GLFW_VERTEX, fX));
      glASSERT();
      glVertexAttribPointerARB(orxDISPLAY_ATTRIBUTE_LOCATION_TEXCOORD, 2, GL_FLOAT, GL_FALSE, sizeof(orxDISPLAY_VERTEX), (GLvoid *)offsetof(orxDISPLAY_GLFW_VERTEX, fU));
      glASSERT();
      glVertexAttribPointerARB(orxDISPLAY_ATTRIBUTE_LOCATION_COLOR, 4, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(orxDISPLAY_VERTEX), (GLvoid *)offsetof(orxDISPLAY_GLFW_VERTEX, stRGBA));
      glASSERT();
    }
#ifndef __orxDISPLAY_OPENGL_ES__

    else
    {
      /* Inits matrices */
      glMatrixMode(GL_PROJECTION);
      glASSERT();
      glLoadIdentity();
      glASSERT();
      (sstDisplay.fLastOrthoBottom >= 0.0)
      ? glOrtho(0.0f, (GLdouble)sstDisplay.fLastOrthoRight, (GLdouble)sstDisplay.fLastOrthoBottom, 0.0f, -1.0f, 1.0f)
      : glOrtho(0.0f, (GLdouble)sstDisplay.fLastOrthoRight, 0.0f, -(GLdouble)sstDisplay.fLastOrthoBottom, -1.0f, 1.0f);
      glASSERT();
      glMatrixMode(GL_MODELVIEW);
      glASSERT();
      glLoadIdentity();
      glASSERT();

      /* Resets client states */
      glEnableClientState(GL_VERTEX_ARRAY);
      glASSERT();
      glEnableClientState(GL_TEXTURE_COORD_ARRAY);
      glASSERT();
      glEnableClientState(GL_COLOR_ARRAY);
      glASSERT();

      /* Selects arrays */
      glVertexPointer(2, GL_FLOAT, sizeof(orxDISPLAY_VERTEX), orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_VBO) ? (GLvoid *)offsetof(orxDISPLAY_GLFW_VERTEX, fX) : &(sstDisplay.astVertexList[0].fX));
      glASSERT();
      glTexCoordPointer(2, GL_FLOAT, sizeof(orxDISPLAY_VERTEX), orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_VBO) ? (GLvoid *)offsetof(orxDISPLAY_GLFW_VERTEX, fU) : &(sstDisplay.astVertexList[0].fU));
      glASSERT();
      glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(orxDISPLAY_VERTEX), orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_VBO) ? (GLvoid *)offsetof(orxDISPLAY_GLFW_VERTEX, stRGBA) : &(sstDisplay.astVertexList[0].stRGBA));
      glASSERT();
    }

#ifndef __orxMAC__
    /* Is OpenGL debug output requested? */
    if(orxConfig_GetBool(orxDISPLAY_KZ_CONFIG_DEBUG_OUTPUT) != orxFALSE)
    {
      /* Is OpenGL debug output available? */
      if(orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_DEBUG_OUTPUT))
      {
        /* Enables OpenGL debug output */
        glEnable(GL_DEBUG_OUTPUT);
        glASSERT();
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glASSERT();

        /* Sets debug message callback */
        glDebugMessageCallback(orxDisplay_GLFW_MessageCallback, NULL);
        glASSERT();
      }
      else
      {
        /* Disables OpenGL debug output */
        glDisable(GL_DEBUG_OUTPUT);
        glASSERT();
        glDisable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glASSERT();

        /* Updates status */
        orxConfig_SetBool(orxDISPLAY_KZ_CONFIG_DEBUG_OUTPUT, orxFALSE);
      }
    }
    else
    {
      /* Disables OpenGL debug output */
      glDisable(GL_DEBUG_OUTPUT);
      glASSERT();
      glDisable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
      glASSERT();
    }
#endif /* __orxMAC__ */

#endif /* !__orxDISPLAY_OPENGL_ES__ */

    /* Gets framebuffer size */
    glfwGetFramebufferSize(sstDisplay.pstWindow, &iFramebufferWidth, &iFramebufferHeight);
    orxVector_Set(&vFramebufferSize, orx2F(iFramebufferWidth), orx2F(iFramebufferHeight), orxFLOAT_0);

    /* Stores config values */
    orxConfig_SetFloat(orxDISPLAY_KZ_CONFIG_WIDTH, sstDisplay.pstScreen->fWidth);
    orxConfig_SetFloat(orxDISPLAY_KZ_CONFIG_HEIGHT, sstDisplay.pstScreen->fHeight);
    orxConfig_SetU32(orxDISPLAY_KZ_CONFIG_DEPTH, sstDisplay.pstScreen->u32Depth);
    orxConfig_SetU32(orxDISPLAY_KZ_CONFIG_REFRESH_RATE, sstDisplay.u32RefreshRate);
    orxConfig_SetVector(orxDISPLAY_KZ_CONFIG_FRAMEBUFFER_SIZE, &vFramebufferSize);
  }

  /* For all texture units */
  for(i = 0; i < (orxS32)sstDisplay.iTextureUnitNumber; i++)
  {
    /* Clears its bound bitmap */
    sstDisplay.apstBoundBitmapList[i] = orxNULL;

    /* Clears its MRU timestamp */
    sstDisplay.adMRUBitmapList[i]     = orxDOUBLE_0;
  }

  /* Clears last modes */
  sstDisplay.eLastBlendMode = orxDISPLAY_BLEND_MODE_NUMBER;
  sstDisplay.eLastBufferMode= orxDISPLAY_BUFFER_MODE_NUMBER;

  /* Resets primitive */
  sstDisplay.ePrimitive     = orxDISPLAY_KE_DEFAULT_PRIMITIVE;

  /* Pops config section */
  orxConfig_PopSection();

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_GLFW_SetFullScreen(orxBOOL _bFullScreen)
{
  orxBOOL   bUpdate = orxFALSE;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Enable? */
  if(_bFullScreen != orxFALSE)
  {
    /* Wasn't already full screen? */
    if(!orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_FULLSCREEN))
    {
      /* Asks for update */
      bUpdate = orxTRUE;
    }
  }
  else
  {
    /* Was full screen? */
    if(orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_FULLSCREEN))
    {
      /* Asks for update */
      bUpdate = orxTRUE;
    }
  }

  /* Should update? */
  if(bUpdate != orxFALSE)
  {
    orxDISPLAY_VIDEO_MODE stVideoMode;

    /* Inits content scale */
    orxVector_Copy(&(sstDisplay.vContentScale), &orxVECTOR_1);

    /* Inits video mode */
    stVideoMode.u32Width        = orxF2U(sstDisplay.pstScreen->fWidth);
    stVideoMode.u32Height       = orxF2U(sstDisplay.pstScreen->fHeight);
    stVideoMode.u32Depth        = sstDisplay.pstScreen->u32Depth;
    stVideoMode.u32RefreshRate  = sstDisplay.u32RefreshRate;
    stVideoMode.bFullScreen     = _bFullScreen;

    /* Updates video mode */
    eResult = orxDisplay_GLFW_SetVideoMode(&stVideoMode);

    /* Failed? */
    if(eResult == orxSTATUS_FAILURE)
    {
      /* Restores previous full screen status */
      stVideoMode.bFullScreen = !stVideoMode.bFullScreen;

      /* Updates video mode */
      orxDisplay_GLFW_SetVideoMode(&stVideoMode);
    }
  }

  /* Done! */
  return eResult;
}

orxBOOL orxFASTCALL orxDisplay_GLFW_IsFullScreen()
{
  orxBOOL bResult;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Updates result */
  bResult = orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_FULLSCREEN) ? orxTRUE : orxFALSE;

  /* Done! */
  return bResult;
}

orxSTATUS orxFASTCALL orxDisplay_GLFW_Init()
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Was not already initialized? */
  if(!(sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY))
  {
    orxU32 i;
    GLushort u16Index;

    /* Cleans static controller */
    orxMemory_Zero(&sstDisplay, sizeof(orxDISPLAY_STATIC));

    /* Resets screen depth & refresh rate */
    sstDisplay.u32Depth       = orxU32_UNDEFINED;
    sstDisplay.u32RefreshRate = orxU32_UNDEFINED;

    /* Stores stbi callbacks */
    sstDisplay.stSTBICallbacks.read = orxDisplay_GLFW_ReadSTBICallback;
    sstDisplay.stSTBICallbacks.skip = orxDisplay_GLFW_SkipSTBICallback;
    sstDisplay.stSTBICallbacks.eof  = orxDisplay_GLFW_EOFSTBICallback;

    /* For all indices */
    for(i = 0, u16Index = 0; i < orxDISPLAY_KU32_INDEX_BUFFER_SIZE; i += 6, u16Index += 4)
    {
      /* Computes them */
      sstDisplay.au16IndexList[i]     = u16Index;
      sstDisplay.au16IndexList[i + 1] = u16Index + 1;
      sstDisplay.au16IndexList[i + 2] = u16Index + 2;
      sstDisplay.au16IndexList[i + 3] = u16Index + 1;
      sstDisplay.au16IndexList[i + 4] = u16Index + 3;
      sstDisplay.au16IndexList[i + 5] = u16Index + 2;
    }

    /* Inits GLFW */
    eResult = (glfwInit() != GLFW_FALSE) ? orxSTATUS_SUCCESS : orxSTATUS_FAILURE;

    /* Valid? */
    if(eResult != orxSTATUS_FAILURE)
    {
      /* Adds event handlers */
      eResult = orxEvent_AddHandler(orxEVENT_TYPE_RENDER, orxDisplay_GLFW_EventHandler);
      eResult = (eResult != orxSTATUS_FAILURE) ? orxEvent_AddHandler(orxEVENT_TYPE_SYSTEM, orxDisplay_GLFW_EventHandler) : orxSTATUS_FAILURE;
      eResult = (eResult != orxSTATUS_FAILURE) ? orxEvent_AddHandler(orxEVENT_TYPE_FIRST_RESERVED, orxDisplay_GLFW_EventHandler) : orxSTATUS_FAILURE;

      /* Success? */
      if(eResult != orxSTATUS_FAILURE)
      {
        /* Filters relevant event IDs */
        orxEvent_SetHandlerIDFlags(orxDisplay_GLFW_EventHandler, orxEVENT_TYPE_RENDER, orxNULL, orxEVENT_GET_FLAG(orxRENDER_EVENT_STOP), orxEVENT_KU32_MASK_ID_ALL);
        orxEvent_SetHandlerIDFlags(orxDisplay_GLFW_EventHandler, orxEVENT_TYPE_SYSTEM, orxNULL, orxEVENT_GET_FLAG(orxSYSTEM_EVENT_CLIPBOARD), orxEVENT_KU32_MASK_ID_ALL);

        /* Creates banks */
        sstDisplay.pstBitmapBank  = orxBank_Create(orxDISPLAY_KU32_BITMAP_BANK_SIZE, sizeof(orxBITMAP), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);
        sstDisplay.pstShaderBank  = orxBank_Create(orxDISPLAY_KU32_SHADER_BANK_SIZE, sizeof(orxDISPLAY_SHADER), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

        /* Valid? */
        if((sstDisplay.pstBitmapBank != orxNULL)
        && (sstDisplay.pstShaderBank != orxNULL))
        {
          orxDISPLAY_VIDEO_MODE stVideoMode;

          /* Inits Basis Universal */
          BasisU_Init();

          /* Updates default mode */
          orxDisplay_GLFW_UpdateDefaultMode();

          /* Pushes display section */
          orxConfig_PushSection(orxDISPLAY_KZ_CONFIG_SECTION);

          /* Gets resolution from config */
          stVideoMode.u32Width        = orxConfig_HasValue(orxDISPLAY_KZ_CONFIG_WIDTH) ? orxConfig_GetU32(orxDISPLAY_KZ_CONFIG_WIDTH) : sstDisplay.u32DefaultWidth;
          stVideoMode.u32Height       = orxConfig_HasValue(orxDISPLAY_KZ_CONFIG_HEIGHT) ? orxConfig_GetU32(orxDISPLAY_KZ_CONFIG_HEIGHT) : sstDisplay.u32DefaultHeight;
          stVideoMode.u32Depth        = orxConfig_HasValue(orxDISPLAY_KZ_CONFIG_DEPTH) ? orxConfig_GetU32(orxDISPLAY_KZ_CONFIG_DEPTH) : sstDisplay.u32DefaultDepth;
          stVideoMode.u32RefreshRate  = orxConfig_HasValue(orxDISPLAY_KZ_CONFIG_REFRESH_RATE) ? orxConfig_GetU32(orxDISPLAY_KZ_CONFIG_REFRESH_RATE) : sstDisplay.u32DefaultRefreshRate;
          stVideoMode.bFullScreen     = orxConfig_GetBool(orxDISPLAY_KZ_CONFIG_FULLSCREEN);

          /* Sets module as ready */
          sstDisplay.u32Flags = orxDISPLAY_KU32_STATIC_FLAG_READY;

          /* Has decoration? */
          if((orxConfig_HasValue(orxDISPLAY_KZ_CONFIG_DECORATION) == orxFALSE) || (orxConfig_GetBool(orxDISPLAY_KZ_CONFIG_DECORATION) != orxFALSE))
          {
            /* Stores default decorated position */
            orxVector_Set(&(sstDisplay.vWindowPosition), orxDISPLAY_KV_DEFAULT_DECORATED_POSITION);
          }

          /* Allocates screen bitmap */
          sstDisplay.pstScreen = (orxBITMAP *)orxBank_Allocate(sstDisplay.pstBitmapBank);
          orxMemory_Zero(sstDisplay.pstScreen, sizeof(orxBITMAP));

          /* Sets video mode? */
          if((eResult = orxDisplay_GLFW_SetVideoMode(&stVideoMode)) == orxSTATUS_FAILURE)
          {
            /* Updates resolution */
            stVideoMode.u32Width        = sstDisplay.u32DefaultWidth;
            stVideoMode.u32Height       = sstDisplay.u32DefaultHeight;
            stVideoMode.u32Depth        = sstDisplay.u32DefaultDepth;
            stVideoMode.u32RefreshRate  = sstDisplay.u32DefaultRefreshRate;
            stVideoMode.bFullScreen     = orxFALSE;

            /* Sets video mode using default parameters */
            eResult = orxDisplay_GLFW_SetVideoMode(&stVideoMode);
          }

          /* Valid? */
          if(eResult != orxSTATUS_FAILURE)
          {
            orxCLOCK *pstClock;

            /* Has VSync value? */
            if(orxConfig_HasValue(orxDISPLAY_KZ_CONFIG_VSYNC) != orxFALSE)
            {
              /* Updates vertical sync */
              orxDisplay_GLFW_EnableVSync(orxConfig_GetBool(orxDISPLAY_KZ_CONFIG_VSYNC));
            }
            else
            {
              /* Enables vertical sync */
              orxDisplay_GLFW_EnableVSync(orxTRUE);
            }

            /* Only allows delayed VSync to fix a busy loop issue in some graphics drivers */
            orxClock_AddGlobalTimer(orxDisplay_GLFW_VSyncFix, orxDISPLAY_KF_VSYNC_DELAY_FIX, 1, orxNULL);

            /* Inits info */
            sstDisplay.bDefaultSmoothing  = orxConfig_GetBool(orxDISPLAY_KZ_CONFIG_SMOOTH);
            sstDisplay.eLastBlendMode     = orxDISPLAY_BLEND_MODE_NUMBER;
            sstDisplay.eLastBufferMode    = orxDISPLAY_BUFFER_MODE_NUMBER;
            sstDisplay.ePrimitive         = orxDISPLAY_KE_DEFAULT_PRIMITIVE;

            /* Gets clock */
            pstClock = orxClock_Get(orxCLOCK_KZ_CORE);

            /* Registers update function */
            if((pstClock != orxNULL) && ((eResult = orxClock_Register(pstClock, orxDisplay_GLFW_Update, orxNULL, orxMODULE_ID_DISPLAY, orxCLOCK_PRIORITY_HIGHER)) != orxSTATUS_FAILURE))
            {
              int iWindowX = 0, iWindowY = 0;

              /* Ignores resize event for now */
              sstDisplay.u32Flags |= orxDISPLAY_KU32_STATIC_FLAG_IGNORE_EVENT;

              /* Registers resize callback */
              glfwSetWindowSizeCallback(sstDisplay.pstWindow, orxDisplay_GLFW_ResizeCallback);

              /* Registers content scale callback */
              glfwSetWindowContentScaleCallback(sstDisplay.pstWindow, orxDisplay_GLFW_ContentScaleCallback);

              /* Registers drop callback */
              glfwSetDropCallback(sstDisplay.pstWindow, orxDisplay_GLFW_DropCallback);

              /* Registers position callback */
              glfwSetWindowPosCallback(sstDisplay.pstWindow, orxDisplay_GLFW_PosCallback);

              /* Reactivates resize event */
              sstDisplay.u32Flags &= ~orxDISPLAY_KU32_STATIC_FLAG_IGNORE_EVENT;

              /* Gets window position */
              glfwGetWindowPos(sstDisplay.pstWindow, &iWindowX, &iWindowY);

              /* Forces a position update */
              orxDisplay_GLFW_PosCallback(sstDisplay.pstWindow, iWindowX, iWindowY);
            }
            else
            {
              /* Terminates GLFW */
              glfwTerminate();

              /* Frees screen bitmap */
              orxBank_Free(sstDisplay.pstBitmapBank, sstDisplay.pstScreen);

              /* Deletes banks */
              orxBank_Delete(sstDisplay.pstBitmapBank);
              sstDisplay.pstBitmapBank = orxNULL;
              orxBank_Delete(sstDisplay.pstShaderBank);
              sstDisplay.pstShaderBank = orxNULL;

              /* Updates status */
              orxFLAG_SET(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_NONE, orxDISPLAY_KU32_STATIC_FLAG_READY);

              /* Logs message */
              orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Failed to register GLFW/display event handler.");
            }
          }
          else
          {
            /* Terminates GLFW */
            glfwTerminate();

            /* Frees screen bitmap */
            orxBank_Free(sstDisplay.pstBitmapBank, sstDisplay.pstScreen);

            /* Deletes banks */
            orxBank_Delete(sstDisplay.pstBitmapBank);
            sstDisplay.pstBitmapBank = orxNULL;
            orxBank_Delete(sstDisplay.pstShaderBank);
            sstDisplay.pstShaderBank = orxNULL;

            /* Updates status */
            orxFLAG_SET(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_NONE, orxDISPLAY_KU32_STATIC_FLAG_READY);

            /* Logs message */
            orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Failed to init GLFW/OpenGL default video mode.");
          }

          /* Pops config section */
          orxConfig_PopSection();
        }
        else
        {
          /* Deletes banks */
          if(sstDisplay.pstBitmapBank != orxNULL)
          {
            orxBank_Delete(sstDisplay.pstBitmapBank);
            sstDisplay.pstBitmapBank = orxNULL;
          }
          if(sstDisplay.pstShaderBank != orxNULL)
          {
            orxBank_Delete(sstDisplay.pstShaderBank);
            sstDisplay.pstShaderBank = orxNULL;
          }

          /* Exits from GLFW */
          glfwTerminate();

          /* Logs message */
          orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Failed to create bitmap/shader banks.");
        }
      }
      else
      {
        /* Exits from GLFW */
        glfwTerminate();

        /* Removes event handlers */
        orxEvent_RemoveHandler(orxEVENT_TYPE_RENDER, orxDisplay_GLFW_EventHandler);
        orxEvent_RemoveHandler(orxEVENT_TYPE_SYSTEM, orxDisplay_GLFW_EventHandler);
        orxEvent_RemoveHandler(orxEVENT_TYPE_FIRST_RESERVED, orxDisplay_GLFW_EventHandler);

        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Failed to register event handlers.");
      }
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Failed to init GLFW.");
    }
  }

  /* Done! */
  return eResult;
}

void orxFASTCALL orxDisplay_GLFW_Exit()
{
  /* Was initialized? */
  if(sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY)
  {
    /* Exits from Basis Universal */
    BasisU_Exit();

    /* Removes VSync fix (to account for rapid exit) */
    orxClock_RemoveGlobalTimer(orxDisplay_GLFW_VSyncFix, -orxFLOAT_1, orxNULL);

    /* Has shader support? */
    if(orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_SHADER))
    {
      /* Deletes default shaders */
      orxDisplay_DeleteShader(sstDisplay.pstDefaultShader);
      orxDisplay_DeleteShader(sstDisplay.pstNoTextureShader);
    }

    /* Has cursor? */
    if(sstDisplay.pstCursor != NULL)
    {
      /* Deletes it */
      glfwDestroyCursor(sstDisplay.pstCursor);
    }

    /* Exits from GLFW */
    glfwTerminate();

    /* Removes event handlers */
    orxEvent_RemoveHandler(orxEVENT_TYPE_RENDER, orxDisplay_GLFW_EventHandler);
    orxEvent_RemoveHandler(orxEVENT_TYPE_SYSTEM, orxDisplay_GLFW_EventHandler);
    orxEvent_RemoveHandler(orxEVENT_TYPE_FIRST_RESERVED, orxDisplay_GLFW_EventHandler);

    /* Unregisters update function */
    orxClock_Unregister(orxClock_Get(orxCLOCK_KZ_CORE), orxDisplay_GLFW_Update);

    /* Deletes banks */
    orxBank_Delete(sstDisplay.pstBitmapBank);
    orxBank_Delete(sstDisplay.pstShaderBank);

    /* Cleans static controller */
    orxMemory_Zero(&sstDisplay, sizeof(orxDISPLAY_STATIC));
  }

  return;
}

orxBOOL orxFASTCALL orxDisplay_GLFW_HasShaderSupport()
{
  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Done! */
  return (orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_SHADER)) ? orxTRUE : orxFALSE;
}

orxHANDLE orxFASTCALL orxDisplay_GLFW_CreateShader(const orxSTRING *_azCodeList, orxU32 _u32Size, const orxLINKLIST *_pstParamList, orxBOOL _bUseCustomParam)
{
  orxHANDLE hResult = orxHANDLE_UNDEFINED;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Has shader support? */
  if(orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_SHADER))
  {
    /* Valid? */
    if((_azCodeList != orxNULL) && (_u32Size > 0))
    {
      orxDISPLAY_SHADER *pstShader;

      /* Creates a new shader */
      pstShader = (orxDISPLAY_SHADER *)orxBank_Allocate(sstDisplay.pstShaderBank);

      /* Successful? */
      if(pstShader != orxNULL)
      {
        orxCHAR  *pc, *pcReplace;
        orxS32    s32Offset, s32Free;
        orxU32    i;

        /* Inits shader code buffer */
        sstDisplay.acShaderCodeBuffer[0]  = sstDisplay.acShaderCodeBuffer[orxDISPLAY_KU32_SHADER_BUFFER_SIZE - 1] = orxCHAR_NULL;
        pc                                = sstDisplay.acShaderCodeBuffer;
        s32Free                           = orxDISPLAY_KU32_SHADER_BUFFER_SIZE - 1;

        /* Pushes display config section */
        orxConfig_PushSection(orxDISPLAY_KZ_CONFIG_SECTION);

        /* Has shader version? */
        if(orxConfig_HasValue(orxDISPLAY_KZ_CONFIG_SHADER_VERSION) != orxFALSE)
        {
          orxU32 u32ShaderVersion;

          /* Gets it */
          u32ShaderVersion = orxConfig_GetU32(orxDISPLAY_KZ_CONFIG_SHADER_VERSION);

          /* Valid? */
          if(u32ShaderVersion != 0)
          {
            /* Is shader version too high? */
            if(u32ShaderVersion > orxDISPLAY_KU32_MAX_SHADER_VERSION)
            {
              /* Updates it */
              u32ShaderVersion = orxDISPLAY_KU32_MAX_SHADER_VERSION;

              /* Enforces it */
              orxConfig_SetU32(orxDISPLAY_KZ_CONFIG_SHADER_VERSION, u32ShaderVersion);
            }

#ifdef __orxDISPLAY_OPENGL_ES__

            /* Prints shader version */
            s32Offset = orxString_NPrint(pc, s32Free, "#version %u es\n", u32ShaderVersion);

#else /* __orxDISPLAY_OPENGL_ES__ */

            /* Prints shader version */
            s32Offset = orxString_NPrint(pc, s32Free, "#version %u\n", u32ShaderVersion);

#endif /* __orxDISPLAY_OPENGL_ES__ */

            pc       += s32Offset;
            s32Free  -= s32Offset;
          }
        }

        /* Has shader extension list? */
        if(orxConfig_HasValue(orxDISPLAY_KZ_CONFIG_SHADER_EXTENSION_LIST) != orxFALSE)
        {
          orxS32 i, s32Count;

          /* For all extensions */
          for(i = 0, s32Count = orxConfig_GetListCount(orxDISPLAY_KZ_CONFIG_SHADER_EXTENSION_LIST);
              i < s32Count;
              i++)
          {
            orxS32          s32Offset;
            orxBOOL         bAdd;
            const orxSTRING zExtension;

            /* Gets it */
            zExtension = orxConfig_GetListString(orxDISPLAY_KZ_CONFIG_SHADER_EXTENSION_LIST, i);

            /* Depending on character */
            switch(*zExtension)
            {
              case orxDISPLAY_KC_SHADER_EXTENSION_REMOVE:
              {
                /* Updates extension string */
                zExtension++;

                /* Sets action */
                bAdd = orxFALSE;

                break;
              }

              case orxDISPLAY_KC_SHADER_EXTENSION_ADD:
              {
                /* Updates extension string */
                zExtension++;

                /* Fall through */
              }

              default:
              {
                /* Sets action */
                bAdd = orxTRUE;

                break;
              }
            }

            /* Prints it */
            s32Offset = orxString_NPrint(pc, s32Free, "#extension %s : %s\n", zExtension, (bAdd != orxFALSE) ? "enable" : "disable");
            pc       += s32Offset;
            s32Free  -= s32Offset;
          }
        }

#ifndef __orxDISPLAY_OPENGL_ES__

        else
        {
          /* Uses GPU shader 4 extension by default */
          s32Offset  = orxString_NPrint(pc, s32Free, "#extension GL_EXT_gpu_shader4 : enable\n");
          pc        += s32Offset;
          s32Free   -= s32Offset;
        }

#endif /* !__orxDISPLAY_OPENGL_ES__ */

        /* Pops config section */
        orxConfig_PopSection();

        /* Has parameters? */
        if(_pstParamList != orxNULL)
        {
          orxSHADER_PARAM *pstParam;

          /* Adds wrapping code */
#ifdef __orxDISPLAY_OPENGL_ES__
          s32Offset = orxString_NPrint(pc, s32Free, "precision mediump float;\nvarying vec2 _gl_TexCoord0_;\nvarying vec4 _Color0_;\n");
#else /* __orxDISPLAY_OPENGL_ES__ */
          s32Offset = orxString_NPrint(pc, s32Free, "varying vec2 _gl_TexCoord0_;\nvarying vec4 _Color0_;\n");
#endif /* __orxDISPLAY_OPENGL_ES__ */
          pc       += s32Offset;
          s32Free  -= s32Offset;

          /* For all parameters */
          for(pstParam = (orxSHADER_PARAM *)orxLinkList_GetFirst(_pstParamList);
              pstParam != orxNULL;
              pstParam = (orxSHADER_PARAM *)orxLinkList_GetNext(&(pstParam->stNode)))
          {
            /* Depending on type */
            switch(pstParam->eType)
            {
              case orxSHADER_PARAM_TYPE_FLOAT:
              case orxSHADER_PARAM_TYPE_TIME:
              {
                /* Adds its literal value */
                s32Offset = (pstParam->u32ArraySize >= 1) ? orxString_NPrint(pc, s32Free, "uniform float %s[%u];\n", pstParam->zName, pstParam->u32ArraySize) : orxString_NPrint(pc, s32Free, "uniform float %s;\n", pstParam->zName);
                pc       += s32Offset;
                s32Free  -= s32Offset;

                break;
              }

              case orxSHADER_PARAM_TYPE_TEXTURE:
              {
                /* Adds its literal value and automated coordinates */
                s32Offset = (pstParam->u32ArraySize >= 1) ? orxString_NPrint(pc, s32Free, "uniform sampler2D %s[%u];\nuniform float %s" orxDISPLAY_KZ_SHADER_SUFFIX_TOP "[%u];\nuniform float %s" orxDISPLAY_KZ_SHADER_SUFFIX_LEFT "[%u];\nuniform float %s" orxDISPLAY_KZ_SHADER_SUFFIX_BOTTOM "[%u];\nuniform float %s" orxDISPLAY_KZ_SHADER_SUFFIX_RIGHT "[%u];\n", pstParam->zName, pstParam->u32ArraySize, pstParam->zName, pstParam->u32ArraySize, pstParam->zName, pstParam->u32ArraySize, pstParam->zName, pstParam->u32ArraySize, pstParam->zName, pstParam->u32ArraySize) : orxString_NPrint(pc, s32Free, "uniform sampler2D %s;\nuniform float %s" orxDISPLAY_KZ_SHADER_SUFFIX_TOP ";\nuniform float %s" orxDISPLAY_KZ_SHADER_SUFFIX_LEFT ";\nuniform float %s" orxDISPLAY_KZ_SHADER_SUFFIX_BOTTOM ";\nuniform float %s" orxDISPLAY_KZ_SHADER_SUFFIX_RIGHT ";\n", pstParam->zName, pstParam->zName, pstParam->zName, pstParam->zName, pstParam->zName);
                pc       += s32Offset;
                s32Free  -= s32Offset;

                break;
              }

              case orxSHADER_PARAM_TYPE_VECTOR:
              {
                /* Adds its literal value */
                s32Offset = (pstParam->u32ArraySize >= 1) ? orxString_NPrint(pc, s32Free, "uniform vec3 %s[%u];\n", pstParam->zName, pstParam->u32ArraySize) : orxString_NPrint(pc, s32Free, "uniform vec3 %s;\n", pstParam->zName);
                pc       += s32Offset;
                s32Free  -= s32Offset;

                break;
              }

              default:
              {
                break;
              }
            }
          }
        }

        /* Adds line directive */
        s32Offset = orxString_NPrint(pc, s32Free, "#line 0\n");
        pc       += s32Offset;
        s32Free  -= s32Offset;

        /* Adds all code fragments */
        for(i = 0; i < _u32Size; i++)
        {
          s32Offset = orxString_NPrint(pc, s32Free, "%s\n", _azCodeList[i]);
          pc       += s32Offset;
          s32Free  -= s32Offset;
        }

        /* For all gl_TexCoord[0] */
        for(pcReplace = (orxCHAR *)orxString_SearchString(sstDisplay.acShaderCodeBuffer, "gl_TexCoord[0]");
            pcReplace != orxNULL;
            pcReplace = (orxCHAR *)orxString_SearchString(pcReplace + 14 * sizeof(orxCHAR), "gl_TexCoord[0]"))
        {
          /* Replaces it */
          orxMemory_Copy(pcReplace, "_gl_TexCoord0_", 14 * sizeof(orxCHAR));
        }

        /* For all gl_Color */
        for(pcReplace = (orxCHAR *)orxString_SearchString(sstDisplay.acShaderCodeBuffer, "gl_Color");
            pcReplace != orxNULL;
            pcReplace = (orxCHAR *)orxString_SearchString(pcReplace + 8 * sizeof(orxCHAR), "gl_Color"))
        {
          /* Replaces it */
          orxMemory_Copy(pcReplace, "_Color0_", 8 * sizeof(orxCHAR));
        }

        /* Inits shader */
        orxMemory_Zero(&(pstShader->stNode), sizeof(orxLINKLIST_NODE));
        pstShader->hProgram               = (GLhandleARB)orxU32_UNDEFINED;
        pstShader->iTextureCount          = 0;
        pstShader->s32ParamCount          = 0;
        pstShader->bPending               = orxFALSE;
        pstShader->bUseCustomParam        = _bUseCustomParam;
        pstShader->zCode                  = orxString_Duplicate(sstDisplay.acShaderCodeBuffer);
        pstShader->astTextureInfoList     = (orxDISPLAY_TEXTURE_INFO *)orxMemory_Allocate(sstDisplay.iTextureUnitNumber * sizeof(orxDISPLAY_TEXTURE_INFO), orxMEMORY_TYPE_MAIN);
        pstShader->astParamInfoList       = (orxDISPLAY_PARAM_INFO *)orxMemory_Allocate(sstDisplay.iTextureUnitNumber * sizeof(orxDISPLAY_PARAM_INFO), orxMEMORY_TYPE_MAIN);
        orxMemory_Zero(pstShader->astTextureInfoList, sstDisplay.iTextureUnitNumber * sizeof(orxDISPLAY_TEXTURE_INFO));
        orxMemory_Zero(pstShader->astParamInfoList, sstDisplay.iTextureUnitNumber * sizeof(orxDISPLAY_PARAM_INFO));

        /* Compiles it */
        if(orxDisplay_GLFW_CompileShader(pstShader) != orxSTATUS_FAILURE)
        {
          /* Updates result */
          hResult = (orxHANDLE)pstShader;
        }
        else
        {
          /* Deletes code */
          orxString_Delete(pstShader->zCode);

          /* Deletes texture info list */
          orxMemory_Free(pstShader->astTextureInfoList);

          /* Deletes param info list */
          orxMemory_Free(pstShader->astParamInfoList);

          /* Frees shader */
          orxBank_Free(sstDisplay.pstShaderBank, pstShader);
        }
      }
    }
  }

  /* Done! */
  return hResult;
}

void orxFASTCALL orxDisplay_GLFW_DeleteShader(orxHANDLE _hShader)
{
  orxDISPLAY_SHADER *pstShader;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT((_hShader != orxHANDLE_UNDEFINED) && (_hShader != orxNULL));

  /* Gets shader */
  pstShader = (orxDISPLAY_SHADER *)_hShader;

  /* Deletes its program */
  glDeleteProgram(pstShader->hProgram);
  glASSERT();

  /* Deletes its code */
  orxString_Delete(pstShader->zCode);

  /* Deletes its texture info list */
  orxMemory_Free(pstShader->astTextureInfoList);

  /* Deletes its param info list */
  orxMemory_Free(pstShader->astParamInfoList);

  /* Frees it */
  orxBank_Free(sstDisplay.pstShaderBank, pstShader);

  return;
}

orxSTATUS orxFASTCALL orxDisplay_GLFW_StartShader(orxHANDLE _hShader)
{
  orxDISPLAY_SHADER  *pstShader;
  orxSTATUS           eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT((_hShader != orxHANDLE_UNDEFINED) && (_hShader != orxNULL));

  /* Gets shader */
  pstShader = (orxDISPLAY_SHADER *)_hShader;

  /* Not pending or use custom param? */
  if((pstShader->bPending == orxFALSE) || (pstShader->bUseCustomParam != orxFALSE))
  {
    /* Draw remaining items */
    orxDisplay_GLFW_DrawArrays();

    /* Adds it to the active list */
    orxLinkList_AddEnd(&(sstDisplay.stActiveShaderList), &(pstShader->stNode));
  }
  else
  {
    /* Resets its pending status */
    pstShader->bPending = orxFALSE;

    /* Updates count */
    sstDisplay.s32PendingShaderCount--;
  }

  /* Uses its program */
  glUseProgramObjectARB(pstShader->hProgram);
  glASSERT();

  /* Updates projection matrix */
  glUNIFORM(Matrix4fvARB, pstShader->iProjectionMatrixLocation, 1, GL_FALSE, (GLfloat *)&(sstDisplay.mProjectionMatrix.aafValueList[0][0]));

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_GLFW_StopShader(orxHANDLE _hShader)
{
  orxDISPLAY_SHADER  *pstShader;
  orxBOOL             bResetShader = orxTRUE;
  orxSTATUS           eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_hShader != orxHANDLE_UNDEFINED);

  /* Gets shader */
  pstShader = (orxDISPLAY_SHADER *)_hShader;

  /* Has shader? */
  if(pstShader != orxNULL)
  {
    /* Not already removed? */
    if((orxLinkList_GetList(&(pstShader->stNode)) != orxNULL)
    && (pstShader->bPending == orxFALSE))
    {
      /* Empty buffer? */
      if(sstDisplay.s32BufferIndex == 0)
      {
        orxDISPLAY_SHADER *pstActive, *pstNextActive;

        /* For all active shaders */
        for(pstActive = (orxDISPLAY_SHADER *)orxLinkList_GetFirst(&(sstDisplay.stActiveShaderList));
            pstActive != orxNULL;
            pstActive = pstNextActive)
        {
          /* Gets next shader */
          pstNextActive = (orxDISPLAY_SHADER *)orxLinkList_GetNext(&(pstActive->stNode));

          /* Was pending removal? */
          if(pstActive->bPending != orxFALSE)
          {
            /* Clears its texture count */
            pstActive->iTextureCount = 0;

            /* Clears its texture info list */
            orxMemory_Zero(pstActive->astTextureInfoList, sstDisplay.iTextureUnitNumber * sizeof(orxDISPLAY_TEXTURE_INFO));

            /* Removes its pending status */
            pstActive->bPending = orxFALSE;

            /* Removes it from active list */
            orxLinkList_Remove(&(pstActive->stNode));

            /* Updates count */
            sstDisplay.s32PendingShaderCount--;
          }
        }

        /* Clears texture count */
        pstShader->iTextureCount = 0;

        /* Clears texture info list */
        orxMemory_Zero(pstShader->astTextureInfoList, sstDisplay.iTextureUnitNumber * sizeof(orxDISPLAY_TEXTURE_INFO));

        /* Removes it from active list */
        orxLinkList_Remove(&(pstShader->stNode));
      }
      /* Using custom param? */
      else if(pstShader->bUseCustomParam != orxFALSE)
      {
        /* Has something to display? */
        if(sstDisplay.s32BufferIndex > 0)
        {
          /* Draws arrays */
          orxDisplay_GLFW_DrawArrays();

          /* Don't reset shader */
          bResetShader = orxFALSE;
        }

        /* Clears texture count */
        pstShader->iTextureCount = 0;

        /* Clears texture info list */
        orxMemory_Zero(pstShader->astTextureInfoList, sstDisplay.iTextureUnitNumber * sizeof(orxDISPLAY_TEXTURE_INFO));

        /* Removes it from active list */
        orxLinkList_Remove(&(pstShader->stNode));
      }
      else
      {
        /* Marks it as pending */
        pstShader->bPending = orxTRUE;

        /* Updates count */
        sstDisplay.s32PendingShaderCount++;
      }
    }
    else
    {
      /* Don't reset shader */
      bResetShader = orxFALSE;

      /* Updates result */
      eResult = orxSTATUS_FAILURE;
    }
  }

  /* Should reset shader? */
  if(bResetShader != orxFALSE)
  {
    /* Uses default program */
    glUseProgramObjectARB(sstDisplay.pstDefaultShader->hProgram);
    glASSERT();

    /* Updates its texture unit */
    glUNIFORM(1iARB, sstDisplay.pstDefaultShader->iTextureLocation, sstDisplay.s32ActiveTextureUnit);

    /* Updates projection matrix */
    glUNIFORM(Matrix4fvARB, sstDisplay.pstDefaultShader->iProjectionMatrixLocation, 1, GL_FALSE, (GLfloat *)&(sstDisplay.mProjectionMatrix.aafValueList[0][0]));
  }

  /* Done! */
  return eResult;
}

orxS32 orxFASTCALL orxDisplay_GLFW_GetParameterID(const orxHANDLE _hShader, const orxSTRING _zParam, orxS32 _s32Index, orxBOOL _bIsTexture)
{
  orxDISPLAY_SHADER  *pstShader;
  orxS32              s32Result;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT((_hShader != orxHANDLE_UNDEFINED) && (_hShader != orxNULL));
  orxASSERT(_zParam != orxNULL);

  /* Gets shader */
  pstShader = (orxDISPLAY_SHADER *)_hShader;

  /* Is a texture? */
  if(_bIsTexture != orxFALSE)
  {
    orxDISPLAY_PARAM_INFO  *pstInfo;
    orxCHAR                 acBuffer[256];

    /* Checks */
    orxASSERT(pstShader->s32ParamCount < sstDisplay.iTextureUnitNumber);

    /* Inits buffer */
    acBuffer[sizeof(acBuffer) - 1] = orxCHAR_NULL;

    /* Gets corresponding param info */
    pstInfo = &pstShader->astParamInfoList[pstShader->s32ParamCount];

    /* Updates result */
    s32Result = pstShader->s32ParamCount++;

    /* Array? */
    if(_s32Index >= 0)
    {
      /* Prints its name */
      orxString_NPrint(acBuffer, sizeof(acBuffer) - 1, "%s[%d]", _zParam, _s32Index);

      /* Gets parameter location */
      pstInfo->iLocation = glGetUniformLocationARB(pstShader->hProgram, acBuffer);
      glASSERT();

      /* Gets top parameter location */
      orxString_NPrint(acBuffer, sizeof(acBuffer) - 1, "%s" orxDISPLAY_KZ_SHADER_SUFFIX_TOP "[%d]", _zParam, _s32Index);
      pstInfo->iLocationTop = glGetUniformLocationARB(pstShader->hProgram, (const GLchar *)acBuffer);
      glASSERT();

      /* Gets left parameter location */
      orxString_NPrint(acBuffer, sizeof(acBuffer) - 1, "%s" orxDISPLAY_KZ_SHADER_SUFFIX_LEFT "[%d]", _zParam, _s32Index);
      pstInfo->iLocationLeft = glGetUniformLocationARB(pstShader->hProgram, (const GLchar *)acBuffer);
      glASSERT();

      /* Gets bottom parameter location */
      orxString_NPrint(acBuffer, sizeof(acBuffer) - 1, "%s" orxDISPLAY_KZ_SHADER_SUFFIX_BOTTOM "[%d]", _zParam, _s32Index);
      pstInfo->iLocationBottom = glGetUniformLocationARB(pstShader->hProgram, (const GLchar *)acBuffer);
      glASSERT();

      /* Gets right parameter location */
      orxString_NPrint(acBuffer, sizeof(acBuffer) - 1, "%s" orxDISPLAY_KZ_SHADER_SUFFIX_RIGHT "[%d]", _zParam, _s32Index);
      pstInfo->iLocationRight = glGetUniformLocationARB(pstShader->hProgram, (const GLchar *)acBuffer);
      glASSERT();
    }
    else
    {
      /* Gets parameter location */
      pstInfo->iLocation = glGetUniformLocationARB(pstShader->hProgram, (const GLchar *)_zParam);
      glASSERT();

      /* Gets top parameter location */
      orxString_NPrint(acBuffer, sizeof(acBuffer) - 1, "%s" orxDISPLAY_KZ_SHADER_SUFFIX_TOP, _zParam);
      pstInfo->iLocationTop = glGetUniformLocationARB(pstShader->hProgram, (const GLchar *)acBuffer);
      glASSERT();

      /* Gets left parameter location */
      orxString_NPrint(acBuffer, sizeof(acBuffer) - 1, "%s" orxDISPLAY_KZ_SHADER_SUFFIX_LEFT, _zParam);
      pstInfo->iLocationLeft = glGetUniformLocationARB(pstShader->hProgram, (const GLchar *)acBuffer);
      glASSERT();

      /* Gets bottom parameter location */
      orxString_NPrint(acBuffer, sizeof(acBuffer) - 1, "%s" orxDISPLAY_KZ_SHADER_SUFFIX_BOTTOM, _zParam);
      pstInfo->iLocationBottom = glGetUniformLocationARB(pstShader->hProgram, (const GLchar *)acBuffer);
      glASSERT();

      /* Gets right parameter location */
      orxString_NPrint(acBuffer, sizeof(acBuffer) - 1, "%s" orxDISPLAY_KZ_SHADER_SUFFIX_RIGHT, _zParam);
      pstInfo->iLocationRight = glGetUniformLocationARB(pstShader->hProgram, (const GLchar *)acBuffer);
      glASSERT();
    }

    /* Not using custom param? */
    if(pstShader->bUseCustomParam == orxFALSE)
    {
      /* Has any texture edge location? */
      if((pstInfo->iLocationTop >= 0)
      || (pstInfo->iLocationLeft >= 0)
      || (pstInfo->iLocationBottom >= 0)
      || (pstInfo->iLocationRight >= 0))
      {
        /* Updates status */
        pstShader->bUseCustomParam = orxTRUE;

        /* Outputs log */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Shader [%u] with \"UseCustomParam = false\" is using edge parameter for texture [%s]: forcing UseCustomParam to true.", (orxU32)(orxUPTR)pstShader->hProgram, _zParam);
      }
    }
  }
  else
  {
    /* Array? */
    if(_s32Index >= 0)
    {
      orxCHAR acBuffer[256];

      /* Prints its name */
      orxString_NPrint(acBuffer, sizeof(acBuffer) - 1, "%s[%d]", _zParam, _s32Index);
      acBuffer[sizeof(acBuffer) - 1] = orxCHAR_NULL;

      /* Gets parameter location */
      s32Result = (orxS32)glGetUniformLocationARB(pstShader->hProgram, acBuffer);
      glASSERT();
    }
    else
    {
      /* Gets parameter location */
      s32Result = (orxS32)glGetUniformLocationARB(pstShader->hProgram, (const GLchar *)_zParam);
      glASSERT();
    }
  }

  /* Done! */
  return s32Result;
}

orxSTATUS orxFASTCALL orxDisplay_GLFW_SetShaderBitmap(orxHANDLE _hShader, orxS32 _s32ID, const orxBITMAP *_pstValue)
{
  orxDISPLAY_SHADER  *pstShader;
  orxSTATUS           eResult = orxSTATUS_FAILURE;
  orxS32              i;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT((_hShader != orxHANDLE_UNDEFINED) && (_hShader != orxNULL));
  orxASSERT((_pstValue != sstDisplay.pstScreen) && "Can't use screen bitmap as texture parameter (ID <%d>) for fragment shader.", _s32ID);

  /* Gets shader */
  pstShader = (orxDISPLAY_SHADER *)_hShader;

  /* No bitmap? */
  if(_pstValue == orxNULL)
  {
    /* Uses screen bitmap */
    _pstValue = sstDisplay.pstScreen;
  }

  /* For all already used texture units */
  for(i = 0; i < pstShader->iTextureCount; i++)
  {
    /* Same location? */
    if(pstShader->astTextureInfoList[i].iLocation == pstShader->astParamInfoList[_s32ID].iLocation)
    {
      /* Different texture? */
      if(pstShader->astTextureInfoList[i].pstBitmap != _pstValue)
      {
        /* Draws remaining items */
        orxDisplay_GLFW_DrawArrays();

        /* Updates texture info */
        pstShader->astTextureInfoList[i].pstBitmap = _pstValue;

        /* Updates corner values */
        glUNIFORM(1fARB, pstShader->astParamInfoList[_s32ID].iLocationTop, (GLfloat)(_pstValue->fRecRealHeight * _pstValue->stClip.vTL.fY));
        glUNIFORM(1fARB, pstShader->astParamInfoList[_s32ID].iLocationLeft, (GLfloat)(_pstValue->fRecRealWidth * _pstValue->stClip.vTL.fX));
        glUNIFORM(1fARB, pstShader->astParamInfoList[_s32ID].iLocationBottom, (GLfloat)(_pstValue->fRecRealHeight * _pstValue->stClip.vBR.fY));
        glUNIFORM(1fARB, pstShader->astParamInfoList[_s32ID].iLocationRight, (GLfloat)(_pstValue->fRecRealWidth * _pstValue->stClip.vBR.fX));
      }

      /* Updates result */
      eResult = orxSTATUS_SUCCESS;

      break;
    }
  }

  /* Not already done? */
  if(eResult == orxSTATUS_FAILURE)
  {
    /* Has free texture unit left? */
    if(pstShader->iTextureCount < sstDisplay.iTextureUnitNumber)
    {
      /* Valid? */
      if(_s32ID >= 0)
      {
        /* Updates texture info */
        pstShader->astTextureInfoList[pstShader->iTextureCount].iLocation = pstShader->astParamInfoList[_s32ID].iLocation;
        pstShader->astTextureInfoList[pstShader->iTextureCount].pstBitmap = _pstValue;

        /* Updates corner values */
        glUNIFORM(1fARB, pstShader->astParamInfoList[_s32ID].iLocationTop, (GLfloat)(_pstValue->fRecRealHeight * _pstValue->stClip.vTL.fY));
        glUNIFORM(1fARB, pstShader->astParamInfoList[_s32ID].iLocationLeft, (GLfloat)(_pstValue->fRecRealWidth * _pstValue->stClip.vTL.fX));
        glUNIFORM(1fARB, pstShader->astParamInfoList[_s32ID].iLocationBottom, (GLfloat)(_pstValue->fRecRealHeight * _pstValue->stClip.vBR.fY));
        glUNIFORM(1fARB, pstShader->astParamInfoList[_s32ID].iLocationRight, (GLfloat)(_pstValue->fRecRealWidth * _pstValue->stClip.vBR.fX));

        /* Updates texture count */
        pstShader->iTextureCount++;

        /* Updates result */
        eResult = orxSTATUS_SUCCESS;
      }
      else
      {
        /* Outputs log */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Can't bind texture parameter (ID <%d>) for fragment shader: invalid ID.", _s32ID);
      }
    }
    else
    {
      /* Outputs log */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Can't bind texture parameter (ID <%d>) for fragment shader: all the texture units are used.", _s32ID);
    }
  }

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_GLFW_SetShaderFloat(orxHANDLE _hShader, orxS32 _s32ID, orxFLOAT _fValue)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT((_hShader != orxHANDLE_UNDEFINED) && (_hShader != orxNULL));

  /* Valid? */
  if(_s32ID >= 0)
  {
    /* Updates its value (no glASSERT() as this can be set more than once per use and would trigger it) */
    glUNIFORM_NO_ASSERT(1fARB, (GLint)_s32ID, (GLfloat)_fValue);

    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }
  else
  {
    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_GLFW_SetShaderVector(orxHANDLE _hShader, orxS32 _s32ID, const orxVECTOR *_pvValue)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT((_hShader != orxHANDLE_UNDEFINED) && (_hShader != orxNULL));
  orxASSERT(_pvValue != orxNULL);

  /* Valid? */
  if(_s32ID >= 0)
  {
    /* Updates its value (no glASSERT() as this can be set more than once per use and would trigger it) */
    glUNIFORM_NO_ASSERT(3fARB, (GLint)_s32ID, (GLfloat)_pvValue->fX, (GLfloat)_pvValue->fY, (GLfloat)_pvValue->fZ);

    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }
  else
  {
    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

orxU32 orxFASTCALL orxDisplay_GLFW_GetShaderID(const orxHANDLE _hShader)
{
  orxDISPLAY_SHADER  *pstShader;
  orxU32              u32Result;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT((_hShader != orxHANDLE_UNDEFINED) && (_hShader != orxNULL));

  /* Gets shader */
  pstShader = (orxDISPLAY_SHADER *)_hShader;

  /* Updates result */
  u32Result = (orxU32)(orxUPTR)pstShader->hProgram;

  /* Done! */
  return u32Result;
}


/***************************************************************************
 * Plugin Related                                                          *
 ***************************************************************************/

orxPLUGIN_USER_CORE_FUNCTION_START(DISPLAY);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_GLFW_Init, DISPLAY, INIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_GLFW_Exit, DISPLAY, EXIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_GLFW_Swap, DISPLAY, SWAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_GLFW_GetScreenBitmap, DISPLAY, GET_SCREEN_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_GLFW_GetScreenSize, DISPLAY, GET_SCREEN_SIZE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_GLFW_CreateBitmap, DISPLAY, CREATE_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_GLFW_DeleteBitmap, DISPLAY, DELETE_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_GLFW_LoadBitmap, DISPLAY, LOAD_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_GLFW_SaveBitmap, DISPLAY, SAVE_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_GLFW_SetTempBitmap, DISPLAY, SET_TEMP_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_GLFW_GetTempBitmap, DISPLAY, GET_TEMP_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_GLFW_SetDestinationBitmaps, DISPLAY, SET_DESTINATION_BITMAPS);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_GLFW_ClearBitmap, DISPLAY, CLEAR_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_GLFW_SetBlendMode, DISPLAY, SET_BLEND_MODE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_GLFW_SetBitmapClipping, DISPLAY, SET_BITMAP_CLIPPING);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_GLFW_SetBitmapData, DISPLAY, SET_BITMAP_DATA);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_GLFW_GetBitmapData, DISPLAY, GET_BITMAP_DATA);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_GLFW_SetPartialBitmapData, DISPLAY, SET_PARTIAL_BITMAP_DATA);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_GLFW_GetBitmapSize, DISPLAY, GET_BITMAP_SIZE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_GLFW_GetBitmapID, DISPLAY, GET_BITMAP_ID);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_GLFW_TransformBitmap, DISPLAY, TRANSFORM_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_GLFW_TransformText, DISPLAY, TRANSFORM_TEXT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_GLFW_DrawLine, DISPLAY, DRAW_LINE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_GLFW_DrawPolyline, DISPLAY, DRAW_POLYLINE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_GLFW_DrawPolygon, DISPLAY, DRAW_POLYGON);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_GLFW_DrawCircle, DISPLAY, DRAW_CIRCLE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_GLFW_DrawOBox, DISPLAY, DRAW_OBOX);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_GLFW_DrawMesh, DISPLAY, DRAW_MESH);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_GLFW_HasShaderSupport, DISPLAY, HAS_SHADER_SUPPORT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_GLFW_CreateShader, DISPLAY, CREATE_SHADER);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_GLFW_DeleteShader, DISPLAY, DELETE_SHADER);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_GLFW_StartShader, DISPLAY, START_SHADER);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_GLFW_StopShader, DISPLAY, STOP_SHADER);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_GLFW_GetParameterID, DISPLAY, GET_PARAMETER_ID);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_GLFW_SetShaderBitmap, DISPLAY, SET_SHADER_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_GLFW_SetShaderFloat, DISPLAY, SET_SHADER_FLOAT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_GLFW_SetShaderVector, DISPLAY, SET_SHADER_VECTOR);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_GLFW_GetShaderID, DISPLAY, GET_SHADER_ID);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_GLFW_EnableVSync, DISPLAY, ENABLE_VSYNC);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_GLFW_IsVSyncEnabled, DISPLAY, IS_VSYNC_ENABLED);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_GLFW_SetFullScreen, DISPLAY, SET_FULL_SCREEN);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_GLFW_IsFullScreen, DISPLAY, IS_FULL_SCREEN);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_GLFW_GetVideoModeCount, DISPLAY, GET_VIDEO_MODE_COUNT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_GLFW_GetVideoMode, DISPLAY, GET_VIDEO_MODE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_GLFW_SetVideoMode, DISPLAY, SET_VIDEO_MODE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_GLFW_IsVideoModeAvailable, DISPLAY, IS_VIDEO_MODE_AVAILABLE);
orxPLUGIN_USER_CORE_FUNCTION_END();

#ifdef __orxGCC__
  #pragma GCC diagnostic pop
#endif /* __orxGCC__ */
