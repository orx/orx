/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2013 Orx-Project
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

#include "GL/glfw.h"
#include "GL/glext.h"
#include "SOIL.h"

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
#define orxDISPLAY_KU32_STATIC_FLAG_IGNORE_RESIZE 0x00000800  /**< Ignore resize event flag */
#define orxDISPLAY_KU32_STATIC_FLAG_FULLSCREEN  0x00001000  /**< Full screen flag */

#define orxDISPLAY_KU32_STATIC_MASK_ALL         0xFFFFFFFF  /**< All mask */

#define orxDISPLAY_KU32_BITMAP_BANK_SIZE        256
#define orxDISPLAY_KU32_SHADER_BANK_SIZE        64

#define orxDISPLAY_KU32_VERTEX_BUFFER_SIZE      (4 * 2048)  /**< 2048 items batch capacity */
#define orxDISPLAY_KU32_INDEX_BUFFER_SIZE       (6 * 2048)  /**< 2048 items batch capacity */
#define orxDISPLAY_KU32_SHADER_BUFFER_SIZE      65536

#define orxDISPLAY_KF_BORDER_FIX                0.1f

#define orxDISPLAY_KU32_CIRCLE_LINE_NUMBER      32


/**  Misc defines
 */
#ifdef __orxDEBUG__

#define glASSERT()                                                        \
do                                                                        \
{                                                                         \
  /* Is window still opened? */                                           \
  if(glfwGetWindowParam(GLFW_OPENED) != GL_FALSE)                         \
  {                                                                       \
    GLenum eError = glGetError();                                         \
    orxASSERT(eError == GL_NO_ERROR && "OpenGL error code: 0x%X", eError);\
  }                                                                       \
} while(orxFALSE)

#else /* __orxDEBUG__ */

#define glASSERT()

#endif /* __orxDEBUG__ */


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

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
  orxRGBA                   stColor;
};

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
  GLhandleARB               hProgram;
  GLint                     iTextureCounter;
  orxS32                    s32ParamCounter;
  orxBOOL                   bActive;
  orxBOOL                   bInitialized;
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
  orxBOOL                   bDefaultSmoothing;
  orxBITMAP                *pstScreen;
  orxBITMAP                *pstDestinationBitmap;
  const orxBITMAP          *pstLastBitmap;
  orxDISPLAY_BLEND_MODE     eLastBlendMode;
  orxS32                    s32BitmapCounter;
  orxS32                    s32ShaderCounter;
  GLint                     iTextureUnitNumber;
  GLuint                    uiFrameBuffer;
  GLuint                    uiIndexBuffer;
  orxS32                    s32ActiveShaderCounter;
  orxS32                    s32BufferIndex;
  orxU32                    u32GLFWFlags;
  orxU32                    u32Flags;
  orxU32                    u32Depth;
  orxU32                    u32RefreshRate;
  orxU32                    u32DefaultWidth;
  orxU32                    u32DefaultHeight;
  orxU32                    u32DefaultDepth;
  orxU32                    u32DefaultRefreshRate;
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
PFNGLUNIFORM1FARBPROC               glUniform1fARB              = NULL;
PFNGLUNIFORM3FARBPROC               glUniform3fARB              = NULL;
PFNGLUNIFORM1IARBPROC               glUniform1iARB              = NULL;

PFNGLGENBUFFERSARBPROC              glGenBuffersARB             = NULL;
PFNGLDELETEBUFFERSARBPROC           glDeleteBuffersARB          = NULL;
PFNGLBINDBUFFERARBPROC              glBindBufferARB             = NULL;
PFNGLBUFFERDATAARBPROC              glBufferDataARB             = NULL;

PFNGLGENFRAMEBUFFERSEXTPROC         glGenFramebuffersEXT        = NULL;
PFNGLDELETEFRAMEBUFFERSEXTPROC      glDeleteFramebuffersEXT     = NULL;
PFNGLBINDFRAMEBUFFEREXTPROC         glBindFramebufferEXT        = NULL;
PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC  glCheckFramebufferStatusEXT = NULL;
PFNGLFRAMEBUFFERTEXTURE2DEXTPROC    glFramebufferTexture2DEXT   = NULL;

  #ifndef __orxLINUX__

PFNGLACTIVETEXTUREARBPROC           glActiveTextureARB          = NULL;

  #endif /* __orxLINUX__ */

#endif /* __orxMAC__ */


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

/** Render inhibiter
 */
static orxSTATUS orxFASTCALL orxDisplay_GLFW_RenderInhibiter(const orxEVENT *_pstEvent)
{
  /* Render stop? */
  if(_pstEvent->eID == orxRENDER_EVENT_STOP)
  {
    /* Polls events */
    glfwPollEvents();
  }

  /* Done! */
  return orxSTATUS_FAILURE;
}

static void GLFWCALL orxDisplay_GLFW_ResizeCallback(int _iWidth, int _iHeight)
{
  /* Not ignoring event? */
  if(!orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_IGNORE_RESIZE))
  {
    /* Valid? */
    if((_iWidth > 0) && (_iHeight > 0))
    {
      orxDISPLAY_VIDEO_MODE stVideoMode;

      /* Inits video mode */
      stVideoMode.u32Width       = (orxU32)_iWidth;
      stVideoMode.u32Height      = (orxU32)_iHeight;
      stVideoMode.u32Depth       = sstDisplay.u32Depth;
      stVideoMode.u32RefreshRate = sstDisplay.u32RefreshRate;

      /* Applies it */
      orxDisplay_SetVideoMode(&stVideoMode);
    }
  }

  return;
}

static void orxFASTCALL orxDisplay_GLFW_Update(const orxCLOCK_INFO *_pstClockInfo, void *_pContext)
{
  /* Profiles */
  orxPROFILER_PUSH_MARKER("orxDisplay_Update");

  /* Foreground? */
  if(glfwGetWindowParam(GLFW_ICONIFIED) == GL_FALSE)
  {
    /* Wasn't in the foreground before? */
    if(orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_BACKGROUND))
    {
      /* Sends foreground event */
      if(orxEvent_SendShort(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_FOREGROUND) != orxSTATUS_FAILURE)
      {
        /* Adds render inhibiter */
        orxEvent_RemoveHandler(orxEVENT_TYPE_RENDER, orxDisplay_GLFW_RenderInhibiter);
      }

      /* Updates foreground status */
      orxFLAG_SET(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_NONE, orxDISPLAY_KU32_STATIC_FLAG_BACKGROUND);
    }
  }

  /* Has focus? */
  if(glfwGetWindowParam(GLFW_ACTIVE) != GL_FALSE)
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

  /* Background? */
  if(glfwGetWindowParam(GLFW_ICONIFIED) != GL_FALSE)
  {
    /* Wasn't in the background before? */
    if(!orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_BACKGROUND))
    {
      /* Sends background event */
      if(orxEvent_SendShort(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_BACKGROUND) != orxSTATUS_FAILURE)
      {
        /* Adds render inhibiter */
        orxEvent_AddHandler(orxEVENT_TYPE_RENDER, orxDisplay_GLFW_RenderInhibiter);
      }

      /* Updates background status */
      orxFLAG_SET(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_BACKGROUND, orxDISPLAY_KU32_STATIC_FLAG_NONE);
    }
  }

  /* Is window closed? */
  if(glfwGetWindowParam(GLFW_OPENED) == GL_FALSE)
  {
    /* Sends system close event */
    orxEvent_SendShort(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_CLOSE);
  }

  /* Profiles */
  orxPROFILER_POP_MARKER();

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
    /* Supports frame buffer? */
    if(glfwExtensionSupported("GL_EXT_framebuffer_object") != GL_FALSE)
    {
#ifndef __orxMAC__

      /* Loads frame buffer extension functions */
      orxDISPLAY_LOAD_EXTENSION_FUNCTION(PFNGLGENFRAMEBUFFERSEXTPROC, glGenFramebuffersEXT);
      orxDISPLAY_LOAD_EXTENSION_FUNCTION(PFNGLDELETEFRAMEBUFFERSEXTPROC, glDeleteFramebuffersEXT);
      orxDISPLAY_LOAD_EXTENSION_FUNCTION(PFNGLBINDFRAMEBUFFEREXTPROC, glBindFramebufferEXT);
      orxDISPLAY_LOAD_EXTENSION_FUNCTION(PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC, glCheckFramebufferStatusEXT);
      orxDISPLAY_LOAD_EXTENSION_FUNCTION(PFNGLFRAMEBUFFERTEXTURE2DEXTPROC, glFramebufferTexture2DEXT);

#endif /* __orxMAC__ */

      /* Updates status flags */
      orxFLAG_SET(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_FRAMEBUFFER, orxDISPLAY_KU32_STATIC_FLAG_NONE);
    }
    else
    {
      /* Updates status flags */
      orxFLAG_SET(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_NONE, orxDISPLAY_KU32_STATIC_FLAG_FRAMEBUFFER);
    }

    /* Has NPOT texture support? */
    if(glfwExtensionSupported("GL_ARB_texture_non_power_of_two") != GL_FALSE)
    {
      /* Updates status flags */
      orxFLAG_SET(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_NPOT, orxDISPLAY_KU32_STATIC_FLAG_NONE);
    }
    else
    {
      /* Updates status flags */
      orxFLAG_SET(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_NONE, orxDISPLAY_KU32_STATIC_FLAG_NPOT);
    }

    /* Can support vertex buffer objects? */
    if(glfwExtensionSupported("GL_ARB_vertex_buffer_object") != GL_FALSE)
    {
#ifndef __orxMAC__

      /* Loads frame buffer extension functions */
      orxDISPLAY_LOAD_EXTENSION_FUNCTION(PFNGLGENBUFFERSARBPROC, glGenBuffersARB);
      orxDISPLAY_LOAD_EXTENSION_FUNCTION(PFNGLDELETEBUFFERSARBPROC, glDeleteBuffersARB);
      orxDISPLAY_LOAD_EXTENSION_FUNCTION(PFNGLBINDBUFFERARBPROC, glBindBufferARB);
      orxDISPLAY_LOAD_EXTENSION_FUNCTION(PFNGLBUFFERDATAARBPROC, glBufferDataARB);

#endif /* __orxMAC__ */

      /* Updates status flags */
      orxFLAG_SET(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_VBO, orxDISPLAY_KU32_STATIC_FLAG_NONE);
    }
    else
    {
      /* Updates status flags */
      orxFLAG_SET(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_NONE, orxDISPLAY_KU32_STATIC_FLAG_VBO);
    }

    /* Can support shader? */
    if((glfwExtensionSupported("GL_ARB_shader_objects") != GL_FALSE)
    && (glfwExtensionSupported("GL_ARB_shading_language_100") != GL_FALSE)
    && (glfwExtensionSupported("GL_ARB_vertex_shader") != GL_FALSE)
    && (glfwExtensionSupported("GL_ARB_fragment_shader") != GL_FALSE))
    {
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
      orxDISPLAY_LOAD_EXTENSION_FUNCTION(PFNGLUNIFORM1FARBPROC, glUniform1fARB);
      orxDISPLAY_LOAD_EXTENSION_FUNCTION(PFNGLUNIFORM3FARBPROC, glUniform3fARB);
      orxDISPLAY_LOAD_EXTENSION_FUNCTION(PFNGLUNIFORM1IARBPROC, glUniform1iARB);

  #ifndef __orxLINUX__

      orxDISPLAY_LOAD_EXTENSION_FUNCTION(PFNGLACTIVETEXTUREARBPROC, glActiveTextureARB);

  #endif /* __orxLINUX__ */

#endif /* __orxMAC__ */

      /* Gets max texture unit number */
      glGetIntegerv(GL_MAX_TEXTURE_COORDS_ARB, &(sstDisplay.iTextureUnitNumber));
      glASSERT();

      /* Updates status flags */
      orxFLAG_SET(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_SHADER, orxDISPLAY_KU32_STATIC_FLAG_NONE);
    }
    else
    {
      /* Updates status flags */
      orxFLAG_SET(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_NONE, orxDISPLAY_KU32_STATIC_FLAG_SHADER);
    }

    /* Updates status flags */
    orxFLAG_SET(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_EXT_READY, orxDISPLAY_KU32_STATIC_FLAG_NONE);
  }

  /* Done! */
  return;
}

