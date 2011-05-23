/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2011 Orx-Project
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


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Animation key structure
 */
typedef struct __orxANIM_KEY_t
{
  union
  {
    struct
    {
      orxFLOAT            fTime;                        /**< Key timestamp : 4 */
      orxSTRUCTURE       *pstData;                      /**< Key data : 8 */
    } stStructure;

    orxANIM_CUSTOM_EVENT  stEvent;
  };

} orxANIM_KEY;

#ifdef __orxMSVC__
  #pragma warning(disable : 4200) 
#endif /* __orxMSVC__ */

/** Animation channel structure
 */
typedef struct __orxANIM_CHANNEL_t
{
  orxU16                u16KeySize;
  orxU16                u16KeyCounter;
  orxANIM_KEY           astKeyList[0];

} orxANIM_CHANNEL;

#ifdef __orxMSVC__
  #pragma warning(default : 4200)
#endif /* __orxMSVC__ */

/** Animation structure
 */
struct __orxANIM_t
{
  orxSTRUCTURE          stStructure;                              /**< Public structure, first structure member : 16 */
  const orxSTRING       zName;                                    /**< Anim name : 20 */
  orxANIM_CHANNEL      *apstChannelList[orxANIM_CHANNEL_ID_NUMBER];/** Channel list */
};


/** Static structure
 */
