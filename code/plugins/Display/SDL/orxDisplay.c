/* Orx - Portable Game Engine
 *
 * Orx is the legal property of its developers, whose names
 * are listed in the COPYRIGHT file distributed
 * with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
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
#include <SDL_image.h>
#include <SDL_opengl.h>


/** Module flags
 */
#define orxDISPLAY_KU32_STATIC_FLAG_NONE        0x00000000 /**< No flags */

#define orxDISPLAY_KU32_STATIC_FLAG_READY       0x00000001 /**< Ready flag */
#define orxDISPLAY_KU32_STATIC_FLAG_VSYNC       0x00000002 /**< VSync flag */

#define orxDISPLAY_KU32_STATIC_MASK_ALL         0xFFFFFFFF /**< All mask */

#define orxDISPLAY_KU32_SCREEN_WIDTH            1024
#define orxDISPLAY_KU32_SCREEN_HEIGHT           768
#define orxDISPLAY_KU32_SCREEN_DEPTH            32

#define orxDISPLAY_KU32_BITMAP_BANK_SIZE        256

#define orxDISPLAY_KU32_BUFFER_SIZE             (12 * 1024)


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
  GLuint                uiTexture;
  orxBOOL               bSmoothing;
  orxFLOAT              fWidth, fHeight;
  orxU32                u32RealWidth, u32RealHeight, u32Depth;
  orxFLOAT              fRecRealWidth, fRecRealHeight;
  orxCOLOR              stColor;
  orxAABOX              stClip;
};

/** Static structure
 */
typedef struct __orxDISPLAY_STATIC_t
{
  orxBANK              *pstBitmapBank;
  orxBOOL               bDefaultSmoothing;
  SDL_Surface          *pstScreenSurface;
  orxBITMAP            *pstScreen;
  orxBITMAP            *pstDestinationBitmap;
  const orxBITMAP      *pstLastBitmap;
  orxDISPLAY_BLEND_MODE eLastBlendMode;
  orxU32                u32SDLFlags;
  orxU32                u32Flags;
  GLfloat               afVertexList[orxDISPLAY_KU32_BUFFER_SIZE];
  GLfloat               afTextureCoordList[orxDISPLAY_KU32_BUFFER_SIZE];


} orxDISPLAY_STATIC;


/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** Static data
 */
static orxDISPLAY_STATIC sstDisplay;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

