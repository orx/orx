/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2022 Orx-Project
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

#define orxFX_SLOT_KU32_MASK_BLEND_CURVE        0x0000000F  /**< Blend curve mask */

#define orxFX_SLOT_KU32_MASK_TYPE               0x000000F0  /**< FX type mask */

#define orxFX_SLOT_KU32_MASK_USER_ALL           0x0000FF00  /**< User all mask */

#define orxFX_SLOT_KU32_MASK_ALL                0xFFFFFFFF  /**< All mask */


#define orxFX_SLOT_KU32_SHIFT_TYPE              4


/** Misc defines
 */
#define orxFX_KU32_REFERENCE_TABLE_SIZE         128

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


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

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
  orxFLOAT    fCyclePhase;                      /**< Cycle phase : 16 */
  orxFLOAT    fAmplification;                   /**< Amplification over time : 20 */
  orxFLOAT    fAcceleration;                    /**< Acceleration over time : 24 */
  orxFLOAT    fPow;                             /**< Curve exponent : 28 */

  union
  {
    struct
    {
      orxFLOAT fStartValue;                     /**< Alpha start value : 32 */
      orxFLOAT fEndValue;                       /**< Alpha end value : 36 */
    };                                          /**< Alpha Fade  : 36 */

    struct
    {
      orxVECTOR vStartValue;                    /**< ColorBlend start value : 40 */
      orxVECTOR vEndValue;                      /**< ColorBlend end value : 52 */
    };                                          /** Color blend : 52 */
  };

  orxU32 u32Flags;                              /**< Flags : 56 */

} orxFX_SLOT;

/** FX structure
 */
struct __orxFX_t
{
  orxSTRUCTURE    stStructure;                            /**< Public structure, first structure member : 32 */
  const orxSTRING zReference;                             /**< FX reference : 20 */
  orxFLOAT        fDuration;                              /**< FX duration : 24 */
  orxFLOAT        fOffset;                                /**< FX offset : 28 */
  orxFX_SLOT      astFXSlotList[orxFX_KU32_SLOT_NUMBER];  /**< FX slot list : 476 */
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
static orxFX_STATIC sstFX;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

/** Semi-private, internal-use only forward declarations
 */
orxVECTOR *orxFASTCALL orxConfig_ToVector(const orxSTRING _zValue, orxVECTOR *_pvVector);

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

