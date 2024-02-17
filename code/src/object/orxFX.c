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
 * @file orxFX.c
 * @date 30/06/2008
 * @author iarwain@orx-project.org
 *
 */


#include "object/orxFX.h"

#include "debug/orxDebug.h"
#include "memory/orxMemory.h"
#include "core/orxConfig.h"
#include "core/orxClock.h"
#include "core/orxEvent.h"
#include "core/orxResource.h"
#include "display/orxDisplay.h"
#include "object/orxStructure.h"
#include "utils/orxHashTable.h"
#include "utils/orxString.h"


/** Module flags
 */
#define orxFX_KU32_STATIC_FLAG_NONE             0x00000000

#define orxFX_KU32_STATIC_FLAG_READY            0x00000001

#define orxFX_KU32_STATIC_MASK_ALL              0xFFFFFFFF


/** Flags
 */
#define orxFX_KU32_FLAG_NONE                    0x00000000  /**< No flags */

#define orxFX_KU32_FLAG_ENABLED                 0x10000000  /**< Enabled flag */
#define orxFX_KU32_FLAG_LOOP                    0x20000000  /**< Loop flag */
#define orxFX_KU32_FLAG_CACHED                  0x40000000  /**< Cached flag */
#define orxFX_KU32_FLAG_STAGGERED               0x80000000  /**< Staggered flag */

#define orxFX_KU32_MASK_ALL                     0xFFFFFFFF  /**< All mask */


/** Slot flags
 */
#define orxFX_SLOT_KU32_FLAG_NONE               0x00000000  /**< No flag */

#define orxFX_SLOT_KU32_FLAG_DEFINED            0x10000000  /**< Defined flag */
#define orxFX_SLOT_KU32_FLAG_AMPLIFICATION      0x20000000  /**< Amplification flag */
#define orxFX_SLOT_KU32_FLAG_ACCELERATION       0x40000000  /**< Acceleration flag */
#define orxFX_SLOT_KU32_FLAG_POW                0x80000000  /**< Pow flag */

#define orxFX_SLOT_KU32_MASK_CURVE              0x000000FF  /**< Curve mask */

#define orxFX_SLOT_KU32_MASK_TYPE               0x00000F00  /**< FX type mask */

#define orxFX_SLOT_KU32_MASK_OPERATION          0x00003000  /**< FX operation mask */

#define orxFX_SLOT_KU32_MASK_VALUE_TYPE         0x0000C000  /**< FX value type mask */

#define orxFX_SLOT_KU32_MASK_USER_ALL           0x00FF0000  /**< User all mask */

#define orxFX_SLOT_KU32_MASK_ALL                0xFFFFFFFF  /**< All mask */


#define orxFX_SLOT_KU32_SHIFT_TYPE              8
#define orxFX_SLOT_KU32_SHIFT_OPERATION         12
#define orxFX_SLOT_KU32_SHIFT_VALUE_TYPE        14


/** Misc defines
 */
#define orxFX_KU32_REFERENCE_TABLE_SIZE         128
#define orxFX_KU32_CURVE_TABLE_SIZE             64

#define orxFX_KU32_SLOT_NUMBER                  8

#define orxFX_KU32_BANK_SIZE                    128

#define orxFX_KZ_CONFIG_SLOT_LIST               "SlotList"
#define orxFX_KZ_CONFIG_TYPE                    "Type"
#define orxFX_KZ_CONFIG_CURVE                   "Curve"
#define orxFX_KZ_CONFIG_POW                     "Pow"
#define orxFX_KZ_CONFIG_ABSOLUTE                "Absolute"
#define orxFX_KZ_CONFIG_LOOP                    "Loop"
#define orxFX_KZ_CONFIG_STAGGER                 "Stagger"
#define orxFX_KZ_CONFIG_AMPLIFICATION           "Amplification"
#define orxFX_KZ_CONFIG_ACCELERATION            "Acceleration"
#define orxFX_KZ_CONFIG_START_TIME              "StartTime"
#define orxFX_KZ_CONFIG_END_TIME                "EndTime"
#define orxFX_KZ_CONFIG_PERIOD                  "Period"
#define orxFX_KZ_CONFIG_PHASE                   "Phase"
#define orxFX_KZ_CONFIG_START_VALUE             "StartValue"
#define orxFX_KZ_CONFIG_END_VALUE               "EndValue"
#define orxFX_KZ_CONFIG_KEEP_IN_CACHE           "KeepInCache"
#define orxFX_KZ_CONFIG_USE_ROTATION            "UseRotation"
#define orxFX_KZ_CONFIG_USE_SCALE               "UseScale"
#define orxFX_KZ_CONFIG_DO_NOT_CACHE            "DoNotCache"

#define orxFX_KZ_LINEAR                         "linear"
#define orxFX_KZ_TRIANGLE                       "triangle"
#define orxFX_KZ_SQUARE                         "square"
#define orxFX_KZ_SINE                           "sine"
#define orxFX_KZ_SMOOTH                         "smooth"
#define orxFX_KZ_SMOOTHER                       "smoother"
#define orxFX_KZ_BEZIER                         "bezier"

#define orxFX_KZ_EASE_IN_SINE                   "easeinsine"
#define orxFX_KZ_EASE_OUT_SINE                  "easeoutsine"
#define orxFX_KZ_EASE_IN_OUT_SINE               "easeinoutsine"
#define orxFX_KZ_EASE_IN_QUAD                   "easeinquad"
#define orxFX_KZ_EASE_OUT_QUAD                  "easeoutquad"
#define orxFX_KZ_EASE_IN_OUT_QUAD               "easeinoutquad"
#define orxFX_KZ_EASE_IN_CUBIC                  "easeincubic"
#define orxFX_KZ_EASE_OUT_CUBIC                 "easeoutcubic"
#define orxFX_KZ_EASE_IN_OUT_CUBIC              "easeinoutcubic"
#define orxFX_KZ_EASE_IN_QUART                  "easeinquart"
#define orxFX_KZ_EASE_OUT_QUART                 "easeoutquart"
#define orxFX_KZ_EASE_IN_OUT_QUART              "easeinoutquart"
#define orxFX_KZ_EASE_IN_QUINT                  "easeinquint"
#define orxFX_KZ_EASE_OUT_QUINT                 "easeoutquint"
#define orxFX_KZ_EASE_IN_OUT_QUINT              "easeinoutquint"
#define orxFX_KZ_EASE_IN_EXPO                   "easeinexpo"
#define orxFX_KZ_EASE_OUT_EXPO                  "easeoutexpo"
#define orxFX_KZ_EASE_IN_OUT_EXPO               "easeinoutexpo"
#define orxFX_KZ_EASE_IN_CIRC                   "easeincirc"
#define orxFX_KZ_EASE_OUT_CIRC                  "easeoutcirc"
#define orxFX_KZ_EASE_IN_OUT_CIRC               "easeinoutcirc"
#define orxFX_KZ_EASE_IN_BACK                   "easeinback"
#define orxFX_KZ_EASE_OUT_BACK                  "easeoutback"
#define orxFX_KZ_EASE_IN_OUT_BACK               "easeinoutback"
#define orxFX_KZ_EASE_IN_ELASTIC                "easeinelastic"
#define orxFX_KZ_EASE_OUT_ELASTIC               "easeoutelastic"
#define orxFX_KZ_EASE_IN_OUT_ELASTIC            "easeinoutelastic"
#define orxFX_KZ_EASE_IN_BOUNCE                 "easeinbounce"
#define orxFX_KZ_EASE_OUT_BOUNCE                "easeoutbounce"
#define orxFX_KZ_EASE_IN_OUT_BOUNCE             "easeinoutbounce"

#define orxFX_KZ_ALPHA                          "alpha"
#define orxFX_KZ_COLOR                          "color"
#define orxFX_KZ_RGB                            "rgb"
#define orxFX_KZ_HSL                            "hsl"
#define orxFX_KZ_HSV                            "hsv"
#define orxFX_KZ_ROTATION                       "rotation"
#define orxFX_KZ_SCALE                          "scale"
#define orxFX_KZ_POSITION                       "position"
#define orxFX_KZ_SPEED                          "speed"
#define orxFX_KZ_VOLUME                         "volume"
#define orxFX_KZ_PITCH                          "pitch"
#define orxFX_KZ_SIZE                           "size"
#define orxFX_KZ_ORIGIN                         "origin"
#define orxFX_KZ_VECTOR                         "vector"
#define orxFX_KZ_FLOAT                          "float"
#define orxFX_KZ_MULTIPLY                       "multiply"


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** FX operation enum
 */
typedef enum __orxFX_OPERATION_t
{
  orxFX_OPERATION_ADD = 0,
  orxFX_OPERATION_MULTIPLY,

  orxFX_OPERATION_NUMBER,

  orxFX_OPERATION_NONE = orxENUM_NONE

} orxFX_OPERATION;


/** FX value type
 */
typedef enum __orxFX_VALUE_TYPE_t
{
  orxFX_VALUE_TYPE_FLOAT = 0,
  orxFX_VALUE_TYPE_VECTOR,

  orxFX_VALUE_TYPE_NUMBER,

  orxFX_VALUE_TYPE_NONE = orxENUM_NONE

} orxFX_VALUE_TYPE;


/** FX context
 */
typedef struct __orxFX_CONTEXT_t
{
  union
  {
    orxVECTOR       vValue;
    orxFLOAT        fValue;
  }                 astValueList[orxFX_TYPE_NUMBER];
  const orxOBJECT  *pstObject;
  orxU32            u32LockFlags, u32UpdateFlags;
  orxFX_TYPE        eColorBlendUpdate;
  orxFLOAT          fStartTime, fEndTime;
} orxFX_CONTEXT;

/** FX slot
 */
typedef struct __orxFX_SLOT_t
{
  orxFX_CURVE_PARAM stCurveParam;                         /**< Curve param : 76 */
  orxU32            u32Flags;                             /**< Flags : 80 */

} orxFX_SLOT;

/** FX structure
 */
struct __orxFX_t
{
  orxSTRUCTURE    stStructure;                            /**< Public structure, first structure member : 64 */
  const orxSTRING zReference;                             /**< FX reference : 72 */
  orxFLOAT        fDuration;                              /**< FX duration : 76 */
  orxFLOAT        fOffset;                                /**< FX offset : 80 */
  orxFX_SLOT      astFXSlotList[orxFX_KU32_SLOT_NUMBER];  /**< FX slot list : 540 */
};

/** Static structure
 */
typedef struct __orxFX_STATIC_t
{
  orxHASHTABLE *pstReferenceTable;                        /**< Reference hash table */
  orxHASHTABLE *pstIDTable;                               /**< ID table */
  orxU32        u32Flags;                                 /**< Control flags */

} orxFX_STATIC;


/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** Static data
 */
static orxFX_STATIC sstFX;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

/* Inits context
 */
static orxINLINE void orxFX_InitContext(orxFX_CONTEXT *_pstContext, const orxFX *_pstFX, const orxOBJECT *_pstObject, orxFLOAT _fStartTime, orxFLOAT _fEndTime)
{
  /* Clears  context */
  orxMemory_Zero(_pstContext, sizeof(orxFX_CONTEXT));

  /* User? */
  if(_pstObject == orxNULL)
  {
    /* Clamps start time */
    _fStartTime = orxMAX(_fStartTime, orxFLOAT_0);

    /* Not instant and looping? */
    if((_pstFX->fDuration > orxFLOAT_0)
    && (orxStructure_TestFlags(_pstFX, orxFX_KU32_FLAG_LOOP)))
    {
      /* Should wrap around? */
      if(_fEndTime > _pstFX->fDuration)
      {
        /* Current value requested? */
        if(_fStartTime == orxFLOAT_0)
        {
          orxFLOAT fTemp;

          /* Wraps around end time */
          fTemp     = orxMath_Mod(_fEndTime, _pstFX->fDuration);
          _fEndTime = (fTemp == orxFLOAT_0) ? _pstFX->fDuration : fTemp;
        }
        else
        {
          orxFLOAT fTemp;

          /* Wraps around both times */
          fTemp       = orxMath_Mod(_fStartTime, _pstFX->fDuration);
          _fEndTime  += fTemp - _fStartTime;
          _fStartTime = fTemp;
        }
      }
    }
  }

  /* Inits multiply neutral elements */
  orxVector_Copy(&(_pstContext->astValueList[orxFX_TYPE_SCALE].vValue), &orxVECTOR_1);
  orxVector_Copy(&(_pstContext->astValueList[orxFX_TYPE_VECTOR_MULTIPLY].vValue), &orxVECTOR_1);
  _pstContext->astValueList[orxFX_TYPE_PITCH].fValue          = orxFLOAT_1;
  _pstContext->astValueList[orxFX_TYPE_FLOAT_MULTIPLY].fValue = orxFLOAT_1;

  /* Inits color blend update */
  _pstContext->eColorBlendUpdate = orxFX_TYPE_NONE;

  /* Stores object */
  _pstContext->pstObject  = _pstObject;

  /* Stores times */
  _pstContext->fStartTime = _fStartTime;
  _pstContext->fEndTime   = _fEndTime;

  /* Done! */
  return;
}

/** Gets FX slot type
 * @param[in] _pstFXSlot        Concerned FX slot
 * @return orxFX_TYPE
 */
