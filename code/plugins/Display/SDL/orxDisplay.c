/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2010 Orx-Project
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
 * @date 14/11/2003
 * @author iarwain@orx-project.org
 *
 * SDL display plugin implementation
 *
 */


#include "orxPluginAPI.h"

#include <SDL.h>
#include <SOIL.h>

#ifdef __orxMAC__

  #define GL_GLEXT_PROTOTYPES

#endif /* __orxMAC__ */

#include <SDL_opengl.h>


/** Module flags
 */
#define orxDISPLAY_KU32_STATIC_FLAG_NONE        0x00000000 /**< No flags */

#define orxDISPLAY_KU32_STATIC_FLAG_READY       0x00000001 /**< Ready flag */
#define orxDISPLAY_KU32_STATIC_FLAG_VSYNC       0x00000002 /**< VSync flag */
#define orxDISPLAY_KU32_STATIC_FLAG_SHADER      0x00000004 /**< Shader support flag */
#define orxDISPLAY_KU32_STATIC_FLAG_FOCUS       0x00000008 /**< Focus flag */

#define orxDISPLAY_KU32_STATIC_MASK_ALL         0xFFFFFFFF /**< All mask */

#define orxDISPLAY_KU32_SCREEN_WIDTH            1024
#define orxDISPLAY_KU32_SCREEN_HEIGHT           768
#define orxDISPLAY_KU32_SCREEN_DEPTH            32

#define orxDISPLAY_KU32_BITMAP_BANK_SIZE        256
#define orxDISPLAY_KU32_SHADER_BANK_SIZE        64

#define orxDISPLAY_KU32_BUFFER_SIZE             (12 * 1024)
#define orxDISPLAY_KU32_SHADER_BUFFER_SIZE      65536


/**  Misc defines
 */
#ifdef __orxDEBUG__

#define glASSERT()                                                      \
do                                                                      \
{                                                                       \
  GLenum eError = glGetError();                                         \
  orxASSERT(eError == GL_NO_ERROR && "OpenGL error code: %ld", eError); \
} while(orxFALSE)

#else /* __orxDEBUG__ */

#define glASSERT()

#endif /* __orxDEBUG__ */


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Internal bitmap structure
 */
struct __orxBITMAP_t
{
  GLuint                    uiTexture;
  orxBOOL                   bSmoothing;
  orxFLOAT                  fWidth, fHeight;
  orxU32                    u32RealWidth, u32RealHeight, u32Depth;
  orxFLOAT                  fRecRealWidth, fRecRealHeight;
  orxCOLOR                  stColor;
  orxAABOX                  stClip;
};

/** Internal texture info structure
 */
typedef struct __orxDISPLAY_TEXTURE_INFO_t
{
  GLint                     iLocation;
  orxBITMAP                *pstBitmap;

} orxDISPLAY_TEXTURE_INFO;

/** Internal shader structure
 */
typedef struct __orxDISPLAY_SHADER_t
{
  GLhandleARB               hProgram;
  GLint                     iTextureCounter;
  orxBOOL                   bActive;
  orxBOOL                   bInitialized;
  orxSTRING                 zCode;
  orxDISPLAY_TEXTURE_INFO  *astTextureInfoList;

} orxDISPLAY_SHADER;

/** Static structure
 */
typedef struct __orxDISPLAY_STATIC_t
{
  orxBANK                  *pstBitmapBank;
  orxBANK                  *pstShaderBank;
  orxBOOL                   bDefaultSmoothing;
  SDL_Surface              *pstScreenSurface;
  orxBITMAP                *pstScreen;
  orxBITMAP                *pstDestinationBitmap;
  const orxBITMAP          *pstLastBitmap;
  orxDISPLAY_BLEND_MODE     eLastBlendMode;
  GLint                     iTextureUnitNumber;
  orxS32                    s32ActiveShaderCounter;
  orxU32                    u32SDLFlags;
  orxU32                    u32Flags;
  GLfloat                   afVertexList[orxDISPLAY_KU32_BUFFER_SIZE];
  GLfloat                   afTextureCoordList[orxDISPLAY_KU32_BUFFER_SIZE];
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

PFNGLCREATEPROGRAMOBJECTARBPROC   glCreateProgramObjectARB  = NULL;
PFNGLCREATESHADEROBJECTARBPROC    glCreateShaderObjectARB   = NULL;
PFNGLDELETEOBJECTARBPROC          glDeleteObjectARB         = NULL;
PFNGLSHADERSOURCEARBPROC          glShaderSourceARB         = NULL;
PFNGLCOMPILESHADERARBPROC         glCompileShaderARB        = NULL;
PFNGLATTACHOBJECTARBPROC          glAttachObjectARB         = NULL;
PFNGLLINKPROGRAMARBPROC           glLinkProgramARB          = NULL;
PFNGLGETOBJECTPARAMETERIVARBPROC  glGetObjectParameterivARB = NULL;
PFNGLGETINFOLOGARBPROC            glGetInfoLogARB           = NULL;
PFNGLUSEPROGRAMOBJECTARBPROC      glUseProgramObjectARB     = NULL;
PFNGLGETUNIFORMLOCATIONARBPROC    glGetUniformLocationARB   = NULL;
PFNGLUNIFORM1FARBPROC             glUniform1fARB            = NULL;
PFNGLUNIFORM3FARBPROC             glUniform3fARB            = NULL;
PFNGLUNIFORM1IARBPROC             glUniform1iARB            = NULL;
PFNGLACTIVETEXTUREARBPROC         glActiveTextureARB        = NULL;

#endif


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

static void orxFASTCALL orxDisplay_SDL_EventUpdate(const orxCLOCK_INFO *_pstClockInfo, void *_pContext)
{
  SDL_Event stSDLEvent;
  Uint8     u8State;

  /* Clears event */
  orxMemory_Zero(&stSDLEvent, sizeof(SDL_Event));

  /* Clears wheel event */
  orxEVENT_SEND(orxEVENT_TYPE_FIRST_RESERVED + SDL_MOUSEBUTTONDOWN, SDL_MOUSEBUTTONDOWN, orxNULL, orxNULL, &stSDLEvent);

  /* Gets application state */
  u8State = SDL_GetAppState();

  /* Has focus? */
  if(u8State & (SDL_APPINPUTFOCUS | SDL_APPMOUSEFOCUS))
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

  /* Handles all pending events */
  while(SDL_PollEvent(&stSDLEvent))
  {
    /* Depending on type */
    switch(stSDLEvent.type)
    {
      /* Closing? */
      case SDL_QUIT:
      {
        /* Sends system close event */
        orxEvent_SendShort(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_CLOSE);

        break;
      }

      /* Gained/Lost focus? */
      case SDL_ACTIVEEVENT:
      {
        /* Sends system focus gained event */
        orxEvent_SendShort(orxEVENT_TYPE_SYSTEM, (stSDLEvent.active.gain) ? orxSYSTEM_EVENT_FOCUS_GAINED : orxSYSTEM_EVENT_FOCUS_LOST);

        break;
      }

      case SDL_MOUSEBUTTONDOWN:
      {
        /* Not a wheel move? */
        if((stSDLEvent.button.button != SDL_BUTTON_WHEELDOWN)
        && (stSDLEvent.button.button != SDL_BUTTON_WHEELUP))
        {
          /* Stops */
          break;
        }
      }
      case SDL_MOUSEMOTION:
      {
        /* Sends reserved event */
        orxEVENT_SEND(orxEVENT_TYPE_FIRST_RESERVED + stSDLEvent.type, stSDLEvent.type, orxNULL, orxNULL, &stSDLEvent);

        break;
      }

      default:
      {
        break;
      }
    }
  }

  return;
}

static orxINLINE void orxDisplay_SDL_InitShaderSupport()
{
  const orxCHAR *zExtensionList;

  /* Gets OpenGL extensions */
  zExtensionList = (const orxCHAR *)glGetString(GL_EXTENSIONS);
  glASSERT();

  /* Can support shader? */
  if((orxString_SearchString(zExtensionList, "GL_ARB_shader_objects") != orxNULL)
  && (orxString_SearchString(zExtensionList, "GL_ARB_shading_language_100") != orxNULL)
  && (orxString_SearchString(zExtensionList, "GL_ARB_vertex_shader") != orxNULL)
  && (orxString_SearchString(zExtensionList, "GL_ARB_fragment_shader") != orxNULL))
  {
#ifndef __orxMAC__

    #define orxDISPLAY_LOAD_EXTENSION_FUNCTION(TYPE, FN)  FN = (TYPE)SDL_GL_GetProcAddress(#FN);

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
    orxDISPLAY_LOAD_EXTENSION_FUNCTION(PFNGLACTIVETEXTUREARBPROC, glActiveTextureARB);

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
}

static orxSTATUS orxFASTCALL orxDisplay_SDL_CompileShader(orxDISPLAY_SHADER *_pstShader)
{
  static const orxCHAR *szVertexShaderSource =
    "void main()"
    "{"
    "  gl_TexCoord[0] = gl_MultiTexCoord0;"
    "  gl_Position    = ftransform();"
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
        orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Couldn't link shader program:\n%s\n", acBuffer);

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
      orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Couldn't compile fragment shader:\n%s\n", acBuffer);

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
    orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Couldn't compile vertex shader:\n%s\n", acBuffer);

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

static orxINLINE void orxDisplay_SDL_InitShader(orxDISPLAY_SHADER *_pstShader)
{
  GLint i;

  /* Uses its program */
  glUseProgramObjectARB(_pstShader->hProgram);
  glASSERT();

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

    /* Screen? */
    if(_pstShader->astTextureInfoList[i].pstBitmap == sstDisplay.pstScreen)
    {
      /* Copies screen content */
      glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, orxF2U(sstDisplay.pstScreen->fHeight) - sstDisplay.pstScreen->u32RealHeight, sstDisplay.pstScreen->u32RealWidth, sstDisplay.pstScreen->u32RealHeight);
      glASSERT();
    }
  }

  /* Updates its status */
  _pstShader->bInitialized = orxTRUE;

  /* Done! */
  return;
}

static orxINLINE void orxDisplay_SDL_PrepareBitmap(const orxBITMAP *_pstBitmap, orxDISPLAY_SMOOTHING _eSmoothing, orxDISPLAY_BLEND_MODE _eBlendMode)
{
  orxBOOL bSmoothing;

  /* Checks */
  orxASSERT((_pstBitmap != orxNULL) && (_pstBitmap != sstDisplay.pstScreen));

  /* New bitmap? */
  if(_pstBitmap != sstDisplay.pstLastBitmap)
  {
    /* No active shader? */
    if(sstDisplay.s32ActiveShaderCounter == 0)
    {
      /* Binds source's texture */
      glBindTexture(GL_TEXTURE_2D, _pstBitmap->uiTexture);
      glASSERT();

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

  /* Applies color */
  glColor4f(_pstBitmap->stColor.vRGB.fR, _pstBitmap->stColor.vRGB.fG, _pstBitmap->stColor.vRGB.fB, _pstBitmap->stColor.fAlpha);
  glASSERT();

  /* Done! */
  return;
}

static orxINLINE void orxDisplay_SDL_DrawBitmap(const orxBITMAP *_pstBitmap, orxDISPLAY_SMOOTHING _eSmoothing, orxDISPLAY_BLEND_MODE _eBlendMode)
{
  GLfloat fWidth, fHeight;

  /* Prepares bitmap for drawing */
  orxDisplay_SDL_PrepareBitmap(_pstBitmap, _eSmoothing, _eBlendMode);

  /* Gets bitmap working size */
  fWidth  = (GLfloat)(_pstBitmap->stClip.vBR.fX - _pstBitmap->stClip.vTL.fX);
  fHeight = (GLfloat)(_pstBitmap->stClip.vBR.fY - _pstBitmap->stClip.vTL.fY);

  /* Defines the vertex list */
  GLfloat afVertexList[] =
  {
    0.0f, fHeight,
    0.0f, 0.0f,
    fWidth, fHeight,
    fWidth, 0.0f
  };

  /* Defines the texture coord list */
  GLfloat afTextureCoordList[] =
  {
    (GLfloat)(_pstBitmap->fRecRealWidth * _pstBitmap->stClip.vTL.fX), (GLfloat)(orxFLOAT_1 - _pstBitmap->fRecRealHeight * _pstBitmap->stClip.vBR.fY),
    (GLfloat)(_pstBitmap->fRecRealWidth * _pstBitmap->stClip.vTL.fX), (GLfloat)(orxFLOAT_1 - _pstBitmap->fRecRealHeight * _pstBitmap->stClip.vTL.fY),
    (GLfloat)(_pstBitmap->fRecRealWidth * _pstBitmap->stClip.vBR.fX), (GLfloat)(orxFLOAT_1 - _pstBitmap->fRecRealHeight * _pstBitmap->stClip.vBR.fY),
    (GLfloat)(_pstBitmap->fRecRealWidth * _pstBitmap->stClip.vBR.fX), (GLfloat)(orxFLOAT_1 - _pstBitmap->fRecRealHeight * _pstBitmap->stClip.vTL.fY)
  };

  /* Selects arrays */
  glVertexPointer(2, GL_FLOAT, 0, afVertexList);
  glASSERT();
  glTexCoordPointer(2, GL_FLOAT, 0, afTextureCoordList);
  glASSERT();

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
        orxDisplay_SDL_InitShader(pstShader);

        /* Draws arrays */
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glASSERT();
      }
    }
  }
  else
  {
    /* Draws arrays */
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glASSERT();
  }

  /* Done! */
  return;
}

