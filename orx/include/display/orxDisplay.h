/**
 * \file orxDisplay.h
 */

/***************************************************************************
 begin                : 23/04/2003
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

#ifndef _orxDISPLAY_H_
#define _orxDISPLAY_H_

#include "orxInclude.h"
#include "plugin/orxPluginCore.h"

#include "math/orxVector.h"
#include "utils/orxString.h"


#define orx2RGBA(R, G, B, A)            ((((R) & 0xFF) << 24) | (((G) & 0xFF) << 16) | (((B) & 0xFF) << 8) | ((A) & 0xFF))
#define orxRGBA_R(ARGB)                 (((ARGB) >> 24) & 0xFF)
#define orxRGBA_G(ARGB)                 (((ARGB) >> 16) & 0xFF)
#define orxRGBA_B(ARGB)                 (((ARGB) >> 8) & 0xFF)
#define orxRGBA_A(ARGB)                 ((ARGB) & 0xFF)

typedef struct __orxBITMAP_t            orxBITMAP;


typedef struct __orxBITMAP_TRANSFORM_t
{
  orxU32    u32SrcX, u32SrcY, u32DstX, u32DstY;
  orxFLOAT  fRotation;
  orxFLOAT  fScaleX;
  orxFLOAT  fScaleY;

} orxBITMAP_TRANSFORM;


/***************************************************************************
 * Functions directly implemented by orx core
 ***************************************************************************/

/** Display module setup */
extern orxDLLAPI orxVOID            orxDisplay_Setup();


/***************************************************************************
 * Functions extended by plugins
 ***************************************************************************/

orxPLUGIN_DECLARE_CORE_FUNCTION(orxDisplay_Init, orxSTATUS);
orxPLUGIN_DECLARE_CORE_FUNCTION(orxDisplay_Exit, orxVOID);

orxPLUGIN_DECLARE_CORE_FUNCTION(orxDisplay_Swap, orxSTATUS);

orxPLUGIN_DECLARE_CORE_FUNCTION(orxDisplay_DrawText, orxSTATUS, orxCONST orxBITMAP *, orxCONST orxBITMAP_TRANSFORM *, orxRGBA, orxCONST orxSTRING);

orxPLUGIN_DECLARE_CORE_FUNCTION(orxDisplay_CreateBitmap, orxBITMAP *, orxU32, orxU32);
orxPLUGIN_DECLARE_CORE_FUNCTION(orxDisplay_DeleteBitmap, orxVOID, orxBITMAP *);

orxPLUGIN_DECLARE_CORE_FUNCTION(orxDisplay_GetScreenBitmap, orxBITMAP *);

orxPLUGIN_DECLARE_CORE_FUNCTION(orxDisplay_ClearBitmap, orxSTATUS, orxBITMAP *, orxRGBA);
orxPLUGIN_DECLARE_CORE_FUNCTION(orxDisplay_TransformBitmap, orxSTATUS, orxBITMAP *, orxCONST orxBITMAP *, orxCONST orxBITMAP_TRANSFORM *, orxU32);

orxPLUGIN_DECLARE_CORE_FUNCTION(orxDisplay_SetBitmapColorKey, orxSTATUS, orxBITMAP *, orxRGBA, orxBOOL);
orxPLUGIN_DECLARE_CORE_FUNCTION(orxDisplay_SetBitmapClipping, orxSTATUS, orxBITMAP *, orxU32, orxU32, orxU32, orxU32);

orxPLUGIN_DECLARE_CORE_FUNCTION(orxDisplay_BlitBitmap, orxSTATUS, orxBITMAP *, orxCONST orxBITMAP *, orxU32, orxU32);

orxPLUGIN_DECLARE_CORE_FUNCTION(orxDisplay_SaveBitmap, orxSTATUS, orxCONST orxBITMAP *, orxCONST orxSTRING);
orxPLUGIN_DECLARE_CORE_FUNCTION(orxDisplay_LoadBitmap, orxBITMAP *, orxCONST orxSTRING);