static orxSTATUS orxFASTCALL orxDisplay_GLFW_CompileShader(orxDISPLAY_SHADER *_pstShader)
{
  static const orxCHAR *szVertexShaderSource =
    "void main()"
    "{"
    "  gl_TexCoord[0] = gl_MultiTexCoord0;"
    "  gl_Position    = gl_ProjectionMatrix * gl_Vertex;"
    "}";

  GLhandleARB hProgram, hVertexShader, hFragmentShader;
  GLint       iSuccess;
  orxSTATUS   eResult = orxSTATUS_FAILURE;

  /* Creates program */
  hProgram = glCreateProgramObjectARB();
  glASSERT();

  /* Creates vertex and fragment shaders */
  hVertexShader   = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
  glASSERT();
  hFragmentShader = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
  glASSERT();

  /* Compiles shader objects */
  glShaderSourceARB(hVertexShader, 1, (const GLcharARB **)&szVertexShaderSource, NULL);
  glASSERT();
  glShaderSourceARB(hFragmentShader, 1, (const GLcharARB **)&(_pstShader->zCode), NULL);
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

      /* Links program */
      glLinkProgramARB(hProgram);
      glASSERT();

      /* Gets linking status */
      glGetObjectParameterivARB(hProgram, GL_OBJECT_LINK_STATUS_ARB, &iSuccess);
      glASSERT();

      /* Success? */
      if(iSuccess != GL_FALSE)
      {
        /* Updates shader */
        _pstShader->hProgram        = hProgram;
        _pstShader->iTextureCounter = 0;

        /* Updates result */
        eResult = orxSTATUS_SUCCESS;
      }
      else
      {
        orxCHAR acBuffer[1024];

        /* Gets log */
        glGetInfoLogARB(hProgram, 1024 * sizeof(orxCHAR), NULL, (GLcharARB *)acBuffer);
        glASSERT();

        /* Outputs log */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Couldn't link shader program:%s%s%s", orxSTRING_EOL, acBuffer, orxSTRING_EOL);

        /* Deletes program */
        glDeleteObjectARB(hProgram);
        glASSERT();
      }
    }
    else
    {
      orxCHAR acBuffer[1024];

      /* Gets log */
      glGetInfoLogARB(hFragmentShader, 1024 * sizeof(orxCHAR), NULL, (GLcharARB *)acBuffer);
      glASSERT();

      /* Outputs log */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Couldn't compile fragment shader:%s%s%s", orxSTRING_EOL, acBuffer, orxSTRING_EOL);

      /* Deletes shader objects & program */
      glDeleteObjectARB(hVertexShader);
      glASSERT();
      glDeleteObjectARB(hFragmentShader);
      glASSERT();
      glDeleteObjectARB(hProgram);
      glASSERT();
    }
  }
  else
  {
    orxCHAR acBuffer[1024];

    /* Gets log */
    glGetInfoLogARB(hVertexShader, 1024 * sizeof(orxCHAR), NULL, (GLcharARB *)acBuffer);
    glASSERT();

    /* Outputs log */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Couldn't compile vertex shader:%s%s%s", orxSTRING_EOL, acBuffer, orxSTRING_EOL);

    /* Deletes shader objects & program */
    glDeleteObjectARB(hVertexShader);
    glASSERT();
    glDeleteObjectARB(hFragmentShader);
    glASSERT();
    glDeleteObjectARB(hProgram);
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
  if(_pstShader->iTextureCounter > 0)
  {
    GLint   i;
    orxBOOL bCaptured = orxFALSE;

    /* For all defined textures */
    for(i = 0; i < _pstShader->iTextureCounter; i++)
    {
      /* Updates corresponding texture unit */
      glUniform1iARB(_pstShader->astTextureInfoList[i].iLocation, i);
      glASSERT();
      glActiveTextureARB(GL_TEXTURE0_ARB + i);
      glASSERT();
      glBindTexture(GL_TEXTURE_2D, _pstShader->astTextureInfoList[i].pstBitmap->uiTexture);
      glASSERT();

      /* Screen and not already captured? */
      if((_pstShader->astTextureInfoList[i].pstBitmap == sstDisplay.pstScreen) && (bCaptured == orxFALSE))
      {
        /* Copies screen content */
        glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, (GLint)(orxF2U(sstDisplay.pstScreen->fHeight) - sstDisplay.pstScreen->u32RealHeight), (GLsizei)orxF2U(sstDisplay.pstScreen->fWidth), (GLsizei)sstDisplay.pstScreen->u32RealHeight);
        glASSERT();

        /* Updates captured status */
        bCaptured = orxTRUE;
      }
    }
  }

  /* Updates its status */
  _pstShader->bInitialized = orxTRUE;

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
    }
    else
    {
      /* Uses client-side index list */
      pIndexContext = (GLvoid *)&(sstDisplay.au16IndexList);
    }

    /* Has active shaders? */
    if(sstDisplay.s32ActiveShaderCounter > 0)
    {
      orxDISPLAY_SHADER *pstShader;

      /* For all shaders */
      for(pstShader = (orxDISPLAY_SHADER *)orxBank_GetNext(sstDisplay.pstShaderBank, orxNULL);
          pstShader != orxNULL;
          pstShader = (orxDISPLAY_SHADER *)orxBank_GetNext(sstDisplay.pstShaderBank, pstShader))
      {
        /* Is active? */
        if(pstShader->bActive != orxFALSE)
        {
          /* Inits shader */
          orxDisplay_GLFW_InitShader(pstShader);

          /* Draws elements */
          glDrawElements(GL_TRIANGLE_STRIP, (GLsizei)(sstDisplay.s32BufferIndex + (sstDisplay.s32BufferIndex >> 1)), GL_UNSIGNED_SHORT, pIndexContext);
          glASSERT();
        }
      }
    }
    else
    {
      /* Draws arrays */
      glDrawElements(GL_TRIANGLE_STRIP, (GLsizei)(sstDisplay.s32BufferIndex + (sstDisplay.s32BufferIndex >> 1)), GL_UNSIGNED_SHORT, pIndexContext);
      glASSERT();
    }

    /* Clears buffer index */
    sstDisplay.s32BufferIndex = 0;

    /* Profiles */
    orxPROFILER_POP_MARKER();
  }

  /* Done! */
  return;
}

static void orxFASTCALL orxDisplay_GLFW_PrepareBitmap(const orxBITMAP *_pstBitmap, orxDISPLAY_SMOOTHING _eSmoothing, orxDISPLAY_BLEND_MODE _eBlendMode)
{
  orxBOOL bSmoothing;

  /* Checks */
  orxASSERT((_pstBitmap != orxNULL) && (_pstBitmap != sstDisplay.pstScreen));

  /* New bitmap? */
  if(_pstBitmap != sstDisplay.pstLastBitmap)
  {
    /* Draws remaining items */
    orxDisplay_GLFW_DrawArrays();

    /* Binds source's texture */
    glBindTexture(GL_TEXTURE_2D, _pstBitmap->uiTexture);
    glASSERT();

    /* No active shader? */
    if(sstDisplay.s32ActiveShaderCounter == 0)
    {
      /* Stores it */
      sstDisplay.pstLastBitmap = _pstBitmap;
    }
    else
    {
      /* Clears last bitmap */
      sstDisplay.pstLastBitmap = orxNULL;
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

      default:
      {
        glDisable(GL_BLEND);
        glASSERT();

        break;
      }
    }
  }

  /* Done! */
  return;
}

static orxINLINE void orxDisplay_GLFW_DrawBitmap(const orxBITMAP *_pstBitmap, const orxDISPLAY_MATRIX *_pmTransform, orxDISPLAY_SMOOTHING _eSmoothing, orxDISPLAY_BLEND_MODE _eBlendMode)
{
  GLfloat fWidth, fHeight;

  /* Prepares bitmap for drawing */
  orxDisplay_GLFW_PrepareBitmap(_pstBitmap, _eSmoothing, _eBlendMode);

  /* Gets bitmap working size */
  fWidth  = (GLfloat)(_pstBitmap->stClip.vBR.fX - _pstBitmap->stClip.vTL.fX);
  fHeight = (GLfloat)(_pstBitmap->stClip.vBR.fY - _pstBitmap->stClip.vTL.fY);

  /* End of buffer? */
  if(sstDisplay.s32BufferIndex > orxDISPLAY_KU32_VERTEX_BUFFER_SIZE - 1)
  {
    /* Draw arrays */
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
  sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 3].fV  = (GLfloat)(orxFLOAT_1 - _pstBitmap->fRecRealHeight * (_pstBitmap->stClip.vTL.fY + orxDISPLAY_KF_BORDER_FIX));
  sstDisplay.astVertexList[sstDisplay.s32BufferIndex].fV      =
  sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 2].fV  = (GLfloat)(orxFLOAT_1 - _pstBitmap->fRecRealHeight * (_pstBitmap->stClip.vBR.fY - orxDISPLAY_KF_BORDER_FIX));

  /* Fills the color list */
  sstDisplay.astVertexList[sstDisplay.s32BufferIndex].stRGBA      =
  sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 1].stRGBA  =
  sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 2].stRGBA  =
  sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 3].stRGBA  = _pstBitmap->stColor;

  /* Updates index */
  sstDisplay.s32BufferIndex += 4;

  /* Done! */
  return;
}

static void orxFASTCALL orxDisplay_GLFW_DrawPrimitive(orxU32 _u32VertexNumber, orxRGBA _stColor, orxBOOL _bFill, orxBOOL _bOpen)
{
  /* Profiles */
  orxPROFILER_PUSH_MARKER("orxDisplay_DrawPrimitive");

  /* Disables texturing */
  glDisable(GL_TEXTURE_2D);
  glASSERT();

  /* Has alpha? */
  if(orxRGBA_A(_stColor) != 0xFF)
  {
    /* Enables alpha blending */
    glEnable(GL_BLEND);
    glASSERT();
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glASSERT();
  }
  else
  {
    /* Disables alpha blending */
    glDisable(GL_BLEND);
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

  /* Reenables texturing */
  glEnable(GL_TEXTURE_2D);
  glASSERT();

  /* Clears last blend mode */
  sstDisplay.eLastBlendMode = orxDISPLAY_BLEND_MODE_NUMBER;

  /* Profiles */
  orxPROFILER_POP_MARKER();

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

orxSTATUS orxFASTCALL orxDisplay_GLFW_TransformText(const orxSTRING _zString, const orxBITMAP *_pstFont, const orxCHARACTER_MAP *_pstMap, const orxDISPLAY_TRANSFORM *_pstTransform, orxDISPLAY_SMOOTHING _eSmoothing, orxDISPLAY_BLEND_MODE _eBlendMode)
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
  orxDisplay_GLFW_PrepareBitmap(_pstFont, _eSmoothing, _eBlendMode);

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
          if(sstDisplay.s32BufferIndex > orxDISPLAY_KU32_VERTEX_BUFFER_SIZE - 1)
          {
            /* Draw arrays */
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
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 3].fV  = (GLfloat)(orxFLOAT_1 - _pstFont->fRecRealHeight * (pstGlyph->fY + orxDISPLAY_KF_BORDER_FIX));
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex].fV      =
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 2].fV  = (GLfloat)(orxFLOAT_1 - _pstFont->fRecRealHeight * (pstGlyph->fY + fHeight - orxDISPLAY_KF_BORDER_FIX));

          /* Fills the color list */
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex].stRGBA      =
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 1].stRGBA  =
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 2].stRGBA  =
          sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 3].stRGBA  = _pstFont->stColor;

          /* Updates counter */
          sstDisplay.s32BufferIndex += 4;
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

orxSTATUS orxFASTCALL orxDisplay_GLFW_DrawMesh(const orxBITMAP *_pstBitmap, orxDISPLAY_SMOOTHING _eSmoothing, orxDISPLAY_BLEND_MODE _eBlendMode, orxU32 _u32VertexNumber, const orxDISPLAY_VERTEX *_astVertexList)
{
  const orxBITMAP  *pstBitmap;
  orxFLOAT          fWidth, fHeight, fTop, fLeft, fXCoef, fYCoef, fXBorder, fYBorder;
  orxU32            i, iIndex, u32VertexNumber = _u32VertexNumber;
  orxSTATUS         eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_u32VertexNumber > 2);
  orxASSERT(_astVertexList != orxNULL);

  /* Gets bitmap to use */
  pstBitmap = (_pstBitmap != orxNULL) ? _pstBitmap : sstDisplay.pstLastBitmap;

  /* Prepares bitmap for drawing */
  orxDisplay_GLFW_PrepareBitmap(pstBitmap, _eSmoothing, _eBlendMode);

  /* Gets bitmap working size */
  fWidth  = pstBitmap->stClip.vBR.fX - pstBitmap->stClip.vTL.fX;
  fHeight = pstBitmap->stClip.vBR.fY - pstBitmap->stClip.vTL.fY;

  /* Gets top-left corner  */
  fTop  = pstBitmap->fRecRealHeight * pstBitmap->stClip.vTL.fY;
  fLeft = pstBitmap->fRecRealWidth * pstBitmap->stClip.vTL.fX;

  /* Gets X & Y coefs */
  fXCoef = pstBitmap->fRecRealWidth * fWidth;
  fYCoef = pstBitmap->fRecRealHeight * fHeight;

  /* Gets X & Y border fixes */
  fXBorder = pstBitmap->fRecRealWidth * orxDISPLAY_KF_BORDER_FIX;
  fYBorder = pstBitmap->fRecRealHeight * orxDISPLAY_KF_BORDER_FIX;

  /* End of buffer? */
  if(sstDisplay.s32BufferIndex + (2 * _u32VertexNumber) > orxDISPLAY_KU32_VERTEX_BUFFER_SIZE - 3)
  {
    /* Draw arrays */
    orxDisplay_GLFW_DrawArrays();

    /* Too many vertices? */
    if(_u32VertexNumber > orxDISPLAY_KU32_VERTEX_BUFFER_SIZE / 2)
    {
      /* Updates vertex number */
      u32VertexNumber = orxDISPLAY_KU32_VERTEX_BUFFER_SIZE / 2;

      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Can't draw full mesh: only drawing %d vertices out of %d.", u32VertexNumber, _u32VertexNumber);

      /* Updates result */
      eResult = orxSTATUS_FAILURE;
    }
  }

  /* For all vertices */
  for(i = 0, iIndex = 0; i < u32VertexNumber; i++, iIndex++)
  {
    /* Copies position */
    sstDisplay.astVertexList[sstDisplay.s32BufferIndex + iIndex].fX = _astVertexList[i].fX;
    sstDisplay.astVertexList[sstDisplay.s32BufferIndex + iIndex].fY = _astVertexList[i].fY;

    /* Updates UV */
    sstDisplay.astVertexList[sstDisplay.s32BufferIndex + iIndex].fU = (GLfloat)(fLeft + (fXCoef * _astVertexList[i].fU) + fXBorder);
    sstDisplay.astVertexList[sstDisplay.s32BufferIndex + iIndex].fV = (GLfloat)(orxFLOAT_1 - (fTop + (fYCoef * _astVertexList[i].fV) - fYBorder));

    /* Copies color */
    sstDisplay.astVertexList[sstDisplay.s32BufferIndex + iIndex].stRGBA = _astVertexList[i].stRGBA;

    /* Quad extremity? */
    if((i != 1) && ((i & 1) == 1))
    {
      /* Copies last two vertices */
      orxMemory_Copy(&(sstDisplay.astVertexList[sstDisplay.s32BufferIndex + iIndex + 1]), &(sstDisplay.astVertexList[sstDisplay.s32BufferIndex + iIndex - 1]), sizeof(orxDISPLAY_VERTEX));
      orxMemory_Copy(&(sstDisplay.astVertexList[sstDisplay.s32BufferIndex + iIndex + 2]), &(sstDisplay.astVertexList[sstDisplay.s32BufferIndex + iIndex]), sizeof(orxDISPLAY_VERTEX));

      /* Updates index */
      iIndex += 2;
    }
  }

  /* Not enough vertices for a final quad in the triangle strip? */
  while(iIndex & 3)
  {
    /* Completes the quad */
    orxMemory_Copy(&(sstDisplay.astVertexList[sstDisplay.s32BufferIndex + iIndex]), &(sstDisplay.astVertexList[sstDisplay.s32BufferIndex + iIndex - 1]), sizeof(orxDISPLAY_VERTEX));
    iIndex++;
  }

  /* Updates index */
  sstDisplay.s32BufferIndex += iIndex;

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
    /* Is last used bitmap? */
    if(sstDisplay.pstLastBitmap == _pstBitmap)
    {
      /* Resets it */
      sstDisplay.pstLastBitmap = orxNULL;
    }

    /* Deletes its texture */
    glDeleteTextures(1, &(_pstBitmap->uiTexture));
    glASSERT();

    /* Deletes it */
    orxBank_Free(sstDisplay.pstBitmapBank, _pstBitmap);
  }

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
    GLint iTexture;

    /* Pushes display section */
    orxConfig_PushSection(orxDISPLAY_KZ_CONFIG_SECTION);

    /* Inits it */
    pstBitmap->bSmoothing     = orxConfig_GetBool(orxDISPLAY_KZ_CONFIG_SMOOTH);
    pstBitmap->fWidth         = orxU2F(_u32Width);
    pstBitmap->fHeight        = orxU2F(_u32Height);
    pstBitmap->u32RealWidth   = orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_NPOT) ? _u32Width : orxMath_GetNextPowerOfTwo(_u32Width);
    pstBitmap->u32RealHeight  = orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_NPOT) ? _u32Height : orxMath_GetNextPowerOfTwo(_u32Height);
    pstBitmap->u32Depth       = 32;
    pstBitmap->fRecRealWidth  = orxFLOAT_1 / orxU2F(pstBitmap->u32RealWidth);
    pstBitmap->fRecRealHeight = orxFLOAT_1 / orxU2F(pstBitmap->u32RealHeight);
    pstBitmap->stColor        = orx2RGBA(0xFF, 0xFF, 0xFF, 0xFF);
    orxVector_Copy(&(pstBitmap->stClip.vTL), &orxVECTOR_0);
    orxVector_Set(&(pstBitmap->stClip.vBR), pstBitmap->fWidth, pstBitmap->fHeight, orxFLOAT_0);

    /* Backups current texture */
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &iTexture);
    glASSERT();

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
    glBindTexture(GL_TEXTURE_2D, iTexture);
    glASSERT();

    /* Pops config section */
    orxConfig_PopSection();
  }

  /* Done! */
  return pstBitmap;
}