static orxINLINE orxFX_TYPE orxFX_GetSlotType(const orxFX_SLOT *_pstFXSlot)
{
  orxFX_TYPE eResult;

  /* Checks */
  orxASSERT(_pstFXSlot != orxNULL);

  /* Updates result */
  eResult = (orxFX_TYPE)((_pstFXSlot->u32Flags & orxFX_SLOT_KU32_MASK_TYPE) >> orxFX_SLOT_KU32_SHIFT_TYPE);

  /* Done! */
  return eResult;
}

/** Finds the first empty slot
 * @param[in] _pstFX            Concerned FX
 * @return orxU32 / orxU32_UNDEFINED
 */
static orxINLINE orxU32 orxFX_FindEmptySlotIndex(const orxFX *_pstFX)
{
  orxU32 i, u32Result = orxU32_UNDEFINED;

  /* Checks */
  orxSTRUCTURE_ASSERT(_pstFX);

  /* For all slots */
  for(i = 0; i < orxFX_KU32_SLOT_NUMBER; i++)
  {
    /* Empty? */
    if(!orxFLAG_TEST(_pstFX->astFXSlotList[i].u32Flags, orxFX_SLOT_KU32_FLAG_DEFINED))
    {
      /* Updates result */
      u32Result = i;
      break;
    }
  }

  /* Done! */
  return u32Result;
}

/** Processes config data
 */
static orxINLINE orxSTATUS orxFX_ProcessData(orxFX *_pstFX)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Has reference? */
  if((_pstFX->zReference != orxNULL)
  && (*(_pstFX->zReference) != orxCHAR_NULL))
  {
    orxU32 u32SlotCount, i;

    /* Pushes its config section */
    orxConfig_PushSection(_pstFX->zReference);

    /* Gets number of declared slots */
    u32SlotCount = orxConfig_GetListCount(orxFX_KZ_CONFIG_SLOT_LIST);

    /* Too many slots? */
    if(u32SlotCount > orxFX_KU32_SLOT_NUMBER)
    {
      /* For all exceeding slots */
      for(i = orxFX_KU32_SLOT_NUMBER; i < u32SlotCount; i++)
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "[%s]: Too many slots for this FX, can't add slot <%s>.", _pstFX->zReference, orxConfig_GetListString(orxFX_KZ_CONFIG_SLOT_LIST, i));
      }

      /* Updates slot count */
      u32SlotCount = orxFX_KU32_SLOT_NUMBER;
    }

    /* For all slots */
    for(i = 0; i < u32SlotCount; i++)
    {
      const orxSTRING zSlotName;

      /* Gets its name */
      zSlotName = orxConfig_GetListString(orxFX_KZ_CONFIG_SLOT_LIST, i);

      /* Valid? */
      if((zSlotName != orxNULL) && (zSlotName != orxSTRING_EMPTY))
      {
        /* Adds slot from config */
        orxFX_AddSlotFromConfig(_pstFX, zSlotName);
      }
      else
      {
        /* Stops */
        break;
      }
    }

    /* Should loop? */
    if(orxConfig_GetBool(orxFX_KZ_CONFIG_LOOP) != orxFALSE)
    {
      /* Updates flags */
      orxStructure_SetFlags(_pstFX, orxFX_KU32_FLAG_LOOP, orxFX_KU32_FLAG_NONE);
    }

    /* Has stagger property? */
    if(orxConfig_HasValue(orxFX_KZ_CONFIG_STAGGER) != orxFALSE)
    {
      /* Sets stagger */
      orxFX_SetStagger(_pstFX, orxConfig_GetListBool(orxFX_KZ_CONFIG_STAGGER, 0), (orxConfig_GetListCount(orxFX_KZ_CONFIG_STAGGER) > 1) ? orxConfig_GetListFloat(orxFX_KZ_CONFIG_STAGGER, 1) : orxFLOAT_0);
    }

    /* Pops config section */
    orxConfig_PopSection();

    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Done! */
  return eResult;
}

static orxINLINE orxFLOAT orxFX_EaseOutBounce(orxFLOAT _fTime)
{
  orxFLOAT fN = orx2F(7.5625f), fD = orx2F(1.0f / 2.75f), fTemp, fResult;

  /* Updates result */
  fResult = (_fTime < fD)
            ? fN * _fTime * _fTime
            : (_fTime < orx2F(2.0f) * fD)
              ? (fTemp = _fTime - orx2F(1.5f) * fD, fN * fTemp * fTemp + orx2F(0.75f))
              : (_fTime < orx2F(2.5f) * fD)
                ? (fTemp = _fTime - orx2F(2.25f) * fD, fN * fTemp * fTemp + orx2F(0.9375f))
                : (fTemp = _fTime - orx2F(2.625f) * fD, fN * fTemp * fTemp + orx2F(0.984375f));

  /* Done! */
  return fResult;
}

/** Get Curve value
 */
static orxINLINE orxFLOAT orxFX_GetCurveValue(const orxFX_SLOT *_pstFXSlot, orxFLOAT _fTime)
{
  orxFLOAT fResult;

  /* Depending on curve */
  switch(orxFLAG_GET(_pstFXSlot->u32Flags, orxFX_SLOT_KU32_MASK_CURVE))
  {
    case orxFX_CURVE_SINE:
    {
      /* Updates result */
      fResult = orx2F(0.5f) * (orxMath_Sin(orxMATH_KF_2_PI * _fTime - orxMATH_KF_PI_BY_2) + orxFLOAT_1);
      break;
    }

    case orxFX_CURVE_SQUARE:
    {
      /* Updates result */
      fResult = ((_fTime >= orx2F(0.25f)) && (_fTime < orx2F(0.75f))) ? orxFLOAT_1 : orxFLOAT_0;
      break;
    }

    case orxFX_CURVE_TRIANGLE:
    {
      /* Updates result */
      fResult = orx2F(2.0f) * _fTime;
      if(fResult > orxFLOAT_1)
      {
        fResult = orx2F(2.0f) - fResult;
      }
      break;
    }

    case orxFX_CURVE_BEZIER:
    {
      orxVECTOR vResult;

      /* Updates result */
      orxVector_Bezier(&vResult, &orxVECTOR_0, &(_pstFXSlot->stCurveParam.vCurvePoint1), &(_pstFXSlot->stCurveParam.vCurvePoint2), &orxVECTOR_1, _fTime);
      fResult = vResult.fY;
      break;
    }

    default:
    case orxFX_CURVE_LINEAR:
    {
      /* Updates result */
      fResult = _fTime;
      break;
    }

    case orxFX_CURVE_SMOOTH:
    {
      /* Updates result */
      fResult = (_fTime * _fTime) * (orx2F(3.0f) - (orx2F(2.0f) * _fTime));
      break;
    }

    case orxFX_CURVE_SMOOTHER:
    {
      /* Updates result */
      fResult = (_fTime * _fTime * _fTime) * (_fTime * ((_fTime * orx2F(6.0f)) - orx2F(15.0f)) + orx2F(10.0f));
      break;
    }

    case orxFX_CURVE_EASE_IN_SINE:
    {
      /* Updates result */
      fResult = orxFLOAT_1 - orxMath_Cos(_fTime * orxMATH_KF_PI_BY_2);
      break;
    }

    case orxFX_CURVE_EASE_OUT_SINE:
    {
      /* Updates result */
      fResult = orxMath_Sin(_fTime * orxMATH_KF_PI_BY_2);
      break;
    }

    case orxFX_CURVE_EASE_IN_OUT_SINE:
    {
      /* Updates result */
      fResult = orx2F(0.5f) * (orxFLOAT_1 - orxMath_Cos(orxMATH_KF_PI * _fTime));
      break;
    }

    case orxFX_CURVE_EASE_IN_QUAD:
    {
      /* Updates result */
      fResult = _fTime * _fTime;
      break;
    }

    case orxFX_CURVE_EASE_OUT_QUAD:
    {
      /* Updates result */
      fResult = orxFLOAT_1 - (orxFLOAT_1 - _fTime) * (orxFLOAT_1 - _fTime);
      break;
    }

    case orxFX_CURVE_EASE_IN_OUT_QUAD:
    {
      /* Updates result */
      orxFLOAT fTemp = orx2F(-2.0f) * _fTime + orx2F(2.0f);
      fResult = (_fTime < orx2F(0.5f)) ? orx2F(2.0f) * _fTime * _fTime : orxFLOAT_1 - orx2F(0.5f) * fTemp * fTemp;
      break;
    }

    case orxFX_CURVE_EASE_IN_CUBIC:
    {
      /* Updates result */
      fResult = _fTime * _fTime * _fTime;
      break;
    }

    case orxFX_CURVE_EASE_OUT_CUBIC:
    {
      /* Updates result */
      fResult = orxFLOAT_1 - (orxFLOAT_1 - _fTime) * (orxFLOAT_1 - _fTime) * (orxFLOAT_1 - _fTime);
      break;
    }

    case orxFX_CURVE_EASE_IN_OUT_CUBIC:
    {
      /* Updates result */
      orxFLOAT fTemp = orx2F(-2.0f) * _fTime + orx2F(2.0f);
      fResult = (_fTime < orx2F(0.5f)) ? orx2F(4.0f) * _fTime * _fTime * _fTime : orxFLOAT_1 - orx2F(0.5f) * fTemp * fTemp * fTemp;
      break;
    }

    case orxFX_CURVE_EASE_IN_QUART:
    {
      /* Updates result */
      fResult = _fTime * _fTime * _fTime * _fTime;
      break;
    }

    case orxFX_CURVE_EASE_OUT_QUART:
    {
      /* Updates result */
      fResult = orxFLOAT_1 - (orxFLOAT_1 - _fTime) * (orxFLOAT_1 - _fTime) * (orxFLOAT_1 - _fTime) * (orxFLOAT_1 - _fTime);
      break;
    }

    case orxFX_CURVE_EASE_IN_OUT_QUART:
    {
      /* Updates result */
      orxFLOAT fTemp = orx2F(-2.0f) * _fTime + orx2F(2.0f);
      fResult = (_fTime < orx2F(0.5f)) ? orx2F(8.0f) * _fTime * _fTime * _fTime * _fTime : orxFLOAT_1 - orx2F(0.5f) * fTemp * fTemp * fTemp * fTemp;
      break;
    }

    case orxFX_CURVE_EASE_IN_QUINT:
    {
      /* Updates result */
      fResult = _fTime * _fTime * _fTime * _fTime * _fTime;
      break;
    }

    case orxFX_CURVE_EASE_OUT_QUINT:
    {
      /* Updates result */
      fResult = orxFLOAT_1 - (orxFLOAT_1 - _fTime) * (orxFLOAT_1 - _fTime) * (orxFLOAT_1 - _fTime) * (orxFLOAT_1 - _fTime) * (orxFLOAT_1 - _fTime);
      break;
    }

    case orxFX_CURVE_EASE_IN_OUT_QUINT:
    {
      /* Updates result */
      orxFLOAT fTemp = orx2F(-2.0f) * _fTime + orx2F(2.0f);
      fResult = (_fTime < orx2F(0.5f)) ? orx2F(16.0f) * _fTime * _fTime * _fTime * _fTime * _fTime : orxFLOAT_1 - orx2F(0.5f) * fTemp * fTemp * fTemp * fTemp * fTemp;
      break;
    }

    case orxFX_CURVE_EASE_IN_EXPO:
    {
      /* Updates result */
      fResult = (_fTime == orxFLOAT_0) ? orxFLOAT_0 : orxMath_Pow(orx2F(2.0f), orx2F(10.0f) * _fTime - orx2F(10.0f));
      break;
    }

    case orxFX_CURVE_EASE_OUT_EXPO:
    {
      /* Updates result */
      fResult = (_fTime == orxFLOAT_1) ? orxFLOAT_1 : orxFLOAT_1 - orxMath_Pow(orx2F(2.0f), orx2F(-10.0f) * _fTime);
      break;
    }

    case orxFX_CURVE_EASE_IN_OUT_EXPO:
    {
      /* Updates result */
      fResult = (_fTime == orxFLOAT_0)
                ? orxFLOAT_0
                : (_fTime == orxFLOAT_1)
                  ? orxFLOAT_1
                  : (_fTime < orx2F(0.5f))
                    ? orx2F(0.5f) * orxMath_Pow(orx2F(2.0f), orx2F(20.0f) * _fTime - orx2F(10.0f))
                    : orxFLOAT_1 - orx2F(0.5f) * orxMath_Pow(orx2F(2.0f), orx2F(-20.0f) * _fTime + orx2F(10.0f));
      break;
    }

    case orxFX_CURVE_EASE_IN_CIRC:
    {
      /* Updates result */
      fResult = orxFLOAT_1 - orxMath_Sqrt(orxFLOAT_1 - _fTime * _fTime);
      break;
    }

    case orxFX_CURVE_EASE_OUT_CIRC:
    {
      /* Updates result */
      fResult = orxMath_Sqrt(orxFLOAT_1 - (_fTime - orxFLOAT_1) * (_fTime - orxFLOAT_1));
      break;
    }

    case orxFX_CURVE_EASE_IN_OUT_CIRC:
    {
      /* Updates result */
      fResult = (_fTime < orx2F(0.5f))
                ? orx2F(0.5f) * (orxFLOAT_1 - orxMath_Sqrt(orxFLOAT_1 - orx2F(4.0f) * _fTime * _fTime))
                : orx2F(0.5f) * (orxFLOAT_1 + orxMath_Sqrt(orxFLOAT_1 - (orx2F(-2.0f) * _fTime + orx2F(2.0f)) * (orx2F(-2.0f) * _fTime + orx2F(2.0f))));
      break;
    }

    case orxFX_CURVE_EASE_IN_BACK:
    {
      /* Updates result */
      orxFLOAT fW2 = orx2F(1.70158f), fW3 = fW2 + orxFLOAT_1;
      fResult = fW3 * _fTime * _fTime * _fTime - fW2 * _fTime * _fTime;
      break;
    }

    case orxFX_CURVE_EASE_OUT_BACK:
    {
      /* Updates result */
      orxFLOAT fW2 = orx2F(1.70158f), fW3 = fW2 + orxFLOAT_1;
      fResult = orxFLOAT_1 + fW3 * (_fTime - orxFLOAT_1) * (_fTime - orxFLOAT_1) * (_fTime - orxFLOAT_1) + fW2 * (_fTime - orxFLOAT_1) * (_fTime - orxFLOAT_1);
      break;
    }

    case orxFX_CURVE_EASE_IN_OUT_BACK:
    {
      /* Updates result */
      orxFLOAT fW = orx2F(1.70158f * 1.525f);
      fResult = (_fTime < orx2F(0.5f))
                ? orx2F(2.0f) * _fTime * _fTime * ((fW + orxFLOAT_1) * orx2F(2.0f) * _fTime - fW)
                : orx2F(0.5f) * ((orx2F(2.0f) * _fTime - orx2F(2.0f)) * (orx2F(2.0f) * _fTime - orx2F(2.0f)) * ((fW + orxFLOAT_1) * (_fTime * orx2F(2.0f) - orx2F(2.0f)) + fW) + orx2F(2.0f));
      break;
    }

    case orxFX_CURVE_EASE_IN_ELASTIC:
    {
      /* Updates result */
      orxFLOAT fW = (orxMATH_KF_2_PI) / orx2F(3.0f);
      fResult = (_fTime == orxFLOAT_0)
                ? orxFLOAT_0
                : (_fTime == orxFLOAT_1)
                  ? orxFLOAT_1
                  : -orxMath_Pow(orx2F(2.0f), orx2F(10.0f) * _fTime - orx2F(10.0f)) * orxMath_Sin(fW * (_fTime * orx2F(10.0f) - orx2F(10.75f)));
      break;
    }

    case orxFX_CURVE_EASE_OUT_ELASTIC:
    {
      /* Updates result */
      orxFLOAT fW = (orxMATH_KF_2_PI) / orx2F(3.0f);
      fResult = (_fTime == orxFLOAT_0)
                ? orxFLOAT_0
                : (_fTime == orxFLOAT_1)
                  ? orxFLOAT_1
                  : orxFLOAT_1 + orxMath_Pow(orx2F(2.0f), orx2F(-10.0f) * _fTime) * orxMath_Sin(fW * (_fTime * orx2F(10.0f) - orx2F(0.75f)));
      break;
    }

    case orxFX_CURVE_EASE_IN_OUT_ELASTIC:
    {
      /* Updates result */
      orxFLOAT fW = (orxMATH_KF_2_PI) / orx2F(4.5f);
      fResult = (_fTime == orxFLOAT_0)
                ? orxFLOAT_0
                : (_fTime == orxFLOAT_1)
                  ? orxFLOAT_1
                  : (_fTime < orx2F(0.5f))
                    ? orx2F(-0.5f) * orxMath_Pow(orx2F(2.0f), orx2F(20.0f) * _fTime - orx2F(10.0f)) * orxMath_Sin(fW * (_fTime * orx2F(20.0f) - orx2F(11.125f)))
                    : orxFLOAT_1 + orx2F(0.5f) * orxMath_Pow(orx2F(2.0f), orx2F(-20.0f) * _fTime + orx2F(10.0f)) * orxMath_Sin(fW * (_fTime * orx2F(20.0f) - orx2F(11.125f)));
      break;
    }

    case orxFX_CURVE_EASE_IN_BOUNCE:
    {
      /* Updates result */
      fResult = orxFLOAT_1 - orxFX_EaseOutBounce(orxFLOAT_1 - _fTime);
      break;
    }

    case orxFX_CURVE_EASE_OUT_BOUNCE:
    {
      /* Updates result */
      fResult = orxFX_EaseOutBounce(_fTime);
      break;
    }

    case orxFX_CURVE_EASE_IN_OUT_BOUNCE:
    {
      /* Updates result */
      fResult = (_fTime < orx2F(0.5f))
                ? orx2F(0.5f) * (orxFLOAT_1 - orxFX_EaseOutBounce(orxFLOAT_1 - orx2F(2.0f) * _fTime))
                : orx2F(0.5f) * (orxFLOAT_1 + orxFX_EaseOutBounce(orx2F(2.0f) * _fTime - orxFLOAT_1));
      break;
    }

  }

  /* Done! */
  return fResult;
}

