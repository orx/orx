/***************************************************************************
 *                                                                         *
 *   This library is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License           *
 *   as published by the Free Software Foundation; either version 2.1      *
 *   of the License, or (at your option) any later version.                *
 *                                                                         *
 ***************************************************************************/

/**
 * @file orxFX.h
 * @date 30/06/2008
 * @author (C) Arcallians
 */

/**
 * @addtogroup Render
 * 
 * FX module
 * Allows to creates rendering special effects affecting color, scale, rotation and position of visuals.
 *
 * @{
 */


#ifndef _orxFX_H_
#define _orxFX_H_


#include "orxInclude.h"
#include "math/orxVector.h"


/** orxFX Helpers
 */
#define orxFX_DEF_INIT_ALPHA_FADE(FX_DEF, START_TIME, END_TIME, CYCLE_PERIOD, CYCLE_PHASIS, FLAGS, START_ALPHA, END_ALPHA)  \
  orxMemory_Zero(&(FX_DEF), sizeof(orxFX_DEF));                                                                             \
  FX_DEF.eFXType      = orxFX_TYPE_ALPHA_FADE;                                                                              \
  FX_DEF.u32FXFlags   = (FLAGS) & orxFX_MASK_USER;                                                                          \
  FX_DEF.fStartTime   = START_TIME;                                                                                         \
  FX_DEF.fEndTime     = END_TIME;                                                                                           \
  FX_DEF.fCyclePeriod = CYCLE_PERIOD;                                                                                       \
  FX_DEF.fCyclePhasis = CYCLE_PHASIS;                                                                                       \
  FX_DEF.u8StartAlpha = START_ALPHA;                                                                                        \
  FX_DEF.u8EndAlpha   = END_ALPHA;

#define orxFX_DEF_INIT_SCALE(FX_DEF, START_TIME, END_TIME, CYCLE_PERIOD, CYCLE_PHASIS, FLAGS, START_SCALE, END_SCALE)       \
  orxMemory_Zero(&(FX_DEF), sizeof(orxFX_DEF));                                                                             \
  FX_DEF.eFXType      = orxFX_TYPE_SCALE;                                                                                   \
  FX_DEF.u32FXFlags   = (FLAGS) & orxFX_MASK_USER;                                                                          \
  FX_DEF.fStartTime   = START_TIME;                                                                                         \
  FX_DEF.fEndTime     = END_TIME;                                                                                           \
  FX_DEF.fCyclePeriod = CYCLE_PERIOD;                                                                                       \
  FX_DEF.fCyclePhasis = CYCLE_PHASIS;                                                                                       \
  FX_DEF.vStartScale  = START_SCALE;                                                                                        \
  FX_DEF.vEndScale    = END_SCALE;

#define orxFX_DEF_INIT_COLOR_BLEND(FX_DEF, START_TIME, END_TIME, CYCLE_PERIOD, CYCLE_PHASIS, FLAGS, START_COLOR, END_COLOR) \
  orxMemory_Zero(&(FX_DEF), sizeof(orxFX_DEF));                                                                             \
  FX_DEF.eFXType      = orxFX_TYPE_COLOR_BLEND;                                                                             \
  FX_DEF.u32FXFlags   = (FLAGS) & orxFX_MASK_USER;                                                                          \
  FX_DEF.fStartTime   = START_TIME;                                                                                         \
  FX_DEF.fEndTime     = END_TIME;                                                                                           \
  FX_DEF.fCyclePeriod = CYCLE_PERIOD;                                                                                       \
  FX_DEF.fCyclePhasis = CYCLE_PHASIS;                                                                                       \
  FX_DEF.stStartColor = START_COLOR;                                                                                        \
  FX_DEF.stEndColor   = END_COLOR;

#define orxFX_DEF_INIT_TRANSLATION(FX_DEF, START_TIME, END_TIME, CYCLE_PERIOD, CYCLE_PHASIS, FLAGS, START_POS, END_POS)     \
  orxMemory_Zero(&(FX_DEF), sizeof(orxFX_DEF));                                                                             \
  FX_DEF.eFXType        = orxFX_TYPE_TRANSLATION;                                                                           \
  FX_DEF.u32FXFlags     = (FLAGS) & orxFX_MASK_USER;                                                                        \
  FX_DEF.fStartTime     = START_TIME;                                                                                       \
  FX_DEF.fEndTime       = END_TIME;                                                                                         \
  FX_DEF.fCyclePeriod   = CYCLE_PERIOD;                                                                                     \
  FX_DEF.fCyclePhasis   = CYCLE_PHASIS;                                                                                     \
  FX_DEF.vStartPosition = START_POS;                                                                                        \
  FX_DEF.vEndPosition   = END_POS;