    /* Creates reference table */
    sstFX.pstReferenceTable = orxHashTable_Create(orxFX_KU32_REFERENCE_TABLE_SIZE, orxHASHTABLE_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

    /* Valid? */
    if(sstFX.pstReferenceTable != orxNULL)
    {
      /* Registers structure type */
      eResult = orxSTRUCTURE_REGISTER(FX, orxSTRUCTURE_STORAGE_TYPE_LINKLIST, orxMEMORY_TYPE_MAIN, orxFX_KU32_BANK_SIZE, orxNULL);

      /* Adds event handler */
      orxEvent_AddHandler(orxEVENT_TYPE_RESOURCE, orxFX_EventHandler);
      orxEvent_SetHandlerIDFlags(orxFX_EventHandler, orxEVENT_TYPE_RESOURCE, orxNULL, orxEVENT_GET_FLAG(orxRESOURCE_EVENT_ADD) | orxEVENT_GET_FLAG(orxRESOURCE_EVENT_UPDATE), orxEVENT_KU32_MASK_ID_ALL);
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Failed to create FX hashtable storage.");
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Tried to initialize the FX module when it was already initialized.");

    /* Already initialized */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Initialized? */
  if(eResult != orxSTATUS_FAILURE)
  {
    /* Inits Flags */
    orxFLAG_SET(sstFX.u32Flags, orxFX_KU32_STATIC_FLAG_READY, orxFX_KU32_STATIC_FLAG_NONE);
  }
  else
  {
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

    /* Updates flags */
    sstFX.u32Flags &= ~orxFX_KU32_STATIC_FLAG_READY;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Tried to exit from the FX module when it wasn't initialized.");
  }

  return;
}

/** Creates an empty FX
 * @return      Created orxFX / orxNULL
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
 * @param[in]   _zConfigID            Config ID
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
 * @param[in] _pstFX            Concerned FX
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
 * @param[in] _pstFX            FX to apply
 * @param[in] _pstObject        Object on which to apply the FX
 * @param[in] _fStartTime       FX local application start time
 * @param[in] _fEndTime         FX local application end time
 * @return    orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxFX_Apply(const orxFX *_pstFX, orxOBJECT *_pstObject, orxFLOAT _fStartTime, orxFLOAT _fEndTime)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxSTRUCTURE_ASSERT(_pstFX);
  orxSTRUCTURE_ASSERT(_pstObject);
  orxASSERT(_fEndTime >= _fStartTime);

  /* Has started? */
  if(_fEndTime >= orxFLOAT_0)
  {
    typedef struct __orxFX_VALUE_t
    {
      union
      {
        orxVECTOR vValue;
        orxFLOAT fValue;
      };
    } orxFX_VALUE;

    orxFX_VALUE astValueList[orxFX_TYPE_NUMBER];
    orxCOLOR    stObjectColor;
    orxU32      u32LockFlags = 0, u32UpdateFlags = 0, i;
    orxFLOAT    fRecDuration;
    orxFX_TYPE  eColorBlendUpdate = orxFX_TYPE_NONE;
    orxBOOL     bFirstCall;

    /* Clears values */
    orxMemory_Zero(astValueList, orxFX_TYPE_NUMBER * sizeof(struct __orxFX_VALUE_t));
    orxVector_SetAll(&(astValueList[orxFX_TYPE_SCALE].vValue), orxFLOAT_1);
    astValueList[orxFX_TYPE_PITCH].fValue = orxFLOAT_1;

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

    /* For all slots */
    for(i = 0; i< orxFX_KU32_SLOT_NUMBER; i++)
    {
      const orxFX_SLOT *pstFXSlot;

      /* Gets the slot */
      pstFXSlot = &(_pstFX->astFXSlotList[i]);

      /* Is defined? */
      if(orxFLAG_TEST(pstFXSlot->u32Flags, orxFX_SLOT_KU32_FLAG_DEFINED))
      {
        orxFLOAT fStartTime;

/* Some versions of GCC have an optimization bug on fEndTime which leads to a bogus value when reaching the end of a slot */
#if defined(__orxGCC__)
        volatile orxFLOAT fEndTime;
#else /* __orxGCC__  */
        orxFLOAT fEndTime;
#endif /* __orxGCC__ */

        /* Gets corrected start and end time */
        fStartTime  = orxMAX(_fStartTime, pstFXSlot->fStartTime);
        fEndTime    = orxMIN(_fEndTime, pstFXSlot->fEndTime);

        /* Updates first call status */
        bFirstCall = (fStartTime == pstFXSlot->fStartTime) ? orxTRUE : orxFALSE;

        /* Is this slot active in the time period? */
        if(fEndTime >= fStartTime)
        {
          orxFX_TYPE eFXType;

          /* Gets slot local time stamps */
          fStartTime -= pstFXSlot->fStartTime;
          fEndTime   -= pstFXSlot->fStartTime;

          /* Gets reciprocal duration */
          fRecDuration = pstFXSlot->fEndTime - pstFXSlot->fStartTime;
          fRecDuration = (fRecDuration > orxFLOAT_0) ? orxFLOAT_1 / fRecDuration : orxFLOAT_1;

          /* Has acceleration? */
          if(orxFLAG_TEST(pstFXSlot->u32Flags, orxFX_SLOT_KU32_FLAG_ACCELERATION))
          {
            orxFLOAT fStartAcceleration, fEndAcceleration;

            /* Gets acceleration coefs */
            fStartAcceleration  = orxLERP(orxFLOAT_1, pstFXSlot->fAcceleration, fStartTime * fRecDuration);
            fEndAcceleration    = orxLERP(orxFLOAT_1, pstFXSlot->fAcceleration, fEndTime * fRecDuration);

            /* Updates the times */
            fStartTime *= fStartAcceleration;
            fEndTime   *= fEndAcceleration;

            /* Updates reciprocal duration */
            fRecDuration = (pstFXSlot->fEndTime - pstFXSlot->fStartTime);
            fRecDuration = (fRecDuration > orxFLOAT_0) ? orxFLOAT_1 / (fRecDuration * pstFXSlot->fAcceleration) : orxFLOAT_1;
          }

          /* Gets FX type */
          eFXType = orxFX_GetSlotType(pstFXSlot);

          /* Is FX type not blocked? */
          if(!orxFLAG_TEST(u32LockFlags, (1 << eFXType)))
          {
            orxFLOAT fPeriod, fFrequency, fStartCoef, fEndCoef;

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

            /* Depending on blend curve */
            switch(pstFXSlot->u32Flags & orxFX_SLOT_KU32_MASK_BLEND_CURVE)
            {
              case orxFX_CURVE_LINEAR:
              {
                /* Gets linear start coef in period [0.0; 1.0] starting at given phase */
                fStartCoef = (fStartTime * fFrequency) + pstFXSlot->fCyclePhase;

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

                /* Gets linear end coef in period [0.0; 1.0] starting at given phase */
                fEndCoef = (fEndTime * fFrequency) + pstFXSlot->fCyclePhase;

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

              case orxFX_CURVE_SMOOTH:
              {
                /* Gets linear start coef in period [0.0; 1.0] starting at given phase */
                fStartCoef = (fStartTime * fFrequency) + pstFXSlot->fCyclePhase;

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
                  else
                  {
                    /* Gets smoothed value */
                    fStartCoef = (fStartCoef * fStartCoef) * (orx2F(3.0f) - (orx2F(2.0f) * fStartCoef));
                  }
                }

                /* Gets linear end coef in period [0.0; 1.0] starting at given phase */
                fEndCoef = (fEndTime * fFrequency) + pstFXSlot->fCyclePhase;

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
                  else
                  {
                    /* Gets smoothed value */
                    fEndCoef = (fEndCoef * fEndCoef) * (orx2F(3.0f) - (orx2F(2.0f) * fEndCoef));
                  }
                }

                break;
              }

              case orxFX_CURVE_SMOOTHER:
              {
                /* Gets linear start coef in period [0.0; 1.0] starting at given phase */
                fStartCoef = (fStartTime * fFrequency) + pstFXSlot->fCyclePhase;

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
                  else
                  {
                    /* Gets smoothed value */
                    fStartCoef = (fStartCoef * fStartCoef * fStartCoef) * (fStartCoef * ((fStartCoef * orx2F(6.0f)) - orx2F(15.0f)) + orx2F(10.0f));
                  }
                }

                /* Gets linear end coef in period [0.0; 1.0] starting at given phase */
                fEndCoef = (fEndTime * fFrequency) + pstFXSlot->fCyclePhase;

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
                  else
                  {
                    /* Gets smoothed value */
                    fEndCoef = (fEndCoef * fEndCoef * fEndCoef) * (fEndCoef * ((fEndCoef * orx2F(6.0f)) - orx2F(15.0f)) + orx2F(10.0f));
                  }
                }

                break;
              }

              case orxFX_CURVE_TRIANGLE:
              {
                /* Gets linear coef in period [0.0; 2.0] starting at given phase */
                fStartCoef = (fStartTime * fFrequency) + pstFXSlot->fCyclePhase;
                fStartCoef = orxMath_Mod(fStartCoef * orx2F(2.0f), orx2F(2.0f));

                /* Gets symetric coef between 1.0 & 2.0 */
                if(fStartCoef > orxFLOAT_1)
                {
                  fStartCoef = orx2F(2.0f) - fStartCoef;
                }

                /* Gets linear coef in period [0.0; 2.0] starting at given phase */
                fEndCoef = (fEndTime * fFrequency) + pstFXSlot->fCyclePhase;
                fEndCoef = orxMath_Mod(fEndCoef * orx2F(2.0f), orx2F(2.0f));

                /* Gets symetric coef between 1.0 & 2.0 */
                if(fEndCoef > orxFLOAT_1)
                {
                  fEndCoef = orx2F(2.0f) - fEndCoef;
                }

                break;
              }

              case orxFX_CURVE_SQUARE:
              {
                /* Gets linear start coef in period [0.0; 1.0] starting at given phase */
                fStartCoef = (fStartTime * fFrequency) + pstFXSlot->fCyclePhase;

                /* Non zero? */
                if(fStartCoef != orxFLOAT_0)
                {
                    /* Gets its modulo */
                    fStartCoef = orxMath_Mod(fStartCoef, orxFLOAT_1);

                    /* High section? */
                    if((fStartCoef >= orx2F(0.25f))
                    && (fStartCoef < orx2F(0.75f)))
                    {
                        /* Sets it at max value */
                        fStartCoef = orxFLOAT_1;
                    }
                    else
                    {
                        /* Sets it at min value */
                        fStartCoef = orxFLOAT_0;
                    }
                }

                /* Gets linear End coef in period [0.0; 1.0] Ending at given phase */
                fEndCoef = (fEndTime * fFrequency) + pstFXSlot->fCyclePhase;

                /* Non zero? */
                if(fEndCoef != orxFLOAT_0)
                {
                    /* Gets its modulo */
                    fEndCoef = orxMath_Mod(fEndCoef, orxFLOAT_1);

                    /* High section? */
                    if((fEndCoef >= orx2F(0.25f))
                    && (fEndCoef < orx2F(0.75f)))
                    {
                        /* Sets it at max value */
                        fEndCoef = orxFLOAT_1;
                    }
                    else
                    {
                        /* Sets it at min value */
                        fEndCoef = orxFLOAT_0;
                    }
                }

                break;
              }

              case orxFX_CURVE_SINE:
              {
                /* Gets sine coef starting at given phase * 2Pi - Pi/2 */
                fStartCoef = (orxMath_Sin((orxMATH_KF_2_PI * (fStartTime + (fPeriod * (pstFXSlot->fCyclePhase - orx2F(0.25f))))) * fFrequency) + orxFLOAT_1) * orx2F(0.5f);

                /* Gets sine coef starting at given phase * 2Pi - Pi/2 */
                fEndCoef = (orxMath_Sin((orxMATH_KF_2_PI * (fEndTime + (fPeriod * (pstFXSlot->fCyclePhase - orx2F(0.25f))))) * fFrequency) + orxFLOAT_1) * orx2F(0.5f);

                break;
              }

              default:
              {
                /* Logs message */
                orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Invalid curve.");

                /* Skips it */
                continue;
              }
            }

            /* Has amplification? */
            if(orxFLAG_TEST(pstFXSlot->u32Flags, orxFX_SLOT_KU32_FLAG_AMPLIFICATION))
            {
              orxFLOAT fStartAmplification, fEndAmplification;

              /* Gets amplification coefs */
              fStartAmplification = orxLERP(orxFLOAT_1, pstFXSlot->fAmplification, fStartTime * fRecDuration);
              fEndAmplification   = orxLERP(orxFLOAT_1, pstFXSlot->fAmplification, fEndTime * fRecDuration);

              /* Updates the coefs */
              fStartCoef *= fStartAmplification;
              fEndCoef   *= fEndAmplification;
            }

            /* Using an exponential curve? */
            if(orxFLAG_TEST(pstFXSlot->u32Flags, orxFX_SLOT_KU32_FLAG_POW))
            {
              /* Updates both coefs */
              fStartCoef = orxMath_Pow(fStartCoef, pstFXSlot->fPow);
              fEndCoef   = orxMath_Pow(fEndCoef, pstFXSlot->fPow);
            }

            /* Depending on FX type */
            switch(eFXType)
            {
              case orxFX_TYPE_ALPHA:
              case orxFX_TYPE_ROTATION:
              case orxFX_TYPE_VOLUME:
              {
                /* Absolute ? */
                if(orxFLAG_TEST(pstFXSlot->u32Flags, orxFX_SLOT_KU32_FLAG_ABSOLUTE))
                {
                  /* Overrides value */
                  astValueList[eFXType].fValue = orxLERP(pstFXSlot->fStartValue, pstFXSlot->fEndValue, fEndCoef);

                  /* Locks it */
                  orxFLAG_SET(u32LockFlags, (1 << eFXType), 0);
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
                    fStartValue = orxLERP(pstFXSlot->fStartValue, pstFXSlot->fEndValue, fStartCoef);
                  }

                  /* Gets end value */
                  fEndValue = orxLERP(pstFXSlot->fStartValue, pstFXSlot->fEndValue, fEndCoef);

                  /* Updates global value */
                  astValueList[eFXType].fValue += fEndValue - fStartValue;
                }

                /* Updates status */
                orxFLAG_SET(u32UpdateFlags, (1 << eFXType), 0);

                break;
              }

              case orxFX_TYPE_PITCH:
              {
                /* Absolute ? */
                if(orxFLAG_TEST(pstFXSlot->u32Flags, orxFX_SLOT_KU32_FLAG_ABSOLUTE))
                {
                  /* Overrides value */
                  astValueList[eFXType].fValue = orxLERP(pstFXSlot->fStartValue, pstFXSlot->fEndValue, fEndCoef);
                  if(astValueList[eFXType].fValue == orxFLOAT_0)
                  {
                    astValueList[eFXType].fValue = orx2F(0.000001f);
                  }

                  /* Locks it */
                  orxFLAG_SET(u32LockFlags, (1 << eFXType), 0);
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
                    fStartValue = orxLERP(pstFXSlot->fStartValue, pstFXSlot->fEndValue, fStartCoef);
                    if(fStartValue == orxFLOAT_0)
                    {
                      fStartValue = orx2F(0.000001f);
                    }
                  }

                  /* Gets end value */
                  fEndValue = orxLERP(pstFXSlot->fStartValue, pstFXSlot->fEndValue, fEndCoef);
                  if(fEndValue == orxFLOAT_0)
                  {
                    fEndValue = orx2F(0.000001f);
                  }

                  /* Updates global value */
                  astValueList[eFXType].fValue *= fEndValue / fStartValue;
                }

                /* Updates status */
                orxFLAG_SET(u32UpdateFlags, (1 << eFXType), 0);

                break;
              }

              case orxFX_TYPE_RGB:
              case orxFX_TYPE_HSL:
              case orxFX_TYPE_HSV:
              {
                /* Valid color blend mix? */
                if((eColorBlendUpdate == orxFX_TYPE_NONE) || (eColorBlendUpdate == eFXType))
                {
                  /* Absolute ? */
                  if(orxFLAG_TEST(pstFXSlot->u32Flags, orxFX_SLOT_KU32_FLAG_ABSOLUTE))
                  {
                    /* Overrides values */
                    orxVector_Lerp(&(astValueList[orxFX_TYPE_RGB].vValue), &(pstFXSlot->vStartValue), &(pstFXSlot->vEndValue), fEndCoef);

                    /* Locks it */
                    orxFLAG_SET(u32LockFlags, ((1 << orxFX_TYPE_RGB) | (1 << orxFX_TYPE_HSL) | (1 << orxFX_TYPE_HSV)), 0);
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
                      orxVector_Lerp(&vStartColor, &(pstFXSlot->vStartValue), &(pstFXSlot->vEndValue), fStartCoef);
                    }

                    /* Gets end value */
                    orxVector_Lerp(&vEndColor, &(pstFXSlot->vStartValue), &(pstFXSlot->vEndValue), fEndCoef);

                    /* Updates global color value */
                    orxVector_Add(&(astValueList[orxFX_TYPE_RGB].vValue), &(astValueList[orxFX_TYPE_RGB].vValue), orxVector_Sub(&vEndColor, &vEndColor, &vStartColor));
                  }

                  /* Updates color blend status */
                  eColorBlendUpdate = eFXType;
                }
                else
                {
                  /* Logs message */
                  orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Invalid color blend mix for FX: only one type of color space can be used at once.");
                }

                break;
              }

              case orxFX_TYPE_POSITION:
              case orxFX_TYPE_SPEED:
              {
                /* Absolute ? */
                if(orxFLAG_TEST(pstFXSlot->u32Flags, orxFX_SLOT_KU32_FLAG_ABSOLUTE))
                {
                  /* Overrides values */
                  orxVector_Lerp(&(astValueList[eFXType].vValue), &(pstFXSlot->vStartValue), &(pstFXSlot->vEndValue), fEndCoef);

                  /* Use rotation? */
                  if(orxFLAG_TEST(pstFXSlot->u32Flags, orxFX_SLOT_KU32_FLAG_USE_ROTATION))
                  {
                    /* Updates vector */
                    orxVector_2DRotate(&(astValueList[eFXType].vValue), &(astValueList[eFXType].vValue), orxObject_GetRotation(_pstObject));
                  }

                  /* Use scale? */
                  if(orxFLAG_TEST(pstFXSlot->u32Flags, orxFX_SLOT_KU32_FLAG_USE_SCALE))
                  {
                    orxVECTOR vScale;

                    /* Updates vector */
                    orxVector_Mul(&(astValueList[eFXType].vValue), &(astValueList[eFXType].vValue), orxObject_GetScale(_pstObject, &vScale));
                  }

                  /* Locks it */
                  orxFLAG_SET(u32LockFlags, (1 << eFXType), 0);
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
                    orxVector_Lerp(&vStartValue, &(pstFXSlot->vStartValue), &(pstFXSlot->vEndValue), fStartCoef);
                  }

                  /* Gets end value */
                  orxVector_Lerp(&vEndValue, &(pstFXSlot->vStartValue), &(pstFXSlot->vEndValue), fEndCoef);

                  /* Gets delta value */
                  orxVector_Sub(&vEndValue, &vEndValue, &vStartValue);

                  /* Use rotation? */
                  if(orxFLAG_TEST(pstFXSlot->u32Flags, orxFX_SLOT_KU32_FLAG_USE_ROTATION))
                  {
                    /* Updates vector */
                    orxVector_2DRotate(&vEndValue, &vEndValue, orxObject_GetRotation(_pstObject));
                  }

                  /* Use scale? */
                  if(orxFLAG_TEST(pstFXSlot->u32Flags, orxFX_SLOT_KU32_FLAG_USE_SCALE))
                  {
                    orxVECTOR vScale;

                    /* Updates vector */
                    orxVector_Mul(&vEndValue, &vEndValue, orxObject_GetScale(_pstObject, &vScale));
                  }

                  /* Updates global value */
                  orxVector_Add(&(astValueList[eFXType].vValue), &(astValueList[eFXType].vValue), &vEndValue);
                }

                /* Updates status */
                orxFLAG_SET(u32UpdateFlags, (1 << eFXType), 0);

                break;
              }

