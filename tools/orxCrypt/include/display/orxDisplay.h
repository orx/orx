/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2012 Orx-Project
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 *    1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 *
 *    2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 *    3. This notice may not be removed or altered from any source
 *    distribution.
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
#include "memory/orxBank.h"
#include "utils/orxHashTable.h"
#include "utils/orxString.h"
#include "utils/orxLinkList.h"


/** Misc defines
 */
typedef orxU32                      orxRGBA;

#ifdef __orxBIG_ENDIAN__

  #define orx2RGBA(R, G, B, A)      ((((R) & 0xFF) << 24) | (((G) & 0xFF) << 16) | (((B) & 0xFF) << 8) | ((A) & 0xFF))
  #define orxRGBA_R(RGBA)           (orxU8)(((RGBA) >> 24) & 0xFF)
  #define orxRGBA_G(RGBA)           (orxU8)(((RGBA) >> 16) & 0xFF)
  #define orxRGBA_B(RGBA)           (orxU8)(((RGBA) >> 8) & 0xFF)
  #define orxRGBA_A(RGBA)           (orxU8)((RGBA) & 0xFF)

#else /* __orxBIG_ENDIAN__ */

  #define orx2RGBA(R, G, B, A)      ((((A) & 0xFF) << 24) | (((B) & 0xFF) << 16) | (((G) & 0xFF) << 8) | ((R) & 0xFF))
  #define orxRGBA_R(RGBA)           (orxU8)((RGBA) & 0xFF)
  #define orxRGBA_G(RGBA)           (orxU8)(((RGBA) >> 8) & 0xFF)
  #define orxRGBA_B(RGBA)           (orxU8)(((RGBA) >> 16) & 0xFF)
  #define orxRGBA_A(RGBA)           (orxU8)(((RGBA) >> 24) & 0xFF)

#endif /* __orxBIG_ENDIAN__ */

#define orxCOLOR_NORMALIZER         (orx2F(1.0f / 255.0f))
#define orxCOLOR_DENORMALIZER       (orx2F(255.0f))


typedef struct __orxBITMAP_t        orxBITMAP;

/** Transform structure
 */
typedef struct __orxDISPLAY_TRANSFORM_t
{
  orxFLOAT  fSrcX, fSrcY, fDstX, fDstY;
  orxFLOAT  fRepeatX, fRepeatY;
  orxFLOAT  fScaleX;
  orxFLOAT  fScaleY;
  orxFLOAT  fRotation;

} orxDISPLAY_TRANSFORM;

/** Video mode structure
 */
typedef struct __orxDISPLAY_VIDEO_MODE_t
{
  orxU32  u32Width, u32Height, u32Depth;

} orxDISPLAY_VIDEO_MODE;

/** Character glyph structure
 */
typedef struct __orxCHARACTER_GLYPH_t
{
  orxFLOAT fX, fY, fWidth;

} orxCHARACTER_GLYPH;

/** Character map structure
 */
typedef struct __orxCHARACTER_MAP_t
{
  orxFLOAT      fCharacterHeight;

  orxBANK      *pstCharacterBank;
  orxHASHTABLE *pstCharacterTable;

} orxCHARACTER_MAP;

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
  union
  {
    orxVECTOR vRGB;                     /**< RGB components: 12 */
    orxVECTOR vHSL;                     /**< HSL components: 12 */
    orxVECTOR vHSV;                     /**< HSV components: 12 */
  };
  orxFLOAT  fAlpha;                     /**< Alpha component: 16 */

} orxCOLOR;


/** Config parameters
 */
#define orxDISPLAY_KZ_CONFIG_SECTION        "Display"
#define orxDISPLAY_KZ_CONFIG_WIDTH          "ScreenWidth"
#define orxDISPLAY_KZ_CONFIG_HEIGHT         "ScreenHeight"
#define orxDISPLAY_KZ_CONFIG_DEPTH          "ScreenDepth"
#define orxDISPLAY_KZ_CONFIG_FULLSCREEN     "FullScreen"
#define orxDISPLAY_KZ_CONFIG_ALLOW_RESIZE   "AllowResize"
#define orxDISPLAY_KZ_CONFIG_DECORATION     "Decoration"
#define orxDISPLAY_KZ_CONFIG_TITLE          "Title"
#define orxDISPLAY_KZ_CONFIG_SMOOTH         "Smoothing"
#define orxDISPLAY_KZ_CONFIG_VSYNC          "VSync"
#define orxDISPLAY_KZ_CONFIG_DEPTHBUFFER    "DepthBuffer"


