/**
 * @file orxDisplay.c
 * 
 * SDL display plugin
 * 
 */
 
 /***************************************************************************
 orxDisplay.c
 SDL display plugin
 
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
#include "math/orxMath.h"
#include "plugin/orxPluginUser.h"

#include "msg/msg_graph.h"

#include "display/orxDisplay.h"



#include <SDL/SDL.h>
#include <SDL/sge.h>

#define KI_BPP    24
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

orxSTATUS orxDisplay_SDL_DrawText(orxCONST orxBITMAP *_pstBitmap, orxCONST orxVECTOR *_pvPos, orxRGBA _stColor, orxCONST orxSTRING _zFormat)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

/* TODO :
 * Write the string onto screen, using char per char pixel writing
 */

  orxASSERT(orxFALSE && "Not implemented yet!");

  /* Done! */
  return eResult;
}

orxVOID orxDisplay_SDL_DeleteBitmap(orxBITMAP *_pstBitmap)
{
  SDL_FreeSurface((SDL_Surface *)_pstBitmap);
  return;
}

orxBITMAP *orxDisplay_SDL_CreateBitmap(orxU32 _u32Width, orxU32 _u32Height)
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

  return((orxBITMAP *)SDL_CreateRGBSurface(SDL_HWSURFACE, _u32Width, _u32Height, KI_BPP,
                                          u32RMask, u32GMask, u32BMask, u32AMask));
}

orxSTATUS orxDisplay_SDL_ClearBitmap(orxBITMAP *_pstBitmap, orxRGBA _stColor)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  eResult = (SDL_FillRect((SDL_Surface *)_pstBitmap, NULL, _stColor) == 0)
            ? orxSTATUS_SUCCESS
            : orxSTATUS_FAILURE;

  return eResult;
}

orxSTATUS orxDisplay_SDL_Swap()
{
  orxSTATUS eResult;

  eResult = (SDL_Flip(spstScreen) == 0)
            ? orxSTATUS_SUCCESS
            : orxSTATUS_FAILURE;

  return eResult;
}

orxSTATUS orxDisplay_SDL_SetBitmapColorKey(orxBITMAP *_pstSrc, orxRGBA _stColor, orxBOOL _bEnable)
{
  orxSTATUS eResult;

  if(_bEnable != orxFALSE)
  {
    eResult = (SDL_SetColorKey((SDL_Surface *)_pstSrc, SDL_SRCCOLORKEY | SDL_RLEACCEL, SDL_MapRGB(((SDL_Surface *)_pstSrc)->format, orxRGBA_R(_stColor), orxRGBA_G(_stColor), orxRGBA_B(_stColor))) == 0)
              ? orxSTATUS_SUCCESS
              : orxSTATUS_FAILURE;
  }
  else
  {
    eResult = (SDL_SetColorKey((SDL_Surface *)_pstSrc, 0, 0) == 0)
              ? orxSTATUS_SUCCESS
              : orxSTATUS_FAILURE;
  }
  
  return eResult;
}

orxSTATUS orxDisplay_SDL_BlitBitmap(orxBITMAP *_pstDst, orxCONST orxBITMAP *_pstSrc, orxCONST orxU32 _u32PosX, orxU32 _u32PosY)
{
  SDL_Rect  stSrcRect, stDstRect;
  orxSTATUS eResult;

  stSrcRect.x = 0;
  stSrcRect.y = 0;
  stSrcRect.w = ((SDL_Surface *)_pstSrc)->w;
  stSrcRect.h = ((SDL_Surface *)_pstSrc)->h;
  stDstRect.x = _u32PosX;
  stDstRect.y = _u32PosY;
  stDstRect.w = 0.0f;
  stDstRect.h = 0.0f;

  eResult = (SDL_BlitSurface((SDL_Surface *)_pstSrc, &stSrcRect, (SDL_Surface *)_pstDst, &stDstRect) == 0)
            ? orxSTATUS_SUCCESS
            : orxSTATUS_FAILURE;

  return eResult;
}

