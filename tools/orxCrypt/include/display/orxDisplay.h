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
 * @file orxDisplay.h
 * @date 23/04/2003
 * @author iarwain@orx-project.org
 *
 * @todo
 */

/**
 * @addtogroup orxDisplay
 * 
 * Display plugin module
 * Module that handles display
 *
 * @{
 */


#ifndef _orxDISPLAY_H_
#define _orxDISPLAY_H_

#include "orxInclude.h"
#include "plugin/orxPluginCore.h"

#include "math/orxVector.h"
#include "utils/orxString.h"
#include "utils/orxLinkList.h"


/** Misc defines
 */
typedef orxU32                      orxRGBA;
#define orx2RGBA(R, G, B, A)        ((((R) & 0xFF) << 24) | (((G) & 0xFF) << 16) | (((B) & 0xFF) << 8) | ((A) & 0xFF))
#define orxRGBA_R(RGBA)             (orxU8)(((RGBA) >> 24) & 0xFF)
#define orxRGBA_G(RGBA)             (orxU8)(((RGBA) >> 16) & 0xFF)
#define orxRGBA_B(RGBA)             (orxU8)(((RGBA) >> 8) & 0xFF)
#define orxRGBA_A(RGBA)             (orxU8)((RGBA) & 0xFF)

#define orxCOLOR_NORMALIZER         (orx2F(1.0f / 255.0f))
#define orxCOLOR_DENORMALIZER       (orx2F(255.0f))


typedef struct __orxBITMAP_t        orxBITMAP;
typedef struct __orxDISPLAY_TEXT_t  orxDISPLAY_TEXT;

/** Transform structure
 */
typedef struct __orxDISPLAY_TRANSFORM_t
{
  orxFLOAT  fSrcX, fSrcY, fDstX, fDstY;
  orxFLOAT  fRotation;
  orxFLOAT  fScaleX;
  orxFLOAT  fScaleY;

} orxDISPLAY_TRANSFORM;

/** Video mode structure
 */
typedef struct __orxDISPLAY_VIDEO_MODE_t
{
  orxU32  u32Width, u32Height, u32Depth;

} orxDISPLAY_VIDEO_MODE;

/** Bitmap smoothing enum
 */
typedef enum __orxDISPLAY_SMOOTHING_t
{
  orxDISPLAY_SMOOTHING_DEFAULT = 0,
  orxDISPLAY_SMOOTHING_ON,
  orxDISPLAY_SMOOTHING_OFF,

  orxDISPLAY_SMOOTHING_NUMBER,

  orxDISPLAY_SMOOTHING_NONE = orxENUM_NONE

} orxDISPLAY_SMOOTHING;

/** Bitmap blend enum
 */
typedef enum __orxDISPLAY_BLEND_MODE_t
{
  orxDISPLAY_BLEND_MODE_ALPHA = 0,
  orxDISPLAY_BLEND_MODE_MULTIPLY,
  orxDISPLAY_BLEND_MODE_ADD,

  orxDISPLAY_BLEND_MODE_NUMBER,

  orxDISPLAY_BLEND_MODE_NONE = orxENUM_NONE

} orxDISPLAY_BLEND_MODE;

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
#define orxDISPLAY_KZ_CONFIG_FULLSCREEN "FullScreen"
#define orxDISPLAY_KZ_CONFIG_DECORATION "Decoration"
#define orxDISPLAY_KZ_CONFIG_FONT       "Font"
#define orxDISPLAY_KZ_CONFIG_TITLE      "Title"
#define orxDISPLAY_KZ_CONFIG_SMOOTH     "Smoothing"
#define orxDISPLAY_KZ_CONFIG_VSYNC      "VSync"


/***************************************************************************
 * Functions directly implemented by orx core
 ***************************************************************************/

/** Display module setup
 */
extern orxDLLAPI void orxFASTCALL orxDisplay_Setup();