orxSTATUS orxFASTCALL orxDisplay_GLFW_ClearBitmap(orxBITMAP *_pstBitmap, orxRGBA _stColor)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBitmap != orxNULL);

  /* Is not screen? */
  if(_pstBitmap != sstDisplay.pstScreen)
  {
    orxBITMAP *pstBackupBitmap;

    /* Backups current destination */
    pstBackupBitmap = sstDisplay.pstDestinationBitmap;

    /* Sets new destination bitmap */
    if(orxDisplay_SetDestinationBitmap(_pstBitmap) != orxSTATUS_FAILURE)
    {
      /* Clears the color buffer with given color */
      glClearColor(orxCOLOR_NORMALIZER * orxU2F(orxRGBA_R(_stColor)), orxCOLOR_NORMALIZER * orxU2F(orxRGBA_G(_stColor)), orxCOLOR_NORMALIZER * orxU2F(orxRGBA_B(_stColor)), orxCOLOR_NORMALIZER * orxU2F(orxRGBA_A(_stColor)));
      glASSERT();
      glClear(GL_COLOR_BUFFER_BIT);
      glASSERT();

      /* Restores previous destination */
      orxDisplay_SetDestinationBitmap(pstBackupBitmap);
    }
    else
    {
      GLint     iTexture;
      orxRGBA  *astBuffer, *pstPixel;

      /* Allocates buffer */
      astBuffer = (orxRGBA *)orxMemory_Allocate(_pstBitmap->u32RealWidth * _pstBitmap->u32RealHeight * sizeof(orxRGBA), orxMEMORY_TYPE_VIDEO);

      /* Checks */
      orxASSERT(astBuffer != orxNULL);

      /* For all pixels */
      for(pstPixel = astBuffer; pstPixel < astBuffer + (_pstBitmap->u32RealWidth * _pstBitmap->u32RealHeight); pstPixel++)
      {
        /* Sets its value */
        *pstPixel = _stColor;
      }

      /* Backups current texture */
      glGetIntegerv(GL_TEXTURE_BINDING_2D, &iTexture);
      glASSERT();

      /* Binds texture */
      glBindTexture(GL_TEXTURE_2D, _pstBitmap->uiTexture);
      glASSERT();

      /* Updates texture */
      glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, (GLsizei)_pstBitmap->u32RealWidth, (GLsizei)_pstBitmap->u32RealHeight, GL_RGBA, GL_UNSIGNED_BYTE, astBuffer);
      glASSERT();

      /* Restores previous texture */
      glBindTexture(GL_TEXTURE_2D, iTexture);
      glASSERT();

      /* Frees buffer */
      orxMemory_Free(astBuffer);
    }
  }
  else
  {
    /* Clears the color buffer with given color */
    glClearColor(orxCOLOR_NORMALIZER * orxU2F(orxRGBA_R(_stColor)), orxCOLOR_NORMALIZER * orxU2F(orxRGBA_G(_stColor)), orxCOLOR_NORMALIZER * orxU2F(orxRGBA_B(_stColor)), orxCOLOR_NORMALIZER * orxU2F(orxRGBA_A(_stColor)));
    glASSERT();
    glClear(GL_COLOR_BUFFER_BIT);
    glASSERT();

    /* Has depth buffer? */
    if(orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_DEPTHBUFFER))
    {
      /* Clears depth buffer */
      glClear(GL_DEPTH_BUFFER_BIT);
      glASSERT();
    }
  }

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_GLFW_Swap()
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Draws remaining items */
  orxDisplay_GLFW_DrawArrays();

  /* Swap buffers */
  glfwSwapBuffers();

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_GLFW_SetBitmapData(orxBITMAP *_pstBitmap, const orxU8 *_au8Data, orxU32 _u32ByteNumber)
{
  orxU32    u32Width, u32Height;
  orxSTATUS eResult;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBitmap != orxNULL);
  orxASSERT(_au8Data != orxNULL);

  /* Gets bitmap's size */
  u32Width  = orxF2U(_pstBitmap->fWidth);
  u32Height = orxF2U(_pstBitmap->fHeight);

  /* Valid? */
  if((_pstBitmap != sstDisplay.pstScreen) && (_u32ByteNumber == u32Width * u32Height * 4 * sizeof(orxU8)))
  {
    GLint   iTexture;
    orxU8  *pu8ImageBuffer;
    orxU32  i, u32LineSize, u32RealLineSize, u32SrcOffset, u32DstOffset;

    /* Allocates buffer */
    pu8ImageBuffer = (orxU8 *)orxMemory_Allocate(_pstBitmap->u32RealWidth * _pstBitmap->u32RealHeight * 4 * sizeof(orxU8), orxMEMORY_TYPE_VIDEO);

    /* Gets line sizes */
    u32LineSize     = orxF2U(_pstBitmap->fWidth) * 4 * sizeof(orxU8);
    u32RealLineSize = _pstBitmap->u32RealWidth * 4 * sizeof(orxU8);

    /* Clears padding */
    orxMemory_Zero(pu8ImageBuffer, u32RealLineSize * (_pstBitmap->u32RealHeight - orxF2U(_pstBitmap->fHeight)));

    /* For all lines */
    for(i = 0, u32SrcOffset = 0, u32DstOffset = u32RealLineSize * (_pstBitmap->u32RealHeight - 1);
        i < u32Height;
        i++, u32SrcOffset += u32LineSize, u32DstOffset -= u32RealLineSize)
    {
      /* Copies data */
      orxMemory_Copy(pu8ImageBuffer + u32DstOffset, _au8Data + u32SrcOffset, u32LineSize);

      /* Adds padding */
      orxMemory_Zero(pu8ImageBuffer + u32DstOffset + u32LineSize, u32RealLineSize - u32LineSize);
    }

    /* Backups current texture */
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &iTexture);
    glASSERT();

    /* Binds texture */
    glBindTexture(GL_TEXTURE_2D, _pstBitmap->uiTexture);
    glASSERT();

    /* Updates its content */
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, (GLsizei)_pstBitmap->u32RealWidth, (GLsizei)_pstBitmap->u32RealHeight, GL_RGBA, GL_UNSIGNED_BYTE, pu8ImageBuffer);
    glASSERT();

    /* Restores previous texture */
    glBindTexture(GL_TEXTURE_2D, iTexture);
    glASSERT();

    /* Frees buffer */
    orxMemory_Free(pu8ImageBuffer);

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

orxSTATUS orxFASTCALL orxDisplay_GLFW_GetBitmapData(orxBITMAP *_pstBitmap, orxU8 *_au8Data, orxU32 _u32ByteNumber)
{
  orxU32    u32BufferSize;
  orxSTATUS eResult;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBitmap != orxNULL);
  orxASSERT(_au8Data != orxNULL);

  /* Gets buffer size */
  u32BufferSize = orxF2U(_pstBitmap->fWidth * _pstBitmap->fHeight) * 4 * sizeof(orxU8);

  /* Is size matching? */
  if(_u32ByteNumber == u32BufferSize)
  {
    orxU32  u32LineSize, u32RealLineSize, u32SrcOffset, u32DstOffset, i;
    orxU8  *pu8ImageData;
    GLint   iTexture;

    /* Draws remaining items */
    orxDisplay_GLFW_DrawArrays();

    /* Allocates buffer */
    pu8ImageData = (orxU8 *)orxMemory_Allocate(_pstBitmap->u32RealWidth * _pstBitmap->u32RealHeight * 4 * sizeof(orxU8), orxMEMORY_TYPE_VIDEO);

    /* Checks */
    orxASSERT(pu8ImageData != orxNULL);

    /* Backups current texture */
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &iTexture);
    glASSERT();

    /* Screen capture? */
    if(_pstBitmap == sstDisplay.pstScreen)
    {
      /* Binds its backup texture */
      glBindTexture(GL_TEXTURE_2D, _pstBitmap->uiTexture);
      glASSERT();

      /* Copies screen content */
      glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, (GLint)(orxF2U(_pstBitmap->fHeight) - _pstBitmap->u32RealHeight), (GLsizei)orxF2U(_pstBitmap->fWidth), (GLsizei)_pstBitmap->u32RealHeight);
      glASSERT();
      glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pu8ImageData);
      glASSERT();
    }
    else
    {
      /* Binds bitmap */
      glBindTexture(GL_TEXTURE_2D, _pstBitmap->uiTexture);
      glASSERT();

      /* Copies bitmap data */
      glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pu8ImageData);
      glASSERT();
    }

    /* Restores previous texture */
    glBindTexture(GL_TEXTURE_2D, iTexture);
    glASSERT();

    /* Gets line sizes */
    u32LineSize     = orxF2U(_pstBitmap->fWidth) * 4 * sizeof(orxU8);
    u32RealLineSize = _pstBitmap->u32RealWidth * 4 * sizeof(orxU8);

    /* Clears padding */
    orxMemory_Zero(_au8Data, u32LineSize * orxF2U(_pstBitmap->fHeight));

    /* For all lines */
    for(i = 0, u32SrcOffset = u32RealLineSize * (_pstBitmap->u32RealHeight - orxF2U(_pstBitmap->fHeight)), u32DstOffset = u32LineSize * (orxF2U(_pstBitmap->fHeight) - 1);
        i < orxF2U(_pstBitmap->fHeight);
        i++, u32SrcOffset += u32RealLineSize, u32DstOffset -= u32LineSize)
    {
      /* Copies data */
      orxMemory_Copy(_au8Data + u32DstOffset, pu8ImageData + u32SrcOffset, u32LineSize);
    }

    /* Frees buffers */
    orxMemory_Free(pu8ImageData);

    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Can't get bitmap's data <0x%X> as the buffer size is %d when it should be %d.", _pstBitmap, _u32ByteNumber, u32BufferSize);

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_GLFW_SetBitmapColorKey(orxBITMAP *_pstBitmap, orxRGBA _stColor, orxBOOL _bEnable)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Not available */
  orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Not available on this platform!");

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_GLFW_SetBitmapColor(orxBITMAP *_pstBitmap, orxRGBA _stColor)
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

orxRGBA orxFASTCALL orxDisplay_GLFW_GetBitmapColor(const orxBITMAP *_pstBitmap)
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