/** Computes coef
 */
static orxINLINE orxFLOAT orxFX_ComputeCoef(const orxFX_SLOT *_pstFXSlot, orxFLOAT _fTime, orxFLOAT _fFrequency)
{
  orxFLOAT fResult;

  /* Gets linear coef in period [0.0; 1.0] starting at given phase */
  fResult = (_fTime * _fFrequency) + _pstFXSlot->stCurveParam.fPhase;

  /* Non zero? */
  if(fResult != orxFLOAT_0)
  {
    orxFX_CURVE eCurve;

    /* Gets its modulo */
    fResult = orxMath_Mod(fResult, orxFLOAT_1);

    /* Gets curve */
    eCurve = (orxFX_CURVE)orxFLAG_GET(_pstFXSlot->u32Flags, orxFX_SLOT_KU32_MASK_CURVE);

    /* Not symmetrical and has wrapped around? */
    if((eCurve >= orxFX_CURVE_SYMMETRIC_NUMBER) && (fResult == orxFLOAT_0))
    {
      /* Sets it at max value */
      fResult = orxFLOAT_1;
    }
    else
    {
      /* Gets curve value */
      fResult = orxFX_GetCurveValue(_pstFXSlot, fResult);
    }
  }

  /* Done! */
  return fResult;
}

