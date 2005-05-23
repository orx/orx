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

#include "math/orxVec.h"
#include "utils/orxString.h"


#define orx2ARGB(A, R, G, B)             ((((A) & 0xFF) << 24) | (((R) & 0xFF) << 16) | (((G) & 0xFF) << 8) | ((B) & 0xFF))


typedef struct __orxBITMAP_t            orxBITMAP;

typedef orxU32                          orxARGB;

typedef struct __orxRGB_t
{
  orxU8 u8Red;
  orxU8 u8Green;
  orxU8 u8Blue;
  
} orxRGB;

typedef struct __orxBITMAP_TRANSFORM_t
{
  orxVEC    vSrcCoord;
  orxVEC    vDstCoorf;

  orxVEC    vScale;
  orxFLOAT  fRotation;

} orxBITMAP_TRANSFORM;


/***************************************************************************
 * Functions directly implemented by orx core
 ***************************************************************************/

/** Function that initializes the display plugin module
 */
extern orxDLLAPI orxVOID          orxFASTCALL orxDisplay_Plugin_Init();


/***************************************************************************
 * Functions extended by plugins
 ***************************************************************************/

orxPLUGIN_DECLARE_CORE_FUNCTION_0(orxDisplay_Init, orxSTATUS);
orxPLUGIN_DECLARE_CORE_FUNCTION_0(orxDisplay_Exit, orxVOID);

orxPLUGIN_DECLARE_CORE_FUNCTION_0(orxDisplay_Swap, orxSTATUS);

orxPLUGIN_DECLARE_CORE_FUNCTION_4(orxDisplay_DrawText, orxSTATUS, orxCONST orxBITMAP *, orxCONST orxVEC *, orxARGB, orxCONST orxSTRING);

orxPLUGIN_DECLARE_CORE_FUNCTION_2(orxDisplay_CreateBitmap, orxBITMAP *, orxCONST orxVEC *, orxU32);
orxPLUGIN_DECLARE_CORE_FUNCTION_1(orxDisplay_DeleteBitmap, orxVOID, orxBITMAP *);

orxPLUGIN_DECLARE_CORE_FUNCTION_0(orxDisplay_GetScreenBitmap, orxBITMAP *);

orxPLUGIN_DECLARE_CORE_FUNCTION_2(orxDisplay_ClearBitmap, orxSTATUS, orxBITMAP *, orxARGB);
orxPLUGIN_DECLARE_CORE_FUNCTION_4(orxDisplay_TransformBitmap, orxSTATUS, orxBITMAP *, orxCONST orxBITMAP *, orxCONST orxBITMAP_TRANSFORM *, orxU32);

orxPLUGIN_DECLARE_CORE_FUNCTION_3(orxDisplay_SetBitmapColorKey, orxSTATUS, orxBITMAP *, orxRGB, orxBOOL);
orxPLUGIN_DECLARE_CORE_FUNCTION_3(orxDisplay_SetBitmapClipping, orxSTATUS, orxBITMAP *, orxCONST orxVEC *, orxCONST orxVEC *);

orxPLUGIN_DECLARE_CORE_FUNCTION_5(orxDisplay_BlitBitmap, orxSTATUS, orxBITMAP *, orxCONST orxBITMAP *, orxCONST orxVEC *, orxCONST orxVEC *, orxCONST orxVEC *);

orxPLUGIN_DECLARE_CORE_FUNCTION_2(orxDisplay_SaveBitmap, orxSTATUS, orxCONST orxBITMAP *, orxCONST orxSTRING);
orxPLUGIN_DECLARE_CORE_FUNCTION_1(orxDisplay_LoadBitmap, orxBITMAP *, orxCONST orxSTRING);

orxPLUGIN_DECLARE_CORE_FUNCTION_2(orxDisplay_GetBitmapSize, orxSTATUS, orxCONST orxBITMAP *, orxVEC *);



orxSTATIC orxINLINE orxDLLAPI orxSTATUS orxDisplay_Init()
{
  return orxPLUGIN_BODY_CORE_FUNCTION(orxDisplay_Init)();
}

orxSTATIC orxINLINE orxDLLAPI orxVOID orxDisplay_Exit()
{
  orxPLUGIN_BODY_CORE_FUNCTION(orxDisplay_Exit)();
}

orxSTATIC orxINLINE orxDLLAPI orxSTATUS orxDisplay_Swap()
{
  return orxPLUGIN_BODY_CORE_FUNCTION(orxDisplay_Swap)();
}

