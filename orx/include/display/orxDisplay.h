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


#define orx2ARGB(A, R, G, B)            ((((A) & 0xFF) << 24) | (((R) & 0xFF) << 16) | (((G) & 0xFF) << 8) | ((B) & 0xFF))


typedef struct __orxBITMAP_t            orxBITMAP;

typedef orxU32                          orxARGB;

typedef struct __orxBITMAP_TRANSFORM_t
{
  orxVEC    vSrcCoord;
  orxVEC    vDstCoord;

  orxVEC    vScale;
  orxFLOAT  fRotation;

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

orxPLUGIN_DECLARE_CORE_FUNCTION(orxDisplay_DrawText, orxSTATUS, orxCONST orxBITMAP *, orxCONST orxVEC *, orxARGB, orxCONST orxSTRING);

orxPLUGIN_DECLARE_CORE_FUNCTION(orxDisplay_CreateBitmap, orxBITMAP *, orxCONST orxVEC *, orxU32);
orxPLUGIN_DECLARE_CORE_FUNCTION(orxDisplay_DeleteBitmap, orxVOID, orxBITMAP *);

orxPLUGIN_DECLARE_CORE_FUNCTION(orxDisplay_GetScreenBitmap, orxBITMAP *);

orxPLUGIN_DECLARE_CORE_FUNCTION(orxDisplay_ClearBitmap, orxSTATUS, orxBITMAP *, orxARGB);
orxPLUGIN_DECLARE_CORE_FUNCTION(orxDisplay_TransformBitmap, orxSTATUS, orxBITMAP *, orxCONST orxBITMAP *, orxCONST orxBITMAP_TRANSFORM *, orxU32);

orxPLUGIN_DECLARE_CORE_FUNCTION(orxDisplay_SetBitmapColorKey, orxSTATUS, orxBITMAP *, orxARGB, orxBOOL);
orxPLUGIN_DECLARE_CORE_FUNCTION(orxDisplay_SetBitmapClipping, orxSTATUS, orxBITMAP *, orxCONST orxVEC *, orxCONST orxVEC *);

orxPLUGIN_DECLARE_CORE_FUNCTION(orxDisplay_BlitBitmap, orxSTATUS, orxBITMAP *, orxCONST orxBITMAP *, orxCONST orxVEC *, orxCONST orxVEC *, orxCONST orxVEC *);

orxPLUGIN_DECLARE_CORE_FUNCTION(orxDisplay_SaveBitmap, orxSTATUS, orxCONST orxBITMAP *, orxCONST orxSTRING);
orxPLUGIN_DECLARE_CORE_FUNCTION(orxDisplay_LoadBitmap, orxBITMAP *, orxCONST orxSTRING);

orxPLUGIN_DECLARE_CORE_FUNCTION(orxDisplay_GetBitmapSize, orxSTATUS, orxCONST orxBITMAP *, orxVEC *);



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

orxSTATIC orxINLINE orxSTATUS orxDisplay_DrawText(orxCONST orxBITMAP *_pstBitmap, orxCONST orxVEC *_pvPos, orxARGB _stColor, orxCONST orxSTRING _zText)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_DrawText)(_pstBitmap, _pvPos, _stColor, _zText);
}

orxSTATIC orxINLINE orxBITMAP *orxDisplay_CreateBitmap(orxCONST orxVEC *_pvSize, orxU32 _u32Flags)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_CreateBitmap)(_pvSize, _u32Flags);
}

orxSTATIC orxINLINE orxVOID orxDisplay_DeleteBitmap(orxBITMAP *_pstBitmap)
{
  orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_DeleteBitmap)(_pstBitmap);
}

orxSTATIC orxINLINE orxBITMAP *orxDisplay_GetScreenBitmap()
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_GetScreenBitmap)();
}

orxSTATIC orxINLINE orxSTATUS orxDisplay_ClearBitmap(orxBITMAP *_pstBitmap, orxARGB _stColor)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_ClearBitmap)(_pstBitmap, _stColor);
}

orxSTATIC orxINLINE orxSTATUS orxDisplay_TransformBitmap(orxBITMAP *_pstDst, orxCONST orxBITMAP *_pstSrc, orxCONST orxBITMAP_TRANSFORM *_pstTransform, orxU32 _u32Flags)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_TransformBitmap)(_pstDst, _pstSrc, _pstTransform, _u32Flags);
}

orxSTATIC orxINLINE orxSTATUS orxDisplay_SetBitmapColorKey(orxBITMAP *_pstBitmap, orxARGB _stColor, orxBOOL _bEnable)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_SetBitmapColorKey)(_pstBitmap, _stColor, _bEnable);
}

orxSTATIC orxINLINE orxSTATUS orxDisplay_SetBitmapClipping(orxBITMAP *_pstBitmap, orxCONST orxVEC *_pvTL, orxCONST orxVEC *_pvBR)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_SetBitmapClipping)(_pstBitmap, _pvTL, _pvBR);
}

orxSTATIC orxINLINE orxSTATUS orxDisplay_BlitBitmap(orxBITMAP *_pstDst, orxCONST orxBITMAP *_pstSrc, orxCONST orxVEC *_pvDstCoord, orxCONST orxVEC *_pvSrcCoord, orxCONST orxVEC *_pvSize)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_BlitBitmap)(_pstDst, _pstSrc, _pvDstCoord, _pvSrcCoord, _pvSize);
}

orxSTATIC orxINLINE orxSTATUS orxDisplay_SaveBitmap(orxCONST orxBITMAP *_pstBitmap, orxCONST orxSTRING _zFileName)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_SaveBitmap)(_pstBitmap, _zFileName);
}

orxSTATIC orxINLINE orxBITMAP *orxDisplay_LoadBitmap(orxCONST orxSTRING _zFileName)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_LoadBitmap)(_zFileName);
}

orxSTATIC orxINLINE orxSTATUS orxDisplay_GetBitmapSize(orxCONST orxBITMAP *_pstBitmap, orxVEC *_pvSize)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_GetBitmapSize)(_pstBitmap, _pvSize);
}


#endif /* _orxDISPLAY_H_ */