static void orxFASTCALL orxFX_ComputeSlot(const orxFX_SLOT *_pstFXSlot, orxFX_CONTEXT *_pstContext)
{
  orxFLOAT fStartTime;
  orxBOOL  bFirstCall;

/* Some versions of GCC have an optimization bug on fEndTime which leads to a bogus value when reaching the end of a slot */
#if defined(__orxGCC__)
  volatile orxFLOAT fEndTime;
#else /* __orxGCC__  */
  orxFLOAT fEndTime;
#endif /* __orxGCC__ */

  /* Gets corrected start and end time */
  fStartTime  = orxMAX(_pstContext->fStartTime, _pstFXSlot->stCurveParam.fStartTime);
  fEndTime    = orxMIN(_pstContext->fEndTime, _pstFXSlot->stCurveParam.fEndTime);

  /* Updates first call status */
  bFirstCall = (fStartTime == _pstFXSlot->stCurveParam.fStartTime) ? orxTRUE : orxFALSE;

  /* Is this slot active in the time period? */
  if(fEndTime >= fStartTime)
  {
    orxFX_TYPE  eFXType;
    orxFLOAT    fRecDuration;

    /* Gets slot local time stamps */
    fStartTime -= _pstFXSlot->stCurveParam.fStartTime;
    fEndTime   -= _pstFXSlot->stCurveParam.fStartTime;

    /* Gets reciprocal duration */
    fRecDuration = _pstFXSlot->stCurveParam.fEndTime - _pstFXSlot->stCurveParam.fStartTime;
    fRecDuration = (fRecDuration > orxFLOAT_0) ? orxFLOAT_1 / fRecDuration : orxFLOAT_1;

    /* Has acceleration? */
    if(orxFLAG_TEST(_pstFXSlot->u32Flags, orxFX_SLOT_KU32_FLAG_ACCELERATION))
    {
      orxFLOAT fStartAcceleration, fEndAcceleration;

      /* Gets acceleration coefs */
      fStartAcceleration  = orxLERP(orxFLOAT_1, _pstFXSlot->stCurveParam.fAcceleration, fStartTime * fRecDuration);
      fEndAcceleration    = orxLERP(orxFLOAT_1, _pstFXSlot->stCurveParam.fAcceleration, fEndTime * fRecDuration);

      /* Updates the times */
      fStartTime *= fStartAcceleration;
      fEndTime   *= fEndAcceleration;

      /* Updates reciprocal duration */
      fRecDuration = (_pstFXSlot->stCurveParam.fEndTime - _pstFXSlot->stCurveParam.fStartTime);
      fRecDuration = (fRecDuration > orxFLOAT_0) ? orxFLOAT_1 / (fRecDuration * _pstFXSlot->stCurveParam.fAcceleration) : orxFLOAT_1;
    }

    /* Gets FX type */
    eFXType = orxFX_GetSlotType(_pstFXSlot);

    /* Is FX type not blocked? */
    if(!orxFLAG_TEST(_pstContext->u32LockFlags, (1 << eFXType)))
    {
      orxFLOAT fPeriod, fFrequency, fStartCoef, fEndCoef;

      /* Has a valid cycle period? */
      if(_pstFXSlot->stCurveParam.fPeriod > orxFLOAT_0)
      {
        /* Gets it */
        fPeriod = _pstFXSlot->stCurveParam.fPeriod;
      }
      else
      {
        /* Gets whole duration as period */
        fPeriod = _pstFXSlot->stCurveParam.fEndTime - _pstFXSlot->stCurveParam.fStartTime;
      }

      /* Instant update? */
      if(fPeriod == orxFLOAT_0)
      {
        /* Gets fake period and frequency */
        fPeriod = fFrequency = orxFLOAT_1;

        /* Updates times */
        fStartTime  = orxFLOAT_0;
        fEndTime    = orxFLOAT_1;
      }
      else
      {
        /* Gets its corresponding frequency */
        fFrequency = orxFLOAT_1 / fPeriod;
      }

      /* Gets coefs */
      fStartCoef  = orxFLAG_TEST(_pstFXSlot->u32Flags, orxFX_SLOT_KU32_FLAG_ABSOLUTE) ? orxFLOAT_0 : orxFX_ComputeCoef(_pstFXSlot, fStartTime, fFrequency);
      fEndCoef    = orxFX_ComputeCoef(_pstFXSlot, fEndTime, fFrequency);

      /* Has amplification? */
      if(orxFLAG_TEST(_pstFXSlot->u32Flags, orxFX_SLOT_KU32_FLAG_AMPLIFICATION))
      {
        orxFLOAT fStartAmplification, fEndAmplification;

        /* Gets amplification coefs */
        fStartAmplification = orxLERP(orxFLOAT_1, _pstFXSlot->stCurveParam.fAmplification, fStartTime * fRecDuration);
        fEndAmplification   = orxLERP(orxFLOAT_1, _pstFXSlot->stCurveParam.fAmplification, fEndTime * fRecDuration);

        /* Updates the coefs */
        fStartCoef *= fStartAmplification;
        fEndCoef   *= fEndAmplification;
      }

      /* Using an exponential curve? */
      if(orxFLAG_TEST(_pstFXSlot->u32Flags, orxFX_SLOT_KU32_FLAG_POW))
      {
        /* Updates both coefs */
        fStartCoef = orxMath_Pow(fStartCoef, _pstFXSlot->stCurveParam.fPow);
        fEndCoef   = orxMath_Pow(fEndCoef, _pstFXSlot->stCurveParam.fPow);
      }

      /* Depending on operation and value type */
      switch(orxFLAG_GET(_pstFXSlot->u32Flags, orxFX_SLOT_KU32_MASK_OPERATION | orxFX_SLOT_KU32_MASK_VALUE_TYPE))
      {
        case (orxFX_OPERATION_ADD << orxFX_SLOT_KU32_SHIFT_OPERATION) | (orxFX_VALUE_TYPE_FLOAT << orxFX_SLOT_KU32_SHIFT_VALUE_TYPE):
        {
          /* Absolute ? */
          if(orxFLAG_TEST(_pstFXSlot->u32Flags, orxFX_SLOT_KU32_FLAG_ABSOLUTE))
          {
            /* Overrides value */
            _pstContext->astValueList[eFXType].fValue = orxLERP(_pstFXSlot->stCurveParam.fStartValue, _pstFXSlot->stCurveParam.fEndValue, fEndCoef);

            /* Locks it */
            orxFLAG_SET(_pstContext->u32LockFlags, (1 << eFXType), 0);
          }
          else
          {
            orxFLOAT fStartValue, fEndValue;

            /* First call? */
            if(bFirstCall != orxFALSE)
            {
              /* Gets start value */
              fStartValue = orxFLOAT_0;
            }
            else
            {
              /* Gets start value */
              fStartValue = orxLERP(_pstFXSlot->stCurveParam.fStartValue, _pstFXSlot->stCurveParam.fEndValue, fStartCoef);
            }

            /* Gets end value */
            fEndValue = orxLERP(_pstFXSlot->stCurveParam.fStartValue, _pstFXSlot->stCurveParam.fEndValue, fEndCoef);

            /* Updates global value */
            _pstContext->astValueList[eFXType].fValue += fEndValue - fStartValue;
          }

          /* Updates status */
          orxFLAG_SET(_pstContext->u32UpdateFlags, (1 << eFXType), 0);

          break;
        }

        case (orxFX_OPERATION_MULTIPLY << orxFX_SLOT_KU32_SHIFT_OPERATION) | (orxFX_VALUE_TYPE_FLOAT << orxFX_SLOT_KU32_SHIFT_VALUE_TYPE):
        {
          /* Absolute ? */
          if(orxFLAG_TEST(_pstFXSlot->u32Flags, orxFX_SLOT_KU32_FLAG_ABSOLUTE))
          {
            /* Overrides value */
            _pstContext->astValueList[eFXType].fValue = orxLERP(_pstFXSlot->stCurveParam.fStartValue, _pstFXSlot->stCurveParam.fEndValue, fEndCoef);
            if(_pstContext->astValueList[eFXType].fValue == orxFLOAT_0)
            {
              _pstContext->astValueList[eFXType].fValue = orx2F(0.000001f);
            }

            /* Locks it */
            orxFLAG_SET(_pstContext->u32LockFlags, (1 << eFXType), 0);
          }
          else
          {
            orxFLOAT fStartValue, fEndValue;

            /* First call? */
            if(bFirstCall != orxFALSE)
            {
              /* Gets start value */
              fStartValue = orxFLOAT_1;
            }
            else
            {
              /* Gets start value */
              fStartValue = orxLERP(_pstFXSlot->stCurveParam.fStartValue, _pstFXSlot->stCurveParam.fEndValue, fStartCoef);
              if(fStartValue == orxFLOAT_0)
              {
                fStartValue = orx2F(0.000001f);
              }
            }

            /* Gets end value */
            fEndValue = orxLERP(_pstFXSlot->stCurveParam.fStartValue, _pstFXSlot->stCurveParam.fEndValue, fEndCoef);
            if(fEndValue == orxFLOAT_0)
            {
              fEndValue = orx2F(0.000001f);
            }

            /* Updates global value */
            _pstContext->astValueList[eFXType].fValue *= fEndValue / fStartValue;
          }

          /* Updates status */
          orxFLAG_SET(_pstContext->u32UpdateFlags, (1 << eFXType), 0);

          break;
        }

        case (orxFX_OPERATION_ADD << orxFX_SLOT_KU32_SHIFT_OPERATION) | (orxFX_VALUE_TYPE_VECTOR << orxFX_SLOT_KU32_SHIFT_VALUE_TYPE):
        {
          /* Valid color blend mix? */
          if(((eFXType != orxFX_TYPE_RGB) && (eFXType != orxFX_TYPE_HSL) && (eFXType != orxFX_TYPE_HSV))
          || ((_pstContext->eColorBlendUpdate == orxFX_TYPE_NONE) || (_pstContext->eColorBlendUpdate == eFXType)))
          {
            /* Absolute ? */
            if(orxFLAG_TEST(_pstFXSlot->u32Flags, orxFX_SLOT_KU32_FLAG_ABSOLUTE))
            {
              /* Overrides values */
              orxVector_Lerp(&(_pstContext->astValueList[eFXType].vValue), &(_pstFXSlot->stCurveParam.vStartValue), &(_pstFXSlot->stCurveParam.vEndValue), fEndCoef);

              /* Use rotation? */
              if(orxFLAG_TEST(_pstFXSlot->u32Flags, orxFX_SLOT_KU32_FLAG_USE_ROTATION))
              {
                /* Updates vector */
                orxVector_2DRotate(&(_pstContext->astValueList[eFXType].vValue), &(_pstContext->astValueList[eFXType].vValue), orxObject_GetRotation(_pstContext->pstObject));
              }

              /* Use scale? */
              if(orxFLAG_TEST(_pstFXSlot->u32Flags, orxFX_SLOT_KU32_FLAG_USE_SCALE))
              {
                orxVECTOR vScale;

                /* Updates vector */
                orxVector_Mul(&(_pstContext->astValueList[eFXType].vValue), &(_pstContext->astValueList[eFXType].vValue), orxObject_GetScale(_pstContext->pstObject, &vScale));
              }

              /* Locks it */
              orxFLAG_SET(_pstContext->u32LockFlags, (1 << eFXType), 0);
            }
            else
            {
              orxVECTOR vStartValue, vEndValue;

              /* First call? */
              if(bFirstCall != orxFALSE)
              {
                /* Gets start value */
                orxVector_SetAll(&vStartValue, orxFLOAT_0);
              }
              else
              {
                /* Gets start value */
                orxVector_Lerp(&vStartValue, &(_pstFXSlot->stCurveParam.vStartValue), &(_pstFXSlot->stCurveParam.vEndValue), fStartCoef);
              }

              /* Gets end value */
              orxVector_Lerp(&vEndValue, &(_pstFXSlot->stCurveParam.vStartValue), &(_pstFXSlot->stCurveParam.vEndValue), fEndCoef);

              /* Gets delta value */
              orxVector_Sub(&vEndValue, &vEndValue, &vStartValue);

              /* Use rotation? */
              if(orxFLAG_TEST(_pstFXSlot->u32Flags, orxFX_SLOT_KU32_FLAG_USE_ROTATION))
              {
                /* Updates vector */
                orxVector_2DRotate(&vEndValue, &vEndValue, orxObject_GetRotation(_pstContext->pstObject));
              }

              /* Use scale? */
              if(orxFLAG_TEST(_pstFXSlot->u32Flags, orxFX_SLOT_KU32_FLAG_USE_SCALE))
              {
                orxVECTOR vScale;

                /* Updates vector */
                orxVector_Mul(&vEndValue, &vEndValue, orxObject_GetScale(_pstContext->pstObject, &vScale));
              }

              /* Updates global value */
              orxVector_Add(&(_pstContext->astValueList[eFXType].vValue), &(_pstContext->astValueList[eFXType].vValue), &vEndValue);
            }

            /* Is a color type? */
            if((eFXType == orxFX_TYPE_RGB) || (eFXType == orxFX_TYPE_HSL) || (eFXType == orxFX_TYPE_HSV))
            {
              /* Updates color blend status */
              _pstContext->eColorBlendUpdate = eFXType;
            }
            else
            {
              /* Updates status */
              orxFLAG_SET(_pstContext->u32UpdateFlags, (1 << eFXType), 0);
            }
          }

          break;
        }

        case (orxFX_OPERATION_MULTIPLY << orxFX_SLOT_KU32_SHIFT_OPERATION) | (orxFX_VALUE_TYPE_VECTOR << orxFX_SLOT_KU32_SHIFT_VALUE_TYPE):
        {
          /* Absolute ? */
          if(orxFLAG_TEST(_pstFXSlot->u32Flags, orxFX_SLOT_KU32_FLAG_ABSOLUTE))
          {
            /* Overrides values */
            orxVector_Lerp(&(_pstContext->astValueList[eFXType].vValue), &(_pstFXSlot->stCurveParam.vStartValue), &(_pstFXSlot->stCurveParam.vEndValue), fEndCoef);

            /* Makes sure we have valid values */
            if(_pstContext->astValueList[eFXType].vValue.fX == orxFLOAT_0)
            {
              _pstContext->astValueList[eFXType].vValue.fX = orx2F(0.000001f);
            }
            if(_pstContext->astValueList[eFXType].vValue.fY == orxFLOAT_0)
            {
              _pstContext->astValueList[eFXType].vValue.fY = orx2F(0.000001f);
            }

            /* Locks it */
            orxFLAG_SET(_pstContext->u32LockFlags, (1 << eFXType), 0);
          }
          else
          {
            orxVECTOR vStartValue, vEndValue;

            /* First call? */
            if(bFirstCall != orxFALSE)
            {
              /* Gets start value */
              orxVector_SetAll(&vStartValue, orxFLOAT_1);
            }
            else
            {
              /* Gets start value */
              orxVector_Lerp(&vStartValue, &(_pstFXSlot->stCurveParam.vStartValue), &(_pstFXSlot->stCurveParam.vEndValue), fStartCoef);

              /* Makes sure we have valid values */
              if(vStartValue.fX == orxFLOAT_0)
              {
                vStartValue.fX = orx2F(0.000001f);
              }
              if(vStartValue.fY == orxFLOAT_0)
              {
                vStartValue.fY = orx2F(0.000001f);
              }
            }

            /* Gets end value */
            orxVector_Lerp(&vEndValue, &(_pstFXSlot->stCurveParam.vStartValue), &(_pstFXSlot->stCurveParam.vEndValue), fEndCoef);

            /* Makes sure we have valid values */
            if(vEndValue.fX == orxFLOAT_0)
            {
              vEndValue.fX = orx2F(0.000001f);
            }
            if(vEndValue.fY == orxFLOAT_0)
            {
              vEndValue.fY = orx2F(0.000001f);
            }

            /* Updates global value */
            orxVector_Mul(&(_pstContext->astValueList[eFXType].vValue), &(_pstContext->astValueList[eFXType].vValue), orxVector_Div(&vEndValue, &vEndValue, &vStartValue));
          }

          /* Updates status */
          orxFLAG_SET(_pstContext->u32UpdateFlags, (1 << eFXType), 0);

          break;
        }

        default:
        {
          /* Logs message */
          orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Invalid FX type when trying to apply FX.");

          break;
        }
      }
    }
  }

  /* Done! */
  return;
}

/** Event handler
 */
static orxSTATUS orxFASTCALL orxFX_EventHandler(const orxEVENT *_pstEvent)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Add or update? */
  if((_pstEvent->eID == orxRESOURCE_EVENT_ADD) || (_pstEvent->eID == orxRESOURCE_EVENT_UPDATE))
  {
    orxRESOURCE_EVENT_PAYLOAD *pstPayload;

    /* Gets payload */
    pstPayload = (orxRESOURCE_EVENT_PAYLOAD *)_pstEvent->pstPayload;

    /* Is config group? */
    if(pstPayload->stGroupID == orxString_Hash(orxCONFIG_KZ_RESOURCE_GROUP))
    {
      orxFX *pstFX;

      /* For all FXs */
      for(pstFX = orxFX(orxStructure_GetFirst(orxSTRUCTURE_ID_FX));
          pstFX != orxNULL;
          pstFX = orxFX(orxStructure_GetNext(pstFX)))
      {
        /* Has reference? */
        if((pstFX->zReference != orxNULL) && (pstFX->zReference != orxSTRING_EMPTY))
        {
          /* Matches? */
          if(orxConfig_GetOriginID(pstFX->zReference) == pstPayload->stNameID)
          {
            orxU32 i;

            /* For all slots */
            for(i = 0; i < orxFX_KU32_SLOT_NUMBER; i++)
            {
              /* Clears it */
              orxFLAG_SET(pstFX->astFXSlotList[i].u32Flags, orxFX_SLOT_KU32_FLAG_NONE, orxFX_SLOT_KU32_FLAG_DEFINED);
            }

            /* Resets duration & offset */
            pstFX->fDuration  = orxFLOAT_0;
            pstFX->fOffset    = orxFLOAT_0;

            /* Clears status */
            orxStructure_SetFlags(pstFX, orxFX_KU32_FLAG_NONE, orxFX_KU32_FLAG_LOOP | orxFX_KU32_FLAG_STAGGERED);

            /* Re-processes its data */
            orxFX_ProcessData(pstFX);
          }
        }
      }
    }
  }

  /* Done! */
  return eResult;
}

/** Deletes all the FXs
 */
static orxINLINE void orxFX_DeleteAll()
{
  orxFX *pstFX;

  /* Gets first FX */
  pstFX = orxFX(orxStructure_GetFirst(orxSTRUCTURE_ID_FX));

  /* Non empty? */
  while(pstFX != orxNULL)
  {
    /* Deletes it */
    orxFX_Delete(pstFX);

    /* Gets first FX */
    pstFX = orxFX(orxStructure_GetFirst(orxSTRUCTURE_ID_FX));
  }

  /* Done! */
  return;
}


/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** FX module setup
 */
void orxFASTCALL orxFX_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_FX, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_FX, orxMODULE_ID_STRING);
  orxModule_AddDependency(orxMODULE_ID_FX, orxMODULE_ID_STRUCTURE);
  orxModule_AddDependency(orxMODULE_ID_FX, orxMODULE_ID_CONFIG);

  /* Done! */
  return;
}