              case orxFX_TYPE_SCALE:
              {
                /* Absolute ? */
                if(orxFLAG_TEST(pstFXSlot->u32Flags, orxFX_SLOT_KU32_FLAG_ABSOLUTE))
                {
                  /* Overrides values */
                  orxVector_Lerp(&(astValueList[eFXType].vValue), &(pstFXSlot->vStartValue), &(pstFXSlot->vEndValue), fEndCoef);

                  /* Makes sure we have valid values */
                  if(astValueList[eFXType].vValue.fX == orxFLOAT_0)
                  {
                    astValueList[eFXType].vValue.fX = orx2F(0.000001f);
                  }
                  if(astValueList[eFXType].vValue.fY == orxFLOAT_0)
                  {
                    astValueList[eFXType].vValue.fY = orx2F(0.000001f);
                  }

                  /* Locks it */
                  orxFLAG_SET(u32LockFlags, (1 << eFXType), 0);
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
                    orxVector_Lerp(&vStartScale, &(pstFXSlot->vStartValue), &(pstFXSlot->vEndValue), fStartCoef);

                    /* Makes sure we have valid values */
                    if(vStartScale.fX == orxFLOAT_0)
                    {
                      vStartScale.fX = orx2F(0.000001f);
                    }
                    if(vStartScale.fY == orxFLOAT_0)
                    {
                      vStartScale.fY = orx2F(0.000001f);
                    }
                  }

                  /* Gets end value */
                  orxVector_Lerp(&vEndScale, &(pstFXSlot->vStartValue), &(pstFXSlot->vEndValue), fEndCoef);

                  /* Makes sure we have valid values */
                  if(vEndScale.fX == orxFLOAT_0)
                  {
                    vEndScale.fX = orx2F(0.000001f);
                  }
                  if(vEndScale.fY == orxFLOAT_0)
                  {
                    vEndScale.fY = orx2F(0.000001f);
                  }

                  /* Updates global scale value */
                  orxVector_Mul(&(astValueList[eFXType].vValue), &(astValueList[eFXType].vValue), orxVector_Div(&vEndScale, &vEndScale, &vStartScale));
                }

