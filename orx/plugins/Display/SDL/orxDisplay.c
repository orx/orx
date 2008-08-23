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
 * @todo
 *
 * SDML display plugin implementation
 *
 */


#include "orxInclude.h"

#include "core/orxConfig.h"
#include "core/orxEvent.h"
#include "core/orxSystem.h"
#include "math/orxMath.h"
#include "plugin/orxPluginUser.h"

#include "display/orxDisplay.h"


#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_rotozoom.h>


/** Module flags
 */
#define orxDISPLAY_KU32_STATIC_FLAG_NONE        0x00000000 /**< No flags */

#define orxDISPLAY_KU32_STATIC_FLAG_READY       0x00000001 /**< Ready flag */

#define orxDISPLAY_KU32_STATIC_MASK_ALL         0xFFFFFFFF /**< All mask */

#ifdef __orxGP2X__

#define orxDISPLAY_KU32_SCREEN_WIDTH            320
#define orxDISPLAY_KU32_SCREEN_HEIGHT           240
#define orxDISPLAY_KU32_SCREEN_DEPTH            16

#else /* __orxGP2X__ */

#define orxDISPLAY_KU32_SCREEN_WIDTH            1024
#define orxDISPLAY_KU32_SCREEN_HEIGHT           768
#define orxDISPLAY_KU32_SCREEN_DEPTH            32

#endif /* __orxGP2X__ */


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Static structure
 */
typedef struct __orxDISPLAY_STATIC_t
{
  orxU32            u32Flags;
  orxFLOAT          fScreenWidth, fScreenHeight;
  SDL_Surface *     pstScreen;

} orxDISPLAY_STATIC;


/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** Static data
 */
orxSTATIC orxDISPLAY_STATIC sstDisplay;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

orxBITMAP *orxDisplay_SDL_GetScreen()
{
  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Done! */
  return((orxBITMAP *)sstDisplay.pstScreen);
}

orxSTATUS orxDisplay_SDL_DrawText(orxCONST orxBITMAP *_pstBitmap, orxCONST orxBITMAP_TRANSFORM *_pstTransform, orxRGBA _stColor, orxCONST orxSTRING _zString)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBitmap != orxNULL);
  orxASSERT(_pstTransform != orxNULL);
  orxASSERT(_zString != orxNULL);

  /* TODO :
   * Write the string onto screen, using char per char pixel writing
   */

  /* Not yet implemented */
  orxLOG("Not yet implemented!");

  /* Done! */
  return eResult;
}

orxVOID orxDisplay_SDL_DeleteBitmap(orxBITMAP *_pstBitmap)
{
  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBitmap != orxNULL);

  /* Deletes it */
  SDL_FreeSurface((SDL_Surface *)_pstBitmap);

  return;
}

orxBITMAP *orxDisplay_SDL_CreateBitmap(orxU32 _u32Width, orxU32 _u32Height)
{
  orxU32 u32RMask, u32GMask, u32BMask, u32AMask;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* SDL interprets each pixel as a 32-bit number, so our masks must depend
     on the endianness (byte order) of the machine */
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
  u32RMask = 0xFF000000;
  u32GMask = 0x00FF0000;
  u32BMask = 0x0000FF00;
  u32AMask = 0x000000FF;
#else
  u32RMask = 0x000000FF;
  u32GMask = 0x0000FF00;
  u32BMask = 0x00FF0000;
  u32AMask = 0xFF000000;
#endif

  return((orxBITMAP *)SDL_CreateRGBSurface(SDL_HWSURFACE, _u32Width, _u32Height, sstDisplay.pstScreen->format->BitsPerPixel, u32RMask, u32GMask, u32BMask, u32AMask));
}

orxSTATUS orxDisplay_SDL_ClearBitmap(orxBITMAP *_pstBitmap, orxRGBA _stColor)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBitmap != orxNULL);

  /* Updates result */
  eResult = (SDL_FillRect((SDL_Surface *)_pstBitmap, NULL, _stColor) == 0)
            ? orxSTATUS_SUCCESS
            : orxSTATUS_FAILURE;

  /* Done! */
  return eResult;
}

