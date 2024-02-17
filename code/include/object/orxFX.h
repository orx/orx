/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008- Orx-Project
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
#define orxFX_SLOT_KU32_FLAG_ABSOLUTE           0x00010000  /**< Absolute flag */
#define orxFX_SLOT_KU32_FLAG_STAGGER            0x00020000  /**< Stagger flag */
#define orxFX_SLOT_KU32_FLAG_USE_ROTATION       0x00040000  /**< Use rotation flag */
#define orxFX_SLOT_KU32_FLAG_USE_SCALE          0x00080000  /**< Use scale flag */


/** FX type enum
 */
typedef enum __orxFX_TYPE_t
{
  orxFX_TYPE_ALPHA = 0,
  orxFX_TYPE_RGB,
  orxFX_TYPE_HSL,
  orxFX_TYPE_HSV,
  orxFX_TYPE_POSITION,
  orxFX_TYPE_ROTATION,
  orxFX_TYPE_SCALE,
  orxFX_TYPE_SPEED,
  orxFX_TYPE_VOLUME,
  orxFX_TYPE_PITCH,
  orxFX_TYPE_SIZE,
  orxFX_TYPE_ORIGIN,

  orxFX_TYPE_FLOAT_ADD,
  orxFX_TYPE_FLOAT_MULTIPLY,
  orxFX_TYPE_VECTOR_ADD,
  orxFX_TYPE_VECTOR_MULTIPLY,

  orxFX_TYPE_NUMBER,

  orxFX_TYPE_MAX_NUMBER = 16,

  orxFX_TYPE_NONE = orxENUM_NONE

} orxFX_TYPE;


/** FX curve enum
 */
typedef enum __orxFX_CURVE_t
{
  orxFX_CURVE_SINE = 0,
  orxFX_CURVE_SQUARE,
  orxFX_CURVE_TRIANGLE,

  orxFX_CURVE_SYMMETRIC_NUMBER,

  /* Below this point all curves are non-symmetrical */

  orxFX_CURVE_BEZIER = orxFX_CURVE_SYMMETRIC_NUMBER,
  orxFX_CURVE_LINEAR,
  orxFX_CURVE_SMOOTH,
  orxFX_CURVE_SMOOTHER,

  orxFX_CURVE_EASE_IN_SINE,
  orxFX_CURVE_EASE_OUT_SINE,
  orxFX_CURVE_EASE_IN_OUT_SINE,
  orxFX_CURVE_EASE_IN_QUAD,
  orxFX_CURVE_EASE_OUT_QUAD,
  orxFX_CURVE_EASE_IN_OUT_QUAD,
  orxFX_CURVE_EASE_IN_CUBIC,
  orxFX_CURVE_EASE_OUT_CUBIC,
  orxFX_CURVE_EASE_IN_OUT_CUBIC,
  orxFX_CURVE_EASE_IN_QUART,
  orxFX_CURVE_EASE_OUT_QUART,
  orxFX_CURVE_EASE_IN_OUT_QUART,
  orxFX_CURVE_EASE_IN_QUINT,
  orxFX_CURVE_EASE_OUT_QUINT,
  orxFX_CURVE_EASE_IN_OUT_QUINT,
  orxFX_CURVE_EASE_IN_EXPO,
  orxFX_CURVE_EASE_OUT_EXPO,
  orxFX_CURVE_EASE_IN_OUT_EXPO,
  orxFX_CURVE_EASE_IN_CIRC,
  orxFX_CURVE_EASE_OUT_CIRC,
  orxFX_CURVE_EASE_IN_OUT_CIRC,
  orxFX_CURVE_EASE_IN_BACK,
  orxFX_CURVE_EASE_OUT_BACK,
  orxFX_CURVE_EASE_IN_OUT_BACK,
  orxFX_CURVE_EASE_IN_ELASTIC,
  orxFX_CURVE_EASE_OUT_ELASTIC,
  orxFX_CURVE_EASE_IN_OUT_ELASTIC,
  orxFX_CURVE_EASE_IN_BOUNCE,
  orxFX_CURVE_EASE_OUT_BOUNCE,
  orxFX_CURVE_EASE_IN_OUT_BOUNCE,

  orxFX_CURVE_NUMBER,

  orxFX_CURVE_NONE = orxENUM_NONE

} orxFX_CURVE;


/** FX curve param
 */
