/**
 * @file orxFX.c
 */

/***************************************************************************
 orxFX.c
 FX module
 begin                : 30/06/2008
 author               : (C) Arcallians
 email                : iarwain@arcallians.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This library is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License           *
 *   as published by the Free Software Foundation; either version 2.1      *
 *   of the License, or (at your option) any later version.                *
 *                                                                         *
 ***************************************************************************/


#include "render/orxFX.h"

#include "debug/orxDebug.h"
#include "memory/orxMemory.h"
#include "core/orxConfig.h"
#include "core/orxClock.h"
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

#define orxFX_KU32_MASK_ALL                     0xFFFFFFFF  /**< All mask */

/** Slot flags
 */
#define orxFX_SLOT_KU32_FLAG_NONE               0x00000000  /**< No flag */

#define orxFX_SLOT_KU32_FLAG_DEFINED            0x10000000  /**< Defined flag */

#define orxFX_SLOT_KU32_MASK_BLEND_CURVE        0x0000000F  /**< Blend curve mask */

#define orxFX_SLOT_KU32_MASK_TYPE               0x000000F0  /**< FX type mask */


#define orxFX_SLOT_KU32_MASK_USER_ALL           0x0000FF00  /**< User all mask */

#define orxFX_SLOT_KU32_MASK_ALL                0xFFFFFFFF  /**< All mask */

#define orxFX_SLOT_KU32_SHIFT_TYPE              4


/** Misc defines
 */
#define orxFX_KU32_REFERENCE_TABLE_SIZE         32

#define orxFX_KU32_SLOT_NUMBER                  5

#define orxFX_KZ_CONFIG_SLOT                    "Slot"
#define orxFX_KZ_CONFIG_TYPE                    "Type"
#define orxFX_KZ_CONFIG_CURVE                   "Curve"
#define orxFX_KZ_CONFIG_SQUARED                 "Squared"
#define orxFX_KZ_CONFIG_ABSOLUTE                "Absolute"
#define orxFX_KZ_CONFIG_LOOP                    "Loop"
#define orxFX_KZ_CONFIG_AMPLIFICATION           "Amplification"
#define orxFX_KZ_CONFIG_START_TIME              "StartTime"
#define orxFX_KZ_CONFIG_END_TIME                "EndTime"
#define orxFX_KZ_CONFIG_PERIOD                  "Period"
#define orxFX_KZ_CONFIG_PHASIS                  "Phasis"
#define orxFX_KZ_CONFIG_START_ALPHA             "StartAlpha"
#define orxFX_KZ_CONFIG_END_ALPHA               "EndAlpha"
#define orxFX_KZ_CONFIG_START_COLOR             "StartColor"
#define orxFX_KZ_CONFIG_END_COLOR               "EndColor"
#define orxFX_KZ_CONFIG_START_ROTATION          "StartRotation"
#define orxFX_KZ_CONFIG_END_ROTATION            "EndRotation"
#define orxFX_KZ_CONFIG_START_SCALE             "StartScale"
#define orxFX_KZ_CONFIG_END_SCALE               "EndScale"
#define orxFX_KZ_CONFIG_START_POSITION          "StartPosition"
#define orxFX_KZ_CONFIG_END_POSITION            "EndPosition"
#define orxFX_KZ_CONFIG_KEEP_IN_CACHE           "KeepInCache"

#define orxFX_KZ_LINEAR                         "linear"
#define orxFX_KZ_SAW                            "saw"
#define orxFX_KZ_SINE                           "sine"
#define orxFX_KZ_ALPHA                          "alpha"
#define orxFX_KZ_COLOR                          "color"
#define orxFX_KZ_ROTATION                       "rotation"
#define orxFX_KZ_SCALE                          "scale"
#define orxFX_KZ_TRANSLATION                    "translation"


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** FX type enum
 */
typedef enum __orxFX_TYPE_t
{
	orxFX_TYPE_ALPHA_FADE = 0,
	orxFX_TYPE_COLOR_BLEND,
	orxFX_TYPE_ROTATION,
	orxFX_TYPE_SCALE,
	orxFX_TYPE_TRANSLATION,

	orxFX_TYPE_NUMBER,

	orxFX_TYPE_MAX_NUMBER = 16,

	orxFX_TYPE_NONE = orxENUM_NONE

} orxFX_TYPE;

/** FX slot
 */
typedef struct __orxFX_SLOT_t
{
  orxFLOAT    fStartTime;                       /**< Start Time : 4 */
  orxFLOAT    fEndTime;                         /**< End Time : 8 */
  orxFLOAT    fCyclePeriod;                     /**< Cycle period : 12 */
  orxFLOAT    fCyclePhasis;                     /**< Cycle phasis : 16 */
  orxFLOAT    fAmplification;                   /**< Amplification over time : 20 */

  union
  {
    struct
    {
      orxFLOAT fStartAlpha;                     /**< Alpha start value : 24 */
      orxFLOAT fEndAlpha;                       /**< Alpha end value : 28 */
    };                                          /**< Alpha Fade  : 28 */

    struct
    {
      orxVECTOR vStartColor;                    /**< ColorBlend start value : 32 */
      orxVECTOR vEndColor;                      /**< ColorBlend end value : 44 */
    };                                          /** Color blend : 44 */

    struct
    {
      orxFLOAT fStartRotation;                  /**< Rotation start value : 24 */
      orxFLOAT fEndRotation;                    /**< Rotation end value : 28 */
    };                                          /**< Scale : 28 */

    struct
    {
      orxVECTOR vStartScale;                    /**< Scale start value : 32 */
      orxVECTOR vEndScale;                      /**< Scale end value : 44 */
    };                                          /**< Scale : 44 */

    struct
    {
      orxVECTOR vStartPosition;                 /**< Translation vector : 32 */
      orxVECTOR vEndPosition;                   /**< Translation end position : 44 */
    };                                          /**< Translation : 44 */
  };

  orxU32 u32Flags;                              /**< Flags : 48 */

} orxFX_SLOT;

/** FX structure
 */
struct __orxFX_t
{
  orxSTRUCTURE  stStructure;                            /**< Public structure, first structure member : 16 */
  orxU32        u32ID;                                  /**< FX ID : 20 */
  orxFLOAT      fDuration;                              /**< FX duration : 24 */
  orxFX_SLOT    astFXSlotList[orxFX_KU32_SLOT_NUMBER];  /**< FX slot list : 264 */

  /* Padding */
  orxPAD(72)
};

/** Static structure
 */
typedef struct __orxFX_STATIC_t
{
  orxHASHTABLE *pstReferenceTable;              /**< Reference hash table */
  orxU32        u32Flags;                       /**< Control flags */

} orxFX_STATIC;


/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** Static data
 */
orxSTATIC orxFX_STATIC sstFX;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

/** Gets FX slot type
 * @param[in] _pstFXSlot        Concerned FX slot
 * @return orxFX_TYPE
 */
