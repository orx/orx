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

#include "display/orxDisplay.h"



#include <SDL/SDL.h>
//#include <SDL/sge.h>

#define KI_BPP    32
#define KI_WIDTH  800
#define KI_HEIGHT 600


/********************
 *   Core Related   *
 ********************/

static SDL_Surface *spstScreen = NULL;

orxBITMAP *orxDisplay_SDL_GetScreen()
{
  return((orxBITMAP *)SDL_GetVideoSurface());
}

orxVOID orxDisplay_SDL_DrawText(orxCONST orxBITMAP *_pstBitmap, orxCONST orxVECTOR *_pvPos, orxARGB _stColor, orxCONST orxSTRING _zFormat)
{
/* TODO :
 * Write the string onto screen, using char per char pixel writing
 */

  orxASSERT(orxFALSE && "Not implemented yet!");
  
  return;
}

orxVOID orxDisplay_SDL_DeleteBitmap(orxBITMAP *_pstBitmap)
{
  SDL_FreeSurface((SDL_Surface *)_pstBitmap);
  return;
}

orxBITMAP *orxDisplay_SDL_CreateBitmap(orxCONST orxVECTOR *_pvSize, orxU32 _u32Flags)
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

  return((orxBITMAP *)SDL_CreateRGBSurface(SDL_HWSURFACE, _pvSize->fX, _pvSize->fY, KI_BPP,
                                          u32RMask, u32GMask, u32BMask, u32AMask));
}

orxVOID orxDisplay_SDL_ClearBitmap(orxBITMAP *_pstBitmap, orxARGB _stColor)
{
  SDL_FillRect((SDL_Surface *)_pstBitmap, NULL, _stColor);

  return;
}

orxVOID orxDisplay_SDL_Swap()
{
  SDL_Flip(spstScreen);

  return;
}

orxVOID orxDisplay_SDL_SetBitmapColorKey(orxBITMAP *_pstSrc, orxARGB _stColor, orxBOOL _bEnable)
{
  if(_bEnable != orxFALSE)
  {
    SDL_SetColorKey((SDL_Surface *)_pstSrc, SDL_SRCCOLORKEY | SDL_RLEACCEL, SDL_MapRGB(((SDL_Surface *)_pstSrc)->format, orxARGB_R(_stColor), orxARGB_G(_stColor), orxARGB_B(_stColor)));
  }
  else
  {
    SDL_SetColorKey((SDL_Surface *)_pstSrc, 0, 0);
  }
  
  return;
}

orxVOID orxDisplay_SDL_BlitBitmap(orxBITMAP *_pstDst, orxCONST orxBITMAP *_pstSrc, orxCONST orxVECTOR *_pvDstCoord, orxCONST orxVECTOR *_pvSrcCoord, orxCONST orxVECTOR *_pvSize)
{
  SDL_Rect stSrcRect, stDstRect;

  stSrcRect.x = _pvSrcCoord->fX;
  stSrcRect.y = _pvSrcCoord->fY;
  stSrcRect.w = _pvSize->fX;
  stSrcRect.h = _pvSize->fY;
  stDstRect.x = _pvDstCoord->fX;
  stDstRect.y = _pvDstCoord->fY;

  SDL_BlitSurface((SDL_Surface *)_pstSrc, &stSrcRect, (SDL_Surface *)_pstDst, &stDstRect);

  return;
}

orxVOID orxDisplay_SDL_TransformBitmap(orxBITMAP *_pstDst, orxCONST orxBITMAP *_pstSrc, orxCONST orxBITMAP_TRANSFORM *_pstTransform, orxU32 _u32Flags)
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

orxVOID orxDisplay_SDL_SaveBitmap(orxCONST orxBITMAP *_pstBitmap, orxCONST orxSTRING _zFilename)
{
  SDL_SaveBMP((SDL_Surface *)_pstBitmap, _zFilename);

  return;
}

orxVOID orxDisplay_SDL_SetBitmapClipping(orxBITMAP *_pstBitmap, orxCONST orxVECTOR *_pvTL, orxCONST orxVECTOR *_pvBR)
{
  SDL_Rect  stClipRect;
  orxVECTOR vSize;

  /* Gets size vector */
  orxVector_Sub(&vSize, _pvBR, _pvTL);

  /* Gets SDL clip rectangle */
  stClipRect.x = _pvTL->fX;
  stClipRect.y = _pvTL->fY;
  stClipRect.w = vSize.fX;
  stClipRect.h = vSize.fY;

  /* Applies it */
  SDL_SetClipRect((SDL_Surface *)_pstBitmap, &stClipRect);

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

orxVOID orxDisplay_SDL_Exit()
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

orxBITMAP *orxDisplay_SDL_LoadBitmap(orxCONST orxSTRING _zFilename)
{
  /* !!! TODO !!!
   * Needs to add a test on requested format.
   * Needs to work with other format than BMP. */

  return((orxBITMAP *)SDL_LoadBMP(_zFilename));
}

orxSTATUS orxDisplay_SDL_GetBitmapSize(orxCONST orxBITMAP *_pstBitmap, orxVECTOR *_pvSize)
{
  orxSTATUS eResult;

  orxASSERT(_pvSize != orxNULL);
  
  /* Non null? */
  if(_pstBitmap != NULL)
  {
    /* Gets size info */
    _pvSize->fX = ((SDL_Surface *)_pstBitmap)->h;
    _pvSize->fY = ((SDL_Surface *)_pstBitmap)->w;

    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }
  else
  {
    /* !!! MSG !!! */

    /* Null pointer -> cleans size values */
    _pvSize->fX = -1;
    _pvSize->fY = -1;

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
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
