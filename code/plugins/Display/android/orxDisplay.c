/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2018 Orx-Project
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
 * @date 13/01/2011
 * @author simons.philippe@gmail.com
 *
 * Android display plugin implementation
 *
 * @todo
 */


#include "orxPluginAPI.h"

#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include "main/orxAndroid.h"

#include <sys/endian.h>

#include "webp/decode.h"

#define STBI_NO_STDIO
#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_PSD
#define STBI_NO_GIF
#define STBI_NO_HDR
#define STBI_NO_PIC
#define STBI_NO_PNM
#define STBI_MALLOC(sz)         orxMemory_Allocate((orxU32)sz, orxMEMORY_TYPE_VIDEO)
#define STBI_REALLOC(p, newsz)  orxMemory_Reallocate(p, newsz)
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

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBIW_MALLOC(sz)        orxMemory_Allocate(sz, orxMEMORY_TYPE_VIDEO)
#define STBIW_REALLOC(p, newsz) orxMemory_Reallocate(p, newsz)
#define STBIW_FREE(p)           orxMemory_Free(p)
#define STBIW_MEMMOVE(a, b, sz) orxMemory_Move(a, b, sz)
#include "stb_image_write.h"
#undef STBIW_MEMMOVE
#undef STBIW_FREE
#undef STBIW_REALLOC
#undef STBIW_MALLOC
#undef STB_IMAGE_WRITE_IMPLEMENTATION


/** Module flags
 */
#define orxDISPLAY_KU32_STATIC_FLAG_NONE        0x00000000  /**< No flags */

#define orxDISPLAY_KU32_STATIC_FLAG_READY       0x00000001  /**< Ready flag */
#define orxDISPLAY_KU32_STATIC_FLAG_SHADER      0x00000002  /**< Shader support flag */
#define orxDISPLAY_KU32_STATIC_FLAG_DEPTHBUFFER 0x00000004  /**< Has depth buffer support flag */
#define orxDISPLAY_KU32_STATIC_FLAG_CUSTOM_IBO  0x00000008  /**< Custom IBO flag */

#define orxDISPLAY_KU32_STATIC_MASK_ALL         0xFFFFFFFF  /**< All mask */

#define orxDISPLAY_KU32_BITMAP_FLAG_NONE        0x00000000  /** No flags */

#define orxDISPLAY_KU32_BITMAP_FLAG_LOADING     0x00000001  /**< Loading flag */
#define orxDISPLAY_KU32_BITMAP_FLAG_DELETE      0x00000002  /**< Delete flag */

#define orxDISPLAY_KU32_BITMAP_MASK_ALL         0xFFFFFFFF  /**< All mask */

#define orxDISPLAY_KU32_BITMAP_BANK_SIZE        128
#define orxDISPLAY_KU32_SHADER_BANK_SIZE        16

#define orxDISPLAY_KU32_VERTEX_BUFFER_SIZE      (4 * 16384) /**< 16384 items batch capacity */
#define orxDISPLAY_KU32_INDEX_BUFFER_SIZE       (6 * 16384) /**< 16384 items batch capacity */
#define orxDISPLAY_KU32_SHADER_BUFFER_SIZE      131072

#define orxDISPLAY_KF_BORDER_FIX                0.1f

#define orxDISPLAY_KU32_CIRCLE_LINE_NUMBER      32

#define orxDISPLAY_KU32_MAX_TEXTURE_UNIT_NUMBER 32
#define orxDISPLAY_KE_DEFAULT_PRIMITIVE         GL_TRIANGLE_STRIP


/**  Misc defines
 */

#define glUNIFORM(EXT, LOCATION, ...) do {if((LOCATION) >= 0) {glUniform##EXT(LOCATION, ##__VA_ARGS__); glASSERT();}} while(orxFALSE)

#ifdef __orxDEBUG__

#define glASSERT()                                                      \
do                                                                      \
{                                                                       \
  GLenum eError = glGetError();                                         \
  orxASSERT(eError == GL_NO_ERROR && "OpenGL error code: 0x%X", eError);\
} while(orxFALSE)

#define eglASSERT()                                                     \
do                                                                      \
{                                                                       \
  EGLint eError = eglGetError();                                        \
  orxASSERT(eError == EGL_SUCCESS && "EGL error code: 0x%X", eError);   \
} while(orxFALSE)

#define glUNIFORM_NO_ASSERT(EXT, LOCATION, ...) do {if((LOCATION) >= 0) {glUniform##EXT(LOCATION, ##__VA_ARGS__); glGetError();}} while(orxFALSE)

#else /* __orxDEBUG__ */

#define glASSERT()

#define eglASSERT()

#define glUNIFORM_NO_ASSERT(EXT, LOCATION, ...) do {if((LOCATION) >= 0) {glUniform##EXT(LOCATION, ##__VA_ARGS__);}} while(orxFALSE)

#endif /* __orxDEBUG__ */

/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

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
typedef struct __orxDISPLAY_ANDROID_VERTEX_t
{
  GLfloat fX, fY;
  GLfloat fU, fV;
  orxRGBA stRGBA;

} orxDISPLAY_ANDROID_VERTEX;

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
  orxRGBA                   stColor;
  const orxSTRING           zLocation;
  orxSTRINGID               stFilenameID;
  orxU32                    u32Flags;
};

/** Internal bitmap save info structure
 */
typedef struct __orxDISPLAY_SAVE_INFO_t
{
  orxU8      *pu8ImageData;
  orxU32      u32Width;
  orxU32      u32Height;
  orxSTRINGID stFilenameID;

} orxDISPLAY_SAVE_INFO;

/** Internal bitmap load info structure
 */
typedef struct __orxDISPLAY_LOAD_INFO_t
{
  orxU8      *pu8ImageBuffer;
  orxU8      *pu8ImageSource;
  orxS64      s64Size;
  orxBITMAP  *pstBitmap;
  GLuint      uiWidth;
  GLuint      uiHeight;
  GLuint      uiRealWidth;
  GLuint      uiRealHeight;

} orxDISPLAY_LOAD_INFO;

/** Internal texture info structure
 */
typedef struct __orxDISPLAY_TEXTURE_INFO_t
{
  GLint iLocation;
  const orxBITMAP *pstBitmap;

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
  GLuint                    uiProgram;
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
  orxBITMAP                *pstScreen;
  const orxBITMAP          *pstTempBitmap;
  orxRGBA                   stLastColor;
  orxU32                    u32LastClipX, u32LastClipY, u32LastClipWidth, u32LastClipHeight;
  orxDISPLAY_BLEND_MODE     eLastBlendMode;
  orxDISPLAY_BUFFER_MODE    eLastBufferMode;
  GLenum                    ePrimitive;
  orxS32                    s32PendingShaderCount;
  GLint                     iLastViewportX, iLastViewportY;
  GLsizei                   iLastViewportWidth, iLastViewportHeight;
  orxFLOAT                  fLastOrthoRight, fLastOrthoBottom;
  orxDISPLAY_SHADER        *pstDefaultShader;
  orxDISPLAY_SHADER        *pstNoTextureShader;
  GLint                     iTextureUnitNumber;
  GLint                     iDrawBufferNumber;
  orxU32                    u32DestinationBitmapCount;
  GLuint                    uiFrameBuffer;
  GLuint                    uiLastFrameBuffer;
  GLuint                    uiVertexBuffer;
  GLuint                    uiIndexBuffer;
  orxS32                    s32BufferIndex;
  orxS32                    s32ElementNumber;
  orxU32                    u32Flags;
  orxU32                    u32Depth;
  orxS32                    s32ActiveTextureUnit;
  stbi_io_callbacks         stSTBICallbacks;
  GLenum                    aeDrawBufferList[orxDISPLAY_KU32_MAX_TEXTURE_UNIT_NUMBER];
  orxBITMAP                *apstDestinationBitmapList[orxDISPLAY_KU32_MAX_TEXTURE_UNIT_NUMBER];
  const orxBITMAP          *apstBoundBitmapList[orxDISPLAY_KU32_MAX_TEXTURE_UNIT_NUMBER];
  orxDOUBLE                 adMRUBitmapList[orxDISPLAY_KU32_MAX_TEXTURE_UNIT_NUMBER];
  orxDISPLAY_PROJ_MATRIX    mProjectionMatrix;
  orxDISPLAY_ANDROID_VERTEX astVertexList[orxDISPLAY_KU32_VERTEX_BUFFER_SIZE];
  GLushort                  au16IndexList[orxDISPLAY_KU32_INDEX_BUFFER_SIZE];
  orxCHAR                   acShaderCodeBuffer[orxDISPLAY_KU32_SHADER_BUFFER_SIZE];

  EGLDisplay                display;
  EGLConfig                 config;
  EGLSurface                surface;
  EGLContext                context;
  EGLint                    format;

} orxDISPLAY_STATIC;


/** KTX header
 */

typedef struct KTX_header_t {
  orxU8  identifier[12];
  orxU32 endianness;
  orxU32 glType;
  orxU32 glTypeSize;
  orxU32 glFormat;
  orxU32 glInternalFormat;
  orxU32 glBaseInternalFormat;
  orxU32 pixelWidth;
  orxU32 pixelHeight;
  orxU32 pixelDepth;
  orxU32 numberOfArrayElements;
  orxU32 numberOfFaces;
  orxU32 numberOfMipmapLevels;
  orxU32 bytesOfKeyValueData;
} KTX_header;

/* KTX files require an unpack alignment of 4 */
#define KTX_GL_UNPACK_ALIGNMENT 4

static int getKTXInfos(orxHANDLE _hResource, int *x, int *y)
{
  KTX_header stHeader;

  /* Loads KTX header from file */
  if(orxResource_Read(_hResource, sizeof(KTX_header), &stHeader, orxNULL, orxNULL) == sizeof(KTX_header))
  {
    *x = stHeader.pixelWidth;
    *y = stHeader.pixelHeight;
    return 1;
  }

  return 0;
}

/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** Static data
 */
static orxDISPLAY_STATIC sstDisplay;
static const orxSTRING szKTXExtention = ".ktx";

/** Prototypes
 */
orxSTATUS orxFASTCALL orxDisplay_Android_StartShader(orxHANDLE _hShader);
orxSTATUS orxFASTCALL orxDisplay_Android_StopShader(orxHANDLE _hShader);
orxSTATUS orxFASTCALL orxDisplay_Android_SetBlendMode(orxDISPLAY_BLEND_MODE _eBlendMode);
orxSTATUS orxFASTCALL orxDisplay_Android_SetDestinationBitmaps(orxBITMAP **_apstBitmapList, orxU32 _u32Number);
orxSTATUS orxFASTCALL orxDisplay_Android_SetVideoMode(const orxDISPLAY_VIDEO_MODE *_pstVideoMode);


#define GL_MAX_DRAW_BUFFERS                              0x8824
void (* glDrawBuffers) (GLsizei n, const GLenum* bufs);

static orxBOOL gl3stubInit()
{
  glDrawBuffers = (void (*)(GLsizei, const GLenum*)) eglGetProcAddress("glDrawBuffers");

  return glDrawBuffers != orxNULL ? orxTRUE : orxFALSE;
}

static EGLConfig defaultEGLChooser(EGLDisplay disp)
{
  EGLint count = 0;
  EGLConfig bestConfig = orxNULL;

  eglGetConfigs(disp, NULL, 0, &count);
  eglASSERT();

  orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Config count = %d", count);

  EGLConfig* configs = new EGLConfig[count];
  eglGetConfigs(disp, configs, count, &count);
  eglASSERT();

  int bestMatch = 1<<30;

  int minDepthBits = orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_DEPTHBUFFER) ? 16 : 0;
  int minRedBits = sstDisplay.u32Depth == 16 ? 5 : 8;
  int minGreenBits = sstDisplay.u32Depth == 16 ? 6 : 8;
  int minBlueBits = sstDisplay.u32Depth == 16 ? 5 : 8;

  int i;
  for (i = 0; i < count; i++)
  {
    int match = 0;
    EGLint surfaceType = 0;
    EGLint blueBits = 0;
    EGLint greenBits = 0;
    EGLint redBits = 0;
    EGLint alphaBits = 0;
    EGLint depthBits = 0;
    EGLint stencilBits = 0;
    EGLint renderableFlags = 0;

    eglGetConfigAttrib(disp, configs[i], EGL_SURFACE_TYPE, &surfaceType);
    eglASSERT();
    eglGetConfigAttrib(disp, configs[i], EGL_BLUE_SIZE, &blueBits);
    eglASSERT();
    eglGetConfigAttrib(disp, configs[i], EGL_GREEN_SIZE, &greenBits);
    eglASSERT();
    eglGetConfigAttrib(disp, configs[i], EGL_RED_SIZE, &redBits);
    eglASSERT();
    eglGetConfigAttrib(disp, configs[i], EGL_ALPHA_SIZE, &alphaBits);
    eglASSERT();
    eglGetConfigAttrib(disp, configs[i], EGL_DEPTH_SIZE, &depthBits);
    eglASSERT();
    eglGetConfigAttrib(disp, configs[i], EGL_STENCIL_SIZE, &stencilBits);
    eglASSERT();
    eglGetConfigAttrib(disp, configs[i], EGL_RENDERABLE_TYPE, &renderableFlags);
    eglASSERT();
    orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Config[%d]: R%dG%dB%dA%d D%dS%d Type=%04x Render=%04x",
      i, redBits, greenBits, blueBits, alphaBits, depthBits, stencilBits, surfaceType, renderableFlags);

    if ((surfaceType & EGL_WINDOW_BIT) == 0)
      continue;
    if ((renderableFlags & EGL_OPENGL_ES2_BIT) == 0)
      continue;
    if (depthBits < minDepthBits)
      continue;
    if ((redBits < minRedBits) || (greenBits < minGreenBits) || (blueBits < minBlueBits))
      continue;

    int penalty = depthBits - minDepthBits;
    match += penalty * penalty;
    penalty = redBits - minRedBits;
    match += penalty * penalty;
    penalty = greenBits - minGreenBits;
    match += penalty * penalty;
    penalty = blueBits - minBlueBits;
    match += penalty * penalty;
    penalty = alphaBits;
    match += penalty * penalty;
    penalty = stencilBits;
    match += penalty * penalty;

    if ((match < bestMatch) || (bestConfig == orxNULL))
    {
      bestMatch = match;
      orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Config[%d] is the new best config", i);
      bestConfig = configs[i];
    }
  }

  delete[] configs;

  return bestConfig;
}

/** Render inhibitor
 */
static orxSTATUS orxFASTCALL orxDisplay_Android_RenderInhibitor(const orxEVENT *_pstEvent)
{
  /* Done! */
  return orxSTATUS_FAILURE;
}

static orxSTATUS orxAndroid_Display_CreateSurface()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  if (sstDisplay.surface == EGL_NO_SURFACE)
  {
    orxU32 u32Width, u32Height;
    int32_t windowWidth, windowHeight;
    orxFLOAT fScale;

    ANativeWindow *window = orxAndroid_GetNativeWindow();

    if(!window)
    {
      return orxSTATUS_FAILURE;
    }

    windowWidth = ANativeWindow_getWidth(window);
    windowHeight = ANativeWindow_getHeight(window);

    orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "native windows size: (%dx%d)", windowWidth, windowHeight);

    if(windowWidth > 0 && windowHeight > 0)
    {
      /* default to native window size */
      u32Width = windowWidth;
      u32Height = windowHeight;
      fScale = orxFLOAT_1;

      /* Pushes config section */
      orxConfig_PushSection(orxDISPLAY_KZ_CONFIG_SECTION);

      /* Has ScreenWidth? */
      if (orxConfig_HasValue(orxDISPLAY_KZ_CONFIG_WIDTH))
      {
        orxU32 u32ConfigWidth;

        u32ConfigWidth = orxConfig_GetU32(orxDISPLAY_KZ_CONFIG_WIDTH);
        if ( windowWidth > u32ConfigWidth )
        {
          u32Width = u32ConfigWidth;
          fScale = orx2F(u32Width) / orx2F(windowWidth);
          u32Height = windowHeight * fScale;
          orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "scaled windows size: (%dx%d)", u32Width, u32Height);
        }

        if(orxConfig_HasValue(orxDISPLAY_KZ_CONFIG_HEIGHT))
        {
          orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "WARNING, Display.ScreenHeight ignored.");
        }
      }
      else
      /* Has ScreenHeight? */
      if (orxConfig_HasValue(orxDISPLAY_KZ_CONFIG_HEIGHT))
      {
        orxU32 u32ConfigHeight;

        u32ConfigHeight = orxConfig_GetU32(orxDISPLAY_KZ_CONFIG_HEIGHT);
        if ( windowHeight > u32ConfigHeight )
        {
          u32Height = u32ConfigHeight;
          fScale = orx2F(u32Height) / orx2F(windowHeight);
          u32Width = windowWidth * fScale;
          orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "scaled windows size: (%dx%d)", u32Width, u32Height);
        }
      }

      /* Updates ScreenHeight value */
      orxConfig_SetU32(orxDISPLAY_KZ_CONFIG_HEIGHT, u32Height);
      /* Updates ScreenWidth value */
      orxConfig_SetU32(orxDISPLAY_KZ_CONFIG_WIDTH, u32Width);

      /* Pops config section */
      orxConfig_PopSection();

      /* Pushes config section */
      orxConfig_PushSection(KZ_CONFIG_ANDROID);

      /* Save scaling */
      orxConfig_SetFloat(KZ_CONFIG_SURFACE_SCALE, fScale);

      /* Pops config section */
      orxConfig_PopSection();

      /* Set framebuffer size */
      ANativeWindow_setBuffersGeometry(window, u32Width, u32Height, sstDisplay.format);

      orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Creating new EGL Surface");
      sstDisplay.surface = eglCreateWindowSurface(sstDisplay.display, sstDisplay.config, window, NULL);
      eglASSERT();

      eglMakeCurrent(sstDisplay.display, sstDisplay.surface, sstDisplay.surface, sstDisplay.context);
      eglASSERT();

      /* Removes render inhibitor */
      orxEvent_RemoveHandler(orxEVENT_TYPE_RENDER, orxDisplay_Android_RenderInhibitor);

      eResult = orxSTATUS_SUCCESS;
    }
  }

  return eResult;
}