orxBITMAP *orxFASTCALL orxDisplay_SDL_GetScreen()
{
  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Done! */
  return sstDisplay.pstScreen;
}

orxSTATUS orxFASTCALL orxDisplay_SDL_TransformText(const orxSTRING _zString, const orxBITMAP *_pstFont, const orxCHARACTER_MAP *_pstMap, const orxDISPLAY_TRANSFORM *_pstTransform, orxDISPLAY_SMOOTHING _eSmoothing, orxDISPLAY_BLEND_MODE _eBlendMode)
{
  const orxCHAR  *pc;
  orxU32          u32Counter;
  GLfloat         fX, fY, fWidth, fHeight;
  orxSTATUS       eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_zString != orxNULL);
  orxASSERT(_pstFont != orxNULL);
  orxASSERT(_pstMap != orxNULL);
  orxASSERT(_pstTransform != orxNULL);

  /* Translates it */
  glTranslatef(_pstTransform->fDstX, _pstTransform->fDstY, 0.0f);
  glASSERT();

  /* Applies rotation */
  glRotatef(orxMATH_KF_RAD_TO_DEG * _pstTransform->fRotation, 0.0f, 0.0f, 1.0f);
  glASSERT();

  /* Applies scale */
  glScalef(_pstTransform->fScaleX, _pstTransform->fScaleY, 1.0f);
  glASSERT();

  /* Applies pivot translation */
  glTranslatef(-_pstTransform->fSrcX, -_pstTransform->fSrcY, 0.0f);
  glASSERT();

  /* Gets character's size */
  fWidth  = _pstMap->vCharacterSize.fX;
  fHeight = _pstMap->vCharacterSize.fY;

  /* Prepares font for drawing */
  orxDisplay_SDL_PrepareBitmap(_pstFont, _eSmoothing, _eBlendMode);

  /* For all characters */
  for(pc = _zString, u32Counter = 0, fX = fY = 0.0f; *pc != orxCHAR_NULL; pc++)
  {
    /* Depending on character */
    switch(*pc)
    {
      case orxCHAR_CR:
      {
        /* Half EOL? */
        if(*(pc + 1) == orxCHAR_LF)
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
        /* Is defined? */
        if(_pstMap->astCharacterList[*pc].fX >= orxFLOAT_0)
        {
          /* End of buffer? */
          if(u32Counter > orxDISPLAY_KU32_BUFFER_SIZE - 12)
          {
            /* Selects arrays */
            glVertexPointer(2, GL_FLOAT, 0, sstDisplay.afVertexList);
            glASSERT();
            glTexCoordPointer(2, GL_FLOAT, 0, sstDisplay.afTextureCoordList);
            glASSERT();

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
                  orxDisplay_SDL_InitShader(pstShader);

                  /* Draws arrays */
                  glDrawArrays(GL_TRIANGLE_STRIP, 0, u32Counter >> 1);
                  glASSERT();
                }
              }
            }
            else
            {
              /* Draws arrays */
              glDrawArrays(GL_TRIANGLE_STRIP, 0, u32Counter >> 1);
              glASSERT();
            }

            /* Resets counter */
            u32Counter = 0;
          }

          /* Outputs vertices and texture coordinates */
          sstDisplay.afVertexList[u32Counter]       =
          sstDisplay.afVertexList[u32Counter + 2]   =
          sstDisplay.afVertexList[u32Counter + 4]   = fX;
          sstDisplay.afVertexList[u32Counter + 6]   =
          sstDisplay.afVertexList[u32Counter + 8]   =
          sstDisplay.afVertexList[u32Counter + 10]  = fX + fWidth;
          sstDisplay.afVertexList[u32Counter + 5]   =
          sstDisplay.afVertexList[u32Counter + 9]   =
          sstDisplay.afVertexList[u32Counter + 11]  = fY;
          sstDisplay.afVertexList[u32Counter + 1]   =
          sstDisplay.afVertexList[u32Counter + 3]   =
          sstDisplay.afVertexList[u32Counter + 7]   = fY + fHeight;
          
          sstDisplay.afTextureCoordList[u32Counter]       =
          sstDisplay.afTextureCoordList[u32Counter + 2]   =
          sstDisplay.afTextureCoordList[u32Counter + 4]   = (GLfloat)(_pstFont->fRecRealWidth * _pstMap->astCharacterList[*pc].fX);
          sstDisplay.afTextureCoordList[u32Counter + 6]   =
          sstDisplay.afTextureCoordList[u32Counter + 8]   =
          sstDisplay.afTextureCoordList[u32Counter + 10]  = (GLfloat)(_pstFont->fRecRealWidth * (_pstMap->astCharacterList[*pc].fX + fWidth));
          sstDisplay.afTextureCoordList[u32Counter + 5]   =
          sstDisplay.afTextureCoordList[u32Counter + 9]   =
          sstDisplay.afTextureCoordList[u32Counter + 11]  = (GLfloat)(orxFLOAT_1 - _pstFont->fRecRealHeight * _pstMap->astCharacterList[*pc].fY);
          sstDisplay.afTextureCoordList[u32Counter + 1]   =
          sstDisplay.afTextureCoordList[u32Counter + 3]   =
          sstDisplay.afTextureCoordList[u32Counter + 7]   = (GLfloat)(orxFLOAT_1 - _pstFont->fRecRealHeight * (_pstMap->astCharacterList[*pc].fY + fHeight));

          /* Updates counter */
          u32Counter += 12;
        }
      }

      /* Updates X position */
      fX += fWidth;
    }
  }

  /* Has remaining data? */
  if(u32Counter != 0)
  {
    /* Selects arrays */
    glVertexPointer(2, GL_FLOAT, 0, sstDisplay.afVertexList);
    glASSERT();
    glTexCoordPointer(2, GL_FLOAT, 0, sstDisplay.afTextureCoordList);
    glASSERT();

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
          orxDisplay_SDL_InitShader(pstShader);

          /* Draws arrays */
          glDrawArrays(GL_TRIANGLE_STRIP, 0, u32Counter >> 1);
          glASSERT();
        }
      }
    }
    else
    {
      /* Draws arrays */
      glDrawArrays(GL_TRIANGLE_STRIP, 0, u32Counter >> 1);
      glASSERT();
    }
  }

  /* Restores identity */
  glLoadIdentity();
  glASSERT();

  /* Done! */
  return eResult;
}

