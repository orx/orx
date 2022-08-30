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
 * @file orxAnim.h
 * @date 11/02/2004
 * @author iarwain@orx-project.org
 *
 */


#include "anim/orxAnim.h"

#include "debug/orxDebug.h"
#include "debug/orxProfiler.h"
#include "core/orxConfig.h"
#include "display/orxGraphic.h"
#include "memory/orxMemory.h"
#include "memory/orxBank.h"
#include "utils/orxString.h"


/** Module flags
 */
#define orxANIM_KU32_STATIC_FLAG_NONE       0x00000000  /**< No flags */

#define orxANIM_KU32_STATIC_FLAG_READY      0x00000001  /**< Ready flag */

#define orxANIM_KU32_STATIC_MASK_ALL        0xFFFFFFFF  /**< All mask */


/** orxANIM flags
 */
#define orxANIM_KU32_FLAG_INTERNAL          0x00000010  /**< Internal structure handling flag  */


/** Misc defines
 */
#define orxANIM_KZ_CONFIG_KEY_DATA          "KeyData"
#define orxANIM_KZ_CONFIG_KEY_DURATION      "KeyDuration"
#define orxANIM_KZ_CONFIG_KEY_EVENT_NAME    "KeyEventName"
#define orxANIM_KZ_CONFIG_KEY_EVENT_VALUE   "KeyEventValue"
#define orxANIM_KZ_CONFIG_DEFAULT_DURATION  "DefaultKeyDuration"

#define orxANIM_KU32_BANK_SIZE              128         /**< Bank size */


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Animation key structure
 */
typedef struct __orxANIM_KEY_t
{
  orxSTRUCTURE *pstData;                    /**< Data key : 4 */
  orxFLOAT      fTimeStamp;                 /**< Data timestamp : 8 */

} orxANIM_KEY;

/** Animation structure
 */
struct __orxANIM_t
{
  orxSTRUCTURE          stStructure;        /**< Public structure, first structure member : 32 */
  const orxSTRING       zName;              /**< Anim name : 20 */
  orxU16                u16KeySize;         /**< Key size : 22 */
  orxU16                u16KeyCount;        /**< Key count : 24 */
  orxU16                u16EventSize;       /**< Event size : 26 */
  orxU16                u16EventCount;      /**< Event count : 28 */
  orxANIM_KEY          *astKeyList;         /**< Key array : 32 */
  orxANIM_CUSTOM_EVENT *astEventList;       /**< Event array : 36 */
};


/** Static structure
 */
typedef struct __orxANIM_STATIC_t
{
  orxU32 u32Flags;                          /**< Control flags : 4 */

} orxANIM_STATIC;


/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** Static data
 */
static orxANIM_STATIC sstAnim;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

/** Finds a key index given a timestamp
 * @param[in]   _pstAnim        Concerned animation
 * @param[in]   _fTimeStamp     Desired timestamp
 * @return      Key index / orxU32_UNDEFINED
 */
static orxINLINE orxU32 orxAnim_FindKeyIndex(const orxANIM *_pstAnim, orxFLOAT _fTimeStamp)
{
  orxU32 u32Count, u32Index;

  /* Checks */
  orxSTRUCTURE_ASSERT(_pstAnim);

  /* Gets count */
  u32Count = orxAnim_GetKeyCount(_pstAnim);

  /* Is animation not empty? */
  if(u32Count != 0)
  {
    orxU32 u32MaxIndex, u32MinIndex;

    /* Dichotomic search */
    for(u32MinIndex = 0, u32MaxIndex = u32Count - 1, u32Index = u32MaxIndex >> 1;
        u32MinIndex < u32MaxIndex;
        u32Index = (u32MinIndex + u32MaxIndex) >> 1)
    {
      /* Updates search range */
      if(_fTimeStamp > _pstAnim->astKeyList[u32Index].fTimeStamp)
      {
        u32MinIndex = u32Index + 1;
      }
      else
      {
        u32MaxIndex = u32Index;
      }
    }

    /* Not found? */
    if(_pstAnim->astKeyList[u32Index].fTimeStamp < _fTimeStamp)
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_ANIM, "Invalid timestamp.");

      /* Not defined */
      u32Index = orxU32_UNDEFINED;
    }
  }
  /* Empty animation */
  else
  {
    /* Not defined */
    u32Index = orxU32_UNDEFINED;
  }

  /* Done! */
  return u32Index;
}

/** Sets an animation key storage size
 * @param[in]   _pstAnim        Concerned animation
 * @param[in]   _u32Size        Desired size
 */
static orxINLINE void orxAnim_SetKeyStorageSize(orxANIM *_pstAnim, orxU32 _u32Size)
{
  /* Checks */
  orxSTRUCTURE_ASSERT(_pstAnim);
  orxASSERT(_u32Size <= orxANIM_KU32_KEY_MAX_NUMBER);

  /* Updates storage size */
  _pstAnim->u16KeySize = (orxU16)_u32Size;

  return;
}