/** Shader texture suffixes
 */
#define orxDISPLAY_KZ_SHADER_SUFFIX_TOP     "_top"
#define orxDISPLAY_KZ_SHADER_SUFFIX_LEFT    "_left"
#define orxDISPLAY_KZ_SHADER_SUFFIX_BOTTOM  "_bottom"
#define orxDISPLAY_KZ_SHADER_SUFFIX_RIGHT   "_right"


/** Event enum
 */
typedef enum __orxDISPLAY_EVENT_t
{
  orxDISPLAY_EVENT_SET_VIDEO_MODE = 0,

#if defined(__orxANDROID_NATIVE__) || defined (__orxANDROID__)

  orxDISPLAY_EVENT_SAVE_CONTEXT,
  orxDISPLAY_EVENT_RESTORE_CONTEXT,

#endif /* __orxANDROID_NATIVE__ || __orxANDROID__ */

  orxDISPLAY_EVENT_NUMBER,

  orxDISPLAY_EVENT_NONE = orxENUM_NONE

} orxDISPLAY_EVENT;

/** Display event payload
 */
typedef struct __orxDISPLAY_EVENT_PAYLOAD_t
{
  orxU32  u32Width;                                     /**< Screen width : 4 */
  orxU32  u32Height;                                    /**< Screen height : 8 */
  orxU32  u32Depth;                                     /**< Screen depth : 12 */
  orxU32  u32PreviousWidth;                             /**< Previous screen width : 16 */
  orxU32  u32PreviousHeight;                            /**< Previous screen height : 20 */
  orxU32  u32PreviousDepth;                             /**< Previous screen depth : 24 */
  orxBOOL bFullScreen;                                  /**< FullScreen? : 28 */

} orxDISPLAY_EVENT_PAYLOAD;


/***************************************************************************
 * Functions directly implemented by orx core
 ***************************************************************************/

#if defined(__orxIPHONE__) && defined(__orxOBJC__)

#import <UIKit/UIKit.h>
#import <OpenGLES/EAGL.h>
#import <OpenGLES/ES1/gl.h>
#import <OpenGLES/ES1/glext.h>
#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>

/** Orx view interface
 */
@interface orxView : UIView
{
@private
  EAGLContext  *poMainContext, *poThreadContext;
  GLuint        uiRenderBuffer, uiDepthBuffer, uiScreenFrameBuffer, uiTextureFrameBuffer;
  BOOL          bShaderSupport, bCompressedTextureSupport;
}

@property (nonatomic, retain) EAGLContext  *poMainContext;
@property (nonatomic, retain) EAGLContext  *poThreadContext;
@property                     BOOL          bShaderSupport;
@property                     BOOL          bCompressedTextureSupport;

@end

#endif /* __orxIPHONE__ && __orxOBJC__ */


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

/** Converts from RGB color space to HSL one
 * @param[in]   _pstDst         Destination color
 * @param[in]   _pstSrc         Source color
 * @return      orxCOLOR
 */