orxSTATUS orxDisplay_SDL_TransformBitmap(orxBITMAP *_pstDst, orxCONST orxBITMAP *_pstSrc, orxCONST orxBITMAP_TRANSFORM *_pstTransform, orxU32 _u32Flags)
{
  SDL_Rect  stRectangle;
  orxSTATUS eResult;

  /* Uses SGE for bitmap transformation */
  stRectangle = sge_transform((SDL_Surface *)_pstSrc, (SDL_Surface *)_pstDst, _pstTransform->fRotation * orxMATH_KF_RAD_TO_DEG, _pstTransform->fScaleX, _pstTransform->fScaleY, _pstTransform->u32SrcX, _pstTransform->u32SrcY, _pstTransform->u32DstX, _pstTransform->u32DstY, _u32Flags);

  /* Updates result */
  eResult = ((stRectangle.x == 0) && (stRectangle.y == 0) && (stRectangle.w == 0) && (stRectangle.h == 0)) ? orxSTATUS_FAILURE : orxSTATUS_SUCCESS;

  /* Done! */
  return eResult;
}

orxSTATUS orxDisplay_SDL_SaveBitmap(orxCONST orxBITMAP *_pstBitmap, orxCONST orxSTRING _zFilename)
{
  orxSTATUS eResult;

  eResult = (SDL_SaveBMP((SDL_Surface *)_pstBitmap, _zFilename) == 0)
            ? orxSTATUS_SUCCESS
            : orxSTATUS_FAILURE;

  return eResult;
}

orxSTATUS orxDisplay_SDL_SetBitmapClipping(orxBITMAP *_pstBitmap, orxU32 _u32TLX, orxU32 _u32TLY, orxU32 _u32BRX, orxU32 _u32BRY)
{
  SDL_Rect  stClipRect;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Gets SDL clip rectangle */
  stClipRect.x = _u32TLX;
  stClipRect.y = _u32TLY;
  stClipRect.w = _u32BRX - _u32TLX;
  stClipRect.h = _u32BRY - _u32TLY;

  /* Applies it */
  SDL_SetClipRect((SDL_Surface *)_pstBitmap, &stClipRect);

  return eResult;
}

orxSTATUS orxDisplay_SDL_Init()
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

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
                                SDL_HWSURFACE | SDL_DOUBLEBUF | SDL_ANYFORMAT);

  if(spstScreen == NULL)
  {
    orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, KZ_MSG_MODE_INIT_FAILED_III, KI_WIDTH, KI_HEIGHT, KI_BPP);

    eResult = orxSTATUS_FAILURE;
  }
  else
  {
    orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, KZ_MSG_MODE_INIT_SUCCESS_III, KI_WIDTH, KI_HEIGHT, KI_BPP);
  }

  return eResult;  
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

orxSTATUS orxDisplay_SDL_GetBitmapSize(orxCONST orxBITMAP *_pstBitmap, orxU32 *_pu32Width, orxU32 *_pu32Height)
{
  orxSTATUS eResult;

  orxASSERT(_pu32Width != orxNULL);
  orxASSERT(_pu32Height != orxNULL);
  
  /* Non null? */
  if(_pstBitmap != NULL)
  {
    /* Gets size info */
    *_pu32Width  = ((SDL_Surface *)_pstBitmap)->w;
    *_pu32Height = ((SDL_Surface *)_pstBitmap)->h;

    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }
  else
  {
    /* !!! MSG !!! */

    /* Null pointer -> cleans size values */
    *_pu32Width  = 0;
    *_pu32Height = 0;

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}


/********************
 *  Plugin Related  *
 ********************/

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
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SDL_GetScreen, DISPLAY, GET_SCREEN_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SDL_ClearBitmap, DISPLAY, CLEAR_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SDL_SetBitmapClipping, DISPLAY, SET_BITMAP_CLIPPING);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SDL_BlitBitmap, DISPLAY, BLIT_BITMAP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SDL_SetBitmapColorKey, DISPLAY, SET_BITMAP_COLOR_KEY);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxDisplay_SDL_DrawText, DISPLAY, DRAW_TEXT);
orxPLUGIN_USER_CORE_FUNCTION_END();