/** Sets an animation internal key count
 * @param[in]   _pstAnim        Concerned animation
 * @param[in]   _u32KeyCount    Desired key count
 */
static orxINLINE void orxAnim_SetKeyCount(orxANIM *_pstAnim, orxU32 _u32KeyCount)
{
  /* Checks */
  orxSTRUCTURE_ASSERT(_pstAnim);
  orxASSERT(_u32KeyCount <= orxAnim_GetKeyStorageSize(_pstAnim));

  /* Updates count */
  _pstAnim->u16KeyCount = (orxU16)_u32KeyCount;

  return;
}

/** Increases an animation internal key count
 * @param[in]   _pstAnim        Concerned animation
 */
static orxINLINE void orxAnim_IncreaseKeyCount(orxANIM *_pstAnim)
{
  orxU32 u32KeyCount;

  /* Checks */
  orxSTRUCTURE_ASSERT(_pstAnim);

  /* Gets key count */
  u32KeyCount = orxAnim_GetKeyCount(_pstAnim);

  /* Updates key count */
  orxAnim_SetKeyCount(_pstAnim, u32KeyCount + 1);

  return;
}

/** Increases an animation internal key count
 * @param[in]   _pstAnim        Concerned animation
 */
static orxINLINE void orxAnim_DecreaseKeyCount(orxANIM *_pstAnim)
{
  orxU32 u32KeyCount;

  /* Checks */
  orxSTRUCTURE_ASSERT(_pstAnim);

  /* Gets key count */
  u32KeyCount = orxAnim_GetKeyCount(_pstAnim);

  /* Updates key count */
  orxAnim_SetKeyCount(_pstAnim, u32KeyCount - 1);

  return;
}

/** Sets an animation event storage size
 * @param[in]   _pstAnim        Concerned animation
 * @param[in]   _u32Size        Desired size
 */
static orxINLINE void orxAnim_SetEventStorageSize(orxANIM *_pstAnim, orxU32 _u32Size)
{
  /* Checks */
  orxSTRUCTURE_ASSERT(_pstAnim);
  orxASSERT(_u32Size <= orxANIM_KU32_EVENT_MAX_NUMBER);

  /* Updates storage size */
  _pstAnim->u16EventSize = (orxU16)_u32Size;

  return;
}

/** Sets an animation internal event count
 * @param[in]   _pstAnim          Concerned animation
 * @param[in]   _u32EventCount    Desired event count
 */
static orxINLINE void orxAnim_SetEventCount(orxANIM *_pstAnim, orxU32 _u32EventCount)
{
  /* Checks */
  orxSTRUCTURE_ASSERT(_pstAnim);
  orxASSERT(_u32EventCount <= orxAnim_GetEventStorageSize(_pstAnim));

  /* Updates count */
  _pstAnim->u16EventCount = (orxU16)_u32EventCount;

  return;
}

/** Increases an animation internal event count
 * @param[in]   _pstAnim        Concerned animation
 */
static orxINLINE void orxAnim_IncreaseEventCount(orxANIM *_pstAnim)
{
  orxU32 u32EventCount;

  /* Checks */
  orxSTRUCTURE_ASSERT(_pstAnim);

  /* Gets event count */
  u32EventCount = orxAnim_GetEventCount(_pstAnim);

  /* Updates event count */
  orxAnim_SetEventCount(_pstAnim, u32EventCount + 1);

  return;
}

/** Increases an animation internal event count
 * @param[in]   _pstAnim        Concerned animation
 */
static orxINLINE void orxAnim_DecreaseEventCount(orxANIM *_pstAnim)
{
  orxU32 u32EventCount;

  /* Checks */
  orxSTRUCTURE_ASSERT(_pstAnim);

  /* Gets event count */
  u32EventCount = orxAnim_GetEventCount(_pstAnim);

  /* Updates event count */
  orxAnim_SetEventCount(_pstAnim, u32EventCount - 1);

  return;
}

/** Deletes all animations
 */
static orxINLINE void orxAnim_DeleteAll()
{
  orxANIM *pstAnim;

  /* Gets first anim */
  pstAnim = orxANIM (orxStructure_GetFirst(orxSTRUCTURE_ID_ANIM));

  /* Non empty? */
  while(pstAnim != orxNULL)
  {
    /* Deletes Animation */
    orxAnim_Delete(pstAnim);

    /* Gets first Animation */
    pstAnim = orxANIM(orxStructure_GetFirst(orxSTRUCTURE_ID_ANIM));
  }

  return;
}

/** Anim name set accessor, semi-private, internal use only
 * @param[in]   _pstAnim        Concerned animation
 * @param[in]   _zName          Name to use
 * @return      orxSTRING / orxSTRING_EMPTY
 */
orxSTATUS orxFASTCALL orxAnim_SetName(orxANIM *_pstAnim, const orxSTRING _zName)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstAnim.u32Flags & orxANIM_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstAnim);

  /* Updates result */
  _pstAnim->zName = _zName;

  /* Updates status */
  orxStructure_SetFlags(_pstAnim, orxANIM_KU32_FLAG_INTERNAL, orxANIM_KU32_FLAG_NONE);

  /* Done! */
  return eResult;
}