static orxCOLOR *orxFASTCALL      orxColor_FromRGBToHSL(orxCOLOR *_pstDst, const orxCOLOR *_pstSrc)
{
  orxCOLOR *pstResult = _pstDst;
  orxFLOAT  fMin, fMax, fDelta, fR, fG, fB;

  /* Checks */
  orxASSERT(_pstDst != orxNULL);
  orxASSERT(_pstSrc != orxNULL);

  /* Gets source red, blue and green components */
  fR = _pstSrc->vRGB.fR;
  fG = _pstSrc->vRGB.fG;
  fB = _pstSrc->vRGB.fB;

  /* Gets min, max & delta values */
  fMin    = orxMIN(fR, orxMIN(fG, fB));
  fMax    = orxMAX(fR, orxMAX(fG, fB));
  fDelta  = fMax - fMin;

  /* Stores lightness */
  pstResult->vHSL.fL = orx2F(0.5f) * (fMax + fMin);

  /* Gray? */
  if(fDelta == orxFLOAT_0)
  {
    /* Gets hue & saturation */
    pstResult->vHSL.fH = pstResult->vHSL.fS = orxFLOAT_0;
  }
  else
  {
    /* Updates saturation */
    pstResult->vHSL.fS = (pstResult->vHSL.fL < orx2F(0.5f))
                       ? fDelta / (fMax + fMin)
                       : fDelta / (orx2F(2.0f) - fMax - fMin);

    /* Red tone? */
    if(fR == fMax)
    {
      /* Updates hue */
      pstResult->vHSL.fH = orx2F(1.0f / 6.0f) * (fG - fB) / fDelta;
    }
    /* Green tone? */
    else if(fG == fMax)
    {
      /* Updates hue */
      pstResult->vHSL.fH = orx2F(1.0f / 3.0f) + (orx2F(1.0f / 6.0f) * (fB - fR) / fDelta);
    }
    /* Blue tone */
    else
    {
      /* Updates hue */
      pstResult->vHSL.fH = orx2F(2.0f / 3.0f) + (orx2F(1.0f / 6.0f) * (fR - fG) / fDelta);
    }

    /* Clamps hue */
    if(pstResult->vHSL.fH < orxFLOAT_0)
    {
      pstResult->vHSL.fH += orxFLOAT_1;
    }
    else if(pstResult->vHSL.fH > orxFLOAT_1)
    {
      pstResult->vHSL.fH -= orxFLOAT_1;
    }
  }

  /* Updates alpha */
  pstResult->fAlpha = _pstSrc->fAlpha;

  /* Done! */
  return pstResult;
}

/** Converts from HSL color space to RGB one
 * @param[in]   _pstDst         Destination color
 * @param[in]   _pstSrc         Source color
 * @return      orxCOLOR
 */
static orxCOLOR *orxFASTCALL      orxColor_FromHSLToRGB(orxCOLOR *_pstDst, const orxCOLOR *_pstSrc)
{
  orxCOLOR *pstResult = _pstDst;
  orxFLOAT  fH, fS, fL;

#define orxCOLOR_GET_RGB_COMPONENT(RESULT, ALT, CHROMA, HUE)                      \
do                                                                                \
{                                                                                 \
  if(HUE < orx2F(1.0f / 6.0f))                                                    \
  {                                                                               \
    RESULT = ALT + (orx2F(6.0f) * HUE * (CHROMA - ALT));                          \
  }                                                                               \
  else if(HUE < orx2F(1.0f / 2.0f))                                               \
  {                                                                               \
    RESULT = CHROMA;                                                              \
  }                                                                               \
  else if(HUE < orx2F(2.0f / 3.0f))                                               \
  {                                                                               \
    RESULT = ALT + (orx2F(6.0f) * (CHROMA - ALT) * (orx2F(2.0f / 3.0f) - HUE));   \
  }                                                                               \
  else                                                                            \
  {                                                                               \
    RESULT = ALT;                                                                 \
  }                                                                               \
  if(RESULT < orxMATH_KF_EPSILON)                                                 \
  {                                                                               \
    RESULT = orxFLOAT_0;                                                          \
  }                                                                               \
  else if(RESULT > orxFLOAT_1 - orxMATH_KF_EPSILON)                               \
  {                                                                               \
    RESULT = orxFLOAT_1;                                                          \
  }                                                                               \
}                                                                                 \
while(orxFALSE)

  /* Checks */
  orxASSERT(_pstDst != orxNULL);
  orxASSERT(_pstSrc != orxNULL);

  /* Gets source hue, saturation and lightness components */
  fH = _pstSrc->vRGB.fH;
  fS = _pstSrc->vRGB.fS;
  fL = _pstSrc->vRGB.fL;

  /* Gray? */
  if(fS == orxFLOAT_0)
  {
    /* Updates result */
    orxVector_SetAll(&(pstResult->vRGB), fL);
  }
  else
  {
    orxFLOAT fChroma, fIntermediate;

    /* Gets chroma */
    fChroma = (fL < orx2F(0.5f))
            ? fL + (fL * fS)
            : fL + fS - (fL * fS);

    /* Gets intermediate value */
    fIntermediate = (orx2F(2.0f) * fL) - fChroma;

    /* Gets RGB components */
    if(fH > orx2F(2.0f / 3.0f))
    {
      orxCOLOR_GET_RGB_COMPONENT(pstResult->vRGB.fR, fIntermediate, fChroma, (fH - orx2F(2.0f / 3.0f)));
    }
    else
    {
      orxCOLOR_GET_RGB_COMPONENT(pstResult->vRGB.fR, fIntermediate, fChroma, (fH + orx2F(1.0f / 3.0f)));
    }
    orxCOLOR_GET_RGB_COMPONENT(pstResult->vRGB.fG, fIntermediate, fChroma, fH);
    if(fH < orx2F(1.0f / 3.0f))
    {
      orxCOLOR_GET_RGB_COMPONENT(pstResult->vRGB.fB, fIntermediate, fChroma, (fH + orx2F(2.0f / 3.0f)));
    }
    else
    {
      orxCOLOR_GET_RGB_COMPONENT(pstResult->vRGB.fB, fIntermediate, fChroma, (fH - orx2F(1.0f / 3.0f)));
    }
  }

  /* Updates alpha */
  pstResult->fAlpha = _pstSrc->fAlpha;

  /* Done! */
  return pstResult;
}

