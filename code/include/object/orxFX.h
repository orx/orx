/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2013 Orx-Project
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
 * @file orxFX.h
 * @date 30/06/2008
 * @author iarwain@orx-project.org
 *
 * @todo
 */

/**
 * @addtogroup orxFX
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
#define orxFX_SLOT_KU32_FLAG_USE_ROTATION       0x00000200  /**< Use rotation flag */
#define orxFX_SLOT_KU32_FLAG_USE_SCALE          0x00000400  /**< Use scale flag */


/** FX curve enum
 */
 typedef enum __orxFX_CURVE_t
 {
   orxFX_CURVE_LINEAR = 0,
   orxFX_CURVE_SMOOTH,
   orxFX_CURVE_SMOOTHER,
   orxFX_CURVE_TRIANGLE,
   orxFX_CURVE_SINE,
   orxFX_CURVE_SQUARE,

   orxFX_CURVE_NUMBER,

   orxFX_CURVE_NONE = orxENUM_NONE

 } orxFX_CURVE;


/** Internal FX structure
 */
typedef struct __orxFX_t                        orxFX;


/** Event enum
 */
typedef enum __orxFX_EVENT_t
{
  orxFX_EVENT_START = 0,                        /**< Event sent when a FX starts */
  orxFX_EVENT_STOP,                             /**< Event sent when a FX stops */
  orxFX_EVENT_ADD,                              /**< Event sent when a FX is added */
  orxFX_EVENT_REMOVE,                           /**< Event sent when a FX is removed */
  orxFX_EVENT_LOOP,                             /**< Event sent when a FX is looping */

  orxFX_EVENT_NUMBER,

  orxFX_EVENT_NONE = orxENUM_NONE

} orxFX_EVENT;

/** FX event payload
 */
typedef struct __orxFX_EVENT_PAYLOAD_t
{
  orxFX          *pstFX;                        /**< FX reference : 4 */
  const orxSTRING zFXName;                      /**< FX name : 8 */

} orxFX_EVENT_PAYLOAD;


/** FX module setup
 */
extern orxDLLAPI void orxFASTCALL               orxFX_Setup();

/** Inits the FX module
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxFX_Init();

/** Exits from the FX module
 */
extern orxDLLAPI void orxFASTCALL               orxFX_Exit();

/** Creates an empty FX
 * @return orxFX / orxNULL
 */
extern orxDLLAPI orxFX *orxFASTCALL             orxFX_Create();

/** Creates an FX from config
 * @param[in]   _zConfigID    Config ID
 * @ return orxFX / orxNULL
 */
extern orxDLLAPI orxFX *orxFASTCALL             orxFX_CreateFromConfig(const orxSTRING _zConfigID);

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
extern orxDLLAPI orxSTATUS orxFASTCALL          orxFX_Apply(const orxFX *_pstFX, orxOBJECT *_pstObject, orxFLOAT _fStartTime, orxFLOAT _fEndTime);

/** Enables/disables an FX
 * @param[in]   _pstFX          Concerned FX
 * @param[in]   _bEnable        Enable / disable
 */
extern orxDLLAPI void orxFASTCALL               orxFX_Enable(orxFX *_pstFX, orxBOOL _bEnable);

/** Is FX enabled?
 * @param[in]   _pstFX          Concerned FX
 * @return      orxTRUE if enabled, orxFALSE otherwise
 */
extern orxDLLAPI orxBOOL orxFASTCALL            orxFX_IsEnabled(const orxFX *_pstFX);

