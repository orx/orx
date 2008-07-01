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


/** Slot flags
 */
#define orxFX_SLOT_KU32_FLAG_NONE               0x00000000  /**< No flag */

#define orxFX_SLOT_KU32_FLAG_DEFINED            0x10000000  /**< Defined flag */

#define orxFX_SLOT_KU32_FLAG_ABSOLUTE           0x00000100  /**< Absolute flag */
#define orxFX_SLOT_KU32_FLAG_BLEND_SQUARE       0x00000200  /**< Square blend curve flag */

#define orxFX_SLOT_KU32_FLAG_BLEND_CURVE_LINEAR 0x00000001  /**< Blend curve linear flag */
#define orxFX_SLOT_KU32_FLAG_BLEND_CURVE_SAW    0x00000002  /**< Blend curve saw flag */
#define orxFX_SLOT_KU32_FLAG_BLEND_CURVE_SINE   0x00000004  /**< Blend curve sine flag */

#define orxFX_SLOT_KU32_MASK_BLEND_CURVE        0x000000FF  /**< Blend curve mask */

#define orxFX_SLOT_KU32_MASK_USER_ALL           0x0000FFFF  /**< User all mask */

#define orxFX_SLOT_KU32_MASK_ALL                0xFFFFFFFF  /**< All mask */


/** Internal FX structure
 */
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

/** Adds alpha fade to an FX
 * @param[in]   _pstFX          Concerned FX
 * @param[in]   _fStartTime     Time start
 * @param[in]   _fEndTime       Time end
 * @param[in]   _fCyclePeriod   Cycle period
 * @param[in]   _fCyclePhasis   Cycle phasis (at start)
 * @param[in]   _u8StartAlpha   Starting alpha value
 * @param[in]   _u8EndAlpha     Ending alpha value
 * @param[in]   _u32Flags       Param flags
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxFX_AddAlphaFade(orxFX *_pstFX, orxFLOAT _fStartTime, orxFLOAT _fEndTime, orxFLOAT _fCyclePeriod, orxFLOAT _fCyclePhasis, orxU8 _u8StartAlpha, orxU8 _u8EndAlpha, orxU32 _u32Flags);

/** Adds color blend to an FX
 * @param[in]   _pstFX          Concerned FX
 * @param[in]   _fStartTime     Time start
 * @param[in]   _fEndTime       Time end
 * @param[in]   _fCyclePeriod   Cycle period
 * @param[in]   _fCyclePhasis   Cycle phasis (at start)
 * @param[in]   _stStartColor   Starting color value
 * @param[in]   _stEndColor     Ending color value
 * @param[in]   _u32Flags       Param flags
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxFX_AddColorBlend(orxFX *_pstFX, orxFLOAT _fStartTime, orxFLOAT _fEndTime, orxFLOAT _fCyclePeriod, orxFLOAT _fCyclePhasis, orxRGBA _stStartColor, orxRGBA _stEndColor, orxU32 _u32Flags);

/** Adds rotation to an FX
 * @param[in]   _pstFX          Concerned FX
 * @param[in]   _fStartTime     Time start
 * @param[in]   _fEndTime       Time end
 * @param[in]   _fCyclePeriod   Cycle period
 * @param[in]   _fCyclePhasis   Cycle phasis (at start)
 * @param[in]   _fStartRotation Starting rotation value
 * @param[in]   _fEndRotation   Ending rotation value
 * @param[in]   _u32Flags       Param flags
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxFX_AddRotation(orxFX *_pstFX, orxFLOAT _fStartTime, orxFLOAT _fEndTime, orxFLOAT _fCyclePeriod, orxFLOAT _fCyclePhasis, orxFLOAT _fStartRotation, orxFLOAT _fEndRotation, orxU32 _u32Flags);

/** Adds scale to an FX
 * @param[in]   _pstFX          Concerned FX
 * @param[in]   _fStartTime     Time start
 * @param[in]   _fEndTime       Time end
 * @param[in]   _fCyclePeriod   Cycle period
 * @param[in]   _fCyclePhasis   Cycle phasis (at start)
 * @param[in]   _pvStartScale   Starting scale value
 * @param[in]   _pvEndScale     Ending scale value
 * @param[in]   _u32Flags       Param flags
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxFX_AddScale(orxFX *_pstFX, orxFLOAT _fStartTime, orxFLOAT _fEndTime, orxFLOAT _fCyclePeriod, orxFLOAT _fCyclePhasis, orxCONST orxVECTOR *_pvStartScale, orxCONST orxVECTOR *_pvEndScale, orxU32 _u32Flags);

/** Adds translation to an FX
 * @param[in]   _pstFX          Concerned FX
 * @param[in]   _fStartTime     Time start
 * @param[in]   _fEndTime       Time end
 * @param[in]   _fCyclePeriod   Cycle period
 * @param[in]   _fCyclePhasis   Cycle phasis (at start)
 * @param[in]   _pvStartPosition Starting position value
 * @param[in]   _pvEndPosition  Ending position value
 * @param[in]   _u32Flags       Param flags
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxFX_AddTranslation(orxFX *_pstFX, orxFLOAT _fStartTime, orxFLOAT _fEndTime, orxFLOAT _fCyclePeriod, orxFLOAT _fCyclePhasis, orxCONST orxVECTOR *_pvStartTranslation, orxCONST orxVECTOR *_pvEndTranslation, orxU32 _u32Flags);

/** Gets FX duration
 * @param[in]   _pstFX          Concerned FX
 * @return      orxFLOAT
 */
extern orxDLLAPI orxFLOAT orxFASTCALL           orxFX_GetDuration(orxCONST orxFX *_pstFX);

/** Tests FX name against given one
 * @param[in]   _pstFX          Concerned FX
 * @param[in]   _zName          Name to test
 * @return      orxTRUE if it's FX name, orxFALSE otherwise
 */
extern orxDLLAPI orxBOOL orxFASTCALL            orxFX_IsName(orxCONST orxFX *_pstFX, orxCONST orxSTRING _zName);

#endif /* _orxFX_H_ */

/** @} */