/** Inits the FX module
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxFX_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(orxFX_TYPE_NUMBER <= orxFX_TYPE_MAX_NUMBER);

  /* Not already Initialized? */
  if(!(sstFX.u32Flags & orxFX_KU32_STATIC_FLAG_READY))
  {
    /* Cleans static controller */
    orxMemory_Zero(&sstFX, sizeof(orxFX_STATIC));

    /* Creates ID table */
    sstFX.pstIDTable = orxHashTable_Create(orxFX_KU32_CURVE_TABLE_SIZE, orxHASHTABLE_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

    /* Success? */
    if(sstFX.pstIDTable != orxNULL)
    {
      /* Creates reference table */
      sstFX.pstReferenceTable = orxHashTable_Create(orxFX_KU32_REFERENCE_TABLE_SIZE, orxHASHTABLE_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

      /* Success? */
      if(sstFX.pstReferenceTable != orxNULL)
      {
        /* Registers structure type */
        eResult = orxSTRUCTURE_REGISTER(FX, orxSTRUCTURE_STORAGE_TYPE_LINKLIST, orxMEMORY_TYPE_MAIN, orxFX_KU32_BANK_SIZE, orxNULL);

        /* Success? */
        if(eResult != orxSTATUS_FAILURE)
        {
          /* Inits Flags */
          orxFLAG_SET(sstFX.u32Flags, orxFX_KU32_STATIC_FLAG_READY, orxFX_KU32_STATIC_FLAG_NONE);

          /* Registers all curves */
#define orxFX_REGISTER_CURVE(NAME) orxHashTable_Add(sstFX.pstIDTable, orxString_Hash(orxFX_KZ_##NAME), (void *)(orxFX_CURVE_##NAME + 1))
          orxFX_REGISTER_CURVE(SINE);
          orxFX_REGISTER_CURVE(SQUARE);
          orxFX_REGISTER_CURVE(TRIANGLE);
          orxFX_REGISTER_CURVE(BEZIER);
          orxFX_REGISTER_CURVE(LINEAR);
          orxFX_REGISTER_CURVE(SMOOTH);
          orxFX_REGISTER_CURVE(SMOOTHER);
          orxFX_REGISTER_CURVE(EASE_IN_SINE);
          orxFX_REGISTER_CURVE(EASE_OUT_SINE);
          orxFX_REGISTER_CURVE(EASE_IN_OUT_SINE);
          orxFX_REGISTER_CURVE(EASE_IN_QUAD);
          orxFX_REGISTER_CURVE(EASE_OUT_QUAD);
          orxFX_REGISTER_CURVE(EASE_IN_OUT_QUAD);
          orxFX_REGISTER_CURVE(EASE_IN_CUBIC);
          orxFX_REGISTER_CURVE(EASE_OUT_CUBIC);
          orxFX_REGISTER_CURVE(EASE_IN_OUT_CUBIC);
          orxFX_REGISTER_CURVE(EASE_IN_QUART);
          orxFX_REGISTER_CURVE(EASE_OUT_QUART);
          orxFX_REGISTER_CURVE(EASE_IN_OUT_QUART);
          orxFX_REGISTER_CURVE(EASE_IN_QUINT);
          orxFX_REGISTER_CURVE(EASE_OUT_QUINT);
          orxFX_REGISTER_CURVE(EASE_IN_OUT_QUINT);
          orxFX_REGISTER_CURVE(EASE_IN_EXPO);
          orxFX_REGISTER_CURVE(EASE_OUT_EXPO);
          orxFX_REGISTER_CURVE(EASE_IN_OUT_EXPO);
          orxFX_REGISTER_CURVE(EASE_IN_CIRC);
          orxFX_REGISTER_CURVE(EASE_OUT_CIRC);
          orxFX_REGISTER_CURVE(EASE_IN_OUT_CIRC);
          orxFX_REGISTER_CURVE(EASE_IN_BACK);
          orxFX_REGISTER_CURVE(EASE_OUT_BACK);
          orxFX_REGISTER_CURVE(EASE_IN_OUT_BACK);
          orxFX_REGISTER_CURVE(EASE_IN_ELASTIC);
          orxFX_REGISTER_CURVE(EASE_OUT_ELASTIC);
          orxFX_REGISTER_CURVE(EASE_IN_OUT_ELASTIC);
          orxFX_REGISTER_CURVE(EASE_IN_BOUNCE);
          orxFX_REGISTER_CURVE(EASE_OUT_BOUNCE);
          orxFX_REGISTER_CURVE(EASE_IN_OUT_BOUNCE);
#undef orxFX_REGISTER_CURVE

          /* Registers all object property types */
#define orxFX_REGISTER_TYPE(NAME) orxHashTable_Add(sstFX.pstIDTable, orxString_Hash(orxFX_KZ_##NAME), (void *)(orxFX_TYPE_##NAME + 1))
          orxFX_REGISTER_TYPE(ALPHA);
          orxFX_REGISTER_TYPE(RGB);
          orxFX_REGISTER_TYPE(HSL);
          orxFX_REGISTER_TYPE(HSV);
          orxFX_REGISTER_TYPE(POSITION);
          orxFX_REGISTER_TYPE(ROTATION);
          orxFX_REGISTER_TYPE(SCALE);
          orxFX_REGISTER_TYPE(SPEED);
          orxFX_REGISTER_TYPE(VOLUME);
          orxFX_REGISTER_TYPE(PITCH);
          orxFX_REGISTER_TYPE(SIZE);
          orxFX_REGISTER_TYPE(ORIGIN);
#undef orxFX_REGISTER_TYPE

          /* Adds event handler */
          orxEvent_AddHandler(orxEVENT_TYPE_RESOURCE, orxFX_EventHandler);
          orxEvent_SetHandlerIDFlags(orxFX_EventHandler, orxEVENT_TYPE_RESOURCE, orxNULL, orxEVENT_GET_FLAG(orxRESOURCE_EVENT_ADD) | orxEVENT_GET_FLAG(orxRESOURCE_EVENT_UPDATE), orxEVENT_KU32_MASK_ID_ALL);
        }
      }
      else
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Failed to create FX reference table.");
      }
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Failed to create FX curve table.");
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Tried to initialize the FX module when it was already initialized.");

    /* Already initialized */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Not initialized? */
  if(eResult == orxSTATUS_FAILURE)
  {
    /* Removes event handler */
    orxEvent_RemoveHandler(orxEVENT_TYPE_RESOURCE, orxFX_EventHandler);

    /* Deletes ID table if needed */
    if(sstFX.pstIDTable != orxNULL)
    {
      orxHashTable_Delete(sstFX.pstIDTable);
    }

    /* Deletes reference table if needed */
    if(sstFX.pstReferenceTable != orxNULL)
    {
      orxHashTable_Delete(sstFX.pstReferenceTable);
    }
  }

  /* Done! */
  return eResult;
}

/** Exits from the FX module
 */
void orxFASTCALL orxFX_Exit()
{
  /* Initialized? */
  if(sstFX.u32Flags & orxFX_KU32_STATIC_FLAG_READY)
  {
    /* Removes event handler */
    orxEvent_RemoveHandler(orxEVENT_TYPE_RESOURCE, orxFX_EventHandler);

    /* Deletes FX list */
    orxFX_DeleteAll();

    /* Unregisters structure type */
    orxStructure_Unregister(orxSTRUCTURE_ID_FX);

    /* Deletes reference table */
    orxHashTable_Delete(sstFX.pstReferenceTable);

    /* Deletes ID table */
    orxHashTable_Delete(sstFX.pstIDTable);

    /* Updates flags */
    sstFX.u32Flags &= ~orxFX_KU32_STATIC_FLAG_READY;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Tried to exit from the FX module when it wasn't initialized.");
  }

  /* Done! */
  return;
}

/** Creates an empty FX
 * @return      orxFX / orxNULL
 */
orxFX *orxFASTCALL orxFX_Create()
{
  orxFX *pstResult;

  /* Checks */
  orxASSERT(sstFX.u32Flags & orxFX_KU32_STATIC_FLAG_READY);

  /* Creates FX */
  pstResult = orxFX(orxStructure_Create(orxSTRUCTURE_ID_FX));

  /* Created? */
  if(pstResult != orxNULL)
  {
    /* Inits flags */
    orxStructure_SetFlags(pstResult, orxFX_KU32_FLAG_ENABLED, orxFX_KU32_MASK_ALL);

    /* Increases count */
    orxStructure_IncreaseCount(pstResult);
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Failed to create FX structure.");
  }

  /* Done! */
  return pstResult;
}

/** Creates an FX from config
 * @param[in]   _zConfigID      Config ID
 * @ return orxFX / orxNULL
 */
orxFX *orxFASTCALL orxFX_CreateFromConfig(const orxSTRING _zConfigID)
{
  orxSTRINGID stID;
  orxFX      *pstResult;

  /* Checks */
  orxASSERT(sstFX.u32Flags & orxFX_KU32_STATIC_FLAG_READY);
  orxASSERT((_zConfigID != orxNULL) && (_zConfigID != orxSTRING_EMPTY));

  /* Gets FX ID */
  stID = orxString_Hash(_zConfigID);

  /* Search for reference */
  pstResult = (orxFX *)orxHashTable_Get(sstFX.pstReferenceTable, stID);

  /* Found? */
  if(pstResult != orxNULL)
  {
    /* Increases count */
    orxStructure_IncreaseCount(pstResult);
  }
  else
  {
    /* Pushes section */
    if((orxConfig_HasSection(_zConfigID) != orxFALSE)
    && (orxConfig_PushSection(_zConfigID) != orxSTATUS_FAILURE))
    {
      /* Creates FX */
      pstResult = orxFX_Create();

      /* Valid? */
      if(pstResult != orxNULL)
      {
        /* Stores its reference */
        pstResult->zReference = orxConfig_GetCurrentSection();

        /* Processes its data */
        if(orxFX_ProcessData(pstResult) != orxSTATUS_FAILURE)
        {
          /* Can be cached? */
          if(orxConfig_GetBool(orxFX_KZ_CONFIG_DO_NOT_CACHE) == orxFALSE)
          {
            /* Adds it to reference table */
            if(orxHashTable_Add(sstFX.pstReferenceTable, stID, pstResult) != orxSTATUS_FAILURE)
            {
              /* Should keep it in cache? */
              if(orxConfig_GetBool(orxFX_KZ_CONFIG_KEEP_IN_CACHE) != orxFALSE)
              {
                /* Increases its reference count to keep it in cache table */
                orxStructure_IncreaseCount(pstResult);

                /* Updates its flags */
                orxStructure_SetFlags(pstResult, orxFX_KU32_FLAG_CACHED, orxFX_KU32_FLAG_NONE);
              }
            }
            else
            {
              /* Logs message */
              orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Failed to add FX to hashtable.");

              /* Deletes it */
              orxFX_Delete(pstResult);

              /* Updates result */
              pstResult = orxNULL;
            }
          }
        }
        else
        {
          /* Logs message */
          orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Can't create FX <%s>: invalid content.", _zConfigID);

          /* Deletes it */
          orxFX_Delete(pstResult);

          /* Updates result */
          pstResult = orxNULL;
        }
      }

      /* Pops previous section */
      orxConfig_PopSection();
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Couldn't create FX because config section (%s) couldn't be found.", _zConfigID);

      /* Updates result */
      pstResult = orxNULL;
    }
  }

  /* Done! */
  return pstResult;
}

/** Deletes an FX
 * @param[in]   _pstFX          Concerned FX
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxFX_Delete(orxFX *_pstFX)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstFX.u32Flags & orxFX_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstFX);

  /* Decreases count */
  orxStructure_DecreaseCount(_pstFX);

  /* Not referenced? */
  if(orxStructure_GetRefCount(_pstFX) == 0)
  {
    /* Has an ID? */
    if((_pstFX->zReference != orxNULL)
    && (_pstFX->zReference != orxSTRING_EMPTY))
    {
      /* Removes from hashtable */
      orxHashTable_Remove(sstFX.pstReferenceTable, orxString_Hash(_pstFX->zReference));
    }

    /* Deletes structure */
    orxStructure_Delete(_pstFX);
  }
  else
  {
    /* Referenced by others */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Clears cache (if any FX is still in active use, it'll remain in memory until not referenced anymore)
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxFX_ClearCache()
{
  orxFX    *pstFX, *pstNextFX;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstFX.u32Flags & orxFX_KU32_STATIC_FLAG_READY);

  /* For all FXs */
  for(pstFX = orxFX(orxStructure_GetFirst(orxSTRUCTURE_ID_FX));
      pstFX != orxNULL;
      pstFX = pstNextFX)
  {
    /* Gets next FX */
    pstNextFX = orxFX(orxStructure_GetNext(pstFX));

    /* Is cached? */
    if(orxStructure_TestFlags(pstFX, orxFX_KU32_FLAG_CACHED))
    {
      /* Updates its flags */
      orxStructure_SetFlags(pstFX, orxFX_KU32_FLAG_NONE, orxFX_KU32_FLAG_CACHED);

      /* Deletes its extra reference */
      orxFX_Delete(pstFX);
    }
  }

  /* Done! */
  return eResult;
}

/** Applies FX on object
 * @param[in]   _pstFX          FX to apply
 * @param[in]   _pstObject      Object on which to apply the FX
 * @param[in]   _fPreviousTime  Previous time
 * @param[in]   _fTime          Current time
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxFX_Apply(const orxFX *_pstFX, orxOBJECT *_pstObject, orxFLOAT _fPreviousTime, orxFLOAT _fTime)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxSTRUCTURE_ASSERT(_pstFX);
  orxSTRUCTURE_ASSERT(_pstObject);
  orxASSERT(_fTime >= _fPreviousTime);

  /* Has started? */
  if(_fTime >= orxFLOAT_0)
  {
    orxFX_CONTEXT stContext;
    orxU32        i;

    /* Inits context */
    orxFX_InitContext(&stContext, _pstFX, _pstObject, _fPreviousTime, _fTime);

    /* For all slots */
    for(i = 0; i< orxFX_KU32_SLOT_NUMBER; i++)
    {
      const orxFX_SLOT *pstFXSlot;

      /* Gets it */
      pstFXSlot = &(_pstFX->astFXSlotList[i]);

      /* Is defined? */
      if(orxFLAG_TEST(pstFXSlot->u32Flags, orxFX_SLOT_KU32_FLAG_DEFINED))
      {
        /* Computes it */
        orxFX_ComputeSlot(pstFXSlot, &stContext);
      }
      else
      {
        /* Done with the slots */
        break;
      }
    }

    /* Global color update? */
    if((orxFLAG_TEST(stContext.u32UpdateFlags, (1 << orxFX_TYPE_ALPHA)))
    || (stContext.eColorBlendUpdate != orxFX_TYPE_NONE))
    {
      orxCOLOR stColor, stObjectColor;

      /* Has object color? */
      if(orxObject_HasColor(_pstObject) != orxFALSE)
      {
        /* Stores object color */
        orxObject_GetColor(_pstObject, &stObjectColor);
      }
      else
      {
        /* Clears color */
        orxColor_Set(&stObjectColor, &orxVECTOR_WHITE, orxFLOAT_1);
      }

      /* Update alpha? */
      if(orxFLAG_TEST(stContext.u32UpdateFlags, (1 << orxFX_TYPE_ALPHA)))
      {
        /* Non absolute? */
        if(!orxFLAG_TEST(stContext.u32LockFlags, (1 << orxFX_TYPE_ALPHA)))
        {
          /* Updates alpha with previous one */
          stColor.fAlpha = stContext.astValueList[orxFX_TYPE_ALPHA].fValue + stObjectColor.fAlpha;
        }
        else
        {
          /* Updates color */
          stColor.fAlpha = stContext.astValueList[orxFX_TYPE_ALPHA].fValue;
        }
      }
      else
      {
        /* Resets alpha */
        stColor.fAlpha = stObjectColor.fAlpha;
      }

      /* Update color blend? */
      if(stContext.eColorBlendUpdate != orxFX_TYPE_NONE)
      {
        /* Non absolute? */
        if(!orxFLAG_TEST(stContext.u32LockFlags, (1 << orxFX_TYPE_RGB) | (1 << orxFX_TYPE_HSL) | (1 << orxFX_TYPE_HSV)))
        {
          /* Depending on color space */
          switch(stContext.eColorBlendUpdate)
          {
            /* HSL */
            case orxFX_TYPE_HSL:
            {
              /* Gets object's HSL color */
              orxColor_FromRGBToHSL(&stObjectColor, &stObjectColor);

              /* Updates color with previous one */
              orxVector_Add(&(stColor.vHSL), &(stContext.astValueList[orxFX_TYPE_RGB].vValue), &(stObjectColor.vHSL));

              /* Applies circular clamp on [0, 1[ */
              stColor.vHSL.fH -= orxS2F(orxF2S(stColor.vHSL.fH) - (orxS32)(stColor.vHSL.fH < orxFLOAT_0));

              /* Gets RGB color */
              orxColor_FromHSLToRGB(&stColor, &stColor);
              break;
            }
            /* HSV */
            case orxFX_TYPE_HSV:
            {
              /* Gets object's HSV color */
              orxColor_FromRGBToHSV(&stObjectColor, &stObjectColor);

              /* Updates color with previous one */
              orxVector_Add(&(stColor.vHSV), &(stContext.astValueList[orxFX_TYPE_RGB].vValue), &(stObjectColor.vHSV));

              /* Applies circular clamp on [0, 1[ */
              stColor.vHSV.fH -= orxS2F(orxF2S(stColor.vHSV.fH) - (orxS32)(stColor.vHSV.fH < orxFLOAT_0));

              /* Gets RGB color */
              orxColor_FromHSVToRGB(&stColor, &stColor);
              break;
            }

            /* RGB */
            default:
            {
              /* Updates color with previous one */
              orxVector_Add(&(stColor.vRGB), &(stContext.astValueList[orxFX_TYPE_RGB].vValue), &(stObjectColor.vRGB));
              break;
            }
          }
        }
        else
        {
          /* Copies value */
          orxVector_Copy(&(stColor.vRGB), &(stContext.astValueList[stContext.eColorBlendUpdate].vValue));

          /* HSL? */
          if(stContext.eColorBlendUpdate == orxFX_TYPE_HSL)
          {
            /* Applies circular clamp on [0, 1[ */
            stColor.vHSL.fH -= orxS2F(orxF2S(stColor.vHSL.fH) - (orxS32)(stColor.vHSL.fH < orxFLOAT_0));

            /* Gets RGB color */
            orxColor_FromHSLToRGB(&stColor, &stColor);
          }
          /* HSV? */
          else if(stContext.eColorBlendUpdate == orxFX_TYPE_HSV)
          {
            /* Applies circular clamp on [0, 1[ */
            stColor.vHSV.fH -= orxS2F(orxF2S(stColor.vHSV.fH) - (orxS32)(stColor.vHSV.fH < orxFLOAT_0));

            /* Gets RGB color */
            orxColor_FromHSVToRGB(&stColor, &stColor);
          }
        }
      }
      else
      {
        /* Resets color */
        orxVector_Copy(&(stColor.vRGB), &(stObjectColor.vRGB));
      }

      /* Applies it */
      orxObject_SetColor(_pstObject, &stColor);
    }

    /* Update rotation? */
    if(orxFLAG_TEST(stContext.u32UpdateFlags, (1 << orxFX_TYPE_ROTATION)))
    {
      /* Non absolute? */
      if(!orxFLAG_TEST(stContext.u32LockFlags, (1 << orxFX_TYPE_ROTATION)))
      {
        /* Updates rotation with previous one */
        stContext.astValueList[orxFX_TYPE_ROTATION].fValue += orxObject_GetRotation(_pstObject);
      }

      /* Applies it */
      orxObject_SetRotation(_pstObject, stContext.astValueList[orxFX_TYPE_ROTATION].fValue);
    }

    /* Update scale? */
    if(orxFLAG_TEST(stContext.u32UpdateFlags, (1 << orxFX_TYPE_SCALE)))
    {
      /* Non absolute? */
      if(!orxFLAG_TEST(stContext.u32LockFlags, (1 << orxFX_TYPE_SCALE)))
      {
        orxVECTOR vObjectScale;

        /* Gets object scale */
        orxObject_GetScale(_pstObject, &vObjectScale);

        /* Updates scale with previous one */
        orxVector_Mul(&(stContext.astValueList[orxFX_TYPE_SCALE].vValue), &(stContext.astValueList[orxFX_TYPE_SCALE].vValue), &vObjectScale);
      }

      /* Applies it */
      orxObject_SetScale(_pstObject, &(stContext.astValueList[orxFX_TYPE_SCALE].vValue));
    }

    /* Update position? */
    if(orxFLAG_TEST(stContext.u32UpdateFlags, (1 << orxFX_TYPE_POSITION)))
    {
      /* Non absolute? */
      if(!orxFLAG_TEST(stContext.u32LockFlags, (1 << orxFX_TYPE_POSITION)))
      {
        orxVECTOR vObjectPosition;

        /* Updates position with previous one */
        orxVector_Add(&(stContext.astValueList[orxFX_TYPE_POSITION].vValue), &(stContext.astValueList[orxFX_TYPE_POSITION].vValue), orxObject_GetPosition(_pstObject, &vObjectPosition));
      }

      /* Applies it */
      orxObject_SetPosition(_pstObject, &(stContext.astValueList[orxFX_TYPE_POSITION].vValue));
    }

    /* Update speed? */
    if(orxFLAG_TEST(stContext.u32UpdateFlags, (1 << orxFX_TYPE_SPEED)))
    {
      /* Non absolute? */
      if(!orxFLAG_TEST(stContext.u32LockFlags, (1 << orxFX_TYPE_SPEED)))
      {
        orxVECTOR vObjectSpeed;

        /* Updates position with previous one */
        orxVector_Add(&(stContext.astValueList[orxFX_TYPE_SPEED].vValue), &(stContext.astValueList[orxFX_TYPE_SPEED].vValue), orxObject_GetSpeed(_pstObject, &vObjectSpeed));
      }

      /* Applies it */
      orxObject_SetSpeed(_pstObject, &(stContext.astValueList[orxFX_TYPE_SPEED].vValue));
    }

    /* Update volume? */
    if(orxFLAG_TEST(stContext.u32UpdateFlags, (1 << orxFX_TYPE_VOLUME)))
    {
      /* Non absolute? */
      if(!orxFLAG_TEST(stContext.u32LockFlags, (1 << orxFX_TYPE_VOLUME)))
      {
        orxSOUND *pstSound;

        /* Gets sounds */
        pstSound = orxObject_GetLastAddedSound(_pstObject);

        /* Valid ? */
        if(pstSound != orxNULL)
        {
          /* Updates volume with previous one */
          stContext.astValueList[orxFX_TYPE_VOLUME].fValue += orxSound_GetVolume(pstSound);
        }
      }

      /* Applies it */
      orxObject_SetVolume(_pstObject, stContext.astValueList[orxFX_TYPE_VOLUME].fValue);
    }

    /* Update pitch? */
    if(orxFLAG_TEST(stContext.u32UpdateFlags, (1 << orxFX_TYPE_PITCH)))
    {
      /* Non absolute? */
      if(!orxFLAG_TEST(stContext.u32LockFlags, (1 << orxFX_TYPE_PITCH)))
      {
        orxSOUND *pstSound;

        /* Gets sounds */
        pstSound = orxObject_GetLastAddedSound(_pstObject);

        /* Valid? */
        if(pstSound != orxNULL)
        {
          /* Updates pitch with previous one */
          stContext.astValueList[orxFX_TYPE_PITCH].fValue *= orxSound_GetPitch(pstSound);
        }
      }

      /* Applies it */
      orxObject_SetPitch(_pstObject, stContext.astValueList[orxFX_TYPE_PITCH].fValue);
    }

    /* Update size? */
    if(orxFLAG_TEST(stContext.u32UpdateFlags, (1 << orxFX_TYPE_SIZE)))
    {
      /* Non absolute? */
      if(!orxFLAG_TEST(stContext.u32LockFlags, (1 << orxFX_TYPE_SIZE)))
      {
        orxVECTOR vObjectSize;

        /* Updates size with previous one */
        orxVector_Add(&(stContext.astValueList[orxFX_TYPE_SIZE].vValue), &(stContext.astValueList[orxFX_TYPE_SIZE].vValue), orxObject_GetSize(_pstObject, &vObjectSize));
      }

      /* Applies it */
      orxObject_SetSize(_pstObject, &(stContext.astValueList[orxFX_TYPE_SIZE].vValue));
    }

    /* Update origin? */
    if(orxFLAG_TEST(stContext.u32UpdateFlags, (1 << orxFX_TYPE_ORIGIN)))
    {
      /* Non absolute? */
      if(!orxFLAG_TEST(stContext.u32LockFlags, (1 << orxFX_TYPE_ORIGIN)))
      {
        orxVECTOR vObjectOrigin;

        /* Updates origin with previous one */
        orxVector_Add(&(stContext.astValueList[orxFX_TYPE_ORIGIN].vValue), &(stContext.astValueList[orxFX_TYPE_ORIGIN].vValue), orxObject_GetOrigin(_pstObject, &vObjectOrigin));
      }

      /* Applies it */
      orxObject_SetOrigin(_pstObject, &(stContext.astValueList[orxFX_TYPE_ORIGIN].vValue));
    }

    /* Updates result */
    eResult = (_fTime >= _pstFX->fDuration) ? orxSTATUS_FAILURE : orxSTATUS_SUCCESS;
  }

  /* Done! */
  return eResult;
}

/** Gets FX user float value between two timestamps
 * @param[in]   _pstFX          Concerned FX
 * @param[in]   _fPreviousTime  Previous time, ignored for Absolute slots. If <= orxFLOAT_0, the value at the current time will be returned, otherwise the value delta between both times will be returned
 * @param[in]   _fTime          Current time
 * @return      Float value
 */
orxFLOAT orxFASTCALL orxFX_GetFloat(const orxFX *_pstFX, orxFLOAT _fPreviousTime, orxFLOAT _fTime)
{
  orxFLOAT fResult = orxFLOAT_0;

  /* Checks */
  orxSTRUCTURE_ASSERT(_pstFX);
  orxASSERT(_fTime >= _fPreviousTime);

  /* Is time valid? */
  if(_fTime >= orxFLOAT_0)
  {
    orxFX_CONTEXT stContext;
    orxFX_TYPE    eType = orxFX_TYPE_NONE;
    orxU32        i;

    /* Inits context */
    orxFX_InitContext(&stContext, _pstFX, orxNULL, _fPreviousTime, _fTime);

    /* For all slots */
    for(i = 0; i< orxFX_KU32_SLOT_NUMBER; i++)
    {
      const orxFX_SLOT *pstFXSlot;

      /* Gets it */
      pstFXSlot = &(_pstFX->astFXSlotList[i]);

      /* Is defined? */
      if(orxFLAG_TEST(pstFXSlot->u32Flags, orxFX_SLOT_KU32_FLAG_DEFINED))
      {
        orxFX_TYPE eSlotType;

        /* Gets its type */
        eSlotType = orxFX_GetSlotType(pstFXSlot);

        /* Should select a type? */
        if((eType == orxFX_TYPE_NONE)
        && ((eSlotType == orxFX_TYPE_FLOAT_ADD) || (eSlotType == orxFX_TYPE_FLOAT_MULTIPLY)))
        {
          /* Updates type */
          eType = eSlotType;
        }

        /* Should compute slot? */
        if(eType == eSlotType)
        {
          /* Computes it */
          orxFX_ComputeSlot(pstFXSlot, &stContext);
        }
#ifdef __orxDEBUG__
        else if(((eType == orxFX_TYPE_VECTOR_MULTIPLY) && (eSlotType == orxFX_TYPE_VECTOR_ADD))
             || ((eType == orxFX_TYPE_VECTOR_ADD) && (eSlotType == orxFX_TYPE_VECTOR_MULTIPLY))
             || ((eType == orxFX_TYPE_FLOAT_MULTIPLY) && (eSlotType == orxFX_TYPE_FLOAT_ADD))
             || ((eType == orxFX_TYPE_FLOAT_ADD) && (eSlotType == orxFX_TYPE_FLOAT_MULTIPLY)))
        {
          /* Logs message */
          orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "[%s]: ignoring slot <%u>: can't mix user values of types <add> & <multiply> in the same FX.", orxFX_GetName(_pstFX), i);
        }
#endif /* __orxDEBUG__ */
      }
      else
      {
        /* Done with the slots */
        break;
      }
    }

    /* Success? */
    if(eType != orxFX_TYPE_NONE)
    {
      /* Updates result */
      fResult = stContext.astValueList[eType].fValue;
    }
  }

  /* Done! */
  return fResult;
}

/** Gets FX user vector value between two timestamps
 * @param[in]   _pstFX          Concerned FX
 * @param[in]   _fPreviousTime  Previous time, ignored for Absolute slots. If <= orxFLOAT_0, the value at the current time will be returned, otherwise the value delta between both times will be returned
 * @param[in]   _fTime          Current time
 * @param[out]  _pvVector       Storage for vector value
 * @return      Vector value if valid, orxNULL otherwise
 */
orxVECTOR *orxFASTCALL orxFX_GetVector(const orxFX *_pstFX, orxFLOAT _fPreviousTime, orxFLOAT _fTime, orxVECTOR *_pvVector)
{
  orxVECTOR *pvResult = orxNULL;

  /* Checks */
  orxSTRUCTURE_ASSERT(_pstFX);
  orxASSERT(_fTime >= _fPreviousTime);

  /* Is time valid? */
  if(_fTime >= orxFLOAT_0)
  {
    orxFX_CONTEXT stContext;
    orxFX_TYPE    eType = orxFX_TYPE_NONE;
    orxU32        i;

    /* Inits context */
    orxFX_InitContext(&stContext, _pstFX, orxNULL, _fPreviousTime, _fTime);

    /* For all slots */
    for(i = 0; i< orxFX_KU32_SLOT_NUMBER; i++)
    {
      const orxFX_SLOT *pstFXSlot;

      /* Gets it */
      pstFXSlot = &(_pstFX->astFXSlotList[i]);

      /* Is defined? */
      if(orxFLAG_TEST(pstFXSlot->u32Flags, orxFX_SLOT_KU32_FLAG_DEFINED))
      {
        orxFX_TYPE eSlotType;

        /* Gets its type */
        eSlotType = orxFX_GetSlotType(pstFXSlot);

        /* Should select a type? */
        if((eType == orxFX_TYPE_NONE)
        && ((eSlotType == orxFX_TYPE_VECTOR_ADD) || (eSlotType == orxFX_TYPE_VECTOR_MULTIPLY)))
        {
          /* Updates type */
          eType = eSlotType;
        }

        /* Should compute slot? */
        if(eType == eSlotType)
        {
          /* Computes it */
          orxFX_ComputeSlot(pstFXSlot, &stContext);
        }
#ifdef __orxDEBUG__
        else if(((eType == orxFX_TYPE_VECTOR_MULTIPLY) && (eSlotType == orxFX_TYPE_VECTOR_ADD))
             || ((eType == orxFX_TYPE_VECTOR_ADD) && (eSlotType == orxFX_TYPE_VECTOR_MULTIPLY))
             || ((eType == orxFX_TYPE_FLOAT_MULTIPLY) && (eSlotType == orxFX_TYPE_FLOAT_ADD))
             || ((eType == orxFX_TYPE_FLOAT_ADD) && (eSlotType == orxFX_TYPE_FLOAT_MULTIPLY)))
        {
          /* Logs message */
          orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "[%s]: ignoring slot <%u>: can't mix user values of types <add> & <multiply> in the same FX.", orxFX_GetName(_pstFX), i);
        }
#endif /* __orxDEBUG__ */
      }
      else
      {
        /* Done with the slots */
        break;
      }
    }

    /* Success? */
    if(eType != orxFX_TYPE_NONE)
    {
      /* Updates result */
      pvResult = _pvVector;
      orxVector_Copy(pvResult, &(stContext.astValueList[eType].vValue));
    }
  }

  /* Done! */
  return pvResult;
}