typedef struct __orxFX_CURVE_PARAM_t
{
  union
  {
    struct
    {
      orxFLOAT          fStartValue;
      orxFLOAT          fEndValue;
    };

    struct
    {
      orxVECTOR         vStartValue;
      orxVECTOR         vEndValue;
    };
  };

  orxVECTOR             vCurvePoint1;           /**< Only used for Bezier curves, as first control point */
  orxVECTOR             vCurvePoint2;           /**< Only used for Bezier curves, as second control point */

  orxFLOAT              fStartTime;
  orxFLOAT              fEndTime;
  orxFLOAT              fPeriod;
  orxFLOAT              fPhase;
  orxFLOAT              fAmplification;
  orxFLOAT              fAcceleration;
  orxFLOAT              fPow;

} orxFX_CURVE_PARAM;


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
 * @return      orxFX / orxNULL
 */
extern orxDLLAPI orxFX *orxFASTCALL             orxFX_Create();

/** Creates an FX from config
 * @param[in]   _zConfigID      Config ID
 * @ return orxFX / orxNULL
 */
extern orxDLLAPI orxFX *orxFASTCALL             orxFX_CreateFromConfig(const orxSTRING _zConfigID);

/** Deletes an FX
 * @param[in]   _pstFX          Concerned FX
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxFX_Delete(orxFX *_pstFX);

/** Clears cache (if any FX is still in active use, it'll remain in memory until not referenced anymore)
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxFX_ClearCache();

/** Applies FX on object
 * @param[in]   _pstFX          FX to apply
 * @param[in]   _pstObject      Object on which to apply the FX
 * @param[in]   _fPreviousTime  Previous time
 * @param[in]   _fTime          Current time
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxFX_Apply(const orxFX *_pstFX, orxOBJECT *_pstObject, orxFLOAT _fPreviousTime, orxFLOAT _fTime);

/** Gets FX user float value between two timestamps
 * @param[in]   _pstFX          Concerned FX
 * @param[in]   _fPreviousTime  Previous time, ignored for Absolute slots. If <= orxFLOAT_0, the value at the current time will be returned, otherwise the value delta between both times will be returned
 * @param[in]   _fTime          Current time
 * @return      Float value
 */
extern orxDLLAPI orxFLOAT orxFASTCALL           orxFX_GetFloat(const orxFX *_pstFX, orxFLOAT _fPreviousTime, orxFLOAT _fTime);

/** Gets FX user vector value between two timestamps
 * @param[in]   _pstFX          Concerned FX
 * @param[in]   _fPreviousTime  Previous time, ignored for Absolute slots. If <= orxFLOAT_0, the value at the current time will be returned, otherwise the value delta between both times will be returned
 * @param[in]   _fTime          Current time
 * @param[out]  _pvVector       Storage for vector value
 * @return      Vector value if valid, orxNULL otherwise
 */
extern orxDLLAPI orxVECTOR *orxFASTCALL         orxFX_GetVector(const orxFX *_pstFX, orxFLOAT _fPreviousTime, orxFLOAT _fTime, orxVECTOR *_pvVector);

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

/** Adds a slot to an FX
 * @param[in]   _pstFX          Concerned FX
 * @param[in]   _eType          Type
 * @param[in]   _eCurve         Curve
 * @param[in]   _pstCurveParam  Curve parameters
 * @param[in]   _u32Flags       Additional flags (Absolute, Stagger, UseRotation/UseScale for position/speed types only)
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxFX_AddSlot(orxFX *_pstFX, orxFX_TYPE _eType, orxFX_CURVE _eCurve, const orxFX_CURVE_PARAM *_pstCurveParam, orxU32 _u32Flags);

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

/** Sets FX stagger / offset
 * @param[in]   _pstFX          Concerned FX
 * @param[in]   _bStagger       If true, this FX will be added after all current FXs
 * @param[in]   _fOffset        Initial offset, in seconds. Cannot result in a negative start time
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxFX_SetStagger(orxFX *_pstFX, orxBOOL _bStagger, orxFLOAT _fOffset);

/** Get FX stagger / offset
 * @param[in]   _pstFX          Concerned FX
 * @param[out]  _pfOffset       If non null, will contain the initial offset
 * @return      orxTRUE if staggered, orxFALSE otherwise
 */
extern orxDLLAPI orxBOOL orxFASTCALL            orxFX_GetStagger(const orxFX *_pstFX, orxFLOAT *_pfOffset);

#endif /* _orxFX_H_ */

/** @} */