void orxFASTCALL orxDisplay_SDL_DeleteBitmap(orxBITMAP *_pstBitmap)
{
  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBitmap != orxNULL);

  /* Not screen? */
  if(_pstBitmap != sstDisplay.pstScreen)
  {
    /* Deletes its texture */
    glDeleteTextures(1, &(_pstBitmap->uiTexture));
    glASSERT();

    /* Deletes it */
    orxBank_Free(sstDisplay.pstBitmapBank, _pstBitmap);
  }

  return;
}

orxBITMAP *orxFASTCALL orxDisplay_SDL_CreateBitmap(orxU32 _u32Width, orxU32 _u32Height)
{
  orxBITMAP *pstBitmap;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Allocates bitmap */
  pstBitmap = (orxBITMAP *)orxBank_Allocate(sstDisplay.pstBitmapBank);

  /* Valid? */
  if(pstBitmap != orxNULL)
  {
    /* Pushes display section */
    orxConfig_PushSection(orxDISPLAY_KZ_CONFIG_SECTION);

    /* Inits it */
    pstBitmap->bSmoothing     = orxConfig_GetBool(orxDISPLAY_KZ_CONFIG_SMOOTH);
    pstBitmap->fWidth         = orxU2F(_u32Width);
    pstBitmap->fHeight        = orxU2F(_u32Height);
    pstBitmap->u32RealWidth   = orxMath_GetNextPowerOfTwo(_u32Width);
    pstBitmap->u32RealHeight  = orxMath_GetNextPowerOfTwo(_u32Height);
    pstBitmap->u32Depth       = 32;
    pstBitmap->fRecRealWidth  = orxFLOAT_1 / orxU2F(pstBitmap->u32RealWidth);
    pstBitmap->fRecRealHeight = orxFLOAT_1 / orxU2F(pstBitmap->u32RealHeight);
    orxColor_Set(&(pstBitmap->stColor), &orxVECTOR_WHITE, orxFLOAT_1);
    orxVector_Copy(&(pstBitmap->stClip.vTL), &orxVECTOR_0);
    orxVector_Set(&(pstBitmap->stClip.vBR), pstBitmap->fWidth, pstBitmap->fHeight, orxFLOAT_0);

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

    /* Pops config section */
    orxConfig_PopSection();
  }

  /* Done! */
  return pstBitmap;
}

orxSTATUS orxFASTCALL orxDisplay_SDL_ClearBitmap(orxBITMAP *_pstBitmap, orxRGBA _stColor)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBitmap != orxNULL);

  /* Is not screen? */
  if(_pstBitmap != sstDisplay.pstScreen)
  {
    orxRGBA *astBuffer, *pstPixel;

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

    /* Binds texture */
    glBindTexture(GL_TEXTURE_2D, _pstBitmap->uiTexture);
    glASSERT();

    /* Updates texture */
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, _pstBitmap->u32RealWidth, _pstBitmap->u32RealHeight, GL_RGBA, GL_UNSIGNED_BYTE, astBuffer);
    glASSERT();

    /* Frees buffer */
    orxMemory_Free(astBuffer);
  }
  else
  {
    /* Clears the color buffer with given color */
    glClearColor((1.0f / 255.f) * orxU2F(orxRGBA_R(_stColor)), (1.0f / 255.f) * orxU2F(orxRGBA_G(_stColor)), (1.0f / 255.f) * orxU2F(orxRGBA_B(_stColor)), (1.0f / 255.f) * orxU2F(orxRGBA_A(_stColor)));
    glASSERT();
    glClear(GL_COLOR_BUFFER_BIT);
    glASSERT();
  }

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_SDL_Swap()
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Swap buffers */
  SDL_GL_SwapBuffers();

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_SDL_SetBitmapData(orxBITMAP *_pstBitmap, const orxU8 *_au8Data, orxU32 _u32ByteNumber)
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
  if((_pstBitmap != sstDisplay.pstScreen) && (_u32ByteNumber == u32Width * u32Height * sizeof(orxRGBA)))
  {
    orxU8        *pu8ImageBuffer;
    orxU32        i, u32LineSize, u32RealLineSize, u32SrcOffset, u32DstOffset;

    /* Allocates buffer */
    pu8ImageBuffer = (orxU8 *)orxMemory_Allocate(_pstBitmap->u32RealWidth * _pstBitmap->u32RealHeight * sizeof(orxRGBA), orxMEMORY_TYPE_VIDEO);

    /* Gets line sizes */
    u32LineSize     = orxF2U(_pstBitmap->fWidth) * sizeof(orxRGBA);
    u32RealLineSize = _pstBitmap->u32RealWidth * sizeof(orxRGBA);

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

    /* Binds texture */
    glBindTexture(GL_TEXTURE_2D, _pstBitmap->uiTexture);
    glASSERT();

    /* Updates its content */
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, _pstBitmap->u32RealWidth, _pstBitmap->u32RealHeight, GL_RGBA, GL_UNSIGNED_BYTE, pu8ImageBuffer);
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

orxSTATUS orxFASTCALL orxDisplay_SDL_SetBitmapColorKey(orxBITMAP *_pstBitmap, orxRGBA _stColor, orxBOOL _bEnable)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Not available */
  orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Not available on this platform!");

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_SDL_SetBitmapColor(orxBITMAP *_pstBitmap, orxRGBA _stColor)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBitmap != orxNULL);

  /* Not screen? */
  if(_pstBitmap != sstDisplay.pstScreen)
  {
    /* Stores it */
    orxColor_SetRGBA(&(_pstBitmap->stColor), _stColor);
  }

  /* Done! */
  return eResult;
}

orxRGBA orxFASTCALL orxDisplay_SDL_GetBitmapColor(const orxBITMAP *_pstBitmap)
{
  orxRGBA stResult = 0;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBitmap != orxNULL);

  /* Not screen? */
  if(_pstBitmap != sstDisplay.pstScreen)
  {
    /* Updates result */
    stResult = orxColor_ToRGBA(&(_pstBitmap->stColor));
  }

  /* Done! */
  return stResult;
}