orxSTATIC orxINLINE orxFX_TYPE orxFX_GetSlotType(orxCONST orxFX_SLOT *_pstFXSlot)
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
orxSTATIC orxINLINE orxU32 orxFX_FindEmptySlotIndex(orxCONST orxFX *_pstFX)
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

orxSTATIC orxINLINE orxSTATUS orxFX_AddSlotFromConfig(orxFX *_pstFX, orxCONST orxSTRING _zSlotID)
{
  orxSTRING zPreviousSection;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxSTRUCTURE_ASSERT(_pstFX);
  orxASSERT((_zSlotID != orxNULL) && (*_zSlotID != *orxSTRING_EMPTY));

  /* Checks */
  orxSTRUCTURE_ASSERT(_pstFX);

  /* Gets previous config section */
  zPreviousSection = orxConfig_GetCurrentSection();

  /* Selects section */
  if((orxConfig_HasSection(_zSlotID) != orxFALSE)
  && (orxConfig_SelectSection(_zSlotID) != orxSTATUS_FAILURE))
  {
    orxSTRING   zCurveType;
    orxFX_CURVE eCurve;

    /* Gets its curve type */
    zCurveType = orxString_LowerCase(orxConfig_GetString(orxFX_KZ_CONFIG_CURVE));

    /* Linear curve? */
    if(orxString_Compare(zCurveType, orxFX_KZ_LINEAR) == 0)
    {
      /* Updates its curve */
      eCurve = orxFX_CURVE_LINEAR;
    }
    /* Saw curve? */
    else if(orxString_Compare(zCurveType, orxFX_KZ_SAW) == 0)
    {
      /* Updates its curve */
      eCurve = orxFX_CURVE_SAW;
    }
    /* Sine curve? */
    else if(orxString_Compare(zCurveType, orxFX_KZ_SINE) == 0)
    {
      /* Updates its curve */
      eCurve = orxFX_CURVE_SINE;
    }
    else
    {
      /* !!! MSG !!! */

      /* Updates result */
      eResult = orxSTATUS_FAILURE;
    }

    /* Valid? */
    if(eResult == orxSTATUS_SUCCESS)
    {
      orxFLOAT  fStartTime, fEndTime, fCyclePeriod, fCyclePhasis, fAmplification;
      orxSTRING zType;
      orxU32    u32Flags = 0;

      /* Gets its start & end time */
      fStartTime  = orxConfig_GetFloat(orxFX_KZ_CONFIG_START_TIME);
      fEndTime    = orxConfig_GetFloat(orxFX_KZ_CONFIG_END_TIME);

      /* Gets its cycle period */
      fCyclePeriod = orxConfig_GetFloat(orxFX_KZ_CONFIG_PERIOD);

      /* Gets it cycle phasis and convert it from degress to radians */
      fCyclePhasis = orxConfig_GetFloat(orxFX_KZ_CONFIG_PHASIS);

      /* Gets its amplification */
      fAmplification = orxConfig_HasValue(orxFX_KZ_CONFIG_AMPLIFICATION) ? orxConfig_GetFloat(orxFX_KZ_CONFIG_AMPLIFICATION) : orxFLOAT_1;

      /* Is absolute? */
      if(orxConfig_GetBool(orxFX_KZ_CONFIG_ABSOLUTE) != orxFALSE)
      {
        /* Updates flags */
        u32Flags |= orxFX_SLOT_KU32_FLAG_ABSOLUTE;
      }

      /* Is squared? */
      if(orxConfig_GetBool(orxFX_KZ_CONFIG_SQUARED) != orxFALSE)
      {
        /* Updates flags */
        u32Flags |= orxFX_SLOT_KU32_FLAG_CURVE_SQUARED;
      }

      /* Gets its type */
      zType = orxString_LowerCase(orxConfig_GetString(orxFX_KZ_CONFIG_TYPE));

      /* Alpha fade? */
      if(orxString_Compare(zType, orxFX_KZ_ALPHA) == 0)
      {
        orxFLOAT fStartAlpha, fEndAlpha;

        /* Gets alpha values */
        fStartAlpha = orxConfig_GetFloat(orxFX_KZ_CONFIG_START_ALPHA);
        fEndAlpha   = orxConfig_GetFloat(orxFX_KZ_CONFIG_END_ALPHA);

        /* Adds alpha fade slot */
        eResult = orxFX_AddAlphaFade(_pstFX, fStartTime, fEndTime, fCyclePeriod, fCyclePhasis, fAmplification, fStartAlpha, fEndAlpha, eCurve, u32Flags);
      }
      /* Color blend? */
      else if(orxString_Compare(zType, orxFX_KZ_COLOR) == 0)
      {
        orxVECTOR vStartColor, vEndColor;

        /* Gets color values */
        orxConfig_GetVector(orxFX_KZ_CONFIG_START_COLOR, &vStartColor);
        orxConfig_GetVector(orxFX_KZ_CONFIG_END_COLOR, &vEndColor);

        /* Adds color blend slot */
        eResult = orxFX_AddColorBlend(_pstFX, fStartTime, fEndTime, fCyclePeriod, fCyclePhasis, fAmplification, &vStartColor, &vEndColor, eCurve, u32Flags);
      }
      /* Rotation? */
      else if(orxString_Compare(zType, orxFX_KZ_ROTATION) == 0)
      {
        orxFLOAT fStartRotation, fEndRotation;

        /* Gets rotation values */
        fStartRotation  = orxConfig_GetFloat(orxFX_KZ_CONFIG_START_ROTATION);
        fEndRotation    = orxConfig_GetFloat(orxFX_KZ_CONFIG_END_ROTATION);

        /* Adds rotation slot */
        eResult = orxFX_AddRotation(_pstFX, fStartTime, fEndTime, fCyclePeriod, fCyclePhasis, fAmplification, orxMATH_KF_DEG_TO_RAD * fStartRotation, orxMATH_KF_DEG_TO_RAD * fEndRotation, eCurve, u32Flags);
      }
      /* Scale? */
      else if(orxString_Compare(zType, orxFX_KZ_SCALE) == 0)
      {
        orxVECTOR vStartScale, vEndScale;

        /* Is config start scale not a vector? */
        if(orxConfig_GetVector(orxFX_KZ_CONFIG_START_SCALE, &vStartScale) == orxSTATUS_FAILURE)
        {
          orxFLOAT fScale;

          /* Gets config uniformed scale */
          fScale = orxConfig_GetFloat(orxFX_KZ_CONFIG_START_SCALE);

          /* Updates vector */
          orxVector_SetAll(&vStartScale, fScale);
        }

        /* Is config end scale not a vector? */
        if(orxConfig_GetVector(orxFX_KZ_CONFIG_END_SCALE, &vEndScale) == orxSTATUS_FAILURE)
        {
          orxFLOAT fScale;

          /* Gets config uniformed scale */
          fScale = orxConfig_GetFloat(orxFX_KZ_CONFIG_END_SCALE);

          /* Updates vector */
          orxVector_SetAll(&vEndScale, fScale);
        }

        /* Adds scale slot */
        eResult = orxFX_AddScale(_pstFX, fStartTime, fEndTime, fCyclePeriod, fCyclePhasis, fAmplification, &vStartScale, &vEndScale, eCurve, u32Flags);
      }
      /* Translation? */
      else if(orxString_Compare(zType, orxFX_KZ_TRANSLATION) == 0)
      {
        orxVECTOR vStartPosition, vEndPosition;

        /* Gets scalevalues */
        orxConfig_GetVector(orxFX_KZ_CONFIG_START_POSITION, &vStartPosition);
        orxConfig_GetVector(orxFX_KZ_CONFIG_END_POSITION, &vEndPosition);

        /* Adds scale slot */
        eResult = orxFX_AddTranslation(_pstFX, fStartTime, fEndTime, fCyclePeriod, fCyclePhasis, fAmplification, &vStartPosition, &vEndPosition, eCurve, u32Flags);
      }
    }

    /* Restores previous section */
    orxConfig_SelectSection(zPreviousSection);
  }
  else
  {
    /* !!! MSG !!! */

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Deletes all the FXs
 */
orxSTATIC orxINLINE orxVOID orxFX_DeleteAll()
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

  return;
}


/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** FX module setup
 */
orxVOID orxFX_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_FX, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_FX, orxMODULE_ID_STRUCTURE);
  orxModule_AddDependency(orxMODULE_ID_FX, orxMODULE_ID_CONFIG);
  orxModule_AddDependency(orxMODULE_ID_FX, orxMODULE_ID_HASHTABLE);

  return;
}