orxSTATUS orxDisplay_SDL_Swap()
{
  SDL_Event stSDLEvent;
  orxSTATUS eResult;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Updates result */
  eResult = (SDL_Flip(sstDisplay.pstScreen) == 0) ? orxSTATUS_SUCCESS : orxSTATUS_FAILURE;

  /* Handles all pending events */
  while(SDL_PollEvent(&stSDLEvent))
  {
    /* Depending on type */
    switch(stSDLEvent.type)
    {
      /* Closing? */
      case SDL_QUIT:
      {
        orxEVENT stEvent;

        /* Inits event */
        orxMemory_Zero(&stEvent, sizeof(orxEVENT));
        stEvent.eType = orxEVENT_TYPE_SYSTEM;
        stEvent.eID   = orxSYSTEM_EVENT_CLOSE;

        /* Sends system close event */
        orxEvent_Send(&stEvent);

        break;
      }

      /* Gained/Lost focus? */
      case SDL_ACTIVEEVENT:
      {
        orxEVENT stEvent;

        /* Inits event */
        orxMemory_Zero(&stEvent, sizeof(orxEVENT));
        stEvent.eType = orxEVENT_TYPE_SYSTEM;
        stEvent.eID   = (stSDLEvent.active.gain) ? orxSYSTEM_EVENT_FOCUS_GAINED : orxSYSTEM_EVENT_FOCUS_LOST;

        /* Sends system focus gained event */
        orxEvent_Send(&stEvent);

        break;
      }

      /* Key pressed? */
      case SDL_KEYDOWN:
      {
        /* Depending on key */
        switch(stSDLEvent.key.keysym.sym)
        {
          /* Escape */
          case SDLK_ESCAPE:
          {
            orxEVENT stEvent;

            /* Inits event */
            orxMemory_Zero(&stEvent, sizeof(orxEVENT));
            stEvent.eType = orxEVENT_TYPE_SYSTEM;
            stEvent.eID   = orxSYSTEM_EVENT_CLOSE;

            /* Sends system close event */
            orxEvent_Send(&stEvent);

            break;
          }

          default:
          {
            break;
          }
        }
      }

      default:
      {
        break;
      }
    }
  }

  /* Done! */
  return eResult;
}

orxSTATUS orxDisplay_SDL_SetBitmapColorKey(orxBITMAP *_pstBitmap, orxRGBA _stColor, orxBOOL _bEnable)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT((_pstBitmap != orxNULL) && (_pstBitmap != (orxBITMAP *)sstDisplay.pstScreen));

  /* Enable? */
  if(_bEnable != orxFALSE)
  {
    /* Updates result */
    eResult = (SDL_SetColorKey((SDL_Surface *)_pstBitmap, SDL_SRCCOLORKEY | SDL_RLEACCEL, SDL_MapRGB(((SDL_Surface *)_pstBitmap)->format, orxRGBA_R(_stColor), orxRGBA_G(_stColor), orxRGBA_B(_stColor))) == 0)
              ? orxSTATUS_SUCCESS
              : orxSTATUS_FAILURE;
  }
  else
  {
    /* Updates result */
    eResult = (SDL_SetColorKey((SDL_Surface *)_pstBitmap, 0, 0) == 0)
              ? orxSTATUS_SUCCESS
              : orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

orxSTATUS orxDisplay_SDL_SetBitmapColor(orxBITMAP *_pstBitmap, orxRGBA _stColor)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT((_pstBitmap != orxNULL) && (_pstBitmap != (orxBITMAP *)sstDisplay.pstScreen));

  /* Not yet implemented */
  orxLOG("Not yet implemented!");

  /* Done! */
  return eResult;
}

