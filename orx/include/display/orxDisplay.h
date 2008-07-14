/***************************************************************************
 *                                                                         *
 *   This library is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License           *
 *   as published by the Free Software Foundation; either version 2.1      *
 *   of the License, or (at your option) any later version.                *
 *                                                                         *
 ***************************************************************************/

/**
 * @file orxDisplay.h
 * @date 23/04/2003
 * @author (C) Arcallians
 */

/**
 * @addtogroup Display
 * 
 * Display plugin module
 * Plugin module that handles display
 *
 * @{
 */


#ifndef _orxDISPLAY_H_
#define _orxDISPLAY_H_

#include "orxInclude.h"
#include "plugin/orxPluginCore.h"

#include "math/orxVector.h"
#include "utils/orxString.h"


typedef struct __orxBITMAP_t            orxBITMAP;

/** Transform structure
 */
typedef struct __orxBITMAP_TRANSFORM_t
{
  orxFLOAT  fSrcX, fSrcY, fDstX, fDstY;
  orxFLOAT  fRotation;
  orxFLOAT  fScaleX;
  orxFLOAT  fScaleY;

} orxBITMAP_TRANSFORM;

/** Color structure
 */
typedef struct __orxCOLOR_t
{
  orxVECTOR vRGB;                       /**< RGB components: 12 */
  orxFLOAT  fAlpha;                     /**< Alpha component: 16 */

} orxCOLOR;


#define orxDISPLAY_KZ_CONFIG_SECTION    "Display"
#define orxDISPLAY_KZ_CONFIG_WIDTH      "ScreenWidth"
#define orxDISPLAY_KZ_CONFIG_HEIGHT     "ScreenHeight"
#define orxDISPLAY_KZ_CONFIG_DEPTH      "ScreenDepth"
#define orxDISPLAY_KZ_CONFIG_FONT       "Font"
#define orxDISPLAY_KZ_CONFIG_TITLE      "Title"


/***************************************************************************
 * Functions directly implemented by orx core
 ***************************************************************************/

/** Display module setup
 */
extern orxDLLAPI orxVOID            orxDisplay_Setup();

/** Sets all components from an orxRGBA
 * @param[in]   _pstColor       Concerned color
 * @return      orxCOLOR
 */
orxSTATIC orxINLINE orxCOLOR *      orxColor_SetRGBA(orxCOLOR *_pstColor, orxRGBA _stRGBA)
{
  orxCOLOR *pstResult = _pstColor;

  /* Checks */
  orxASSERT(_pstColor != orxNULL);

  /* Stores RGB */
  orxVector_Set(&(_pstColor->vRGB), orxRGBA_R(_stRGBA), orxRGBA_G(_stRGBA), orxRGBA_B(_stRGBA));

  /* Stores alpha */
  _pstColor->fAlpha = orxRGBA_A(_stRGBA) * orxRGBA_NORMALIZER;

  /* Done! */
  return pstResult;
}

/** Sets all components
 * @param[in]   _pstColor       Concerned color
 * @param[in]   _pvRGB          RGB components
 * @param[in]   _fAlpha         Normalized alpha component
 * @return      orxCOLOR
 */
orxSTATIC orxINLINE orxCOLOR *      orxColor_Set(orxCOLOR *_pstColor, orxCONST orxVECTOR *_pvRGB, orxFLOAT _fAlpha)
{
  orxCOLOR *pstResult = _pstColor;

  /* Checks */
  orxASSERT(_pstColor != orxNULL);

  /* Stores RGB */
  orxVector_Copy(&(_pstColor->vRGB), _pvRGB);

  /* Stores alpha */
  _pstColor->fAlpha = _fAlpha;

  /* Done! */
  return pstResult;
}

/** Sets RGB components
 * @param[in]   _pstColor       Concerned color
 * @param[in]   _pvRGB          RGB components
 * @return      orxCOLOR
 */
orxSTATIC orxINLINE orxCOLOR *      orxColor_SetRGB(orxCOLOR *_pstColor, orxCONST orxVECTOR *_pvRGB)
{
  orxCOLOR *pstResult = _pstColor;

  /* Checks */
  orxASSERT(_pstColor != orxNULL);
  orxASSERT(_pvRGB != orxNULL);

  /* Stores components */
  orxVector_Copy(&(_pstColor->vRGB), _pvRGB);

  /* Done! */
  return pstResult;
}

/** Sets alpha component
 * @param[in]   _pstColor       Concerned color
 * @param[in]   _fAlpha         Normalized alpha component
 * @return      orxCOLOR / orxNULL
 */
orxSTATIC orxINLINE orxCOLOR *      orxColor_SetAlpha(orxCOLOR *_pstColor, orxFLOAT _fAlpha)
{
  orxCOLOR *pstResult = _pstColor;

  /* Checks */
  orxASSERT(_pstColor != orxNULL);

  /* Stores it */
  _pstColor->fAlpha = _fAlpha;

  /* Done! */
  return pstResult;
}

/** Gets orxRGBA from an orxCOLOR
 * @param[in]   _pstColor       Concerned color
 * @return      orxRGBA
 */
orxSTATIC orxINLINE orxRGBA orxColor_ToRGBA(orxCONST orxCOLOR *_pstColor)
{
  orxRGBA   stResult;
  orxVECTOR vColor;
  orxFLOAT  fAlpha;

  /* Checks */
  orxASSERT(_pstColor != orxNULL);

  /* Clamps RGB components */
  orxVector_Clamp(&vColor, &(_pstColor->vRGB), &orxVECTOR_0, &orxVECTOR_WHITE);

  /* Clamps alpha */
  fAlpha = orxCLAMP(_pstColor->fAlpha, orxFLOAT_0, orxFLOAT_1);

  /* Updates result */
  stResult = orx2RGBA(orxF2U(vColor.fR), orxF2U(vColor.fG), orxF2U(vColor.fB), orxF2U(255.0f * fAlpha));

  /* Done! */
  return stResult;
}