orxSTATUS orxFASTCALL orxDisplay_SDL_SetDestinationBitmap(orxBITMAP *_pstDst)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT((_pstDst == sstDisplay.pstScreen) && "Can only draw on screen with this version!");

  /* Different destination bitmap? */
  if(_pstDst != sstDisplay.pstDestinationBitmap)
  {
    /* Stores it */
    sstDisplay.pstDestinationBitmap = _pstDst;

    /* Inits viewport */
    glViewport(0, 0, (GLsizei)sstDisplay.pstDestinationBitmap->fWidth, (GLsizei)sstDisplay.pstDestinationBitmap->fHeight);
    glASSERT();

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

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_SDL_BlitBitmap(const orxBITMAP *_pstSrc, const orxFLOAT _fPosX, orxFLOAT _fPosY, orxDISPLAY_SMOOTHING _eSmoothing, orxDISPLAY_BLEND_MODE _eBlendMode)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT((_pstSrc != orxNULL) && (_pstSrc != sstDisplay.pstScreen));

  /* Translates it */
  glTranslatef(_fPosX, _fPosY, 0.0f);
  glASSERT();

  /* Draws it */
  orxDisplay_SDL_DrawBitmap(_pstSrc, _eSmoothing, _eBlendMode);

  /* Restores identity */
  glLoadIdentity();
  glASSERT();

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_SDL_TransformBitmap(const orxBITMAP *_pstSrc, const orxDISPLAY_TRANSFORM *_pstTransform, orxDISPLAY_SMOOTHING _eSmoothing, orxDISPLAY_BLEND_MODE _eBlendMode)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT((_pstSrc != orxNULL) && (_pstSrc != sstDisplay.pstScreen));
  orxASSERT(_pstTransform != orxNULL);

  /* Translates it */
  glTranslatef(_pstTransform->fDstX, _pstTransform->fDstY, 0.0f);
  glASSERT();

  /* Applies rotation */
  glRotatef(orxMATH_KF_RAD_TO_DEG * _pstTransform->fRotation, 0.0f, 0.0f, 1.0f);
  glASSERT();

  /* Applies scale */
  glScalef(_pstTransform->fScaleX, _pstTransform->fScaleY, 1.0f);
  glASSERT();

  /* Applies pivot translation */
  glTranslatef(-_pstTransform->fSrcX, -_pstTransform->fSrcY, 0.0f);
  glASSERT();

  /* No repeat? */
  if((_pstTransform->fRepeatX == orxFLOAT_1) && (_pstTransform->fRepeatY == orxFLOAT_1))
  {
    /* Draws it */
    orxDisplay_SDL_DrawBitmap(_pstSrc, _eSmoothing, _eBlendMode);
  }
  else
  {
    orxFLOAT  i, j, fRecRepeatX;
    GLfloat   fX, fY, fWidth, fHeight, fTop, fBottom, fLeft, fRight;
    orxU32    u32Counter;

    /* Prepares bitmap for drawing */
    orxDisplay_SDL_PrepareBitmap(_pstSrc, _eSmoothing, _eBlendMode);

    /* Inits bitmap height */
    fHeight = (GLfloat)((_pstSrc->stClip.vBR.fY - _pstSrc->stClip.vTL.fY) / _pstTransform->fRepeatY);

    /* Inits texture coords */
    fLeft   = _pstSrc->fRecRealWidth * _pstSrc->stClip.vTL.fX;
    fTop    = orxFLOAT_1 - _pstSrc->fRecRealHeight * _pstSrc->stClip.vTL.fY;

    /* For all lines */
    for(fY = 0.0f, i = _pstTransform->fRepeatY, u32Counter = 0, fRecRepeatX = orxFLOAT_1 / _pstTransform->fRepeatX; i > orxFLOAT_0; i -= orxFLOAT_1, fY += fHeight)
    {
      /* Partial line? */
      if(i < orxFLOAT_1)
      {
        /* Updates height */
        fHeight *= (GLfloat)i;

        /* Resets texture coords */
        fRight  = (GLfloat)(_pstSrc->fRecRealWidth * _pstSrc->stClip.vBR.fX);
        fBottom = (GLfloat)(orxFLOAT_1 - _pstSrc->fRecRealHeight * (_pstSrc->stClip.vTL.fY + (i * (_pstSrc->stClip.vBR.fY - _pstSrc->stClip.vTL.fY))));
      }
      else
      {
        /* Resets texture coords */
        fRight  = (GLfloat)(_pstSrc->fRecRealWidth * _pstSrc->stClip.vBR.fX);
        fBottom = (GLfloat)(orxFLOAT_1 - _pstSrc->fRecRealHeight * _pstSrc->stClip.vBR.fY);
      }

      /* Resets bitmap width */
      fWidth = (GLfloat)((_pstSrc->stClip.vBR.fX - _pstSrc->stClip.vTL.fX) * fRecRepeatX);

      /* For all columns */
      for(fX = 0.0f, j = _pstTransform->fRepeatX; j > orxFLOAT_0; j -= orxFLOAT_1, fX += fWidth)
      {
        /* End of buffer? */
        if(u32Counter > orxDISPLAY_KU32_BUFFER_SIZE - 12)
        {
          /* Selects arrays */
          glVertexPointer(2, GL_FLOAT, 0, sstDisplay.afVertexList);
          glASSERT();
          glTexCoordPointer(2, GL_FLOAT, 0, sstDisplay.afTextureCoordList);
          glASSERT();

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
                orxDisplay_SDL_InitShader(pstShader);

                /* Draws arrays */
                glDrawArrays(GL_TRIANGLE_STRIP, 0, u32Counter >> 1);
                glASSERT();
              }
            }
          }
          else
          {
            /* Draws arrays */
            glDrawArrays(GL_TRIANGLE_STRIP, 0, u32Counter >> 1);
            glASSERT();
          }

          /* Resets counter */
          u32Counter = 0;
        }

        /* Partial column? */
        if(j < orxFLOAT_1)
        {
          /* Updates width */
          fWidth *= (GLfloat)j;

          /* Updates texture right coord */
          fRight = (GLfloat)(_pstSrc->fRecRealWidth * (_pstSrc->stClip.vTL.fX + (j * (_pstSrc->stClip.vBR.fX - _pstSrc->stClip.vTL.fX))));
        }

        /* Outputs vertices and texture coordinates */
        sstDisplay.afVertexList[u32Counter]       =
        sstDisplay.afVertexList[u32Counter + 2]   =
        sstDisplay.afVertexList[u32Counter + 4]   = fX;
        sstDisplay.afVertexList[u32Counter + 6]   =
        sstDisplay.afVertexList[u32Counter + 8]   =
        sstDisplay.afVertexList[u32Counter + 10]  = fX + fWidth;
        sstDisplay.afVertexList[u32Counter + 5]   =
        sstDisplay.afVertexList[u32Counter + 9]   =
        sstDisplay.afVertexList[u32Counter + 11]  = fY;
        sstDisplay.afVertexList[u32Counter + 1]   =
        sstDisplay.afVertexList[u32Counter + 3]   =
        sstDisplay.afVertexList[u32Counter + 7]   = fY + fHeight;

        sstDisplay.afTextureCoordList[u32Counter]       =
        sstDisplay.afTextureCoordList[u32Counter + 2]   =
        sstDisplay.afTextureCoordList[u32Counter + 4]   = fLeft;
        sstDisplay.afTextureCoordList[u32Counter + 6]   =
        sstDisplay.afTextureCoordList[u32Counter + 8]   =
        sstDisplay.afTextureCoordList[u32Counter + 10]  = fRight;
        sstDisplay.afTextureCoordList[u32Counter + 5]   =
        sstDisplay.afTextureCoordList[u32Counter + 9]   =
        sstDisplay.afTextureCoordList[u32Counter + 11]  = fTop;
        sstDisplay.afTextureCoordList[u32Counter + 1]   =
        sstDisplay.afTextureCoordList[u32Counter + 3]   =
        sstDisplay.afTextureCoordList[u32Counter + 7]   = fBottom;

        /* Updates counter */
        u32Counter += 12;
      }
    }

    /* Has remaining data? */
    if(u32Counter != 0)
    {
      /* Selects arrays */
      glVertexPointer(2, GL_FLOAT, 0, sstDisplay.afVertexList);
      glASSERT();
      glTexCoordPointer(2, GL_FLOAT, 0, sstDisplay.afTextureCoordList);
      glASSERT();

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
            orxDisplay_SDL_InitShader(pstShader);

            /* Draws arrays */
            glDrawArrays(GL_TRIANGLE_STRIP, 0, u32Counter >> 1);
            glASSERT();
          }
        }
      }
      else
      {
        /* Draws arrays */
        glDrawArrays(GL_TRIANGLE_STRIP, 0, u32Counter >> 1);
        glASSERT();
      }
    }
  }

  /* Restores identity */
  glLoadIdentity();
  glASSERT();

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_SDL_SaveBitmap(const orxBITMAP *_pstBitmap, const orxSTRING _zFilename)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBitmap != orxNULL);
  orxASSERT(_zFilename != orxNULL);

  /* Screen capture? */
  if(_pstBitmap == sstDisplay.pstScreen)
  {
    int             iFormat;
    orxU32          u32Length;
    const orxCHAR  *zExtension;

    /* Gets file name's length */
    u32Length = orxString_GetLength(_zFilename);

    /* Gets extension */
    zExtension = (u32Length > 3) ? _zFilename + u32Length - 3 : orxSTRING_EMPTY;

    /* DDS? */
    if(orxString_ICompare(zExtension, "dds") == 0)
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

    /* Saves screenshot */
    SOIL_save_screenshot(_zFilename, iFormat, 0, 0, orxF2U(sstDisplay.pstScreen->fWidth), orxF2U(sstDisplay.pstScreen->fHeight));

    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Can't save bitmap tp <%s>: only screen can be saved to file.", _zFilename);

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

orxBITMAP *orxFASTCALL orxDisplay_SDL_LoadBitmap(const orxSTRING _zFilename)
{
  unsigned char  *pu8ImageData;
  GLuint          uiWidth, uiHeight, uiBytesPerPixel;
  orxBITMAP      *pstResult = orxNULL;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Loads image */
  pu8ImageData = SOIL_load_image(_zFilename, (int *)&uiWidth, (int *)&uiHeight, (int *)&uiBytesPerPixel, SOIL_LOAD_RGBA);

  /* Valid? */
  if(pu8ImageData != NULL)
  {
    /* Allocates bitmap */
    pstResult = (orxBITMAP *)orxBank_Allocate(sstDisplay.pstBitmapBank);

    /* Valid? */
    if(pstResult != orxNULL)
    {
      GLuint  i, uiSrcOffset, uiDstOffset, uiLineSize, uiRealLineSize;
      orxU8  *pu8ImageBuffer;
      GLuint  uiRealWidth, uiRealHeight;

      /* Gets its real size */
      uiRealWidth   = orxMath_GetNextPowerOfTwo(uiWidth);
      uiRealHeight  = orxMath_GetNextPowerOfTwo(uiHeight);
   
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
      orxColor_Set(&(pstResult->stColor), &orxVECTOR_WHITE, orxFLOAT_1);
      orxVector_Copy(&(pstResult->stClip.vTL), &orxVECTOR_0);
      orxVector_Set(&(pstResult->stClip.vBR), pstResult->fWidth, pstResult->fHeight, orxFLOAT_0);
 
      /* Allocates buffer */
      pu8ImageBuffer = (orxU8 *)orxMemory_Allocate(uiRealWidth * uiRealHeight * sizeof(orxRGBA), orxMEMORY_TYPE_VIDEO);

      /* Checks? */
      orxASSERT(pu8ImageBuffer != orxNULL);

      /* Gets line sizes */
      uiLineSize      = uiWidth * sizeof(orxRGBA);
      uiRealLineSize  = uiRealWidth * sizeof(orxRGBA);

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

      /* Creates new texture */
      glGenTextures(1, &pstResult->uiTexture);
      glASSERT();
      glBindTexture(GL_TEXTURE_2D, pstResult->uiTexture);
      glASSERT();
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, pstResult->u32RealWidth, pstResult->u32RealHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, pu8ImageBuffer);
      glASSERT();
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glASSERT();
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glASSERT();
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (pstResult->bSmoothing != orxFALSE) ? GL_LINEAR : GL_NEAREST);
      glASSERT();
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (pstResult->bSmoothing != orxFALSE) ? GL_LINEAR : GL_NEAREST);
      glASSERT();

      /* Frees image buffer */
      orxMemory_Free(pu8ImageBuffer);

      /* Pops config section */
      orxConfig_PopSection();
    }

    /* Deletes surface */
    SOIL_free_image_data(pu8ImageData);
  }

  /* Done! */
  return pstResult;
}