static void orxFASTCALL orxDisplay_SDL_EventUpdate(const orxCLOCK_INFO *_pstClockInfo, void *_pContext)
{
  SDL_Event stSDLEvent;

  /* Clears event */
  orxMemory_Zero(&stSDLEvent, sizeof(SDL_Event));

  /* Clears wheel event */
  orxEVENT_SEND(orxEVENT_TYPE_FIRST_RESERVED + SDL_MOUSEBUTTONDOWN, SDL_MOUSEBUTTONDOWN, orxNULL, orxNULL, &stSDLEvent);

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

static orxINLINE void orxDisplay_SDL_PrepareBitmap(const orxBITMAP *_pstBitmap, orxDISPLAY_SMOOTHING _eSmoothing, orxDISPLAY_BLEND_MODE _eBlendMode)
{
  orxBOOL bSmoothing;

  /* Checks */
  orxASSERT((_pstBitmap != orxNULL) && (_pstBitmap != sstDisplay.pstScreen));

  /* New bitmap? */
  if(_pstBitmap != sstDisplay.pstLastBitmap)
  {
    /* Binds source's texture */
    glBindTexture(GL_TEXTURE_2D, _pstBitmap->uiTexture);
    glASSERT();

    /* Stores it */
    sstDisplay.pstLastBitmap = _pstBitmap;
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

  /* Done! */
  return;
}

static orxINLINE void orxDisplay_SDL_DrawBitmap(const orxBITMAP *_pstBitmap, orxDISPLAY_SMOOTHING _eSmoothing, orxDISPLAY_BLEND_MODE _eBlendMode)
{
  orxFLOAT fWidth, fHeight;

  /* Prepares bitmap for drawing */
  orxDisplay_SDL_PrepareBitmap(_pstBitmap, _eSmoothing, _eBlendMode);

  /* Gets bitmap working size */
  fWidth  = _pstBitmap->stClip.vBR.fX - _pstBitmap->stClip.vTL.fX;
  fHeight = _pstBitmap->stClip.vBR.fY - _pstBitmap->stClip.vTL.fY;

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
    _pstBitmap->fRecRealWidth * _pstBitmap->stClip.vTL.fX, orxFLOAT_1 - _pstBitmap->fRecRealHeight * (_pstBitmap->fHeight - _pstBitmap->stClip.vBR.fY),
    _pstBitmap->fRecRealWidth * _pstBitmap->stClip.vTL.fX, orxFLOAT_1 - _pstBitmap->fRecRealHeight * (_pstBitmap->fHeight - _pstBitmap->stClip.vTL.fY),
    _pstBitmap->fRecRealWidth * _pstBitmap->stClip.vBR.fX, orxFLOAT_1 - _pstBitmap->fRecRealHeight * (_pstBitmap->fHeight - _pstBitmap->stClip.vBR.fY),
    _pstBitmap->fRecRealWidth * _pstBitmap->stClip.vBR.fX, orxFLOAT_1 - _pstBitmap->fRecRealHeight * (_pstBitmap->fHeight - _pstBitmap->stClip.vTL.fY)
  };

  /* Renders it */
  glVertexPointer(2, GL_FLOAT, 0, afVertexList);
  glASSERT();
  glTexCoordPointer(2, GL_FLOAT, 0, afTextureCoordList);
  glASSERT();
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  glASSERT();

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
          sstDisplay.afTextureCoordList[u32Counter + 4]   = _pstFont->fRecRealWidth * _pstMap->astCharacterList[*pc].fX;
          sstDisplay.afTextureCoordList[u32Counter + 6]   =
          sstDisplay.afTextureCoordList[u32Counter + 8]   =
          sstDisplay.afTextureCoordList[u32Counter + 10]  = _pstFont->fRecRealWidth * (_pstMap->astCharacterList[*pc].fX + fWidth);
          sstDisplay.afTextureCoordList[u32Counter + 5]   =
          sstDisplay.afTextureCoordList[u32Counter + 9]   =
          sstDisplay.afTextureCoordList[u32Counter + 11]  = orxFLOAT_1 - _pstFont->fRecRealHeight * (_pstFont->fHeight - _pstMap->astCharacterList[*pc].fY);
          sstDisplay.afTextureCoordList[u32Counter + 1]   =
          sstDisplay.afTextureCoordList[u32Counter + 3]   =
          sstDisplay.afTextureCoordList[u32Counter + 7]   = orxFLOAT_1 - _pstFont->fRecRealHeight * (_pstFont->fHeight - _pstMap->astCharacterList[*pc].fY - fHeight);
          
          /* Updates counter */
          u32Counter += 12;

          /* End of buffer? */
          if(u32Counter > orxDISPLAY_KU32_BUFFER_SIZE - 12)
          {
            /* Renders them */
            glVertexPointer(2, GL_FLOAT, 0, sstDisplay.afVertexList);
            glASSERT();
            glTexCoordPointer(2, GL_FLOAT, 0, sstDisplay.afTextureCoordList);
            glASSERT();
            glDrawArrays(GL_TRIANGLE_STRIP, 0, u32Counter >> 1);
            glASSERT();
          }
        }
      }

      /* Updates X position */
      fX += fWidth;
    }
  }

  /* Renders last data */
  glVertexPointer(2, GL_FLOAT, 0, sstDisplay.afVertexList);
  glASSERT();
  glTexCoordPointer(2, GL_FLOAT, 0, sstDisplay.afTextureCoordList);
  glASSERT();
  glDrawArrays(GL_TRIANGLE_STRIP, 0, u32Counter >> 1);
  glASSERT();

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
  if((_pstBitmap != sstDisplay.pstScreen) && (_u32ByteNumber == u32Width * u32Height * 4))
  {
    orxU8        *au8Buffer, *pu8Dst;
    const orxU8  *pu8Src;
    orxU32        i, u32Offset;

    /* Allocates buffer */
    au8Buffer = (orxU8 *)orxMemory_Allocate(_pstBitmap->u32RealWidth * _pstBitmap->u32RealHeight * 4 * sizeof(orxU8), orxMEMORY_TYPE_MAIN);

    /* Checks */
    orxASSERT(au8Buffer != orxNULL);

    /* For all visible rows */
    for(i = 0, u32Offset = (_pstBitmap->u32RealWidth - u32Width) * 4, pu8Src = _au8Data, pu8Dst = au8Buffer + ((_pstBitmap->u32RealHeight - u32Height) * _pstBitmap->u32RealWidth * 4); i < u32Height; i++)
    {
      orxU32 j;

      /* For all visible columns */
      for(j = 0; j < u32Width; j++, pu8Src += 4, pu8Dst += 4)
      {
        /* Copies pixel data */
        pu8Dst[0] = pu8Src[0];
        pu8Dst[1] = pu8Src[1];
        pu8Dst[2] = pu8Src[2];
        pu8Dst[3] = pu8Src[3];
      }

      /* Skips unvisible pixels */
      pu8Dst += u32Offset;
    }

    /* Binds texture */
    glBindTexture(GL_TEXTURE_2D, _pstBitmap->uiTexture);
    glASSERT();

    /* Updates its content */
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, _pstBitmap->u32RealWidth, _pstBitmap->u32RealHeight, GL_RGBA, GL_UNSIGNED_BYTE, au8Buffer);
    glASSERT();

    /* Frees buffer */
    orxMemory_Free(au8Buffer);

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

  /* Draws it */
  orxDisplay_SDL_DrawBitmap(_pstSrc, _eSmoothing, _eBlendMode);

  /* Restores identity */
  glLoadIdentity();
  glASSERT();

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_SDL_SaveBitmap(const orxBITMAP *_pstBitmap, const orxSTRING _zFilename)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  //! TODO
  /* Not available */
  orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Not available on this platform!");

  /* Done! */
  return eResult;
}