orxPLUGIN_DECLARE_CORE_FUNCTION(orxDisplay_GetBitmapSize, orxSTATUS, orxCONST orxBITMAP *, orxU32 *, orxU32 *);



orxSTATIC orxINLINE orxSTATUS orxDisplay_Init()
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_Init)();
}

orxSTATIC orxINLINE orxVOID orxDisplay_Exit()
{
  orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_Exit)();
}

orxSTATIC orxINLINE orxSTATUS orxDisplay_Swap()
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_Swap)();
}

orxSTATIC orxINLINE orxSTATUS orxDisplay_DrawText(orxCONST orxBITMAP *_pstBitmap, orxCONST orxBITMAP_TRANSFORM *_pstTransform, orxRGBA _stColor, orxCONST orxSTRING _zText)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_DrawText)(_pstBitmap, _pstTransform, _stColor, _zText);
}

orxSTATIC orxINLINE orxBITMAP *orxDisplay_CreateBitmap(orxU32 _u32Width, orxU32 _u32Height)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_CreateBitmap)(_u32Width, _u32Height);
}

orxSTATIC orxINLINE orxVOID orxDisplay_DeleteBitmap(orxBITMAP *_pstBitmap)
{
  orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_DeleteBitmap)(_pstBitmap);
}

orxSTATIC orxINLINE orxBITMAP *orxDisplay_GetScreenBitmap()
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_GetScreenBitmap)();
}

orxSTATIC orxINLINE orxSTATUS orxDisplay_ClearBitmap(orxBITMAP *_pstBitmap, orxRGBA _stColor)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_ClearBitmap)(_pstBitmap, _stColor);
}

orxSTATIC orxINLINE orxSTATUS orxDisplay_TransformBitmap(orxBITMAP *_pstDst, orxCONST orxBITMAP *_pstSrc, orxCONST orxBITMAP_TRANSFORM *_pstTransform, orxU32 _u32Flags)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_TransformBitmap)(_pstDst, _pstSrc, _pstTransform, _u32Flags);
}

orxSTATIC orxINLINE orxSTATUS orxDisplay_SetBitmapColorKey(orxBITMAP *_pstBitmap, orxRGBA _stColor, orxBOOL _bEnable)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_SetBitmapColorKey)(_pstBitmap, _stColor, _bEnable);
}

orxSTATIC orxINLINE orxSTATUS orxDisplay_SetBitmapClipping(orxBITMAP *_pstBitmap, orxU32 _u32TLX, orxU32 _u32TLY, orxU32 _u32BRX, orxU32 _u32BRY)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_SetBitmapClipping)(_pstBitmap, _u32TLX, _u32TLY, _u32BRX, _u32BRY);
}

orxSTATIC orxINLINE orxSTATUS orxDisplay_BlitBitmap(orxBITMAP *_pstDst, orxCONST orxBITMAP *_pstSrc, orxU32 _u32PosX, orxU32 _u32PosY)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_BlitBitmap)(_pstDst, _pstSrc, _u32PosX, _u32PosY);
}

orxSTATIC orxINLINE orxSTATUS orxDisplay_SaveBitmap(orxCONST orxBITMAP *_pstBitmap, orxCONST orxSTRING _zFileName)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_SaveBitmap)(_pstBitmap, _zFileName);
}

orxSTATIC orxINLINE orxBITMAP *orxDisplay_LoadBitmap(orxCONST orxSTRING _zFileName)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_LoadBitmap)(_zFileName);
}

orxSTATIC orxINLINE orxSTATUS orxDisplay_GetBitmapSize(orxCONST orxBITMAP *_pstBitmap, orxU32 *_pu32Width, orxU32 *_pu32Height)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_GetBitmapSize)(_pstBitmap, _pu32Width, _pu32Height);
}


#endif /* _orxDISPLAY_H_ */