orxSTATUS orxFASTCALL orxDisplay_SDL_GetBitmapSize(const orxBITMAP *_pstBitmap, orxFLOAT *_pfWidth, orxFLOAT *_pfHeight)
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

orxSTATUS orxFASTCALL orxDisplay_SDL_GetScreenSize(orxFLOAT *_pfWidth, orxFLOAT *_pfHeight)
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

orxSTATUS orxFASTCALL orxDisplay_SDL_SetBitmapClipping(orxBITMAP *_pstBitmap, orxU32 _u32TLX, orxU32 _u32TLY, orxU32 _u32BRX, orxU32 _u32BRY)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBitmap != orxNULL);

  /* Screen? */
  if(_pstBitmap == sstDisplay.pstScreen)
  {
    /* Enables clipping */
    glEnable(GL_SCISSOR_TEST);
    glASSERT();

    /* Stores screen clipping */
    glScissor(_u32TLX, orxF2U(sstDisplay.pstScreen->fHeight) - _u32BRY, _u32BRX - _u32TLX, _u32BRY - _u32TLY);
    glASSERT();
  }

  /* Stores clip coords */
  orxVector_Set(&(_pstBitmap->stClip.vTL), orxU2F(_u32TLX), orxU2F(_u32TLY), orxFLOAT_0);
  orxVector_Set(&(_pstBitmap->stClip.vBR), orxU2F(_u32BRX), orxU2F(_u32BRY), orxFLOAT_0);

  /* Done! */
  return eResult;
}

orxU32 orxFASTCALL orxDisplay_SDL_GetVideoModeCounter()
{
  SDL_Rect  **apstModeList;
  orxU32      u32Result = 0;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Gets video mode list */
  apstModeList = SDL_ListModes(NULL, sstDisplay.u32SDLFlags | SDL_FULLSCREEN);

  /* Valid? */
  if((apstModeList != NULL) && (apstModeList != (SDL_Rect **)-1))
  {
    /* Updates result */
    for(; *apstModeList != NULL; apstModeList++, u32Result++);
  }

  /* Done! */
  return u32Result;
}

orxDISPLAY_VIDEO_MODE *orxFASTCALL orxDisplay_SDL_GetVideoMode(orxU32 _u32Index, orxDISPLAY_VIDEO_MODE *_pstVideoMode)
{
  orxDISPLAY_VIDEO_MODE *pstResult;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstVideoMode != orxNULL);

  /* Is index valid? */
  if(_u32Index < orxDisplay_SDL_GetVideoModeCounter())
  {
    SDL_Rect **apstModeList;

    /* Gets video mode list */
    apstModeList = SDL_ListModes(NULL, sstDisplay.u32SDLFlags | SDL_FULLSCREEN);

    /* Valid? */
    if((apstModeList != NULL) && (apstModeList != (SDL_Rect **)-1))
    {
      /* Stores info */
      _pstVideoMode->u32Width   = apstModeList[_u32Index]->w;
      _pstVideoMode->u32Height  = apstModeList[_u32Index]->h;
      _pstVideoMode->u32Depth   = SDL_GetVideoInfo()->vfmt->BitsPerPixel;

      /* Updates result */
      pstResult = _pstVideoMode;
    }
  }
  else
  {
    /* Updates result */
    pstResult = orxNULL;
  }

  /* Done! */
  return pstResult;
}

orxBOOL orxFASTCALL orxDisplay_SDL_IsVideoModeAvailable(const orxDISPLAY_VIDEO_MODE *_pstVideoMode)
{
  orxBOOL bResult = orxFALSE;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstVideoMode != orxNULL);

  /* Updates result */
  bResult = SDL_VideoModeOK(_pstVideoMode->u32Width, _pstVideoMode->u32Height, _pstVideoMode->u32Depth, sstDisplay.u32SDLFlags) ? orxTRUE : orxFALSE;

  /* Done! */
  return bResult;
}