/** Enables/disables an FX
 * @param[in]   _pstFX        Concerned FX
 * @param[in]   _bEnable      enable / disable
 */
void orxFASTCALL orxFX_Enable(orxFX *_pstFX, orxBOOL _bEnable)
{
  /* Checks */
  orxASSERT(sstFX.u32Flags & orxFX_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstFX);

  /* Enable? */
  if(_bEnable != orxFALSE)
  {
    /* Updates status flags */
    orxStructure_SetFlags(_pstFX, orxFX_KU32_FLAG_ENABLED, orxFX_KU32_FLAG_NONE);
  }
  else
  {
    /* Updates status flags */
    orxStructure_SetFlags(_pstFX, orxFX_KU32_FLAG_NONE, orxFX_KU32_FLAG_ENABLED);
  }

  /* Done! */
  return;
}

/** Is FX enabled?
 * @param[in]   _pstFX        Concerned FX
 * @return      orxTRUE if enabled, orxFALSE otherwise
 */
orxBOOL orxFASTCALL orxFX_IsEnabled(const orxFX *_pstFX)
{
  /* Checks */
  orxASSERT(sstFX.u32Flags & orxFX_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstFX);

  /* Done! */
  return(orxStructure_TestFlags(_pstFX, orxFX_KU32_FLAG_ENABLED));
}