/** Sets all components from an orxRGBA
 * @param[in]   _pstColor       Concerned color
 * @param[in]   _stRGBA         RGBA values to set
 * @return      orxCOLOR
 */
static orxINLINE orxCOLOR *       orxColor_SetRGBA(orxCOLOR *_pstColor, orxRGBA _stRGBA)
{
  orxCOLOR *pstResult = _pstColor;

  /* Checks */
  orxASSERT(_pstColor != orxNULL);

  /* Stores RGB */
  orxVector_Set(&(_pstColor->vRGB), orxCOLOR_NORMALIZER * orxU2F(orxRGBA_R(_stRGBA)), orxCOLOR_NORMALIZER * orxU2F(orxRGBA_G(_stRGBA)), orxCOLOR_NORMALIZER * orxU2F(orxRGBA_B(_stRGBA)));

  /* Stores alpha */
  _pstColor->fAlpha = orxCOLOR_NORMALIZER * orxRGBA_A(_stRGBA);

  /* Done! */
  return pstResult;
}

/** Sets all components
 * @param[in]   _pstColor       Concerned color
 * @param[in]   _pvRGB          RGB components
 * @param[in]   _fAlpha         Normalized alpha component
 * @return      orxCOLOR
 */
static orxINLINE orxCOLOR *       orxColor_Set(orxCOLOR *_pstColor, const orxVECTOR *_pvRGB, orxFLOAT _fAlpha)
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
static orxINLINE orxCOLOR *       orxColor_SetRGB(orxCOLOR *_pstColor, const orxVECTOR *_pvRGB)
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
static orxINLINE orxCOLOR *       orxColor_SetAlpha(orxCOLOR *_pstColor, orxFLOAT _fAlpha)
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
static orxINLINE orxRGBA          orxColor_ToRGBA(const orxCOLOR *_pstColor)
{
  orxRGBA   stResult;
  orxVECTOR vColor;
  orxFLOAT  fAlpha;

  /* Checks */
  orxASSERT(_pstColor != orxNULL);

  /* Clamps RGB components */
  orxVector_Clamp(&vColor, &(_pstColor->vRGB), &orxVECTOR_0, &orxVECTOR_WHITE);

  /* De-normalizes vector */
  orxVector_Mulf(&vColor, &vColor, orxCOLOR_DENORMALIZER);

  /* Clamps alpha */
  fAlpha = orxCLAMP(_pstColor->fAlpha, orxFLOAT_0, orxFLOAT_1);

  /* Updates result */
  stResult = orx2RGBA(orxF2U(vColor.fR), orxF2U(vColor.fG), orxF2U(vColor.fB), orxF2U(orxCOLOR_DENORMALIZER * fAlpha));

  /* Done! */
  return stResult;
}

/** Copies an orxCOLOR into another one
 * @param[in]   _pstDst         Destination color
 * @param[in]   _pstSrc         Source color
 * @return      orxCOLOR
 */
static orxINLINE orxCOLOR *       orxColor_Copy(orxCOLOR *_pstDst, const orxCOLOR *_pstSrc)
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

/** Inits the display module
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxDisplay_Init();

/** Exits from the display module
 */
extern orxDLLAPI void orxFASTCALL                     orxDisplay_Exit();


/** Swaps/flips bufers (display on screen the current frame)
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxDisplay_Swap();


/** Creates an empty text
 * @return orxDISPLAY_TEXT / orxNULL
 */
extern orxDLLAPI orxDISPLAY_TEXT *orxFASTCALL         orxDisplay_CreateText();

/** Deletes a text
 * @param[in]   _pstText                              Concerned text
 */
extern orxDLLAPI void orxFASTCALL                     orxDisplay_DeleteText(orxDISPLAY_TEXT *_pstText);