orxSTATIC orxINLINE orxDLLAPI orxSTATUS orxDisplay_DrawText(orxCONST orxBITMAP *_pstBitmap, orxCONST orxVEC *_pvPos, orxARGB _stColor, orxCONST orxSTRING _zText)
{
  return orxPLUGIN_BODY_CORE_FUNCTION(orxDisplay_DrawText)(_pstBitmap, _pvPos, _stColor, _zText);
}

orxSTATIC orxINLINE orxDLLAPI orxBITMAP *orxDisplay_CreateBitmap(orxCONST orxVEC *_pvSize, orxU32 _u32Flags)
{
  return orxPLUGIN_BODY_CORE_FUNCTION(orxDisplay_CreateBitmap)(_pvSize, _u32Flags);
}

orxSTATIC orxINLINE orxDLLAPI orxVOID orxDisplay_DeleteBitmap(orxBITMAP *_pstBitmap)
{
  orxPLUGIN_BODY_CORE_FUNCTION(orxDisplay_DeleteBitmap)(_pstBitmap);
}

orxSTATIC orxINLINE orxDLLAPI orxBITMAP *orxDisplay_GetScreenBitmap()
{
  return orxPLUGIN_BODY_CORE_FUNCTION(orxDisplay_GetScreenBitmap)();
}

orxSTATIC orxINLINE orxDLLAPI orxSTATUS orxDisplay_ClearBitmap(orxBITMAP *_pstBitmap, orxARGB _stColor)
{
  return orxPLUGIN_BODY_CORE_FUNCTION(orxDisplay_ClearBitmap)(_pstBitmap, _stColor);
}

orxSTATIC orxINLINE orxDLLAPI orxSTATUS orxDisplay_TransformBitmap(orxBITMAP *_pstDst, orxCONST orxBITMAP *_pstSrc, orxCONST orxBITMAP_TRANSFORM *_pstTransform, orxU32 _u32Flags)
{
  return orxPLUGIN_BODY_CORE_FUNCTION(orxDisplay_TransformBitmap)(_pstDst, _pstSrc, _pstTransform, _u32Flags);
}

orxSTATIC orxINLINE orxDLLAPI orxSTATUS orxDisplay_SetBitmapColorKey(orxBITMAP *_pstBitmap, orxRGB _stColor, orxBOOL _bEnable)
{
  return orxPLUGIN_BODY_CORE_FUNCTION(orxDisplay_SetBitmapColorKey)(_pstBitmap, _stColor, _bEnable);
}

orxSTATIC orxINLINE orxDLLAPI orxSTATUS orxDisplay_SetBitmapClipping(orxBITMAP *_pstBitmap, orxCONST orxVEC *_pvTL, orxCONST orxVEC *_pvBR)
{
  return orxPLUGIN_BODY_CORE_FUNCTION(orxDisplay_SetBitmapClipping)(_pstBitmap, _pvTL, _pvBR);
}

orxSTATIC orxINLINE orxDLLAPI orxSTATUS orxDisplay_BlitBitmap(orxBITMAP *_pstDst, orxCONST orxBITMAP *_pstSrc, orxCONST orxVEC *_pvDstCoord, orxCONST orxVEC *_pvSrcCoord, orxCONST orxVEC *_pvSize)
{
  return orxPLUGIN_BODY_CORE_FUNCTION(orxDisplay_BlitBitmap)(_pstDst, _pstSrc, _pvDstCoord, _pvSrcCoord, _pvSize);
}

orxSTATIC orxINLINE orxDLLAPI orxSTATUS orxDisplay_SaveBitmap(orxCONST orxBITMAP *_pstBitmap, orxCONST orxSTRING _zFileName)
{
  return orxPLUGIN_BODY_CORE_FUNCTION(orxDisplay_SaveBitmap)(_pstBitmap, _zFileName);
}

orxSTATIC orxINLINE orxDLLAPI orxBITMAP *orxDisplay_LoadBitmap(orxCONST orxSTRING _zFileName)
{
  return orxPLUGIN_BODY_CORE_FUNCTION(orxDisplay_LoadBitmap)(_zFileName);
}

orxSTATIC orxINLINE orxDLLAPI orxSTATUS orxDisplay_GetBitmapSize(orxCONST orxBITMAP *_pstBitmap, orxVEC *_pvSize)
{
  return orxPLUGIN_BODY_CORE_FUNCTION(orxDisplay_GetBitmapSize)(_pstBitmap, _pvSize);
}


#endif /* _orxDISPLAY_H_ */