orxSTATUS orxFASTCALL orxDisplay_GLFW_SetDestinationBitmap(orxBITMAP *_pstBitmap)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Different destination bitmap? */
  if(_pstBitmap != sstDisplay.pstDestinationBitmap)
  {
    /* Draws remaining items */
    orxDisplay_GLFW_DrawArrays();

    /* Stores it */
    sstDisplay.pstDestinationBitmap = _pstBitmap;

    /* Has framebuffer support? */
    if(orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_FRAMEBUFFER))
    {
      /* Screen? */
      if(_pstBitmap == sstDisplay.pstScreen)
      {
        /* Binds default frame buffer */
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
        glASSERT();

        /* Updates result */
        eResult = (glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT) == GL_FRAMEBUFFER_COMPLETE_EXT) ? orxSTATUS_SUCCESS : orxSTATUS_FAILURE;
        glASSERT();

        /* Flushes pending commands */
        glFlush();
        glASSERT();
      }
      /* Valid texture? */
      else if(_pstBitmap != orxNULL)
      {
        /* Binds frame buffer */
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, sstDisplay.uiFrameBuffer);
        glASSERT();

        /* Links texture to it */
        glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, _pstBitmap->uiTexture, 0);
        glASSERT();

        /* Updates result */
        eResult = (glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT) == GL_FRAMEBUFFER_COMPLETE_EXT) ? orxSTATUS_SUCCESS : orxSTATUS_FAILURE;
        glASSERT();
      }
      else
      {
        /* Updates result */
        eResult = orxSTATUS_FAILURE;
      }
    }
    else
    {
      /* Not screen? */
      if(_pstBitmap != sstDisplay.pstScreen)
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Can't set bitmap <0x%X> as destination bitmap: only screen can be used as frame buffer isn't supported by this hardware.", _pstBitmap);

        /* Updates result */
        eResult = orxSTATUS_FAILURE;
      }
    }

    /* Success? */
    if(eResult != orxSTATUS_FAILURE)
    {
      /* Is screen? */
      if(sstDisplay.pstDestinationBitmap == sstDisplay.pstScreen)
      {
        /* Inits viewport */
        glViewport(0, 0, (GLsizei)orxF2S(sstDisplay.pstDestinationBitmap->fWidth), (GLsizei)orxF2S(sstDisplay.pstDestinationBitmap->fHeight));
        glASSERT();
      }
      else
      {
        /* Inits viewport */
        glViewport(0, (orxS32)sstDisplay.pstDestinationBitmap->u32RealHeight - orxF2S(sstDisplay.pstDestinationBitmap->fHeight), (GLsizei)orxF2S(sstDisplay.pstDestinationBitmap->fWidth), (GLsizei)orxF2S(sstDisplay.pstDestinationBitmap->fHeight));
        glASSERT();
      }

      /* Inits matrices */
      glMatrixMode(GL_PROJECTION);
      glASSERT();
      glLoadIdentity();
      glASSERT();
      glOrtho(0.0f, sstDisplay.pstDestinationBitmap->fWidth, sstDisplay.pstDestinationBitmap->fHeight, 0.0f, -1.0f, 1.0f);
      glASSERT();
      glMatrixMode(GL_MODELVIEW);
      glASSERT();
      glLoadIdentity();
      glASSERT();
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

orxSTATUS orxFASTCALL orxDisplay_GLFW_TransformBitmap(const orxBITMAP *_pstSrc, const orxDISPLAY_TRANSFORM *_pstTransform, orxDISPLAY_SMOOTHING _eSmoothing, orxDISPLAY_BLEND_MODE _eBlendMode)
{
  orxDISPLAY_MATRIX mTransform;
  orxSTATUS         eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT((_pstSrc != orxNULL) && (_pstSrc != sstDisplay.pstScreen));
  orxASSERT(_pstTransform != orxNULL);

  /* Inits matrix */
  orxDisplay_GLFW_InitMatrix(&mTransform, _pstTransform->fDstX, _pstTransform->fDstY, _pstTransform->fScaleX, _pstTransform->fScaleY, _pstTransform->fRotation, _pstTransform->fSrcX, _pstTransform->fSrcY);

  /* No repeat? */
  if((_pstTransform->fRepeatX == orxFLOAT_1) && (_pstTransform->fRepeatY == orxFLOAT_1))
  {
    /* Draws it */
    orxDisplay_GLFW_DrawBitmap(_pstSrc, &mTransform, _eSmoothing, _eBlendMode);
  }
  else
  {
    orxFLOAT  i, j, fRecRepeatX;
    GLfloat   fX, fY, fWidth, fHeight, fTop, fBottom, fLeft, fRight;

    /* Prepares bitmap for drawing */
    orxDisplay_GLFW_PrepareBitmap(_pstSrc, _eSmoothing, _eBlendMode);

    /* Inits bitmap height */
    fHeight = (GLfloat)((_pstSrc->stClip.vBR.fY - _pstSrc->stClip.vTL.fY) / _pstTransform->fRepeatY);

    /* Inits texture coords */
    fLeft   = _pstSrc->fRecRealWidth * (_pstSrc->stClip.vTL.fX + orxDISPLAY_KF_BORDER_FIX);
    fTop    = orxFLOAT_1 - _pstSrc->fRecRealHeight * (_pstSrc->stClip.vTL.fY + orxDISPLAY_KF_BORDER_FIX);

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
        fBottom = (GLfloat)(orxFLOAT_1 - _pstSrc->fRecRealHeight * (_pstSrc->stClip.vTL.fY + (i * (_pstSrc->stClip.vBR.fY - _pstSrc->stClip.vTL.fY)) - orxDISPLAY_KF_BORDER_FIX));
      }
      else
      {
        /* Resets texture coords */
        fRight  = (GLfloat)(_pstSrc->fRecRealWidth * (_pstSrc->stClip.vBR.fX - orxDISPLAY_KF_BORDER_FIX));
        fBottom = (GLfloat)(orxFLOAT_1 - _pstSrc->fRecRealHeight * (_pstSrc->stClip.vBR.fY - orxDISPLAY_KF_BORDER_FIX));
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
        if(sstDisplay.s32BufferIndex > orxDISPLAY_KU32_VERTEX_BUFFER_SIZE - 1)
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
        sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 3].stRGBA  = _pstSrc->stColor;

        /* Updates counter */
        sstDisplay.s32BufferIndex += 4;
      }
    }
  }

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_GLFW_SaveBitmap(const orxBITMAP *_pstBitmap, const orxSTRING _zFilename)
{
  int             iFormat;
  GLint           iTexture;
  orxU32          u32Length, u32LineSize, u32RealLineSize, u32SrcOffset, u32DstOffset, i;
  const orxCHAR  *zExtension;
  orxU8          *pu8ImageBuffer, *pu8ImageData;
  orxSTATUS       eResult;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBitmap != orxNULL);
  orxASSERT(_zFilename != orxNULL);

  /* Gets file name's length */
  u32Length = orxString_GetLength(_zFilename);

  /* Gets extension */
  zExtension = (u32Length > 3) ? _zFilename + u32Length - 3 : orxSTRING_EMPTY;

  /* PNG? */
  if(orxString_ICompare(zExtension, "png") == 0)
  {
    /* Updates format */
    iFormat = SOIL_SAVE_TYPE_PNG;
  }
  /* DDS? */
  else if(orxString_ICompare(zExtension, "dds") == 0)
  {
    /* Updates format */
    iFormat = SOIL_SAVE_TYPE_DDS;
  }
  /* BMP? */
  else if(orxString_ICompare(zExtension, "bmp") == 0)
  {
    /* Updates format */
    iFormat = SOIL_SAVE_TYPE_BMP;
  }
  /* TGA */
  else
  {
    /* Updates format */
    iFormat = SOIL_SAVE_TYPE_TGA;
  }

  /* Allocates buffers */
  pu8ImageData    = (orxU8 *)orxMemory_Allocate(_pstBitmap->u32RealWidth * _pstBitmap->u32RealHeight * 4 * sizeof(orxU8), orxMEMORY_TYPE_VIDEO);
  pu8ImageBuffer  = (orxU8 *)orxMemory_Allocate(orxF2U(_pstBitmap->fWidth * _pstBitmap->fHeight) * 4 * sizeof(orxU8), orxMEMORY_TYPE_VIDEO);

  /* Checks */
  orxASSERT(pu8ImageData != orxNULL);
  orxASSERT(pu8ImageBuffer != orxNULL);

  /* Backups current texture */
  glGetIntegerv(GL_TEXTURE_BINDING_2D, &iTexture);
  glASSERT();

  /* Binds its associated texture */
  glBindTexture(GL_TEXTURE_2D, _pstBitmap->uiTexture);
  glASSERT();

  /* Screen capture? */
  if(_pstBitmap == sstDisplay.pstScreen)
  {
    /* Isn't screen the current destination? */
    if(sstDisplay.pstScreen != sstDisplay.pstDestinationBitmap)
    {
      /* Sets it as destination */
      orxDisplay_GLFW_SetDestinationBitmap(sstDisplay.pstScreen);
    }

    /* Copies screen content */
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, (GLint)(orxF2U(_pstBitmap->fHeight) - _pstBitmap->u32RealHeight), (GLsizei)orxF2U(_pstBitmap->fWidth), (GLsizei)_pstBitmap->u32RealHeight);
    glASSERT();
  }

  /* Copies bitmap data */
  glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pu8ImageData);
  glASSERT();

  /* Restores previous texture */
  glBindTexture(GL_TEXTURE_2D, iTexture);
  glASSERT();

  /* Gets line sizes */
  u32LineSize     = orxF2U(_pstBitmap->fWidth) * 4 * sizeof(orxU8);
  u32RealLineSize = _pstBitmap->u32RealWidth * 4 * sizeof(orxU8);

  /* Clears padding */
  orxMemory_Zero(pu8ImageBuffer, u32LineSize * orxF2U(_pstBitmap->fHeight));

  /* For all lines */
  for(i = 0, u32SrcOffset = u32RealLineSize * (_pstBitmap->u32RealHeight - orxF2U(_pstBitmap->fHeight)), u32DstOffset = u32LineSize * (orxF2U(_pstBitmap->fHeight) - 1);
    i < orxF2U(_pstBitmap->fHeight);
    i++, u32SrcOffset += u32RealLineSize, u32DstOffset -= u32LineSize)
  {
    /* Copies data */
    orxMemory_Copy(pu8ImageBuffer + u32DstOffset, pu8ImageData + u32SrcOffset, u32LineSize);
  }

  /* Saves screenshot */
  eResult = SOIL_save_image(_zFilename, iFormat, orxF2U(_pstBitmap->fWidth), orxF2U(_pstBitmap->fHeight), 4, pu8ImageBuffer) != 0 ? orxSTATUS_SUCCESS : orxSTATUS_FAILURE;

  /* Frees buffers */
  orxMemory_Free(pu8ImageBuffer);
  orxMemory_Free(pu8ImageData);

  /* Done! */
  return eResult;
}