/** Converts from RGB color space to HSV one
 * @param[in]   _pstDst         Destination color
 * @param[in]   _pstSrc         Source color
 * @return      orxCOLOR
 */
static orxCOLOR *orxFASTCALL      orxColor_FromRGBToHSV(orxCOLOR *_pstDst, const orxCOLOR *_pstSrc)
{
  orxCOLOR *pstResult = _pstDst;
  orxFLOAT  fMin, fMax, fDelta, fR, fG, fB;

  /* Checks */
  orxASSERT(_pstDst != orxNULL);
  orxASSERT(_pstSrc != orxNULL);

  /* Gets source red, blue and green components */
  fR = _pstSrc->vRGB.fR;
  fG = _pstSrc->vRGB.fG;
  fB = _pstSrc->vRGB.fB;

  /* Gets min, max & delta values */
  fMin    = orxMIN(fR, orxMIN(fG, fB));
  fMax    = orxMAX(fR, orxMAX(fG, fB));
  fDelta  = fMax - fMin;

  /* Stores value */
  pstResult->vHSL.fV = fMax;

  /* Gray? */
  if(fDelta == orxFLOAT_0)
  {
    /* Gets hue & saturation */
    pstResult->vHSL.fH = pstResult->vHSL.fS = orxFLOAT_0;
  }
  else
  {
    /* Updates saturation */
    pstResult->vHSL.fS = fDelta / fMax;

    /* Red tone? */
    if(fR == fMax)
    {
      /* Updates hue */
      pstResult->vHSL.fH = orx2F(1.0f / 6.0f) * (fG - fB) / fDelta;
    }
    /* Green tone? */
    else if(fG == fMax)
    {
      /* Updates hue */
      pstResult->vHSL.fH = orx2F(1.0f / 3.0f) + (orx2F(1.0f / 6.0f) * (fB - fR) / fDelta);
    }
    /* Blue tone */
    else
    {
      /* Updates hue */
      pstResult->vHSL.fH = orx2F(2.0f / 3.0f) + (orx2F(1.0f / 6.0f) * (fR - fG) / fDelta);
    }

    /* Clamps hue */
    if(pstResult->vHSL.fH < orxFLOAT_0)
    {
      pstResult->vHSL.fH += orxFLOAT_1;
    }
    else if(pstResult->vHSL.fH > orxFLOAT_1)
    {
      pstResult->vHSL.fH -= orxFLOAT_1;
    }
  }

  /* Updates alpha */
  pstResult->fAlpha = _pstSrc->fAlpha;

  /* Done! */
  return pstResult;
}

/** Converts from HSV color space to RGB one
 * @param[in]   _pstDst         Destination color
 * @param[in]   _pstSrc         Source color
 * @return      orxCOLOR
 */