orxSTATUS orxFASTCALL orxDisplay_SDL_SetVideoMode(const orxDISPLAY_VIDEO_MODE *_pstVideoMode)
{
  orxS32    s32BitmapCounter, s32ShaderCounter;
  orxU8   **aau8BufferArray = orxNULL;
  orxSTATUS eResult;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstVideoMode != orxNULL);

  /* Gets bitmap counter */
  s32BitmapCounter = (orxS32)orxBank_GetCounter(sstDisplay.pstBitmapBank) - 1;

  /* Valid? */
  if(s32BitmapCounter > 0)
  {
    orxBITMAP  *pstBitmap;
    orxU32      u32Index;

    /* Allocates buffer array */
    aau8BufferArray = (orxU8 **)orxMemory_Allocate(s32BitmapCounter * sizeof(orxU8 *), orxMEMORY_TYPE_MAIN);

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

  /* Gets shader counter */
  s32ShaderCounter = (orxS32)orxBank_GetCounter(sstDisplay.pstShaderBank);

  /* Valid? */
  if(s32ShaderCounter > 0)
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
      pstShader->hProgram = (GLhandleARB)orxHANDLE_UNDEFINED;
    }
  }

  /* Enables double buffer for OpenGL */
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);

  /* Pushes display section */
  orxConfig_PushSection(orxDISPLAY_KZ_CONFIG_SECTION);

  /* No decoration? */
  if((orxConfig_HasValue(orxDISPLAY_KZ_CONFIG_DECORATION) != orxFALSE)
  && (orxConfig_GetBool(orxDISPLAY_KZ_CONFIG_DECORATION) == orxFALSE))
  {
    /* Updates flags */
    orxFLAG_SET(sstDisplay.u32SDLFlags, SDL_NOFRAME, 0);
  }
  else
  {
    /* Updates flags */
    orxFLAG_SET(sstDisplay.u32SDLFlags, 0, SDL_NOFRAME);
  }

  /* Updates its title */
  SDL_WM_SetCaption(orxConfig_GetString(orxDISPLAY_KZ_CONFIG_TITLE), orxNULL);

  /* Pops config section */
  orxConfig_PopSection();

  /* Updates video mode */
  sstDisplay.pstScreenSurface = SDL_SetVideoMode(_pstVideoMode->u32Width, _pstVideoMode->u32Height, _pstVideoMode->u32Depth, sstDisplay.u32SDLFlags);

  /* Success? */
  if(sstDisplay.pstScreenSurface != NULL)
  {
    /* Inits it */
    glEnable(GL_TEXTURE_2D);
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
    glEnableClientState(GL_VERTEX_ARRAY);
    glASSERT();
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glASSERT();

    /* Updates screen info */
    sstDisplay.pstScreen->fWidth          = orx2F(_pstVideoMode->u32Width);
    sstDisplay.pstScreen->fHeight         = orx2F(_pstVideoMode->u32Height);
    sstDisplay.pstScreen->u32RealWidth    = orxMath_GetNextPowerOfTwo(_pstVideoMode->u32Width);
    sstDisplay.pstScreen->u32RealHeight   = orxMath_GetNextPowerOfTwo(_pstVideoMode->u32Height);
    sstDisplay.pstScreen->u32Depth        = _pstVideoMode->u32Depth;
    sstDisplay.pstScreen->bSmoothing      = sstDisplay.bDefaultSmoothing;
    sstDisplay.pstScreen->fRecRealWidth   = orxFLOAT_1 / orxU2F(sstDisplay.pstScreen->u32RealWidth);
    sstDisplay.pstScreen->fRecRealHeight  = orxFLOAT_1 / orxU2F(sstDisplay.pstScreen->u32RealHeight);
    orxVector_Copy(&(sstDisplay.pstScreen->stClip.vTL), &orxVECTOR_0);
    orxVector_Set(&(sstDisplay.pstScreen->stClip.vBR), sstDisplay.pstScreen->fWidth, sstDisplay.pstScreen->fHeight, orxFLOAT_0);

    /* Creates texture for screen backup */
    glGenTextures(1, &(sstDisplay.pstScreen->uiTexture));
    glASSERT();
    glBindTexture(GL_TEXTURE_2D, sstDisplay.pstScreen->uiTexture);
    glASSERT();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, sstDisplay.pstScreen->u32RealWidth, sstDisplay.pstScreen->u32RealHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
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

    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }
  else
  {
    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Had bitmaps? */
  if(s32BitmapCounter > 0)
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
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, pstBitmap->u32RealWidth, pstBitmap->u32RealHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, aau8BufferArray[u32Index]);
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
  if(s32ShaderCounter > 0)
  {
    orxDISPLAY_SHADER *pstShader;

    /* For all shaders */
    for(pstShader = (orxDISPLAY_SHADER *)orxBank_GetNext(sstDisplay.pstShaderBank, orxNULL);
        pstShader != orxNULL;
        pstShader = (orxDISPLAY_SHADER *)orxBank_GetNext(sstDisplay.pstShaderBank, pstShader))
    {
      /* Compiles it */
      orxDisplay_SDL_CompileShader(pstShader);
    }
  }

  /* Clears last blend mode & last bitmap */
  sstDisplay.eLastBlendMode = orxDISPLAY_BLEND_MODE_NUMBER;
  sstDisplay.pstLastBitmap  = orxNULL;

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_SDL_EnableVSync(orxBOOL _bEnable)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

#if defined(__orxWINDOWS__)

  static BOOL (WINAPI *pfnWGLSwapIntervalEXT)(int) = NULL;

  /* Not initialized? */
  if(pfnWGLSwapIntervalEXT == NULL)
  {
    /* Inits it */
    pfnWGLSwapIntervalEXT = (BOOL (WINAPI *)(int))SDL_GL_GetProcAddress("wglSwapIntervalEXT");

    /* Valid? */
    if(pfnWGLSwapIntervalEXT != NULL)
    {
      /* Updates VSync status */
      pfnWGLSwapIntervalEXT((_bEnable != orxFALSE) ? 1 : 0);
    }
  }
  else
  {
    /* Updates VSync status */
    pfnWGLSwapIntervalEXT((_bEnable != orxFALSE) ? 1 : 0);
  }

#elif defined(__orxLINUX__)

  static int (__stdcall *pfnglXSwapIntervalSGI)(int) = NULL;

  /* Not initialized? */
  if(pfnglXSwapIntervalSGI == NULL)
  {
    /* Inits it */
    pfnglXSwapIntervalSGI = (int (__stdcall *)(int))SDL_GL_GetProcAddress("glXSwapIntervalSGI");

    /* Valid? */
    if(pfnglXSwapIntervalSGI != NULL)
    {
      /* Updates VSync status */
      pfnglXSwapIntervalSGI((_bEnable != orxFALSE) ? 1 : 0);
    }
  }
  else
  {
    /* Updates VSync status */
    pfnglXSwapIntervalSGI((_bEnable != orxFALSE) ? 1 : 0);
  }

#endif

  /* Enable? */
  if(_bEnable != orxFALSE)
  {
    /* Enables vertical sync */
    SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, 1);

    /* Updates status */
    orxFLAG_SET(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_VSYNC, orxDISPLAY_KU32_STATIC_FLAG_NONE);
  }
  else
  {
    /* Disables vertical Sync */
    SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, 0);

    /* Updates status */
    orxFLAG_SET(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_NONE, orxDISPLAY_KU32_STATIC_FLAG_VSYNC);
  }

  /* Done! */
  return eResult;
}

orxBOOL orxFASTCALL orxDisplay_SDL_IsVSyncEnabled()
{
  orxBOOL bResult = orxFALSE;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Updates result */
  bResult = orxFLAG_TEST(sstDisplay.u32Flags, orxDISPLAY_KU32_STATIC_FLAG_VSYNC) ? orxTRUE : orxFALSE;

  /* Done! */
  return bResult;
}

orxSTATUS orxFASTCALL orxDisplay_SDL_SetFullScreen(orxBOOL _bFullScreen)
{
  orxBOOL   bUpdate = orxFALSE;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Enable? */
  if(_bFullScreen != orxFALSE)
  {
    /* Wasn't already full screen? */
    if(!orxFLAG_TEST(sstDisplay.u32SDLFlags, SDL_FULLSCREEN))
    {
      /* Updates window style */
      orxFLAG_SET(sstDisplay.u32SDLFlags, SDL_FULLSCREEN, 0);

      /* Asks for update */
      bUpdate = orxTRUE;
    }
  }
  else
  {
    /* Was full screen? */
    if(orxFLAG_TEST(sstDisplay.u32SDLFlags, SDL_FULLSCREEN))
    {
      /* Updates window style */
      orxFLAG_SET(sstDisplay.u32SDLFlags, 0, SDL_FULLSCREEN);

      /* Asks for update */
      bUpdate = orxTRUE;
    }
  }

  /* Should update? */
  if(bUpdate != orxFALSE)
  {
    orxDISPLAY_VIDEO_MODE stVideoMode;

    /* Inits video mode */
    stVideoMode.u32Width  = orxF2U(sstDisplay.pstScreen->fWidth);
    stVideoMode.u32Height = orxF2U(sstDisplay.pstScreen->fHeight);
    stVideoMode.u32Depth  = sstDisplay.pstScreen->u32Depth;

    /* Updates video mode */
    eResult = orxDisplay_SDL_SetVideoMode(&stVideoMode);

    /* Failed? */
    if(eResult == orxSTATUS_FAILURE)
    {
      /* Restores previous full screen status */
      orxFLAG_SWAP(sstDisplay.u32SDLFlags, SDL_FULLSCREEN);

      /* Updates video mode */
      orxDisplay_SDL_SetVideoMode(&stVideoMode);
    }
  }

  /* Done! */
  return eResult;
}

orxBOOL orxFASTCALL orxDisplay_SDL_IsFullScreen()
{
  orxBOOL bResult = orxFALSE;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Updates result */
  bResult = orxFLAG_TEST(sstDisplay.u32SDLFlags, SDL_FULLSCREEN) ? orxTRUE : orxFALSE;

  /* Done! */
  return bResult;
}