orxBITMAP *orxFASTCALL orxDisplay_GLFW_LoadBitmap(const orxSTRING _zFilename)
{
  const orxSTRING zResourceName;
  orxBITMAP      *pstResult = orxNULL;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Gets resource name */
  zResourceName = orxResource_Locate(orxTEXTURE_KZ_RESOURCE_GROUP, _zFilename);

  /* Success? */
  if(zResourceName != orxNULL)
  {
    orxHANDLE hResource;

    /* Opens it */
    hResource = orxResource_Open(zResourceName);

    /* Success? */
    if(hResource != orxHANDLE_UNDEFINED)
    {
      orxS32  s32Size;
      orxU8  *pu8Buffer;

      /* Gets its size */
      s32Size = orxResource_GetSize(hResource);

      /* Allocates buffer */
      pu8Buffer = (orxU8 *)orxMemory_Allocate(s32Size, orxMEMORY_TYPE_MAIN);

      /* Success? */
      if(pu8Buffer != orxNULL)
      {
        unsigned char  *pu8ImageData;
        GLuint          uiWidth, uiHeight, uiBytesPerPixel;

        /* Loads data from resource */
        s32Size = orxResource_Read(hResource, s32Size, pu8Buffer);

        /* Loads image */
        pu8ImageData = SOIL_load_image_from_memory(pu8Buffer, s32Size, (int *)&uiWidth, (int *)&uiHeight, (int *)&uiBytesPerPixel, SOIL_LOAD_RGBA);

        /* Valid? */
        if(pu8ImageData != NULL)
        {
          /* Allocates bitmap */
          pstResult = (orxBITMAP *)orxBank_Allocate(sstDisplay.pstBitmapBank);

          /* Valid? */
          if(pstResult != orxNULL)
          {
            GLuint  i, uiSrcOffset, uiDstOffset, uiLineSize, uiRealLineSize, uiRealWidth, uiRealHeight;
            GLint   iTexture;
            orxU8  *pu8ImageBuffer;

            /* Gets its real size */
            uiRealWidth   = orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_NPOT) ? uiWidth : (GLuint)orxMath_GetNextPowerOfTwo(uiWidth);
            uiRealHeight  = orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_NPOT) ? uiHeight : (GLuint)orxMath_GetNextPowerOfTwo(uiHeight);

            /* Pushes display section */
            orxConfig_PushSection(orxDISPLAY_KZ_CONFIG_SECTION);

            /* Inits bitmap */
            pstResult->bSmoothing     = orxConfig_GetBool(orxDISPLAY_KZ_CONFIG_SMOOTH);
            pstResult->fWidth         = orxU2F(uiWidth);
            pstResult->fHeight        = orxU2F(uiHeight);
            pstResult->u32RealWidth   = uiRealWidth;
            pstResult->u32RealHeight  = uiRealHeight;
            pstResult->u32Depth       = 32;
            pstResult->fRecRealWidth  = orxFLOAT_1 / orxU2F(pstResult->u32RealWidth);
            pstResult->fRecRealHeight = orxFLOAT_1 / orxU2F(pstResult->u32RealHeight);
            pstResult->stColor        = orx2RGBA(0xFF, 0xFF, 0xFF, 0xFF);
            orxVector_Copy(&(pstResult->stClip.vTL), &orxVECTOR_0);
            orxVector_Set(&(pstResult->stClip.vBR), pstResult->fWidth, pstResult->fHeight, orxFLOAT_0);

            /* Allocates buffer */
            pu8ImageBuffer = (orxU8 *)orxMemory_Allocate(uiRealWidth * uiRealHeight * 4 * sizeof(orxU8), orxMEMORY_TYPE_VIDEO);

            /* Checks */
            orxASSERT(pu8ImageBuffer != orxNULL);

            /* Gets line sizes */
            uiLineSize      = uiWidth * 4 * sizeof(orxU8);
            uiRealLineSize  = uiRealWidth * 4 * sizeof(orxU8);

            /* Clears padding */
            orxMemory_Zero(pu8ImageBuffer, uiRealLineSize * (uiRealHeight - uiHeight));

            /* For all lines */
            for(i = 0, uiSrcOffset = 0, uiDstOffset = uiRealLineSize * (uiRealHeight - 1);
                i < uiHeight;
                i++, uiSrcOffset += uiLineSize, uiDstOffset -= uiRealLineSize)
            {
              /* Copies data */
              orxMemory_Copy(pu8ImageBuffer + uiDstOffset, pu8ImageData + uiSrcOffset, uiLineSize);

              /* Adds padding */
              orxMemory_Zero(pu8ImageBuffer + uiDstOffset + uiLineSize, uiRealLineSize - uiLineSize);
            }

            /* Backups current texture */
            glGetIntegerv(GL_TEXTURE_BINDING_2D, &iTexture);
            glASSERT();

            /* Creates new texture */
            glGenTextures(1, &pstResult->uiTexture);
            glASSERT();
            glBindTexture(GL_TEXTURE_2D, pstResult->uiTexture);
            glASSERT();
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)pstResult->u32RealWidth, (GLsizei)pstResult->u32RealHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, pu8ImageBuffer);
            glASSERT();
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glASSERT();
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glASSERT();
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (pstResult->bSmoothing != orxFALSE) ? GL_LINEAR : GL_NEAREST);
            glASSERT();
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (pstResult->bSmoothing != orxFALSE) ? GL_LINEAR : GL_NEAREST);
            glASSERT();

            /* Restores previous texture */
            glBindTexture(GL_TEXTURE_2D, iTexture);
            glASSERT();

            /* Frees image buffer */
            orxMemory_Free(pu8ImageBuffer);

            /* Pops config section */
            orxConfig_PopSection();
          }

          /* Deletes surface */
          SOIL_free_image_data(pu8ImageData);
        }

        /* Frees buffer */
        orxMemory_Free(pu8Buffer);
      }

      /* Closes resource */
      orxResource_Close(hResource);
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
  orxASSERT(_pstBitmap != orxNULL);

  /* Destination bitmap? */
  if(_pstBitmap == sstDisplay.pstDestinationBitmap)
  {
    /* Draws remaining items */
    orxDisplay_GLFW_DrawArrays();

    /* Is screen? */
    if(sstDisplay.pstDestinationBitmap == sstDisplay.pstScreen)
    {
      /* Sets OpenGL clipping */
      glScissor((GLint)_u32TLX, (GLint)(orxF2U(sstDisplay.pstDestinationBitmap->fHeight) - _u32BRY), (GLsizei)(_u32BRX - _u32TLX), (GLsizei)(_u32BRY - _u32TLY));
      glASSERT();
    }
    else
    {
      /* Sets OpenGL clipping */
      glScissor((GLint)_u32TLX, (GLint)(sstDisplay.pstDestinationBitmap->u32RealHeight - _u32BRY), (GLsizei)(_u32BRX - _u32TLX), (GLsizei)(_u32BRY - _u32TLY));
      glASSERT();
    }
  }

  /* Stores clip coords */
  orxVector_Set(&(_pstBitmap->stClip.vTL), orxU2F(_u32TLX), orxU2F(_u32TLY), orxFLOAT_0);
  orxVector_Set(&(_pstBitmap->stClip.vBR), orxU2F(_u32BRX), orxU2F(_u32BRY), orxFLOAT_0);

  /* Done! */
  return eResult;
}

orxU32 orxFASTCALL orxDisplay_GLFW_GetVideoModeCounter()
{
  GLFWvidmode astModeList[256];
  orxU32      u32Result = 0;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Gets video mode list */
  u32Result = (orxU32)glfwGetVideoModes(astModeList, 256);

  /* Done! */
  return u32Result;
}

orxDISPLAY_VIDEO_MODE *orxFASTCALL orxDisplay_GLFW_GetVideoMode(orxU32 _u32Index, orxDISPLAY_VIDEO_MODE *_pstVideoMode)
{
  GLFWvidmode             astModeList[256];
  orxU32                  u32Counter;
  orxDISPLAY_VIDEO_MODE  *pstResult;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstVideoMode != orxNULL);

  /* Gets video mode list */
  u32Counter = (orxU32)glfwGetVideoModes(astModeList, 256);

  /* Request the default mode? */
  if(_u32Index == orxU32_UNDEFINED)
  {
    /* Stores info */
    _pstVideoMode->u32Width       = sstDisplay.u32DefaultWidth;
    _pstVideoMode->u32Height      = sstDisplay.u32DefaultHeight;
    _pstVideoMode->u32Depth       = sstDisplay.u32DefaultDepth;
    _pstVideoMode->u32RefreshRate = sstDisplay.u32DefaultRefreshRate;

    /* 24-bit? */
    if(_pstVideoMode->u32Depth == 24)
    {
      /* Gets 32-bit instead */
      _pstVideoMode->u32Depth = 32;
    }

    /* Updates result */
    pstResult = _pstVideoMode;
  }
  /* Is index valid? */
  else if(_u32Index < u32Counter)
  {
    /* Stores info */
    _pstVideoMode->u32Width       = astModeList[_u32Index].Width;
    _pstVideoMode->u32Height      = astModeList[_u32Index].Height;
    _pstVideoMode->u32Depth       = astModeList[_u32Index].RedBits + astModeList[_u32Index].GreenBits + astModeList[_u32Index].BlueBits;
    _pstVideoMode->u32RefreshRate = (astModeList[_u32Index].RefreshRate != 0) ? astModeList[_u32Index].RefreshRate : sstDisplay.u32DefaultRefreshRate;

    /* 24-bit? */
    if(_pstVideoMode->u32Depth == 24)
    {
      /* Gets 32-bit instead */
      _pstVideoMode->u32Depth = 32;
    }

    /* Updates result */
    pstResult = _pstVideoMode;
  }
  else
  {
    /* Updates result */
    pstResult = orxNULL;
  }

  /* Done! */
  return pstResult;
}

orxBOOL orxFASTCALL orxDisplay_GLFW_IsVideoModeAvailable(const orxDISPLAY_VIDEO_MODE *_pstVideoMode)
{
  GLFWvidmode astModeList[64];
  orxU32      u32Counter, i;
  orxBOOL     bResult = orxFALSE;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstVideoMode != orxNULL);

  /* Gets video mode list */
  u32Counter = (orxU32)glfwGetVideoModes(astModeList, 64);

  /* For all mode */
  for(i = 0; i < u32Counter; i++)
  {
    /* Matches? */
    if((_pstVideoMode->u32Width == (orxU32)astModeList[i].Width)
    && (_pstVideoMode->u32Height == (orxU32)astModeList[i].Height)
    && ((_pstVideoMode->u32Depth == (orxU32)(astModeList[i].RedBits + astModeList[i].GreenBits + astModeList[i].BlueBits))
     || ((_pstVideoMode->u32Depth == 32)
      && (astModeList[i].RedBits + astModeList[i].GreenBits + astModeList[i].BlueBits == 24)))
    && ((_pstVideoMode->u32RefreshRate == (orxU32)astModeList[i].RefreshRate)
     || (_pstVideoMode->u32RefreshRate == 0)))
    {
      /* Updates result */
      bResult = orxTRUE;

      break;
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
    /* Updates VSync status */
    glfwSwapInterval(1);

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
  orxBOOL bResult = orxFALSE;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Updates result */
  bResult = orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_VSYNC) ? orxTRUE : orxFALSE;

  /* Done! */
  return bResult;
}