/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** Animation module setup
 */
void orxFASTCALL orxAnim_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_ANIM, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_ANIM, orxMODULE_ID_BANK);
  orxModule_AddDependency(orxMODULE_ID_ANIM, orxMODULE_ID_STRING);
  orxModule_AddDependency(orxMODULE_ID_ANIM, orxMODULE_ID_SYSTEM);
  orxModule_AddDependency(orxMODULE_ID_ANIM, orxMODULE_ID_PROFILER);
  orxModule_AddDependency(orxMODULE_ID_ANIM, orxMODULE_ID_CONFIG);
  orxModule_AddDependency(orxMODULE_ID_ANIM, orxMODULE_ID_STRUCTURE);
  orxModule_AddDependency(orxMODULE_ID_ANIM, orxMODULE_ID_GRAPHIC);

  return;
}

/** Inits the Animation module
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxAnim_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Not already Initialized? */
  if(!(sstAnim.u32Flags & orxANIM_KU32_STATIC_FLAG_READY))
  {
    /* Cleans static controller */
    orxMemory_Zero(&sstAnim, sizeof(orxANIM_STATIC));

    /* Registers structure type */
    eResult = orxSTRUCTURE_REGISTER(ANIM, orxSTRUCTURE_STORAGE_TYPE_LINKLIST, orxMEMORY_TYPE_MAIN, orxANIM_KU32_BANK_SIZE, orxNULL);
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_ANIM, "Anim module already loaded.");

    /* Already initialized */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Initialized? */
  if(eResult != orxSTATUS_FAILURE)
  {
    /* Inits Flags */
    sstAnim.u32Flags = orxANIM_KU32_STATIC_FLAG_READY;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_ANIM, "Failed registering Anim module.");
  }

  /* Done! */
  return eResult;
}

/** Exits from the Animation module
 */
void orxFASTCALL orxAnim_Exit()
{
  /* Initialized? */
  if(sstAnim.u32Flags & orxANIM_KU32_STATIC_FLAG_READY)
  {
    /* Deletes anim list */
    orxAnim_DeleteAll();

    /* Unregisters structure type */
    orxStructure_Unregister(orxSTRUCTURE_ID_ANIM);

    /* Updates flags */
    sstAnim.u32Flags &= ~orxANIM_KU32_STATIC_FLAG_READY;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_ANIM,"Anim module not initialized.");
  }

  return;
}

/** Creates an empty animation
 * @param[in]   _u32Flags       Flags for created animation
 * @param[in]   _u32KeyNumber   Number of keys for this animation
 * @param[in]   _u32EventNumber Number of events for this animation
 * @return      Created orxANIM / orxNULL
 */
orxANIM *orxFASTCALL orxAnim_Create(orxU32 _u32Flags, orxU32 _u32KeyNumber, orxU32 _u32EventNumber)
{
  orxANIM *pstAnim;

  /* Checks */
  orxASSERT(sstAnim.u32Flags & orxANIM_KU32_STATIC_FLAG_READY);
  orxASSERT((_u32Flags & orxANIM_KU32_MASK_USER_ALL) == _u32Flags);
  orxASSERT(_u32KeyNumber <= orxANIM_KU32_KEY_MAX_NUMBER);
  orxASSERT(_u32EventNumber <= orxANIM_KU32_EVENT_MAX_NUMBER);

  /* Creates anim */
  pstAnim = orxANIM(orxStructure_Create(orxSTRUCTURE_ID_ANIM));

  /* Valid? */
  if(pstAnim != orxNULL)
  {
    /* Inits flags */
    orxStructure_SetFlags(pstAnim, _u32Flags & orxANIM_KU32_MASK_USER_ALL, orxANIM_KU32_MASK_ALL);

    /* Clears its name */
    pstAnim->zName = orxSTRING_EMPTY;

    /* 2D Animation? */
    if(orxFLAG_TEST(_u32Flags, orxANIM_KU32_FLAG_2D))
    {
      /* Has keys? */
      if(_u32KeyNumber != 0)
      {
        /* Allocates key array */
        pstAnim->astKeyList = (orxANIM_KEY *)orxMemory_Allocate(_u32KeyNumber * sizeof(orxANIM_KEY), orxMEMORY_TYPE_MAIN);

        /* Cleans it */
        orxMemory_Zero(pstAnim->astKeyList, _u32KeyNumber * sizeof(orxANIM_KEY));
      }

      /* Valid? */
      if((_u32KeyNumber == 0) || (pstAnim->astKeyList != orxNULL))
      {
        /* Has events? */
        if(_u32EventNumber != 0)
        {
          /* Allocates event array */
          pstAnim->astEventList = (orxANIM_CUSTOM_EVENT *)orxMemory_Allocate(_u32EventNumber * sizeof(orxANIM_CUSTOM_EVENT), orxMEMORY_TYPE_MAIN);

          /* Cleans it */
          orxMemory_Zero(pstAnim->astEventList, _u32EventNumber * sizeof(orxANIM_CUSTOM_EVENT));
        }

        /* Valid? */
        if((_u32EventNumber == 0) || (pstAnim->astEventList != orxNULL))
        {
          /* Sets storage size & count */
          orxAnim_SetKeyStorageSize(pstAnim, _u32KeyNumber);
          orxAnim_SetKeyCount(pstAnim, 0);
          orxAnim_SetEventStorageSize(pstAnim, _u32EventNumber);
          orxAnim_SetEventCount(pstAnim, 0);

          /* Increases count */
          orxStructure_IncreaseCount(pstAnim);
        }
        else
        {
          /* Frees key array */
          orxMemory_Free(pstAnim->astKeyList);

          /* Logs message */
          orxDEBUG_PRINT(orxDEBUG_LEVEL_ANIM, "Failed allocating key list.");

          /* Frees partially allocated anim */
          orxStructure_Delete(pstAnim);

          /* Updates result */
          pstAnim = orxNULL;
        }
      }
      else
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_ANIM, "Failed allocating key list.");

        /* Frees partially allocated anim */
        orxStructure_Delete(pstAnim);

        /* Updates result */
        pstAnim = orxNULL;
      }
    }
    /* Other Animation Type? */
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_ANIM, "2D animations are the only ones supported currently.");

      /* Frees partially allocated anim */
      orxStructure_Delete(pstAnim);

      /* Updates result */
      pstAnim = orxNULL;
    }
  }

  /* Done! */
  return pstAnim;
}