static void orxAndroid_Display_DestroySurface()
{
  eglMakeCurrent(sstDisplay.display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
  eglASSERT();

  if( sstDisplay.surface != EGL_NO_SURFACE )
  {
    orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Destroying EGL Surface");
    eglDestroySurface(sstDisplay.display, sstDisplay.surface);
    eglASSERT();
    sstDisplay.surface = EGL_NO_SURFACE;

    /* Adds render inhibitor */
    orxEvent_AddHandler(orxEVENT_TYPE_RENDER, orxDisplay_Android_RenderInhibitor);
  }
}

static void orxAndroid_Display_CreateContext()
{
  orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Creating new EGL Context");

  sstDisplay.display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
  eglASSERT();
  orxASSERT(sstDisplay.display != EGL_NO_DISPLAY);

  eglInitialize(sstDisplay.display, 0, 0);
  eglASSERT();
  sstDisplay.config = defaultEGLChooser(sstDisplay.display);
  orxASSERT(sstDisplay.config != orxNULL);

  eglGetConfigAttrib(sstDisplay.display, sstDisplay.config, EGL_NATIVE_VISUAL_ID, &sstDisplay.format);
  eglASSERT();

  EGLint contextAttrs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
  sstDisplay.context = eglCreateContext(sstDisplay.display, sstDisplay.config, EGL_NO_CONTEXT, contextAttrs);
  eglASSERT();
  orxASSERT(sstDisplay.context != EGL_NO_CONTEXT);
}

static orxDISPLAY_PROJ_MATRIX *orxDisplay_Android_OrthoProjMatrix(orxDISPLAY_PROJ_MATRIX *_pmResult, orxFLOAT _fLeft, orxFLOAT _fRight, orxFLOAT _fBottom, orxFLOAT _fTop, orxFLOAT _fNear, orxFLOAT _fFar)
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

static orxINLINE void orxDisplay_Android_BindBitmap(const orxBITMAP *_pstBitmap)
{
  orxDOUBLE dBestTime;
  orxS32    i, s32BestCandidate;

  /* For all texture units */
  for(i = 0, s32BestCandidate = -1, dBestTime = orxDOUBLE_MAX; i < (orxS32)sstDisplay.iTextureUnitNumber; i++)
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
    /* Selects unit */
    glActiveTexture(GL_TEXTURE0 + i);
    glASSERT();

    /* Updates MRU timestamp */
    sstDisplay.adMRUBitmapList[i] = orxSystem_GetSystemTime();

    /* Updates active texture unit */
    sstDisplay.s32ActiveTextureUnit = i;
  }
  else
  {
    /* Selects unit */
    glActiveTexture(GL_TEXTURE0 + s32BestCandidate);
    glASSERT();

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

static int orxDisplay_Android_ReadSTBICallback(void *_hResource, char *_pBuffer, int _iSize)
{
  /* Reads data */
  return (int)orxResource_Read((orxHANDLE)_hResource, _iSize, (orxU8 *)_pBuffer, orxNULL, orxNULL);
}

static void orxDisplay_Android_SkipSTBICallback(void *_hResource, int _iOffset)
{
  /* Seeks offset */
  orxResource_Seek((orxHANDLE)_hResource, _iOffset, orxSEEK_OFFSET_WHENCE_CURRENT);

  /* Done! */
  return;
}

static int orxDisplay_Android_EOFSTBICallback(void *_hResource)
{
  /* End of buffer? */
  return (orxResource_Tell((orxHANDLE)_hResource) == orxResource_GetSize(_hResource)) ? 1 : 0;
}

static void orxFASTCALL orxDisplay_Android_ReadKTXResourceCallback(orxHANDLE _hResource, orxS64 _s64Size, void *_pBuffer, void *_pContext)
{
  orxDISPLAY_EVENT_PAYLOAD  stPayload;
  orxBITMAP                *pstBitmap;

  /* Gets associated bitmap */
  pstBitmap = (orxBITMAP *)_pContext;

  /* Inits payload */
  stPayload.stBitmap.zLocation      = pstBitmap->zLocation;
  stPayload.stBitmap.stFilenameID   = pstBitmap->stFilenameID;
  stPayload.stBitmap.u32ID          = orxU32_UNDEFINED;

  if(_s64Size >= sizeof(KTX_header))
  {
    KTX_header *stHeader;
    orxU8      *pu8ImageData;
    orxBOOL     bCompressed = orxFALSE;
    GLint       previousUnpackAlignment;
    GLuint      uiWidth, uiHeight;
    GLenum      eInternalFormat;

    stHeader = (KTX_header*)_pBuffer;
    uiWidth  = stHeader->pixelWidth;
    uiHeight = stHeader->pixelHeight;

    /* Check glType and glFormat */
    if(stHeader->glType == 0 || stHeader->glFormat == 0)
    {
      orxASSERT(stHeader->glType + stHeader->glFormat == 0 && "Can't load KTX texture <%s>: either both or none of glType, glFormat must be zero, aborting.", pstBitmap->zLocation);
      bCompressed = orxTRUE;
    }

    /* KTX files require an unpack alignment of 4 */
    glGetIntegerv(GL_UNPACK_ALIGNMENT, &previousUnpackAlignment);
    if(previousUnpackAlignment != KTX_GL_UNPACK_ALIGNMENT)
    {
      glPixelStorei(GL_UNPACK_ALIGNMENT, KTX_GL_UNPACK_ALIGNMENT);
    }

    if(bCompressed == orxTRUE)
    {
      eInternalFormat = stHeader->glInternalFormat;
    }
    else
    {
      eInternalFormat = stHeader->glBaseInternalFormat;
    }

    /* Loads image */
    pu8ImageData = (orxU8 *)_pBuffer;

    if(_s64Size >= sizeof(KTX_header) + stHeader->bytesOfKeyValueData)
    {
      orxU32  u32DataSize, u32RoundedDataSize;
      GLuint  uiRealWidth, uiRealHeight;
      orxS32  i;
      orxU8  *pu8ImageBuffer;

      /* Skip header */
      pu8ImageData        = (orxU8 *)(pu8ImageData + sizeof(KTX_header) + stHeader->bytesOfKeyValueData);
      u32DataSize         = *((orxU32 *)pu8ImageData);
      u32RoundedDataSize  = orxALIGN(u32DataSize, 4);

      /* Uses image buffer */
      pu8ImageBuffer = pu8ImageData + sizeof(u32DataSize);

      /* Gets real size */
      uiRealWidth   = uiWidth;
      uiRealHeight  = uiHeight;

      /* Inits bitmap */
      pstBitmap->fWidth         = orxU2F(uiWidth);
      pstBitmap->fHeight        = orxU2F(uiHeight);
      pstBitmap->u32RealWidth   = (orxU32)uiRealWidth;
      pstBitmap->u32RealHeight  = (orxU32)uiRealHeight;
      pstBitmap->u32Depth       = 32;
      pstBitmap->fRecRealWidth  = orxFLOAT_1 / orxU2F(pstBitmap->u32RealWidth);
      pstBitmap->fRecRealHeight = orxFLOAT_1 / orxU2F(pstBitmap->u32RealHeight);
      pstBitmap->u32DataSize    = u32RoundedDataSize;
      orxVector_Copy(&(pstBitmap->stClip.vTL), &orxVECTOR_0);
      orxVector_Set(&(pstBitmap->stClip.vBR), pstBitmap->fWidth, pstBitmap->fHeight, orxFLOAT_0);

      /* Tracks video memory */
      orxMEMORY_TRACK(VIDEO, pstBitmap->u32DataSize, orxTRUE);

      /* Creates new texture */
      glGenTextures(1, &pstBitmap->uiTexture);
      glASSERT();
      glBindTexture(GL_TEXTURE_2D, pstBitmap->uiTexture);
      glASSERT();
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glASSERT();
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glASSERT();
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (pstBitmap->bSmoothing != orxFALSE) ? GL_LINEAR : GL_NEAREST);
      glASSERT();
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (pstBitmap->bSmoothing != orxFALSE) ? GL_LINEAR : GL_NEAREST);
      glASSERT();

      /* Compressed? */
      if(bCompressed == orxTRUE)
      {
        /* Loads compressed data */
        glCompressedTexImage2D(GL_TEXTURE_2D, 0, eInternalFormat, (GLsizei)pstBitmap->u32RealWidth, (GLsizei)pstBitmap->u32RealHeight, 0, u32DataSize, pu8ImageBuffer);
      }
      else
      {
        /* Loads data */
        glTexImage2D(GL_TEXTURE_2D, 0, eInternalFormat, (GLsizei)pstBitmap->u32RealWidth, (GLsizei)pstBitmap->u32RealHeight, 0, stHeader->glFormat, stHeader->glType, pu8ImageBuffer);
      }
      glASSERT();

      /* Restores previous texture */
      glBindTexture(GL_TEXTURE_2D, (sstDisplay.apstBoundBitmapList[sstDisplay.s32ActiveTextureUnit] != orxNULL) ? sstDisplay.apstBoundBitmapList[sstDisplay.s32ActiveTextureUnit]->uiTexture : 0);
      glASSERT();

      /* For all bound bitmaps */
      for(i = 0; i < (orxS32)sstDisplay.iTextureUnitNumber; i++)
      {
        /* Is deleted bitmap? */
        if(sstDisplay.apstBoundBitmapList[i] == pstBitmap)
        {
          /* Resets it */
          sstDisplay.apstBoundBitmapList[i] = orxNULL;
          sstDisplay.adMRUBitmapList[i]     = orxDOUBLE_0;
        }
      }

      /* Updates payload */
      stPayload.stBitmap.u32ID = (orxU32)pstBitmap->uiTexture;

      /* Sends event */
      orxEVENT_SEND(orxEVENT_TYPE_DISPLAY, orxDISPLAY_EVENT_LOAD_BITMAP, pstBitmap, orxNULL, &stPayload);

      /* Clears loading flag */
      orxFLAG_SET(pstBitmap->u32Flags, orxDISPLAY_KU32_BITMAP_FLAG_NONE, orxDISPLAY_KU32_BITMAP_FLAG_LOADING);
      orxMEMORY_BARRIER();
    }
    else
    {
      /* Clears info */
      pstBitmap->fWidth         =
      pstBitmap->fHeight        = orxFLOAT_1;
      pstBitmap->u32RealWidth   =
      pstBitmap->u32RealHeight  = 1;

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

      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Couldn't process data for bitmap <%s>: temp texture will remain in use.", pstBitmap->zLocation);

      /* Sends event */
      orxEVENT_SEND(orxEVENT_TYPE_DISPLAY, orxDISPLAY_EVENT_LOAD_BITMAP, pstBitmap, orxNULL, &stPayload);

      /* Clears loading flag */
      orxFLAG_SET(pstBitmap->u32Flags, orxDISPLAY_KU32_BITMAP_FLAG_NONE, orxDISPLAY_KU32_BITMAP_FLAG_LOADING);
    }

    if(previousUnpackAlignment != KTX_GL_UNPACK_ALIGNMENT)
    {
      glPixelStorei(GL_UNPACK_ALIGNMENT, previousUnpackAlignment);
    }
  }
  else
  {
    /* Clears info */
    pstBitmap->fWidth         =
    pstBitmap->fHeight        = orxFLOAT_1;
    pstBitmap->u32RealWidth   =
    pstBitmap->u32RealHeight  = 1;

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

    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Couldn't process data for bitmap <%s>: temp texture will remain in use.", pstBitmap->zLocation);

    /* Sends event */
    orxEVENT_SEND(orxEVENT_TYPE_DISPLAY, orxDISPLAY_EVENT_LOAD_BITMAP, pstBitmap, orxNULL, &stPayload);

    /* Clears loading flag */
    orxFLAG_SET(pstBitmap->u32Flags, orxDISPLAY_KU32_BITMAP_FLAG_NONE, orxDISPLAY_KU32_BITMAP_FLAG_LOADING);
  }

  /* Asked for deletion? */
  if(orxFLAG_TEST(pstBitmap->u32Flags, orxDISPLAY_KU32_BITMAP_FLAG_DELETE))
  {
    /* Deletes it */
    orxDisplay_DeleteBitmap(pstBitmap);
  }

  /* Frees buffer */
  orxMemory_Free(_pBuffer);

  /* Closes resource */
  orxResource_Close(_hResource);
}

static orxSTATUS orxFASTCALL orxDisplay_Android_DecompressBitmapCallback(void *_pContext)
{
  orxDISPLAY_LOAD_INFO     *pstInfo;
  orxSTATUS                 eResult = orxSTATUS_SUCCESS;

  /* Gets load info */
  pstInfo = (orxDISPLAY_LOAD_INFO *)_pContext;

  /* Hasn't exited yet? */
  if(sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY)
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
    pstInfo->pstBitmap->u32DataSize    = pstInfo->pstBitmap->u32RealWidth * pstInfo->pstBitmap->u32RealHeight * 4 * sizeof(orxU8);
    orxVector_Copy(&(pstInfo->pstBitmap->stClip.vTL), &orxVECTOR_0);
    orxVector_Set(&(pstInfo->pstBitmap->stClip.vBR), pstInfo->pstBitmap->fWidth, pstInfo->pstBitmap->fHeight, orxFLOAT_0);

    /* Tracks video memory */
    orxMEMORY_TRACK(VIDEO, pstInfo->pstBitmap->u32DataSize, orxTRUE);

    /* Creates new texture */
    glGenTextures(1, &(pstInfo->pstBitmap->uiTexture));
    glASSERT();
    glBindTexture(GL_TEXTURE_2D, pstInfo->pstBitmap->uiTexture);
    glASSERT();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)pstInfo->pstBitmap->u32RealWidth, (GLsizei)pstInfo->pstBitmap->u32RealHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, (pstInfo->pu8ImageBuffer != orxNULL) ? pstInfo->pu8ImageBuffer : NULL);
    glASSERT();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glASSERT();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glASSERT();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (pstInfo->pstBitmap->bSmoothing != orxFALSE) ? GL_LINEAR : GL_NEAREST);
    glASSERT();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (pstInfo->pstBitmap->bSmoothing != orxFALSE) ? GL_LINEAR : GL_NEAREST);
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

    /* Sends event */
    orxEVENT_SEND(orxEVENT_TYPE_DISPLAY, orxDISPLAY_EVENT_LOAD_BITMAP, pstInfo->pstBitmap, orxNULL, &stPayload);

    /* Frees image buffer */
    if(pstInfo->pu8ImageBuffer != pstInfo->pu8ImageSource)
    {
      orxMemory_Free(pstInfo->pu8ImageBuffer);
    }
    pstInfo->pu8ImageBuffer = orxNULL;

    /* Frees source */
    if(pstInfo->pu8ImageSource != orxNULL)
    {
      stbi_image_free(pstInfo->pu8ImageSource);
      pstInfo->pu8ImageSource = orxNULL;
    }

    /* Clears loading flag */
    orxFLAG_SET(pstInfo->pstBitmap->u32Flags, orxDISPLAY_KU32_BITMAP_FLAG_NONE, orxDISPLAY_KU32_BITMAP_FLAG_LOADING);
    orxMEMORY_BARRIER();

    /* Asked for deletion? */
    if(orxFLAG_TEST(pstInfo->pstBitmap->u32Flags, orxDISPLAY_KU32_BITMAP_FLAG_DELETE))
    {
      /* Deletes it */
      orxDisplay_DeleteBitmap(pstInfo->pstBitmap);
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
      stbi_image_free(pstInfo->pu8ImageSource);
      pstInfo->pu8ImageSource = orxNULL;
    }
  }

  /* Frees load info */
  orxMemory_Free(pstInfo);

  /* Done! */
  return eResult;
}