static orxCOLOR *orxFASTCALL      orxColor_FromHSVToRGB(orxCOLOR *_pstDst, const orxCOLOR *_pstSrc)
{
  orxCOLOR *pstResult = _pstDst;
  orxFLOAT  fH, fS, fV;

  /* Checks */
  orxASSERT(_pstDst != orxNULL);
  orxASSERT(_pstSrc != orxNULL);

  /* Gets source hue, saturation and value components */
  fH = _pstSrc->vRGB.fH;
  fS = _pstSrc->vRGB.fS;
  fV = _pstSrc->vRGB.fV;

  /* Gray? */
  if(fS == orxFLOAT_0)
  {
    /* Updates result */
    orxVector_SetAll(&(pstResult->vRGB), fV);
  }
  else
  {
    orxFLOAT fFullHue, fSector, fIntermediate;

    /* Gets intermediate value */
    fIntermediate = fV * fS;

    /* Gets full hue & sector */
    fFullHue  = orx2F(6.0f) * fH;
    fSector   = orxMath_Floor(fFullHue);

    /* Depending on sector */
    switch(orxF2U(fSector))
    {
      default:
      case 0:
      {
        /* Updates RGB components */
        pstResult->vRGB.fR = fV;
        pstResult->vRGB.fG = fV - (fIntermediate - (fIntermediate * (fFullHue - fSector)));
        pstResult->vRGB.fB = fV - fIntermediate;

        break;
      }

      case 1:
      {
        /* Updates RGB components */
        pstResult->vRGB.fR = fV - (fIntermediate * (fFullHue - fSector));
        pstResult->vRGB.fG = fV;
        pstResult->vRGB.fB = fV - fIntermediate;

        break;
      }

      case 2:
      {
        /* Updates RGB components */
        pstResult->vRGB.fR = fV - fIntermediate;
        pstResult->vRGB.fG = fV;
        pstResult->vRGB.fB = fV - (fIntermediate - (fIntermediate * (fFullHue - fSector)));

        break;
      }

      case 3:
      {
        /* Updates RGB components */
        pstResult->vRGB.fR = fV - fIntermediate;
        pstResult->vRGB.fG = fV - (fIntermediate * (fFullHue - fSector));
        pstResult->vRGB.fB = fV;

        break;
      }

      case 4:
      {
        /* Updates RGB components */
        pstResult->vRGB.fR = fV - (fIntermediate - (fIntermediate * (fFullHue - fSector)));
        pstResult->vRGB.fG = fV - fIntermediate;
        pstResult->vRGB.fB = fV;

        break;
      }

      case 5:
      {
        /* Updates RGB components */
        pstResult->vRGB.fR = fV;
        pstResult->vRGB.fG = fV - fIntermediate;
        pstResult->vRGB.fB = fV - (fIntermediate * (fFullHue - fSector));

        break;
      }
    }
  }

  /* Updates alpha */
  pstResult->fAlpha = _pstSrc->fAlpha;

  /* Done! */
  return pstResult;
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

/** Transforms a text (on a bitmap)
 * @param[in]   _zString                              String to display
 * @param[in]   _pstFont                              Font bitmap
 * @param[in]   _pstMap                               Character map
 * @param[in]   _pstTransform                         Transformation info (position, scale, rotation, ...)
 * @param[in]   _eSmoothing                           Bitmap smoothing type
 * @param[in]   _eBlendMode                           Blend mode
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxDisplay_TransformText(const orxSTRING _zString, const orxBITMAP *_pstFont, const orxCHARACTER_MAP *_pstMap, const orxDISPLAY_TRANSFORM *_pstTransform, orxDISPLAY_SMOOTHING _eSmoothing, orxDISPLAY_BLEND_MODE _eBlendMode);


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

/** Sets destination bitmap
 * @param[in]   _pstDst                               Destination bitmap
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxDisplay_SetDestinationBitmap(orxBITMAP *_pstDst);

/** Transforms (and blits onto another) a bitmap
 * @param[in]   _pstSrc                               Bitmap to transform and draw
 * @param[in]   _pstTransform                         Transformation info (position, scale, rotation, ...)
 * @param[in]   _eSmoothing                           Bitmap smoothing type
 * @param[in]   _eBlendMode                           Blend mode
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxDisplay_TransformBitmap(const orxBITMAP *_pstSrc, const orxDISPLAY_TRANSFORM *_pstTransform, orxDISPLAY_SMOOTHING _eSmoothing, orxDISPLAY_BLEND_MODE _eBlendMode);


/** Sets a bitmap data (RGBA memory format)
 * @param[in]   _pstBitmap                            Concerned bitmap
 * @param[in]   _au8Data                              Data (4 channels, RGBA)
 * @param[in]   _u32ByteNumber                        Number of bytes
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxDisplay_SetBitmapData(orxBITMAP *_pstBitmap, const orxU8 *_au8Data, orxU32 _u32ByteNumber);

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
 * @param[in]   _pstSrc                               Bitmap to blit (will begin at top left corner)
 * @param[in]   _fPosX                                X-axis value of the position where to blit the source bitmap
 * @param[in]   _fPosY                                Y-axis value of the position where to blit the source bitmap
 * @param[in]   _eSmoothing                           Bitmap smoothing type
 * @param[in]   _eBlendMode                           Blend mode
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxDisplay_BlitBitmap(const orxBITMAP *_pstSrc, orxFLOAT _fPosX, orxFLOAT _fPosY, orxDISPLAY_SMOOTHING _eSmoothing, orxDISPLAY_BLEND_MODE _eBlendMode);


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


/** Gets a bitmap data (RGBA memory format)
 * @param[in]   _pstBitmap                            Concerned bitmap
 * @param[in]   _au8Data                              Output buffer (4 channels, RGBA)
 * @param[in]   _u32ByteNumber                        Number of bytes of the buffer
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxDisplay_GetBitmapData(orxBITMAP *_pstBitmap, orxU8 *_au8Data, orxU32 _u32ByteNumber);

/** Gets bitmap color (lighting/hue)
 * @param[in]   _pstBitmap                            Concerned bitmap
 * @return orxRGBA
 */
extern orxDLLAPI orxRGBA orxFASTCALL                  orxDisplay_GetBitmapColor(const orxBITMAP *_pstBitmap);

/** Gets a bitmap size
 * @param[in]   _pstBitmap                            Concerned bitmap
 * @param[out]   _pfWidth                             Bitmap width
 * @param[out]   _pfHeight                            Bitmap height
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxDisplay_GetBitmapSize(const orxBITMAP *_pstBitmap, orxFLOAT *_pfWidth, orxFLOAT *_pfHeight);


/** Has shader support?
 * @return orxTRUE / orxFALSE
 */
extern orxDLLAPI orxBOOL orxFASTCALL                  orxDisplay_HasShaderSupport();

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

/** Starts a shader rendering
 * @param[in]   _hShader                              Shader to start
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxDisplay_StartShader(const orxHANDLE _hShader);

/** Stops a shader rendering
 * @param[in]   _hShader                              Shader to stop
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxDisplay_StopShader(const orxHANDLE _hShader);

/** Gets a shader parameter's ID
 * @param[in]   _hShader                              Concerned shader
 * @param[in]   _zParam                               Parameter name
 * @param[in]   _s32Index                             Parameter index, -1 for non-array types
 * @param[in]   _bIsTexture                           Is parameter a texture?
 * @return Parameter ID
 */
extern orxDLLAPI orxS32 orxFASTCALL                   orxDisplay_GetParameterID(orxHANDLE _hShader, const orxSTRING _zParam, orxS32 _s32Index, orxBOOL _bIsTexture);

/** Sets a shader parameter (orxBITMAP)
 * @param[in]   _hShader                              Concerned shader
 * @param[in]   _s32ID                                ID of parameter to set
 * @param[in]   _pstValue                             Value (orxBITMAP) for this parameter
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxDisplay_SetShaderBitmap(orxHANDLE _hShader, orxS32 _s32ID, const orxBITMAP *_pstValue);

/** Sets a shader parameter (orxFLOAT)
 * @param[in]   _hShader                              Concerned shader
 * @param[in]   _s32ID                                ID of parameter to set
 * @param[in]   _fValue                               Value (orxFLOAT) for this parameter
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxDisplay_SetShaderFloat(orxHANDLE _hShader, orxS32 _s32ID, orxFLOAT _fValue);

/** Sets a shader parameter (orxVECTOR)
 * @param[in]   _hShader                              Concerned shader
 * @param[in]   _s32ID                                ID of parameter to set
 * @param[in]   _pvValue                              Value (orxVECTOR) for this parameter
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxDisplay_SetShaderVector(orxHANDLE _hShader, orxS32 _s32ID, const orxVECTOR *_pvValue);


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