/** Transforms a text (on a bitmap)
 * @param[in]   _pstDst                               Destination bitmap
 * @param[in]   _pstText                              Text to transform (display)
 * @param[in]   _pstTransform                         Transformation info (positions, scale, rotation, ...)
 * @param[in]   _stColor                              Color to use
 * @param[in]   _eBlendMode                           Blend mode
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxDisplay_TransformText(orxBITMAP *_pstDst, const orxDISPLAY_TEXT *_pstText, const orxDISPLAY_TRANSFORM *_pstTransform, orxRGBA _stColor, orxDISPLAY_BLEND_MODE _eBlendMode);

/** Sets a text string
 * @param[in]   _pstText                              Concerned text
 * @param[in]   _zString                              String to set
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxDisplay_SetTextString(orxDISPLAY_TEXT *_pstText, const orxSTRING _zString);

/** Sets a text font
 * @param[in]   _pstText                              Concerned text
 * @param[in]   _zFont                                Font to set
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxDisplay_SetTextFont(orxDISPLAY_TEXT *_pstText, const orxSTRING _zFont);

/** Gets a text string
 * @param[in]   _pstText                              Concerned text
 * @return orxSTRING / orxNULL
 */
extern orxDLLAPI const orxSTRING orxFASTCALL          orxDisplay_GetTextString(const orxDISPLAY_TEXT *_pstText);

/** Gets a text font
 * @param[in]   _pstText                              Concerned text
 * @return orxSTRING / orxNULL
 */
extern orxDLLAPI const orxSTRING orxFASTCALL          orxDisplay_GetTextFont(const orxDISPLAY_TEXT *_pstText);

/** Gets a text size
 * @param[in]   _pstText                              Concerned text
 * @param[out]  _pfWidth                              Text's width
 * @param[out]  _pfHeight                             Text's height
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxDisplay_GetTextSize(const orxDISPLAY_TEXT *_pstText, orxFLOAT *_pfWidth, orxFLOAT *_pfHeight);


/** Prints a string
 * @param[in]   _pstBitmap                            Concerned bitmap
 * @param[in]   _zString                              String to display
 * @param[in]   _pstTransform                         Transformation info (positions, scale, rotation, ...)
 * @param[in]   _stColor                              Color to use for the text
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxDisplay_PrintString(const orxBITMAP *_pstBitmap, const orxSTRING _zString, const orxDISPLAY_TRANSFORM *_pstTransform, orxRGBA _stColor);


/** Creates a bitmap
 * @param[in]   _u32Width                             Bitmap width
 * @param[in]   _u32Height                            Bitmap height
 * @return orxBITMAP / orxNULL
 */
extern orxDLLAPI orxBITMAP *orxFASTCALL               orxDisplay_CreateBitmap(orxU32 _u32Width, orxU32 _u32Height);

/** Deletes a bitmap
 * @param[in]   _pstBitmap                            Concerned bitmap
 */
extern orxDLLAPI void orxFASTCALL                     orxDisplay_DeleteBitmap(orxBITMAP *_pstBitmap);


/** Gets screen bitmap
 * @return orxBITMAP / orxNULL
 */
extern orxDLLAPI orxBITMAP *orxFASTCALL               orxDisplay_GetScreenBitmap();