static orxSTATUS orxFASTCALL orxDisplay_Android_DecompressBitmap(void *_pContext)
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
    orxU8  *pu8ImageData;
    GLuint  uiBytesPerPixel;

    /* Loads image */
    pu8ImageData = stbi_load_from_memory((orxU8 *)pstInfo->pu8ImageSource, (int)pstInfo->s64Size, (int *)&(pstInfo->uiWidth), (int *)&(pstInfo->uiHeight), (int *)&uiBytesPerPixel, STBI_rgb_alpha);

    /* Valid? */
    if(pu8ImageData != NULL)
    {
      /* Uses image buffer */
      pstInfo->pu8ImageBuffer = pu8ImageData;

      /* Gets real size */
      pstInfo->uiRealWidth  = pstInfo->uiWidth;
      pstInfo->uiRealHeight = pstInfo->uiHeight;

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

static void orxFASTCALL orxDisplay_Android_ReadResourceCallback(orxHANDLE _hResource, orxS64 _s64Size, void *_pBuffer, void *_pContext)
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
    if(orxThread_RunTask(&orxDisplay_Android_DecompressBitmap, orxDisplay_Android_DecompressBitmapCallback, orxNULL, (void *)pstInfo) == orxSTATUS_FAILURE)
    {
      /* Frees load info */
      orxMemory_Free(pstInfo);
    }
  }
  else
  {
    /* Decompresses bitmap */
    if(orxDisplay_Android_DecompressBitmap(pstInfo) != orxSTATUS_FAILURE)
    {
      /* Upload texture */
      orxDisplay_Android_DecompressBitmapCallback(pstInfo);
    }
  }

  /* Closes resource */
  orxResource_Close(_hResource);
}

static orxSTATUS orxFASTCALL orxDisplay_Android_LoadKTXBitmapData(orxBITMAP *_pstBitmap)
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
    pu8Buffer = (orxU8 *)orxMemory_Allocate((orxU32)s64Size, orxMEMORY_TYPE_MAIN);

    /* Success? */
    if(pu8Buffer != orxNULL)
    {
      /* Asynchronous? */
      if(sstDisplay.pstTempBitmap != orxNULL)
      {
        int iWidth, iHeight;

        /* Gets its info */
        if(getKTXInfos(hResource, &iWidth, &iHeight) != 0)
        {
          /* Resets resource cursor */
          orxResource_Seek(hResource, 0, orxSEEK_OFFSET_WHENCE_START);

          /* Updates asynchronous loading flag */
          orxFLAG_SET(_pstBitmap->u32Flags, orxDISPLAY_KU32_BITMAP_FLAG_LOADING, orxDISPLAY_KU32_BITMAP_FLAG_NONE);

          /* Loads data from resource */
          s64Size = orxResource_Read(hResource, s64Size, pu8Buffer, orxDisplay_Android_ReadKTXResourceCallback, (void *)_pstBitmap);

          /* Successful asynchronous call? */
          if(s64Size < 0)
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
          orxDisplay_Android_ReadKTXResourceCallback(hResource, s64Size, (void *)pu8Buffer, (void *)_pstBitmap);

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

static orxSTATUS orxFASTCALL orxDisplay_Android_LoadBitmapData(orxBITMAP *_pstBitmap)
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
    pu8Buffer = (orxU8 *)orxMemory_Allocate((orxU32)s64Size, orxMEMORY_TYPE_MAIN);

    /* Success? */
    if(pu8Buffer != orxNULL)
    {
      /* Asynchronous? */
      if(sstDisplay.pstTempBitmap != orxNULL)
      {
        int iWidth, iHeight, iComp;

        /* Gets its info */
        if(stbi_info_from_callbacks(&(sstDisplay.stSTBICallbacks), (void *)hResource, &iWidth, &iHeight, &iComp) != 0)
        {
          /* Resets resource cursor */
          orxResource_Seek(hResource, 0, orxSEEK_OFFSET_WHENCE_START);

          /* Updates asynchronous loading flag */
          orxFLAG_SET(_pstBitmap->u32Flags, orxDISPLAY_KU32_BITMAP_FLAG_LOADING, orxDISPLAY_KU32_BITMAP_FLAG_NONE);

          /* Loads data from resource */
          s64Size = orxResource_Read(hResource, s64Size, pu8Buffer, orxDisplay_Android_ReadResourceCallback, (void *)_pstBitmap);

          /* Successful asynchronous call? */
          if(s64Size < 0)
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
          orxDisplay_Android_ReadResourceCallback(hResource, s64Size, (void *)pu8Buffer, (void *)_pstBitmap);

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

static void orxFASTCALL orxDisplay_Android_DeleteBitmapData(orxBITMAP *_pstBitmap)
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


static orxSTATUS orxFASTCALL orxDisplay_Android_SaveBitmapData(void *_pContext)
{
  orxDISPLAY_SAVE_INFO *pstInfo;
  const orxCHAR        *zExtension;
  const orxSTRING       zFilename;
  orxU32                u32Length;
  orxSTATUS             eResult = orxSTATUS_FAILURE;

  /* Gets save info */
  pstInfo = (orxDISPLAY_SAVE_INFO *)_pContext;

  /* Gets filename */
  zFilename = orxString_GetFromID(pstInfo->stFilenameID);

  /* Gets file name's length */
  u32Length = orxString_GetLength(zFilename);

  /* Gets extension */
  zExtension = (u32Length > 3) ? zFilename + u32Length - 3 : orxSTRING_EMPTY;

  /* PNG? */
  if(orxString_ICompare(zExtension, "png") == 0)
  {
    /* Saves image to disk */
    eResult = stbi_write_png(zFilename, pstInfo->u32Width, pstInfo->u32Height, 4, pstInfo->pu8ImageData, 0) != 0 ? orxSTATUS_SUCCESS : orxSTATUS_FAILURE;
  }
  /* BMP? */
  else if(orxString_ICompare(zExtension, "bmp") == 0)
  {
    /* Saves image to disk */
    eResult = stbi_write_bmp(zFilename, pstInfo->u32Width, pstInfo->u32Height, 4, pstInfo->pu8ImageData) != 0 ? orxSTATUS_SUCCESS : orxSTATUS_FAILURE;
  }
  /* TGA */
  else
  {
    /* Saves image to disk */
    eResult = stbi_write_tga(zFilename, pstInfo->u32Width, pstInfo->u32Height, 4, pstInfo->pu8ImageData) != 0 ? orxSTATUS_SUCCESS : orxSTATUS_FAILURE;
  }

  /* Deletes data */
  orxMemory_Free(pstInfo->pu8ImageData);

  /* Deletes save info */
  orxMemory_Free(pstInfo);

  /* Done! */
  return eResult;
}

static orxINLINE orxDISPLAY_MATRIX *orxDisplay_Android_InitMatrix(orxDISPLAY_MATRIX *_pmMatrix, orxFLOAT _fPosX, orxFLOAT _fPosY, orxFLOAT _fScaleX, orxFLOAT _fScaleY, orxFLOAT _fRotation, orxFLOAT _fPivotX, orxFLOAT _fPivotY)
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
  fSCosX = _fScaleX * fCos;
  fSCosY = _fScaleY * fCos;
  fSSinX = _fScaleX * fSin;
  fSSinY = _fScaleY * fSin;
  fTX = _fPosX - (_fPivotX * fSCosX) + (_fPivotY * fSSinY);
  fTY = _fPosY - (_fPivotX * fSSinX) - (_fPivotY * fSCosY);

  /* Updates matrix */
  orxVector_Set(&(_pmMatrix->vX), fSCosX, -fSSinY, fTX);
  orxVector_Set(&(_pmMatrix->vY), fSSinX, fSCosY, fTY);

  /* Done! */
  return _pmMatrix;
}

static orxSTATUS orxFASTCALL orxDisplay_Android_CompileShader(orxDISPLAY_SHADER *_pstShader)
{
  static const orxSTRING szVertexShaderSource =
  "attribute vec2 _vPosition_;"
  "uniform mat4 _mProjection_;"
  "attribute mediump vec2 _vTexCoord_;"
  "varying mediump vec2 _gl_TexCoord0_;"
  "attribute mediump vec4 _vColor_;"
  "varying mediump vec4 _Color0_;"
  "void main()"
  "{"
  "  mediump float fCoef = 1.0 / 255.0;"
  "  gl_Position      = _mProjection_ * vec4(_vPosition_.xy, 0.0, 1.0);"
  "  _gl_TexCoord0_   = _vTexCoord_;"
  "  _Color0_         = fCoef * _vColor_;"
  "}";

  GLuint    uiProgram, uiVertexShader, uiFragmentShader;
  GLint     iSuccess;
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Creates program */
  uiProgram = glCreateProgram();
  glASSERT();

  /* Creates vertex and fragment shaders */
  uiVertexShader   = glCreateShader(GL_VERTEX_SHADER);
  glASSERT();
  uiFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glASSERT();

  /* Compiles shader objects */
  glShaderSource(uiVertexShader, 1, (const char **)&szVertexShaderSource, NULL);
  glASSERT();
  glShaderSource(uiFragmentShader, 1, (const char **)&(_pstShader->zCode), NULL);
  glASSERT();
  glCompileShader(uiVertexShader);
  glASSERT();
  glCompileShader(uiFragmentShader);
  glASSERT();

  /* Gets vertex shader compiling status */
  glGetShaderiv(uiVertexShader, GL_COMPILE_STATUS, &iSuccess);
  glASSERT();

  /* Success? */
  if(iSuccess != GL_FALSE)
  {
    /* Gets fragment shader compiling status */
    glGetShaderiv(uiFragmentShader, GL_COMPILE_STATUS, &iSuccess);
    glASSERT();

    /* Success? */
    if(iSuccess != GL_FALSE)
    {
      /* Attaches shader objects to program */
      glAttachShader(uiProgram, uiVertexShader);
      glASSERT();
      glAttachShader(uiProgram, uiFragmentShader);
      glASSERT();

      /* Deletes shader objects */
      glDeleteShader(uiVertexShader);
      glASSERT();
      glDeleteShader(uiFragmentShader);
      glASSERT();

      /* Binds attributes */
      glBindAttribLocation(uiProgram, orxDISPLAY_ATTRIBUTE_LOCATION_VERTEX, "_vPosition_");
      glASSERT();
      glBindAttribLocation(uiProgram, orxDISPLAY_ATTRIBUTE_LOCATION_TEXCOORD, "_vTexCoord_");
      glASSERT();
      glBindAttribLocation(uiProgram, orxDISPLAY_ATTRIBUTE_LOCATION_COLOR, "_vColor_");
      glASSERT();

      /* Links program */
      glLinkProgram(uiProgram);
      glASSERT();

      /* Gets texture location */
      _pstShader->iTextureLocation = glGetUniformLocation(uiProgram, "_Texture_");
      glASSERT();

      /* Gets projection matrix location */
      _pstShader->iProjectionMatrixLocation = glGetUniformLocation(uiProgram, "_mProjection_");
      glASSERT();

      /* Gets linking status */
      glGetProgramiv(uiProgram, GL_LINK_STATUS, &iSuccess);
      glASSERT();

      /* Success? */
      if(iSuccess != GL_FALSE)
      {
        /* Updates shader */
        _pstShader->uiProgram     = uiProgram;
        _pstShader->iTextureCount = 0;

        /* Updates result */
        eResult = orxSTATUS_SUCCESS;
      }
      else
      {
        orxCHAR acBuffer[4096];

        /* Gets log */
        glGetProgramInfoLog(uiProgram, sizeof(acBuffer) - 1, NULL, (char *)acBuffer);
        glASSERT();
        acBuffer[sizeof(acBuffer) - 1] = orxCHAR_NULL;

        /* Outputs log */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Couldn't link shader program:\n%s\n", acBuffer);

        /* Deletes program */
        glDeleteProgram(uiProgram);
        glASSERT();
      }
    }
    else
    {
      orxCHAR acBuffer[4096];

      /* Gets log */
      glGetShaderInfoLog(uiFragmentShader, sizeof(acBuffer) - 1, NULL, (char *)acBuffer);
      glASSERT();
      acBuffer[sizeof(acBuffer) - 1] = orxCHAR_NULL;

      /* Outputs log */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Couldn't compile fragment shader:\n%s\n", acBuffer);

      /* Deletes shader objects & program */
      glDeleteShader(uiVertexShader);
      glASSERT();
      glDeleteShader(uiFragmentShader);
      glASSERT();
      glDeleteProgram(uiProgram);
      glASSERT();
    }
  }
  else
  {
    orxCHAR acBuffer[4096];

    /* Gets log */
    glGetShaderInfoLog(uiVertexShader, sizeof(acBuffer) - 1, NULL, (char *)acBuffer);
    glASSERT();
    acBuffer[sizeof(acBuffer) - 1] = orxCHAR_NULL;

    /* Outputs log */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Couldn't compile vertex shader:\n%s\n", acBuffer);

    /* Deletes shader objects & program */
    glDeleteShader(uiVertexShader);
    glASSERT();
    glDeleteShader(uiFragmentShader);
    glASSERT();
    glDeleteProgram(uiProgram);
    glASSERT();
  }

  /* Done! */
  return eResult;
}

static void orxFASTCALL orxDisplay_Android_InitShader(orxDISPLAY_SHADER *_pstShader)
{
  /* Uses shader's program */
  glUseProgram(_pstShader->uiProgram);
  glASSERT();

  /* Has custom textures? */
  if(_pstShader->iTextureCount > 0)
  {
    GLint i;

    /* For all defined textures */
    for(i = 0; i < _pstShader->iTextureCount; i++)
    {
      /* Binds bitmap */
      orxDisplay_Android_BindBitmap(_pstShader->astTextureInfoList[i].pstBitmap);

      /* Updates shader uniform */
      glUNIFORM(1i, _pstShader->astTextureInfoList[i].iLocation, sstDisplay.s32ActiveTextureUnit);
    }
  }

  /* Done! */
  return;
}

static void orxFASTCALL orxDisplay_Android_DrawArrays()
{
  /* Has data? */
  if(sstDisplay.s32BufferIndex > 0)
  {
    /* Profiles */
    orxPROFILER_PUSH_MARKER("orxDisplay_DrawArrays");

     /* Indirect mode? */
    if(sstDisplay.eLastBufferMode == orxDISPLAY_BUFFER_MODE_INDIRECT)
    {
      /* Sends vertex buffer */
      glBufferSubData(GL_ARRAY_BUFFER, 0, (GLsizei)(sstDisplay.s32BufferIndex * sizeof(orxDISPLAY_ANDROID_VERTEX)), sstDisplay.astVertexList);
      glASSERT();
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
        orxDisplay_Android_InitShader(pstShader);

        /* Draws elements */
        glDrawElements(sstDisplay.ePrimitive, sstDisplay.s32ElementNumber, GL_UNSIGNED_SHORT, 0);
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

      /* Uses default program */
      orxDisplay_Android_StopShader(orxNULL);
    }
    else
    {
      /* Draws elements */
      glDrawElements(sstDisplay.ePrimitive, sstDisplay.s32ElementNumber, GL_UNSIGNED_SHORT, 0);
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

static void orxFASTCALL orxDisplay_Android_SetBufferMode(orxDISPLAY_BUFFER_MODE _eBufferMode)
{
  /* New blend mode? */
  if(_eBufferMode != sstDisplay.eLastBufferMode)
  {
    /* Draws remaining items */
    orxDisplay_Android_DrawArrays();

    /* Indirect? */
    if(_eBufferMode == orxDISPLAY_BUFFER_MODE_INDIRECT)
    {
      /* Reverts back to default primitive */
      sstDisplay.ePrimitive = orxDISPLAY_KE_DEFAULT_PRIMITIVE;

      /* Inits VBO */
      glBufferData(GL_ARRAY_BUFFER, orxDISPLAY_KU32_VERTEX_BUFFER_SIZE * sizeof(orxDISPLAY_ANDROID_VERTEX), NULL, GL_DYNAMIC_DRAW);
      glASSERT();

      /* Was using custom IBO? */
      if(orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_CUSTOM_IBO))
      {
        /* Fills IBO */
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, orxDISPLAY_KU32_INDEX_BUFFER_SIZE * sizeof(GLushort), sstDisplay.au16IndexList, GL_STATIC_DRAW);
        glASSERT();

        /* Updates flags */
        orxFLAG_SET(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_NONE, orxDISPLAY_KU32_STATIC_FLAG_CUSTOM_IBO);
      }
    }

    /* Stores it */
    sstDisplay.eLastBufferMode = _eBufferMode;
  }

  /* Done! */
  return;
}

static void orxFASTCALL orxDisplay_Android_PrepareBitmap(const orxBITMAP *_pstBitmap, orxDISPLAY_SMOOTHING _eSmoothing, orxDISPLAY_BLEND_MODE _eBlendMode, orxDISPLAY_BUFFER_MODE _eBufferMode)
{
  orxBOOL bSmoothing;

  /* Checks */
  orxASSERT((_pstBitmap != orxNULL) && (_pstBitmap != sstDisplay.pstScreen));

  /* Has pending shaders? */
  if(sstDisplay.s32PendingShaderCount != 0)
  {
    /* Draws remaining items */
    orxDisplay_Android_DrawArrays();

    /* Checks */
    orxASSERT(sstDisplay.s32PendingShaderCount == 0);
  }

  /* New bitmap? */
  if(_pstBitmap != sstDisplay.apstBoundBitmapList[sstDisplay.s32ActiveTextureUnit])
  {
    /* Draws remaining items */
    orxDisplay_Android_DrawArrays();

    /* Binds source bitmap */
    orxDisplay_Android_BindBitmap(_pstBitmap);

    /* No other shader active? */
    if(orxLinkList_GetCount(&(sstDisplay.stActiveShaderList)) == 0)
    {
      /* Updates shader uniform */
      glUNIFORM(1i, sstDisplay.pstDefaultShader->iTextureLocation, sstDisplay.s32ActiveTextureUnit);
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
    orxDisplay_Android_DrawArrays();

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
  orxDisplay_Android_SetBlendMode(_eBlendMode);

  /* Sets buffer mode */
  orxDisplay_Android_SetBufferMode(_eBufferMode);

  /* Done! */
  return;
}

static orxINLINE void orxDisplay_Android_DrawBitmap(const orxBITMAP *_pstBitmap, const orxDISPLAY_MATRIX *_pmTransform, orxDISPLAY_SMOOTHING _eSmoothing, orxDISPLAY_BLEND_MODE _eBlendMode)
{
  GLfloat fWidth, fHeight;

  /* Prepares bitmap for drawing */
  orxDisplay_Android_PrepareBitmap(_pstBitmap, _eSmoothing, _eBlendMode, orxDISPLAY_BUFFER_MODE_INDIRECT);

  /* Gets bitmap working size */
  fWidth = (GLfloat) (_pstBitmap->stClip.vBR.fX - _pstBitmap->stClip.vTL.fX);
  fHeight = (GLfloat) (_pstBitmap->stClip.vBR.fY - _pstBitmap->stClip.vTL.fY);

  /* End of buffer? */
  if(sstDisplay.s32BufferIndex > orxDISPLAY_KU32_VERTEX_BUFFER_SIZE - 5)
  {
    /* Draws arrays */
    orxDisplay_Android_DrawArrays();
  }

  /* Fills the vertex list */
  sstDisplay.astVertexList[sstDisplay.s32BufferIndex].fX     = (_pmTransform->vX.fY * fHeight) + _pmTransform->vX.fZ;
  sstDisplay.astVertexList[sstDisplay.s32BufferIndex].fY     = (_pmTransform->vY.fY * fHeight) + _pmTransform->vY.fZ;
  sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 1].fX = _pmTransform->vX.fZ;
  sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 1].fY = _pmTransform->vY.fZ;
  sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 2].fX = (_pmTransform->vX.fX * fWidth) + (_pmTransform->vX.fY * fHeight) + _pmTransform->vX.fZ;
  sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 2].fY = (_pmTransform->vY.fX * fWidth) + (_pmTransform->vY.fY * fHeight) + _pmTransform->vY.fZ;
  sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 3].fX = (_pmTransform->vX.fX * fWidth) + _pmTransform->vX.fZ;
  sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 3].fY = (_pmTransform->vY.fX * fWidth) + _pmTransform->vY.fZ;

  /* Fills the texture coord list */
  sstDisplay.astVertexList[sstDisplay.s32BufferIndex].fU     =
  sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 1].fU = (GLfloat) (_pstBitmap->fRecRealWidth * (_pstBitmap->stClip.vTL.fX + orxDISPLAY_KF_BORDER_FIX));
  sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 2].fU =
  sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 3].fU = (GLfloat) (_pstBitmap->fRecRealWidth * (_pstBitmap->stClip.vBR.fX - orxDISPLAY_KF_BORDER_FIX));
  sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 1].fV =
  sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 3].fV = (GLfloat)(_pstBitmap->fRecRealHeight * (_pstBitmap->stClip.vTL.fY + orxDISPLAY_KF_BORDER_FIX));
  sstDisplay.astVertexList[sstDisplay.s32BufferIndex].fV     =
  sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 2].fV = (GLfloat)(_pstBitmap->fRecRealHeight * (_pstBitmap->stClip.vBR.fY - orxDISPLAY_KF_BORDER_FIX));

  /* Fills the color list */
  sstDisplay.astVertexList[sstDisplay.s32BufferIndex].stRGBA     =
  sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 1].stRGBA =
  sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 2].stRGBA =
  sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 3].stRGBA = _pstBitmap->stColor;

  /* Updates index & element number */
  sstDisplay.s32BufferIndex   += 4;
  sstDisplay.s32ElementNumber += 6;

  /* Done! */
  return;
}