/** Inits the FX module
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFX_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(orxFX_TYPE_NUMBER <= orxFX_TYPE_MAX_NUMBER);

  /* Not already Initialized? */
  if(!(sstFX.u32Flags & orxFX_KU32_STATIC_FLAG_READY))
  {
    /* Cleans static controller */
    orxMemory_Zero(&sstFX, sizeof(orxFX_STATIC));

    /* Creates reference table */
    sstFX.pstReferenceTable = orxHashTable_Create(orxFX_KU32_REFERENCE_TABLE_SIZE, orxHASHTABLE_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

    /* Valid? */
    if(sstFX.pstReferenceTable != orxNULL)
    {
      /* Registers structure type */
      eResult = orxSTRUCTURE_REGISTER(FX, orxSTRUCTURE_STORAGE_TYPE_LINKLIST, orxMEMORY_TYPE_MAIN, orxNULL);
    }
    else
    {
      /* !!! MSG !!! */
    }
  }
  else
  {
    /* !!! MSG !!! */

    /* Already initialized */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Initialized? */
  if(eResult == orxSTATUS_SUCCESS)
  {
    /* Inits Flags */
    sstFX.u32Flags = orxFX_KU32_STATIC_FLAG_READY;
  }

  /* Done! */
  return eResult;
}

/** Exits from the FX module
 */
orxVOID orxFX_Exit()
{
  /* Initialized? */
  if(sstFX.u32Flags & orxFX_KU32_STATIC_FLAG_READY)
  {
    /* Deletes FX list */
    orxFX_DeleteAll();

    /* Unregisters structure type */
    orxStructure_Unregister(orxSTRUCTURE_ID_FX);

    /* Deletes reference table */
    orxHashTable_Delete(sstFX.pstReferenceTable);

    /* Updates flags */
    sstFX.u32Flags &= ~orxFX_KU32_STATIC_FLAG_READY;
  }
  else
  {
    /* !!! MSG !!! */
  }

  return;
}

/** Creates an empty FX
 * @return      Created orxFX / orxNULL
 */
orxFX *orxFX_Create()
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
  }
  else
  {
    /* !!! MSG !!! */
  }

  /* Done! */
  return pstResult;
}

/** Creates an FX from config
 * @param[in]   _zConfigID            Config ID
 * @ return orxFX / orxNULL
 */
orxFX *orxFASTCALL orxFX_CreateFromConfig(orxCONST orxSTRING _zConfigID)
{
  orxU32  u32ID;
  orxFX  *pstResult;

  /* Checks */
  orxASSERT(sstFX.u32Flags & orxFX_KU32_STATIC_FLAG_READY);
  orxASSERT((_zConfigID != orxNULL) && (*_zConfigID != *orxSTRING_EMPTY));

  /* Gets FX ID */
  u32ID = orxString_ToCRC(_zConfigID);

  /* Search for reference */
  pstResult = orxHashTable_Get(sstFX.pstReferenceTable, u32ID);

  /* Not already created? */
  if(pstResult == orxNULL)
  {
    orxSTRING zPreviousSection;

    /* Gets previous config section */
    zPreviousSection = orxConfig_GetCurrentSection();

    /* Selects section */
    if((orxConfig_HasSection(_zConfigID) != orxFALSE)
    && (orxConfig_SelectSection(_zConfigID) != orxSTATUS_FAILURE))
    {
      /* Creates FX */
      pstResult = orxFX_Create();

      /* Valid? */
      if(pstResult != orxNULL)
      {
        orxCHAR acSlotID[16];
        orxU32  i;

        /* Clears buffer */
        orxMemory_Zero(acSlotID, 16 * sizeof(orxCHAR));

        /* Stores its ID */
        pstResult->u32ID = orxString_ToCRC(_zConfigID);

        /* Adds it to reference table */
        if(orxHashTable_Add(sstFX.pstReferenceTable, pstResult->u32ID, pstResult) != orxSTATUS_FAILURE)
        {
          /* For all slots */
          for(i = 0; i < orxFX_KU32_SLOT_NUMBER; i++)
          {
            orxSTRING zSlotName;

            /* Gets its ID */
            orxString_Print(acSlotID, "%s%d", orxFX_KZ_CONFIG_SLOT, i + 1);

            /* Gets its name */
            zSlotName = orxConfig_GetString(acSlotID);

            /* Valid? */
            if((zSlotName != orxNULL) && (*zSlotName != *orxSTRING_EMPTY))
            {
              /* Adds slot from config */
              orxFX_AddSlotFromConfig(pstResult, zSlotName);
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
            orxStructure_SetFlags(pstResult, orxFX_KU32_FLAG_LOOP, orxFX_KU32_FLAG_NONE);
          }

          /* Should keep it in cache? */
          if(orxConfig_GetBool(orxFX_KZ_CONFIG_KEEP_IN_CACHE) != orxFALSE)
          {
            /* Increases its reference counter to keep it in cache table */
            orxStructure_IncreaseCounter(pstResult);
          }
        }
        else
        {
          /* !!! MSG !!! */

          /* Deletes it */
          orxFX_Delete(pstResult);

          /* Updates result */
          pstResult = orxNULL;
        }
      }

      /* Restores previous section */
      orxConfig_SelectSection(zPreviousSection);
    }
    else
    {
      /* !!! MSG !!! */

      /* Updates result */
      pstResult = orxNULL;
    }
  }
  else
  {
    /* Updates reference counter */
    orxStructure_IncreaseCounter(pstResult);
  }

  /* Done! */
  return pstResult;
}

/** Deletes an FX
 * @param[in] _pstFX            Concerned FX
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxFX_Delete(orxFX *_pstFX)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstFX.u32Flags & orxFX_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstFX);

  /* Has an ID? */
  if((_pstFX->u32ID != 0)
  && (_pstFX->u32ID != orxU32_UNDEFINED))
  {
    /* Not referenced? */
    if(orxStructure_GetRefCounter(_pstFX) == 0)
    {
      /* Removes from hashtable */
      orxHashTable_Remove(sstFX.pstReferenceTable, _pstFX->u32ID);

      /* Deletes structure */
      orxStructure_Delete(_pstFX);
    }
    else
    {
      /* Decreases its reference counter */
      orxStructure_DecreaseCounter(_pstFX);
    }
  }
  else
  {
    /* Not referenced? */
    if(orxStructure_GetRefCounter(_pstFX) == 0)
    {
      /* Deletes structure */
      orxStructure_Delete(_pstFX);
    }
    else
    {
      /* !!! MSG !!! */

      /* Referenced by others */
      eResult = orxSTATUS_FAILURE;
    }
  }

  /* Done! */
  return eResult;
}