orxSTATUS orxFASTCALL orxDisplay_GLFW_SetVideoMode(const orxDISPLAY_VIDEO_MODE *_pstVideoMode)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Has specified video mode? */
  if(_pstVideoMode != orxNULL)
  {
    int iWidth, iHeight, iDepth, iRefreshRate;

    /* Gets its info */
    iWidth        = (int)_pstVideoMode->u32Width;
    iHeight       = (int)_pstVideoMode->u32Height;
    iDepth        = (int)_pstVideoMode->u32Depth;
    iRefreshRate  = (int)_pstVideoMode->u32RefreshRate;

    /* Not in full screen and same depth/refresh rate? */
    if(!orxFLAG_TEST_ALL(sstDisplay.u32GLFWFlags, GLFW_FULLSCREEN)
    && !orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_FULLSCREEN)
    && (sstDisplay.u32Depth == (orxU32)iDepth)
    && (sstDisplay.u32RefreshRate == (orxU32)iRefreshRate))
    {
      orxDISPLAY_EVENT_PAYLOAD stPayload;

      /* Inits event payload */
      orxMemory_Zero(&stPayload, sizeof(orxDISPLAY_EVENT_PAYLOAD));
      stPayload.u32Width                = (orxU32)iWidth;
      stPayload.u32Height               = (orxU32)iHeight;
      stPayload.u32Depth                = (orxU32)iDepth;
      stPayload.u32RefreshRate          = (orxU32)iRefreshRate;
      stPayload.u32PreviousWidth        = orxF2U(sstDisplay.pstScreen->fWidth);
      stPayload.u32PreviousHeight       = orxF2U(sstDisplay.pstScreen->fHeight);
      stPayload.u32PreviousDepth        = sstDisplay.pstScreen->u32Depth;
      stPayload.u32PreviousRefreshRate  = sstDisplay.u32RefreshRate;
      stPayload.bFullScreen             = orxFLAG_TEST_ALL(sstDisplay.u32GLFWFlags, GLFW_FULLSCREEN) ? orxTRUE : orxFALSE;

      /* Ignores resize event for now */
      sstDisplay.u32Flags |= orxDISPLAY_KU32_STATIC_FLAG_IGNORE_RESIZE;

      /* Resizes window */
      glfwSetWindowSize(iWidth, iHeight);

      /* Reactivates resize event */
      sstDisplay.u32Flags &= ~orxDISPLAY_KU32_STATIC_FLAG_IGNORE_RESIZE;

      /* Deletes screen backup texture */
      glDeleteTextures(1, &(sstDisplay.pstScreen->uiTexture));
      glASSERT();

      /* Stores screen info */
      sstDisplay.pstScreen->fWidth          = orxS2F(iWidth);
      sstDisplay.pstScreen->fHeight         = orxS2F(iHeight);
      sstDisplay.pstScreen->u32RealWidth    = orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_NPOT) ? (orxU32)iWidth : orxMath_GetNextPowerOfTwo((orxU32)iWidth);
      sstDisplay.pstScreen->u32RealHeight   = orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_NPOT) ? (orxU32)iHeight : orxMath_GetNextPowerOfTwo((orxU32)iHeight);
      sstDisplay.pstScreen->u32Depth        = _pstVideoMode->u32Depth;
      sstDisplay.pstScreen->bSmoothing      = sstDisplay.bDefaultSmoothing;
      sstDisplay.pstScreen->fRecRealWidth   = orxFLOAT_1 / orxU2F(sstDisplay.pstScreen->u32RealWidth);
      sstDisplay.pstScreen->fRecRealHeight  = orxFLOAT_1 / orxU2F(sstDisplay.pstScreen->u32RealHeight);

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

      /* Clears destination bitmap */
      sstDisplay.pstDestinationBitmap = orxNULL;

      /* Clears new display surface */
      glScissor(0, 0, (GLsizei)sstDisplay.pstScreen->u32RealWidth, (GLsizei)sstDisplay.pstScreen->u32RealHeight);
      glASSERT();
      glClear(GL_COLOR_BUFFER_BIT);
      glASSERT();

      /* Has depth buffer? */
      if(orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_DEPTHBUFFER))
      {
        /* Clears depth buffer */
        glClearDepth(1.0f);
        glASSERT();
        glClear(GL_DEPTH_BUFFER_BIT);
        glASSERT();
      }

      /* Stores screen depth & refresh rate */
      sstDisplay.u32Depth       = (orxU32)iDepth;
      sstDisplay.u32RefreshRate = (orxU32)iRefreshRate;

      /* Sends event */
      orxEVENT_SEND(orxEVENT_TYPE_DISPLAY, orxDISPLAY_EVENT_SET_VIDEO_MODE, orxNULL, orxNULL, &stPayload);
    }
    else
    {
      orxU8 **aau8BufferArray;

      /* Has opened window? */
      if(glfwGetWindowParam(GLFW_OPENED) != GL_FALSE)
      {
        /* Gets bitmap counter */
        sstDisplay.s32BitmapCounter = (orxS32)orxBank_GetCounter(sstDisplay.pstBitmapBank) - 1;

        /* Valid? */
        if(sstDisplay.s32BitmapCounter > 0)
        {
          orxBITMAP  *pstBitmap;
          orxU32      u32Index;

          /* Allocates buffer array */
          aau8BufferArray = (orxU8 **)orxMemory_Allocate(sstDisplay.s32BitmapCounter * sizeof(orxU8 *), orxMEMORY_TYPE_MAIN);

          /* Checks */
          orxASSERT(aau8BufferArray != orxNULL);

          /* For all bitmaps */
          for(pstBitmap = (orxBITMAP *)orxBank_GetNext(sstDisplay.pstBitmapBank, orxNULL), u32Index = 0;
              pstBitmap != orxNULL;
              pstBitmap = (orxBITMAP *)orxBank_GetNext(sstDisplay.pstBitmapBank, pstBitmap))
          {
            /* Not screen? */
            if(pstBitmap != sstDisplay.pstScreen)
            {
              /* Allocates its buffer */
              aau8BufferArray[u32Index] = (orxU8 *)orxMemory_Allocate(pstBitmap->u32RealWidth * pstBitmap->u32RealHeight * 4 * sizeof(orxU8), orxMEMORY_TYPE_VIDEO);

              /* Checks */
              orxASSERT(aau8BufferArray[u32Index] != orxNULL);

              /* Binds bitmap */
              glBindTexture(GL_TEXTURE_2D, pstBitmap->uiTexture);
              glASSERT();

              /* Copies bitmap data */
              glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, aau8BufferArray[u32Index++]);
              glASSERT();

              /* Deletes it */
              glDeleteTextures(1, &(pstBitmap->uiTexture));
              glASSERT();
            }
          }
        }

        /* Deletes screen backup texture */
        glDeleteTextures(1, &(sstDisplay.pstScreen->uiTexture));
        glASSERT();

        /* Gets shader counter */
        sstDisplay.s32ShaderCounter = (orxS32)orxBank_GetCounter(sstDisplay.pstShaderBank);

        /* Valid? */
        if(sstDisplay.s32ShaderCounter > 0)
        {
          orxDISPLAY_SHADER *pstShader;

          /* For all shaders */
          for(pstShader = (orxDISPLAY_SHADER *)orxBank_GetNext(sstDisplay.pstShaderBank, orxNULL);
              pstShader != orxNULL;
              pstShader = (orxDISPLAY_SHADER *)orxBank_GetNext(sstDisplay.pstShaderBank, pstShader))
          {
            /* Deletes its program */
            glDeleteObjectARB(pstShader->hProgram);
            glASSERT();
            pstShader->hProgram = (GLhandleARB)orxU32_UNDEFINED;
          }
        }

        /* Had frame buffer? */
        if(sstDisplay.uiFrameBuffer != 0)
        {
          /* Deletes it */
          glDeleteFramebuffersEXT(1, &(sstDisplay.uiFrameBuffer));
          glASSERT();
          sstDisplay.uiFrameBuffer = 0;
        }

        /* Had index buffer? */
        if(sstDisplay.uiIndexBuffer != 0)
        {
          /* Deletes it */
          glDeleteBuffersARB(1, &(sstDisplay.uiIndexBuffer));
          glASSERT();
          sstDisplay.uiIndexBuffer = 0;
        }

        /* Closes window */
        glfwCloseWindow();
      }

      /* Updates window hints */
      glfwOpenWindowHint(GLFW_WINDOW_NO_RESIZE, orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_NO_RESIZE) ? GL_TRUE : GL_FALSE);
      glfwOpenWindowHint(GLFW_REFRESH_RATE, iRefreshRate);

      /* Depending on video depth */
      switch(iDepth)
      {
        /* 16-bit */
        case 16:
        {
          /* Updates video mode */
          eResult = (glfwOpenWindow(iWidth, iHeight, 5, 6, 5, 0, orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_DEPTHBUFFER) ? 16 : 0, 0, (int)sstDisplay.u32GLFWFlags) != GL_FALSE) ? orxSTATUS_SUCCESS : orxSTATUS_FAILURE;

          break;
        }

        /* 24-bit */
        case 24:
        {
          /* Updates video mode */
          eResult = (glfwOpenWindow(iWidth, iHeight, 8, 8, 8, 0, orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_DEPTHBUFFER) ? 16 : 0, 0, (int)sstDisplay.u32GLFWFlags) != GL_FALSE) ? orxSTATUS_SUCCESS : orxSTATUS_FAILURE;

          break;
        }

        /* 32-bit */
        default:
        case 32:
        {
          /* Updates video mode */
          eResult = (glfwOpenWindow(iWidth, iHeight, 8, 8, 8, 8, orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_DEPTHBUFFER) ? 16 : 0, 0, (int)sstDisplay.u32GLFWFlags) != GL_FALSE) ? orxSTATUS_SUCCESS : orxSTATUS_FAILURE;

          break;
        }
      }

      /* Success? */
      if(eResult != orxSTATUS_FAILURE)
      {
        orxDISPLAY_EVENT_PAYLOAD stPayload;

        /* Inits event payload */
        orxMemory_Zero(&stPayload, sizeof(orxDISPLAY_EVENT_PAYLOAD));
        stPayload.u32Width                = (orxU32)iWidth;
        stPayload.u32Height               = (orxU32)iHeight;
        stPayload.u32Depth                = (orxU32)iDepth;
        stPayload.u32RefreshRate          = (orxU32)iRefreshRate;
        stPayload.u32PreviousWidth        = orxF2U(sstDisplay.pstScreen->fWidth);
        stPayload.u32PreviousHeight       = orxF2U(sstDisplay.pstScreen->fHeight);
        stPayload.u32PreviousDepth        = sstDisplay.pstScreen->u32Depth;
        stPayload.u32PreviousRefreshRate  = sstDisplay.u32RefreshRate;
        stPayload.bFullScreen             = orxFLAG_TEST_ALL(sstDisplay.u32GLFWFlags, GLFW_FULLSCREEN) ? orxTRUE : orxFALSE;

        /* Inits extensions */
        orxDisplay_GLFW_InitExtensions();

        /* Inits OpenGL */
        glEnable(GL_TEXTURE_2D);
        glASSERT();
        glEnable(GL_SCISSOR_TEST);
        glASSERT();
        glDisable(GL_DITHER);
        glASSERT();
        glDisable(GL_LIGHTING);
        glASSERT();
        glDisable(GL_FOG);
        glASSERT();
        glDisable(GL_CULL_FACE);
        glASSERT();
        glDisable(GL_DEPTH_TEST);
        glASSERT();
        glDisable(GL_STENCIL_TEST);
        glASSERT();
        glDisable(GL_ALPHA_TEST);
        glASSERT();
        glEnableClientState(GL_VERTEX_ARRAY);
        glASSERT();
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glASSERT();
        glEnableClientState(GL_COLOR_ARRAY);
        glASSERT();

        /* Selects arrays */
        glVertexPointer(2, GL_FLOAT, sizeof(orxDISPLAY_VERTEX), &(sstDisplay.astVertexList[0].fX));
        glASSERT();
        glTexCoordPointer(2, GL_FLOAT, sizeof(orxDISPLAY_VERTEX), &(sstDisplay.astVertexList[0].fU));
        glASSERT();
        glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(orxDISPLAY_VERTEX), &(sstDisplay.astVertexList[0].stRGBA));
        glASSERT();

        /* Has framebuffer support? */
        if(orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_FRAMEBUFFER))
        {
          /* Generates frame buffer */
          glGenFramebuffersEXT(1, &(sstDisplay.uiFrameBuffer));
          glASSERT();
        }

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

        /* Has VBO support? */
        if(orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_VBO))
        {
          /* Generates index buffer object (IBO) */
          glGenBuffersARB(1, &(sstDisplay.uiIndexBuffer));
          glASSERT();

          /* Binds it */
          glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, sstDisplay.uiIndexBuffer);
          glASSERT();

          /* Fills it */
          glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, orxDISPLAY_KU32_INDEX_BUFFER_SIZE * sizeof(GLushort), &(sstDisplay.au16IndexList), GL_STATIC_DRAW);
          glASSERT();
        }

        /* Updates screen info */
        if(_pstVideoMode != orxNULL)
        {
          sstDisplay.pstScreen->fWidth          = orx2F(iWidth);
          sstDisplay.pstScreen->fHeight         = orx2F(iHeight);
          sstDisplay.pstScreen->u32RealWidth    = orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_NPOT) ? (orxU32)iWidth : orxMath_GetNextPowerOfTwo((orxU32)iWidth);
          sstDisplay.pstScreen->u32RealHeight   = orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_NPOT) ? (orxU32)iHeight : orxMath_GetNextPowerOfTwo((orxU32)iHeight);
          sstDisplay.pstScreen->u32Depth        = _pstVideoMode->u32Depth;
          sstDisplay.pstScreen->bSmoothing      = sstDisplay.bDefaultSmoothing;
          sstDisplay.pstScreen->fRecRealWidth   = orxFLOAT_1 / orxU2F(sstDisplay.pstScreen->u32RealWidth);
          sstDisplay.pstScreen->fRecRealHeight  = orxFLOAT_1 / orxU2F(sstDisplay.pstScreen->u32RealHeight);
        }
        orxVector_Copy(&(sstDisplay.pstScreen->stClip.vTL), &orxVECTOR_0);
        orxVector_Set(&(sstDisplay.pstScreen->stClip.vBR), sstDisplay.pstScreen->fWidth, sstDisplay.pstScreen->fHeight, orxFLOAT_0);

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

        /* Clears destination bitmap */
        sstDisplay.pstDestinationBitmap = orxNULL;

        /* Clears new display surface */
        glScissor(0, 0, (GLsizei)sstDisplay.pstScreen->u32RealWidth, (GLsizei)sstDisplay.pstScreen->u32RealHeight);
        glASSERT();
        glClear(GL_COLOR_BUFFER_BIT);
        glASSERT();

        /* Enforces VSync status */
        orxDisplay_GLFW_EnableVSync(orxDisplay_GLFW_IsVSyncEnabled());

        /* Had bitmaps? */
        if(sstDisplay.s32BitmapCounter > 0)
        {
          orxBITMAP  *pstBitmap;
          orxU32      u32Index;

          /* For all bitmaps */
          for(pstBitmap = (orxBITMAP *)orxBank_GetNext(sstDisplay.pstBitmapBank, orxNULL), u32Index = 0;
              pstBitmap != orxNULL;
              pstBitmap = (orxBITMAP *)orxBank_GetNext(sstDisplay.pstBitmapBank, pstBitmap))
          {
            /* Not screen? */
            if(pstBitmap != sstDisplay.pstScreen)
            {
              /* Creates new texture */
              glGenTextures(1, &pstBitmap->uiTexture);
              glASSERT();
              glBindTexture(GL_TEXTURE_2D, pstBitmap->uiTexture);
              glASSERT();
              glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)pstBitmap->u32RealWidth, (GLsizei)pstBitmap->u32RealHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid *)aau8BufferArray[u32Index]);
              glASSERT();
              glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
              glASSERT();
              glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
              glASSERT();
              glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (pstBitmap->bSmoothing != orxFALSE) ? GL_LINEAR : GL_NEAREST);
              glASSERT();
              glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (pstBitmap->bSmoothing != orxFALSE) ? GL_LINEAR : GL_NEAREST);
              glASSERT();

              /* Deletes buffer */
              orxMemory_Free(aau8BufferArray[u32Index++]);
            }
          }

          /* Deletes buffer array */
          orxMemory_Free(aau8BufferArray);
        }

        /* Had shaders? */
        if(sstDisplay.s32ShaderCounter > 0)
        {
          orxDISPLAY_SHADER *pstShader;

          /* For all shaders */
          for(pstShader = (orxDISPLAY_SHADER *)orxBank_GetNext(sstDisplay.pstShaderBank, orxNULL);
              pstShader != orxNULL;
              pstShader = (orxDISPLAY_SHADER *)orxBank_GetNext(sstDisplay.pstShaderBank, pstShader))
          {
            /* Compiles it */
            orxDisplay_GLFW_CompileShader(pstShader);
          }
        }

        /* Clears counters */
        sstDisplay.s32BitmapCounter = sstDisplay.s32ShaderCounter = 0;

        /* Stores screen depth & refresh rate */
        sstDisplay.u32Depth       = (orxU32)iDepth;
        sstDisplay.u32RefreshRate = (orxU32)iRefreshRate;

        /* Sends event */
        orxEVENT_SEND(orxEVENT_TYPE_DISPLAY, orxDISPLAY_EVENT_SET_VIDEO_MODE, orxNULL, orxNULL, &stPayload);
      }
    }
  }

  /* Succesful? */
  if(eResult != orxSTATUS_FAILURE)
  {
    /* Pushes display section */
    orxConfig_PushSection(orxDISPLAY_KZ_CONFIG_SECTION);

    /* Isn't fullscreen? */
    if(!orxFLAG_TEST_ALL(sstDisplay.u32GLFWFlags, GLFW_FULLSCREEN))
    {
      orxVECTOR vPosition;

      /* Has position value? */
      if(orxConfig_GetVector(orxDISPLAY_KZ_CONFIG_POSITION, &vPosition) != orxNULL)
      {
        /* Updates window's position */
        glfwSetWindowPos((int)vPosition.fX, (int)vPosition.fY);
      }
    }

    /* Updates its title */
    glfwSetWindowTitle(orxConfig_GetString(orxDISPLAY_KZ_CONFIG_TITLE));

    /* Pops config section */
    orxConfig_PopSection();
  }

  /* Clears last blend mode & last bitmap */
  sstDisplay.eLastBlendMode = orxDISPLAY_BLEND_MODE_NUMBER;
  sstDisplay.pstLastBitmap  = orxNULL;

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
    if(!orxFLAG_TEST_ALL(sstDisplay.u32GLFWFlags, GLFW_FULLSCREEN))
    {
      /* Updates window style */
      orxFLAG_SET(sstDisplay.u32GLFWFlags, GLFW_FULLSCREEN, GLFW_WINDOW);

      /* Asks for update */
      bUpdate = orxTRUE;
    }
  }
  else
  {
    /* Was full screen? */
    if(orxFLAG_TEST_ALL(sstDisplay.u32GLFWFlags, GLFW_FULLSCREEN))
    {
      /* Updates window style */
      orxFLAG_SET(sstDisplay.u32GLFWFlags, GLFW_WINDOW, GLFW_FULLSCREEN);

      /* Asks for update */
      bUpdate = orxTRUE;
    }
  }

  /* Should update? */
  if(bUpdate != orxFALSE)
  {
    orxDISPLAY_VIDEO_MODE stVideoMode;

    /* Inits video mode */
    stVideoMode.u32Width        = orxF2U(sstDisplay.pstScreen->fWidth);
    stVideoMode.u32Height       = orxF2U(sstDisplay.pstScreen->fHeight);
    stVideoMode.u32Depth        = sstDisplay.pstScreen->u32Depth;
    stVideoMode.u32RefreshRate  = sstDisplay.u32RefreshRate;

    /* Updates video mode */
    eResult = orxDisplay_GLFW_SetVideoMode(&stVideoMode);

    /* Failed? */
    if(eResult == orxSTATUS_FAILURE)
    {
      /* Restores previous full screen status */
      orxFLAG_SWAP(sstDisplay.u32GLFWFlags, GLFW_FULLSCREEN);

      /* Updates video mode */
      orxDisplay_GLFW_SetVideoMode(&stVideoMode);
    }

    /* Has full screen? */
    if(orxFLAG_TEST(sstDisplay.u32GLFWFlags, GLFW_FULLSCREEN))
    {
      /* Updates current status */
      orxFLAG_SET(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_FULLSCREEN, orxDISPLAY_KU32_STATIC_FLAG_NONE);
    }
    else
    {
      /* Updates current status */
      orxFLAG_SET(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_NONE, orxDISPLAY_KU32_STATIC_FLAG_FULLSCREEN);
    }
  }

  /* Done! */
  return eResult;
}