/** Creates an animation from config
 * @param[in]   _zConfigID                    Config ID
 * @return      orxANIMSET / orxNULL
 */
orxANIM *orxFASTCALL orxAnim_CreateFromConfig(const orxSTRING _zConfigID)
{
  orxANIM *pstResult = orxNULL;

  /* Checks */
  orxASSERT(sstAnim.u32Flags & orxANIM_KU32_STATIC_FLAG_READY);

  /* Pushes section */
  if((orxConfig_HasSection(_zConfigID) != orxFALSE)
  && (orxConfig_PushSection(_zConfigID) != orxSTATUS_FAILURE))
  {
    orxU32  u32KeyCount, u32EventCount;
    orxCHAR acID[32], acEventID[32];

    /* Clears buffers */
    orxMemory_Zero(acID, 32 * sizeof(orxCHAR));
    orxMemory_Zero(acEventID, 32 * sizeof(orxCHAR));

    /* For all keys/events */
    for(u32KeyCount = 0, u32EventCount = 0, orxString_Print(acID, "%s%d", orxANIM_KZ_CONFIG_KEY_DATA, u32KeyCount + 1), orxString_Print(acEventID, "%s%d", orxANIM_KZ_CONFIG_KEY_EVENT_NAME, u32KeyCount + 1);
        orxConfig_HasValue(acID) != orxFALSE;
        u32KeyCount++, orxString_Print(acID, "%s%d", orxANIM_KZ_CONFIG_KEY_DATA, u32KeyCount + 1), orxString_Print(acEventID, "%s%d", orxANIM_KZ_CONFIG_KEY_EVENT_NAME, u32KeyCount + 1))
    {
      /* Has matching event? */
      if(orxConfig_HasValue(acEventID) != orxFALSE)
      {
        /* Updates event count */
        u32EventCount++;
      }
    }

    /* Success? */
    if(u32KeyCount > 0)
    {
      /* Creates 2D animation */
      pstResult = orxAnim_Create(orxANIM_KU32_FLAG_2D, u32KeyCount, u32EventCount);

      /* Valid? */
      if(pstResult != orxNULL)
      {
        orxCHAR   acTimeID[32], acValueID[32];
        orxFLOAT  fTimeStamp = orxFLOAT_0;
        orxU32    i;

        /* Stores its name */
        pstResult->zName = orxConfig_GetCurrentSection();

        /* Clears buffers */
        orxMemory_Zero(acID, 32 * sizeof(orxCHAR));
        orxMemory_Zero(acEventID, 32 * sizeof(orxCHAR));
        orxMemory_Zero(acTimeID, 32 * sizeof(orxCHAR));
        orxMemory_Zero(acValueID, 32 * sizeof(orxCHAR));

        /* For all keys */
        for(i = 0; i < u32KeyCount; i++)
        {
          const orxSTRING zDataName;

          /* Gets data ID */
          orxString_Print(acID, "%s%d", orxANIM_KZ_CONFIG_KEY_DATA, i + 1);

          /* Gets its name */
          zDataName = orxConfig_GetString(acID);

          /* Valid? */
          if((zDataName != orxNULL) && (zDataName != orxSTRING_EMPTY))
          {
            orxGRAPHIC *pstGraphic;

            /* Creates it */
            pstGraphic = orxGraphic_CreateFromConfig(zDataName);

            /* Valid? */
            if(pstGraphic != orxNULL)
            {
              /* Gets matching event ID */
              orxString_Print(acEventID, "%s%d", orxANIM_KZ_CONFIG_KEY_EVENT_NAME, i + 1);

              /* Defined? */
              if(orxConfig_HasValue(acEventID) != orxFALSE)
              {
                const orxSTRING zEventName;

                /* Gets its name */
                zEventName = orxConfig_GetString(acEventID);

                /* Valid? */
                if((zEventName != orxNULL) && (zEventName != orxSTRING_EMPTY))
                {
                  /* Gets its value IDs */
                  orxString_Print(acValueID, "%s%d", orxANIM_KZ_CONFIG_KEY_EVENT_VALUE, i + 1);

                  /* Adds it */
                  if(orxAnim_AddEvent(pstResult, zEventName, fTimeStamp, orxConfig_GetFloat(acValueID)) == orxSTATUS_FAILURE)
                  {
                    /* Logs message */
                    orxDEBUG_PRINT(orxDEBUG_LEVEL_ANIM, "Failed to add event <%s> to animation <%s>.", zEventName, pstResult->zName);
                  }
                }
              }

              /* Gets duration ID */
              orxString_Print(acTimeID, "%s%d", orxANIM_KZ_CONFIG_KEY_DURATION, i + 1);

              /* Updates its timestamp */
              fTimeStamp += orxConfig_HasValue(acTimeID) ? orxConfig_GetFloat(acTimeID) : orxConfig_GetFloat(orxANIM_KZ_CONFIG_DEFAULT_DURATION);

              /* Adds it */
              if(orxAnim_AddKey(pstResult, orxSTRUCTURE(pstGraphic), fTimeStamp) != orxSTATUS_FAILURE)
              {
                /* Updates graphic's owner */
                orxStructure_SetOwner(pstGraphic, pstResult);
              }
              else
              {
                /* Logs message */
                orxDEBUG_PRINT(orxDEBUG_LEVEL_ANIM, "Failed to add graphic to animation.");

                /* Deletes it */
                orxGraphic_Delete(pstGraphic);
              }
            }
          }
        }

        /* Updates status flags */
        orxStructure_SetFlags(pstResult, orxANIM_KU32_FLAG_INTERNAL, orxANIM_KU32_FLAG_NONE);
      }
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_ANIM, "Couldn't find any key/data in config for anim \"%s\".", _zConfigID);
    }

    /* Pops previous section */
    orxConfig_PopSection();
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_ANIM, "\"%s\" does not exist in config.", _zConfigID);

    /* Updates result */
    pstResult = orxNULL;
  }

  /* Done! */
  return pstResult;
}