                /* Updates scale status */
                orxFLAG_SET(u32UpdateFlags, (1 << eFXType), 0);

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
      }
      else
      {
        /* Done with the slots */
        break;
      }
    }

    /* Global color update? */
    if((orxFLAG_TEST(u32UpdateFlags, (1 << orxFX_TYPE_ALPHA)))
    || (eColorBlendUpdate != orxFX_TYPE_NONE))
    {
      orxCOLOR stColor;

      /* Update alpha? */
      if(orxFLAG_TEST(u32UpdateFlags, (1 << orxFX_TYPE_ALPHA)))
      {
        /* Non absolute? */
        if(!orxFLAG_TEST(u32LockFlags, (1 << orxFX_TYPE_ALPHA)))
        {
          /* Updates alpha with previous one */
          stColor.fAlpha = astValueList[orxFX_TYPE_ALPHA].fValue + stObjectColor.fAlpha;
        }
        else
        {
          /* Updates color */
          stColor.fAlpha = astValueList[orxFX_TYPE_ALPHA].fValue;
        }
      }
      else
      {
        /* Resets alpha */
        stColor.fAlpha = stObjectColor.fAlpha;
      }

      /* Update color blend? */
      if(eColorBlendUpdate != orxFX_TYPE_NONE)
      {
        /* Non absolute? */
        if(!orxFLAG_TEST(u32LockFlags, (1 << orxFX_TYPE_RGB)))
        {
          /* HSL? */
          if(eColorBlendUpdate == orxFX_TYPE_HSL)
          {
            /* Gets object's HSL color */
            orxColor_FromRGBToHSL(&stObjectColor, &stObjectColor);

            /* Updates color with previous one */
            orxVector_Add(&(stColor.vHSL), &(astValueList[orxFX_TYPE_RGB].vValue), &(stObjectColor.vHSL));

            /* Applies circular clamp on [0, 1[ */
            stColor.vHSL.fH -= orxS2F(orxF2S(stColor.vHSL.fH) - (orxS32)(stColor.vHSL.fH < orxFLOAT_0));

            /* Gets RGB color */
            orxColor_FromHSLToRGB(&stColor, &stColor);
          }
          /* HSV? */
          else if(eColorBlendUpdate == orxFX_TYPE_HSV)
          {
            /* Gets object's HSV color */
            orxColor_FromRGBToHSV(&stObjectColor, &stObjectColor);

            /* Updates color with previous one */
            orxVector_Add(&(stColor.vHSV), &(astValueList[orxFX_TYPE_RGB].vValue), &(stObjectColor.vHSV));

            /* Applies circular clamp on [0, 1[ */
            stColor.vHSV.fH -= orxS2F(orxF2S(stColor.vHSV.fH) - (orxS32)(stColor.vHSV.fH < orxFLOAT_0));

            /* Gets RGB color */
            orxColor_FromHSVToRGB(&stColor, &stColor);
          }
          /* RGB */
          else
          {
            /* Updates color with previous one */
            orxVector_Add(&(stColor.vRGB), &(astValueList[orxFX_TYPE_RGB].vValue), &(stObjectColor.vRGB));
          }
        }
        else
        {
          /* Copies value */
          orxVector_Copy(&(stColor.vRGB), &(astValueList[orxFX_TYPE_RGB].vValue));

          /* HSL? */
          if(eColorBlendUpdate == orxFX_TYPE_HSL)
          {
            /* Applies circular clamp on [0, 1[ */
            stColor.vHSL.fH -= orxS2F(orxF2S(stColor.vHSL.fH) - (orxS32)(stColor.vHSL.fH < orxFLOAT_0));

            /* Gets RGB color */
            orxColor_FromHSLToRGB(&stColor, &stColor);
          }
          /* HSV? */
          else if(eColorBlendUpdate == orxFX_TYPE_HSV)
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
    if(orxFLAG_TEST(u32UpdateFlags, (1 << orxFX_TYPE_ROTATION)))
    {
      /* Non absolute? */
      if(!orxFLAG_TEST(u32LockFlags, (1 << orxFX_TYPE_ROTATION)))
      {
        /* Updates rotation with previous one */
        astValueList[orxFX_TYPE_ROTATION].fValue += orxObject_GetRotation(_pstObject);
      }

      /* Applies it */
      orxObject_SetRotation(_pstObject, astValueList[orxFX_TYPE_ROTATION].fValue);
    }

    /* Update scale? */
    if(orxFLAG_TEST(u32UpdateFlags, (1 << orxFX_TYPE_SCALE)))
    {
      /* Non absolute? */
      if(!orxFLAG_TEST(u32LockFlags, (1 << orxFX_TYPE_SCALE)))
      {
        orxVECTOR vObjectScale;

        /* Gets object scale */
        orxObject_GetScale(_pstObject, &vObjectScale);

        /* Updates scale with previous one */
        orxVector_Mul(&(astValueList[orxFX_TYPE_SCALE].vValue), &(astValueList[orxFX_TYPE_SCALE].vValue), &vObjectScale);
      }

      /* Applies it */
      orxObject_SetScale(_pstObject, &(astValueList[orxFX_TYPE_SCALE].vValue));
    }

    /* Update translation? */
    if(orxFLAG_TEST(u32UpdateFlags, (1 << orxFX_TYPE_POSITION)))
    {
      /* Non absolute? */
      if(!orxFLAG_TEST(u32LockFlags, (1 << orxFX_TYPE_POSITION)))
      {
        orxVECTOR vObjectPosition;

        /* Updates position with previous one */
        orxVector_Add(&(astValueList[orxFX_TYPE_POSITION].vValue), &(astValueList[orxFX_TYPE_POSITION].vValue), orxObject_GetPosition(_pstObject, &vObjectPosition));
      }

      /* Applies it */
      orxObject_SetPosition(_pstObject, &(astValueList[orxFX_TYPE_POSITION].vValue));
    }

    /* Update speed? */
    if(orxFLAG_TEST(u32UpdateFlags, (1 << orxFX_TYPE_SPEED)))
    {
      /* Non absolute? */
      if(!orxFLAG_TEST(u32LockFlags, (1 << orxFX_TYPE_SPEED)))
      {
        orxVECTOR vObjectSpeed;

        /* Updates position with previous one */
        orxVector_Add(&(astValueList[orxFX_TYPE_SPEED].vValue), &(astValueList[orxFX_TYPE_SPEED].vValue), orxObject_GetSpeed(_pstObject, &vObjectSpeed));
      }

      /* Applies it */
      orxObject_SetSpeed(_pstObject, &(astValueList[orxFX_TYPE_SPEED].vValue));
    }

    /* Update volume? */
    if(orxFLAG_TEST(u32UpdateFlags, (1 << orxFX_TYPE_VOLUME)))
    {
      /* Non absolute? */
      if(!orxFLAG_TEST(u32LockFlags, (1 << orxFX_TYPE_VOLUME)))
      {
        orxSOUND *pstSound;

        /* Gets sounds */
        pstSound = orxObject_GetLastAddedSound(_pstObject);

        /* Valid ? */
        if(pstSound != orxNULL)
        {
          /* Updates volume with previous one */
          astValueList[orxFX_TYPE_VOLUME].fValue += orxSound_GetVolume(pstSound);
        }
      }

      /* Applies it */
      orxObject_SetVolume(_pstObject, astValueList[orxFX_TYPE_VOLUME].fValue);
    }

    /* Update pitch? */
    if(orxFLAG_TEST(u32UpdateFlags, (1 << orxFX_TYPE_PITCH)))
    {
      /* Non absolute? */
      if(!orxFLAG_TEST(u32LockFlags, (1 << orxFX_TYPE_PITCH)))
      {
        orxSOUND *pstSound;

        /* Gets sounds */
        pstSound = orxObject_GetLastAddedSound(_pstObject);

        /* Valid? */
        if(pstSound != orxNULL)
        {
          /* Updates pitch with previous one */
          astValueList[orxFX_TYPE_PITCH].fValue *= orxSound_GetPitch(pstSound);
        }
      }

      /* Applies it */
      orxObject_SetPitch(_pstObject, astValueList[orxFX_TYPE_PITCH].fValue);
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
orxSTATUS orxFASTCALL orxFX_AddAlpha(orxFX *_pstFX, orxFLOAT _fStartTime, orxFLOAT _fEndTime, orxFLOAT _fCyclePeriod, orxFLOAT _fCyclePhase, orxFLOAT _fAmplification, orxFLOAT _fAcceleration, orxFLOAT _fStartAlpha, orxFLOAT _fEndAlpha, orxFX_CURVE _eCurve, orxFLOAT _fPow, orxU32 _u32Flags)
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
    pstFXSlot->fCyclePhase    = _fCyclePhase;
    pstFXSlot->fAmplification = _fAmplification;
    pstFXSlot->fAcceleration  = _fAcceleration;
    pstFXSlot->fPow           = _fPow;
    pstFXSlot->fStartValue    = _fStartAlpha;
    pstFXSlot->fEndValue      = _fEndAlpha;
    pstFXSlot->u32Flags       = (_u32Flags & orxFX_SLOT_KU32_MASK_USER_ALL) | _eCurve | (orxFX_TYPE_ALPHA << orxFX_SLOT_KU32_SHIFT_TYPE) | orxFX_SLOT_KU32_FLAG_DEFINED;
    if(_fAmplification != orxFLOAT_1)
    {
      pstFXSlot->u32Flags |= orxFX_SLOT_KU32_FLAG_AMPLIFICATION;
    }
    if(_fAcceleration != orxFLOAT_1)
    {
      pstFXSlot->u32Flags |= orxFX_SLOT_KU32_FLAG_ACCELERATION;
    }
    if(_fPow != orxFLOAT_1)
    {
        pstFXSlot->u32Flags |= orxFX_SLOT_KU32_FLAG_POW;
    }

    /* Is longer than current FX duration? */
    if(_fEndTime > _pstFX->fDuration)
    {
      /* Updates it */
      _pstFX->fDuration = _fEndTime;
    }

    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Done! */
  return eResult;
}

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
orxSTATUS orxFASTCALL orxFX_AddRGB(orxFX *_pstFX, orxFLOAT _fStartTime, orxFLOAT _fEndTime, orxFLOAT _fCyclePeriod, orxFLOAT _fCyclePhase, orxFLOAT _fAmplification, orxFLOAT _fAcceleration, orxVECTOR *_pvStartColor, orxVECTOR *_pvEndColor, orxFX_CURVE _eCurve, orxFLOAT _fPow, orxU32 _u32Flags)
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
    pstFXSlot->fCyclePhase    = _fCyclePhase;
    pstFXSlot->fAmplification = _fAmplification;
    pstFXSlot->fAcceleration  = _fAcceleration;
    pstFXSlot->fPow           = _fPow;
    orxVector_Copy(&(pstFXSlot->vStartValue), _pvStartColor);
    orxVector_Copy(&(pstFXSlot->vEndValue), _pvEndColor);
    pstFXSlot->u32Flags       = (_u32Flags & orxFX_SLOT_KU32_MASK_USER_ALL) | _eCurve | (orxFX_TYPE_RGB << orxFX_SLOT_KU32_SHIFT_TYPE) | orxFX_SLOT_KU32_FLAG_DEFINED;
    if(_fAmplification != orxFLOAT_1)
    {
        pstFXSlot->u32Flags |= orxFX_SLOT_KU32_FLAG_AMPLIFICATION;
    }
    if(_fAcceleration != orxFLOAT_1)
    {
        pstFXSlot->u32Flags |= orxFX_SLOT_KU32_FLAG_ACCELERATION;
    }
    if(_fPow != orxFLOAT_1)
    {
        pstFXSlot->u32Flags |= orxFX_SLOT_KU32_FLAG_POW;
    }

    /* Is longer than current FX duration? */
    if(_fEndTime > _pstFX->fDuration)
    {
      /* Updates it */
      _pstFX->fDuration = _fEndTime;
    }

    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Done! */
  return eResult;
}

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
orxSTATUS orxFASTCALL orxFX_AddHSL(orxFX *_pstFX, orxFLOAT _fStartTime, orxFLOAT _fEndTime, orxFLOAT _fCyclePeriod, orxFLOAT _fCyclePhase, orxFLOAT _fAmplification, orxFLOAT _fAcceleration, orxVECTOR *_pvStartColor, orxVECTOR *_pvEndColor, orxFX_CURVE _eCurve, orxFLOAT _fPow, orxU32 _u32Flags)
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
    pstFXSlot->fCyclePhase    = _fCyclePhase;
    pstFXSlot->fAmplification = _fAmplification;
    pstFXSlot->fAcceleration  = _fAcceleration;
    pstFXSlot->fPow           = _fPow;
    orxVector_Copy(&(pstFXSlot->vStartValue), _pvStartColor);
    orxVector_Copy(&(pstFXSlot->vEndValue), _pvEndColor);
    pstFXSlot->u32Flags       = (_u32Flags & orxFX_SLOT_KU32_MASK_USER_ALL) | _eCurve | (orxFX_TYPE_HSL << orxFX_SLOT_KU32_SHIFT_TYPE) | orxFX_SLOT_KU32_FLAG_DEFINED;
    if(_fAmplification != orxFLOAT_1)
    {
        pstFXSlot->u32Flags |= orxFX_SLOT_KU32_FLAG_AMPLIFICATION;
    }
    if(_fAcceleration != orxFLOAT_1)
    {
        pstFXSlot->u32Flags |= orxFX_SLOT_KU32_FLAG_ACCELERATION;
    }
    if(_fPow != orxFLOAT_1)
    {
        pstFXSlot->u32Flags |= orxFX_SLOT_KU32_FLAG_POW;
    }

    /* Is longer than current FX duration? */
    if(_fEndTime > _pstFX->fDuration)
    {
      /* Updates it */
      _pstFX->fDuration = _fEndTime;
    }

    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Done! */
  return eResult;
}

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
orxSTATUS orxFASTCALL orxFX_AddHSV(orxFX *_pstFX, orxFLOAT _fStartTime, orxFLOAT _fEndTime, orxFLOAT _fCyclePeriod, orxFLOAT _fCyclePhase, orxFLOAT _fAmplification, orxFLOAT _fAcceleration, orxVECTOR *_pvStartColor, orxVECTOR *_pvEndColor, orxFX_CURVE _eCurve, orxFLOAT _fPow, orxU32 _u32Flags)
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
    pstFXSlot->fCyclePhase    = _fCyclePhase;
    pstFXSlot->fAmplification = _fAmplification;
    pstFXSlot->fAcceleration  = _fAcceleration;
    pstFXSlot->fPow           = _fPow;
    orxVector_Copy(&(pstFXSlot->vStartValue), _pvStartColor);
    orxVector_Copy(&(pstFXSlot->vEndValue), _pvEndColor);
    pstFXSlot->u32Flags       = (_u32Flags & orxFX_SLOT_KU32_MASK_USER_ALL) | _eCurve | (orxFX_TYPE_HSV << orxFX_SLOT_KU32_SHIFT_TYPE) | orxFX_SLOT_KU32_FLAG_DEFINED;
    if(_fAmplification != orxFLOAT_1)
    {
        pstFXSlot->u32Flags |= orxFX_SLOT_KU32_FLAG_AMPLIFICATION;
    }
    if(_fAcceleration != orxFLOAT_1)
    {
        pstFXSlot->u32Flags |= orxFX_SLOT_KU32_FLAG_ACCELERATION;
    }
    if(_fPow != orxFLOAT_1)
    {
        pstFXSlot->u32Flags |= orxFX_SLOT_KU32_FLAG_POW;
    }

    /* Is longer than current FX duration? */
    if(_fEndTime > _pstFX->fDuration)
    {
      /* Updates it */
      _pstFX->fDuration = _fEndTime;
    }

    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Done! */
  return eResult;
}

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
orxSTATUS orxFASTCALL orxFX_AddRotation(orxFX *_pstFX, orxFLOAT _fStartTime, orxFLOAT _fEndTime, orxFLOAT _fCyclePeriod, orxFLOAT _fCyclePhase, orxFLOAT _fAmplification, orxFLOAT _fAcceleration, orxFLOAT _fStartRotation, orxFLOAT _fEndRotation, orxFX_CURVE _eCurve, orxFLOAT _fPow, orxU32 _u32Flags)
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
    pstFXSlot->fCyclePhase    = _fCyclePhase;
    pstFXSlot->fAmplification = _fAmplification;
    pstFXSlot->fAcceleration  = _fAcceleration;
    pstFXSlot->fPow           = _fPow;
    pstFXSlot->fStartValue    = _fStartRotation;
    pstFXSlot->fEndValue      = _fEndRotation;
    pstFXSlot->u32Flags       = (_u32Flags & orxFX_SLOT_KU32_MASK_USER_ALL) | _eCurve | (orxFX_TYPE_ROTATION << orxFX_SLOT_KU32_SHIFT_TYPE) | orxFX_SLOT_KU32_FLAG_DEFINED;
    if(_fAmplification != orxFLOAT_1)
    {
        pstFXSlot->u32Flags |= orxFX_SLOT_KU32_FLAG_AMPLIFICATION;
    }
    if(_fAcceleration != orxFLOAT_1)
    {
        pstFXSlot->u32Flags |= orxFX_SLOT_KU32_FLAG_ACCELERATION;
    }
    if(_fPow != orxFLOAT_1)
    {
        pstFXSlot->u32Flags |= orxFX_SLOT_KU32_FLAG_POW;
    }

    /* Is longer than current FX duration? */
    if(_fEndTime > _pstFX->fDuration)
    {
      /* Updates it */
      _pstFX->fDuration = _fEndTime;
    }

    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Done! */
  return eResult;
}

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
orxSTATUS orxFASTCALL orxFX_AddScale(orxFX *_pstFX, orxFLOAT _fStartTime, orxFLOAT _fEndTime, orxFLOAT _fCyclePeriod, orxFLOAT _fCyclePhase, orxFLOAT _fAmplification, orxFLOAT _fAcceleration, const orxVECTOR *_pvStartScale, const orxVECTOR *_pvEndScale, orxFX_CURVE _eCurve, orxFLOAT _fPow, orxU32 _u32Flags)
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
    pstFXSlot->fCyclePhase    = _fCyclePhase;
    pstFXSlot->fAmplification = _fAmplification;
    pstFXSlot->fAcceleration  = _fAcceleration;
    pstFXSlot->fPow           = _fPow;
    orxVector_Copy(&(pstFXSlot->vStartValue), _pvStartScale);
    orxVector_Copy(&(pstFXSlot->vEndValue), _pvEndScale);
    pstFXSlot->u32Flags       = (_u32Flags & orxFX_SLOT_KU32_MASK_USER_ALL) | _eCurve | (orxFX_TYPE_SCALE << orxFX_SLOT_KU32_SHIFT_TYPE) | orxFX_SLOT_KU32_FLAG_DEFINED;
    if(_fAmplification != orxFLOAT_1)
    {
        pstFXSlot->u32Flags |= orxFX_SLOT_KU32_FLAG_AMPLIFICATION;
    }
    if(_fAcceleration != orxFLOAT_1)
    {
        pstFXSlot->u32Flags |= orxFX_SLOT_KU32_FLAG_ACCELERATION;
    }
    if(_fPow != orxFLOAT_1)
    {
        pstFXSlot->u32Flags |= orxFX_SLOT_KU32_FLAG_POW;
    }

    /* Is longer than current FX duration? */
    if(_fEndTime > _pstFX->fDuration)
    {
      /* Updates it */
      _pstFX->fDuration = _fEndTime;
    }

    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Done! */
  return eResult;
}