/** Gets screen size
 * @param[out]   _pfWidth                             Screen width
 * @param[out]   _pfHeight                            Screen height
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxDisplay_GetScreenSize(orxFLOAT *_pfWidth, orxFLOAT *_pfHeight);


/** Clears a bitmap
 * @param[in]   _pstBitmap                            Concerned bitmap
 * @param[in]   _stColor                              Color to clear the bitmap with
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxDisplay_ClearBitmap(orxBITMAP *_pstBitmap, orxRGBA _stColor);

/** Transforms (and blits onto another) a bitmap
 * @param[in]   _pstDst                               Bitmap where to blit the result, can be screen
 * @param[in]   _pstSrc                               Bitmap to transform and draw
 * @param[in]   _pstTransform                         Transformation info (positions, scale, rotation, ...)
 * @param[in]   _eSmoothing                           Bitmap smoothing type
 * @param[in]   _eBlendMode                           Blend mode
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxDisplay_TransformBitmap(orxBITMAP *_pstDst, const orxBITMAP *_pstSrc, const orxDISPLAY_TRANSFORM *_pstTransform, orxDISPLAY_SMOOTHING _eSmoothing, orxDISPLAY_BLEND_MODE _eBlendMode);


/** Sets a bitmap color key (used with non alpha transparency)
 * @param[in]   _pstBitmap                            Concerned bitmap
 * @param[in]   _stColor                              Color to use as transparent one
 * @param[in]   _bEnable                              Enable / disable transparence for this color
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxDisplay_SetBitmapColorKey(orxBITMAP *_pstBitmap, orxRGBA _stColor, orxBOOL _bEnable);

/** Sets a bitmap color (lighting/hue)
 * @param[in]   _pstBitmap                            Concerned bitmap
 * @param[in]   _stColor                              Color to apply on the bitmap
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxDisplay_SetBitmapColor(orxBITMAP *_pstBitmap, orxRGBA _stColor);

/** Sets a bitmap clipping for blitting (both as source and destination)
 * @param[in]   _pstBitmap                            Concerned bitmap
 * @param[in]   _u32TLX                               Top left X coord in pixels
 * @param[in]   _u32TLY                               Top left Y coord in pixels
 * @param[in]   _u32BRX                               Bottom right X coord in pixels
 * @param[in]   _u32BRY                               Bottom right Y coord in pixels
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxDisplay_SetBitmapClipping(orxBITMAP *_pstBitmap, orxU32 _u32TLX, orxU32 _u32TLY, orxU32 _u32BRX, orxU32 _u32BRY);


/** Blits a bitmap (no transformation)
 * @param[in]   _pstDst                               Bitmap where to blit
 * @param[in]   _pstSrc                               Bitmap to blit (will begin at top left corner)
 * @param[in]   _fPosX                                X-axis value of the position where to blit the source bitmap
 * @param[in]   _fPosY                                Y-axis value of the position where to blit the source bitmap
 * @param[in]   _eBlendMode                           Blend mode
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxDisplay_BlitBitmap(orxBITMAP *_pstDst, const orxBITMAP *_pstSrc, orxFLOAT _fPosX, orxFLOAT _fPosY, orxDISPLAY_BLEND_MODE _eBlendMode);


/** Saves a bitmap to file
 * @param[in]   _pstBitmap                            Concerned bitmap
 * @param[in]   _zFileName                            Name of the file where to store the bitmap
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxDisplay_SaveBitmap(const orxBITMAP *_pstBitmap, const orxSTRING _zFileName);

/** Loads a bitmap from file
 * @param[in]   _zFileName                            Name of the file to load
 * @return orxBITMAP * / orxNULL
 */
extern orxDLLAPI orxBITMAP *orxFASTCALL               orxDisplay_LoadBitmap(const orxSTRING _zFileName);


/** Gets a bitmap size
 * @param[in]   _pstBitmap                            Concerned bitmap
 * @param[out]   _pfWidth                             Bitmap width
 * @param[out]   _pfHeight                            Bitmap height
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxDisplay_GetBitmapSize(const orxBITMAP *_pstBitmap, orxFLOAT *_pfWidth, orxFLOAT *_pfHeight);

/** Gets bitmap color (lighting/hue)
 * @param[in]   _pstBitmap                            Concerned bitmap
 * @return orxRGBA
 */
extern orxDLLAPI orxRGBA orxFASTCALL                  orxDisplay_GetBitmapColor(const orxBITMAP *_pstBitmap);


/** Creates (compiles) a shader
 * @param[in]   _zCode                                Shader code to compile
 * @param[in]   _pstParamList                         Shader parameters (should be a link list of orxSHADER_PARAM)
 * @return orxHANDLE of the compiled shader is successful, orxHANDLE_UNDEFINED otherwise
 */