/** FX definition flags
 */
#define orxFX_DEF_KU32_FLAG_NONE                0x00000000

#define orxFX_DEF_KU32_FLAG_ABSOLUTE            0x00000100
#define orxFX_DEF_KU32_FLAG_RESET_AFTER_END     0x00000200
#define orxFX_DEF_KU32_FLAG_DESTROY_AFTER_END   0x00000400

#define orxFX_DEF_KU32_FLAG_BLEND_SQUARE        0x00000800

#define orxFX_DEF_KU32_FLAG_BLEND_CURVE_LINEAR  0x00000001
#define orxFX_DEF_KU32_FLAG_BLEND_CURVE_SAW     0x00000002
#define orxFX_DEF_KU32_FLAG_BLEND_CURVE_SINUS   0x00000004

#define orxFX_DEF_KU32_MASK_BLEND_CURVE         0x000000FF

#define orxFX_DEF_KU32_MASK_ALL                 0xFFFFFFFF


/** FX type enum
 */
typedef enum __orxFX_TYPE_t
{
	orxFX_TYPE_SCALE = 0,
	orxFX_TYPE_ALPHA_FADE,
	orxFX_TYPE_COLOR_BLEND,
	orxFX_TYPE_TRANSLATION,

	orxFX_TYPE_NUMBER,

	orxFX_TYPE_NONE = orxENUM_NONE

} orxFX_TYPE;

/** FX definition
 */
typedef struct __orxFX_DEF_t
{
	orxFX_TYPE  eFXType;                          /**< Type : 4 */
  orxFLOAT    fStartTime;                       /**< Start Time : 8 */
  orxFLOAT    fEndTime;                         /**< End Time : 12 */
  orxFLOAT    fCyclePeriod;                     /**< Cycle period : 16 */
  orxFLOAT    fCyclePhasis;                     /**< Cycle phasis : 20 */

  union
  {
    struct
    {
      orxU8 u8StartAlpha;                       /**< Alpha start value : 21 */
      orxU8 u8EndAlpha;                         /**< Alpha end value : 22 */
    };                                          /**< Alpha Fade  : 22 */

    struct
    {
      orxVECTOR vStartScale;                    /**< Scale start value : 32 */
      orxVECTOR vEndScale;                      /**< Scale end value : 44 */
    };                                          /**< Scale : 44 */

    struct
    {
      orxRGBA stStartColor;                     /**< ColorBlend start value : 24 */
      orxRGBA stEndColor;                       /**< ColorBlend end value : 28 */
    };                                          /** Color blend : 28 */

    struct
    {
      orxVECTOR vStartPosition;                 /**< Translation vector : 32 */
      orxVECTOR vEndPosition;                   /**< Translation end position : 44 */
    };                                          /**< Translation : 44 */
  };

  orxU32 u32Flags;                              /* Flags : 48 */

} orxFX_DEF;


/** Internal FX structure */
typedef struct __orxFX_t                        orxFX;


/** FX module setup
 */
extern orxDLLAPI orxVOID                        orxFX_Setup();

/** Inits the FX module
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS                      orxFX_Init();

/** Exits from the FX module
 */
extern orxDLLAPI orxVOID                        orxFX_Exit();

/** Creates an empty FX
 * @return orxFX / orxNULL
 */
extern orxDLLAPI orxFX *                        orxFX_Create();

/** Creates an FX from config
 * @param[in]   _zConfigID    Config ID
 * @ return orxFX / orxNULL
 */
extern orxDLLAPI orxFX *orxFASTCALL             orxFX_CreateFromConfig(orxCONST orxSTRING _zConfigID);

/** Deletes an FX
 * @param[in] _pstFX            Concerned FX
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxFX_Delete(orxFX *_pstFX);

/** Enables/disables an FX
 * @param[in]   _pstFX          Concerned FX
 * @param[in]   _bEnable        Enable / disable
 */
extern orxDLLAPI orxVOID orxFASTCALL            orxFX_Enable(orxFX *_pstFX, orxBOOL _bEnable);

/** Is FX enabled?
 * @param[in]   _pstFX          Concerned FX
 * @return      orxTRUE if enabled, orxFALSE otherwise
 */
extern orxDLLAPI orxBOOL orxFASTCALL            orxFX_IsEnabled(orxCONST orxFX *_pstFX);

#endif /* _orxFX_H_ */

/** @} */