static void orxFASTCALL orxDisplay_Android_DrawPrimitive(orxU32 _u32VertexNumber, orxRGBA _stColor, orxBOOL _bFill, orxBOOL _bOpen)
{
  /* Profiles */
  orxPROFILER_PUSH_MARKER("orxDisplay_DrawPrimitive");

  /* Starts no texture shader */
  orxDisplay_Android_StartShader((orxHANDLE)sstDisplay.pstNoTextureShader);

  /* Inits it */
  orxDisplay_Android_InitShader(sstDisplay.pstNoTextureShader);

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

  /* Copies vertex buffer */
  glBufferSubData(GL_ARRAY_BUFFER, 0, (GLsizei)(_u32VertexNumber * sizeof(orxDISPLAY_ANDROID_VERTEX)), sstDisplay.astVertexList);
  glASSERT();

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

  /* Bypasses the full screen rendering when stopping shader */
  sstDisplay.s32BufferIndex = -1;

  /* Stops current shader */
  orxDisplay_Android_StopShader((orxHANDLE)sstDisplay.pstNoTextureShader);

  /* Resets buffer index */
  sstDisplay.s32BufferIndex = 0;

  /* Profiles */
  orxPROFILER_POP_MARKER();

  /* Done! */
  return;
}

static orxINLINE GLenum orxDisplay_Android_GetOpenGLPrimitive(orxDISPLAY_PRIMITIVE _ePrimitive)
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

  /* Done! */
  return eResult;
}

orxBITMAP *orxFASTCALL orxDisplay_Android_GetScreenBitmap()
{
  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Done! */
  return sstDisplay.pstScreen;
}

orxSTATUS orxFASTCALL orxDisplay_Android_TransformText(const orxSTRING _zString, const orxBITMAP *_pstFont, const orxCHARACTER_MAP *_pstMap, const orxDISPLAY_TRANSFORM *_pstTransform, orxDISPLAY_SMOOTHING _eSmoothing, orxDISPLAY_BLEND_MODE _eBlendMode)
{
  orxDISPLAY_MATRIX mTransform;
  const orxCHAR *pc;
  orxU32 u32CharacterCodePoint;
  GLfloat fX, fY, fHeight;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_zString != orxNULL);
  orxASSERT(_pstFont != orxNULL);
  orxASSERT(_pstMap != orxNULL);
  orxASSERT(_pstTransform != orxNULL);

  /* Inits matrix */
  orxDisplay_Android_InitMatrix(&mTransform, _pstTransform->fDstX, _pstTransform->fDstY, _pstTransform->fScaleX, _pstTransform->fScaleY, _pstTransform->fRotation, _pstTransform->fSrcX, _pstTransform->fSrcY);

  /* Gets character's height */
  fHeight = _pstMap->fCharacterHeight;

  /* Prepares font for drawing */
  orxDisplay_Android_PrepareBitmap(_pstFont, _eSmoothing, _eBlendMode, orxDISPLAY_BUFFER_MODE_INDIRECT);

  /* For all characters */
  for (u32CharacterCodePoint = orxString_GetFirstCharacterCodePoint(_zString, &pc), fX = 0.0f, fY = 0.0f;
       u32CharacterCodePoint != orxCHAR_NULL;
       u32CharacterCodePoint = orxString_GetFirstCharacterCodePoint(pc, &pc))
  {
    /* Depending on character */
    switch (u32CharacterCodePoint)
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

        /* Gets glyph from table */
        pstGlyph = (orxCHARACTER_GLYPH *) orxHashTable_Get(_pstMap->pstCharacterTable, u32CharacterCodePoint);
        orxFLOAT                  fWidth;

        /* Valid? */
        if(pstGlyph != orxNULL)
        {
          /* Gets character width */
          fWidth = pstGlyph->fWidth;

          /* End of buffer? */
          if(sstDisplay.s32BufferIndex > orxDISPLAY_KU32_VERTEX_BUFFER_SIZE - 5)
          {
            /* Draws arrays */
            orxDisplay_Android_DrawArrays();
          }

          /* Outputs vertices and texture coordinates */
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex].fX     = (mTransform.vX.fX * fX) + (mTransform.vX.fY * (fY + fHeight)) + mTransform.vX.fZ;
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex].fY     = (mTransform.vY.fX * fX) + (mTransform.vY.fY * (fY + fHeight)) + mTransform.vY.fZ;
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 1].fX = (mTransform.vX.fX * fX) + (mTransform.vX.fY * fY) + mTransform.vX.fZ;
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 1].fY = (mTransform.vY.fX * fX) + (mTransform.vY.fY * fY) + mTransform.vY.fZ;
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 2].fX = (mTransform.vX.fX * (fX + fWidth)) + (mTransform.vX.fY * (fY + fHeight)) + mTransform.vX.fZ;
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 2].fY = (mTransform.vY.fX * (fX + fWidth)) + (mTransform.vY.fY * (fY + fHeight)) + mTransform.vY.fZ;
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 3].fX = (mTransform.vX.fX * (fX + fWidth)) + (mTransform.vX.fY * fY) + mTransform.vX.fZ;
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 3].fY = (mTransform.vY.fX * (fX + fWidth)) + (mTransform.vY.fY * fY) + mTransform.vY.fZ;

          sstDisplay.astVertexList[sstDisplay.s32BufferIndex].fU     =
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 1].fU = (GLfloat) (_pstFont->fRecRealWidth * (pstGlyph->fX + orxDISPLAY_KF_BORDER_FIX));
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 2].fU =
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 3].fU = (GLfloat) (_pstFont->fRecRealWidth * (pstGlyph->fX + fWidth - orxDISPLAY_KF_BORDER_FIX));
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 1].fV =
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 3].fV = (GLfloat)(_pstFont->fRecRealHeight * (pstGlyph->fY + orxDISPLAY_KF_BORDER_FIX));
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex].fV     =
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 2].fV = (GLfloat)(_pstFont->fRecRealHeight * (pstGlyph->fY + fHeight - orxDISPLAY_KF_BORDER_FIX));

          /* Fills the color list */
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex].stRGBA     =
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 1].stRGBA =
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 2].stRGBA =
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 3].stRGBA = _pstFont->stColor;

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

orxSTATUS orxFASTCALL orxDisplay_Android_DrawLine(const orxVECTOR *_pvStart, const orxVECTOR *_pvEnd, orxRGBA _stColor)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pvStart != orxNULL);
  orxASSERT(_pvEnd != orxNULL);

  /* Draws remaining items */
  orxDisplay_Android_DrawArrays();

  /* Copies vertices */
  sstDisplay.astVertexList[0].fX = (GLfloat)(_pvStart->fX);
  sstDisplay.astVertexList[0].fY = (GLfloat)(_pvStart->fY);
  sstDisplay.astVertexList[1].fX = (GLfloat)(_pvEnd->fX);
  sstDisplay.astVertexList[1].fY = (GLfloat)(_pvEnd->fY);

  /* Copies color */
  sstDisplay.astVertexList[0].stRGBA =
  sstDisplay.astVertexList[1].stRGBA = _stColor;

  /* Draws it */
  orxDisplay_Android_DrawPrimitive(2, _stColor, orxFALSE, orxTRUE);

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_Android_DrawPolyline(const orxVECTOR *_avVertexList, orxU32 _u32VertexNumber, orxRGBA _stColor)
{
  orxU32    i;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_avVertexList != orxNULL);
  orxASSERT(_u32VertexNumber > 0);

  /* Draws remaining items */
  orxDisplay_Android_DrawArrays();

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
  orxDisplay_Android_DrawPrimitive(_u32VertexNumber, _stColor, orxFALSE, orxTRUE);

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_Android_DrawPolygon(const orxVECTOR *_avVertexList, orxU32 _u32VertexNumber, orxRGBA _stColor, orxBOOL _bFill)
{
  orxU32    i;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_avVertexList != orxNULL);
  orxASSERT(_u32VertexNumber > 0);

  /* Draws remaining items */
  orxDisplay_Android_DrawArrays();

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
  orxDisplay_Android_DrawPrimitive(_u32VertexNumber, _stColor, _bFill, orxFALSE);

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_Android_DrawCircle(const orxVECTOR *_pvCenter, orxFLOAT _fRadius, orxRGBA _stColor, orxBOOL _bFill)
{
  orxU32    i;
  orxFLOAT  fAngle;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pvCenter != orxNULL);
  orxASSERT(_fRadius >= orxFLOAT_0);

  /* Draws remaining items */
  orxDisplay_Android_DrawArrays();

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
  orxDisplay_Android_DrawPrimitive(orxDISPLAY_KU32_CIRCLE_LINE_NUMBER, _stColor, _bFill, orxFALSE);

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_Android_DrawOBox(const orxOBOX *_pstBox, orxRGBA _stColor, orxBOOL _bFill)
{
  orxVECTOR vOrigin;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBox != orxNULL);

  /* Draws remaining items */
  orxDisplay_Android_DrawArrays();

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
  orxDisplay_Android_DrawPrimitive(4, _stColor, _bFill, orxFALSE);

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_Android_DrawMesh(const orxDISPLAY_MESH *_pstMesh, const orxBITMAP *_pstBitmap, orxDISPLAY_SMOOTHING _eSmoothing, orxDISPLAY_BLEND_MODE _eBlendMode)
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
  orxDisplay_Android_PrepareBitmap(pstBitmap, _eSmoothing, _eBlendMode, orxDISPLAY_BUFFER_MODE_DIRECT);

  /* Stores primitive */
  sstDisplay.ePrimitive = orxDisplay_Android_GetOpenGLPrimitive(_pstMesh->ePrimitive);

  /* Gets element number */
  u32ElementNumber = ((_pstMesh->u32IndexNumber != 0) && (_pstMesh->au16IndexList != orxNULL)) ? _pstMesh->u32IndexNumber : _pstMesh->u32VertexNumber + (_pstMesh->u32VertexNumber >> 1);

  /* Fills VBO */
  glBufferData(GL_ARRAY_BUFFER, _pstMesh->u32VertexNumber * sizeof(orxDISPLAY_ANDROID_VERTEX), _pstMesh->astVertexList, GL_STREAM_DRAW);
  glASSERT();

  /* Has index buffer? */
  if((_pstMesh->au16IndexList != orxNULL)
  && (_pstMesh->u32IndexNumber > 1))
  {
    /* Fills IBO */
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, _pstMesh->u32IndexNumber * sizeof(GLushort), _pstMesh->au16IndexList, GL_STREAM_DRAW);
    glASSERT();

    /* Updates flags */
    orxFLAG_SET(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_CUSTOM_IBO, orxDISPLAY_KU32_STATIC_FLAG_NONE);
  }

  /* Updates buffer index */
  sstDisplay.s32BufferIndex = _pstMesh->u32VertexNumber;

  /* Updates element number */
  sstDisplay.s32ElementNumber = u32ElementNumber;

  /* Draws mesh */
  orxDisplay_Android_DrawArrays();

  /* Done! */
  return eResult;
}

void orxFASTCALL orxDisplay_Android_DeleteBitmap(orxBITMAP *_pstBitmap)
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
      orxDisplay_Android_DeleteBitmapData(_pstBitmap);

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

orxBITMAP *orxFASTCALL orxDisplay_Android_CreateBitmap(orxU32 _u32Width, orxU32 _u32Height)
{
  orxBITMAP *pstBitmap;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Allocates bitmap */
  pstBitmap = (orxBITMAP *) orxBank_Allocate(sstDisplay.pstBitmapBank);

  /* Valid? */
  if(pstBitmap != orxNULL)
  {
    /* Pushes display section */
    orxConfig_PushSection(orxDISPLAY_KZ_CONFIG_SECTION);

    /* Inits it */
    pstBitmap->bSmoothing     = sstDisplay.bDefaultSmoothing;
    pstBitmap->fWidth         = orxU2F(_u32Width);
    pstBitmap->fHeight        = orxU2F(_u32Height);
    pstBitmap->u32RealWidth   = _u32Width;
    pstBitmap->u32RealHeight  = _u32Height;
    pstBitmap->u32Depth       = 32;
    pstBitmap->fRecRealWidth  = orxFLOAT_1 / orxU2F(pstBitmap->u32RealWidth);
    pstBitmap->fRecRealHeight = orxFLOAT_1 / orxU2F(pstBitmap->u32RealHeight);
    pstBitmap->u32DataSize    = pstBitmap->u32RealWidth * pstBitmap->u32RealHeight * 4 * sizeof(orxU8);
    pstBitmap->stColor        = orx2RGBA(0xFF, 0xFF, 0xFF, 0xFF);
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
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, pstBitmap->u32RealWidth, pstBitmap->u32RealHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
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

    /* Pops config section */
    orxConfig_PopSection();
  }

  /* Done! */
  return pstBitmap;
}