/** Deletes an animation
 * @param[in]   _pstAnim        Animation to delete
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxAnim_Delete(orxANIM *_pstAnim)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstAnim.u32Flags & orxANIM_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstAnim);

  /* Decreases count */
  orxStructure_DecreaseCount(_pstAnim);

  /* Not referenced? */
  if(orxStructure_GetRefCount(_pstAnim) == 0)
  {
    /* 2D Animation? */
    if(orxStructure_TestFlags(_pstAnim, orxANIM_KU32_FLAG_2D) != orxFALSE)
    {
      /* Removes all keys */
      orxAnim_RemoveAllKeys(_pstAnim);
    }
    /* Other Animation Type? */
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_ANIM, "2D animations are the only ones supported currently.");
    }

    /* Removes all events */
    orxAnim_RemoveAllEvents(_pstAnim);

    /* Frees key array */
    orxMemory_Free(_pstAnim->astKeyList);

    /* Frees event array */
    orxMemory_Free(_pstAnim->astEventList);

    /* Deletes structure */
    orxStructure_Delete(_pstAnim);
  }
  else
  {
    /* Referenced by others */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Adds a key to an animation
 * @param[in]   _pstAnim        Animation concerned
 * @param[in]   _pstData        Key data to add
 * @param[in]   _fTimeStamp     Timestamp for this key
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxAnim_AddKey(orxANIM *_pstAnim, orxSTRUCTURE *_pstData, orxFLOAT _fTimeStamp)
{
  orxU32    u32Count, u32Size;
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(sstAnim.u32Flags & orxANIM_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstAnim);
  orxASSERT(_pstData != orxNULL);
  orxASSERT(orxStructure_TestFlags(_pstAnim, orxANIM_KU32_FLAG_2D) != orxFALSE);
  orxASSERT((orxAnim_GetKeyCount(_pstAnim) == 0) || (_fTimeStamp > _pstAnim->astKeyList[orxAnim_GetKeyCount(_pstAnim) - 1].fTimeStamp));

  /* Gets storage size & count */
  u32Size     = orxAnim_GetKeyStorageSize(_pstAnim);
  u32Count    = orxAnim_GetKeyCount(_pstAnim);

  /* Is there free room? */
  if(u32Count < u32Size)
  {
    orxANIM_KEY *pstKey;

    /* Gets key pointer */
    pstKey              = &(_pstAnim->astKeyList[u32Count]);

    /* Stores key info */
    pstKey->pstData     = _pstData;
    pstKey->fTimeStamp  = _fTimeStamp;

    /* Updates structure reference count */
    orxStructure_IncreaseCount(_pstData);

    /* Updates key count */
    orxAnim_IncreaseKeyCount(_pstAnim);

    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_ANIM, "No room to add key into animation.");

    /* Updates status */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Removes last added key from an animation
 * @param[in]   _pstAnim        Concerned animation
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxAnim_RemoveLastKey(orxANIM *_pstAnim)
{
  orxU32    u32Count;
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(sstAnim.u32Flags & orxANIM_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstAnim);
  orxASSERT(orxStructure_TestFlags(_pstAnim, orxANIM_KU32_FLAG_2D) != orxFALSE);

  /* Gets count */
  u32Count = orxAnim_GetKeyCount(_pstAnim);

  /* Has key? */
  if(u32Count != 0)
  {
    orxANIM_KEY *pstKey;

    /* Gets real index */
    u32Count--;

    /* Gets key pointer */
    pstKey = &(_pstAnim->astKeyList[u32Count]);

    /* Updates key count */
    orxAnim_DecreaseKeyCount(_pstAnim);

    /* Updates structure reference count */
    orxStructure_DecreaseCount(pstKey->pstData);

    /* Is data internally handled? */
    if(orxStructure_TestFlags(_pstAnim, orxANIM_KU32_FLAG_INTERNAL))
    {
      /* Is 2D data? */
      if(orxStructure_TestFlags(_pstAnim, orxANIM_KU32_FLAG_2D))
      {
        /* Removes its owner */
        orxStructure_SetOwner(pstKey->pstData, orxNULL);

        /* Deletes it */
        orxGraphic_Delete(orxGRAPHIC(pstKey->pstData));
      }
    }

    /* Cleans the key info */
    orxMemory_Zero(pstKey, sizeof(orxANIM_KEY));

    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_ANIM, "Insert keys before attempting to remove last key.");

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Removes all keys from an animation
 * @param[in]   _pstAnim        Concerned animation
 */
void orxFASTCALL orxAnim_RemoveAllKeys(orxANIM *_pstAnim)
{
  /* Checks */
  orxASSERT(sstAnim.u32Flags & orxANIM_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstAnim);
  orxASSERT(orxStructure_TestFlags(_pstAnim, orxANIM_KU32_FLAG_2D) != orxFALSE);

  /* Until there are no key left */
  while((orxAnim_GetKeyCount(_pstAnim) != 0) && (orxAnim_RemoveLastKey(_pstAnim) != orxSTATUS_FAILURE));

  /* Done! */
  return;
}

/** Adds an event to an animation
 * @param[in]   _pstAnim        Concerned animation
 * @param[in]   _zEventName     Event name to add
 * @param[in]   _fTimeStamp     Timestamp for this event
 * @param[in]   _fValue         Value for this event
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxAnim_AddEvent(orxANIM *_pstAnim, const orxSTRING _zEventName, orxFLOAT _fTimeStamp, orxFLOAT _fValue)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(sstAnim.u32Flags & orxANIM_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstAnim);
  orxASSERT(_zEventName != orxNULL);

  /* Valid timestamp? */
  if((orxAnim_GetEventCount(_pstAnim) == 0) || (_fTimeStamp >= _pstAnim->astEventList[orxAnim_GetEventCount(_pstAnim) - 1].fTimeStamp))
  {
    orxU32 u32Count, u32Size;

     /* Gets storage size & count */
     u32Size     = orxAnim_GetEventStorageSize(_pstAnim);
     u32Count    = orxAnim_GetEventCount(_pstAnim);

     /* Is there free room? */
     if(u32Count < u32Size)
     {
       orxANIM_CUSTOM_EVENT *pstEvent;

       /* Gets event pointer */
       pstEvent = &(_pstAnim->astEventList[u32Count]);

       /* Stores key info */
       pstEvent->zName       = orxString_Store(_zEventName);
       pstEvent->fTimeStamp  = _fTimeStamp;
       pstEvent->fValue      = _fValue;

       /* Updates event count */
       orxAnim_IncreaseEventCount(_pstAnim);

       /* Updates result */
       eResult = orxSTATUS_SUCCESS;
     }
     else
     {
       /* Logs message */
       orxDEBUG_PRINT(orxDEBUG_LEVEL_ANIM, "No room to add event into animation.");

       /* Updates status */
       eResult = orxSTATUS_FAILURE;
     }
  }
  else
  {
     /* Logs message */
     orxDEBUG_PRINT(orxDEBUG_LEVEL_ANIM, "Can't add event <%s>: its timestamp [%g] needs to be strictly greater than previous event's one (<%s> @ [%g]).", _zEventName, _fTimeStamp, _pstAnim->astEventList[orxAnim_GetEventCount(_pstAnim) - 1].zName, _pstAnim->astEventList[orxAnim_GetEventCount(_pstAnim) - 1].fTimeStamp);

     /* Updates status */
     eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Removes last added event from an animation
 * @param[in]   _pstAnim        Concerned animation
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxAnim_RemoveLastEvent(orxANIM *_pstAnim)
{
  orxU32    u32Count;
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(sstAnim.u32Flags & orxANIM_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstAnim);

  /* Gets count */
  u32Count = orxAnim_GetEventCount(_pstAnim);

  /* Has event? */
  if(u32Count != 0)
  {
    orxANIM_CUSTOM_EVENT *pstEvent;

    /* Gets real index */
    u32Count--;

    /* Gets event pointer */
    pstEvent = &(_pstAnim->astEventList[u32Count]);

    /* Updates event count */
    orxAnim_DecreaseEventCount(_pstAnim);

    /* Cleans the event info */
    orxMemory_Zero(pstEvent, sizeof(orxANIM_CUSTOM_EVENT));

    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_ANIM, "Insert events before attempting to remove last event.");

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Removes all events from an animation
 * @param[in]   _pstAnim        Concerned animation
 */
void orxFASTCALL orxAnim_RemoveAllEvents(orxANIM *_pstAnim)
{
  /* Checks */
  orxASSERT(sstAnim.u32Flags & orxANIM_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstAnim);

  /* Until there are no event left */
  while((orxAnim_GetEventCount(_pstAnim) != 0) && (orxAnim_RemoveLastEvent(_pstAnim) != orxSTATUS_FAILURE));

  /* Done! */
  return;
}

/** Gets next event
 * @param[in]   _pstAnim        Concerned animation
 * @param[in]   _pstEvent       Event, orxNULL for first
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
const orxANIM_CUSTOM_EVENT *orxFASTCALL orxAnim_GetNextEvent(const orxANIM *_pstAnim, const orxANIM_CUSTOM_EVENT *_pstEvent)
{
  orxU32                u32Count;
  orxANIM_CUSTOM_EVENT *pstResult = orxNULL;

  /* Checks */
  orxASSERT(sstAnim.u32Flags & orxANIM_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstAnim);

  /* Has events and request is in range? */
  if((u32Count = orxAnim_GetEventCount(_pstAnim)) > 0)
  {
    /* Has valid event? */
    if(_pstEvent != orxNULL)
    {
      orxU32 i = 0;

      /* Finds event */
      for(i = 0; (i < u32Count - 1) && (&(_pstAnim->astEventList[i]) != _pstEvent); i++)
        ;

      /* Valid? */
      if(i < u32Count - 1)
      {
        /* Updates result */
        pstResult = &(_pstAnim->astEventList[i + 1]);
      }
    }
    else
    {
      /* Updates result */
      pstResult = &(_pstAnim->astEventList[0]);
    }
  }

  /* Done! */
  return pstResult;
}

/** Gets event strictly after given timestamp
 * @param[in]   _pstAnim        Concerned animation
 * @param[in]   _fTimeStamp     Time stamp, excluded
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
const orxANIM_CUSTOM_EVENT *orxFASTCALL orxAnim_GetEventAfter(const orxANIM *_pstAnim, orxFLOAT _fTimeStamp)
{
  orxU32                u32Count;
  orxANIM_CUSTOM_EVENT *pstResult = orxNULL;

  /* Checks */
  orxASSERT(sstAnim.u32Flags & orxANIM_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstAnim);

  /* Has events and request is in range? */
  if((u32Count = orxAnim_GetEventCount(_pstAnim)) > 0)
  {
    orxU32 i;

    /* For all events */
    for(i = 0; i < u32Count; i++)
    {
      orxANIM_CUSTOM_EVENT *pstEvent;

      /* Gets it */
      pstEvent = &_pstAnim->astEventList[i];

      /* In range? */
      if(pstEvent->fTimeStamp > _fTimeStamp)
      {
        /* No selection or better one? */
        if((pstResult == orxNULL) || (pstEvent->fTimeStamp < pstResult->fTimeStamp))
        {
          /* Selects it */
          pstResult = pstEvent;
        }
      }
    }
  }

  /* Done! */
  return pstResult;
}

/** Gets animation's key index from a time stamp
 * @param[in]   _pstAnim        Concerned animation
 * @param[in]   _fTimeStamp     TimeStamp of the desired animation key
 * @return      Animation key index / orxU32_UNDEFINED
 */
orxU32 orxFASTCALL orxAnim_GetKey(const orxANIM *_pstAnim, orxFLOAT _fTimeStamp)
{
  orxU32    u32Result;

  /* Checks */
  orxASSERT(sstAnim.u32Flags & orxANIM_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstAnim);
  orxASSERT(orxStructure_TestFlags(_pstAnim, orxANIM_KU32_FLAG_2D) != orxFALSE);

  /* Updates result */
  u32Result = orxAnim_FindKeyIndex(_pstAnim, _fTimeStamp);

  /* Done! */
  return u32Result;
}

/** Animation key data accessor
 * @param[in]   _pstAnim        Concerned animation
 * @param[in]   _u32Index       Index of desired key
 * @return      Desired orxSTRUCTURE / orxNULL
 */
orxSTRUCTURE *orxFASTCALL orxAnim_GetKeyData(const orxANIM *_pstAnim, orxU32 _u32Index)
{
  orxU32        u32Count;
  orxSTRUCTURE *pstResult;

  /* Checks */
  orxASSERT(sstAnim.u32Flags & orxANIM_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstAnim);
  orxASSERT(orxStructure_TestFlags(_pstAnim, orxANIM_KU32_FLAG_2D) != orxFALSE);

  /* Gets count */
  u32Count = orxAnim_GetKeyCount(_pstAnim);

  /* Is index valid? */
  if(_u32Index < u32Count)
  {
    /* Updates result */
    pstResult = _pstAnim->astKeyList[_u32Index].pstData;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_ANIM, "Attempt made to get key data from animation outside index range.");

    /* Updates result */
    pstResult = orxNULL;
  }

  /* Done! */
  return pstResult;
}

/** Animation key storage size accessor
 * @param[in]   _pstAnim        Concerned animation
 * @return      Animation key storage size
 */
orxU32 orxFASTCALL orxAnim_GetKeyStorageSize(const orxANIM *_pstAnim)
{
  /* Checks */
  orxASSERT(sstAnim.u32Flags & orxANIM_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstAnim);
  orxASSERT(orxStructure_TestFlags(_pstAnim, orxANIM_KU32_FLAG_2D) != orxFALSE);

  /* Gets storage size */
  return (orxU32)_pstAnim->u16KeySize;
}

/** Animation key count accessor
 * @param[in]   _pstAnim        Concerned animation
 * @return      Animation key count
 */
orxU32 orxFASTCALL orxAnim_GetKeyCount(const orxANIM *_pstAnim)
{
  /* Checks */
  orxASSERT(sstAnim.u32Flags & orxANIM_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstAnim);
  orxASSERT(orxStructure_TestFlags(_pstAnim, orxANIM_KU32_FLAG_2D) != orxFALSE);

  /* Gets count */
  return (orxU32)_pstAnim->u16KeyCount;
}

/** Anim event storage size accessor
 * @param[in]   _pstAnim        Concerned animation
 * @return      Anim event storage size
 */
orxU32 orxFASTCALL orxAnim_GetEventStorageSize(const orxANIM *_pstAnim)
{
  /* Checks */
  orxASSERT(sstAnim.u32Flags & orxANIM_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstAnim);
  orxASSERT(orxStructure_TestFlags(_pstAnim, orxANIM_KU32_FLAG_2D) != orxFALSE);

  /* Gets storage size */
  return (orxU32)_pstAnim->u16EventSize;
}

/** Anim event count accessor
 * @param[in]   _pstAnim        Concerned animation
 * @return      Anim event count
 */
orxU32 orxFASTCALL orxAnim_GetEventCount(const orxANIM *_pstAnim)
{
  /* Checks */
  orxASSERT(sstAnim.u32Flags & orxANIM_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstAnim);
  orxASSERT(orxStructure_TestFlags(_pstAnim, orxANIM_KU32_FLAG_2D) != orxFALSE);

  /* Gets count */
  return (orxU32)_pstAnim->u16EventCount;
}

/** Animation time length accessor
 * @param[in]   _pstAnim        Concerned animation
 * @return      Animation time length
 */
orxFLOAT orxFASTCALL orxAnim_GetLength(const orxANIM *_pstAnim)
{
  orxFLOAT fLength;

  /* Checks */
  orxASSERT(sstAnim.u32Flags & orxANIM_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstAnim);

  /* 2D? */
  if(orxStructure_TestFlags(_pstAnim, orxANIM_KU32_FLAG_2D) != orxFALSE)
  {
    orxU32 u32Count;

    /* Gets key count */
    u32Count = orxAnim_GetKeyCount(_pstAnim);

    /* Is animation non empty? */
    if(u32Count != 0)
    {
      /* Gets length */
      fLength = _pstAnim->astKeyList[u32Count - 1].fTimeStamp;
    }
    else
    {
      /* Updates result */
      fLength = orxFLOAT_0;
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_ANIM, "2D animations are the only ones supported currently.");

    /* Updates result */
    fLength = orx2F(-1.0f);
  }

  /* Done! */
  return fLength;
}

/** Anim name get accessor
 * @param[in]   _pstAnim        Concerned animation
 * @return      orxSTRING / orxSTRING_EMPTY
 */
const orxSTRING orxFASTCALL orxAnim_GetName(const orxANIM *_pstAnim)
{
  const orxSTRING zResult;

  /* Checks */
  orxASSERT(sstAnim.u32Flags & orxANIM_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstAnim);

  /* Updates result */
  zResult = _pstAnim->zName;

  /* Done! */
  return zResult;
}