/** Adds a slot to an FX
 * @param[in]   _pstFX          Concerned FX
 * @param[in]   _eType          Type
 * @param[in]   _eCurve         Curve
 * @param[in]   _pstCurveParam  Curve parameters
 * @param[in]   _u32Flags       Additional flags (Absolute, Stagger, UseRotation/UseScale for position/speed types only)
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxFX_AddSlot(orxFX *_pstFX, orxFX_TYPE _eType, orxFX_CURVE _eCurve, const orxFX_CURVE_PARAM *_pstCurveParam, orxU32 _u32Flags)
{
  orxU32    u32Index;
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(sstFX.u32Flags & orxFX_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstFX);
  orxASSERT(_eType < orxFX_TYPE_NUMBER);
  orxASSERT(_eCurve < orxFX_CURVE_NUMBER);
  orxASSERT(_pstCurveParam != orxNULL);
  orxASSERT(_pstCurveParam->fStartTime >= orxFLOAT_0);
  orxASSERT(_pstCurveParam->fEndTime >= _pstCurveParam->fStartTime);
  orxASSERT(orxFLAG_GET(_u32Flags, orxFX_SLOT_KU32_MASK_USER_ALL) == _u32Flags);

  /* Finds empty slot index */
  u32Index = orxFX_FindEmptySlotIndex(_pstFX);

  /* Valid? */
  if(u32Index != orxU32_UNDEFINED)
  {
    orxFX_SLOT       *pstFXSlot;
    orxFX_OPERATION   eOperation = orxFX_OPERATION_ADD;
    orxFX_VALUE_TYPE  eValueType = orxFX_VALUE_TYPE_FLOAT;

    /* Clears unused flags */
    if((_eType != orxFX_TYPE_POSITION)
    && (_eType != orxFX_TYPE_SPEED))
    {
      orxFLAG_SET(_u32Flags, orxFX_SLOT_KU32_FLAG_NONE, orxFX_SLOT_KU32_FLAG_USE_SCALE | orxFX_SLOT_KU32_FLAG_USE_ROTATION);
    }

    /* Depending on the type */
    switch(_eType)
    {
      case orxFX_TYPE_SCALE:
      case orxFX_TYPE_VECTOR_MULTIPLY:
      {
        /* Updates operation */
        eOperation = orxFX_OPERATION_MULTIPLY;

        /* Fall through */
      }

      case orxFX_TYPE_RGB:
      case orxFX_TYPE_HSL:
      case orxFX_TYPE_HSV:
      case orxFX_TYPE_POSITION:
      case orxFX_TYPE_SPEED:
      case orxFX_TYPE_SIZE:
      case orxFX_TYPE_ORIGIN:
      case orxFX_TYPE_VECTOR_ADD:
      {
        /* Updates value type */
        eValueType = orxFX_VALUE_TYPE_VECTOR;

        break;
      }

      case orxFX_TYPE_PITCH:
      case orxFX_TYPE_FLOAT_MULTIPLY:
      {
        /* Updates operation */
        eOperation = orxFX_OPERATION_MULTIPLY;

        break;
      }

      default:
      {
        break;
      }
    }

    /* Gets the slot */
    pstFXSlot = &(_pstFX->astFXSlotList[u32Index]);

    /* Updates it */
    orxMemory_Copy(&(pstFXSlot->stCurveParam), _pstCurveParam, sizeof(orxFX_CURVE_PARAM));
    pstFXSlot->u32Flags   = orxFLAG_GET(_u32Flags, orxFX_SLOT_KU32_MASK_USER_ALL);
    pstFXSlot->u32Flags  |= _eCurve;
    pstFXSlot->u32Flags  |= (_eType << orxFX_SLOT_KU32_SHIFT_TYPE);
    pstFXSlot->u32Flags  |= (eOperation << orxFX_SLOT_KU32_SHIFT_OPERATION);
    pstFXSlot->u32Flags  |= (eValueType << orxFX_SLOT_KU32_SHIFT_VALUE_TYPE);
    pstFXSlot->u32Flags  |= orxFX_SLOT_KU32_FLAG_DEFINED;
    if(_pstCurveParam->fAmplification != orxFLOAT_1)
    {
      pstFXSlot->u32Flags |= orxFX_SLOT_KU32_FLAG_AMPLIFICATION;
    }
    if(_pstCurveParam->fAcceleration != orxFLOAT_1)
    {
      pstFXSlot->u32Flags |= orxFX_SLOT_KU32_FLAG_ACCELERATION;
    }
    if(_pstCurveParam->fPow != orxFLOAT_1)
    {
      pstFXSlot->u32Flags |= orxFX_SLOT_KU32_FLAG_POW;
    }

    /* Should stagger? */
    if(orxFLAG_TEST(_u32Flags, orxFX_SLOT_KU32_FLAG_STAGGER))
    {
      /* Updates times */
      pstFXSlot->stCurveParam.fStartTime += _pstFX->fDuration;
      pstFXSlot->stCurveParam.fEndTime   += _pstFX->fDuration;
    }

    /* Is longer than current FX duration? */
    if(pstFXSlot->stCurveParam.fEndTime > _pstFX->fDuration)
    {
      /* Updates it */
      _pstFX->fDuration = pstFXSlot->stCurveParam.fEndTime;
    }

    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Done! */
  return eResult;
}

