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
#include "object/orxObject.h"


/** Slot flags
 */
#define orxFX_SLOT_KU32_FLAG_ABSOLUTE           0x00000100  /**< Absolute flag */


/** FX curve enum
 */
 typedef enum __orxFX_CURVE_t
 {
   orxFX_CURVE_LINEAR = 0,
   orxFX_CURVE_SAW,
   orxFX_CURVE_SINE,

   orxFX_CURVE_NUMBER,

   orxFX_CURVE_NONE = orxENUM_NONE

 } orxFX_CURVE;


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

/** Applies FX on object
 * @param[in] _pstFX            FX to apply
 * @param[in] _pstObject        Object on which to apply the FX
 * @param[in] _fStartTime       FX local application start time
 * @param[in] _fEndTime         FX local application end time
 * @return    orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxFX_Apply(orxCONST orxFX *_pstFX, orxOBJECT *_pstObject, orxFLOAT _fStartTime, orxFLOAT _fEndTime);

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
 * @param[in]   _fAmplification Curve linear amplification over time (1.0 for none)
 * @param[in]   _fStartAlpha    Starting alpha value
 * @param[in]   _fEndAlpha      Ending alpha value
 * @param[in]   _eCurve         Blending curve type
 * @param[in]   _fPow           Blending curve exponent
 * @param[in]   _u32Flags       Param flags
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxFX_AddAlphaFade(orxFX *_pstFX, orxFLOAT _fStartTime, orxFLOAT _fEndTime, orxFLOAT _fCyclePeriod, orxFLOAT _fCyclePhasis, orxFLOAT _fAmplification, orxFLOAT _fStartAlpha, orxFLOAT _fEndAlpha, orxFX_CURVE _eCurve, orxFLOAT _fPow, orxU32 _u32Flags);

/** Adds color blend to an FX
 * @param[in]   _pstFX          Concerned FX
 * @param[in]   _fStartTime     Time start
 * @param[in]   _fEndTime       Time end
 * @param[in]   _fCyclePeriod   Cycle period
 * @param[in]   _fCyclePhasis   Cycle phasis (at start)
 * @param[in]   _fAmplification Curve linear amplification over time (1.0 for none)
 * @param[in]   _pvStartColor   Starting color value
 * @param[in]   _pvEndColor     Ending color value
 * @param[in]   _eCurve         Blending curve type
 * @param[in]   _fPow           Blending curve exponent
 * @param[in]   _u32Flags       Param flags
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxFX_AddColorBlend(orxFX *_pstFX, orxFLOAT _fStartTime, orxFLOAT _fEndTime, orxFLOAT _fCyclePeriod, orxFLOAT _fCyclePhasis, orxFLOAT _fAmplification, orxVECTOR *_pvStartColor, orxVECTOR *_pvEndColor, orxFX_CURVE _eCurve, orxFLOAT _fPow, orxU32 _u32Flags);

/** Adds rotation to an FX
 * @param[in]   _pstFX          Concerned FX
 * @param[in]   _fStartTime     Time start
 * @param[in]   _fEndTime       Time end
 * @param[in]   _fCyclePeriod   Cycle period
 * @param[in]   _fCyclePhasis   Cycle phasis (at start)
 * @param[in]   _fAmplification Curve linear amplification over time (1.0 for none)
 * @param[in]   _fStartRotation Starting rotation value
 * @param[in]   _fEndRotation   Ending rotation value
 * @param[in]   _eCurve         Blending curve type
 * @param[in]   _fPow           Blending curve exponent
 * @param[in]   _u32Flags       Param flags
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxFX_AddRotation(orxFX *_pstFX, orxFLOAT _fStartTime, orxFLOAT _fEndTime, orxFLOAT _fCyclePeriod, orxFLOAT _fCyclePhasis, orxFLOAT _fAmplification, orxFLOAT _fStartRotation, orxFLOAT _fEndRotation, orxFX_CURVE _eCurve, orxFLOAT _fPow, orxU32 _u32Flags);

/** Adds scale to an FX
 * @param[in]   _pstFX          Concerned FX
 * @param[in]   _fStartTime     Time start
 * @param[in]   _fEndTime       Time end
 * @param[in]   _fCyclePeriod   Cycle period
 * @param[in]   _fCyclePhasis   Cycle phasis (at start)
 * @param[in]   _fAmplification Curve linear amplification over time (1.0 for none)
 * @param[in]   _pvStartScale   Starting scale value
 * @param[in]   _pvEndScale     Ending scale value
 * @param[in]   _eCurve         Blending curve type
 * @param[in]   _fPow           Blending curve exponent
 * @param[in]   _u32Flags       Param flags
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxFX_AddScale(orxFX *_pstFX, orxFLOAT _fStartTime, orxFLOAT _fEndTime, orxFLOAT _fCyclePeriod, orxFLOAT _fCyclePhasis, orxFLOAT _fAmplification, orxCONST orxVECTOR *_pvStartScale, orxCONST orxVECTOR *_pvEndScale, orxFX_CURVE _eCurve, orxFLOAT _fPow, orxU32 _u32Flags);

/** Adds translation to an FX
 * @param[in]   _pstFX          Concerned FX
 * @param[in]   _fStartTime     Time start
 * @param[in]   _fEndTime       Time end
 * @param[in]   _fCyclePeriod   Cycle period
 * @param[in]   _fCyclePhasis   Cycle phasis (at start)
 * @param[in]   _fAmplification Curve linear amplification over time (1.0 for none)
 * @param[in]   _pvStartPosition Starting position value
 * @param[in]   _pvEndPosition  Ending position value
 * @param[in]   _eCurve         Blending curve type
 * @param[in]   _fPow           Blending curve exponent
 * @param[in]   _u32Flags       Param flags
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxFX_AddTranslation(orxFX *_pstFX, orxFLOAT _fStartTime, orxFLOAT _fEndTime, orxFLOAT _fCyclePeriod, orxFLOAT _fCyclePhasis, orxFLOAT _fAmplification, orxCONST orxVECTOR *_pvStartTranslation, orxCONST orxVECTOR *_pvEndTranslation, orxFX_CURVE _eCurve, orxFLOAT _fPow, orxU32 _u32Flags);

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

/** Set FX loop property
 * @param[in]   _pstFX          Concerned FX
 * @param[in]   _bLoop          Loop / don't loop
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxFX_Loop(orxFX *_pstFX, orxBOOL _bLoop);

/** Is FX looping
 * @param[in]   _pstFX          Concerned FX
 * @return      orxTRUE if looping, orxFALSE otherwise
 */
extern orxDLLAPI orxBOOL orxFASTCALL            orxFX_IsLooping(orxCONST orxFX *_pstFX);

#endif /* _orxFX_H_ */

/** @} */