orxSTATUS orxFASTCALL orxDisplay_SDL_Init()
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Was not already initialized? */
  if(!(sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY))
  {
    /* Cleans static controller */
    orxMemory_Zero(&sstDisplay, sizeof(orxDISPLAY_STATIC));

    /* Is SDL partly initialized? */
    if(SDL_WasInit(SDL_INIT_EVERYTHING) != 0)
    {
      /* Inits the video subsystem */
      eResult = (SDL_InitSubSystem(SDL_INIT_VIDEO) == 0) ? orxSTATUS_SUCCESS : orxSTATUS_FAILURE;
    }
    else
    {
      /* Inits SDL with video */
      eResult = (SDL_Init(SDL_INIT_VIDEO) == 0) ? orxSTATUS_SUCCESS : orxSTATUS_FAILURE;
    }

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

        /* Pushes display section */
        orxConfig_PushSection(orxDISPLAY_KZ_CONFIG_SECTION);

        /* Gets resolution from config */
        stVideoMode.u32Width  = orxConfig_HasValue(orxDISPLAY_KZ_CONFIG_WIDTH) ? orxConfig_GetU32(orxDISPLAY_KZ_CONFIG_WIDTH) : orxDISPLAY_KU32_SCREEN_WIDTH;
        stVideoMode.u32Height = orxConfig_HasValue(orxDISPLAY_KZ_CONFIG_HEIGHT) ? orxConfig_GetU32(orxDISPLAY_KZ_CONFIG_HEIGHT) : orxDISPLAY_KU32_SCREEN_HEIGHT;
        stVideoMode.u32Depth  = orxConfig_HasValue(orxDISPLAY_KZ_CONFIG_DEPTH) ? orxConfig_GetU32(orxDISPLAY_KZ_CONFIG_DEPTH) : orxDISPLAY_KU32_SCREEN_DEPTH;

        /* Full screen? */
        if(orxConfig_GetBool(orxDISPLAY_KZ_CONFIG_FULLSCREEN) != orxFALSE)
        {
          /* Updates flags */
          sstDisplay.u32SDLFlags = SDL_OPENGL | SDL_FULLSCREEN;
        }
        /* No decoration? */
        else if((orxConfig_HasValue(orxDISPLAY_KZ_CONFIG_DECORATION) != orxFALSE)
             && (orxConfig_GetBool(orxDISPLAY_KZ_CONFIG_DECORATION) == orxFALSE))
        {
          /* Updates flags */
          sstDisplay.u32SDLFlags = SDL_OPENGL | SDL_NOFRAME;
        }
        else
        {
          /* Updates flags */
          sstDisplay.u32SDLFlags = SDL_OPENGL;
        }

        /* Sets module as ready */
        sstDisplay.u32Flags = orxDISPLAY_KU32_STATIC_FLAG_READY;

        /* Has VSync value? */
        if(orxConfig_HasValue(orxDISPLAY_KZ_CONFIG_VSYNC) != orxFALSE)
        {
          /* Updates vertical sync */
          orxDisplay_SDL_EnableVSync(orxConfig_GetBool(orxDISPLAY_KZ_CONFIG_VSYNC));
        }
        else
        {
          /* Enables vertical sync */
          orxDisplay_SDL_EnableVSync(orxTRUE);
        }

        /* Updates its title */
        SDL_WM_SetCaption(orxConfig_GetString(orxDISPLAY_KZ_CONFIG_TITLE), orxNULL);

        /* Allocates screen bitmap */
        sstDisplay.pstScreen = (orxBITMAP *)orxBank_Allocate(sstDisplay.pstBitmapBank);
        orxMemory_Zero(sstDisplay.pstScreen, sizeof(orxBITMAP));

        /* Sets video mode? */
        if((eResult = orxDisplay_SDL_SetVideoMode(&stVideoMode)) == orxSTATUS_FAILURE)
        {
          /* Updates display flags */
          sstDisplay.u32SDLFlags = SDL_OPENGL;

          /* Updates resolution */
          stVideoMode.u32Width  = orxDISPLAY_KU32_SCREEN_WIDTH;
          stVideoMode.u32Height = orxDISPLAY_KU32_SCREEN_HEIGHT;
          stVideoMode.u32Depth  = orxDISPLAY_KU32_SCREEN_DEPTH;

          /* Sets video mode using default parameters */
          eResult = orxDisplay_SDL_SetVideoMode(&stVideoMode);
        }

        /* Valid? */
        if(eResult != orxSTATUS_FAILURE)
        {
          orxCLOCK *pstClock;

          /* Inits info */
          sstDisplay.bDefaultSmoothing  = orxConfig_GetBool(orxDISPLAY_KZ_CONFIG_SMOOTH);
          sstDisplay.eLastBlendMode     = orxDISPLAY_BLEND_MODE_NUMBER;

          /* Gets clock */
          pstClock = orxClock_FindFirst(orx2F(-1.0f), orxCLOCK_TYPE_CORE);

          /* Valid? */
          if(pstClock != orxNULL)
          {
            /* Registers event update function */
            eResult = orxClock_Register(pstClock, orxDisplay_SDL_EventUpdate, orxNULL, orxMODULE_ID_DISPLAY, orxCLOCK_PRIORITY_HIGHEST);
          }

          /* Inits shader support */
          orxDisplay_SDL_InitShaderSupport();
          
          /* Shows mouse cursor */
          SDL_ShowCursor(orxTRUE);
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
          orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Failed to init SDL/OpenGL default video mode.");
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
      orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Failed to init SDL.");
    }
  }

  /* Done! */
  return eResult;
}

void orxFASTCALL orxDisplay_SDL_Exit()
{
  /* Was initialized? */
  if(sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY)
  {
    /* Is video the only subsystem initialized? */
    if(SDL_WasInit(SDL_INIT_EVERYTHING) == SDL_INIT_VIDEO)
    {
      /* Exits from SDL */
      SDL_Quit();
    }
    else
    {
      /* Exits from video subsystem */
      SDL_QuitSubSystem(SDL_INIT_VIDEO);
    }

    /* Deletes banks */
    orxBank_Delete(sstDisplay.pstBitmapBank);
    orxBank_Delete(sstDisplay.pstShaderBank);

    /* Cleans static controller */
    orxMemory_Zero(&sstDisplay, sizeof(orxDISPLAY_STATIC));
  }

  return;
}

orxHANDLE orxFASTCALL orxDisplay_SDL_CreateShader(const orxSTRING _zCode, const orxLINKLIST *_pstParamList)
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
            {
              orxS32 s32Offset;

              /* Adds its literal value */
              s32Offset = orxString_NPrint(pc, s32Free, "uniform float %s;\n", pstParam->zName);
              pc       += s32Offset;
              s32Free  -= s32Offset;

              break;
            }

            case orxSHADER_PARAM_TYPE_TEXTURE:
            {
              orxS32 s32Offset;

              /* Adds its literal value */
              s32Offset = orxString_NPrint(pc, s32Free, "uniform sampler2D %s;\n", pstParam->zName);
              pc       += s32Offset;
              s32Free  -= s32Offset;

              break;
            }

            case orxSHADER_PARAM_TYPE_VECTOR:
            {
              orxS32 s32Offset;

              /* Adds its literal value */
              s32Offset = orxString_NPrint(pc, s32Free, "uniform vec3 %s;\n", pstParam->zName);
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
        pstShader->hProgram               = (GLhandleARB)orxHANDLE_UNDEFINED;
        pstShader->iTextureCounter        = 0;
        pstShader->bActive                = orxFALSE;
        pstShader->bInitialized           = orxFALSE;
        pstShader->zCode                  = orxString_Duplicate(sstDisplay.acShaderCodeBuffer);
        pstShader->astTextureInfoList     = (orxDISPLAY_TEXTURE_INFO *)orxMemory_Allocate(sstDisplay.iTextureUnitNumber * sizeof(orxDISPLAY_TEXTURE_INFO), orxMEMORY_TYPE_MAIN);
        orxMemory_Zero(pstShader->astTextureInfoList, sstDisplay.iTextureUnitNumber * sizeof(orxDISPLAY_TEXTURE_INFO));

        /* Compiles it */
        if(orxDisplay_SDL_CompileShader(pstShader) != orxSTATUS_FAILURE)
        {
          /* Updates result */
          hResult = (orxHANDLE)pstShader;
        }
        else
        {
          /* Deletes code */
          orxString_Delete(pstShader->zCode);

          /* Deltes texture info list */
          orxMemory_Free(pstShader->astTextureInfoList);

          /* Frees shader */
          orxBank_Free(sstDisplay.pstShaderBank, pstShader);
        }
      }
    }
  }

  /* Done! */
  return hResult;
}

void orxFASTCALL orxDisplay_SDL_DeleteShader(orxHANDLE _hShader)
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

  /* Frees it */
  orxBank_Free(sstDisplay.pstShaderBank, pstShader);

  return;
}