orxSTATUS orxFASTCALL orxDisplay_Android_ClearBitmap(orxBITMAP *_pstBitmap, orxRGBA _stColor)
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
    if(orxDisplay_Android_SetDestinationBitmaps(&_pstBitmap, 1) != orxSTATUS_FAILURE)
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
      orxDisplay_Android_SetDestinationBitmaps(apstBackupBitmap, u32BackupBitmapCount);
    }
    /* Not screen? */
    else if(_pstBitmap != sstDisplay.pstScreen)
    {
      orxRGBA *astBuffer, *pstPixel;

      /* Allocates buffer */
      astBuffer = (orxRGBA *)orxMemory_Allocate(_pstBitmap->u32RealWidth * _pstBitmap->u32RealHeight * sizeof(orxRGBA), orxMEMORY_TYPE_MAIN);

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

orxSTATUS orxFASTCALL orxDisplay_Android_SetBlendMode(orxDISPLAY_BLEND_MODE _eBlendMode)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* New blend mode? */
  if(_eBlendMode != sstDisplay.eLastBlendMode)
  {
    /* Draws remaining items */
    orxDisplay_Android_DrawArrays();

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

orxSTATUS orxFASTCALL orxDisplay_Android_Swap()
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Draws remaining items */
  orxDisplay_Android_DrawArrays();

  /* Swaps buffers */
  eglSwapBuffers(sstDisplay.display, sstDisplay.surface);
  eglASSERT();

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_Android_SetBitmapData(orxBITMAP *_pstBitmap, const orxU8 *_au8Data, orxU32 _u32ByteNumber)
{
  orxU32 u32Width, u32Height;
  orxSTATUS eResult;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBitmap != orxNULL);
  orxASSERT(_au8Data != orxNULL);

  /* Gets bitmap's size */
  u32Width = orxF2U(_pstBitmap->fWidth);
  u32Height = orxF2U(_pstBitmap->fHeight);

  /* Valid? */
  if((_pstBitmap != sstDisplay.pstScreen) && (_u32ByteNumber == u32Width * u32Height * sizeof(orxRGBA)))
  {
    orxU8 *pu8ImageBuffer;

    pu8ImageBuffer = (orxU8 *)_au8Data;

    /* Binds texture */
    glBindTexture(GL_TEXTURE_2D, _pstBitmap->uiTexture);
    glASSERT();

    /* Updates its content */
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, _pstBitmap->u32RealWidth, _pstBitmap->u32RealHeight, GL_RGBA, GL_UNSIGNED_BYTE, pu8ImageBuffer);
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
      orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Can't set bitmap data: format needs to be RGBA.");
    }

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_Android_GetBitmapData(const orxBITMAP *_pstBitmap, orxU8 *_au8Data, orxU32 _u32ByteNumber)
{
  orxU32      u32BufferSize;
  orxSTATUS   eResult;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBitmap != orxNULL);
  orxASSERT(_au8Data != orxNULL);

  /* Gets buffer size */
  u32BufferSize = orxF2U(_pstBitmap->fWidth * _pstBitmap->fHeight) * 4 * sizeof(orxU8);

  /* Is size matching? */
  if(_u32ByteNumber == u32BufferSize)
  {
    orxBITMAP  *apstBackupBitmap[orxDISPLAY_KU32_MAX_TEXTURE_UNIT_NUMBER];
    orxU32      u32BackupBitmapCount;

    /* Backups current destinations */
    orxMemory_Copy(apstBackupBitmap, sstDisplay.apstDestinationBitmapList, sstDisplay.u32DestinationBitmapCount * sizeof(orxBITMAP *));
    u32BackupBitmapCount = sstDisplay.u32DestinationBitmapCount;

    /* Sets new destination bitmap */
    if((eResult = orxDisplay_Android_SetDestinationBitmaps((orxBITMAP **)&_pstBitmap, 1)) != orxSTATUS_FAILURE)
    {
      orxU8  *pu8ImageBuffer;

      /* Allocates buffer */
      pu8ImageBuffer = (_pstBitmap != sstDisplay.pstScreen) ? _au8Data : (orxU8 *)orxMemory_Allocate(_pstBitmap->u32RealWidth * _pstBitmap->u32RealHeight * 4 * sizeof(orxU8), orxMEMORY_TYPE_VIDEO);

      /* Checks */
      orxASSERT(pu8ImageBuffer != orxNULL);

      /* Reads OpenGL data */
      glReadPixels(0, 0, _pstBitmap->u32RealWidth, _pstBitmap->u32RealHeight, GL_RGBA, GL_UNSIGNED_BYTE, pu8ImageBuffer);
      glASSERT();

      if(_pstBitmap == sstDisplay.pstScreen)
      {
        orxRGBA stOpaque;
        orxU32  u32LineSize, u32RealLineSize, u32SrcOffset, u32DstOffset, i;

        /* Sets opaque pixel */
        stOpaque = orx2RGBA(0x00, 0x00, 0x00, 0xFF);

        /* Gets line sizes */
        u32LineSize     = orxF2U(_pstBitmap->fWidth) * 4 * sizeof(orxU8);
        u32RealLineSize = _pstBitmap->u32RealWidth * 4 * sizeof(orxU8);

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

      /* Restores previous destinations */
      orxDisplay_Android_SetDestinationBitmaps(apstBackupBitmap, u32BackupBitmapCount);
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Can't get bitmap's data <0x%X> as the buffer size is %ld when it should be %ls.", _pstBitmap, _u32ByteNumber, u32BufferSize);

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_Android_SetBitmapColorKey(orxBITMAP *_pstBitmap, orxRGBA _stColor, orxBOOL _bEnable)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Not available */
  orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Not available on this platform!");

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_Android_SetBitmapColor(orxBITMAP *_pstBitmap, orxRGBA _stColor)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBitmap != orxNULL);

  /* Not screen? */
  if(_pstBitmap != sstDisplay.pstScreen)
  {
    /* Stores it */
    _pstBitmap->stColor = _stColor;
  }

  /* Done! */
  return eResult;
}

orxRGBA orxFASTCALL orxDisplay_Android_GetBitmapColor(const orxBITMAP *_pstBitmap)
{
  orxRGBA stResult = orx2RGBA(0, 0, 0, 0);

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBitmap != orxNULL);

  /* Not screen? */
  if(_pstBitmap != sstDisplay.pstScreen)
  {
    /* Updates result */
    stResult = _pstBitmap->stColor;
  }

  /* Done! */
  return stResult;
}

orxSTATUS orxFASTCALL orxDisplay_Android_SetDestinationBitmaps(orxBITMAP **_apstBitmapList, orxU32 _u32Number)
{
  orxU32    i, u32Number;
  orxBOOL   bUseFrameBuffer = orxFALSE;
  orxFLOAT  fOrthoRight, fOrthoBottom;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_apstBitmapList != orxNULL)

  /* Too many destinations? */
  if(_u32Number > (orxU32)sstDisplay.iDrawBufferNumber)
  {
    /* Outputs logs */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Can only attach the first <%d> bitmaps as destinations, out of the <%d> requested.", sstDisplay.iDrawBufferNumber, _u32Number);

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
    orxBOOL bDraw;

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
        orxDisplay_Android_DrawArrays();
      }

      /* Using framebuffer? */
      if(bUseFrameBuffer != orxFALSE)
      {
        /* Different framebuffer? */
        if(sstDisplay.uiFrameBuffer != sstDisplay.uiLastFrameBuffer)
        {
          /* Binds frame buffer */
          glBindFramebuffer(GL_FRAMEBUFFER, sstDisplay.uiFrameBuffer);
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
              glBindFramebuffer(GL_FRAMEBUFFER, 0);
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
          glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, pstBitmap->uiTexture, 0);
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
          glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + j, GL_TEXTURE_2D, 0, 0);
          glASSERT();
        }

        /* Supports more than a single draw buffer? */
        if((sstDisplay.iDrawBufferNumber > 1) && (i != 0))
        {
          /* Updates draw buffers */
          glDrawBuffers((GLsizei)i, sstDisplay.aeDrawBufferList);
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
      eResult = (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE) ? orxSTATUS_SUCCESS : orxSTATUS_FAILURE;
      glASSERT();
    }
  }

  /* Success? */
  if(eResult != orxSTATUS_FAILURE)
  {
    GLint   iX, iY;
    GLsizei iWidth, iHeight;

    /* Is screen? */
    if(sstDisplay.apstDestinationBitmapList[0] == sstDisplay.pstScreen)
    {
      /* Updates viewport info */
      iX      = 0;
      iY      = 0;
      iWidth  = (GLsizei)orxF2S(sstDisplay.apstDestinationBitmapList[0]->fWidth);
      iHeight = (GLsizei)orxF2S(sstDisplay.apstDestinationBitmapList[0]->fHeight);

      /* Updates ortho info */
      fOrthoRight   = sstDisplay.apstDestinationBitmapList[0]->fWidth;
      fOrthoBottom  = sstDisplay.apstDestinationBitmapList[0]->fHeight;
    }
    else
    {
      /* Supports more than a single draw buffer? */
      if(sstDisplay.iDrawBufferNumber > 1)
      {
        /* Updates draw buffers */
        glDrawBuffers((GLsizei)sstDisplay.u32DestinationBitmapCount, sstDisplay.aeDrawBufferList);
        glASSERT();
      }

      /* Updates viewport info */
      iX      = 0;
      iY      = 0;
      iWidth  = (GLsizei)orxF2S(sstDisplay.apstDestinationBitmapList[0]->fWidth);
      iHeight = (GLsizei)orxF2S(sstDisplay.apstDestinationBitmapList[0]->fHeight);

      /* Updates ortho info */
      fOrthoRight   = sstDisplay.apstDestinationBitmapList[0]->fWidth;
      fOrthoBottom  = -sstDisplay.apstDestinationBitmapList[0]->fHeight;
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
      (fOrthoBottom >= orxFLOAT_0)
      ? orxDisplay_Android_OrthoProjMatrix(&(sstDisplay.mProjectionMatrix), orxFLOAT_0, fOrthoRight, fOrthoBottom, orxFLOAT_0, -orxFLOAT_1, orxFLOAT_1)
      : orxDisplay_Android_OrthoProjMatrix(&(sstDisplay.mProjectionMatrix), orxFLOAT_0, fOrthoRight, orxFLOAT_0, -fOrthoBottom, -orxFLOAT_1, orxFLOAT_1);

      /* Passes it to shader */
      glUNIFORM(Matrix4fv, sstDisplay.pstDefaultShader->iProjectionMatrixLocation, 1, GL_FALSE, (GLfloat *)&(sstDisplay.mProjectionMatrix.aafValueList[0][0]));
    }
  }

  /* Done! */
  return eResult;
}

orxU32 orxFASTCALL orxDisplay_Android_GetBitmapID(const orxBITMAP *_pstBitmap)
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

