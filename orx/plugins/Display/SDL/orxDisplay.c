/**
 * \file DISPLAY_plug.c
 */

/***************************************************************************
 begin                : 14/11/2003
 author               : (C) Arcallians
 email                : iarwain@arcallians.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "orxInclude.h"

#include "debug/orxDebug.h"
#include "plugin/orxPluginUser.h"

#include "msg/msg_graph.h"


#include <SDL/SDL.h>
//#include <SDL/sge.h>

#define KI_BPP    16
#define KI_WIDTH  800
#define KI_HEIGHT 600


/********************
 *   Core Related   *
 ********************/

static SDL_Surface *spstScreen = NULL;

SDL_Surface *orxDisplay_SDL_GetScreen()
{
  return(SDL_GetVideoSurface());
}

void orxDisplay_SDL_DrawText(SDL_Surface *_pst_bmp, orxS32 _s32X, orxS32 _s32Y, orxU32 _u32Color, const orxCHAR *_zFormat, ...)
{
  char acBuffer[1024];
  va_list stArgs;

  va_start(stArgs, _zFormat);
  vsprintf(acBuffer, _zFormat, stArgs);
  va_end(stArgs);

/* TODO :
 * Write the string onto screen, using char per char pixel writing
 */
  return;
}

void orxDisplay_SDL_DeleteBitmap(SDL_Surface *_pstBitmap)
{
  SDL_FreeSurface(_pstBitmap);
  return;
}

SDL_Surface *orxDisplay_SDL_CreateBitmap(orxS32 _s32W, orxS32 _s32H)
{
  orxU32 u32RMask, u32GMask, u32BMask, u32AMask;

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

  return SDL_CreateRGBSurface(SDL_HWSURFACE, _s32W, _s32H, KI_BPP,
                              u32RMask, u32GMask, u32BMask, u32AMask);
}

void orxDisplay_SDL_ClearBitmap(SDL_Surface *_pstBitmap)
{
  SDL_FillRect(_pstBitmap, NULL, 0x00000000);

  return;
}

void orxDisplay_SDL_Swap()
{
  SDL_Flip(spstScreen);

  return;
}

void orxDisplay_SDL_SetBitmapColorKey(SDL_Surface *_pstSrc, orxU32 _u32Red, orxU32 _u32Green, orxU32 _u32Blue, orxBOOL _bEnable)
{
  if(_bEnable != orxFALSE)
  {
    SDL_SetColorKey(_pstSrc, SDL_SRCCOLORKEY | SDL_RLEACCEL, SDL_MapRGB(_pstSrc->format, _u32Red, _u32Green, _u32Blue));
  }
  else
  {
    SDL_SetColorKey(_pstSrc, 0, 0);
  }
  
  return;
}

void orxDisplay_SDL_BlitBitmap(SDL_Surface *_pstSrc, SDL_Surface *_pstDst, orxS32 _s32SrcX, orxS32 _s32SrcY, orxS32 _s32DstX, orxS32 _s32DstY, orxS32 _s32W, orxS32 _s32H)
{
  SDL_Rect stSrcRect, stDstRect;

  stSrcRect.x = _s32SrcX;
  stSrcRect.y = _s32SrcY;
  stSrcRect.w = _s32W;
  stSrcRect.h = _s32H;
  stDstRect.x = _s32DstX;
  stDstRect.y = _s32DstY;

  SDL_BlitSurface(_pstSrc, &stSrcRect, _pstDst, &stDstRect);

  return;
}

void orxDisplay_SDL_TransformBitmap(SDL_Surface *_pstSrc, SDL_Surface *_pstDst, orxFLOAT _fRotation, orxFLOAT _fScaleX, orxFLOAT _fScaleY, orxS32 _s32SrcX, orxS32 _s32SrcY, orxS32 _s32DstX, orxS32 _s32DstY, orxBOOL _bAntialiased)
{
//  orxU32 u32Flags;

  orxASSERT(orxFALSE && "Not implemented yet");
  
  /* !!! TODO : Code this using hardware acceleration !!! */
  
  /* Updates flags */
//  u32Flags = (_bAntialiased != orxFALSE) ? SGE_TAA : 0; 

  /* Transforms surface */
//    sge_transform(_pstSrc, _pstDst, _fRotation * (180.0 / orxPI), _fScaleX, _fScaleY, _s32SrcX, _s32SrcY, _s32DstX, _s32DstY, u32Flags);

  return;
}

void orxDisplay_SDL_SaveBitmap(const orxCHAR *_zFilename, SDL_Surface *_pstBitmap)
{
  SDL_SaveBMP(_pstBitmap, _zFilename);

  return;
}