orxBITMAP *orxFASTCALL orxDisplay_SDL_LoadBitmap(const orxSTRING _zFilename)
{
  SDL_Surface  *pstSurface;
  orxBITMAP    *pstResult = orxNULL;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Loads image */
  pstSurface = IMG_Load(_zFilename);

  /* Valid? */
  if(pstSurface != NULL)
  {
    /* Allocates bitmap */
    pstResult = (orxBITMAP *)orxBank_Allocate(sstDisplay.pstBitmapBank);

    /* Valid? */
    if(pstResult != orxNULL)
    {
      GLuint  uiWidth, uiHeight, uiRealWidth, uiRealHeight;
      GLenum  eFormat;

      /* Gets image's size */
      uiWidth   = pstSurface->w;
      uiHeight  = pstSurface->h;

      /* Gets its real size */
      uiRealWidth   = orxMath_GetNextPowerOfTwo(uiWidth);
      uiRealHeight  = orxMath_GetNextPowerOfTwo(uiHeight);
   
      /* Depending on the number of channels */
      switch(pstSurface->format->BytesPerPixel)
      {
        case 3:
        {
          /* Updates format */
          eFormat = (pstSurface->format->Rmask == 0x000000FF) ? GL_RGB : GL_BGR;

          break;
        }

        case 4:
        {
          /* Updates format */
          eFormat = (pstSurface->format->Rmask == 0x000000FF) ? GL_RGBA : GL_BGRA;

          break;
        }

        default:
        {
          /* Updates format */
          eFormat = GL_INVALID_ENUM;

          break;
        }
      }

      /* Valid? */
      if(eFormat != GL_INVALID_ENUM)
      {
        orxU8  *pu8ImageBuffer;
        orxBOOL bNPOT = orxFALSE;

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
   
        /* NPOT texture? */
        if((uiRealWidth != uiWidth) || (uiRealHeight != uiHeight))
        {
          GLuint i, uiSrcOffset, uiDstOffset, uiLineSize, uiRealLineSize;

          /* Allocates buffer */
          pu8ImageBuffer = (orxU8 *)orxMemory_Allocate(uiRealWidth * uiRealHeight * sizeof(GLuint), orxMEMORY_TYPE_VIDEO);

          /* Checks? */
          orxASSERT(pu8ImageBuffer != orxNULL);

          /* Gets line sizes */
          uiLineSize      = uiWidth * sizeof(GLuint);
          uiRealLineSize  = uiRealWidth * sizeof(GLuint);

          /* Adds padding */
          orxMemory_Zero(pu8ImageBuffer, uiRealLineSize * (uiRealHeight - uiHeight));

          /* For all lines */
          for(i = 0, uiSrcOffset = 0, uiDstOffset = uiRealLineSize * (uiRealHeight - uiHeight);
              i < uiHeight;
              i++, uiSrcOffset += uiLineSize, uiDstOffset += uiRealLineSize)
          {
            /* Copies data */
            orxMemory_Copy(pu8ImageBuffer + uiDstOffset, (orxU8 *)pstSurface->pixels + uiSrcOffset, uiLineSize);

            /* Adds padding */
            orxMemory_Zero(pu8ImageBuffer + uiDstOffset + uiLineSize, uiRealLineSize - uiLineSize);
          }

          /* Updates status */
          bNPOT = orxTRUE;
        }
        else
        {
          /* Uses current buffer */
          pu8ImageBuffer = (orxU8 *)pstSurface->pixels;
        }

        /* Creates new texture */
        glGenTextures(1, &pstResult->uiTexture);
        glASSERT();
        glBindTexture(GL_TEXTURE_2D, pstResult->uiTexture);
        glASSERT();
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, pstResult->u32RealWidth, pstResult->u32RealHeight, 0, eFormat, GL_UNSIGNED_BYTE, pu8ImageBuffer);
        glASSERT();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glASSERT();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glASSERT();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (pstResult->bSmoothing != orxFALSE) ? GL_LINEAR : GL_NEAREST);
        glASSERT();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (pstResult->bSmoothing != orxFALSE) ? GL_LINEAR : GL_NEAREST);
        glASSERT();

        /* NPOT texture? */
        if(bNPOT != orxFALSE)
        {
          /* Frees image buffer */
          orxMemory_Free(pu8ImageBuffer);
        }

        /* Pops config section */
        orxConfig_PopSection();
      }
    }

    /* Deletes surface */
    SDL_FreeSurface(pstSurface);
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
  orxU32 u32Result = 0;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  //! TODO
  /* Not yet implemented */
  orxLOG("Not implemented yet!");

  /* Done! */
  return u32Result;
}