orxSTATUS orxFASTCALL orxDisplay_Android_TransformBitmap(const orxBITMAP *_pstSrc, const orxDISPLAY_TRANSFORM *_pstTransform, orxDISPLAY_SMOOTHING _eSmoothing, orxDISPLAY_BLEND_MODE _eBlendMode)
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
      orxDisplay_Android_DrawArrays();
    }

    /* Sets buffer mode */
    orxDisplay_Android_SetBufferMode(orxDISPLAY_BUFFER_MODE_INDIRECT);

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
    sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 3].stRGBA  = sstDisplay.pstScreen->stColor;

    /* Updates index & element number */
    sstDisplay.s32BufferIndex   += 4;
    sstDisplay.s32ElementNumber += 6;

    /* Draws arrays */
    orxDisplay_Android_DrawArrays();
  }
  else
  {
    orxDISPLAY_MATRIX mTransform;

    /* Inits matrix */
    orxDisplay_Android_InitMatrix(&mTransform, _pstTransform->fDstX, _pstTransform->fDstY, _pstTransform->fScaleX, _pstTransform->fScaleY, _pstTransform->fRotation, _pstTransform->fSrcX, _pstTransform->fSrcY);

    /* No repeat? */
    if((_pstTransform->fRepeatX == orxFLOAT_1) && (_pstTransform->fRepeatY == orxFLOAT_1))
    {
      /* Draws it */
      orxDisplay_Android_DrawBitmap(_pstSrc, &mTransform, _eSmoothing, _eBlendMode);
    }
    else
    {
      orxFLOAT i, j, fRecRepeatX;
      GLfloat fX, fY, fWidth, fHeight, fTop, fBottom, fLeft, fRight;

      /* Prepares bitmap for drawing */
      orxDisplay_Android_PrepareBitmap(_pstSrc, _eSmoothing, _eBlendMode, orxDISPLAY_BUFFER_MODE_INDIRECT);

      /* Inits bitmap height */
      fHeight = (GLfloat) ((_pstSrc->stClip.vBR.fY - _pstSrc->stClip.vTL.fY) / _pstTransform->fRepeatY);

      /* Inits texture coords */
      fLeft = _pstSrc->fRecRealWidth * (_pstSrc->stClip.vTL.fX + orxDISPLAY_KF_BORDER_FIX);
      fTop  = _pstSrc->fRecRealHeight * (_pstSrc->stClip.vTL.fY + orxDISPLAY_KF_BORDER_FIX);

      /* For all lines */
      for (fY = 0.0f, i = _pstTransform->fRepeatY, fRecRepeatX = orxFLOAT_1 / _pstTransform->fRepeatX; i > orxFLOAT_0; i -= orxFLOAT_1, fY += fHeight)
      {
        /* Partial line? */
        if(i < orxFLOAT_1)
        {
          /* Updates height */
          fHeight *= (GLfloat) i;

          /* Resets texture coords */
          fRight = (GLfloat)(_pstSrc->fRecRealWidth * (_pstSrc->stClip.vBR.fX - orxDISPLAY_KF_BORDER_FIX));
          fBottom = (GLfloat)(_pstSrc->fRecRealHeight * (_pstSrc->stClip.vTL.fY + (i * (_pstSrc->stClip.vBR.fY - _pstSrc->stClip.vTL.fY)) - orxDISPLAY_KF_BORDER_FIX));
        }
        else
        {
          /* Resets texture coords */
          fRight  = (GLfloat)(_pstSrc->fRecRealWidth * (_pstSrc->stClip.vBR.fX - orxDISPLAY_KF_BORDER_FIX));
          fBottom = (GLfloat)(_pstSrc->fRecRealHeight * (_pstSrc->stClip.vBR.fY - orxDISPLAY_KF_BORDER_FIX));
        }

        /* Resets bitmap width */
        fWidth = (GLfloat) ((_pstSrc->stClip.vBR.fX - _pstSrc->stClip.vTL.fX) * fRecRepeatX);

        /* For all columns */
        for (fX = 0.0f, j = _pstTransform->fRepeatX; j > orxFLOAT_0; j -= orxFLOAT_1, fX += fWidth)
        {
          /* Partial column? */
          if(j < orxFLOAT_1)
          {
            /* Updates width */
            fWidth *= (GLfloat) j;

            /* Updates texture right coord */
            fRight = (GLfloat) (_pstSrc->fRecRealWidth * (_pstSrc->stClip.vTL.fX + (j * (_pstSrc->stClip.vBR.fX - _pstSrc->stClip.vTL.fX))));
          }

          /* End of buffer? */
          if(sstDisplay.s32BufferIndex > orxDISPLAY_KU32_VERTEX_BUFFER_SIZE - 5)
          {
            /* Draws arrays */
            orxDisplay_Android_DrawArrays();
          }

          /* Outputs vertices and texture coordinates */
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex].fX     = (mTransform.vX.fX * fX) + (mTransform.vX.fY * (fY + fHeight)) + mTransform.vX.fZ;
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex].fY     = (mTransform.vY.fX * fX) + (mTransform.vY.fY * (fY + fHeight)) + mTransform.vY.fZ;
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 1].fX = (mTransform.vX.fX * fX) + (mTransform.vX.fY * fY) + mTransform.vX.fZ;
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 1].fY = (mTransform.vY.fX * fX) + (mTransform.vY.fY * fY) + mTransform.vY.fZ;
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 2].fX = (mTransform.vX.fX * (fX + fWidth)) + (mTransform.vX.fY * (fY + fHeight)) + mTransform.vX.fZ;
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 2].fY = (mTransform.vY.fX * (fX + fWidth)) + (mTransform.vY.fY * (fY + fHeight)) + mTransform.vY.fZ;
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 3].fX = (mTransform.vX.fX * (fX + fWidth)) + (mTransform.vX.fY * fY) + mTransform.vX.fZ;
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 3].fY = (mTransform.vY.fX * (fX + fWidth)) + (mTransform.vY.fY * fY) + mTransform.vY.fZ;

          sstDisplay.astVertexList[sstDisplay.s32BufferIndex].fU     =
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 1].fU = fLeft;
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 2].fU =
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 3].fU = fRight;
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 1].fV =
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 3].fV = fTop;
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex].fV     =
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 2].fV = fBottom;

          /* Fills the color list */
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex].stRGBA     =
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 1].stRGBA =
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 2].stRGBA =
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 3].stRGBA = _pstSrc->stColor;

          /* Updates index & element number */
          sstDisplay.s32BufferIndex   += 4;
          sstDisplay.s32ElementNumber += 6;
        }
      }
    }
  }

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_Android_SaveBitmap(const orxBITMAP *_pstBitmap, const orxSTRING _zFilename)
{
  orxU32          u32BufferSize;
  orxU8          *pu8ImageData;
  orxSTATUS       eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBitmap != orxNULL);
  orxASSERT(_zFilename != orxNULL);

  /* Gets buffer size */
  u32BufferSize = orxF2U(_pstBitmap->fWidth * _pstBitmap->fHeight) * 4 * sizeof(orxU8);

  /* Allocates buffer */
  pu8ImageData = (orxU8 *)orxMemory_Allocate(u32BufferSize, orxMEMORY_TYPE_MAIN);

  /* Valid? */
  if(pu8ImageData != orxNULL)
  {
    orxDISPLAY_SAVE_INFO *pstInfo = orxNULL;

    /* Gets bitmap data */
    if(orxDisplay_Android_GetBitmapData(_pstBitmap, pu8ImageData, u32BufferSize) != orxSTATUS_FAILURE)
    {
      /* Allocates save info */
      pstInfo = (orxDISPLAY_SAVE_INFO *)orxMemory_Allocate(sizeof(orxDISPLAY_SAVE_INFO), orxMEMORY_TYPE_TEMP);

      /* Valid? */
      if(pstInfo != orxNULL)
      {
        /* Inits it */
        pstInfo->pu8ImageData   = pu8ImageData;
        pstInfo->stFilenameID   = orxString_GetID(_zFilename);
        pstInfo->u32Width       = orxF2U(_pstBitmap->fWidth);
        pstInfo->u32Height      = orxF2U(_pstBitmap->fHeight);

        /* Runs asynchronous task */
        eResult = orxThread_RunTask(&orxDisplay_Android_SaveBitmapData, orxNULL, orxNULL, (void *)pstInfo);
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

orxSTATUS orxFASTCALL orxDisplay_Android_SetTempBitmap(const orxBITMAP *_pstBitmap)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Stores it */
  sstDisplay.pstTempBitmap = _pstBitmap;

  /* Done! */
  return eResult;
}

const orxBITMAP *orxFASTCALL orxDisplay_Android_GetTempBitmap()
{
  const orxBITMAP *pstResult;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Updates result */
  pstResult = sstDisplay.pstTempBitmap;

  /* Done! */
  return pstResult;
}

orxBITMAP *orxFASTCALL orxDisplay_Android_LoadBitmap(const orxSTRING _zFilename)
{
  const orxSTRING zResourceName;
  orxHANDLE       hResource;
  orxBITMAP      *pstResult = orxNULL;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_zFilename != orxNULL);

  /* Gets resource name */
  zResourceName = orxResource_Locate(orxTEXTURE_KZ_RESOURCE_GROUP, _zFilename);

  /* Success? */
  if(zResourceName != orxNULL)
  {
    pstResult = (orxBITMAP *)orxBank_Allocate(sstDisplay.pstBitmapBank);

    /* Valid? */
    if(pstResult != orxNULL)
    {
      /* Inits it */
      pstResult->bSmoothing     = sstDisplay.bDefaultSmoothing;
      pstResult->zLocation      = zResourceName;
      pstResult->stFilenameID   = orxString_GetID(_zFilename);
      pstResult->u32Flags       = orxDISPLAY_KU32_BITMAP_FLAG_NONE;

      /* Loads its data */
      if(orxString_SearchString(_zFilename, szKTXExtention) != orxNULL)
      {
        if(orxDisplay_Android_LoadKTXBitmapData(pstResult) == orxSTATUS_FAILURE)
        {
          /* Deletes it */
          orxBank_Free(sstDisplay.pstBitmapBank, pstResult);

          /* Updates result */
          pstResult = orxNULL;
        }
      }
      else if(orxDisplay_Android_LoadBitmapData(pstResult) == orxSTATUS_FAILURE)
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

orxSTATUS orxFASTCALL orxDisplay_Android_GetBitmapSize(const orxBITMAP *_pstBitmap, orxFLOAT *_pfWidth, orxFLOAT *_pfHeight)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBitmap != orxNULL);
  orxASSERT(_pfWidth != orxNULL);
  orxASSERT(_pfHeight != orxNULL);

  /* Gets size */
  *_pfWidth = _pstBitmap->fWidth;
  *_pfHeight = _pstBitmap->fHeight;

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_Android_GetScreenSize(orxFLOAT *_pfWidth, orxFLOAT *_pfHeight)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pfWidth != orxNULL);
  orxASSERT(_pfHeight != orxNULL);

  /* Gets size */
  *_pfWidth = sstDisplay.pstScreen->fWidth;
  *_pfHeight = sstDisplay.pstScreen->fHeight;

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_Android_SetBitmapClipping(orxBITMAP *_pstBitmap, orxU32 _u32TLX, orxU32 _u32TLY, orxU32 _u32BRX, orxU32 _u32BRY)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* No destination bitmap? */
  if(_pstBitmap == orxNULL)
  {
    /* Defaults to destination */
    _pstBitmap = sstDisplay.apstDestinationBitmapList[0];
  }

  /* Destination bitmap? */
  if(_pstBitmap == sstDisplay.apstDestinationBitmapList[0])
  {
    orxU32 u32ClipX, u32ClipY, u32ClipWidth, u32ClipHeight;

    /* Draws remaining items */
    orxDisplay_Android_DrawArrays();

    /* Gets new clipping values */
    u32ClipX      = _u32TLX;
    u32ClipY      = (_pstBitmap == sstDisplay.pstScreen) ? orxF2U(sstDisplay.apstDestinationBitmapList[0]->fHeight) - _u32BRY : _u32TLY;
    u32ClipWidth  = _u32BRX - _u32TLX;
    u32ClipHeight = _u32BRY - _u32TLY;

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

orxSTATUS orxFASTCALL orxDisplay_Android_EnableVSync(orxBOOL _bEnable)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Not available */
  orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Not available on this platform!");

  /* Done! */
  return eResult;
}

orxBOOL orxFASTCALL orxDisplay_Android_IsVSyncEnabled()
{
  orxBOOL bResult = orxTRUE;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Not available */
  orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Not available on this platform!");

  /* Done! */
  return bResult;
}

orxSTATUS orxFASTCALL orxDisplay_Android_SetFullScreen(orxBOOL _bFullScreen)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Not available */
  orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Not available on this platform!");

  /* Done! */
  return eResult;
}

orxBOOL orxFASTCALL orxDisplay_Android_IsFullScreen()
{
  orxBOOL bResult = orxTRUE;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Not available */
  orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Not available on this platform!");

  /* Done! */
  return bResult;
}

orxU32 orxFASTCALL orxDisplay_Android_GetVideoModeCount()
{
  orxU32 u32Result = 1;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Done! */
  return u32Result;
}

orxDISPLAY_VIDEO_MODE *orxFASTCALL orxDisplay_Android_GetVideoMode(orxU32 _u32Index, orxDISPLAY_VIDEO_MODE *_pstVideoMode)
{
  orxDISPLAY_VIDEO_MODE *pstResult;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Gets default mode */
  _pstVideoMode->u32Width       = orxF2U(sstDisplay.pstScreen->fWidth);
  _pstVideoMode->u32Height      = orxF2U(sstDisplay.pstScreen->fHeight);
  _pstVideoMode->u32Depth       = sstDisplay.u32Depth;
  _pstVideoMode->u32RefreshRate = 60;
  _pstVideoMode->bFullScreen    = orxTRUE;

  /* Updates result */
  pstResult = _pstVideoMode;

  /* Done! */
  return pstResult;
}

orxSTATUS orxFASTCALL orxDisplay_Android_SetVideoMode(const orxDISPLAY_VIDEO_MODE *_pstVideoMode)
{
  orxS32    i;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Draws remaining items */
  orxDisplay_Android_DrawArrays();

  /* Has specified video mode? */
  if(_pstVideoMode != orxNULL)
  {
    int iWidth, iHeight;

    /* recreate surface */
    orxAndroid_Display_DestroySurface();
    eResult = orxAndroid_Display_CreateSurface();

    if( eResult == orxSTATUS_SUCCESS )
    {
      int iDepth, iRefreshRate;

      /* Gets its info */
      eglQuerySurface(sstDisplay.display, sstDisplay.surface, EGL_WIDTH, &iWidth);
      eglASSERT();
      eglQuerySurface(sstDisplay.display, sstDisplay.surface, EGL_HEIGHT, &iHeight);
      eglASSERT();
      iDepth        = (int)_pstVideoMode->u32Depth;
      iRefreshRate  = (int)_pstVideoMode->u32RefreshRate;

      orxDISPLAY_EVENT_PAYLOAD stPayload;

      /* Inits event payload */
      orxMemory_Zero(&stPayload, sizeof(orxDISPLAY_EVENT_PAYLOAD));
      stPayload.stVideoMode.u32Width                = (orxU32)iWidth;
      stPayload.stVideoMode.u32Height               = (orxU32)iHeight;
      stPayload.stVideoMode.u32Depth                = (orxU32)iDepth;
      stPayload.stVideoMode.u32RefreshRate          = (orxU32)iRefreshRate;
      stPayload.stVideoMode.u32PreviousWidth        = orxF2U(sstDisplay.pstScreen->fWidth);
      stPayload.stVideoMode.u32PreviousHeight       = orxF2U(sstDisplay.pstScreen->fHeight);
      stPayload.stVideoMode.u32PreviousDepth        = sstDisplay.pstScreen->u32Depth;
      stPayload.stVideoMode.u32PreviousRefreshRate  = 60;
      stPayload.stVideoMode.bFullScreen             = _pstVideoMode->bFullScreen;

      orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "surface changed (%dx%d)->(%dx%d)",
                   stPayload.stVideoMode.u32PreviousWidth,
                   stPayload.stVideoMode.u32PreviousHeight,
                   stPayload.stVideoMode.u32Width,
                   stPayload.stVideoMode.u32Height);

      /* Stores screen info */
      sstDisplay.pstScreen->fWidth          = orxS2F(iWidth);
      sstDisplay.pstScreen->fHeight         = orxS2F(iHeight);
      sstDisplay.pstScreen->u32RealWidth    = (orxU32)iWidth;
      sstDisplay.pstScreen->u32RealHeight   = (orxU32)iHeight;
      sstDisplay.pstScreen->u32Depth        = (orxU32)iDepth;
      sstDisplay.pstScreen->bSmoothing      = sstDisplay.bDefaultSmoothing;
      sstDisplay.pstScreen->fRecRealWidth   = orxFLOAT_1 / orxU2F(sstDisplay.pstScreen->u32RealWidth);
      sstDisplay.pstScreen->fRecRealHeight  = orxFLOAT_1 / orxU2F(sstDisplay.pstScreen->u32RealHeight);
      sstDisplay.pstScreen->u32DataSize     = sstDisplay.pstScreen->u32RealWidth * sstDisplay.pstScreen->u32RealHeight * 4 * sizeof(orxU8);

      /* Updates bound texture */
      sstDisplay.apstBoundBitmapList[sstDisplay.s32ActiveTextureUnit] = orxNULL;

      /* Clears destination bitmap */
      sstDisplay.apstDestinationBitmapList[0] = orxNULL;
      sstDisplay.u32DestinationBitmapCount    = 1;

      /* Clears new display surface */
      glScissor(0, 0, (GLsizei)sstDisplay.pstScreen->u32RealWidth, (GLsizei)sstDisplay.pstScreen->u32RealHeight);
      glASSERT();
      glClearColor(orxCOLOR_NORMALIZER * orxU2F(orxRGBA_R(sstDisplay.stLastColor)), orxCOLOR_NORMALIZER * orxU2F(orxRGBA_G(sstDisplay.stLastColor)), orxCOLOR_NORMALIZER * orxU2F(orxRGBA_B(sstDisplay.stLastColor)), orxCOLOR_NORMALIZER * orxU2F(orxRGBA_A(sstDisplay.stLastColor)));
      glASSERT();
      glClear(GL_COLOR_BUFFER_BIT);
      glASSERT();

      /* Stores clipping values */
      sstDisplay.u32LastClipX       = 0;
      sstDisplay.u32LastClipY       = 0;
      sstDisplay.u32LastClipWidth   = sstDisplay.pstScreen->u32RealWidth;
      sstDisplay.u32LastClipHeight  = sstDisplay.pstScreen->u32RealHeight;

      /* Stores screen depth & refresh rate */
      sstDisplay.u32Depth       = (orxU32)iDepth;

      /* Sends event */
      orxEVENT_SEND(orxEVENT_TYPE_DISPLAY, orxDISPLAY_EVENT_SET_VIDEO_MODE, orxNULL, orxNULL, &stPayload);
    }
  }

  /* Binds buffers */
  glBindBuffer(GL_ARRAY_BUFFER, sstDisplay.uiVertexBuffer);
  glASSERT();

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sstDisplay.uiIndexBuffer);
  glASSERT();

  /* Enables vextex attribute arrays */
  glEnableVertexAttribArray(orxDISPLAY_ATTRIBUTE_LOCATION_VERTEX);
  glASSERT();
  glEnableVertexAttribArray(orxDISPLAY_ATTRIBUTE_LOCATION_TEXCOORD);
  glASSERT();
  glEnableVertexAttribArray(orxDISPLAY_ATTRIBUTE_LOCATION_COLOR);
  glASSERT();

  /* Sets vextex attribute arrays */
  glVertexAttribPointer(orxDISPLAY_ATTRIBUTE_LOCATION_VERTEX, 2, GL_FLOAT, GL_FALSE, sizeof(orxDISPLAY_VERTEX), (GLvoid *)offsetof(orxDISPLAY_ANDROID_VERTEX, fX));
  glASSERT();
  glVertexAttribPointer(orxDISPLAY_ATTRIBUTE_LOCATION_TEXCOORD, 2, GL_FLOAT, GL_FALSE, sizeof(orxDISPLAY_VERTEX), (GLvoid *)offsetof(orxDISPLAY_ANDROID_VERTEX, fU));
  glASSERT();
  glVertexAttribPointer(orxDISPLAY_ATTRIBUTE_LOCATION_COLOR, 4, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(orxDISPLAY_VERTEX), (GLvoid *)offsetof(orxDISPLAY_ANDROID_VERTEX, stRGBA));
  glASSERT();

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
    glClearDepthf(1.0f);
    glASSERT();
    glClear(GL_DEPTH_BUFFER_BIT);
    glASSERT();
  }
  else
  {
    glDisable(GL_DEPTH_TEST);
    glASSERT();
  }

  /* Common init */
  glEnable(GL_SCISSOR_TEST);
  glASSERT();
  glDisable(GL_CULL_FACE);
  glASSERT();
  glDisable(GL_STENCIL_TEST);
  glASSERT();

  /* Updates active texture unit */
  sstDisplay.s32ActiveTextureUnit = 0;

  /* Selects first texture unit */
  glActiveTexture(GL_TEXTURE0);
  glASSERT();

  /* Uses default shader */
  orxDisplay_Android_StopShader(orxNULL);

  /* Inits matrices */
  sstDisplay.fLastOrthoRight  = (sstDisplay.apstDestinationBitmapList[0] != orxNULL) ? sstDisplay.apstDestinationBitmapList[0]->fWidth : sstDisplay.pstScreen->fWidth;
  sstDisplay.fLastOrthoBottom = (sstDisplay.apstDestinationBitmapList[0] != orxNULL)
                                ? (sstDisplay.apstDestinationBitmapList[0] == sstDisplay.pstScreen)
                                  ? sstDisplay.apstDestinationBitmapList[0]->fHeight
                                  : -sstDisplay.apstDestinationBitmapList[0]->fHeight
                                : sstDisplay.pstScreen->fHeight;

  (sstDisplay.fLastOrthoBottom >= 0.0)
  ? orxDisplay_Android_OrthoProjMatrix(&(sstDisplay.mProjectionMatrix), orxFLOAT_0, sstDisplay.fLastOrthoRight, sstDisplay.fLastOrthoBottom, orxFLOAT_0, -orxFLOAT_1, orxFLOAT_1)
  : orxDisplay_Android_OrthoProjMatrix(&(sstDisplay.mProjectionMatrix), orxFLOAT_0, sstDisplay.fLastOrthoRight, orxFLOAT_0, -sstDisplay.fLastOrthoBottom, -orxFLOAT_1, orxFLOAT_1);

  /* Passes it to shader */
  glUNIFORM(Matrix4fv, sstDisplay.pstDefaultShader->iProjectionMatrixLocation, 1, GL_FALSE, (GLfloat *)&(sstDisplay.mProjectionMatrix.aafValueList[0][0]));

  /* For all texture units */
  for(i = 0; i < (orxU32)sstDisplay.iTextureUnitNumber; i++)
  {
    /* Clears its bound bitmap */
    sstDisplay.apstBoundBitmapList[i] = orxNULL;

    /* Clears its MRU timestamp */
    sstDisplay.adMRUBitmapList[i] = orxDOUBLE_0;
  }

  /* Clears last modes */
  sstDisplay.eLastBlendMode = orxDISPLAY_BLEND_MODE_NUMBER;
  sstDisplay.eLastBufferMode= orxDISPLAY_BUFFER_MODE_NUMBER;

  /* Resets primitive */
  sstDisplay.ePrimitive     = orxDISPLAY_KE_DEFAULT_PRIMITIVE;

  /* Done! */
  return eResult;
}