/** Applies FX on object
 * @param[in] _pstFX            FX to apply
 * @param[in] _pstObject        Object on which to apply the FX
 * @param[in] _fStartTime       FX local application start time
 * @param[in] _fEndTime         FX local application end time
 * @return    orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxFX_Apply(orxCONST orxFX *_pstFX, orxOBJECT *_pstObject, orxFLOAT _fStartTime, orxFLOAT _fEndTime)
{
  orxU32    i;
  orxCOLOR  stColor;
  orxBOOL   bAlphaLock = orxFALSE, bColorBlendLock = orxFALSE, bRotationLock = orxFALSE, bScaleLock = orxFALSE, bTranslationLock = orxFALSE;
  orxBOOL   bAlphaUpdate = orxFALSE, bColorBlendUpdate = orxFALSE, bRotationUpdate = orxFALSE, bScaleUpdate = orxFALSE, bTranslationUpdate = orxFALSE;
  orxBOOL   bFirstCall;
  orxFLOAT  fAlpha = orxFLOAT_0, fRotation = orxFLOAT_0, fInvDuration;
  orxVECTOR vColor, vScale, vPosition;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxSTRUCTURE_ASSERT(_pstFX);
  orxSTRUCTURE_ASSERT(_pstObject);
  orxASSERT(_fEndTime >= _fStartTime);

  /* Has started? */
  if(_fEndTime >= orxFLOAT_0)
  {
    /* Clears color, scale and position vectors */
    orxVector_SetAll(&vColor, orxFLOAT_0);
    orxVector_SetAll(&vScale, orxFLOAT_1);
    orxVector_SetAll(&vPosition, orxFLOAT_0);

    /* Has object color? */
    if(orxObject_HasColor(_pstObject) != orxFALSE)
    {
      /* Stores object color */
      orxObject_GetColor(_pstObject, &stColor);
    }
    else
    {
      /* Clears color */
      orxColor_Set(&stColor, &orxVECTOR_WHITE, orx2F(255.0f));
    }

    /* For all slots */
    for(i = 0; i< orxFX_KU32_SLOT_NUMBER; i++)
    {
      orxCONST orxFX_SLOT *pstFXSlot;

      /* Gets the slot */
      pstFXSlot = &(_pstFX->astFXSlotList[i]);

      /* Is defined? */
      if(orxFLAG_TEST(pstFXSlot->u32Flags, orxFX_SLOT_KU32_FLAG_DEFINED))
      {
        orxFLOAT fStartTime, fEndTime, fPeriod, fFrequency, fStartCoef, fEndCoef;

        /* Gets corrected start and end time */
        fStartTime  = orxMAX(_fStartTime, pstFXSlot->fStartTime);
        fEndTime    = orxMIN(_fEndTime, pstFXSlot->fEndTime);

        /* Updates first call status */
        bFirstCall = (fStartTime == pstFXSlot->fStartTime) ? orxTRUE : orxFALSE;

        /* Is this slot active in the time period? */
        if(fEndTime >= fStartTime)
        {
          orxFX_TYPE eFXType;

          /* Gets 1.0 / duration */
          fInvDuration = orxFLOAT_1 / (pstFXSlot->fEndTime - pstFXSlot->fStartTime);

          /* Gets FX type */
          eFXType = orxFX_GetSlotType(pstFXSlot);

          /* Is FX type not blocked? */
          if(((eFXType == orxFX_TYPE_ALPHA_FADE) && (bAlphaLock == orxFALSE))
          || ((eFXType == orxFX_TYPE_COLOR_BLEND) && (bColorBlendLock == orxFALSE))
          || ((eFXType == orxFX_TYPE_ROTATION) && (bRotationLock == orxFALSE))
          || ((eFXType == orxFX_TYPE_SCALE) && (bScaleLock == orxFALSE))
          || ((eFXType == orxFX_TYPE_TRANSLATION) && (bTranslationLock == orxFALSE)))
          {
            /* Has a valid cycle period? */
            if(pstFXSlot->fCyclePeriod > orxFLOAT_0)
            {
              /* Gets it */
              fPeriod = pstFXSlot->fCyclePeriod;
            }
            else
            {
              /* Gets whole duration as period */
              fPeriod = pstFXSlot->fEndTime - pstFXSlot->fStartTime;
            }

            /* Gets its corresponding frequency */
            fFrequency = (fPeriod != orxFLOAT_0) ? (orxFLOAT_1 / fPeriod) : orxFLOAT_1;

            /* Depending on blend curve */
            switch(pstFXSlot->u32Flags & orxFX_SLOT_KU32_MASK_BLEND_CURVE)
            {
              case orxFX_CURVE_LINEAR:
              {
                /* Gets linear start coef in period [0.0; 1.0] starting at given phasis */
                fStartCoef = (fStartTime * fFrequency) + pstFXSlot->fCyclePhasis;

                /* Non zero? */
                if(fStartCoef != orxFLOAT_0)
                {
                  /* Gets its modulo */
                  fStartCoef = orxMath_Mod(fStartCoef, orxFLOAT_1);

                  /* Zero? */
                  if(fStartCoef == orxFLOAT_0)
                  {
                    /* Sets it at max value */
                    fStartCoef = orxFLOAT_1;
                  }
                }

                /* Gets linear end coef in period [0.0; 1.0] starting at given phasis */
                fEndCoef = (fEndTime * fFrequency) + pstFXSlot->fCyclePhasis;

                /* Non zero? */
                if(fEndCoef != orxFLOAT_0)
                {
                  /* Gets its modulo */
                  fEndCoef = orxMath_Mod(fEndCoef, orxFLOAT_1);

                  /* Zero? */
                  if(fEndCoef == orxFLOAT_0)
                  {
                    /* Sets it at max value */
                    fEndCoef = orxFLOAT_1;
                  }
                }

                break;
              }

              case orxFX_CURVE_SAW:
              {
                /* Gets linear coef in period [0.0; 2.0] starting at given phasis */
                fStartCoef = (fStartTime * fFrequency) + pstFXSlot->fCyclePhasis;
                fStartCoef = orxMath_Mod(fStartCoef * orx2F(2.0f), orx2F(2.0f));

                /* Gets symetric coef between 1.0 & 2.0 */
                if(fStartCoef > orxFLOAT_1)
                {
                  fStartCoef = orx2F(2.0f) - fStartCoef;
                }
                
                /* Gets linear coef in period [0.0; 2.0] starting at given phasis */
                fEndCoef = (fEndTime * fFrequency) + pstFXSlot->fCyclePhasis;
                fEndCoef = orxMath_Mod(fEndCoef * orx2F(2.0f), orx2F(2.0f));

                /* Gets symetric coef between 1.0 & 2.0 */
                if(fEndCoef > orxFLOAT_1)
                {
                  fEndCoef = orx2F(2.0f) - fEndCoef;
                }

                break;
              }

              case orxFX_CURVE_SINE:
              {
                /* Gets sine coef starting at given phasis * 2Pi - Pi/2 */
                fStartCoef = (orxMath_Sin((orxMATH_KF_2_PI * (fStartTime + (fPeriod * (pstFXSlot->fCyclePhasis - orx2F(0.25f))))) * fFrequency) + orxFLOAT_1) * orx2F(0.5f);

                /* Gets sine coef starting at given phasis * 2Pi - Pi/2 */
                fEndCoef = (orxMath_Sin((orxMATH_KF_2_PI * (fEndTime + (fPeriod * (pstFXSlot->fCyclePhasis - orx2F(0.25f))))) * fFrequency) + orxFLOAT_1) * orx2F(0.5f);

                break;
              }

              default:
              {
                /* !!! MSG !!! */

                /* Skips it */
                continue;
              }
            }

            /* Has amplification? */
            if(pstFXSlot->fAmplification != orxFLOAT_1)
            {
              orxFLOAT fStartAmplification, fEndAmplification;

              /* Gets amplification coefs */
              fStartAmplification = orxLERP(orxFLOAT_1, pstFXSlot->fAmplification, (fStartTime - pstFXSlot->fStartTime) * fInvDuration);
              fEndAmplification   = orxLERP(orxFLOAT_1, pstFXSlot->fAmplification, (fEndTime - pstFXSlot->fStartTime) * fInvDuration);

              /* Updates the coefs */
              fStartCoef *= fStartAmplification;
              fEndCoef   *= fEndAmplification;
            }

            /* Using a squared curve? */
            if(orxFLAG_TEST(pstFXSlot->u32Flags, orxFX_SLOT_KU32_FLAG_CURVE_SQUARED))
            {
              /* Squares both coefs */
              fStartCoef *= fStartCoef;
              fEndCoef   *= fEndCoef;
            }

            /* Clamps the coefs */
            fStartCoef  = orxCLAMP(fStartCoef, orxFLOAT_0, orxFLOAT_1);
            fEndCoef    = orxCLAMP(fEndCoef, orxFLOAT_0, orxFLOAT_1);

            /* Depending on FX type */
            switch(eFXType)
            {
              case orxFX_TYPE_ALPHA_FADE:
              {
                /* Absolute ? */
                if(orxFLAG_TEST(pstFXSlot->u32Flags, orxFX_SLOT_KU32_FLAG_ABSOLUTE))
                {
                  /* Overrides value */
                  fAlpha = orxLERP(pstFXSlot->fStartAlpha, pstFXSlot->fEndAlpha, fEndCoef);

                  /* Locks it */
                  bAlphaLock = orxTRUE;
                }
                else
                {
                  orxFLOAT fStartAlpha, fEndAlpha;

                  /* First call? */
                  if(bFirstCall != orxFALSE)
                  {
                    /* Gets start value */
                    fStartAlpha = orxFLOAT_0;
                  }
                  else
                  {
                    /* Gets start value */
                    fStartAlpha = orxLERP(pstFXSlot->fStartAlpha, pstFXSlot->fEndAlpha, fStartCoef);
                  }

                  /* Gets end value */
                  fEndAlpha = orxLERP(pstFXSlot->fStartAlpha, pstFXSlot->fEndAlpha, fEndCoef);

                  /* Updates global alpha value */
                  fAlpha += fEndAlpha - fStartAlpha;
                }

                /* Updates alpha status */
                bAlphaUpdate = orxTRUE;

                break;
              }

              case orxFX_TYPE_COLOR_BLEND:
              {
                /* Absolute ? */
                if(orxFLAG_TEST(pstFXSlot->u32Flags, orxFX_SLOT_KU32_FLAG_ABSOLUTE))
                {
                  /* Overrides values */
                  orxVector_Lerp(&vColor, &(pstFXSlot->vStartColor), &(pstFXSlot->vEndColor), fEndCoef);

                  /* Locks it */
                  bColorBlendLock = orxTRUE;

                }
                else
                {
                  orxVECTOR vStartColor, vEndColor;

                  /* First call? */
                  if(bFirstCall != orxFALSE)
                  {
                    /* Gets start value */
                    orxVector_SetAll(&vStartColor, orxFLOAT_0);
                  }
                  else
                  {
                    /* Gets start value */
                    orxVector_Lerp(&vStartColor, &(pstFXSlot->vStartColor), &(pstFXSlot->vEndColor), fStartCoef);
                  }

                  /* Gets end value */
                  orxVector_Lerp(&vEndColor, &(pstFXSlot->vStartColor), &(pstFXSlot->vEndColor), fEndCoef);

                  /* Updates global color value */
                  orxVector_Add(&vColor, &vColor, orxVector_Sub(&vEndColor, &vEndColor, &vStartColor));
                }

                /* Updates color blend status */
                bColorBlendUpdate = orxTRUE;

                break;
              }

              case orxFX_TYPE_ROTATION:
              {
                /* Absolute ? */
                if(orxFLAG_TEST(pstFXSlot->u32Flags, orxFX_SLOT_KU32_FLAG_ABSOLUTE))
                {
                  /* Overrides value */
                  fRotation = orxLERP(pstFXSlot->fStartRotation, pstFXSlot->fEndRotation, fEndCoef);

                  /* Locks it */
                  bRotationLock = orxTRUE;
                }
                else
                {
                  orxFLOAT fStartRotation, fEndRotation;

                  /* First call? */
                  if(bFirstCall != orxFALSE)
                  {
                    /* Gets start value */
                    fStartRotation = orxFLOAT_0;
                  }
                  else
                  {
                    /* Gets start value */
                    fStartRotation = orxLERP(pstFXSlot->fStartAlpha, pstFXSlot->fEndAlpha, fStartCoef);
                  }

                  /* Gets end value */
                  fEndRotation = orxLERP(pstFXSlot->fStartRotation, pstFXSlot->fEndRotation, fEndCoef);

                  /* Updates global alpha value */
                  fRotation += fEndRotation - fStartRotation;
                }

                /* Updates rotation status */
                bRotationUpdate = orxTRUE;

                break;
              }

              case orxFX_TYPE_SCALE:
              {
                /* Absolute ? */
                if(orxFLAG_TEST(pstFXSlot->u32Flags, orxFX_SLOT_KU32_FLAG_ABSOLUTE))
                {
                  /* Overrides values */
                  orxVector_Lerp(&vScale, &(pstFXSlot->vStartScale), &(pstFXSlot->vEndScale), fEndCoef);

                  /* Locks it */
                  bScaleLock = orxTRUE;

                }
                else
                {
                  orxVECTOR vStartScale, vEndScale;

                  /* First call? */
                  if(bFirstCall != orxFALSE)
                  {
                    /* Gets start value */
                    orxVector_SetAll(&vStartScale, orxFLOAT_1);
                  }
                  else
                  {
                    /* Gets start value */
                    orxVector_Lerp(&vStartScale, &(pstFXSlot->vStartScale), &(pstFXSlot->vEndScale), fStartCoef);

                    /* Neutralizes Z scale */
                    vStartScale.fZ = orxFLOAT_1;
                  }

                  /* Gets end value */
                  orxVector_Lerp(&vEndScale, &(pstFXSlot->vStartScale), &(pstFXSlot->vEndScale), fEndCoef);

                  /* Updates global scale value */
                  orxVector_Mul(&vScale, &vScale, orxVector_Div(&vEndScale, &vEndScale, &vStartScale));
                }

                /* Updates scale status */
                bScaleUpdate = orxTRUE;

                break;
              }

              case orxFX_TYPE_TRANSLATION:
              {
                /* Absolute ? */
                if(orxFLAG_TEST(pstFXSlot->u32Flags, orxFX_SLOT_KU32_FLAG_ABSOLUTE))
                {
                  /* Overrides values */
                  orxVector_Lerp(&vPosition, &(pstFXSlot->vStartPosition), &(pstFXSlot->vEndPosition), fEndCoef);

                  /* Locks it */
                  bTranslationLock = orxTRUE;

                }
                else
                {
                  orxVECTOR vStartPosition, vEndPosition;

                  /* First call? */
                  if(bFirstCall != orxFALSE)
                  {
                    /* Gets start value */
                    orxVector_SetAll(&vStartPosition, orxFLOAT_0);
                  }
                  else
                  {
                    /* Gets start value */
                    orxVector_Lerp(&vStartPosition, &(pstFXSlot->vStartPosition), &(pstFXSlot->vEndPosition), fStartCoef);
                  }

                  /* Gets end value */
                  orxVector_Lerp(&vEndPosition, &(pstFXSlot->vStartPosition), &(pstFXSlot->vEndPosition), fEndCoef);

                  /* Updates global position value */
                  orxVector_Add(&vPosition, &vPosition, orxVector_Sub(&vEndPosition, &vEndPosition, &vStartPosition));
                }

                /* Updates translation status */
                bTranslationUpdate = orxTRUE;

                break;
              }

              default:
              {
                /* !!! MSG !!! */

                break;
              }
            }
          }
        }
      }
      else
      {
        /* Done with the slots */
        break;
      }
    }

    /* Global color update? */
    if((bAlphaUpdate != orxFALSE)
    || (bColorBlendUpdate != orxFALSE))
    {
      /* Update alpha? */
      if(bAlphaUpdate != orxFALSE)
      {
        /* Non absolute? */
        if(bAlphaLock == orxFALSE)
        {
          /* Updates alpha with previous one */
          fAlpha += stColor.fAlpha;
        }
      }
      else
      {
        /* Resets alpha */
        fAlpha = stColor.fAlpha;
      }

      /* Update color blend? */
      if(bColorBlendUpdate != orxFALSE)
      {
        /* Non absolute */
        if(bColorBlendLock == orxFALSE)
        {
          /* Updates color with previous one */
          orxVector_Add(&vColor, &vColor, &(stColor.vRGB));
        }
      }
      else
      {
        /* Resets color */
        orxVector_Copy(&vColor, &(stColor.vRGB));
      }

      /* Updates global color */
      orxColor_SetRGB(&stColor, &vColor);
      orxColor_SetAlpha(&stColor, fAlpha);

      /* Applies it */
      orxObject_SetColor(_pstObject, &stColor);
    }

    /* Update rotation? */
    if(bRotationUpdate != orxFALSE)
    {
      /* Non absolute? */
      if(bRotationLock == orxFALSE)
      {
        /* Updates rotation with previous one */
        fRotation += orxObject_GetRotation(_pstObject);
      }

      /* Applies it */
      orxObject_SetRotation(_pstObject, fRotation);
    }

    /* Update scale? */
    if(bScaleUpdate != orxFALSE)
    {
      /* Non absolute? */
      if(bScaleLock == orxFALSE)
      {
        orxFLOAT fObjectScaleX, fObjectScaleY;

        /* Gets object scale */
        orxObject_GetScale(_pstObject, &fObjectScaleX, &fObjectScaleY);

        /* Updates scale with previous one */
        vScale.fX *= fObjectScaleX;
        vScale.fY *= fObjectScaleY;
      }

      /* Applies it */
      orxObject_SetScale(_pstObject, vScale.fX, vScale.fY);
    }

    /* Update translation? */
    if(bTranslationUpdate != orxFALSE)
    {
      /* Non absolute? */
      if(bTranslationLock == orxFALSE)
      {
        orxVECTOR vObjectPosition;

        /* Updates position with previous one */
        orxVector_Add(&vPosition, &vPosition, orxObject_GetPosition(_pstObject, &vObjectPosition));
      }

      /* Applies it */
      orxObject_SetPosition(_pstObject, &vPosition);
    }

    /* Updates result */
    eResult = (_fEndTime >= _pstFX->fDuration) ? orxSTATUS_FAILURE : orxSTATUS_SUCCESS;
  }

  /* Done! */
  return eResult;
}