orxDISPLAY_VIDEO_MODE *orxFASTCALL orxDisplay_SDL_GetVideoMode(orxU32 _u32Index, orxDISPLAY_VIDEO_MODE *_pstVideoMode)
{
  orxDISPLAY_VIDEO_MODE *pstResult = orxNULL;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  //! TODO
  /* Not yet implemented */
  orxLOG("Not implemented yet!");

  /* Done! */
  return pstResult;
}

orxSTATUS orxFASTCALL orxDisplay_SDL_SetVideoMode(const orxDISPLAY_VIDEO_MODE *_pstVideoMode)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Enables double buffer for OpenGL */
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 32);
  SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);

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

    /* Has destination bitmap? */
    if(sstDisplay.pstDestinationBitmap != orxNULL)
    {
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

orxBOOL orxFASTCALL orxDisplay_SDL_IsVideoModeAvailable(const orxDISPLAY_VIDEO_MODE *_pstVideoMode)
{
  orxBOOL bResult = orxFALSE;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  //! TODO
  /* Not yet implemented */
  orxLOG("Not implemented yet!");

  /* Done! */
  return bResult;
}

orxSTATUS orxFASTCALL orxDisplay_SDL_EnableVSync(orxBOOL _bEnable)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

#ifdef __orxWINDOWS__

  static BOOL (WINAPI *pfnWGLSwapIntervalEXT)(int) = NULL;

  /* Not initialized? */
  if(pfnWGLSwapIntervalEXT == NULL)
  {
    /* Inits it */
    pfnWGLSwapIntervalEXT = (BOOL (WINAPI *)(int))wglGetProcAddress("wglSwapIntervalEXT");
  }

  /* Valid? */
  if(pfnWGLSwapIntervalEXT != NULL)
  {
    /* Updates VSync status */
    pfnWGLSwapIntervalEXT((_bEnable != orxFALSE) ? 1 : 0);
  }

#endif /* __orxWINDOWS__ */

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
    stVideoMode.u32Width  = sstDisplay.pstScreen->u32RealWidth;
    stVideoMode.u32Height = sstDisplay.pstScreen->u32RealHeight;
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
      /* Creates bitmap bank */
      sstDisplay.pstBitmapBank = orxBank_Create(orxDISPLAY_KU32_BITMAP_BANK_SIZE, sizeof(orxBITMAP), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

      /* Valid? */
      if(sstDisplay.pstBitmapBank != orxNULL)
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

          /* Inits screen info */
          sstDisplay.bDefaultSmoothing        = orxConfig_GetBool(orxDISPLAY_KZ_CONFIG_SMOOTH);
          sstDisplay.pstScreen                = (orxBITMAP *)orxBank_Allocate(sstDisplay.pstBitmapBank);
          orxMemory_Zero(sstDisplay.pstScreen, sizeof(orxBITMAP));
          sstDisplay.pstScreen->fWidth        = orx2F(stVideoMode.u32Width);
          sstDisplay.pstScreen->fHeight       = orx2F(stVideoMode.u32Height);
          sstDisplay.pstScreen->u32RealWidth  = stVideoMode.u32Width;
          sstDisplay.pstScreen->u32RealHeight = stVideoMode.u32Height;
          sstDisplay.pstScreen->u32Depth      = stVideoMode.u32Depth;
          orxVector_Copy(&(sstDisplay.pstScreen->stClip.vTL), &orxVECTOR_0);
          orxVector_Set(&(sstDisplay.pstScreen->stClip.vBR), sstDisplay.pstScreen->fWidth, sstDisplay.pstScreen->fHeight, orxFLOAT_0);
          sstDisplay.eLastBlendMode           = orxDISPLAY_BLEND_MODE_NUMBER;

          /* Gets clock */
          pstClock = orxClock_FindFirst(orx2F(-1.0f), orxCLOCK_TYPE_CORE);

          /* Valid? */
          if(pstClock != orxNULL)
          {
            /* Registers event update function */
            eResult = orxClock_Register(pstClock, orxDisplay_SDL_EventUpdate, orxNULL, orxMODULE_ID_DISPLAY, orxCLOCK_PRIORITY_HIGHEST);
          }

          /* Shows mouse cursor */
          SDL_ShowCursor(orxTRUE);
        }
        else
        {
          /* Deletes bitmap bank */
          orxBank_Delete(sstDisplay.pstBitmapBank);
          sstDisplay.pstBitmapBank = orxNULL;

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
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Failed to create bitmap bank.");
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

    /* Deletes bank */
    orxBank_Delete(sstDisplay.pstBitmapBank);

    /* Cleans static controller */
    orxMemory_Zero(&sstDisplay, sizeof(orxDISPLAY_STATIC));
  }

  return;
}