orxBOOL orxFASTCALL orxDisplay_GLFW_IsFullScreen()
{
  orxBOOL bResult = orxFALSE;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Updates result */
  bResult = orxFLAG_TEST_ALL(sstDisplay.u32GLFWFlags, GLFW_FULLSCREEN) ? orxTRUE : orxFALSE;

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

    /* For all indices */
    for(i = 0, u16Index = 0; i < orxDISPLAY_KU32_INDEX_BUFFER_SIZE; i += 6, u16Index += 4)
    {
      /* Computes them */
      sstDisplay.au16IndexList[i]     = u16Index;
      sstDisplay.au16IndexList[i + 1] = u16Index;
      sstDisplay.au16IndexList[i + 2] = u16Index + 1;
      sstDisplay.au16IndexList[i + 3] = u16Index + 2;
      sstDisplay.au16IndexList[i + 4] = u16Index + 3;
      sstDisplay.au16IndexList[i + 5] = u16Index + 3;
    }

    /* Inits GLFW */
    eResult = (glfwInit() != GL_FALSE) ? orxSTATUS_SUCCESS : orxSTATUS_FAILURE;

    /* Valid? */
    if(eResult != orxSTATUS_FAILURE)
    {
      /* Creates banks */
      sstDisplay.pstBitmapBank  = orxBank_Create(orxDISPLAY_KU32_BITMAP_BANK_SIZE, sizeof(orxBITMAP), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);
      sstDisplay.pstShaderBank  = orxBank_Create(orxDISPLAY_KU32_SHADER_BANK_SIZE, sizeof(orxDISPLAY_SHADER), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

      /* Valid? */
      if((sstDisplay.pstBitmapBank != orxNULL)
      && (sstDisplay.pstShaderBank != orxNULL))
      {
        orxDISPLAY_VIDEO_MODE stVideoMode;
        GLFWvidmode           stDesktopMode;

        /* Gets desktop mode */
        glfwGetDesktopMode(&stDesktopMode);

        /* Updates default mode */
        sstDisplay.u32DefaultWidth        = (orxU32)stDesktopMode.Width;
        sstDisplay.u32DefaultHeight       = (orxU32)stDesktopMode.Height;
        sstDisplay.u32DefaultDepth        = (orxU32)(stDesktopMode.RedBits + stDesktopMode.GreenBits +stDesktopMode.BlueBits);
        sstDisplay.u32DefaultRefreshRate  = stDesktopMode.RefreshRate;

        /* Pushes display section */
        orxConfig_PushSection(orxDISPLAY_KZ_CONFIG_SECTION);

        /* Gets resolution from config */
        stVideoMode.u32Width        = orxConfig_HasValue(orxDISPLAY_KZ_CONFIG_WIDTH) ? orxConfig_GetU32(orxDISPLAY_KZ_CONFIG_WIDTH) : sstDisplay.u32DefaultWidth;
        stVideoMode.u32Height       = orxConfig_HasValue(orxDISPLAY_KZ_CONFIG_HEIGHT) ? orxConfig_GetU32(orxDISPLAY_KZ_CONFIG_HEIGHT) : sstDisplay.u32DefaultHeight;
        stVideoMode.u32Depth        = orxConfig_HasValue(orxDISPLAY_KZ_CONFIG_DEPTH) ? orxConfig_GetU32(orxDISPLAY_KZ_CONFIG_DEPTH) : sstDisplay.u32DefaultDepth;
        stVideoMode.u32RefreshRate  = orxConfig_HasValue(orxDISPLAY_KZ_CONFIG_REFRESH_RATE) ? orxConfig_GetU32(orxDISPLAY_KZ_CONFIG_REFRESH_RATE) : sstDisplay.u32DefaultRefreshRate;

        /* Full screen? */
        if(orxConfig_GetBool(orxDISPLAY_KZ_CONFIG_FULLSCREEN) != orxFALSE)
        {
          /* Updates flags */
          sstDisplay.u32GLFWFlags = GLFW_FULLSCREEN;
        }
        else
        {
          /* Updates flags */
          sstDisplay.u32GLFWFlags = GLFW_WINDOW;
        }

        /* No decoration? */
        if((orxConfig_HasValue(orxDISPLAY_KZ_CONFIG_DECORATION) != orxFALSE)
        && (orxConfig_GetBool(orxDISPLAY_KZ_CONFIG_DECORATION) == orxFALSE))
        {
          /* Logs message */
          orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "This GLFW plugin can't create windows with no decorations.");
        }

        /* Sets module as ready */
        sstDisplay.u32Flags = orxDISPLAY_KU32_STATIC_FLAG_READY;

        /* Depth buffer? */
        if(orxConfig_GetBool(orxDISPLAY_KZ_CONFIG_DEPTHBUFFER) != orxFALSE)
        {
           /* Updates flags */
           orxFLAG_SET(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_DEPTHBUFFER, orxDISPLAY_KU32_STATIC_FLAG_NONE);
        }

        /* Doesn't allow resize? */
        if(orxConfig_GetBool(orxDISPLAY_KZ_CONFIG_ALLOW_RESIZE) == orxFALSE)
        {
          /* Updates flags */
          orxFLAG_SET(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_NO_RESIZE, orxDISPLAY_KU32_STATIC_FLAG_NONE);
        }

        /* Allocates screen bitmap */
        sstDisplay.pstScreen = (orxBITMAP *)orxBank_Allocate(sstDisplay.pstBitmapBank);
        orxMemory_Zero(sstDisplay.pstScreen, sizeof(orxBITMAP));

        /* Sets video mode? */
        if((eResult = orxDisplay_GLFW_SetVideoMode(&stVideoMode)) == orxSTATUS_FAILURE)
        {
          /* Updates display flags */
          sstDisplay.u32GLFWFlags = GLFW_WINDOW;

          /* Updates resolution */
          stVideoMode.u32Width        = sstDisplay.u32DefaultWidth;
          stVideoMode.u32Height       = sstDisplay.u32DefaultHeight;
          stVideoMode.u32Depth        = sstDisplay.u32DefaultDepth;
          stVideoMode.u32RefreshRate  = sstDisplay.u32DefaultRefreshRate;

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

          /* Has VSync value? */
          if(orxConfig_HasValue(orxDISPLAY_KZ_CONFIG_VSYNC) != orxFALSE)
          {
            /* Updates vertical sync */
            orxDisplay_GLFW_EnableVSync(orxConfig_GetBool(orxDISPLAY_KZ_CONFIG_VSYNC));
          }

          /* Inits info */
          sstDisplay.bDefaultSmoothing  = orxConfig_GetBool(orxDISPLAY_KZ_CONFIG_SMOOTH);
          sstDisplay.eLastBlendMode     = orxDISPLAY_BLEND_MODE_NUMBER;

          /* Gets clock */
          pstClock = orxClock_FindFirst(orx2F(-1.0f), orxCLOCK_TYPE_CORE);

          /* Valid? */
          if(pstClock != orxNULL)
          {
            /* Registers update function */
            eResult = orxClock_Register(pstClock, orxDisplay_GLFW_Update, orxNULL, orxMODULE_ID_DISPLAY, orxCLOCK_PRIORITY_HIGHEST);
          }

          /* Shows mouse cursor */
          glfwEnable(GLFW_MOUSE_CURSOR);

          /* Ignores resize event for now */
          sstDisplay.u32Flags |= orxDISPLAY_KU32_STATIC_FLAG_IGNORE_RESIZE;

          /* Registers resize callback */
          glfwSetWindowSizeCallback(orxDisplay_GLFW_ResizeCallback);

          /* Reactivates resize event */
          sstDisplay.u32Flags &= ~orxDISPLAY_KU32_STATIC_FLAG_IGNORE_RESIZE;
        }
        else
        {
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

        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Failed to create bitmap/shader banks.");
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
    /* Exits from GLFW */
    glfwTerminate();

    /* Unregisters update function */
    orxClock_Unregister(orxClock_FindFirst(orx2F(-1.0f), orxCLOCK_TYPE_CORE), orxDisplay_GLFW_Update);

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

orxHANDLE orxFASTCALL orxDisplay_GLFW_CreateShader(const orxSTRING _zCode, const orxLINKLIST *_pstParamList)
{
  orxHANDLE hResult = orxHANDLE_UNDEFINED;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstParamList != orxNULL);

  /* Has shader support? */
  if(orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_SHADER))
  {
    /* Valid? */
    if((_zCode != orxNULL) && (_zCode != orxSTRING_EMPTY))
    {
      orxDISPLAY_SHADER *pstShader;

      /* Creates a new shader */
      pstShader = (orxDISPLAY_SHADER *)orxBank_Allocate(sstDisplay.pstShaderBank);

      /* Successful? */
      if(pstShader != orxNULL)
      {
        orxSHADER_PARAM  *pstParam;
        orxCHAR          *pc;
        orxS32            s32Free;

        /* Inits shader code buffer */
        sstDisplay.acShaderCodeBuffer[0]  = sstDisplay.acShaderCodeBuffer[orxDISPLAY_KU32_SHADER_BUFFER_SIZE - 1] = orxCHAR_NULL;
        pc                                = sstDisplay.acShaderCodeBuffer;
        s32Free                           = orxDISPLAY_KU32_SHADER_BUFFER_SIZE - 1;

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
              orxS32 s32Offset;

              /* Adds its literal value */
              s32Offset = (pstParam->u32ArraySize >= 1) ? orxString_NPrint(pc, s32Free, "uniform float %s[%d];\n", pstParam->zName, pstParam->u32ArraySize) : orxString_NPrint(pc, s32Free, "uniform float %s;\n", pstParam->zName);
              pc       += s32Offset;
              s32Free  -= s32Offset;

              break;
            }

            case orxSHADER_PARAM_TYPE_TEXTURE:
            {
              orxS32 s32Offset;

              /* Adds its literal value and automated coordinates */
              s32Offset = (pstParam->u32ArraySize >= 1) ? orxString_NPrint(pc, s32Free, "uniform sampler2D %s[%d];\nuniform float %s"orxDISPLAY_KZ_SHADER_SUFFIX_TOP"[%d];\nuniform float %s"orxDISPLAY_KZ_SHADER_SUFFIX_LEFT"[%d];\nuniform float %s"orxDISPLAY_KZ_SHADER_SUFFIX_BOTTOM"[%d];\nuniform float %s"orxDISPLAY_KZ_SHADER_SUFFIX_RIGHT"[%d];\n", pstParam->zName, pstParam->u32ArraySize, pstParam->zName, pstParam->u32ArraySize, pstParam->zName, pstParam->u32ArraySize, pstParam->zName, pstParam->u32ArraySize, pstParam->zName, pstParam->u32ArraySize) : orxString_NPrint(pc, s32Free, "uniform sampler2D %s;\nuniform float %s"orxDISPLAY_KZ_SHADER_SUFFIX_TOP";\nuniform float %s"orxDISPLAY_KZ_SHADER_SUFFIX_LEFT";\nuniform float %s"orxDISPLAY_KZ_SHADER_SUFFIX_BOTTOM";\nuniform float %s"orxDISPLAY_KZ_SHADER_SUFFIX_RIGHT";\n", pstParam->zName, pstParam->zName, pstParam->zName, pstParam->zName, pstParam->zName);
              pc       += s32Offset;
              s32Free  -= s32Offset;

              break;
            }

            case orxSHADER_PARAM_TYPE_VECTOR:
            {
              orxS32 s32Offset;

              /* Adds its literal value */
              s32Offset = (pstParam->u32ArraySize >= 1) ? orxString_NPrint(pc, s32Free, "uniform vec3 %s[%d];\n", pstParam->zName, pstParam->u32ArraySize) : orxString_NPrint(pc, s32Free, "uniform vec3 %s;\n", pstParam->zName);
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

        /* Adds code */
        orxString_NPrint(pc, s32Free, "%s\n", _zCode);

        /* Inits shader */
        pstShader->hProgram               = (GLhandleARB)orxU32_UNDEFINED;
        pstShader->iTextureCounter        = 0;
        pstShader->s32ParamCounter        = 0;
        pstShader->bActive                = orxFALSE;
        pstShader->bInitialized           = orxFALSE;
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
  glDeleteObjectARB(pstShader->hProgram);
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

  /* Draws remaining items */
  orxDisplay_GLFW_DrawArrays();

  /* Gets shader */
  pstShader = (orxDISPLAY_SHADER *)_hShader;

  /* Uses program */
  glUseProgramObjectARB(pstShader->hProgram);
  glASSERT();

  /* Updates its status */
  pstShader->bActive      = orxTRUE;
  pstShader->bInitialized = orxFALSE;

  /* Updates active shader counter */
  sstDisplay.s32ActiveShaderCounter++;

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_GLFW_StopShader(orxHANDLE _hShader)
{
  orxDISPLAY_SHADER  *pstShader;
  orxSTATUS           eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT((_hShader != orxHANDLE_UNDEFINED) && (_hShader != orxNULL));

  /* Draws remaining items */
  orxDisplay_GLFW_DrawArrays();

  /* Gets shader */
  pstShader = (orxDISPLAY_SHADER *)_hShader;

  /* Wasn't initialized? */
  if(pstShader->bInitialized == orxFALSE)
  {
    /* Inits it */
    orxDisplay_GLFW_InitShader(pstShader);

    /* Defines the vertex list */
    sstDisplay.astVertexList[0].fX  =
    sstDisplay.astVertexList[1].fX  = sstDisplay.pstScreen->stClip.vTL.fX;
    sstDisplay.astVertexList[2].fX  =
    sstDisplay.astVertexList[3].fX  = sstDisplay.pstScreen->stClip.vBR.fX;
    sstDisplay.astVertexList[1].fY  =
    sstDisplay.astVertexList[3].fY  = sstDisplay.pstScreen->stClip.vTL.fY;
    sstDisplay.astVertexList[0].fY  =
    sstDisplay.astVertexList[2].fY  = sstDisplay.pstScreen->stClip.vBR.fY;

    /* Defines the texture coord list */
    sstDisplay.astVertexList[0].fU  =
    sstDisplay.astVertexList[1].fU  = (GLfloat)(sstDisplay.pstScreen->fRecRealWidth * sstDisplay.pstScreen->stClip.vTL.fX);
    sstDisplay.astVertexList[2].fU  =
    sstDisplay.astVertexList[3].fU  = (GLfloat)(sstDisplay.pstScreen->fRecRealWidth * sstDisplay.pstScreen->stClip.vBR.fX);
    sstDisplay.astVertexList[1].fV  =
    sstDisplay.astVertexList[3].fV  = (GLfloat)(orxFLOAT_1 - sstDisplay.pstScreen->fRecRealHeight * sstDisplay.pstScreen->stClip.vTL.fY);
    sstDisplay.astVertexList[0].fV  =
    sstDisplay.astVertexList[2].fV  = (GLfloat)(orxFLOAT_1 - sstDisplay.pstScreen->fRecRealHeight * sstDisplay.pstScreen->stClip.vBR.fY);

    /* Fills the color list */
    sstDisplay.astVertexList[sstDisplay.s32BufferIndex].stRGBA      =
    sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 1].stRGBA  =
    sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 2].stRGBA  =
    sstDisplay.astVertexList[sstDisplay.s32BufferIndex + 3].stRGBA  = sstDisplay.pstScreen->stColor;

    /* Updates counter */
    sstDisplay.s32BufferIndex = 4;

    /* Draws arrays */
    orxDisplay_GLFW_DrawArrays();
  }

  /* Uses default program */
  glUseProgramObjectARB(0);
  glASSERT();

  /* Selects first texture unit */
  glActiveTextureARB(GL_TEXTURE0_ARB);
  glASSERT();

  /* Clears texture counter */
  pstShader->iTextureCounter = 0;

  /* Clears texture info list */
  orxMemory_Zero(pstShader->astTextureInfoList, sstDisplay.iTextureUnitNumber * sizeof(orxDISPLAY_TEXTURE_INFO));

  /* Updates its status */
  pstShader->bActive = orxFALSE;

  /* Updates active shader counter */
  sstDisplay.s32ActiveShaderCounter--;

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
    orxASSERT(pstShader->s32ParamCounter < sstDisplay.iTextureUnitNumber);

    /* Inits buffer */
    acBuffer[255] = orxCHAR_NULL;

    /* Gets corresponding param info */
    pstInfo = &pstShader->astParamInfoList[pstShader->s32ParamCounter];

    /* Updates result */
    s32Result = pstShader->s32ParamCounter++;

    /* Array? */
    if(_s32Index >= 0)
    {
      /* Prints its name */
      orxString_NPrint(acBuffer, 255, "%s[%d]", _zParam, _s32Index);

      /* Gets parameter location */
      pstInfo->iLocation = glGetUniformLocationARB(pstShader->hProgram, acBuffer);
      glASSERT();

      /* Gets top parameter location */
      orxString_NPrint(acBuffer, 255, "%s"orxDISPLAY_KZ_SHADER_SUFFIX_TOP"%[ld]", _zParam, _s32Index);
      pstInfo->iLocationTop = glGetUniformLocationARB(pstShader->hProgram, (const GLcharARB *)acBuffer);
      glASSERT();

      /* Gets left parameter location */
      orxString_NPrint(acBuffer, 255, "%s"orxDISPLAY_KZ_SHADER_SUFFIX_LEFT"%[ld]", _zParam, _s32Index);
      pstInfo->iLocationLeft = glGetUniformLocationARB(pstShader->hProgram, (const GLcharARB *)acBuffer);
      glASSERT();

      /* Gets bottom parameter location */
      orxString_NPrint(acBuffer, 255, "%s"orxDISPLAY_KZ_SHADER_SUFFIX_BOTTOM"%[ld]", _zParam, _s32Index);
      pstInfo->iLocationBottom = glGetUniformLocationARB(pstShader->hProgram, (const GLcharARB *)acBuffer);
      glASSERT();

      /* Gets right parameter location */
      orxString_NPrint(acBuffer, 255, "%s"orxDISPLAY_KZ_SHADER_SUFFIX_RIGHT"%[ld]", _zParam, _s32Index);
      pstInfo->iLocationRight = glGetUniformLocationARB(pstShader->hProgram, (const GLcharARB *)acBuffer);
      glASSERT();
    }
    else
    {
      /* Gets parameter location */
      pstInfo->iLocation = glGetUniformLocationARB(pstShader->hProgram, (const GLcharARB *)_zParam);
      glASSERT();

      /* Gets top parameter location */
      orxString_NPrint(acBuffer, 255, "%s"orxDISPLAY_KZ_SHADER_SUFFIX_TOP, _zParam);
      pstInfo->iLocationTop = glGetUniformLocationARB(pstShader->hProgram, (const GLcharARB *)acBuffer);
      glASSERT();

      /* Gets left parameter location */
      orxString_NPrint(acBuffer, 255, "%s"orxDISPLAY_KZ_SHADER_SUFFIX_LEFT, _zParam);
      pstInfo->iLocationLeft = glGetUniformLocationARB(pstShader->hProgram, (const GLcharARB *)acBuffer);
      glASSERT();

      /* Gets bottom parameter location */
      orxString_NPrint(acBuffer, 255, "%s"orxDISPLAY_KZ_SHADER_SUFFIX_BOTTOM, _zParam);
      pstInfo->iLocationBottom = glGetUniformLocationARB(pstShader->hProgram, (const GLcharARB *)acBuffer);
      glASSERT();

      /* Gets right parameter location */
      orxString_NPrint(acBuffer, 255, "%s"orxDISPLAY_KZ_SHADER_SUFFIX_RIGHT, _zParam);
      pstInfo->iLocationRight = glGetUniformLocationARB(pstShader->hProgram, (const GLcharARB *)acBuffer);
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
      orxString_NPrint(acBuffer, 255, "%s[%d]", _zParam, _s32Index);
      acBuffer[255] = orxCHAR_NULL;

      /* Gets parameter location */
      s32Result = (orxS32)glGetUniformLocationARB(pstShader->hProgram, acBuffer);
      glASSERT();
    }
    else
    {
      /* Gets parameter location */
      s32Result = (orxS32)glGetUniformLocationARB(pstShader->hProgram, (const GLcharARB *)_zParam);
      glASSERT();
    }
  }

  /* Done! */
  return s32Result;
}

orxSTATUS orxFASTCALL orxDisplay_GLFW_SetShaderBitmap(orxHANDLE _hShader, orxS32 _s32ID, const orxBITMAP *_pstValue)
{
  orxDISPLAY_SHADER  *pstShader;
  orxSTATUS           eResult;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT((_hShader != orxHANDLE_UNDEFINED) && (_hShader != orxNULL));

  /* Gets shader */
  pstShader = (orxDISPLAY_SHADER *)_hShader;

  /* Has free texture unit left? */
  if(pstShader->iTextureCounter < sstDisplay.iTextureUnitNumber)
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
      pstShader->astTextureInfoList[pstShader->iTextureCounter].iLocation = pstShader->astParamInfoList[_s32ID].iLocation;
      pstShader->astTextureInfoList[pstShader->iTextureCounter].pstBitmap = _pstValue;

      /* Updates corner values */
      glUniform1fARB(pstShader->astParamInfoList[_s32ID].iLocationTop, (GLfloat)(orxFLOAT_1 - (_pstValue->fRecRealHeight * _pstValue->stClip.vTL.fY)));
      glASSERT();
      glUniform1fARB(pstShader->astParamInfoList[_s32ID].iLocationLeft, (GLfloat)(_pstValue->fRecRealWidth * _pstValue->stClip.vTL.fX));
      glASSERT();
      glUniform1fARB(pstShader->astParamInfoList[_s32ID].iLocationBottom, (GLfloat)(orxFLOAT_1 - (_pstValue->fRecRealHeight * _pstValue->stClip.vBR.fY)));
      glASSERT();
      glUniform1fARB(pstShader->astParamInfoList[_s32ID].iLocationRight, (GLfloat)(_pstValue->fRecRealWidth * _pstValue->stClip.vBR.fX));
      glASSERT();

      /* Updates texture counter */
      pstShader->iTextureCounter++;

      /* Updates result */
      eResult = orxSTATUS_SUCCESS;
    }
    else
    {
      /* Outputs log */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Can't find texture parameter (ID <%d>) for fragment shader.", _s32ID);

      /* Updates result */
      eResult = orxSTATUS_FAILURE;
    }
  }
  else
  {
    /* Outputs log */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Can't bind texture parameter (ID <%d>) for fragment shader: all the texture units are used.", _s32ID);

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
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
    /* Updates its value */
    glUniform1fARB((GLint)_s32ID, (GLfloat)_fValue);
    glASSERT();

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
    /* Updates its value */
    glUniform3fARB((GLint)_s32ID, (GLfloat)_pvValue->fX, (GLfloat)_pvValue->fY, (GLfloat)_pvValue->fZ);
    glASSERT();

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
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_GLFW_Init, DISPLAY, INIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_GLFW_Exit, DISPLAY, EXIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_GLFW_Swap, DISPLAY, SWAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_GLFW_GetScreenBitmap, DISPLAY, GET_SCREEN_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_GLFW_GetScreenSize, DISPLAY, GET_SCREEN_SIZE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_GLFW_CreateBitmap, DISPLAY, CREATE_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_GLFW_DeleteBitmap, DISPLAY, DELETE_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_GLFW_LoadBitmap, DISPLAY, LOAD_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_GLFW_SaveBitmap, DISPLAY, SAVE_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_GLFW_SetDestinationBitmap, DISPLAY, SET_DESTINATION_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_GLFW_ClearBitmap, DISPLAY, CLEAR_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_GLFW_SetBitmapClipping, DISPLAY, SET_BITMAP_CLIPPING);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_GLFW_SetBitmapColorKey, DISPLAY, SET_BITMAP_COLOR_KEY);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_GLFW_SetBitmapData, DISPLAY, SET_BITMAP_DATA);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_GLFW_GetBitmapData, DISPLAY, GET_BITMAP_DATA);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_GLFW_SetBitmapColor, DISPLAY, SET_BITMAP_COLOR);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_GLFW_GetBitmapColor, DISPLAY, GET_BITMAP_COLOR);
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
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_GLFW_EnableVSync, DISPLAY, ENABLE_VSYNC);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_GLFW_IsVSyncEnabled, DISPLAY, IS_VSYNC_ENABLED);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_GLFW_SetFullScreen, DISPLAY, SET_FULL_SCREEN);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_GLFW_IsFullScreen, DISPLAY, IS_FULL_SCREEN);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_GLFW_GetVideoModeCounter, DISPLAY, GET_VIDEO_MODE_COUNTER);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_GLFW_GetVideoMode, DISPLAY, GET_VIDEO_MODE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_GLFW_SetVideoMode, DISPLAY, SET_VIDEO_MODE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_GLFW_IsVideoModeAvailable, DISPLAY, IS_VIDEO_MODE_AVAILABLE);
orxPLUGIN_USER_CORE_FUNCTION_END();