/** Enables/disables an FX
 * @param[in]   _pstFX        Concerned FX
 * @param[in]   _bEnable      enable / disable
 */
orxVOID orxFASTCALL orxFX_Enable(orxFX *_pstFX, orxBOOL _bEnable)
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

  return;
}

/** Is FX enabled?
 * @param[in]   _pstFX        Concerned FX
 * @return      orxTRUE if enabled, orxFALSE otherwise
 */
orxBOOL orxFASTCALL orxFX_IsEnabled(orxCONST orxFX *_pstFX)
{
  /* Checks */
  orxASSERT(sstFX.u32Flags & orxFX_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstFX);

  /* Done! */
  return(orxStructure_TestFlags(_pstFX, orxFX_KU32_FLAG_ENABLED));
}

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
 * @param[in]   _u32Flags       Param flags
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxFX_AddAlphaFade(orxFX *_pstFX, orxFLOAT _fStartTime, orxFLOAT _fEndTime, orxFLOAT _fCyclePeriod, orxFLOAT _fCyclePhasis, orxFLOAT _fAmplification, orxFLOAT _fStartAlpha, orxFLOAT _fEndAlpha, orxFX_CURVE _eCurve, orxU32 _u32Flags)
{
  orxU32    u32Index;
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(sstFX.u32Flags & orxFX_KU32_STATIC_FLAG_READY);
  orxASSERT((_u32Flags & orxFX_SLOT_KU32_MASK_USER_ALL) == _u32Flags);
  orxASSERT(_eCurve < orxFX_CURVE_NUMBER);
  orxASSERT(_fStartTime >= orxFLOAT_0);
  orxASSERT(_fEndTime >= _fStartTime);
  orxSTRUCTURE_ASSERT(_pstFX);

  /* Finds empty slot index */
  u32Index = orxFX_FindEmptySlotIndex(_pstFX);

  /* Valid? */
  if(u32Index != orxU32_UNDEFINED)
  {
    orxFX_SLOT *pstFXSlot;

    /* Gets the slot */
    pstFXSlot = &(_pstFX->astFXSlotList[u32Index]);

    /* Updates its parameters */
    pstFXSlot->fStartTime     = _fStartTime;
    pstFXSlot->fEndTime       = _fEndTime;
    pstFXSlot->fCyclePeriod   = _fCyclePeriod;
    pstFXSlot->fCyclePhasis   = _fCyclePhasis;
    pstFXSlot->fAmplification = _fAmplification;
    pstFXSlot->fStartAlpha    = _fStartAlpha;
    pstFXSlot->fEndAlpha      = _fEndAlpha;
    pstFXSlot->u32Flags       = (_u32Flags & orxFX_SLOT_KU32_MASK_USER_ALL) | _eCurve | (orxFX_TYPE_ALPHA_FADE << orxFX_SLOT_KU32_SHIFT_TYPE) |orxFX_SLOT_KU32_FLAG_DEFINED;

    /* Is longer than current FX duration? */
    if(_fEndTime > _pstFX->fDuration)
    {
      /* Updates it */
      _pstFX->fDuration = _fEndTime;
    }
  }

  /* Done! */
  return eResult;
}

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
 * @param[in]   _u32Flags       Param flags
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxFX_AddColorBlend(orxFX *_pstFX, orxFLOAT _fStartTime, orxFLOAT _fEndTime, orxFLOAT _fCyclePeriod, orxFLOAT _fCyclePhasis, orxFLOAT _fAmplification, orxVECTOR *_pvStartColor, orxVECTOR *_pvEndColor, orxFX_CURVE _eCurve, orxU32 _u32Flags)
{
  orxU32    u32Index;
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(sstFX.u32Flags & orxFX_KU32_STATIC_FLAG_READY);
  orxASSERT((_u32Flags & orxFX_SLOT_KU32_MASK_USER_ALL) == _u32Flags);
  orxASSERT(_eCurve < orxFX_CURVE_NUMBER);
  orxASSERT(_fStartTime >= orxFLOAT_0);
  orxASSERT(_fEndTime >= _fStartTime);
  orxSTRUCTURE_ASSERT(_pstFX);
  orxASSERT(_pvStartColor != orxNULL);
  orxASSERT(_pvEndColor != orxNULL);

  /* Finds empty slot index */
  u32Index = orxFX_FindEmptySlotIndex(_pstFX);

  /* Valid? */
  if(u32Index != orxU32_UNDEFINED)
  {
    orxFX_SLOT *pstFXSlot;

    /* Gets the slot */
    pstFXSlot = &(_pstFX->astFXSlotList[u32Index]);

    /* Updates its parameters */
    pstFXSlot->fStartTime     = _fStartTime;
    pstFXSlot->fEndTime       = _fEndTime;
    pstFXSlot->fCyclePeriod   = _fCyclePeriod;
    pstFXSlot->fCyclePhasis   = _fCyclePhasis;
    pstFXSlot->fAmplification = _fAmplification;
    orxVector_Copy(&(pstFXSlot->vStartColor), _pvStartColor);
    orxVector_Copy(&(pstFXSlot->vEndColor), _pvEndColor);
    pstFXSlot->u32Flags       = (_u32Flags & orxFX_SLOT_KU32_MASK_USER_ALL) | _eCurve | (orxFX_TYPE_COLOR_BLEND << orxFX_SLOT_KU32_SHIFT_TYPE) | orxFX_SLOT_KU32_FLAG_DEFINED;

    /* Is longer than current FX duration? */
    if(_fEndTime > _pstFX->fDuration)
    {
      /* Updates it */
      _pstFX->fDuration = _fEndTime;
    }
  }

  /* Done! */
  return eResult;
}

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
 * @param[in]   _u32Flags       Param flags
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxFX_AddRotation(orxFX *_pstFX, orxFLOAT _fStartTime, orxFLOAT _fEndTime, orxFLOAT _fCyclePeriod, orxFLOAT _fCyclePhasis, orxFLOAT _fAmplification, orxFLOAT _fStartRotation, orxFLOAT _fEndRotation, orxFX_CURVE _eCurve, orxU32 _u32Flags)
{
  orxU32    u32Index;
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(sstFX.u32Flags & orxFX_KU32_STATIC_FLAG_READY);
  orxASSERT((_u32Flags & orxFX_SLOT_KU32_MASK_USER_ALL) == _u32Flags);
  orxASSERT(_eCurve < orxFX_CURVE_NUMBER);
  orxASSERT(_fStartTime >= orxFLOAT_0);
  orxASSERT(_fEndTime >= _fStartTime);
  orxSTRUCTURE_ASSERT(_pstFX);

  /* Finds empty slot index */
  u32Index = orxFX_FindEmptySlotIndex(_pstFX);

  /* Valid? */
  if(u32Index != orxU32_UNDEFINED)
  {
    orxFX_SLOT *pstFXSlot;

    /* Gets the slot */
    pstFXSlot = &(_pstFX->astFXSlotList[u32Index]);

    /* Updates its parameters */
    pstFXSlot->fStartTime     = _fStartTime;
    pstFXSlot->fEndTime       = _fEndTime;
    pstFXSlot->fCyclePeriod   = _fCyclePeriod;
    pstFXSlot->fCyclePhasis   = _fCyclePhasis;
    pstFXSlot->fAmplification = _fAmplification;
    pstFXSlot->fStartRotation = _fStartRotation;
    pstFXSlot->fEndRotation   = _fEndRotation;
    pstFXSlot->u32Flags       = (_u32Flags & orxFX_SLOT_KU32_MASK_USER_ALL) | _eCurve | (orxFX_TYPE_ROTATION << orxFX_SLOT_KU32_SHIFT_TYPE) | orxFX_SLOT_KU32_FLAG_DEFINED;

    /* Is longer than current FX duration? */
    if(_fEndTime > _pstFX->fDuration)
    {
      /* Updates it */
      _pstFX->fDuration = _fEndTime;
    }
  }

  /* Done! */
  return eResult;
}

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
 * @param[in]   _u32Flags       Param flags
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxFX_AddScale(orxFX *_pstFX, orxFLOAT _fStartTime, orxFLOAT _fEndTime, orxFLOAT _fCyclePeriod, orxFLOAT _fCyclePhasis, orxFLOAT _fAmplification, orxCONST orxVECTOR *_pvStartScale, orxCONST orxVECTOR *_pvEndScale, orxFX_CURVE _eCurve, orxU32 _u32Flags)
{
  orxU32    u32Index;
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(sstFX.u32Flags & orxFX_KU32_STATIC_FLAG_READY);
  orxASSERT((_u32Flags & orxFX_SLOT_KU32_MASK_USER_ALL) == _u32Flags);
  orxASSERT(_eCurve < orxFX_CURVE_NUMBER);
  orxASSERT(_fStartTime >= orxFLOAT_0);
  orxASSERT(_fEndTime >= _fStartTime);
  orxSTRUCTURE_ASSERT(_pstFX);
  orxASSERT(_pvStartScale != orxNULL);
  orxASSERT(_pvEndScale != orxNULL);

  /* Finds empty slot index */
  u32Index = orxFX_FindEmptySlotIndex(_pstFX);

  /* Valid? */
  if(u32Index != orxU32_UNDEFINED)
  {
    orxFX_SLOT *pstFXSlot;

    /* Gets the slot */
    pstFXSlot = &(_pstFX->astFXSlotList[u32Index]);

    /* Updates its parameters */
    pstFXSlot->fStartTime     = _fStartTime;
    pstFXSlot->fEndTime       = _fEndTime;
    pstFXSlot->fCyclePeriod   = _fCyclePeriod;
    pstFXSlot->fCyclePhasis   = _fCyclePhasis;
    pstFXSlot->fAmplification = _fAmplification;
    orxVector_Copy(&(pstFXSlot->vStartScale), _pvStartScale);
    orxVector_Copy(&(pstFXSlot->vEndScale), _pvEndScale);
    pstFXSlot->u32Flags       = (_u32Flags & orxFX_SLOT_KU32_MASK_USER_ALL) | _eCurve | (orxFX_TYPE_SCALE << orxFX_SLOT_KU32_SHIFT_TYPE) | orxFX_SLOT_KU32_FLAG_DEFINED;

    /* Is longer than current FX duration? */
    if(_fEndTime > _pstFX->fDuration)
    {
      /* Updates it */
      _pstFX->fDuration = _fEndTime;
    }
  }

  /* Done! */
  return eResult;
}

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
 * @param[in]   _u32Flags       Param flags
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxFX_AddTranslation(orxFX *_pstFX, orxFLOAT _fStartTime, orxFLOAT _fEndTime, orxFLOAT _fCyclePeriod, orxFLOAT _fCyclePhasis, orxFLOAT _fAmplification, orxCONST orxVECTOR *_pvStartPosition, orxCONST orxVECTOR *_pvEndPosition, orxFX_CURVE _eCurve, orxU32 _u32Flags)
{
  orxU32    u32Index;
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(sstFX.u32Flags & orxFX_KU32_STATIC_FLAG_READY);
  orxASSERT((_u32Flags & orxFX_SLOT_KU32_MASK_USER_ALL) == _u32Flags);
  orxASSERT(_eCurve < orxFX_CURVE_NUMBER);
  orxASSERT(_fStartTime >= orxFLOAT_0);
  orxASSERT(_fEndTime >= _fStartTime);
  orxSTRUCTURE_ASSERT(_pstFX);
  orxASSERT(_pvStartPosition != orxNULL);
  orxASSERT(_pvEndPosition != orxNULL);

  /* Finds empty slot index */
  u32Index = orxFX_FindEmptySlotIndex(_pstFX);

  /* Valid? */
  if(u32Index != orxU32_UNDEFINED)
  {
    orxFX_SLOT *pstFXSlot;

    /* Gets the slot */
    pstFXSlot = &(_pstFX->astFXSlotList[u32Index]);

    /* Updates its parameters */
    pstFXSlot->fStartTime     = _fStartTime;
    pstFXSlot->fEndTime       = _fEndTime;
    pstFXSlot->fCyclePeriod   = _fCyclePeriod;
    pstFXSlot->fCyclePhasis   = _fCyclePhasis;
    pstFXSlot->fAmplification = _fAmplification;
    orxVector_Copy(&(pstFXSlot->vStartPosition), _pvStartPosition);
    orxVector_Copy(&(pstFXSlot->vEndPosition), _pvEndPosition);
    pstFXSlot->u32Flags       = (_u32Flags & orxFX_SLOT_KU32_MASK_USER_ALL) | _eCurve | (orxFX_TYPE_TRANSLATION << orxFX_SLOT_KU32_SHIFT_TYPE) | orxFX_SLOT_KU32_FLAG_DEFINED;

    /* Is longer than current FX duration? */
    if(_fEndTime > _pstFX->fDuration)
    {
      /* Updates it */
      _pstFX->fDuration = _fEndTime;
    }
  }

  /* Done! */
  return eResult;
}

/** Gets FX duration
 * @param[in]   _pstFX          Concerned FX
 * @return      orxFLOAT
 */
orxFLOAT orxFASTCALL orxFX_GetDuration(orxCONST orxFX *_pstFX)
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

/** Tests FX name against given one
 * @param[in]   _pstFX          Concerned FX
 * @param[in]   _zName          Name to test
 * @return      orxTRUE if it's FX name, orxFALSE otherwise
 */
orxBOOL orxFASTCALL orxFX_IsName(orxCONST orxFX *_pstFX, orxCONST orxSTRING _zName)
{
  orxBOOL bResult;

  /* Checks */
  orxASSERT(sstFX.u32Flags & orxFX_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstFX);

  /* Updates result */
  bResult = (orxString_ToCRC(_zName) == _pstFX->u32ID) ? orxTRUE : orxFALSE;

  /* Done! */
  return bResult;
}

/** Set FX loop property
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
orxBOOL orxFASTCALL orxFX_IsLooping(orxCONST orxFX *_pstFX)
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