/** Adds a slot to an FX from config
 * @param[in]   _pstFX          Concerned FX
 * @param[in]   _zSlotID        Config ID
 * return       orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxFX_AddSlotFromConfig(orxFX *_pstFX, const orxSTRING _zSlotID)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxSTRUCTURE_ASSERT(_pstFX);
  orxASSERT((_zSlotID != orxNULL) && (_zSlotID != orxSTRING_EMPTY));

  /* Pushes section */
  if((orxConfig_HasSection(_zSlotID) != orxFALSE)
  && (orxConfig_PushSection(_zSlotID) != orxSTATUS_FAILURE))
  {
    orxFX_CURVE_PARAM stCurveParam;
    void             *pID;
    orxCOLORSPACE     eColorSpace = orxCOLORSPACE_NONE;
    orxFX_CURVE       eCurve      = orxFX_CURVE_LINEAR;
    orxFX_TYPE        eType       = orxFX_TYPE_NONE;
    orxFX_OPERATION   eOperation  = orxFX_OPERATION_ADD;
    orxU32            u32Flags    = orxFX_SLOT_KU32_FLAG_NONE;
    orxCHAR           acBuffer[32];

    /* Clears curve param */
    orxMemory_Zero(&stCurveParam, sizeof(orxFX_CURVE_PARAM));

    /* Gets curve's lowercase name */
    orxString_NPrint(acBuffer, sizeof(acBuffer), "%s", orxConfig_GetListString(orxFX_KZ_CONFIG_CURVE, 0));
    orxString_LowerCase(acBuffer);

    /* Retrieves curve's registration */
    pID = orxHashTable_Get(sstFX.pstIDTable, orxString_Hash(acBuffer));

    /* Valid? */
    if(pID != 0)
    {
      /* Gets curve */
      eCurve = (orxFX_CURVE)((orxU32)(orxUPTR)pID - 1);

      /* Bezier? */
      if(eCurve == orxFX_CURVE_BEZIER)
      {
        /* Can't get its control points */
        if((orxConfig_GetListVector(orxFX_KZ_CONFIG_CURVE, 1, &(stCurveParam.vCurvePoint1)) == orxNULL)
        || (orxConfig_GetListVector(orxFX_KZ_CONFIG_CURVE, 2, &(stCurveParam.vCurvePoint2)) == orxNULL))
        {
          /* Logs message */
          orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Couldn't add slot [%s]: invalid control point(s) for Bezier curve, defaulting to linear curve.", _zSlotID);

          /* Updates curve */
          eCurve = orxFX_CURVE_LINEAR;
        }
      }
    }

    /* Gets curve parameters */
    stCurveParam.fStartTime     = orxConfig_GetFloat(orxFX_KZ_CONFIG_START_TIME);
    stCurveParam.fEndTime       = orxConfig_GetFloat(orxFX_KZ_CONFIG_END_TIME);
    stCurveParam.fPeriod        = orxConfig_GetFloat(orxFX_KZ_CONFIG_PERIOD);
    stCurveParam.fPhase         = orxConfig_GetFloat(orxFX_KZ_CONFIG_PHASE);
    stCurveParam.fAmplification = orxConfig_HasValue(orxFX_KZ_CONFIG_AMPLIFICATION) ? orxConfig_GetFloat(orxFX_KZ_CONFIG_AMPLIFICATION) : orxFLOAT_1;
    stCurveParam.fAcceleration  = orxConfig_HasValue(orxFX_KZ_CONFIG_ACCELERATION) ? orxConfig_GetFloat(orxFX_KZ_CONFIG_ACCELERATION) : orxFLOAT_1;
    stCurveParam.fPow           = orxConfig_HasValue(orxFX_KZ_CONFIG_POW) ? orxConfig_GetFloat(orxFX_KZ_CONFIG_POW) : orxFLOAT_1;

    /* Is absolute? */
    if(orxConfig_GetBool(orxFX_KZ_CONFIG_ABSOLUTE) != orxFALSE)
    {
      /* Updates flags */
      u32Flags |= orxFX_SLOT_KU32_FLAG_ABSOLUTE;
    }

    /* Should stagger? */
    if(orxConfig_GetBool(orxFX_KZ_CONFIG_STAGGER) != orxFALSE)
    {
      /* Updates flags */
      u32Flags |= orxFX_SLOT_KU32_FLAG_STAGGER;
    }

    /* Gets type's lowercase name */
    orxString_NPrint(acBuffer, sizeof(acBuffer), "%s", orxConfig_GetString(orxFX_KZ_CONFIG_TYPE));
    orxString_LowerCase(acBuffer);

    /* Retrieves type's registration */
    pID = orxHashTable_Get(sstFX.pstIDTable, orxString_Hash(acBuffer));

    /* Valid? */
    if(pID != 0)
    {
      /* Gets type */
      eType = (orxFX_TYPE)((orxU32)(orxUPTR)pID - 1);
    }

    /* Depending on type */
    switch(eType)
    {
      case orxFX_TYPE_RGB:
      {
        /* Updates color space */
        eColorSpace = orxCOLORSPACE_RGB;
        break;
      }
      case orxFX_TYPE_HSL:
      {
        /* Updates color space */
        eColorSpace = orxCOLORSPACE_HSL;
        break;
      }
      case orxFX_TYPE_HSV:
      {
        /* Updates color space */
        eColorSpace = orxCOLORSPACE_HSV;
        break;
      }
      case orxFX_TYPE_POSITION:
      case orxFX_TYPE_SPEED:
      {
        /* Use rotation? */
        if(orxConfig_GetBool(orxFX_KZ_CONFIG_USE_ROTATION) != orxFALSE)
        {
          /* Updates local flags */
          u32Flags |= orxFX_SLOT_KU32_FLAG_USE_ROTATION;
        }

        /* Use scale? */
        if(orxConfig_GetBool(orxFX_KZ_CONFIG_USE_SCALE) != orxFALSE)
        {
          /* Updates local flags */
          u32Flags |= orxFX_SLOT_KU32_FLAG_USE_SCALE;
        }
        break;
      }
      case orxFX_TYPE_NONE:
      {
        /* Color? */
        if(orxString_Compare(acBuffer, orxFX_KZ_COLOR) == 0)
        {
          /* Updates color space */
          eColorSpace = orxCOLORSPACE_COMPONENT;

          /* Updates type */
          eType = orxFX_TYPE_RGB;
        }
        /* Vector? */
        else if(orxString_SearchString(acBuffer, orxFX_KZ_VECTOR) != orxNULL)
        {
          /* Updates type */
          eType = (orxString_SearchString(acBuffer, orxFX_KZ_MULTIPLY) != orxNULL) ? orxFX_TYPE_VECTOR_MULTIPLY : orxFX_TYPE_VECTOR_ADD;
        }
        /* Float? */
        else if(orxString_SearchString(acBuffer, orxFX_KZ_FLOAT) != orxNULL)
        {
          /* Updates type */
          eType = (orxString_SearchString(acBuffer, orxFX_KZ_MULTIPLY) != orxNULL) ? orxFX_TYPE_FLOAT_MULTIPLY : orxFX_TYPE_FLOAT_ADD;
        }
        break;
      }
      default:
      {
        break;
      }
    }

    /* Valid? */
    if(eType != orxFX_TYPE_NONE)
    {
      /* Depending on type */
      switch(eType)
      {
        case orxFX_TYPE_PITCH:
        case orxFX_TYPE_FLOAT_MULTIPLY:
        {
          /* Updates operation */
          eOperation = orxFX_OPERATION_MULTIPLY;

          /* Sets default values */
          stCurveParam.fStartValue = stCurveParam.fEndValue = orxFLOAT_1;

          /* Fall through */
        }

        case orxFX_TYPE_ALPHA:
        case orxFX_TYPE_ROTATION:
        case orxFX_TYPE_VOLUME:
        case orxFX_TYPE_FLOAT_ADD:
        {
          /* Gets float values */
          if(orxConfig_HasValue(orxFX_KZ_CONFIG_START_VALUE) != orxFALSE)
          {
            stCurveParam.fStartValue = orxConfig_GetFloat(orxFX_KZ_CONFIG_START_VALUE);
          }
          if(orxConfig_HasValue(orxFX_KZ_CONFIG_END_VALUE) != orxFALSE)
          {
            stCurveParam.fEndValue = orxConfig_GetFloat(orxFX_KZ_CONFIG_END_VALUE);
          }

          /* Rotation? */
          if(eType == orxFX_TYPE_ROTATION)
          {
            /* Converts values to radians */
            stCurveParam.fStartValue *= orxMATH_KF_DEG_TO_RAD;
            stCurveParam.fEndValue   *= orxMATH_KF_DEG_TO_RAD;
          }

          break;
        }

        case orxFX_TYPE_SCALE:
        case orxFX_TYPE_VECTOR_MULTIPLY:
        {
          /* Updates operation */
          eOperation = orxFX_OPERATION_MULTIPLY;

          /* Sets default values */
          orxVector_Copy(&(stCurveParam.vStartValue), &orxVECTOR_1);
          orxVector_Copy(&(stCurveParam.vEndValue), &orxVECTOR_1);

          /* Fall through */
        }

        case orxFX_TYPE_POSITION:
        case orxFX_TYPE_SPEED:
        case orxFX_TYPE_SIZE:
        case orxFX_TYPE_ORIGIN:
        case orxFX_TYPE_VECTOR_ADD:
        {
          /* Has start value? */
          if(orxConfig_HasValue(orxFX_KZ_CONFIG_START_VALUE) != orxFALSE)
          {
            /* Not a vector? */
            if(orxConfig_GetVector(orxFX_KZ_CONFIG_START_VALUE, &(stCurveParam.vStartValue)) == orxNULL)
            {
              orxFLOAT fValue;

              /* Gets float value */
              fValue = orxConfig_GetFloat(orxFX_KZ_CONFIG_START_VALUE);

              /* Updates value */
              orxVector_SetAll(&(stCurveParam.vStartValue), fValue);
            }
          }
          /* Has end value? */
          if(orxConfig_HasValue(orxFX_KZ_CONFIG_END_VALUE) != orxFALSE)
          {
            /* Not a vector? */
            if(orxConfig_GetVector(orxFX_KZ_CONFIG_END_VALUE, &(stCurveParam.vEndValue)) == orxNULL)
            {
              orxFLOAT fValue;

              /* Gets float value */
              fValue = orxConfig_GetFloat(orxFX_KZ_CONFIG_END_VALUE);

              /* Updates value */
              orxVector_SetAll(&(stCurveParam.vEndValue), fValue);
            }
          }

          break;
        }

        case orxFX_TYPE_RGB:
        case orxFX_TYPE_HSL:
        case orxFX_TYPE_HSV:
        {
          /* Gets color values */
          orxConfig_GetColorVector(orxFX_KZ_CONFIG_START_VALUE, eColorSpace, &(stCurveParam.vStartValue));
          orxConfig_GetColorVector(orxFX_KZ_CONFIG_END_VALUE, eColorSpace, &(stCurveParam.vEndValue));

          /* Should normalize? */
          if(eColorSpace == orxCOLORSPACE_COMPONENT)
          {
            /* Normalizes values */
            orxVector_Mulf(&(stCurveParam.vStartValue), &(stCurveParam.vStartValue), orxCOLOR_NORMALIZER);
            orxVector_Mulf(&(stCurveParam.vEndValue), &(stCurveParam.vEndValue), orxCOLOR_NORMALIZER);
          }

          break;
        }

        default:
        {
          /* Logs message */
          orxASSERT(orxFALSE && "Invalid FX slot type <%u>.", eType);

          break;
        }
      }

      /* Adds slot */
      eResult = orxFX_AddSlot(_pstFX, eType, eCurve, &stCurveParam, u32Flags);
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Couldn't add slot [%s]: type <%s> is invalid.", _zSlotID, acBuffer);
    }

    /* Pops previous section */
    orxConfig_PopSection();
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Couldn't add slot: config section [%s] is missing.", _zSlotID);
  }

  /* Done! */
  return eResult;
}

/** Gets FX duration
 * @param[in]   _pstFX          Concerned FX
 * @return      orxFLOAT
 */
orxFLOAT orxFASTCALL orxFX_GetDuration(const orxFX *_pstFX)
{
  orxFLOAT fResult;

  /* Checks */
  orxASSERT(sstFX.u32Flags & orxFX_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstFX);

  /* Updates result */
  fResult = _pstFX->fDuration;

  /* Done! */
  return fResult;
}

/** Gets FX name
 * @param[in]   _pstFX          Concerned FX
 * @return      orxSTRING / orxSTRING_EMPTY
 */
const orxSTRING orxFASTCALL orxFX_GetName(const orxFX *_pstFX)
{
  const orxSTRING zResult;

  /* Checks */
  orxASSERT(sstFX.u32Flags & orxFX_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstFX);

  /* Has reference? */
  if(_pstFX->zReference != orxNULL)
  {
    /* Updates result */
    zResult = _pstFX->zReference;
  }
  else
  {
    /* Updates result */
    zResult = orxSTRING_EMPTY;
  }

  /* Done! */
  return zResult;
}

/** Sets FX loop property
 * @param[in]   _pstFX          Concerned FX
 * @param[in]   _bLoop          Loop / don't loop
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxFX_Loop(orxFX *_pstFX, orxBOOL _bLoop)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstFX.u32Flags & orxFX_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstFX);

  /* Should loop? */
  if(_bLoop != orxFALSE)
  {
    /* Updates status */
    orxStructure_SetFlags(_pstFX, orxFX_KU32_FLAG_LOOP, orxFX_KU32_FLAG_NONE);
  }
  else
  {
    /* Updates status */
    orxStructure_SetFlags(_pstFX, orxFX_KU32_FLAG_NONE, orxFX_KU32_FLAG_LOOP);
  }

  /* Done! */
  return eResult;
}

/** Is FX looping
 * @param[in]   _pstFX          Concerned FX
 * @return      orxTRUE if looping, orxFALSE otherwise
 */
orxBOOL orxFASTCALL orxFX_IsLooping(const orxFX *_pstFX)
{
  orxBOOL bResult;

  /* Checks */
  orxASSERT(sstFX.u32Flags & orxFX_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstFX);

  /* Updates result */
  bResult = (orxStructure_TestFlags(_pstFX, orxFX_KU32_FLAG_LOOP)) ? orxTRUE : orxFALSE;

  /* Done! */
  return bResult;
}

/** Sets FX stagger / offset
 * @param[in]   _pstFX          Concerned FX
 * @param[in]   _bStagger       If true, this FX will be added after all current FXs
 * @param[in]   _fOffset        Initial offset, in seconds. Cannot result in a negative start time
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxFX_SetStagger(orxFX *_pstFX, orxBOOL _bStagger, orxFLOAT _fOffset)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstFX.u32Flags & orxFX_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstFX);
  orxASSERT((_fOffset >= orxFLOAT_0) || (_bStagger != orxFALSE));

  /* Should stagger? */
  if(_bStagger != orxFALSE)
  {
    /* Updates status */
    orxStructure_SetFlags(_pstFX, orxFX_KU32_FLAG_STAGGERED, orxFX_KU32_FLAG_NONE);
  }
  else
  {
    /* Updates status */
    orxStructure_SetFlags(_pstFX, orxFX_KU32_FLAG_NONE, orxFX_KU32_FLAG_STAGGERED);
  }

  /* Stores offset */
  _pstFX->fOffset = _fOffset;

  /* Done! */
  return eResult;
}

/** Get FX stagger / offset
 * @param[in]   _pstFX          Concerned FX
 * @param[out]  _pfOffset       If non null, will contain the initial offset
 * @return      orxTRUE if staggered, orxFALSE otherwise
 */
orxBOOL orxFASTCALL orxFX_GetStagger(const orxFX *_pstFX, orxFLOAT *_pfOffset)
{
  orxBOOL bResult;

  /* Checks */
  orxASSERT(sstFX.u32Flags & orxFX_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstFX);

  /* Updates result */
  bResult = (orxStructure_TestFlags(_pstFX, orxFX_KU32_FLAG_STAGGERED)) ? orxTRUE : orxFALSE;

  /* Retrieves offset */
  if(_pfOffset != orxNULL)
  {
    *_pfOffset = _pstFX->fOffset;
  }

  /* Done! */
  return bResult;
}