extern orxDLLAPI orxHANDLE orxFASTCALL                orxDisplay_CreateShader(const orxSTRING _zCode, const orxLINKLIST *_pstParamList);

/** Deletes a compiled shader
 * @param[in]   _hShader                              Shader to delete
 */
extern orxDLLAPI void orxFASTCALL                     orxDisplay_DeleteShader(orxHANDLE _hShader);

/** Renders a shader
 * @param[in]   _hShader                              Shader to render
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxDisplay_RenderShader(const orxHANDLE _hShader);

/** Sets a shader parameter (orxBITMAP)
 * @param[in]   _hShader                              Concerned shader
 * @param[in]   _zParam                               Parameter to set
 * @param[in]   _pstValue                             Value (orxBITMAP) for this parameter
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxDisplay_SetShaderBitmap(orxHANDLE _hShader, const orxSTRING _zParam, orxBITMAP *_pstValue);

/** Sets a shader parameter (orxFLOAT)
 * @param[in]   _hShader                              Concerned shader
 * @param[in]   _zParam                               Parameter to set
 * @param[in]   _fValue                               Value (orxFLOAT) for this parameter
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxDisplay_SetShaderFloat(orxHANDLE _hShader, const orxSTRING _zParam, orxFLOAT _fValue);

/** Sets a shader parameter (orxVECTOR)
 * @param[in]   _hShader                              Concerned shader
 * @param[in]   _zParam                               Parameter to set
 * @param[in]   _pvValue                              Value (orxVECTOR) for this parameter
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxDisplay_SetShaderVector(orxHANDLE _hShader, const orxSTRING _zParam, const orxVECTOR *_pvValue);


/** Gets application input manager (if embedded with display)
 * @return orxHANDLE / orxHANDLE_UNDEFINED
 */
extern orxDLLAPI orxHANDLE orxFASTCALL                orxDisplay_GetApplicationInput();


/** Enables / disables vertical synchro
 * @param[in]   _bEnable                              Enable / disable
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxDisplay_EnableVSync(orxBOOL _bEnable);

/** Is vertical synchro enabled?
 * @return orxTRUE if enabled, orxFALSE otherwise
 */
extern orxDLLAPI orxBOOL orxFASTCALL                  orxDisplay_IsVSyncEnabled();


/** Sets full screen mode
 * @param[in]   _bFullScreen                          orxTRUE / orxFALSE
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxDisplay_SetFullScreen(orxBOOL _bFullScreen);

/** Is in full screen mode?
 * @return orxTRUE if full screen, orxFALSE otherwise
 */
extern orxDLLAPI orxBOOL orxFASTCALL                  orxDisplay_IsFullScreen();


/** Gets available video mode counter
 * @return Available video mode counter
 */
extern orxDLLAPI orxU32 orxFASTCALL                   orxDisplay_GetVideoModeCounter();

/** Gets an available video mode
 * @param[in]   _u32Index                             Video mode index, must be lesser than orxDisplay_GetVideoModeCounter()
 * @param[out]  _pstVideoMode                         Storage for the video mode
 * @return orxDISPLAY_VIDEO_MODE / orxNULL if invalid
 */
extern orxDLLAPI orxDISPLAY_VIDEO_MODE *orxFASTCALL   orxDisplay_GetVideoMode(orxU32 _u32Index, orxDISPLAY_VIDEO_MODE *_pstVideoMode);

/** Gets an available video mode
 * @param[in]  _pstVideoMode                          Video mode to set
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxDisplay_SetVideoMode(const orxDISPLAY_VIDEO_MODE *_pstVideoMode);

/** Is video mode available
 * @param[in]  _pstVideoMode                          Video mode to test
 * @return orxTRUE is available, orxFALSE otherwise
 */
extern orxDLLAPI orxBOOL orxFASTCALL                  orxDisplay_IsVideoModeAvailable(const orxDISPLAY_VIDEO_MODE *_pstVideoMode);


#endif /* _orxDISPLAY_H_ */

/** @} */