orxHANDLE orxFASTCALL orxDisplay_SDL_CreateShader(const orxSTRING _zCode, const orxLINKLIST *_pstParamList)
{
  orxHANDLE hResult = orxHANDLE_UNDEFINED;

  //! TODO
  /* Not yet implemented */
  orxLOG("Not implemented yet!");

  /* Done! */
  return hResult;
}

void orxFASTCALL orxDisplay_SDL_DeleteShader(orxHANDLE _hShader)
{
  //! TODO
  /* Not yet implemented */
  orxLOG("Not implemented yet!");
}

orxSTATUS orxFASTCALL orxDisplay_SDL_RenderShader(orxHANDLE _hShader)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  //! TODO
  /* Not yet implemented */
  orxLOG("Not implemented yet!");

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_SDL_SetShaderBitmap(orxHANDLE _hShader, const orxSTRING _zParam, orxBITMAP *_pstValue)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  //! TODO
  /* Not yet implemented */
  orxLOG("Not implemented yet!");

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_SDL_SetShaderFloat(orxHANDLE _hShader, const orxSTRING _zParam, orxFLOAT _fValue)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  //! TODO
  /* Not yet implemented */
  orxLOG("Not implemented yet!");

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxDisplay_SDL_SetShaderVector(orxHANDLE _hShader, const orxSTRING _zParam, const orxVECTOR *_pvValue)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  //! TODO
  /* Not yet implemented */
  orxLOG("Not implemented yet!");

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
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SDL_RenderShader, DISPLAY, RENDER_SHADER);
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