/** Copies an orxCOLOR into another one
 * @param[in]   _pstDst         Destination color
 * @param[in]   _pstSrc         Source color
 * @return      orxCOLOR
 */
orxSTATIC orxINLINE orxCOLOR * orxColor_Copy(orxCOLOR *_pstDst, orxCONST orxCOLOR *_pstSrc)
{
  /* Checks */
  orxASSERT(_pstDst != orxNULL);
  orxASSERT(_pstSrc != orxNULL);

  /* Copies it */
  orxMemory_Copy(_pstDst, _pstSrc, sizeof(orxCOLOR));

  /* Done! */
  return _pstDst;
}


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
orxPLUGIN_DECLARE_CORE_FUNCTION(orxDisplay_GetScreenSize, orxSTATUS, orxFLOAT *, orxFLOAT *);

orxPLUGIN_DECLARE_CORE_FUNCTION(orxDisplay_ClearBitmap, orxSTATUS, orxBITMAP *, orxRGBA);
orxPLUGIN_DECLARE_CORE_FUNCTION(orxDisplay_TransformBitmap, orxSTATUS, orxBITMAP *, orxCONST orxBITMAP *, orxCONST orxBITMAP_TRANSFORM *, orxU32);

orxPLUGIN_DECLARE_CORE_FUNCTION(orxDisplay_SetBitmapColorKey, orxSTATUS, orxBITMAP *, orxRGBA, orxBOOL);
orxPLUGIN_DECLARE_CORE_FUNCTION(orxDisplay_SetBitmapColor, orxSTATUS, orxBITMAP *, orxRGBA);
orxPLUGIN_DECLARE_CORE_FUNCTION(orxDisplay_SetBitmapClipping, orxSTATUS, orxBITMAP *, orxU32, orxU32, orxU32, orxU32);

orxPLUGIN_DECLARE_CORE_FUNCTION(orxDisplay_BlitBitmap, orxSTATUS, orxBITMAP *, orxCONST orxBITMAP *, orxFLOAT, orxFLOAT);

orxPLUGIN_DECLARE_CORE_FUNCTION(orxDisplay_SaveBitmap, orxSTATUS, orxCONST orxBITMAP *, orxCONST orxSTRING);
orxPLUGIN_DECLARE_CORE_FUNCTION(orxDisplay_LoadBitmap, orxBITMAP *, orxCONST orxSTRING);

orxPLUGIN_DECLARE_CORE_FUNCTION(orxDisplay_GetBitmapColor, orxRGBA, orxCONST orxBITMAP *);
orxPLUGIN_DECLARE_CORE_FUNCTION(orxDisplay_GetBitmapSize, orxSTATUS, orxCONST orxBITMAP *, orxFLOAT *, orxFLOAT *);

orxPLUGIN_DECLARE_CORE_FUNCTION(orxDisplay_GetApplicationInput, orxHANDLE);
orxPLUGIN_DECLARE_CORE_FUNCTION(orxDisplay_EnableVSync, orxSTATUS, orxBOOL);



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

orxSTATIC orxINLINE orxSTATUS orxDisplay_GetScreenSize(orxFLOAT *_pfWidth, orxFLOAT *_pfHeight)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_GetScreenSize)(_pfWidth, _pfHeight);
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

orxSTATIC orxINLINE orxSTATUS orxDisplay_SetBitmapColor(orxBITMAP *_pstBitmap, orxRGBA _stColor)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_SetBitmapColor)(_pstBitmap, _stColor);
}

orxSTATIC orxINLINE orxSTATUS orxDisplay_SetBitmapClipping(orxBITMAP *_pstBitmap, orxU32 _u32TLX, orxU32 _u32TLY, orxU32 _u32BRX, orxU32 _u32BRY)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_SetBitmapClipping)(_pstBitmap, _u32TLX, _u32TLY, _u32BRX, _u32BRY);
}

orxSTATIC orxINLINE orxSTATUS orxDisplay_BlitBitmap(orxBITMAP *_pstDst, orxCONST orxBITMAP *_pstSrc, orxFLOAT _fPosX, orxFLOAT _fPosY)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_BlitBitmap)(_pstDst, _pstSrc, _fPosX, _fPosY);
}

orxSTATIC orxINLINE orxSTATUS orxDisplay_SaveBitmap(orxCONST orxBITMAP *_pstBitmap, orxCONST orxSTRING _zFileName)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_SaveBitmap)(_pstBitmap, _zFileName);
}

orxSTATIC orxINLINE orxBITMAP *orxDisplay_LoadBitmap(orxCONST orxSTRING _zFileName)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_LoadBitmap)(_zFileName);
}

orxSTATIC orxINLINE orxSTATUS orxDisplay_GetBitmapSize(orxCONST orxBITMAP *_pstBitmap, orxFLOAT *_pfWidth, orxFLOAT *_pfHeight)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_GetBitmapSize)(_pstBitmap, _pfWidth, _pfHeight);
}

orxSTATIC orxINLINE orxRGBA orxDisplay_GetBitmapColor(orxCONST orxBITMAP *_pstBitmap)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_GetBitmapColor)(_pstBitmap);
}

orxSTATIC orxINLINE orxHANDLE orxDisplay_GetApplicationInput()
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_GetApplicationInput)();
}

orxSTATIC orxINLINE orxSTATUS orxDisplay_EnableVSync(orxBOOL _bEnable)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxDisplay_EnableVSync)(_bEnable);
}


#endif /* _orxDISPLAY_H_ */