typedef struct __orxANIM_STATIC_t
{
  orxU32 u32Flags;                                                /**< Control flags : 4 */

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
 * @param[in]   _eChannelID     Concerned channel
 * @param[in]   _fTime          Desired timestamp
 * @return      Key index / orxU32_UNDEFINED
 */
static orxINLINE orxU32 orxAnim_FindKeyIndex(const orxANIM *_pstAnim, orxANIM_CHANNEL_ID _eChannelID, orxFLOAT _fTime)
{
  orxU32 u32Result = orxU32_UNDEFINED;

  /* Checks */
  orxSTRUCTURE_ASSERT(_pstAnim);
  orxASSERT(_eChannelID < orxANIM_CHANNEL_ID_NUMBER);

  /* Has channel? */
  if(_pstAnim->apstChannelList[_eChannelID] != orxNULL)
  {
    orxANIM_CHANNEL *pstChannel;

    /* Gets it */
    pstChannel = _pstAnim->apstChannelList[_eChannelID];

    //! TODO
  }

  ///* Gets counter */
  //u32Counter = orxAnim_GetKeyCounter(_pstAnim);

  ///* Is animation not empty? */
  //if(u32Counter != 0)
  //{
  //  /* Dichotomic search */
  //  for(u32MinIndex = 0, u32MaxIndex = u32Counter - 1, u32Index = u32MaxIndex >> 1;
  //      u32MinIndex < u32MaxIndex;
  //      u32Index = (u32MinIndex + u32MaxIndex) >> 1)
  //  {
  //    /* Updates search range */
  //    if(_fTime > _pstAnim->astKeyList[u32Index].stStructure.fTime)
  //    {
  //      u32MinIndex = u32Index + 1;
  //    }
  //    else
  //    {
  //      u32MaxIndex = u32Index;
  //    }
  //  }

  //  /* Not found? */
  //  if(_pstAnim->astKeyList[u32Index].stStructure.fTime < _fTime)
  //  {
  //    /* Logs message */
  //    orxDEBUG_PRINT(orxDEBUG_LEVEL_ANIM, "Invalid timestamp.");

  //    /* Not defined */
  //    u32Index = orxU32_UNDEFINED;
  //  }
  //}
  ///* Empty animation */
  //else
  //{
  //  /* Logs message */
  //  orxDEBUG_PRINT(orxDEBUG_LEVEL_ANIM, "Animation count is 0.");

  //  /* Not defined */
  //  u32Index = orxU32_UNDEFINED;
  //}

  /* Done! */
  return u32Result;
}

/** Deletes all animations
 */
static orxINLINE void orxAnim_DeleteAll()
{
  register orxANIM *pstAnim;

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
    eResult = orxSTRUCTURE_REGISTER(ANIM, orxSTRUCTURE_STORAGE_TYPE_LINKLIST, orxMEMORY_TYPE_MAIN, orxNULL);
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
 * @return      Created orxANIM / orxNULL
 */
orxANIM *orxFASTCALL orxAnim_Create(orxU32 _u32Flags)
{
  orxANIM *pstAnim;

  /* Checks */
  orxASSERT(sstAnim.u32Flags & orxANIM_KU32_STATIC_FLAG_READY);
  orxASSERT((_u32Flags & orxANIM_KU32_MASK_USER_ALL) == _u32Flags);

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
      /* Increases counter */
      orxStructure_IncreaseCounter(pstAnim);
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
    //orxU32  u32KeyCounter, u32EventCounter;
    orxCHAR acID[32], acEventID[32];

    /* Clears buffers */
    orxMemory_Zero(acID, 32 * sizeof(orxCHAR));
    orxMemory_Zero(acEventID, 32 * sizeof(orxCHAR));

    ///* For all keys/events */
    //for(u32KeyCounter = 1, u32EventCounter = 1, orxString_Print(acID, "%s%ld", orxANIM_KZ_CONFIG_KEY_DATA, u32KeyCounter), orxString_Print(acEventID, "%s%ld", orxANIM_KZ_CONFIG_KEY_EVENT_NAME, u32KeyCounter);
    //    orxConfig_HasValue(acID) != orxFALSE;
    //    u32KeyCounter++, orxString_Print(acID, "%s%ld", orxANIM_KZ_CONFIG_KEY_DATA, u32KeyCounter), orxString_Print(acEventID, "%s%ld", orxANIM_KZ_CONFIG_KEY_EVENT_NAME, u32KeyCounter))
    //{
    //  /* Has matching event? */
    //  if(orxConfig_HasValue(acEventID) != orxFALSE)
    //  {
    //    /* Updates event counter */
    //    u32EventCounter++;
    //  }
    //}

    ///* Valid? */
    //if(u32KeyCounter > 1)
    //{
    //  /* Creates 2D animation */
    //  pstResult = orxAnim_Create(orxANIM_KU32_FLAG_2D, --u32KeyCounter, --u32EventCounter);

    //  /* Valid? */
    //  if(pstResult != orxNULL)
    //  {
    //    orxCHAR   acTimeID[32], acValueID[32];
    //    orxFLOAT  fTimeStamp = orxFLOAT_0;
    //    orxU32    i;

    //    /* Stores its name */
    //    pstResult->zName = orxConfig_GetCurrentSection();

    //    /* Clears buffers */
    //    orxMemory_Zero(acID, 32 * sizeof(orxCHAR));
    //    orxMemory_Zero(acEventID, 32 * sizeof(orxCHAR));
    //    orxMemory_Zero(acTimeID, 32 * sizeof(orxCHAR));
    //    orxMemory_Zero(acValueID, 32 * sizeof(orxCHAR));

    //    /* For all keys */
    //    for(i = 0; i < u32KeyCounter; i++)
    //    {
    //      const orxSTRING zDataName;

    //      /* Gets data ID */
    //      orxString_Print(acID, "%s%ld", orxANIM_KZ_CONFIG_KEY_DATA, i + 1);

    //      /* Gets its name */
    //      zDataName = orxConfig_GetString(acID);

    //      /* Valid? */
    //      if((zDataName != orxNULL) && (zDataName != orxSTRING_EMPTY))
    //      {
    //        orxGRAPHIC *pstGraphic;

    //        /* Creates it */
    //        pstGraphic = orxGraphic_CreateFromConfig(zDataName);

    //        /* Valid? */
    //        if(pstGraphic != orxNULL)
    //        {
    //          const orxSTRING zEventName;

    //          /* Gets duration ID */
    //          orxString_Print(acTimeID, "%s%ld", orxANIM_KZ_CONFIG_KEY_DURATION, i + 1);

    //          /* Updates its timestamp */
    //          fTimeStamp += orxConfig_HasValue(acTimeID) ? orxConfig_GetFloat(acTimeID) : orxConfig_GetFloat(orxANIM_KZ_CONFIG_DEFAULT_DURATION);

    //          /* Adds it */
    //          if(orxAnim_AddKey(pstResult, orxSTRUCTURE(pstGraphic), fTimeStamp) == orxSTATUS_FAILURE)
    //          {
    //            /* Logs message */
    //            orxDEBUG_PRINT(orxDEBUG_LEVEL_ANIM, "Failed to add graphic to animation.");

    //            /* Deletes it */
    //            orxGraphic_Delete(pstGraphic);
    //          }

    //          /* Gets matching event ID */
    //          orxString_Print(acEventID, "%s%ld", orxANIM_KZ_CONFIG_KEY_EVENT_NAME, i + 1);

    //          /* Exist? */
    //          if(orxConfig_HasValue(acEventID) != orxFALSE)
    //          {
    //            /* Gets its name */
    //            zEventName = orxConfig_GetString(acEventID);

    //            /* Valid? */
    //            if((zEventName != orxNULL) && (zEventName != orxSTRING_EMPTY))
    //            {
    //              /* Gets its value IDs */
    //              orxString_Print(acValueID, "%s%ld", orxANIM_KZ_CONFIG_KEY_EVENT_VALUE, i + 1);

    //              /* Adds it */
    //              if(orxAnim_AddEvent(pstResult, zEventName, fTimeStamp, orxConfig_GetFloat(acValueID)) == orxSTATUS_FAILURE)
    //              {
    //                /* Logs message */
    //                orxDEBUG_PRINT(orxDEBUG_LEVEL_ANIM, "Failed to add event <%s> to animation <%s>.", zEventName, pstResult->zName);
    //              }
    //            }
    //          }
    //        }
    //      }
    //    }

    //    /* Updates status flags */
    //    orxStructure_SetFlags(pstResult, orxANIM_KU32_FLAG_INTERNAL, orxANIM_KU32_FLAG_NONE);
    //  }
    //}
    //else
    //{
    //  /* Logs message */
    //  orxDEBUG_PRINT(orxDEBUG_LEVEL_ANIM, "Couldn't find any key/data in config for anim \"%s\".", _zConfigID);
    //}

    ///* Pops previous section */
    //orxConfig_PopSection();
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

  /* Decreases counter */
  orxStructure_DecreaseCounter(_pstAnim);

  /* Not referenced? */
  if(orxStructure_GetRefCounter(_pstAnim) == 0)
  {
    orxU32 i;

    /* For all channels */
    for(i = 0; i < (orxU32)orxANIM_CHANNEL_ID_NUMBER; i++)
    {
      /* Is valid? */
      if(_pstAnim->apstChannelList[i] != orxNULL)
      {
        /* Deletes it */
        orxAnim_DeleteChannel(_pstAnim->apstChannelList[i]);
      }
    }

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
  orxU32    u32Counter, u32Size;
  orxSTATUS eResult;

  ///* Checks */
  //orxASSERT(sstAnim.u32Flags & orxANIM_KU32_STATIC_FLAG_READY);
  //orxSTRUCTURE_ASSERT(_pstAnim);
  //orxASSERT(_pstData != orxNULL);
  //orxASSERT(orxStructure_TestFlags(_pstAnim, orxANIM_KU32_FLAG_2D) != orxFALSE);
  //orxASSERT((orxAnim_GetKeyCounter(_pstAnim) == 0) || (_fTimeStamp > _pstAnim->astKeyList[orxAnim_GetKeyCounter(_pstAnim) - 1].stStructure.fTime));

  ///* Gets storage size & counter */
  //u32Size     = orxAnim_GetKeyStorageSize(_pstAnim);
  //u32Counter  = orxAnim_GetKeyCounter(_pstAnim);

  ///* Is there free room? */
  //if(u32Counter < u32Size)
  //{
  //  orxANIM_KEY *pstKey;

  //  /* Gets key pointer */
  //  pstKey              = &(_pstAnim->astKeyList[u32Counter]);

  //  /* Stores key info */
  //  pstKey->stStructure.pstData = _pstData;
  //  pstKey->stStructure.fTime   = _fTimeStamp;

  //  /* Updates structure reference counter */
  //  orxStructure_IncreaseCounter(_pstData);

  //  /* Updates key counter */
  //  orxAnim_IncreaseKeyCounter(_pstAnim);

  //  /* Updates result */
  //  eResult = orxSTATUS_SUCCESS;
  //}
  //else
  //{
  //  /* Logs message */
  //  orxDEBUG_PRINT(orxDEBUG_LEVEL_ANIM, "No room to add key into animation.");

  //  /* Updates status */
  //  eResult = orxSTATUS_FAILURE;
  //}

  /* Done! */
  return eResult;
}

/** Removes last added key from an animation
 * @param[in]   _pstAnim        Concerned animation
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxAnim_RemoveLastKey(orxANIM *_pstAnim)
{
  orxU32    u32Counter;
  orxSTATUS eResult;

  ///* Checks */
  //orxASSERT(sstAnim.u32Flags & orxANIM_KU32_STATIC_FLAG_READY);
  //orxSTRUCTURE_ASSERT(_pstAnim);
  //orxASSERT(orxStructure_TestFlags(_pstAnim, orxANIM_KU32_FLAG_2D) != orxFALSE);

  ///* Gets counter */
  //u32Counter = orxAnim_GetKeyCounter(_pstAnim);

  ///* Has key? */
  //if(u32Counter != 0)
  //{
  //  orxANIM_KEY *pstKey;

  //  /* Gets real index */
  //  u32Counter--;

  //  /* Gets key pointer */
  //  pstKey = &(_pstAnim->astKeyList[u32Counter]);

  //  /* Updates key counter */
  //  orxAnim_DecreaseKeyCounter(_pstAnim);

  //  /* Updates structure reference counter */
  //  orxStructure_DecreaseCounter(orxSTRUCTURE(pstKey->stStructure.pstData));

  //  /* Is data internally handled? */
  //  if(orxStructure_TestFlags(_pstAnim, orxANIM_KU32_FLAG_INTERNAL))
  //  {
  //    /* Is 2D data? */
  //    if(orxStructure_TestFlags(_pstAnim, orxANIM_KU32_FLAG_2D))
  //    {
  //      /* Deletes it */
  //      orxGraphic_Delete(orxGRAPHIC(pstKey->stStructure.pstData));
  //    }
  //  }

  //  /* Cleans the key info */
  //  orxMemory_Zero(pstKey, sizeof(orxANIM_KEY));

  //  /* Updates result */
  //  eResult = orxSTATUS_SUCCESS;
  //}
  //else
  //{
  //  /* Logs message */
  //  orxDEBUG_PRINT(orxDEBUG_LEVEL_ANIM, "Insert keys before attempting to remove last key.");

  //  /* Updates result */
  //  eResult = orxSTATUS_FAILURE;
  //}

  /* Done! */
  return eResult;
}

/** Removes all keys from an animation
 * @param[in]   _pstAnim        Concerned animation
 */
void orxFASTCALL orxAnim_RemoveAllKeys(orxANIM *_pstAnim)
{
  ///* Checks */
  //orxASSERT(sstAnim.u32Flags & orxANIM_KU32_STATIC_FLAG_READY);
  //orxSTRUCTURE_ASSERT(_pstAnim);
  //orxASSERT(orxStructure_TestFlags(_pstAnim, orxANIM_KU32_FLAG_2D) != orxFALSE);

  ///* Until there are no key left */
  //while((orxAnim_GetKeyCounter(_pstAnim) != 0) && (orxAnim_RemoveLastKey(_pstAnim) != orxSTATUS_FAILURE));

  ///* Done! */
  //return;
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
  orxU32    u32Counter, u32Size;
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(sstAnim.u32Flags & orxANIM_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstAnim);
  orxASSERT(_zEventName != orxNULL);

  /* Valid timestamp? */
  if((orxAnim_GetEventCounter(_pstAnim) == 0) || (_fTimeStamp > _pstAnim->astEventList[orxAnim_GetEventCounter(_pstAnim) - 1].fTime))
  {
     /* Gets storage size & counter */
     u32Size     = orxAnim_GetEventStorageSize(_pstAnim);
     u32Counter  = orxAnim_GetEventCounter(_pstAnim);

     /* Is there free room? */
     if(u32Counter < u32Size)
     {
       orxANIM_CUSTOM_EVENT *pstEvent;

       /* Gets event pointer */
       pstEvent = &(_pstAnim->astEventList[u32Counter]);

       /* Stores key info */
       pstEvent->zName  = orxString_Duplicate(_zEventName);
       pstEvent->fTime  = _fTimeStamp;
       pstEvent->fValue = _fValue;

       /* Updates event counter */
       orxAnim_IncreaseEventCounter(_pstAnim);

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
     orxDEBUG_PRINT(orxDEBUG_LEVEL_ANIM, "Can't add event <%s>: its timestamp [%g] needs to be strictly greater than previous event's one (<%s> @ [%g]).", _zEventName, _fTimeStamp, _pstAnim->astEventList[orxAnim_GetEventCounter(_pstAnim) - 1].zName, _pstAnim->astEventList[orxAnim_GetEventCounter(_pstAnim) - 1].fTime);

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
  orxU32    u32Counter;
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(sstAnim.u32Flags & orxANIM_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstAnim);

  /* Gets counter */
  u32Counter = orxAnim_GetEventCounter(_pstAnim);

  /* Has event? */
  if(u32Counter != 0)
  {
    orxANIM_CUSTOM_EVENT *pstEvent;

    /* Gets real index */
    u32Counter--;

    /* Gets event pointer */
    pstEvent = &(_pstAnim->astEventList[u32Counter]);

    /* Updates event counter */
    orxAnim_DecreaseEventCounter(_pstAnim);

    /* Deletes event name */
    orxString_Delete((orxSTRING)pstEvent->zName);

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
  while((orxAnim_GetEventCounter(_pstAnim) != 0) && (orxAnim_RemoveLastEvent(_pstAnim) != orxSTATUS_FAILURE));

  /* Done! */
  return;
}

/** Gets next event after given timestamp
 * @param[in]   _pstAnim        Concerned animation
 * @param[in]   _fTimeStamp     Time stamp, excluded
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
const orxANIM_CUSTOM_EVENT *orxFASTCALL orxAnim_GetNextEvent(const orxANIM *_pstAnim, orxFLOAT _fTimeStamp)
{
  orxU32                u32Counter;
  orxANIM_CUSTOM_EVENT *pstResult = orxNULL;

  /* Checks */
  orxASSERT(sstAnim.u32Flags & orxANIM_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstAnim);

  /* Has events and request is in range? */
  if((u32Counter = orxAnim_GetEventCounter(_pstAnim)) > 0)
  {
    orxU32 i;

    /* For all events */
    for(i = 0; i < u32Counter; i++)
    {
      orxANIM_CUSTOM_EVENT *pstEvent;

      /* Gets it */
      pstEvent = &_pstAnim->astEventList[i];

      /* In range? */
      if(pstEvent->fTime > _fTimeStamp)
      {
        /* No selection or better one? */
        if((pstResult == orxNULL) || (pstEvent->fTime < pstResult->fTime))
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

/** Updates animation given a timestamp
 * @param[in]   _pstAnim        Concerned animation
 * @param[in]   _fTimeStamp     TimeStamp for animation update
 * @param[out]  _pu32CurrentKey Current key as a result of update
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxAnim_Update(orxANIM *_pstAnim, orxFLOAT _fTimeStamp, orxU32 *_pu32CurrentKey)
{
  orxU32    u32Index;
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(sstAnim.u32Flags & orxANIM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pu32CurrentKey != orxNULL);
  orxSTRUCTURE_ASSERT(_pstAnim);
  orxASSERT(orxStructure_TestFlags(_pstAnim, orxANIM_KU32_FLAG_2D) != orxFALSE);

  /* Finds corresponding key index */
  u32Index = orxAnim_FindKeyIndex(_pstAnim, _fTimeStamp);

  /* Found? */
  if(u32Index != orxU32_UNDEFINED)
  {
    /* Updates current key */
    *_pu32CurrentKey = u32Index;

    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_ANIM, "Timestamp does not exist in animation.");

    /* Updates current key */
    *_pu32CurrentKey = orxU32_UNDEFINED;

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Animation key data accessor
 * @param[in]   _pstAnim        Concerned animation
 * @param[in]   _u32Index       Index of desired key
 * @return      Desired orxSTRUCTURE / orxNULL
 */
orxSTRUCTURE *orxFASTCALL orxAnim_GetKeyData(const orxANIM *_pstAnim, orxU32 _u32Index)
{
  orxU32        u32Counter;
  orxSTRUCTURE *pstResult;

  /* Checks */
  orxASSERT(sstAnim.u32Flags & orxANIM_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstAnim);
  orxASSERT(orxStructure_TestFlags(_pstAnim, orxANIM_KU32_FLAG_2D) != orxFALSE);

  /* Gets counter */
  u32Counter = orxAnim_GetKeyCounter(_pstAnim);

  /* Is index valid? */
  if(_u32Index < u32Counter)
  {
    /* Updates result */
    pstResult = orxSTRUCTURE(_pstAnim->astKeyList[_u32Index].stStructure.pstData);
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

/** Animation key counter accessor
 * @param[in]   _pstAnim        Concerned animation
 * @return      Animation key counter
 */
orxU32 orxFASTCALL orxAnim_GetKeyCounter(const orxANIM *_pstAnim)
{
  /* Checks */
  orxASSERT(sstAnim.u32Flags & orxANIM_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstAnim);
  orxASSERT(orxStructure_TestFlags(_pstAnim, orxANIM_KU32_FLAG_2D) != orxFALSE);

  /* Gets counter */
  return (orxU32)_pstAnim->u16KeyCounter;
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

/** Anim event counter accessor
 * @param[in]   _pstAnim        Concerned animation
 * @return      Anim event counter
 */
orxU32 orxFASTCALL orxAnim_GetEventCounter(const orxANIM *_pstAnim)
{
  /* Checks */
  orxASSERT(sstAnim.u32Flags & orxANIM_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstAnim);
  orxASSERT(orxStructure_TestFlags(_pstAnim, orxANIM_KU32_FLAG_2D) != orxFALSE);

  /* Gets counter */
  return (orxU32)_pstAnim->u16EventCounter;
}

/** Animation time length accessor
 * @param[in]   _pstAnim        Concerned animation
 * @return      Animation time length
 */
orxFLOAT orxFASTCALL orxAnim_GetLength(const orxANIM *_pstAnim)
{
  orxU32    u32Counter;
  orxFLOAT  fLength = orxFLOAT_0;

  /* Checks */
  orxASSERT(sstAnim.u32Flags & orxANIM_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstAnim);

  /* 2D? */
  if(orxStructure_TestFlags(_pstAnim, orxANIM_KU32_FLAG_2D) != orxFALSE)
  {
    /* Gets key counter */
    u32Counter = orxAnim_GetKeyCounter(_pstAnim);

    /* Is animation non empty? */
    if(u32Counter != 0)
    {
      /* Gets length */
      fLength = _pstAnim->astKeyList[u32Counter - 1].stStructure.fTime;
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

/** Anim ID get accessor
 * @param[in]   _pstAnim        Concerned animation
 * @return      Anim ID / orxU32_UNDEFINED
 */
orxU32 orxFASTCALL orxAnim_GetID(const orxANIM *_pstAnim)
{
  orxU32 u32Result;

  /* Checks */
  orxASSERT(sstAnim.u32Flags & orxANIM_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstAnim);

  /* Updates result */
  u32Result = orxString_ToCRC(_pstAnim->zName);

  /* Done! */
  return u32Result;
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