orxRGBA orxDisplay_SDL_GetBitmapColor(orxCONST orxBITMAP *_pstBitmap)
{
  orxRGBA stResult = 0;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT((_pstBitmap != orxNULL) && (_pstBitmap != (orxBITMAP *)sstDisplay.pstScreen));

  /* Not yet implemented */
  orxLOG("Not yet implemented!");

  /* Done! */
  return stResult;
}

orxSTATUS orxDisplay_SDL_BlitBitmap(orxBITMAP *_pstDst, orxCONST orxBITMAP *_pstSrc, orxCONST orxFLOAT _fPosX, orxFLOAT _fPosY)
{
  SDL_Rect  stSrcRect, stDstRect;
  orxSTATUS eResult;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstDst != orxNULL);
  orxASSERT((_pstSrc != orxNULL) && (_pstSrc != (orxBITMAP *)sstDisplay.pstScreen));

  /* Inits blitting rectangles */
  stSrcRect.x = 0;
  stSrcRect.y = 0;
  stSrcRect.w = ((SDL_Surface *)_pstSrc)->w;
  stSrcRect.h = ((SDL_Surface *)_pstSrc)->h;
  stDstRect.x = orxF2U(_fPosX);
  stDstRect.y = orxF2U(_fPosY);
  stDstRect.w = 0;
  stDstRect.h = 0;

  /* Updates result */
  eResult = (SDL_BlitSurface((SDL_Surface *)_pstSrc, &stSrcRect, (SDL_Surface *)_pstDst, &stDstRect) == 0)
            ? orxSTATUS_SUCCESS
            : orxSTATUS_FAILURE;

  /* Done! */
  return eResult;
}

orxSTATUS orxDisplay_SDL_TransformBitmap(orxBITMAP *_pstDst, orxCONST orxBITMAP *_pstSrc, orxCONST orxBITMAP_TRANSFORM *_pstTransform, orxDISPLAY_SMOOTHING _eSmoothing)
{
  SDL_Surface  *pstSurface;
  orxSTATUS     eResult;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstDst != orxNULL);
  orxASSERT((_pstSrc != orxNULL) && (_pstSrc != (orxBITMAP *)sstDisplay.pstScreen));
  orxASSERT(_pstTransform != orxNULL);

  /* Creates transformed surface */
  pstSurface = rotozoomSurface((SDL_Surface *)_pstSrc, _pstTransform->fRotation, _pstTransform->fScaleX, 0);

  /* Valid? */
  if(pstSurface != orxNULL)
  {
    SDL_Rect stSrcRect, stDstRect;

    /* Inits blitting rectangles */
    stSrcRect.x = orxF2U(_pstTransform->fSrcX);
    stSrcRect.y = orxF2U(_pstTransform->fSrcX);
    stSrcRect.w = ((SDL_Surface *)_pstSrc)->w;
    stSrcRect.h = ((SDL_Surface *)_pstSrc)->h;
    stDstRect.x = orxF2U(_pstTransform->fDstX);
    stDstRect.y = orxF2U(_pstTransform->fDstX);
    stDstRect.w = 0;
    stDstRect.h = 0;

    /* Updates result */
    eResult = (SDL_BlitSurface(pstSurface, &stSrcRect, (SDL_Surface *)_pstDst, &stDstRect) == 0)
              ? orxSTATUS_SUCCESS
              : orxSTATUS_FAILURE;

    /* Deletes transformed surface */
    SDL_FreeSurface(pstSurface);
  }
  else
  {
    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

orxSTATUS orxDisplay_SDL_SaveBitmap(orxCONST orxBITMAP *_pstBitmap, orxCONST orxSTRING _zFilename)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBitmap != orxNULL);
  orxASSERT(_zFilename != orxNULL);

  /* Updates result */
  eResult = (SDL_SaveBMP((SDL_Surface *)_pstBitmap, _zFilename) == 0) ? orxSTATUS_SUCCESS : orxSTATUS_FAILURE;

  /* Done! */
  return eResult;
}