void orxDisplay_SDL_SetBitmapClipping(SDL_Surface *_pstBitmap, orxS32 _s32X, orxS32 _s32Y, orxS32 _s32W, orxS32 _s32H)
{
  SDL_Rect stClipRect;

  stClipRect.x = _s32X;
  stClipRect.y = _s32Y;
  stClipRect.w = _s32W;
  stClipRect.h = _s32H;

  SDL_SetClipRect(_pstBitmap, &stClipRect);

  return;
}

orxU32 orxDisplay_SDL_Init()
{
  if(SDL_WasInit(SDL_INIT_EVERYTHING) != 0)
  {
    SDL_InitSubSystem(SDL_INIT_VIDEO);
  }
  else
  {
    SDL_Init(SDL_INIT_VIDEO);
  }

  spstScreen = SDL_SetVideoMode(KI_WIDTH,
                                KI_HEIGHT,
                                KI_BPP,
                                SDL_HWSURFACE | SDL_DOUBLEBUF);

  if(spstScreen == NULL)
  {
    orxDEBUG_LOG(orxDEBUG_LEVEL_DISPLAY, KZ_MSG_MODE_INIT_FAILED_III, KI_WIDTH, KI_HEIGHT, KI_BPP);

    return EXIT_FAILURE;
  }
  else
  {
    orxDEBUG_LOG(orxDEBUG_LEVEL_DISPLAY, KZ_MSG_MODE_INIT_SUCCESS_III, KI_WIDTH, KI_HEIGHT, KI_BPP);
  }

  return EXIT_SUCCESS;  
}

void orxDisplay_SDL_Exit()
{
  if(SDL_WasInit(SDL_INIT_EVERYTHING) == SDL_INIT_VIDEO)
  {
    SDL_Quit();
  }
  else
  {
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
  }

  return;
}

SDL_Surface *orxDisplay_SDL_LoadBitmap(const orxCHAR *_zFilename)
{
  /* !!! TODO !!!
   * Needs to add a test on requested format.
   * Needs to work with other format than BMP. */

  return(SDL_LoadBMP(_zFilename));
}

void orxDisplay_SDL_GetBitmapSize(SDL_Surface *_pstBitmap, orxS32 *_ps32Width, orxS32 *_ps32Height)
{
  /* Non null? */
  if(_pstBitmap != NULL)
  {
    /* Gets size info */
    *_ps32Height = _pstBitmap->h;
    *_ps32Width = _pstBitmap->w;
  }
  else
  {
    /* Null pointer -> cleans size values */
    /* !!! MSG !!! */
    *_ps32Height = -1;
    *_ps32Width = -1;
  }

  return;
}



/********************
 *  Plugin Related  *
 ********************/

orxSTATIC orxPLUGIN_USER_FUNCTION_INFO sau32Display_Function[orxPLUGIN_FUNCTION_BASE_ID_DISPLAY_NUMBER];

extern orxDLLEXPORT orxVOID orxPlugin_Init(orxS32 *_ps32Number, orxPLUGIN_USER_FUNCTION_INFO **_ppstInfo)
{
  orxPLUGIN_USER_FUNCTION_START(sau32Display_Function);
  orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SDL_Init, DISPLAY, INIT);
  orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SDL_Exit, DISPLAY, EXIT);
  orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SDL_Swap, DISPLAY, SWAP);
  orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SDL_CreateBitmap, DISPLAY, CREATE_BITMAP);
  orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SDL_DeleteBitmap, DISPLAY, DELETE_BITMAP);
  orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SDL_SaveBitmap, DISPLAY, SAVE_BITMAP);
  orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SDL_TransformBitmap, DISPLAY, TRANSFORM_BITMAP);
  orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SDL_LoadBitmap, DISPLAY, LOAD_BITMAP);
  orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SDL_GetBitmapSize, DISPLAY, GET_BITMAP_SIZE);
  orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SDL_GetScreen, DISPLAY, GET_SCREEN_BITMAP);
  orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SDL_ClearBitmap, DISPLAY, CLEAR_BITMAP);
  orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SDL_SetBitmapClipping, DISPLAY, SET_BITMAP_CLIPPING);
  orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SDL_BlitBitmap, DISPLAY, BLIT_BITMAP);
  orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SDL_SetBitmapColorKey, DISPLAY, SET_BITMAP_COLOR_KEY);
  orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SDL_DrawText, DISPLAY, DRAW_TEXT);
  orxPLUGIN_USER_FUNCTION_END(_ps32Number, _ppstInfo);

  return;
}