/** Adds position to an FX
 * @param[in]   _pstFX          Concerned FX
 * @param[in]   _fStartTime     Time start
 * @param[in]   _fEndTime       Time end
 * @param[in]   _fCyclePeriod   Cycle period
 * @param[in]   _fCyclePhase    Cycle phase (at start)
 * @param[in]   _fAmplification Curve linear amplification over time (1.0 for none)
 * @param[in]   _fAcceleration  Curve linear acceleration over time (1.0 for none)
 * @param[in]   _pvStartPosition Starting position value
 * @param[in]   _pvEndPosition  Ending position value
 * @param[in]   _eCurve         Blending curve type
 * @param[in]   _fPow           Blending curve exponent
 * @param[in]   _u32Flags       Param flags
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxFX_AddPosition(orxFX *_pstFX, orxFLOAT _fStartTime, orxFLOAT _fEndTime, orxFLOAT _fCyclePeriod, orxFLOAT _fCyclePhase, orxFLOAT _fAmplification, orxFLOAT _fAcceleration, const orxVECTOR *_pvStartPosition, const orxVECTOR *_pvEndPosition, orxFX_CURVE _eCurve, orxFLOAT _fPow, orxU32 _u32Flags)
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
    pstFXSlot->fCyclePhase    = _fCyclePhase;
    pstFXSlot->fAmplification = _fAmplification;
    pstFXSlot->fAcceleration  = _fAcceleration;
    pstFXSlot->fPow           = _fPow;
    orxVector_Copy(&(pstFXSlot->vStartValue), _pvStartPosition);
    orxVector_Copy(&(pstFXSlot->vEndValue), _pvEndPosition);
    pstFXSlot->u32Flags       = (_u32Flags & orxFX_SLOT_KU32_MASK_USER_ALL) | _eCurve | (orxFX_TYPE_POSITION << orxFX_SLOT_KU32_SHIFT_TYPE) | orxFX_SLOT_KU32_FLAG_DEFINED;
    if(_fAmplification != orxFLOAT_1)
    {
        pstFXSlot->u32Flags |= orxFX_SLOT_KU32_FLAG_AMPLIFICATION;
    }
    if(_fAcceleration != orxFLOAT_1)
    {
        pstFXSlot->u32Flags |= orxFX_SLOT_KU32_FLAG_ACCELERATION;
    }
    if(_fPow != orxFLOAT_1)
    {
        pstFXSlot->u32Flags |= orxFX_SLOT_KU32_FLAG_POW;
    }

    /* Is longer than current FX duration? */
    if(_fEndTime > _pstFX->fDuration)
    {
      /* Updates it */
      _pstFX->fDuration = _fEndTime;
    }

    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Done! */
  return eResult;
}

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
orxSTATUS orxFASTCALL orxFX_AddSpeed(orxFX *_pstFX, orxFLOAT _fStartTime, orxFLOAT _fEndTime, orxFLOAT _fCyclePeriod, orxFLOAT _fCyclePhase, orxFLOAT _fAmplification, orxFLOAT _fAcceleration, const orxVECTOR *_pvStartSpeed, const orxVECTOR *_pvEndSpeed, orxFX_CURVE _eCurve, orxFLOAT _fPow, orxU32 _u32Flags)
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
  orxASSERT(_pvStartSpeed!= orxNULL);
  orxASSERT(_pvEndSpeed!= orxNULL);

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
    pstFXSlot->fCyclePhase    = _fCyclePhase;
    pstFXSlot->fAmplification = _fAmplification;
    pstFXSlot->fAcceleration  = _fAcceleration;
    pstFXSlot->fPow           = _fPow;
    orxVector_Copy(&(pstFXSlot->vStartValue), _pvStartSpeed);
    orxVector_Copy(&(pstFXSlot->vEndValue), _pvEndSpeed);
    pstFXSlot->u32Flags       = (_u32Flags & orxFX_SLOT_KU32_MASK_USER_ALL) | _eCurve | (orxFX_TYPE_SPEED << orxFX_SLOT_KU32_SHIFT_TYPE) | orxFX_SLOT_KU32_FLAG_DEFINED;
    if(_fAmplification != orxFLOAT_1)
    {
        pstFXSlot->u32Flags |= orxFX_SLOT_KU32_FLAG_AMPLIFICATION;
    }
    if(_fAcceleration != orxFLOAT_1)
    {
        pstFXSlot->u32Flags |= orxFX_SLOT_KU32_FLAG_ACCELERATION;
    }
    if(_fPow != orxFLOAT_1)
    {
        pstFXSlot->u32Flags |= orxFX_SLOT_KU32_FLAG_POW;
    }

    /* Is longer than current FX duration? */
    if(_fEndTime > _pstFX->fDuration)
    {
      /* Updates it */
      _pstFX->fDuration = _fEndTime;
    }

    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Done! */
  return eResult;
}

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
orxSTATUS orxFASTCALL orxFX_AddVolume(orxFX *_pstFX, orxFLOAT _fStartTime, orxFLOAT _fEndTime, orxFLOAT _fCyclePeriod, orxFLOAT _fCyclePhase, orxFLOAT _fAmplification, orxFLOAT _fAcceleration, orxFLOAT _fStartVolume, orxFLOAT _fEndVolume, orxFX_CURVE _eCurve, orxFLOAT _fPow, orxU32 _u32Flags)
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
    pstFXSlot->fCyclePhase    = _fCyclePhase;
    pstFXSlot->fAmplification = _fAmplification;
    pstFXSlot->fAcceleration  = _fAcceleration;
    pstFXSlot->fPow           = _fPow;
    pstFXSlot->fStartValue    = _fStartVolume;
    pstFXSlot->fEndValue      = _fEndVolume;
    pstFXSlot->u32Flags       = (_u32Flags & orxFX_SLOT_KU32_MASK_USER_ALL) | _eCurve | (orxFX_TYPE_VOLUME << orxFX_SLOT_KU32_SHIFT_TYPE) | orxFX_SLOT_KU32_FLAG_DEFINED;
    if(_fAmplification != orxFLOAT_1)
    {
        pstFXSlot->u32Flags |= orxFX_SLOT_KU32_FLAG_AMPLIFICATION;
    }
    if(_fAcceleration != orxFLOAT_1)
    {
        pstFXSlot->u32Flags |= orxFX_SLOT_KU32_FLAG_ACCELERATION;
    }
    if(_fPow != orxFLOAT_1)
    {
        pstFXSlot->u32Flags |= orxFX_SLOT_KU32_FLAG_POW;
    }

    /* Is longer than current FX duration? */
    if(_fEndTime > _pstFX->fDuration)
    {
      /* Updates it */
      _pstFX->fDuration = _fEndTime;
    }

    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Done! */
  return eResult;
}

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
orxSTATUS orxFASTCALL orxFX_AddPitch(orxFX *_pstFX, orxFLOAT _fStartTime, orxFLOAT _fEndTime, orxFLOAT _fCyclePeriod, orxFLOAT _fCyclePhase, orxFLOAT _fAmplification, orxFLOAT _fAcceleration, orxFLOAT _fStartPitch, orxFLOAT _fEndPitch, orxFX_CURVE _eCurve, orxFLOAT _fPow, orxU32 _u32Flags)
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
    pstFXSlot->fCyclePhase    = _fCyclePhase;
    pstFXSlot->fAmplification = _fAmplification;
    pstFXSlot->fAcceleration  = _fAcceleration;
    pstFXSlot->fPow           = _fPow;
    pstFXSlot->fStartValue    = _fStartPitch;
    pstFXSlot->fEndValue      = _fEndPitch;
    pstFXSlot->u32Flags       = (_u32Flags & orxFX_SLOT_KU32_MASK_USER_ALL) | _eCurve | (orxFX_TYPE_PITCH << orxFX_SLOT_KU32_SHIFT_TYPE) | orxFX_SLOT_KU32_FLAG_DEFINED;
    if(_fAmplification != orxFLOAT_1)
    {
        pstFXSlot->u32Flags |= orxFX_SLOT_KU32_FLAG_AMPLIFICATION;
    }
    if(_fAcceleration != orxFLOAT_1)
    {
        pstFXSlot->u32Flags |= orxFX_SLOT_KU32_FLAG_ACCELERATION;
    }
    if(_fPow != orxFLOAT_1)
    {
        pstFXSlot->u32Flags |= orxFX_SLOT_KU32_FLAG_POW;
    }

    /* Is longer than current FX duration? */
    if(_fEndTime > _pstFX->fDuration)
    {
      /* Updates it */
      _pstFX->fDuration = _fEndTime;
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
  orxSTATUS eResult;

  /* Checks */
  orxSTRUCTURE_ASSERT(_pstFX);
  orxASSERT((_zSlotID != orxNULL) && (_zSlotID != orxSTRING_EMPTY));

  /* Checks */
  orxSTRUCTURE_ASSERT(_pstFX);

  /* Pushes section */
  if((orxConfig_HasSection(_zSlotID) != orxFALSE)
  && (orxConfig_PushSection(_zSlotID) != orxSTATUS_FAILURE))
  {
    const orxSTRING zCurveType;
    const orxSTRING zType;
    orxFX_CURVE     eCurve;
    orxFLOAT        fStartTime, fEndTime, fCyclePeriod, fCyclePhase, fAmplification, fAcceleration, fPow;
    orxU32          u32Flags = 0;

    /* Gets its curve type */
    zCurveType = orxConfig_GetString(orxFX_KZ_CONFIG_CURVE);

    /* Linear curve? */
    if(orxString_ICompare(zCurveType, orxFX_KZ_LINEAR) == 0)
    {
      /* Updates its curve */
      eCurve = orxFX_CURVE_LINEAR;
    }
    /* Triangle curve? */
    else if(orxString_ICompare(zCurveType, orxFX_KZ_TRIANGLE) == 0)
    {
      /* Updates its curve */
      eCurve = orxFX_CURVE_TRIANGLE;
    }
    /* Square curve? */
    else if(orxString_ICompare(zCurveType, orxFX_KZ_SQUARE) == 0)
    {
      /* Updates its curve */
      eCurve = orxFX_CURVE_SQUARE;
    }
    /* Sine curve? */
    else if(orxString_ICompare(zCurveType, orxFX_KZ_SINE) == 0)
    {
      /* Updates its curve */
      eCurve = orxFX_CURVE_SINE;
    }
    /* Smoother curve? */
    else if(orxString_ICompare(zCurveType, orxFX_KZ_SMOOTHER) == 0)
    {
      /* Updates its curve */
      eCurve = orxFX_CURVE_SMOOTHER;
    }
    /* Smooth curve? */
    else if(orxString_ICompare(zCurveType, orxFX_KZ_SMOOTH) == 0)
    {
      /* Updates its curve */
      eCurve = orxFX_CURVE_SMOOTH;
    }
    /* Defaults to linear */
    else
    {
      /* Updates its curve */
      eCurve = orxFX_CURVE_LINEAR;
    }

    /* Gets its start & end time */
    fStartTime  = orxConfig_GetFloat(orxFX_KZ_CONFIG_START_TIME);
    fEndTime    = orxConfig_GetFloat(orxFX_KZ_CONFIG_END_TIME);

    /* Should stagger? */
    if(orxConfig_GetBool(orxFX_KZ_CONFIG_STAGGER) != orxFALSE)
    {
      /* Updates times */
      fStartTime += _pstFX->fDuration;
      fEndTime   += _pstFX->fDuration;
    }

    /* Gets its cycle period */
    fCyclePeriod = orxConfig_GetFloat(orxFX_KZ_CONFIG_PERIOD);

    /* Gets it cycle phase and convert it from degrees to radians */
    fCyclePhase = orxConfig_GetFloat(orxFX_KZ_CONFIG_PHASE);

    /* Gets its amplification */
    fAmplification = orxConfig_HasValue(orxFX_KZ_CONFIG_AMPLIFICATION) ? orxConfig_GetFloat(orxFX_KZ_CONFIG_AMPLIFICATION) : orxFLOAT_1;

    /* Gets its acceleration */
    fAcceleration = orxConfig_HasValue(orxFX_KZ_CONFIG_ACCELERATION) ? orxConfig_GetFloat(orxFX_KZ_CONFIG_ACCELERATION) : orxFLOAT_1;

    /* Is absolute? */
    if(orxConfig_GetBool(orxFX_KZ_CONFIG_ABSOLUTE) != orxFALSE)
    {
      /* Updates flags */
      u32Flags |= orxFX_SLOT_KU32_FLAG_ABSOLUTE;
    }

    /* Gets exponent? */
    fPow = orxConfig_HasValue(orxFX_KZ_CONFIG_POW) ? orxConfig_GetFloat(orxFX_KZ_CONFIG_POW) : orxFLOAT_1;

    /* Gets its type */
    zType = orxConfig_GetString(orxFX_KZ_CONFIG_TYPE);

    /* Alpha fade? */
    if(orxString_ICompare(zType, orxFX_KZ_ALPHA) == 0)
    {
      orxFLOAT fStartAlpha, fEndAlpha;

      /* Gets alpha values */
      fStartAlpha = orxConfig_GetFloat(orxFX_KZ_CONFIG_START_VALUE);
      fEndAlpha   = orxConfig_GetFloat(orxFX_KZ_CONFIG_END_VALUE);

      /* Adds alpha slot */
      eResult = orxFX_AddAlpha(_pstFX, fStartTime, fEndTime, fCyclePeriod, fCyclePhase, fAmplification, fAcceleration, fStartAlpha, fEndAlpha, eCurve, fPow, u32Flags);
    }
    /* Color blend? */
    else if(orxString_ICompare(zType, orxFX_KZ_COLOR) == 0)
    {
      orxVECTOR       vStartColor, vEndColor;
      const orxSTRING zColor;

      /* Gets start color literal */
      zColor = orxConfig_GetString(orxFX_KZ_CONFIG_START_VALUE);

      /* Not a vector value? */
      if(orxConfig_ToVector(zColor, &vStartColor) == orxNULL)
      {
        /* Not empty? */
        if(zColor != orxSTRING_EMPTY)
        {
          /* Pushes color section */
          orxConfig_PushSection(orxCOLOR_KZ_CONFIG_SECTION);

          /* Retrieves its value */
          orxConfig_GetVector(zColor, &vStartColor);

          /* Pops config section */
          orxConfig_PopSection();
        }
        else
        {
          /* Defaults to black */
          orxVector_SetAll(&vStartColor, orxFLOAT_0);
        }
      }

      /* Gets end color literal */
      zColor = orxConfig_GetString(orxFX_KZ_CONFIG_END_VALUE);

      /* Not a vector value? */
      if(orxConfig_ToVector(zColor, &vEndColor) == orxNULL)
      {
        /* Not empty? */
        if(zColor != orxSTRING_EMPTY)
        {
          /* Pushes color section */
          orxConfig_PushSection(orxCOLOR_KZ_CONFIG_SECTION);

          /* Retrieves its value */
          orxConfig_GetVector(zColor, &vEndColor);

          /* Pops config section */
          orxConfig_PopSection();
        }
        else
        {
          /* Defaults to black */
          orxVector_SetAll(&vEndColor, orxFLOAT_0);
        }
      }

      /* Normalizes them */
      orxVector_Mulf(&vStartColor, &vStartColor, orxCOLOR_NORMALIZER);
      orxVector_Mulf(&vEndColor, &vEndColor, orxCOLOR_NORMALIZER);

      /* Adds RGB slot */
      eResult = orxFX_AddRGB(_pstFX, fStartTime, fEndTime, fCyclePeriod, fCyclePhase, fAmplification, fAcceleration, &vStartColor, &vEndColor, eCurve, fPow, u32Flags);
    }
    /* RGB blend? */
    else if(orxString_ICompare(zType, orxFX_KZ_RGB) == 0)
    {
      orxVECTOR vStartColor, vEndColor;

      /* Gets color values */
      if(orxConfig_GetVector(orxFX_KZ_CONFIG_START_VALUE, &vStartColor) == orxNULL)
      {
        orxVector_SetAll(&vStartColor, orxFLOAT_0);
      }
      if(orxConfig_GetVector(orxFX_KZ_CONFIG_END_VALUE, &vEndColor) == orxNULL)
      {
        orxVector_SetAll(&vEndColor, orxFLOAT_0);
      }

      /* Adds RGB slot */
      eResult = orxFX_AddRGB(_pstFX, fStartTime, fEndTime, fCyclePeriod, fCyclePhase, fAmplification, fAcceleration, &vStartColor, &vEndColor, eCurve, fPow, u32Flags);
    }
    /* HSL blend? */
    else if(orxString_ICompare(zType, orxFX_KZ_HSL) == 0)
    {
      orxVECTOR vStartColor, vEndColor;

      /* Gets color values */
      if(orxConfig_GetVector(orxFX_KZ_CONFIG_START_VALUE, &vStartColor) == orxNULL)
      {
        orxVector_SetAll(&vStartColor, orxFLOAT_0);
      }
      if(orxConfig_GetVector(orxFX_KZ_CONFIG_END_VALUE, &vEndColor) == orxNULL)
      {
        orxVector_SetAll(&vEndColor, orxFLOAT_0);
      }

      /* Adds HSL slot */
      eResult = orxFX_AddHSL(_pstFX, fStartTime, fEndTime, fCyclePeriod, fCyclePhase, fAmplification, fAcceleration, &vStartColor, &vEndColor, eCurve, fPow, u32Flags);
    }
    /* HSV blend? */
    else if(orxString_ICompare(zType, orxFX_KZ_HSV) == 0)
    {
      orxVECTOR vStartColor, vEndColor;

      /* Gets color values */
      if(orxConfig_GetVector(orxFX_KZ_CONFIG_START_VALUE, &vStartColor) == orxNULL)
      {
        orxVector_SetAll(&vStartColor, orxFLOAT_0);
      }
      if(orxConfig_GetVector(orxFX_KZ_CONFIG_END_VALUE, &vEndColor) == orxNULL)
      {
        orxVector_SetAll(&vEndColor, orxFLOAT_0);
      }

      /* Adds RGB slot */
      eResult = orxFX_AddHSV(_pstFX, fStartTime, fEndTime, fCyclePeriod, fCyclePhase, fAmplification, fAcceleration, &vStartColor, &vEndColor, eCurve, fPow, u32Flags);
    }
    /* Rotation? */
    else if(orxString_ICompare(zType, orxFX_KZ_ROTATION) == 0)
    {
      orxFLOAT fStartRotation, fEndRotation;

      /* Gets rotation values */
      fStartRotation  = orxConfig_GetFloat(orxFX_KZ_CONFIG_START_VALUE);
      fEndRotation    = orxConfig_GetFloat(orxFX_KZ_CONFIG_END_VALUE);

      /* Adds rotation slot */
      eResult = orxFX_AddRotation(_pstFX, fStartTime, fEndTime, fCyclePeriod, fCyclePhase, fAmplification, fAcceleration, orxMATH_KF_DEG_TO_RAD * fStartRotation, orxMATH_KF_DEG_TO_RAD * fEndRotation, eCurve, fPow, u32Flags);
    }
    /* Scale? */
    else if(orxString_ICompare(zType, orxFX_KZ_SCALE) == 0)
    {
      orxVECTOR vStartScale, vEndScale;

      /* Is config start scale not a vector? */
      if(orxConfig_GetVector(orxFX_KZ_CONFIG_START_VALUE, &vStartScale) == orxNULL)
      {
        orxFLOAT fScale;

        /* Gets config uniformed scale */
        fScale = (orxConfig_HasValue(orxFX_KZ_CONFIG_START_VALUE) != orxFALSE) ? orxConfig_GetFloat(orxFX_KZ_CONFIG_START_VALUE) : orxFLOAT_1;

        /* Updates vector */
        orxVector_SetAll(&vStartScale, fScale);
      }

      /* Is config end scale not a vector? */
      if(orxConfig_GetVector(orxFX_KZ_CONFIG_END_VALUE, &vEndScale) == orxNULL)
      {
        orxFLOAT fScale;

        /* Gets config uniformed scale */
        fScale = (orxConfig_HasValue(orxFX_KZ_CONFIG_END_VALUE) != orxFALSE) ? orxConfig_GetFloat(orxFX_KZ_CONFIG_END_VALUE) : orxFLOAT_1;

        /* Updates vector */
        orxVector_SetAll(&vEndScale, fScale);
      }

      /* Adds scale slot */
      eResult = orxFX_AddScale(_pstFX, fStartTime, fEndTime, fCyclePeriod, fCyclePhase, fAmplification, fAcceleration, &vStartScale, &vEndScale, eCurve, fPow, u32Flags);
    }
    /* Position? */
    else if(orxString_ICompare(zType, orxFX_KZ_POSITION) == 0)
    {
      orxVECTOR vStartPosition, vEndPosition;
      orxU32    u32LocalFlags;

      /* Gets position values */
      if(orxConfig_GetVector(orxFX_KZ_CONFIG_START_VALUE, &vStartPosition) == orxNULL)
      {
        orxVector_SetAll(&vStartPosition, orxFLOAT_0);
      }
      if(orxConfig_GetVector(orxFX_KZ_CONFIG_END_VALUE, &vEndPosition) == orxNULL)
      {
        orxVector_SetAll(&vEndPosition, orxFLOAT_0);
      }

      /* Use rotation? */
      if(orxConfig_GetBool(orxFX_KZ_CONFIG_USE_ROTATION) != orxFALSE)
      {
        /* Updates local flags */
        u32LocalFlags = orxFX_SLOT_KU32_FLAG_USE_ROTATION;
      }
      else
      {
        /* Updates local flags */
        u32LocalFlags = orxFX_SLOT_KU32_FLAG_NONE;
      }

      /* Use scale? */
      if(orxConfig_GetBool(orxFX_KZ_CONFIG_USE_SCALE) != orxFALSE)
      {
        /* Updates local flags */
        u32LocalFlags |= orxFX_SLOT_KU32_FLAG_USE_SCALE;
      }

      /* Adds position slot */
      eResult = orxFX_AddPosition(_pstFX, fStartTime, fEndTime, fCyclePeriod, fCyclePhase, fAmplification, fAcceleration, &vStartPosition, &vEndPosition, eCurve, fPow, u32Flags | u32LocalFlags);
    }
    /* Speed? */
    else if(orxString_ICompare(zType, orxFX_KZ_SPEED) == 0)
    {
      orxVECTOR vStartSpeed, vEndSpeed;
      orxU32    u32LocalFlags;

      /* Gets speed values */
      if(orxConfig_GetVector(orxFX_KZ_CONFIG_START_VALUE, &vStartSpeed) == orxNULL)
      {
        orxVector_SetAll(&vStartSpeed, orxFLOAT_0);
      }
      if(orxConfig_GetVector(orxFX_KZ_CONFIG_END_VALUE, &vEndSpeed) == orxNULL)
      {
        orxVector_SetAll(&vEndSpeed, orxFLOAT_0);
      }

      /* Use rotation? */
      if(orxConfig_GetBool(orxFX_KZ_CONFIG_USE_ROTATION) != orxFALSE)
      {
        /* Updates local flags */
        u32LocalFlags = orxFX_SLOT_KU32_FLAG_USE_ROTATION;
      }
      else
      {
        /* Updates local flags */
        u32LocalFlags = orxFX_SLOT_KU32_FLAG_NONE;
      }

      /* Use scale? */
      if(orxConfig_GetBool(orxFX_KZ_CONFIG_USE_SCALE) != orxFALSE)
      {
        /* Updates local flags */
        u32LocalFlags |= orxFX_SLOT_KU32_FLAG_USE_SCALE;
      }

      /* Adds speed slot */
      eResult = orxFX_AddSpeed(_pstFX, fStartTime, fEndTime, fCyclePeriod, fCyclePhase, fAmplification, fAcceleration, &vStartSpeed, &vEndSpeed, eCurve, fPow, u32Flags | u32LocalFlags);
    }
    /* Volume? */
    else if(orxString_ICompare(zType, orxFX_KZ_VOLUME) == 0)
    {
      orxFLOAT fStartVolume, fEndVolume;

      /* Gets volume values */
      fStartVolume  = orxConfig_GetFloat(orxFX_KZ_CONFIG_START_VALUE);
      fEndVolume    = orxConfig_GetFloat(orxFX_KZ_CONFIG_END_VALUE);

      /* Adds volume slot */
      eResult = orxFX_AddVolume(_pstFX, fStartTime, fEndTime, fCyclePeriod, fCyclePhase, fAmplification, fAcceleration, fStartVolume, fEndVolume, eCurve, fPow, u32Flags);
    }
    /* Pitch? */
    else if(orxString_ICompare(zType, orxFX_KZ_PITCH) == 0)
    {
      orxFLOAT fStartPitch, fEndPitch;

      /* Gets volume values */
      fStartPitch = (orxConfig_HasValue(orxFX_KZ_CONFIG_START_VALUE) != orxFALSE) ? orxConfig_GetFloat(orxFX_KZ_CONFIG_START_VALUE) : orxFLOAT_1;
      fEndPitch   = (orxConfig_HasValue(orxFX_KZ_CONFIG_END_VALUE) != orxFALSE) ? orxConfig_GetFloat(orxFX_KZ_CONFIG_END_VALUE) : orxFLOAT_1;

      /* Adds pitch slot */
      eResult = orxFX_AddPitch(_pstFX, fStartTime, fEndTime, fCyclePeriod, fCyclePhase, fAmplification, fAcceleration, fStartPitch, fEndPitch, eCurve, fPow, u32Flags);
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Couldn't add slot [%s]: invalid Type found: <%s>.", _zSlotID, zType);

      /* Updates result */
      eResult = orxSTATUS_FAILURE;
    }

    /* Pops previous section */
    orxConfig_PopSection();
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Couldn't add slot: config section [%s] is missing.", _zSlotID);

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
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