orxBITMAP *orxDisplay_SDL_LoadBitmap(orxCONST orxSTRING _zFilename)
{
  orxBITMAP *pstResult = orxNULL;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Loads image */
  pstResult = (orxBITMAP *)IMG_Load(_zFilename);

  /* Done! */
  return pstResult;
}

orxSTATUS orxDisplay_SDL_GetBitmapSize(orxCONST orxBITMAP *_pstBitmap, orxFLOAT *_pfWidth, orxFLOAT *_pfHeight)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBitmap != orxNULL);
  orxASSERT(_pfWidth != orxNULL);
  orxASSERT(_pfHeight != orxNULL);

  /* Gets size */
  *_pfWidth   = orxS2F(((SDL_Surface *)_pstBitmap)->w);
  *_pfHeight  = orxS2F(((SDL_Surface *)_pstBitmap)->h);

  /* Done! */
  return eResult;
}

orxSTATUS orxDisplay_SDL_GetScreenSize(orxFLOAT *_pfWidth, orxFLOAT *_pfHeight)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pfWidth != orxNULL);
  orxASSERT(_pfHeight != orxNULL);

  /* Gets size */
  *_pfWidth   = sstDisplay.fScreenWidth;
  *_pfHeight  = sstDisplay.fScreenHeight;

  /* Done! */
  return eResult;
}

orxSTATUS orxDisplay_SDL_SetBitmapClipping(orxBITMAP *_pstBitmap, orxU32 _u32TLX, orxU32 _u32TLY, orxU32 _u32BRX, orxU32 _u32BRY)
{
  SDL_Rect  stClipRect;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBitmap != orxNULL);

  /* Gets SDL clip rectangle */
  stClipRect.x = _u32TLX;
  stClipRect.y = _u32TLY;
  stClipRect.w = _u32BRX - _u32TLX;
  stClipRect.h = _u32BRY - _u32TLY;

  /* Applies it */
  SDL_SetClipRect((SDL_Surface *)_pstBitmap, &stClipRect);

  /* Done! */
  return eResult;
}

orxSTATUS orxDisplay_SDL_EnableVSync(orxBOOL _bEnable)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  /* Not yet implemented */
  orxLOG("Not implemented yet!");

  /* Done! */
  return eResult;
}

orxBOOL orxDisplay_SDL_IsVSyncEnabled()
{
  orxBOOL bResult = orxFALSE;

  /* Checks */
  orxASSERT((sstDisplay.u32Flags & orxDISPLAY_KU32_STATIC_FLAG_READY) == orxDISPLAY_KU32_STATIC_FLAG_READY);

  //! TODO: Writes its implementation

  /* Done! */
  return bResult;
}