orxBOOL orxFASTCALL orxDisplay_Android_IsVideoModeAvailable(const orxDISPLAY_VIDEO_MODE *_pstVideoMode)
{
  orxBOOL bResult = orxTRUE;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Not available */
  orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Not available on this platform!");

  /* Done! */
  return bResult;
}

static orxSTATUS orxFASTCALL orxDisplay_Android_EventHandler(const orxEVENT *_pstEvent)
{
  /* Render stop? */
  if(_pstEvent->eType == orxEVENT_TYPE_RENDER && _pstEvent->eID == orxRENDER_EVENT_STOP)
  {
    /* Draws remaining items */
    orxDisplay_Android_DrawArrays();
  }

  if(_pstEvent->eType == orxANDROID_EVENT_TYPE_SURFACE && _pstEvent->eID == orxANDROID_EVENT_SURFACE_DESTROYED)
  {
    orxAndroid_Display_DestroySurface();
  }

  if(_pstEvent->eType == orxANDROID_EVENT_TYPE_SURFACE && _pstEvent->eID == orxANDROID_EVENT_SURFACE_CREATED)
  {
    orxAndroid_Display_CreateSurface();
  }

  if(_pstEvent->eType == orxANDROID_EVENT_TYPE_SURFACE && _pstEvent->eID == orxANDROID_EVENT_SURFACE_CHANGED)
  {
    orxANDROID_SURFACE_CHANGED_EVENT  *pstSurfaceChangedEvent;

    /* Gets payload */
    pstSurfaceChangedEvent = (orxANDROID_SURFACE_CHANGED_EVENT *)_pstEvent->pstPayload;

    /* Valid? */
    if((pstSurfaceChangedEvent->u32Width > 0) && (pstSurfaceChangedEvent->u32Height > 0))
    {
      orxDISPLAY_VIDEO_MODE stVideoMode;

      /* Inits video mode */
      stVideoMode.u32Width        = pstSurfaceChangedEvent->u32Width;
      stVideoMode.u32Height       = pstSurfaceChangedEvent->u32Height;
      stVideoMode.u32Depth        = sstDisplay.u32Depth;
      stVideoMode.u32RefreshRate  = 60;
      stVideoMode.bFullScreen     = orxTRUE;

      /* Applies it */
      orxDisplay_Android_SetVideoMode(&stVideoMode);
    }
  }

  /* Done! */
  return orxSTATUS_SUCCESS;
}


/*
 * init android display
 */
orxSTATUS orxFASTCALL orxDisplay_Android_Init()
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Was not already initialized? */
  if(!(sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY))
  {
    orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "orxDisplay_Android_Init()");

    /* Cleans static controller */
    orxMemory_Zero(&sstDisplay, sizeof(orxDISPLAY_STATIC));

    sstDisplay.surface = EGL_NO_SURFACE;
    sstDisplay.context = EGL_NO_CONTEXT;
    sstDisplay.display = EGL_NO_DISPLAY;
    sstDisplay.config = orxNULL;

    orxU32 i;
    GLushort u16Index;

    /* Stores stbi callbacks */
    sstDisplay.stSTBICallbacks.read = orxDisplay_Android_ReadSTBICallback;
    sstDisplay.stSTBICallbacks.skip = orxDisplay_Android_SkipSTBICallback;
    sstDisplay.stSTBICallbacks.eof  = orxDisplay_Android_EOFSTBICallback;

    /* For all indices */
    for (i = 0, u16Index = 0; i < orxDISPLAY_KU32_INDEX_BUFFER_SIZE; i += 6, u16Index += 4)
    {
      /* Computes them */
      sstDisplay.au16IndexList[i] = u16Index;
      sstDisplay.au16IndexList[i + 1] = u16Index;
      sstDisplay.au16IndexList[i + 2] = u16Index + 1;
      sstDisplay.au16IndexList[i + 3] = u16Index + 2;
      sstDisplay.au16IndexList[i + 4] = u16Index + 3;
      sstDisplay.au16IndexList[i + 5] = u16Index + 3;
    }

    /* Creates banks */
    sstDisplay.pstBitmapBank = orxBank_Create(orxDISPLAY_KU32_BITMAP_BANK_SIZE, sizeof(orxBITMAP), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);
    sstDisplay.pstShaderBank  = orxBank_Create(orxDISPLAY_KU32_SHADER_BANK_SIZE, sizeof(orxDISPLAY_SHADER), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

    /* Valid? */
    if(sstDisplay.pstBitmapBank != orxNULL
    && (sstDisplay.pstShaderBank != orxNULL))
    {
        orxDISPLAY_EVENT_PAYLOAD stPayload;
        const orxSTRING zGlRenderer;
        const orxSTRING zGlVersion;
        int32_t width, height;

        /* Pushes display section */
        orxConfig_PushSection(orxDISPLAY_KZ_CONFIG_SECTION);

        /* Depth buffer? */
        if(orxConfig_GetBool(orxDISPLAY_KZ_CONFIG_DEPTHBUFFER) != orxFALSE)
        {
          /* Updates flags */
           sstDisplay.u32Flags = orxDISPLAY_KU32_STATIC_FLAG_DEPTHBUFFER;
        }
        else
        {
          sstDisplay.u32Flags = orxDISPLAY_KU32_STATIC_FLAG_NONE;
        }

        sstDisplay.u32Depth = orxConfig_HasValue(orxDISPLAY_KZ_CONFIG_DEPTH) ? orxConfig_GetU32(orxDISPLAY_KZ_CONFIG_DEPTH) : 24;

        // Create OpenGL ES Context
        orxAndroid_Display_CreateContext();

        // Create OpenGL ES Surface
        if(orxAndroid_Display_CreateSurface() == orxSTATUS_FAILURE)
        {
          orxConfig_PopSection();
          return orxSTATUS_FAILURE;
        }

        eglQuerySurface(sstDisplay.display, sstDisplay.surface, EGL_WIDTH, &width);
        eglASSERT();
        eglQuerySurface(sstDisplay.display, sstDisplay.surface, EGL_HEIGHT, &height);
        eglASSERT();

        zGlRenderer = (const orxSTRING) glGetString(GL_RENDERER);
        glASSERT();
        zGlVersion = (const orxSTRING) glGetString(GL_VERSION);
        glASSERT();

        orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Renderer: %s, Version: %s", zGlRenderer, zGlVersion);

        /* Adds event handler */
        orxEvent_AddHandler(orxEVENT_TYPE_RENDER, orxDisplay_Android_EventHandler);
        orxEvent_AddHandler(orxANDROID_EVENT_TYPE_SURFACE, orxDisplay_Android_EventHandler);
        orxEvent_SetHandlerIDFlags(orxDisplay_Android_EventHandler, orxEVENT_TYPE_RENDER, orxNULL, orxEVENT_GET_FLAG(orxRENDER_EVENT_STOP), orxEVENT_KU32_MASK_ID_ALL);

        /* Inits default values */
        sstDisplay.bDefaultSmoothing  = orxConfig_GetBool(orxDISPLAY_KZ_CONFIG_SMOOTH);
        sstDisplay.eLastBlendMode     = orxDISPLAY_BLEND_MODE_NUMBER;
        sstDisplay.eLastBufferMode    = orxDISPLAY_BUFFER_MODE_NUMBER;
        sstDisplay.ePrimitive         = orxDISPLAY_KE_DEFAULT_PRIMITIVE;

        /* Allocates screen bitmap */
        sstDisplay.pstScreen = (orxBITMAP *) orxBank_Allocate(sstDisplay.pstBitmapBank);
        orxMemory_Zero(sstDisplay.pstScreen, sizeof(orxBITMAP));

        sstDisplay.pstScreen->fWidth = orxU2F(width);
        sstDisplay.pstScreen->fHeight = orxU2F(height);
        sstDisplay.pstScreen->u32RealWidth = orxF2U(sstDisplay.pstScreen->fWidth);
        sstDisplay.pstScreen->u32RealHeight = orxF2U(sstDisplay.pstScreen->fHeight);
        sstDisplay.pstScreen->fRecRealWidth = orxFLOAT_1 / orxU2F(sstDisplay.pstScreen->u32RealWidth);
        sstDisplay.pstScreen->fRecRealHeight = orxFLOAT_1 / orxU2F(sstDisplay.pstScreen->u32RealHeight);
        sstDisplay.pstScreen->u32DataSize    = sstDisplay.pstScreen->u32RealWidth * sstDisplay.pstScreen->u32RealHeight * 4 * sizeof(orxU8);
        orxVector_Copy(&(sstDisplay.pstScreen->stClip.vTL), &orxVECTOR_0);
        orxVector_Set(&(sstDisplay.pstScreen->stClip.vBR), sstDisplay.pstScreen->fWidth, sstDisplay.pstScreen->fHeight, orxFLOAT_0);

        glGenFramebuffers(1, &sstDisplay.uiFrameBuffer);
        glASSERT();

        /* Updates config info */
        orxConfig_SetU32(orxDISPLAY_KZ_CONFIG_DEPTH, sstDisplay.u32Depth);

        /* Pops config section */
        orxConfig_PopSection();

        /* Gets max texture unit number */
        glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &(sstDisplay.iTextureUnitNumber));
        glASSERT();
        sstDisplay.iTextureUnitNumber = orxMIN(sstDisplay.iTextureUnitNumber, orxDISPLAY_KU32_MAX_TEXTURE_UNIT_NUMBER);

        if(orxString_SearchString(zGlVersion, "OpenGL ES 3.") && gl3stubInit())
        {
          orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "OpenGL ES 3 inited!");
          glGetIntegerv(GL_MAX_DRAW_BUFFERS, &sstDisplay.iDrawBufferNumber);
          glASSERT();
          orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "GL_MAX_DRAW_BUFFERS = %d", sstDisplay.iDrawBufferNumber);
          sstDisplay.iDrawBufferNumber = orxMIN(sstDisplay.iDrawBufferNumber, orxDISPLAY_KU32_MAX_TEXTURE_UNIT_NUMBER);
        }
        else
        {
          sstDisplay.iDrawBufferNumber = 1;
        }

        /* Fills the list of draw buffer symbols */
        for(i = 0; i < (orxU32)sstDisplay.iDrawBufferNumber; i++)
        {
          sstDisplay.aeDrawBufferList[i] = GL_COLOR_ATTACHMENT0 + i;
        }

        /* hack for old Adreno drivers */
        if(orxString_SearchString(zGlRenderer, "Adreno") && orxString_SearchString(zGlVersion, "OpenGL ES 2.0"))
        {
          orxU32 u32Version;
          orxString_ToU32(zGlVersion + orxString_GetLength("OpenGL ES 2.0"), &u32Version, orxNULL);
          if(u32Version > 0 && u32Version < 1849878)
          {
            orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Bugged Andreno GPU driver found!, enabling workaround");
            sstDisplay.iTextureUnitNumber = 1;
          }
        }

        /* Pushes config section */
        orxConfig_PushSection(orxDISPLAY_KZ_CONFIG_SECTION);

        /* Stores texture unit and draw buffer numbers */
        orxConfig_SetU32("TextureUnitNumber", (orxU32)sstDisplay.iTextureUnitNumber);
        orxConfig_SetU32("DrawBufferNumber", (orxU32)sstDisplay.iDrawBufferNumber);

        /* Pops config section */
        orxConfig_PopSection();

        static const orxSTRING szFragmentShaderSource =
        "precision mediump float;"
        "varying vec2 _gl_TexCoord0_;"
        "varying vec4 _Color0_;"
        "uniform sampler2D _Texture_;"
        "void main()"
        "{"
        "  gl_FragColor = _Color0_.rgba * texture2D(_Texture_, _gl_TexCoord0_).rgba;"
        "}";
        static const orxSTRING szNoTextureFragmentShaderSource =
        "precision mediump float;"
        "varying vec2 _gl_TexCoord0_;"
        "varying vec4 _Color0_;"
        "uniform sampler2D _Texture_;"
        "void main()"
        "{"
        "  gl_FragColor = _Color0_;"
        "}";

        /* Inits flags */
        orxFLAG_SET(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_SHADER | orxDISPLAY_KU32_STATIC_FLAG_READY, orxDISPLAY_KU32_STATIC_FLAG_NONE);

        /* Clears destination bitmap */
        sstDisplay.apstDestinationBitmapList[0] = orxNULL;
        sstDisplay.u32DestinationBitmapCount    = 1;

        /* Updates bound texture */
        sstDisplay.apstBoundBitmapList[sstDisplay.s32ActiveTextureUnit] = orxNULL;

        /* Creates default shaders */
        sstDisplay.pstDefaultShader   = (orxDISPLAY_SHADER*) orxDisplay_CreateShader(&szFragmentShaderSource, 1, orxNULL, orxFALSE);
        sstDisplay.pstNoTextureShader = (orxDISPLAY_SHADER*) orxDisplay_CreateShader(&szNoTextureFragmentShaderSource, 1, orxNULL, orxTRUE);

        /* Generates index buffer object (VBO/IBO) */
        glGenBuffers(1, &(sstDisplay.uiVertexBuffer));
        glASSERT();
        glGenBuffers(1, &(sstDisplay.uiIndexBuffer));
        glASSERT();

        /* Binds them */
        glBindBuffer(GL_ARRAY_BUFFER, sstDisplay.uiVertexBuffer);
        glASSERT();

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sstDisplay.uiIndexBuffer);
        glASSERT();

        /* Inits VBO */
        glBufferData(GL_ARRAY_BUFFER, orxDISPLAY_KU32_VERTEX_BUFFER_SIZE * sizeof(orxDISPLAY_ANDROID_VERTEX), NULL, GL_DYNAMIC_DRAW);
        glASSERT();

        /* Fills IBO */
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, orxDISPLAY_KU32_INDEX_BUFFER_SIZE * sizeof(GLushort), sstDisplay.au16IndexList, GL_STATIC_DRAW);
        glASSERT();

        /* Set up OpenGL state */
        orxDisplay_Android_SetVideoMode(orxNULL);

        /* Inits event payload */
        orxMemory_Zero(&stPayload, sizeof(orxDISPLAY_EVENT_PAYLOAD));
        stPayload.stVideoMode.u32Width    = orxF2U(sstDisplay.pstScreen->fWidth);
        stPayload.stVideoMode.u32Height   = orxF2U(sstDisplay.pstScreen->fHeight);
        stPayload.stVideoMode.u32Depth    = sstDisplay.u32Depth;
        stPayload.stVideoMode.bFullScreen = orxTRUE;

        /* Sends it */
        orxEVENT_SEND(orxEVENT_TYPE_DISPLAY, orxDISPLAY_EVENT_SET_VIDEO_MODE, orxNULL, orxNULL, &stPayload);
    }
    else
    {
      /* Deletes banks */
      orxBank_Delete(sstDisplay.pstBitmapBank);
      sstDisplay.pstBitmapBank = orxNULL;
      orxBank_Delete(sstDisplay.pstShaderBank);
      sstDisplay.pstShaderBank = orxNULL;

      /* Updates result */
      eResult = orxSTATUS_FAILURE;
    }
  }
  else
  {
    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

void orxFASTCALL orxDisplay_Android_Exit()
{
  if(sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY)
  {
    /* Removes event handler */
    orxEvent_RemoveHandler(orxEVENT_TYPE_RENDER, orxDisplay_Android_EventHandler);
    orxEvent_RemoveHandler(orxANDROID_EVENT_TYPE_SURFACE, orxDisplay_Android_EventHandler);

    /* Deletes default shaders */
    orxDisplay_DeleteShader(sstDisplay.pstDefaultShader);
    orxDisplay_DeleteShader(sstDisplay.pstNoTextureShader);

    sstDisplay.u32Flags = orxDISPLAY_KU32_STATIC_FLAG_NONE;

    glDeleteFramebuffers(1, &sstDisplay.uiFrameBuffer);
    glASSERT();

    /* Has index buffer? */
    if(sstDisplay.uiIndexBuffer != 0)
    {
      /* Deletes it */
      glDeleteBuffers(1, &(sstDisplay.uiIndexBuffer));
      glASSERT();
      sstDisplay.uiIndexBuffer = 0;
    }

    /* Deletes banks */
    orxBank_Delete(sstDisplay.pstBitmapBank);
    orxBank_Delete(sstDisplay.pstShaderBank);

    if (sstDisplay.display != EGL_NO_DISPLAY)
    {
      eglMakeCurrent(sstDisplay.display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
      eglASSERT();

      if (sstDisplay.context != EGL_NO_CONTEXT)
      {
        eglDestroyContext(sstDisplay.display, sstDisplay.context);
        eglASSERT();

        sstDisplay.context = EGL_NO_CONTEXT;
      }
      if (sstDisplay.surface != EGL_NO_SURFACE)
      {
        eglDestroySurface(sstDisplay.display, sstDisplay.surface);
        eglASSERT();

        sstDisplay.surface = EGL_NO_SURFACE;
      }
      eglTerminate(sstDisplay.display);
      eglASSERT();

      sstDisplay.display = EGL_NO_DISPLAY;
    }

    /* Cleans static controller */
    orxMemory_Zero(&sstDisplay, sizeof(orxDISPLAY_STATIC));
  }

  /* Done! */
  return;
}

orxBOOL orxFASTCALL orxDisplay_Android_HasShaderSupport()
{
  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Done! */
  return orxTRUE;
}

orxHANDLE orxFASTCALL orxDisplay_Android_CreateShader(const orxSTRING *_azCodeList, orxU32 _u32Size, const orxLINKLIST *_pstParamList, orxBOOL _bUseCustomParam)
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
        orxS32    s32Free, s32Offset;
        orxU32    i;

        /* Inits shader code buffer */
        sstDisplay.acShaderCodeBuffer[0]  = sstDisplay.acShaderCodeBuffer[orxDISPLAY_KU32_SHADER_BUFFER_SIZE - 1] = orxCHAR_NULL;
        pc                                = sstDisplay.acShaderCodeBuffer;
        s32Free                           = orxDISPLAY_KU32_SHADER_BUFFER_SIZE - 1;

        /* Has parameters? */
        if(_pstParamList != orxNULL)
        {
          orxSHADER_PARAM *pstParam;

          /* Adds wrapping code */
          s32Offset = orxString_NPrint(pc, s32Free, "precision mediump float;\nvarying vec2 _gl_TexCoord0_;\nvarying vec4 _Color0_;\n");
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
                s32Offset = (pstParam->u32ArraySize >= 1) ? orxString_NPrint(pc, s32Free, "uniform float %s[%ld];\n", pstParam->zName, pstParam->u32ArraySize) : orxString_NPrint(pc, s32Free, "uniform float %s;\n", pstParam->zName);
                pc       += s32Offset;
                s32Free  -= s32Offset;

                break;
              }

              case orxSHADER_PARAM_TYPE_TEXTURE:
              {
                /* Adds its literal value and automated coordinates */
                s32Offset = (pstParam->u32ArraySize >= 1) ? orxString_NPrint(pc, s32Free, "uniform sampler2D %s[%ld];\nuniform float %s" orxDISPLAY_KZ_SHADER_SUFFIX_TOP "[%ld];\nuniform float %s" orxDISPLAY_KZ_SHADER_SUFFIX_LEFT "[%ld];\nuniform float %s" orxDISPLAY_KZ_SHADER_SUFFIX_BOTTOM "[%ld];\nuniform float %s" orxDISPLAY_KZ_SHADER_SUFFIX_RIGHT "[%ld];\n", pstParam->zName, pstParam->u32ArraySize, pstParam->zName, pstParam->u32ArraySize, pstParam->zName, pstParam->u32ArraySize, pstParam->zName, pstParam->u32ArraySize, pstParam->zName, pstParam->u32ArraySize) : orxString_NPrint(pc, s32Free, "uniform sampler2D %s;\nuniform float %s" orxDISPLAY_KZ_SHADER_SUFFIX_TOP ";\nuniform float %s" orxDISPLAY_KZ_SHADER_SUFFIX_LEFT ";\nuniform float %s" orxDISPLAY_KZ_SHADER_SUFFIX_BOTTOM ";\nuniform float %s" orxDISPLAY_KZ_SHADER_SUFFIX_RIGHT ";\n", pstParam->zName, pstParam->zName, pstParam->zName, pstParam->zName, pstParam->zName);
                pc       += s32Offset;
                s32Free  -= s32Offset;

                break;
              }

              case orxSHADER_PARAM_TYPE_VECTOR:
              {
                /* Adds its literal value */
                s32Offset = (pstParam->u32ArraySize >= 1) ? orxString_NPrint(pc, s32Free, "uniform vec3 %s[%ld];\n", pstParam->zName, pstParam->u32ArraySize) : orxString_NPrint(pc, s32Free, "uniform vec3 %s;\n", pstParam->zName);
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
        pstShader->uiProgram              = (GLuint)orxU32_UNDEFINED;
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
        if(orxDisplay_Android_CompileShader(pstShader) != orxSTATUS_FAILURE)
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

void orxFASTCALL orxDisplay_Android_DeleteShader(orxHANDLE _hShader)
{
  orxDISPLAY_SHADER *pstShader;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT((_hShader != orxHANDLE_UNDEFINED) && (_hShader != orxNULL));

  /* Gets shader */
  pstShader = (orxDISPLAY_SHADER *)_hShader;

  /* Deletes its program */
  glDeleteProgram(pstShader->uiProgram);
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

orxSTATUS orxFASTCALL orxDisplay_Android_StartShader(orxHANDLE _hShader)
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
    /* Draws remaining items */
    orxDisplay_Android_DrawArrays();

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
  glUseProgram(pstShader->uiProgram);
  glASSERT();

  /* Updates projection matrix */
  glUNIFORM(Matrix4fv, pstShader->iProjectionMatrixLocation, 1, GL_FALSE, (GLfloat *)&(sstDisplay.mProjectionMatrix.aafValueList[0][0]));

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_Android_StopShader(orxHANDLE _hShader)
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

        /* Clears its texture info list */
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
          orxDisplay_Android_DrawArrays();

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

      /* Updates projection matrix */
      glUNIFORM(Matrix4fv, sstDisplay.pstDefaultShader->iProjectionMatrixLocation, 1, GL_FALSE, (GLfloat *)&(sstDisplay.mProjectionMatrix.aafValueList[0][0]));
    }
    else
    {
      /* Don't reset shader */
      bResetShader = orxFALSE;
    }
  }

  /* Should reset shader? */
  if(bResetShader != orxFALSE)
  {
    /* Uses default program */
    glUseProgram(sstDisplay.pstDefaultShader->uiProgram);
    glASSERT();

    /* Updates its texture unit */
    glUNIFORM(1i, sstDisplay.pstDefaultShader->iTextureLocation, sstDisplay.s32ActiveTextureUnit);

    /* Updates projection matrix */
    glUNIFORM(Matrix4fv, sstDisplay.pstDefaultShader->iProjectionMatrixLocation, 1, GL_FALSE, (GLfloat *)&(sstDisplay.mProjectionMatrix.aafValueList[0][0]));
  }

  /* Done! */
  return eResult;
}


orxS32 orxFASTCALL orxDisplay_Android_GetParameterID(const orxHANDLE _hShader, const orxSTRING _zParam, orxS32 _s32Index, orxBOOL _bIsTexture)
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
      orxString_NPrint(acBuffer, sizeof(acBuffer) - 1, "%s[%ld]", _zParam, _s32Index);

      /* Gets parameter location */
      pstInfo->iLocation = glGetUniformLocation(pstShader->uiProgram, acBuffer);
      glASSERT();

      /* Gets top parameter location */
      orxString_NPrint(acBuffer, sizeof(acBuffer) - 1, "%s" orxDISPLAY_KZ_SHADER_SUFFIX_TOP "%[ld]", _zParam, _s32Index);
      pstInfo->iLocationTop = glGetUniformLocation(pstShader->uiProgram, (const char *)acBuffer);
      glASSERT();

      /* Gets left parameter location */
      orxString_NPrint(acBuffer, sizeof(acBuffer) - 1, "%s" orxDISPLAY_KZ_SHADER_SUFFIX_LEFT "%[ld]", _zParam, _s32Index);
      pstInfo->iLocationLeft = glGetUniformLocation(pstShader->uiProgram, (const char *)acBuffer);
      glASSERT();

      /* Gets bottom parameter location */
      orxString_NPrint(acBuffer, sizeof(acBuffer) - 1, "%s" orxDISPLAY_KZ_SHADER_SUFFIX_BOTTOM "%[ld]", _zParam, _s32Index);
      pstInfo->iLocationBottom = glGetUniformLocation(pstShader->uiProgram, (const char *)acBuffer);
      glASSERT();

      /* Gets right parameter location */
      orxString_NPrint(acBuffer, sizeof(acBuffer) - 1, "%s" orxDISPLAY_KZ_SHADER_SUFFIX_RIGHT "%[ld]", _zParam, _s32Index);
      pstInfo->iLocationRight = glGetUniformLocation(pstShader->uiProgram, (const char *)acBuffer);
      glASSERT();
    }
    else
    {
      /* Gets parameter location */
      pstInfo->iLocation = glGetUniformLocation(pstShader->uiProgram, (const char *)_zParam);
      glASSERT();

      /* Gets top parameter location */
      orxString_NPrint(acBuffer, sizeof(acBuffer) - 1, "%s" orxDISPLAY_KZ_SHADER_SUFFIX_TOP, _zParam);
      pstInfo->iLocationTop = glGetUniformLocation(pstShader->uiProgram, (const char *)acBuffer);
      glASSERT();

      /* Gets left parameter location */
      orxString_NPrint(acBuffer, sizeof(acBuffer) - 1, "%s" orxDISPLAY_KZ_SHADER_SUFFIX_LEFT, _zParam);
      pstInfo->iLocationLeft = glGetUniformLocation(pstShader->uiProgram, (const char *)acBuffer);
      glASSERT();

      /* Gets bottom parameter location */
      orxString_NPrint(acBuffer, sizeof(acBuffer) - 1, "%s" orxDISPLAY_KZ_SHADER_SUFFIX_BOTTOM, _zParam);
      pstInfo->iLocationBottom = glGetUniformLocation(pstShader->uiProgram, (const char *)acBuffer);
      glASSERT();

      /* Gets right parameter location */
      orxString_NPrint(acBuffer, sizeof(acBuffer) - 1, "%s" orxDISPLAY_KZ_SHADER_SUFFIX_RIGHT, _zParam);
      pstInfo->iLocationRight = glGetUniformLocation(pstShader->uiProgram, (const char *)acBuffer);
      glASSERT();
    }
  }
  else
  {
    /* Array? */
    if(_s32Index >= 0)
    {
      orxCHAR acBuffer[256];

      /* Prints its name */
      orxString_NPrint(acBuffer, sizeof(acBuffer) - 1, "%s[%ld]", _zParam, _s32Index);
      acBuffer[sizeof(acBuffer) - 1] = orxCHAR_NULL;

      /* Gets parameter location */
      s32Result = (orxS32)glGetUniformLocation(pstShader->uiProgram, acBuffer);
      glASSERT();
    }
    else
    {
      /* Gets parameter location */
      s32Result = (orxS32)glGetUniformLocation(pstShader->uiProgram, (const char *)_zParam);
      glASSERT();
    }
  }

  /* Done! */
  return s32Result;
}