orxSTATUS orxFASTCALL orxDisplay_SDL_StartShader(orxHANDLE _hShader)
{
  orxDISPLAY_SHADER  *pstShader;
  orxSTATUS           eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT((_hShader != orxHANDLE_UNDEFINED) && (_hShader != orxNULL));

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

orxSTATUS orxFASTCALL orxDisplay_SDL_StopShader(orxHANDLE _hShader)
{
  orxDISPLAY_SHADER  *pstShader;
  orxSTATUS           eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT((_hShader != orxHANDLE_UNDEFINED) && (_hShader != orxNULL));

  /* Gets shader */
  pstShader = (orxDISPLAY_SHADER *)_hShader;

  /* Wasn't initialized? */
  if(pstShader->bInitialized == orxFALSE)
  {
    /* Inits it */
    orxDisplay_SDL_InitShader(pstShader);

    /* Defines the vertex list */
    GLfloat afVertexList[] =
    {
      sstDisplay.pstScreen->stClip.vTL.fX, sstDisplay.pstScreen->stClip.vBR.fY,
      sstDisplay.pstScreen->stClip.vTL.fX, sstDisplay.pstScreen->stClip.vTL.fY,
      sstDisplay.pstScreen->stClip.vBR.fX, sstDisplay.pstScreen->stClip.vBR.fY,
      sstDisplay.pstScreen->stClip.vBR.fX, sstDisplay.pstScreen->stClip.vTL.fY
    };

    /* Defines the texture coord list */
    GLfloat afTextureCoordList[] =
    {
      (GLfloat)(sstDisplay.pstScreen->fRecRealWidth * sstDisplay.pstScreen->stClip.vTL.fX), (GLfloat)(orxFLOAT_1 - sstDisplay.pstScreen->fRecRealHeight * sstDisplay.pstScreen->stClip.vBR.fY),
      (GLfloat)(sstDisplay.pstScreen->fRecRealWidth * sstDisplay.pstScreen->stClip.vTL.fX), (GLfloat)(orxFLOAT_1 - sstDisplay.pstScreen->fRecRealHeight * sstDisplay.pstScreen->stClip.vTL.fY),
      (GLfloat)(sstDisplay.pstScreen->fRecRealWidth * sstDisplay.pstScreen->stClip.vBR.fX), (GLfloat)(orxFLOAT_1 - sstDisplay.pstScreen->fRecRealHeight * sstDisplay.pstScreen->stClip.vBR.fY),
      (GLfloat)(sstDisplay.pstScreen->fRecRealWidth * sstDisplay.pstScreen->stClip.vBR.fX), (GLfloat)(orxFLOAT_1 - sstDisplay.pstScreen->fRecRealHeight * sstDisplay.pstScreen->stClip.vTL.fY)
    };

    /* Selects arrays */
    glVertexPointer(2, GL_FLOAT, 0, afVertexList);
    glASSERT();
    glTexCoordPointer(2, GL_FLOAT, 0, afTextureCoordList);
    glASSERT();

    /* Draws arrays */
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glASSERT();
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

orxSTATUS orxFASTCALL orxDisplay_SDL_SetShaderBitmap(orxHANDLE _hShader, const orxSTRING _zParam, orxBITMAP *_pstValue)
{
  orxDISPLAY_SHADER  *pstShader;
  orxSTATUS           eResult;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT((_hShader != orxHANDLE_UNDEFINED) && (_hShader != orxNULL));
  orxASSERT(_zParam != orxNULL);
  orxASSERT(_pstValue != orxNULL);

  /* Gets shader */
  pstShader = (orxDISPLAY_SHADER *)_hShader;

  /* Has free texture unit left? */
  if(pstShader->iTextureCounter < sstDisplay.iTextureUnitNumber)
  {
    GLint iLocation;

    /* Gets parameter location */
    iLocation = glGetUniformLocationARB(pstShader->hProgram, (const GLcharARB *)_zParam);
    glASSERT();

    /* Valid? */
    if(iLocation != -1)
    {
      /* Updates texture info */
      pstShader->astTextureInfoList[pstShader->iTextureCounter].iLocation = iLocation;
      pstShader->astTextureInfoList[pstShader->iTextureCounter].pstBitmap = _pstValue;

      /* Updates texture counter */
      pstShader->iTextureCounter++;

      /* Updates result */
      eResult = orxSTATUS_SUCCESS;
    }
    else
    {
      /* Outputs log */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Can't find texture parameter <%s> for fragment shader.", _zParam);

      /* Updates result */
      eResult = orxSTATUS_FAILURE;
    }
  }
  else
  {
    /* Outputs log */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Can't bind texture parameter <%s> for fragment shader: all the texture units are used.", _zParam);

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_SDL_SetShaderFloat(orxHANDLE _hShader, const orxSTRING _zParam, orxFLOAT _fValue)
{
  orxDISPLAY_SHADER  *pstShader;
  GLint               iLocation;
  orxSTATUS           eResult;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT((_hShader != orxHANDLE_UNDEFINED) && (_hShader != orxNULL));
  orxASSERT(_zParam != orxNULL);

  /* Gets shader */
  pstShader = (orxDISPLAY_SHADER *)_hShader;

  /* Gets parameter location */
  iLocation = glGetUniformLocationARB(pstShader->hProgram, (const GLcharARB *)_zParam);
  glASSERT();

  /* Valid? */
  if(iLocation != -1)
  {
    /* Updates its value */
    glUniform1fARB(iLocation, (GLfloat)_fValue);
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

orxSTATUS orxFASTCALL orxDisplay_SDL_SetShaderVector(orxHANDLE _hShader, const orxSTRING _zParam, const orxVECTOR *_pvValue)
{
  orxDISPLAY_SHADER  *pstShader;
  GLint               iLocation;
  orxSTATUS           eResult;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT((_hShader != orxHANDLE_UNDEFINED) && (_hShader != orxNULL));
  orxASSERT(_zParam != orxNULL);
  orxASSERT(_pvValue != orxNULL);

  /* Gets shader */
  pstShader = (orxDISPLAY_SHADER *)_hShader;

  /* Gets parameter location */
  iLocation = glGetUniformLocationARB(pstShader->hProgram, (const GLcharARB *)_zParam);
  glASSERT();

  /* Valid? */
  if(iLocation != -1)
  {
    /* Updates its value */
    glUniform3fARB(iLocation, (GLfloat)_pvValue->fX, (GLfloat)_pvValue->fY, (GLfloat)_pvValue->fZ);
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
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SDL_Init, DISPLAY, INIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SDL_Exit, DISPLAY, EXIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SDL_Swap, DISPLAY, SWAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SDL_CreateBitmap, DISPLAY, CREATE_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SDL_DeleteBitmap, DISPLAY, DELETE_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SDL_SaveBitmap, DISPLAY, SAVE_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SDL_SetDestinationBitmap, DISPLAY, SET_DESTINATION_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SDL_TransformBitmap, DISPLAY, TRANSFORM_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SDL_TransformText, DISPLAY, TRANSFORM_TEXT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SDL_LoadBitmap, DISPLAY, LOAD_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SDL_GetBitmapSize, DISPLAY, GET_BITMAP_SIZE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SDL_GetScreenSize, DISPLAY, GET_SCREEN_SIZE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SDL_GetScreen, DISPLAY, GET_SCREEN_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SDL_ClearBitmap, DISPLAY, CLEAR_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SDL_SetBitmapClipping, DISPLAY, SET_BITMAP_CLIPPING);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SDL_BlitBitmap, DISPLAY, BLIT_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SDL_SetBitmapData, DISPLAY, SET_BITMAP_DATA);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SDL_SetBitmapColorKey, DISPLAY, SET_BITMAP_COLOR_KEY);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SDL_SetBitmapColor, DISPLAY, SET_BITMAP_COLOR);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SDL_GetBitmapColor, DISPLAY, GET_BITMAP_COLOR);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SDL_CreateShader, DISPLAY, CREATE_SHADER);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SDL_DeleteShader, DISPLAY, DELETE_SHADER);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SDL_StartShader, DISPLAY, START_SHADER);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SDL_StopShader, DISPLAY, STOP_SHADER);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SDL_SetShaderBitmap, DISPLAY, SET_SHADER_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SDL_SetShaderFloat, DISPLAY, SET_SHADER_FLOAT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SDL_SetShaderVector, DISPLAY, SET_SHADER_VECTOR);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SDL_EnableVSync, DISPLAY, ENABLE_VSYNC);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SDL_IsVSyncEnabled, DISPLAY, IS_VSYNC_ENABLED);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SDL_SetFullScreen, DISPLAY, SET_FULL_SCREEN);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SDL_IsFullScreen, DISPLAY, IS_FULL_SCREEN);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SDL_GetVideoModeCounter, DISPLAY, GET_VIDEO_MODE_COUNTER);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SDL_GetVideoMode, DISPLAY, GET_VIDEO_MODE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SDL_SetVideoMode, DISPLAY, SET_VIDEO_MODE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SDL_IsVideoModeAvailable, DISPLAY, IS_VIDEO_MODE_AVAILABLE);
orxPLUGIN_USER_CORE_FUNCTION_END();