orxSTATUS orxDisplay_SDL_Init()
{
  orxSTATUS eResult;

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
#ifdef __orxGP2X__

      /* Inits display using config values? */
      sstDisplay.pstScreen = SDL_SetVideoMode(orxDISPLAY_KU32_SCREEN_WIDTH, orxDISPLAY_KU32_SCREEN_HEIGHT, orxDISPLAY_KU32_SCREEN_DEPTH, SDL_HWSURFACE | SDL_DOUBLEBUF | SDL_ANYFORMAT);

      /* Stores values */
      sstDisplay.fScreenWidth   = orxU2F(orxDISPLAY_KU32_SCREEN_WIDTH);
      sstDisplay.fScreenHeight  = orxU2F(orxDISPLAY_KU32_SCREEN_HEIGHT);

#else /* __orxGP2X__ */

      {
        orxU32 u32ConfigWidth, u32ConfigHeight, u32ConfigDepth, u32Flags;

        /* Gets resolution from config */
        orxConfig_SelectSection(orxDISPLAY_KZ_CONFIG_SECTION);
        u32ConfigWidth  = orxConfig_GetU32(orxDISPLAY_KZ_CONFIG_WIDTH);
        u32ConfigHeight = orxConfig_GetU32(orxDISPLAY_KZ_CONFIG_HEIGHT);
        u32ConfigDepth  = orxConfig_GetU32(orxDISPLAY_KZ_CONFIG_DEPTH);

        /* Full screen? */
        if(orxConfig_GetBool(orxDISPLAY_KZ_CONFIG_FULLSCREEN) != orxFALSE)
        {
          /* Updates flags */
          u32Flags = SDL_FULLSCREEN | SDL_HWSURFACE | SDL_DOUBLEBUF | SDL_ANYFORMAT;
        }
        else
        {
          /* Updates flags */
          u32Flags = SDL_HWSURFACE | SDL_DOUBLEBUF | SDL_ANYFORMAT;
        }

        /* Inits display using config values? */
        if((sstDisplay.pstScreen = SDL_SetVideoMode(u32ConfigWidth, u32ConfigHeight, u32ConfigDepth, u32Flags)) == orxNULL)
        {
          /* Inits display using default parameters */
          sstDisplay.pstScreen = SDL_SetVideoMode(orxDISPLAY_KU32_SCREEN_WIDTH, orxDISPLAY_KU32_SCREEN_HEIGHT, orxDISPLAY_KU32_SCREEN_DEPTH, SDL_HWSURFACE | SDL_DOUBLEBUF | SDL_ANYFORMAT);

          /* Stores values */
          sstDisplay.fScreenWidth   = orxU2F(orxDISPLAY_KU32_SCREEN_WIDTH);
          sstDisplay.fScreenHeight  = orxU2F(orxDISPLAY_KU32_SCREEN_HEIGHT);
        }
        else
        {
          /* Stores values */
          sstDisplay.fScreenWidth   = orxU2F(u32ConfigWidth);
          sstDisplay.fScreenHeight  = orxU2F(u32ConfigHeight);
        }
      }

#endif /* __orxGP2X__ */

      /* Updates result ? */
      eResult = (sstDisplay.pstScreen != NULL) ? orxSTATUS_SUCCESS : orxSTATUS_FAILURE;

      /* Valid? */
      if(eResult != orxSTATUS_FAILURE)
      {
        /* Sets module as ready */
        sstDisplay.u32Flags = orxDISPLAY_KU32_STATIC_FLAG_READY;
      }
    }
  }

  /* Done! */
  return eResult;  
}

orxVOID orxDisplay_SDL_Exit()
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

    /* Cleans static controller */
    orxMemory_Zero(&sstDisplay, sizeof(orxDISPLAY_STATIC));
  }

  return;
}

orxHANDLE orxDisplay_SDL_GetApplicationInput()
{
  orxHANDLE hResult = orxHANDLE_UNDEFINED;

  /* Not yet implemented */
  orxLOG("Not implemented yet!");

  /* Done! */
  return hResult;
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
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SDL_TransformBitmap, DISPLAY, TRANSFORM_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SDL_LoadBitmap, DISPLAY, LOAD_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SDL_GetBitmapSize, DISPLAY, GET_BITMAP_SIZE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SDL_GetScreenSize, DISPLAY, GET_SCREEN_SIZE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SDL_GetScreen, DISPLAY, GET_SCREEN_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SDL_ClearBitmap, DISPLAY, CLEAR_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SDL_SetBitmapClipping, DISPLAY, SET_BITMAP_CLIPPING);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SDL_BlitBitmap, DISPLAY, BLIT_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SDL_SetBitmapColorKey, DISPLAY, SET_BITMAP_COLOR_KEY);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SDL_SetBitmapColor, DISPLAY, SET_BITMAP_COLOR);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SDL_GetBitmapColor, DISPLAY, GET_BITMAP_COLOR);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SDL_DrawText, DISPLAY, DRAW_TEXT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SDL_GetApplicationInput, DISPLAY, GET_APPLICATION_INPUT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SDL_EnableVSync, DISPLAY, ENABLE_VSYNC);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SDL_IsVSyncEnabled, DISPLAY, IS_VSYNC_ENABLED);
orxPLUGIN_USER_CORE_FUNCTION_END();