orxSTATUS orxFASTCALL orxDisplay_Android_SetShaderBitmap(orxHANDLE _hShader, orxS32 _s32ID, const orxBITMAP *_pstValue)
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
        orxDisplay_Android_DrawArrays();

        /* Updates texture info */
        pstShader->astTextureInfoList[i].pstBitmap = _pstValue;

        /* Updates corner values */
        glUNIFORM(1f, pstShader->astParamInfoList[_s32ID].iLocationTop,  (GLfloat)((_pstValue->fRecRealHeight * _pstValue->stClip.vTL.fY)));
        glUNIFORM(1f, pstShader->astParamInfoList[_s32ID].iLocationLeft, (GLfloat)(_pstValue->fRecRealWidth * _pstValue->stClip.vTL.fX));
        glUNIFORM(1f, pstShader->astParamInfoList[_s32ID].iLocationBottom, (GLfloat)((_pstValue->fRecRealHeight * _pstValue->stClip.vBR.fY)));
        glUNIFORM(1f, pstShader->astParamInfoList[_s32ID].iLocationRight, (GLfloat)(_pstValue->fRecRealWidth * _pstValue->stClip.vBR.fX));
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
        /* No bitmap? */
        if(_pstValue == orxNULL)
        {
          /* Uses screen bitmap */
          _pstValue = sstDisplay.pstScreen;
        }

        /* Updates texture info */
        pstShader->astTextureInfoList[pstShader->iTextureCount].iLocation = pstShader->astParamInfoList[_s32ID].iLocation;
        pstShader->astTextureInfoList[pstShader->iTextureCount].pstBitmap = _pstValue;

        /* Updates corner values */
        glUNIFORM(1f, pstShader->astParamInfoList[_s32ID].iLocationTop, (GLfloat)((_pstValue->fRecRealHeight * _pstValue->stClip.vTL.fY)));
        glUNIFORM(1f, pstShader->astParamInfoList[_s32ID].iLocationLeft, (GLfloat)(_pstValue->fRecRealWidth * _pstValue->stClip.vTL.fX));
        glUNIFORM(1f, pstShader->astParamInfoList[_s32ID].iLocationBottom, (GLfloat)((_pstValue->fRecRealHeight * _pstValue->stClip.vBR.fY)));
        glUNIFORM(1f, pstShader->astParamInfoList[_s32ID].iLocationRight, (GLfloat)(_pstValue->fRecRealWidth * _pstValue->stClip.vBR.fX));

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

orxSTATUS orxFASTCALL orxDisplay_Android_SetShaderFloat(orxHANDLE _hShader, orxS32 _s32ID, orxFLOAT _fValue)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT((_hShader != orxHANDLE_UNDEFINED) && (_hShader != orxNULL));

  /* Valid? */
  if(_s32ID >= 0)
  {
    /* Updates its value (no glASSERT() as this can be set more than once per use and would trigger it) */
    glUNIFORM_NO_ASSERT(1f, (GLint)_s32ID, (GLfloat)_fValue);

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

orxSTATUS orxFASTCALL orxDisplay_Android_SetShaderVector(orxHANDLE _hShader, orxS32 _s32ID, const orxVECTOR *_pvValue)
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
    glUNIFORM_NO_ASSERT(3f, (GLint)_s32ID, (GLfloat)_pvValue->fX, (GLfloat)_pvValue->fY, (GLfloat)_pvValue->fZ);

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

/***************************************************************************
 * Plugin Related                                                          *
 ***************************************************************************/
orxPLUGIN_USER_CORE_FUNCTION_START(DISPLAY);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Android_Init, DISPLAY, INIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Android_Exit, DISPLAY, EXIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Android_Swap, DISPLAY, SWAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Android_CreateBitmap, DISPLAY, CREATE_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Android_DeleteBitmap, DISPLAY, DELETE_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Android_SaveBitmap, DISPLAY, SAVE_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Android_SetTempBitmap, DISPLAY, SET_TEMP_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Android_GetTempBitmap, DISPLAY, GET_TEMP_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Android_SetDestinationBitmaps, DISPLAY, SET_DESTINATION_BITMAPS);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Android_TransformBitmap, DISPLAY, TRANSFORM_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Android_TransformText, DISPLAY, TRANSFORM_TEXT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Android_LoadBitmap, DISPLAY, LOAD_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Android_GetBitmapSize, DISPLAY, GET_BITMAP_SIZE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Android_GetScreenSize, DISPLAY, GET_SCREEN_SIZE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Android_GetScreenBitmap, DISPLAY, GET_SCREEN_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Android_ClearBitmap, DISPLAY, CLEAR_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Android_SetBlendMode, DISPLAY, SET_BLEND_MODE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Android_SetBitmapClipping, DISPLAY, SET_BITMAP_CLIPPING);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Android_GetBitmapID, DISPLAY, GET_BITMAP_ID);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Android_SetBitmapData, DISPLAY, SET_BITMAP_DATA);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Android_GetBitmapData, DISPLAY, GET_BITMAP_DATA);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Android_SetBitmapColorKey, DISPLAY, SET_BITMAP_COLOR_KEY);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Android_SetBitmapColor, DISPLAY, SET_BITMAP_COLOR);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Android_GetBitmapColor, DISPLAY, GET_BITMAP_COLOR);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Android_DrawLine, DISPLAY, DRAW_LINE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Android_DrawPolyline, DISPLAY, DRAW_POLYLINE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Android_DrawPolygon, DISPLAY, DRAW_POLYGON);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Android_DrawCircle, DISPLAY, DRAW_CIRCLE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Android_DrawOBox, DISPLAY, DRAW_OBOX);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Android_DrawMesh, DISPLAY, DRAW_MESH);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Android_HasShaderSupport, DISPLAY, HAS_SHADER_SUPPORT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Android_CreateShader, DISPLAY, CREATE_SHADER);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Android_DeleteShader, DISPLAY, DELETE_SHADER);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Android_StartShader, DISPLAY, START_SHADER);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Android_StopShader, DISPLAY, STOP_SHADER);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Android_GetParameterID, DISPLAY, GET_PARAMETER_ID);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Android_SetShaderBitmap, DISPLAY, SET_SHADER_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Android_SetShaderFloat, DISPLAY, SET_SHADER_FLOAT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Android_SetShaderVector, DISPLAY, SET_SHADER_VECTOR);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Android_EnableVSync, DISPLAY, ENABLE_VSYNC);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Android_IsVSyncEnabled, DISPLAY, IS_VSYNC_ENABLED);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Android_SetFullScreen, DISPLAY, SET_FULL_SCREEN);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Android_IsFullScreen, DISPLAY, IS_FULL_SCREEN);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Android_GetVideoModeCount, DISPLAY, GET_VIDEO_MODE_COUNT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Android_GetVideoMode, DISPLAY, GET_VIDEO_MODE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Android_SetVideoMode, DISPLAY, SET_VIDEO_MODE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_Android_IsVideoModeAvailable, DISPLAY, IS_VIDEO_MODE_AVAILABLE);
orxPLUGIN_USER_CORE_FUNCTION_END();