/** Adds alpha to an FX
 * @param[in]   _pstFX          Concerned FX
 * @param[in]   _fStartTime     Time start
 * @param[in]   _fEndTime       Time end
 * @param[in]   _fCyclePeriod   Cycle period
 * @param[in]   _fCyclePhase    Cycle phase (at start)
 * @param[in]   _fAmplification Curve linear amplification over time (1.0 for none)
 * @param[in]   _fAcceleration  Curve linear acceleration over time (1.0 for none)
 * @param[in]   _fStartAlpha    Starting alpha value
 * @param[in]   _fEndAlpha      Ending alpha value
 * @param[in]   _eCurve         Blending curve type
 * @param[in]   _fPow           Blending curve exponent
 * @param[in]   _u32Flags       Param flags
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxFX_AddAlpha(orxFX *_pstFX, orxFLOAT _fStartTime, orxFLOAT _fEndTime, orxFLOAT _fCyclePeriod, orxFLOAT _fCyclePhase, orxFLOAT _fAmplification, orxFLOAT _fAcceleration, orxFLOAT _fStartAlpha, orxFLOAT _fEndAlpha, orxFX_CURVE _eCurve, orxFLOAT _fPow, orxU32 _u32Flags);

/** Adds RGB color to an FX
 * @param[in]   _pstFX          Concerned FX
 * @param[in]   _fStartTime     Time start
 * @param[in]   _fEndTime       Time end
 * @param[in]   _fCyclePeriod   Cycle period
 * @param[in]   _fCyclePhase    Cycle phase (at start)
 * @param[in]   _fAmplification Curve linear amplification over time (1.0 for none)
 * @param[in]   _fAcceleration  Curve linear acceleration over time (1.0 for none)
 * @param[in]   _pvStartColor   Starting color value
 * @param[in]   _pvEndColor     Ending color value
 * @param[in]   _eCurve         Blending curve type
 * @param[in]   _fPow           Blending curve exponent
 * @param[in]   _u32Flags       Param flags
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxFX_AddRGB(orxFX *_pstFX, orxFLOAT _fStartTime, orxFLOAT _fEndTime, orxFLOAT _fCyclePeriod, orxFLOAT _fCyclePhase, orxFLOAT _fAmplification, orxFLOAT _fAcceleration, orxVECTOR *_pvStartColor, orxVECTOR *_pvEndColor, orxFX_CURVE _eCurve, orxFLOAT _fPow, orxU32 _u32Flags);

/** Adds HSL color to an FX
 * @param[in]   _pstFX          Concerned FX
 * @param[in]   _fStartTime     Time start
 * @param[in]   _fEndTime       Time end
 * @param[in]   _fCyclePeriod   Cycle period
 * @param[in]   _fCyclePhase    Cycle phase (at start)
 * @param[in]   _fAmplification Curve linear amplification over time (1.0 for none)
 * @param[in]   _fAcceleration  Curve linear acceleration over time (1.0 for none)
 * @param[in]   _pvStartColor   Starting color value
 * @param[in]   _pvEndColor     Ending color value
 * @param[in]   _eCurve         Blending curve type
 * @param[in]   _fPow           Blending curve exponent
 * @param[in]   _u32Flags       Param flags
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxFX_AddHSL(orxFX *_pstFX, orxFLOAT _fStartTime, orxFLOAT _fEndTime, orxFLOAT _fCyclePeriod, orxFLOAT _fCyclePhase, orxFLOAT _fAmplification, orxFLOAT _fAcceleration, orxVECTOR *_pvStartColor, orxVECTOR *_pvEndColor, orxFX_CURVE _eCurve, orxFLOAT _fPow, orxU32 _u32Flags);

/** Adds HSV color to an FX
 * @param[in]   _pstFX          Concerned FX
 * @param[in]   _fStartTime     Time start
 * @param[in]   _fEndTime       Time end
 * @param[in]   _fCyclePeriod   Cycle period
 * @param[in]   _fCyclePhase    Cycle phase (at start)
 * @param[in]   _fAmplification Curve linear amplification over time (1.0 for none)
 * @param[in]   _fAcceleration  Curve linear acceleration over time (1.0 for none)
 * @param[in]   _pvStartColor   Starting color value
 * @param[in]   _pvEndColor     Ending color value
 * @param[in]   _eCurve         Blending curve type
 * @param[in]   _fPow           Blending curve exponent
 * @param[in]   _u32Flags       Param flags
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxFX_AddHSV(orxFX *_pstFX, orxFLOAT _fStartTime, orxFLOAT _fEndTime, orxFLOAT _fCyclePeriod, orxFLOAT _fCyclePhase, orxFLOAT _fAmplification, orxFLOAT _fAcceleration, orxVECTOR *_pvStartColor, orxVECTOR *_pvEndColor, orxFX_CURVE _eCurve, orxFLOAT _fPow, orxU32 _u32Flags);

/** Adds rotation to an FX
 * @param[in]   _pstFX          Concerned FX
 * @param[in]   _fStartTime     Time start
 * @param[in]   _fEndTime       Time end
 * @param[in]   _fCyclePeriod   Cycle period
 * @param[in]   _fCyclePhase    Cycle phase (at start)
 * @param[in]   _fAmplification Curve linear amplification over time (1.0 for none)
 * @param[in]   _fAcceleration  Curve linear acceleration over time (1.0 for none)
 * @param[in]   _fStartRotation Starting rotation value (radians)
 * @param[in]   _fEndRotation   Ending rotation value (radians)
 * @param[in]   _eCurve         Blending curve type
 * @param[in]   _fPow           Blending curve exponent
 * @param[in]   _u32Flags       Param flags
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxFX_AddRotation(orxFX *_pstFX, orxFLOAT _fStartTime, orxFLOAT _fEndTime, orxFLOAT _fCyclePeriod, orxFLOAT _fCyclePhase, orxFLOAT _fAmplification, orxFLOAT _fAcceleration, orxFLOAT _fStartRotation, orxFLOAT _fEndRotation, orxFX_CURVE _eCurve, orxFLOAT _fPow, orxU32 _u32Flags);

/** Adds scale to an FX
 * @param[in]   _pstFX          Concerned FX
 * @param[in]   _fStartTime     Time start
 * @param[in]   _fEndTime       Time end
 * @param[in]   _fCyclePeriod   Cycle period
 * @param[in]   _fCyclePhase    Cycle phase (at start)
 * @param[in]   _fAmplification Curve linear amplification over time (1.0 for none)
 * @param[in]   _fAcceleration  Curve linear acceleration over time (1.0 for none)
 * @param[in]   _pvStartScale   Starting scale value
 * @param[in]   _pvEndScale     Ending scale value
 * @param[in]   _eCurve         Blending curve type
 * @param[in]   _fPow           Blending curve exponent
 * @param[in]   _u32Flags       Param flags
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxFX_AddScale(orxFX *_pstFX, orxFLOAT _fStartTime, orxFLOAT _fEndTime, orxFLOAT _fCyclePeriod, orxFLOAT _fCyclePhase, orxFLOAT _fAmplification, orxFLOAT _fAcceleration, const orxVECTOR *_pvStartScale, const orxVECTOR *_pvEndScale, orxFX_CURVE _eCurve, orxFLOAT _fPow, orxU32 _u32Flags);

/** Adds position to an FX
 * @param[in]   _pstFX          Concerned FX
 * @param[in]   _fStartTime     Time start
 * @param[in]   _fEndTime       Time end
 * @param[in]   _fCyclePeriod   Cycle period
 * @param[in]   _fCyclePhase    Cycle phase (at start)
 * @param[in]   _fAmplification Curve linear amplification over time (1.0 for none)
 * @param[in]   _fAcceleration  Curve linear acceleration over time (1.0 for none)
 * @param[in]   _pvStartTranslation Starting position value
 * @param[in]   _pvEndTranslation Ending position value
 * @param[in]   _eCurve         Blending curve type
 * @param[in]   _fPow           Blending curve exponent
 * @param[in]   _u32Flags       Param flags
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxFX_AddPosition(orxFX *_pstFX, orxFLOAT _fStartTime, orxFLOAT _fEndTime, orxFLOAT _fCyclePeriod, orxFLOAT _fCyclePhase, orxFLOAT _fAmplification, orxFLOAT _fAcceleration, const orxVECTOR *_pvStartTranslation, const orxVECTOR *_pvEndTranslation, orxFX_CURVE _eCurve, orxFLOAT _fPow, orxU32 _u32Flags);

/** Adds speed to an FX
 * @param[in]   _pstFX          Concerned FX
 * @param[in]   _fStartTime     Time start
 * @param[in]   _fEndTime       Time end
 * @param[in]   _fCyclePeriod   Cycle period
 * @param[in]   _fCyclePhase    Cycle phase (at start)
 * @param[in]   _fAmplification Curve linear amplification over time (1.0 for none)
 * @param[in]   _fAcceleration  Curve linear acceleration over time (1.0 for none)
 * @param[in]   _pvStartSpeed   Starting speed value
 * @param[in]   _pvEndSpeed     Ending speed value
 * @param[in]   _eCurve         Blending curve type
 * @param[in]   _fPow           Blending curve exponent
 * @param[in]   _u32Flags       Param flags
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxFX_AddSpeed(orxFX *_pstFX, orxFLOAT _fStartTime, orxFLOAT _fEndTime, orxFLOAT _fCyclePeriod, orxFLOAT _fCyclePhase, orxFLOAT _fAmplification, orxFLOAT _fAcceleration, const orxVECTOR *_pvStartSpeed, const orxVECTOR *_pvEndSpeed, orxFX_CURVE _eCurve, orxFLOAT _fPow, orxU32 _u32Flags);

/** Adds volume to an FX
 * @param[in]   _pstFX          Concerned FX
 * @param[in]   _fStartTime     Time start
 * @param[in]   _fEndTime       Time end
 * @param[in]   _fCyclePeriod   Cycle period
 * @param[in]   _fCyclePhase    Cycle phase (at start)
 * @param[in]   _fAmplification Curve linear amplification over time (1.0 for none)
 * @param[in]   _fAcceleration  Curve linear acceleration over time (1.0 for none)
 * @param[in]   _fStartVolume   Starting volume value
 * @param[in]   _fEndVolume     Ending volume value
 * @param[in]   _eCurve         Blending curve type
 * @param[in]   _fPow           Blending curve exponent
 * @param[in]   _u32Flags       Param flags
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxFX_AddVolume(orxFX *_pstFX, orxFLOAT _fStartTime, orxFLOAT _fEndTime, orxFLOAT _fCyclePeriod, orxFLOAT _fCyclePhase, orxFLOAT _fAmplification, orxFLOAT _fAcceleration, orxFLOAT _fStartVolume, orxFLOAT _fEndVolume, orxFX_CURVE _eCurve, orxFLOAT _fPow, orxU32 _u32Flags);

/** Adds pitch to an FX
 * @param[in]   _pstFX          Concerned FX
 * @param[in]   _fStartTime     Time start
 * @param[in]   _fEndTime       Time end
 * @param[in]   _fCyclePeriod   Cycle period
 * @param[in]   _fCyclePhase    Cycle phase (at start)
 * @param[in]   _fAmplification Curve linear amplification over time (1.0 for none)
 * @param[in]   _fAcceleration  Curve linear acceleration over time (1.0 for none)
 * @param[in]   _fStartPitch    Starting pitch value
 * @param[in]   _fEndPitch      Ending pitch value
 * @param[in]   _eCurve         Blending curve type
 * @param[in]   _fPow           Blending curve exponent
 * @param[in]   _u32Flags       Param flags
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxFX_AddPitch(orxFX *_pstFX, orxFLOAT _fStartTime, orxFLOAT _fEndTime, orxFLOAT _fCyclePeriod, orxFLOAT _fCyclePhase, orxFLOAT _fAmplification, orxFLOAT _fAcceleration, orxFLOAT _fStartPitch, orxFLOAT _fEndPitch, orxFX_CURVE _eCurve, orxFLOAT _fPow, orxU32 _u32Flags);

/** Adds a slot to an FX from config
 * @param[in]   _pstFX          Concerned FX
 * @param[in]   _zSlotID        Config ID
 * return       orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxFX_AddSlotFromConfig(orxFX *_pstFX, const orxSTRING _zSlotID);

/** Gets FX duration
 * @param[in]   _pstFX          Concerned FX
 * @return      orxFLOAT
 */
extern orxDLLAPI orxFLOAT orxFASTCALL           orxFX_GetDuration(const orxFX *_pstFX);

/** Gets FX name
 * @param[in]   _pstFX          Concerned FX
 * @return      orxSTRING / orxSTRING_EMPTY
 */
extern orxDLLAPI const orxSTRING orxFASTCALL    orxFX_GetName(const orxFX *_pstFX);

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
extern orxDLLAPI orxBOOL orxFASTCALL            orxFX_IsLooping(const orxFX *_pstFX);

#endif /* _orxFX_H_ */

/** @} */
