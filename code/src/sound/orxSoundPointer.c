/* Orx - Portable Game Engine
 *
 * Orx is the legal property of its developers, whose names
 * are listed in the COPYRIGHT file distributed
 * with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

/**
 * @file orxSoundPointer.c
 * @date 20/07/2008
 * @author iarwain@orx-project.org
 *
 */


#include "sound/orxSoundPointer.h"

#include "debug/orxDebug.h"
#include "memory/orxMemory.h"
#include "core/orxClock.h"
#include "core/orxEvent.h"
#include "object/orxStructure.h"
#include "object/orxObject.h"


/** Module flags
 */
#define orxSOUNDPOINTER_KU32_STATIC_FLAG_NONE       0x00000000

#define orxSOUNDPOINTER_KU32_STATIC_FLAG_READY      0x00000001

#define orxSOUNDPOINTER_KU32_STATIC_MASK_ALL        0xFFFFFFFF


/** Flags
 */
#define orxSOUNDPOINTER_KU32_FLAG_NONE              0x00000000  /**< No flags */

#define orxSOUNDPOINTER_KU32_FLAG_ENABLED           0x10000000  /**< Enabled flag */

#define orxSOUNDPOINTER_KU32_MASK_ALL               0xFFFFFFFF  /**< All mask */


/** Holder flags
 */
#define orxSOUNDPOINTER_HOLDER_KU32_FLAG_NONE       0x00000000  /**< No flags */

#define orxSOUNDPOINTER_HOLDER_KU32_FLAG_INTERNAL   0x10000000  /**< Internal flag */

#define orxSOUNDPOINTER_HOLDER_KU32_MASK_ALL        0xFFFFFFFF  /**< All mask */


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** SoundPointer holder structure
 */
typedef struct __orxSOUNDPOINTER_HOLDER_t
{
  orxSOUND *pstSound;                                                       /**< Sound reference : 4 */
  orxU32    u32Flags;                                                       /**< Flags : 8 */

} orxSOUNDPOINTER_HOLDER;

/** SoundPointer structure
 */
struct __orxSOUNDPOINTER_t
{
  orxSTRUCTURE            stStructure;                                      /**< Public structure, first structure member : 16 */
  orxSOUNDPOINTER_HOLDER  astSoundList[orxSOUNDPOINTER_KU32_SOUND_NUMBER];  /**< Sound list : 48 */
  orxU32                  u32LastAddedIndex;                                /**< Last added sound index : 52 */
  orxFLOAT                fTimeMultiplier;                                  /**< Current time multiplier : 56 */
  const orxSTRUCTURE     *pstOwner;                                         /**< Owner structure : 60 */
};

/** Static structure
 */
typedef struct __orxSOUNDPOINTER_STATIC_t
{
  orxU32 u32Flags;                                                          /**< Control flags */

} orxSOUNDPOINTER_STATIC;


/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** Static data
 */
static orxSOUNDPOINTER_STATIC sstSoundPointer;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

/** Deletes all the SoundPointers
 */
static orxINLINE void orxSoundPointer_DeleteAll()
{
  orxSOUNDPOINTER *pstSoundPointer;

  /* Gets first SoundPointer */
  pstSoundPointer = orxSOUNDPOINTER(orxStructure_GetFirst(orxSTRUCTURE_ID_SOUNDPOINTER));

  /* Non empty? */
  while(pstSoundPointer != orxNULL)
  {
    /* Deletes it */
    orxSoundPointer_Delete(pstSoundPointer);

    /* Gets first SoundPointer */
    pstSoundPointer = orxSOUNDPOINTER(orxStructure_GetFirst(orxSTRUCTURE_ID_SOUNDPOINTER));
  }

  return;
}

/** Updates the SoundPointer (Callback for generic structure update calling)
 * @param[in]   _pstStructure                 Generic Structure or the concerned Body
 * @param[in]   _pstCaller                    Structure of the caller
 * @param[in]   _pstClockInfo                 Clock info used for time updates
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
static orxSTATUS orxFASTCALL orxSoundPointer_Update(orxSTRUCTURE *_pstStructure, const orxSTRUCTURE *_pstCaller, const orxCLOCK_INFO *_pstClockInfo)
{
  orxSOUNDPOINTER *pstSoundPointer;
  orxOBJECT    *pstObject;
  orxSTATUS     eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstSoundPointer.u32Flags & orxSOUNDPOINTER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstStructure);
  orxSTRUCTURE_ASSERT(_pstCaller);

  /* Gets SoundPointer */
  pstSoundPointer = orxSOUNDPOINTER(_pstStructure);

  /* Gets calling object */
  pstObject = orxOBJECT(_pstCaller);

  /* Is enabled? */
  if(orxSoundPointer_IsEnabled(pstSoundPointer) != orxFALSE)
  {
    orxU32    i;
    orxFLOAT  fFrequencyCoef;

    /* Has a multiply modifier? */
    if(_pstClockInfo->eModType == orxCLOCK_MOD_TYPE_MULTIPLY)
    {
      /* Gets frequency coef */
      fFrequencyCoef = (_pstClockInfo->fModValue != pstSoundPointer->fTimeMultiplier) ? _pstClockInfo->fModValue / pstSoundPointer->fTimeMultiplier : orxFLOAT_1;

      /* Stores multiplier */
      pstSoundPointer->fTimeMultiplier = _pstClockInfo->fModValue;
    }
    else
    {
      /* Reverts frequency coef */
      fFrequencyCoef = (pstSoundPointer->fTimeMultiplier != orxFLOAT_1) ? orxFLOAT_1 / pstSoundPointer->fTimeMultiplier : orxFLOAT_1;

      /* Stores multiplier */
      pstSoundPointer->fTimeMultiplier = orxFLOAT_1;
    }

    /* For all Sounds */
    for(i = 0; i < orxSOUNDPOINTER_KU32_SOUND_NUMBER; i++)
    {
      orxSOUND *pstSound;

      /* Gets sound */
      pstSound = pstSoundPointer->astSoundList[i].pstSound;

      /* Valid? */
      if(pstSound != orxNULL)
      {
        /* Is sound stopped? */
        if(orxSound_GetStatus(pstSound) == orxSOUND_STATUS_STOP)
        {
          orxSOUND_EVENT_PAYLOAD stPayload;

          /* Inits event payload */
          orxMemory_Zero(&stPayload, sizeof(orxSOUND_EVENT_PAYLOAD));
          stPayload.pstSound    = pstSound;
          stPayload.zSoundName  = orxSound_GetName(pstSound);

          /* Sends event */
          orxEVENT_SEND(orxEVENT_TYPE_SOUND, orxSOUND_EVENT_STOP, pstSoundPointer->pstOwner, pstSoundPointer->pstOwner, &stPayload);

          /* Removes it */
          orxSoundPointer_RemoveSound(pstSoundPointer, pstSound);
        }
        else
        {
          orxVECTOR vPosition;

          /* Updates its position */
          orxSound_SetPosition(pstSound, orxObject_GetWorldPosition(pstObject, &vPosition));

          /* Updates its pitch */
          orxSound_SetPitch(pstSound, orxSound_GetPitch(pstSound) * fFrequencyCoef);
        }
      }
    }
  }

  /* Done! */
  return eResult;
}

/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** SoundPointer module setup
 */
void orxFASTCALL orxSoundPointer_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_SOUNDPOINTER, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_SOUNDPOINTER, orxMODULE_ID_STRUCTURE);
  orxModule_AddDependency(orxMODULE_ID_SOUNDPOINTER, orxMODULE_ID_CLOCK);
  orxModule_AddDependency(orxMODULE_ID_SOUNDPOINTER, orxMODULE_ID_EVENT);
  orxModule_AddDependency(orxMODULE_ID_SOUNDPOINTER, orxMODULE_ID_SOUND);

  return;
}

/** Inits the SoundPointer module
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxSoundPointer_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Not already Initialized? */
  if(!(sstSoundPointer.u32Flags & orxSOUNDPOINTER_KU32_STATIC_FLAG_READY))
  {
    /* Cleans static controller */
    orxMemory_Zero(&sstSoundPointer, sizeof(orxSOUNDPOINTER_STATIC));

    /* Registers structure type */
    eResult = orxSTRUCTURE_REGISTER(SOUNDPOINTER, orxSTRUCTURE_STORAGE_TYPE_LINKLIST, orxMEMORY_TYPE_MAIN, orxSoundPointer_Update);

    /* Initialized? */
    if(eResult == orxSTATUS_SUCCESS)
    {
      /* Inits Flags */
      sstSoundPointer.u32Flags = orxSOUNDPOINTER_KU32_STATIC_FLAG_READY;
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "Failed to register structure for Sound Pointer module.");
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "Tried to initialize Sound Pointer module when it was already initialized.");

    /* Already initialized */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Done! */
  return eResult;
}

/** Exits from the SoundPointer module
 */
void orxFASTCALL orxSoundPointer_Exit()
{
  /* Initialized? */
  if(sstSoundPointer.u32Flags & orxSOUNDPOINTER_KU32_STATIC_FLAG_READY)
  {
    /* Deletes SoundPointer list */
    orxSoundPointer_DeleteAll();

    /* Unregisters structure type */
    orxStructure_Unregister(orxSTRUCTURE_ID_SOUNDPOINTER);

    /* Updates flags */
    sstSoundPointer.u32Flags &= ~orxSOUNDPOINTER_KU32_STATIC_FLAG_READY;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "Tried to exit Sound Pointer module when it wasn't initialized.");
  }

  return;
}

/** Creates an empty SoundPointer
 * @param[in]   _pstOwner           Sound's owner used for event callbacks (usually an orxOBJECT)
 * @return      Created orxSOUNDPOINTER / orxNULL
 */
orxSOUNDPOINTER *orxFASTCALL orxSoundPointer_Create(const orxSTRUCTURE *_pstOwner)
{
  orxSOUNDPOINTER *pstResult;

  /* Checks */
  orxASSERT(sstSoundPointer.u32Flags & orxSOUNDPOINTER_KU32_STATIC_FLAG_READY);
  orxASSERT(orxOBJECT(_pstOwner));

  /* Creates SoundPointer */
  pstResult = orxSOUNDPOINTER(orxStructure_Create(orxSTRUCTURE_ID_SOUNDPOINTER));

  /* Created? */
  if(pstResult != orxNULL)
  {
    /* Clears last added sound index */
    pstResult->u32LastAddedIndex = orxU32_UNDEFINED;

    /* Clears time multiplier */
    pstResult->fTimeMultiplier = orxFLOAT_1;

    /* Stores owner */
    pstResult->pstOwner = _pstOwner;

    /* Inits flags */
    orxStructure_SetFlags(pstResult, orxSOUNDPOINTER_KU32_FLAG_ENABLED, orxSOUNDPOINTER_KU32_MASK_ALL);
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "Failed to create Sound Pointer structure.");
  }

  /* Done! */
  return pstResult;
}

/** Deletes a SoundPointer
 * @param[in] _pstSoundPointer      Concerned SoundPointer
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxSoundPointer_Delete(orxSOUNDPOINTER *_pstSoundPointer)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstSoundPointer.u32Flags & orxSOUNDPOINTER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstSoundPointer);

  /* Not referenced? */
  if(orxStructure_GetRefCounter(_pstSoundPointer) == 0)
  {
    orxU32 i;

    /* For all sounds */
    for(i = 0; i < orxSOUNDPOINTER_KU32_SOUND_NUMBER; i++)
    {
      /* Valid? */
      if(_pstSoundPointer->astSoundList[i].pstSound != orxNULL)
      {
        /* Decreases its reference counter */
        orxStructure_DecreaseCounter(_pstSoundPointer->astSoundList[i].pstSound);

        /* Is internal? */
        if(orxFLAG_TEST(_pstSoundPointer->astSoundList[i].u32Flags, orxSOUNDPOINTER_HOLDER_KU32_FLAG_INTERNAL))
        {
          /* Deletes it */
          orxSound_Delete(_pstSoundPointer->astSoundList[i].pstSound);
        }
      }
    }

    /* Deletes structure */
    orxStructure_Delete(_pstSoundPointer);
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "Sound Pointer is still referenced by others. Not deleting.");

    /* Referenced by others */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Gets a SoundPointer owner
 * @param[in]   _pstSoundPointer    Concerned SoundPointer
 * @return      orxSTRUCTURE / orxNULL
 */
orxSTRUCTURE *orxFASTCALL orxSoundPointer_GetOwner(const orxSOUNDPOINTER *_pstSoundPointer)
{
  orxSTRUCTURE *pstResult;

  /* Checks */
  orxASSERT(sstSoundPointer.u32Flags & orxSOUNDPOINTER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstSoundPointer);

  /* Updates result */
  pstResult = orxSTRUCTURE(_pstSoundPointer->pstOwner);

  /* Done! */
  return pstResult;
}

/** Enables/disables a SoundPointer
 * @param[in]   _pstSoundPointer    Concerned SoundPointer
 * @param[in]   _bEnable        Enable / disable
 */
void orxFASTCALL orxSoundPointer_Enable(orxSOUNDPOINTER *_pstSoundPointer, orxBOOL _bEnable)
{
  /* Checks */
  orxASSERT(sstSoundPointer.u32Flags & orxSOUNDPOINTER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstSoundPointer);

  /* Enable? */
  if(_bEnable != orxFALSE)
  {
    /* Updates status flags */
    orxStructure_SetFlags(_pstSoundPointer, orxSOUNDPOINTER_KU32_FLAG_ENABLED, orxSOUNDPOINTER_KU32_FLAG_NONE);
  }
  else
  {
    /* Updates status flags */
    orxStructure_SetFlags(_pstSoundPointer, orxSOUNDPOINTER_KU32_FLAG_NONE, orxSOUNDPOINTER_KU32_FLAG_ENABLED);
  }

  return;
}

/** Is SoundPointer enabled?
 * @param[in]   _pstSoundPointer    Concerned SoundPointer
 * @return      orxTRUE if enabled, orxFALSE otherwise
 */
orxBOOL orxFASTCALL orxSoundPointer_IsEnabled(const orxSOUNDPOINTER *_pstSoundPointer)
{
  /* Checks */
  orxASSERT(sstSoundPointer.u32Flags & orxSOUNDPOINTER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstSoundPointer);

  /* Done! */
  return(orxStructure_TestFlags(_pstSoundPointer, orxSOUNDPOINTER_KU32_FLAG_ENABLED));
}

/** Plays all related sounds
 * @param[in] _pstSoundPointer      Concerned SoundPointer
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxSoundPointer_Play(orxSOUNDPOINTER *_pstSoundPointer)
{
  orxU32    i;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstSoundPointer.u32Flags & orxSOUNDPOINTER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstSoundPointer);

  /* For all sounds */
  for(i = 0; i < orxSOUNDPOINTER_KU32_SOUND_NUMBER; i++)
  {
    orxSOUND *pstSound;

    /* Gets sound */
    pstSound = _pstSoundPointer->astSoundList[i].pstSound;

    /* Valid? */
    if(pstSound != orxNULL)
    {
      /* Plays it */
      orxSound_Play(pstSound);
    }
  }

  /* Done! */
  return eResult;
}

/** Pauses all related sounds
 * @param[in] _pstSoundPointer      Concerned SoundPointer
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxSoundPointer_Pause(orxSOUNDPOINTER *_pstSoundPointer)
{
  orxU32    i;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstSoundPointer.u32Flags & orxSOUNDPOINTER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstSoundPointer);

  /* For all sounds */
  for(i = 0; i < orxSOUNDPOINTER_KU32_SOUND_NUMBER; i++)
  {
    orxSOUND *pstSound;

    /* Gets sound */
    pstSound = _pstSoundPointer->astSoundList[i].pstSound;

    /* Valid? */
    if(pstSound != orxNULL)
    {
      /* Pauses it */
      orxSound_Pause(pstSound);
    }
  }

  /* Done! */
  return eResult;
}

/** Stops all related sounds
 * @param[in] _pstSoundPointer      Concerned SoundPointer
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxSoundPointer_Stop(orxSOUNDPOINTER *_pstSoundPointer)
{
  orxU32    i;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstSoundPointer.u32Flags & orxSOUNDPOINTER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstSoundPointer);

  /* For all sounds */
  for(i = 0; i < orxSOUNDPOINTER_KU32_SOUND_NUMBER; i++)
  {
    orxSOUND *pstSound;

    /* Gets sound */
    pstSound = _pstSoundPointer->astSoundList[i].pstSound;

    /* Valid? */
    if(pstSound != orxNULL)
    {
      /* Stops it */
      orxSound_Stop(pstSound);
    }
  }

  /* Done! */
  return eResult;
}

/** Adds a sound
 * @param[in]   _pstSoundPointer    Concerned SoundPointer
 * @param[in]   _pstSound           Sound to add
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxSoundPointer_AddSound(orxSOUNDPOINTER *_pstSoundPointer, orxSOUND *_pstSound)
{
  orxU32    u32Index;
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(sstSoundPointer.u32Flags & orxSOUNDPOINTER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstSoundPointer);
  orxSTRUCTURE_ASSERT(_pstSound);

  /* Finds an empty slot */
  for(u32Index = 0; (u32Index < orxSOUNDPOINTER_KU32_SOUND_NUMBER) && (_pstSoundPointer->astSoundList[u32Index].pstSound != orxNULL); u32Index++);

  /* Found? */
  if(u32Index < orxSOUNDPOINTER_KU32_SOUND_NUMBER)
  {
    orxSOUND_EVENT_PAYLOAD stPayload;

    /* Increases its reference counter */
    orxStructure_IncreaseCounter(_pstSound);

    /* Adds it to holder */
    _pstSoundPointer->astSoundList[u32Index].pstSound = _pstSound;

    /* Updates its flags */
    orxFLAG_SET(_pstSoundPointer->astSoundList[u32Index].u32Flags, orxSOUNDPOINTER_HOLDER_KU32_FLAG_NONE, orxSOUNDPOINTER_HOLDER_KU32_MASK_ALL);

    /* Stores it as last added sound */
    _pstSoundPointer->u32LastAddedIndex = u32Index;

    /* Inits event payload */
    orxMemory_Zero(&stPayload, sizeof(orxSOUND_EVENT_PAYLOAD));
    stPayload.pstSound    = _pstSound;
    stPayload.zSoundName  = orxSound_GetName(_pstSound);

    /* Sends event */
    orxEVENT_SEND(orxEVENT_TYPE_SOUND, orxSOUND_EVENT_START, _pstSoundPointer->pstOwner, _pstSoundPointer->pstOwner, &stPayload);

    /* Plays it */
    eResult = orxSound_Play(_pstSound);
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "No available slots for sound.");

    /* Clears last added sound index */
    _pstSoundPointer->u32LastAddedIndex = orxU32_UNDEFINED;

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Removes a sound
 * @param[in]   _pstSoundPointer    Concerned SoundPointer
 * @param[in]   _pstSound           Sound to remove
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxSoundPointer_RemoveSound(orxSOUNDPOINTER *_pstSoundPointer, orxSOUND *_pstSound)
{
  orxU32    i;
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(sstSoundPointer.u32Flags & orxSOUNDPOINTER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstSoundPointer);
  orxSTRUCTURE_ASSERT(_pstSound);

  /* For all slots */
  for(i = 0; i < orxSOUNDPOINTER_KU32_SOUND_NUMBER; i++)
  {
    orxSOUND *pstSound;

    /* Gets sound */
    pstSound = _pstSoundPointer->astSoundList[i].pstSound;

    /* Valid? */
    if(pstSound != orxNULL)
    {
      /* Found? */
      if(pstSound == _pstSound)
      {
        /* Decreases its reference counter */
        orxStructure_DecreaseCounter(pstSound);

        /* Was last added sound? */
        if(_pstSoundPointer->u32LastAddedIndex == i)
        {
          /* Clears last added sound index */
          _pstSoundPointer->u32LastAddedIndex = orxU32_UNDEFINED;
        }

        /* Removes its reference */
        _pstSoundPointer->astSoundList[i].pstSound = orxNULL;

        /* Is internal? */
        if(orxFLAG_TEST(_pstSoundPointer->astSoundList[i].u32Flags, orxSOUNDPOINTER_HOLDER_KU32_FLAG_INTERNAL))
        {
          /* Deletes it */
          orxSound_Delete(pstSound);
        }

        /* Updates result */
        eResult = orxSTATUS_SUCCESS;

        break;
      }
    }
  }

  /* Done! */
  return eResult;
}

/** Adds a sound using its config ID
 * @param[in]   _pstSoundPointer    Concerned SoundPointer
 * @param[in]   _zSoundConfigID     Config ID of the sound to add
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxSoundPointer_AddSoundFromConfig(orxSOUNDPOINTER *_pstSoundPointer, const orxSTRING _zSoundConfigID)
{
  orxU32    u32Index;
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(sstSoundPointer.u32Flags & orxSOUNDPOINTER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstSoundPointer);
  orxASSERT((_zSoundConfigID != orxNULL) && (_zSoundConfigID != orxSTRING_EMPTY));

  /* Finds an empty slot */
  for(u32Index = 0; (u32Index < orxSOUNDPOINTER_KU32_SOUND_NUMBER) && (_pstSoundPointer->astSoundList[u32Index].pstSound != orxNULL); u32Index++);

  /* Not found? */
  if(u32Index == orxSOUNDPOINTER_KU32_SOUND_NUMBER)
  {
    orxFLOAT  fShortestDuration;
    orxU32    u32ShortestIndex;

    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "No free slot to play sound <%s>, replacing shortest one.", _zSoundConfigID);

    /* Gets first index */
    u32ShortestIndex = (_pstSoundPointer->u32LastAddedIndex == 0) ? 1 : 0;

    /* Gets its duration */
    fShortestDuration = orxSound_GetDuration(_pstSoundPointer->astSoundList[u32ShortestIndex].pstSound);

    for(u32Index = u32ShortestIndex + 1; (u32Index < orxSOUNDPOINTER_KU32_SOUND_NUMBER); u32Index++)
    {
      /* Not the lattest added one? */
      if(u32Index != _pstSoundPointer->u32LastAddedIndex)
      {
        orxFLOAT fDuration;

        /* Gets its duration */
        fDuration = orxSound_GetDuration(_pstSoundPointer->astSoundList[u32Index].pstSound);

        /* Shorter? */
        if(fDuration < fShortestDuration)
        {
          /* Selects it */
          u32ShortestIndex = u32Index;
        }
      }
    }

    /* Removes it */
    orxSoundPointer_RemoveSound(_pstSoundPointer, _pstSoundPointer->astSoundList[u32ShortestIndex].pstSound);

    /* Updates index */
    u32Index = u32ShortestIndex;
  }

  /* Found? */
  if(u32Index < orxSOUNDPOINTER_KU32_SOUND_NUMBER)
  {
    orxSOUND *pstSound;

    /* Creates sound */
    pstSound = orxSound_CreateFromConfig(_zSoundConfigID);

    /* Valid? */
    if(pstSound != orxNULL)
    {
      orxSOUND_EVENT_PAYLOAD stPayload;

      /* Increases its reference counter */
      orxStructure_IncreaseCounter(pstSound);

      /* Adds it to holder */
      _pstSoundPointer->astSoundList[u32Index].pstSound = pstSound;

      /* Updates its flags */
      orxFLAG_SET(_pstSoundPointer->astSoundList[u32Index].u32Flags, orxSOUNDPOINTER_HOLDER_KU32_FLAG_INTERNAL, orxSOUNDPOINTER_HOLDER_KU32_MASK_ALL);

      /* Stores it as last added sound */
      _pstSoundPointer->u32LastAddedIndex = u32Index;

      /* Inits event payload */
      orxMemory_Zero(&stPayload, sizeof(orxSOUND_EVENT_PAYLOAD));
      stPayload.pstSound    = pstSound;
      stPayload.zSoundName  = orxSound_GetName(pstSound);

      /* Sends event */
      orxEVENT_SEND(orxEVENT_TYPE_SOUND, orxSOUND_EVENT_START, _pstSoundPointer->pstOwner, _pstSoundPointer->pstOwner, &stPayload);

      /* Plays it */
      eResult = orxSound_Play(pstSound);
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "Could not find object named '%s' in config.", _zSoundConfigID);

      /* Clears last added sound index */
      _pstSoundPointer->u32LastAddedIndex = orxU32_UNDEFINED;

      /* Updates result */
      eResult = orxSTATUS_FAILURE;
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "Failed to find an empty slot to put sound <%s> into.", _zSoundConfigID);

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Removes a sound using using its config ID
 * @param[in]   _pstSoundPointer    Concerned SoundPointer
 * @param[in]   _zSoundConfigID     Config ID of the sound to remove
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxSoundPointer_RemoveSoundFromConfig(orxSOUNDPOINTER *_pstSoundPointer, const orxSTRING _zSoundConfigID)
{
  orxU32    i;
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(sstSoundPointer.u32Flags & orxSOUNDPOINTER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstSoundPointer);
  orxASSERT((_zSoundConfigID != orxNULL) && (_zSoundConfigID != orxSTRING_EMPTY));

  /* For all slots */
  for(i = 0; i < orxSOUNDPOINTER_KU32_SOUND_NUMBER; i++)
  {
    orxSOUND *pstSound;

    /* Gets sound */
    pstSound = _pstSoundPointer->astSoundList[i].pstSound;

    /* Valid? */
    if(pstSound != orxNULL)
    {
      /* Found? */
      if(orxString_ToCRC(_zSoundConfigID) == orxString_ToCRC(orxSound_GetName(pstSound)))
      {
        /* Decreases its reference counter */
        orxStructure_DecreaseCounter(pstSound);

        /* Was last added sound? */
        if(_pstSoundPointer->u32LastAddedIndex == i)
        {
          /* Clears last added sound index */
          _pstSoundPointer->u32LastAddedIndex = orxU32_UNDEFINED;
        }

        /* Removes its reference */
        _pstSoundPointer->astSoundList[i].pstSound = orxNULL;

        /* Is internal? */
        if(orxFLAG_TEST(_pstSoundPointer->astSoundList[i].u32Flags, orxSOUNDPOINTER_HOLDER_KU32_FLAG_INTERNAL))
        {
          /* Deletes it */
          orxSound_Delete(pstSound);
        }

        /* Updates result */
        eResult = orxSTATUS_SUCCESS;
        break;
      }
    }
  }

  /* Done! */
  return eResult;
}

/** Gets last added sound (Do *NOT* destroy it directly before removing it!!!)
 * @param[in]   _pstSoundPointer    Concerned SoundPointer
 * @return      orxSOUND / orxNULL
 */
orxSOUND *orxFASTCALL orxSoundPointer_GetLastAddedSound(const orxSOUNDPOINTER *_pstSoundPointer)
{
  orxSOUND *pstResult;

  /* Checks */
  orxASSERT(sstSoundPointer.u32Flags & orxSOUNDPOINTER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstSoundPointer);

  /* Has valid last added sound */
  if((_pstSoundPointer->u32LastAddedIndex != orxU32_UNDEFINED)
  && (_pstSoundPointer->astSoundList[_pstSoundPointer->u32LastAddedIndex].pstSound != orxNULL))
  {
    /* Updates result */
    pstResult = _pstSoundPointer->astSoundList[_pstSoundPointer->u32LastAddedIndex].pstSound;
  }
  else
  {
    /* Updates result */
    pstResult = orxNULL;
  }

  /* Done! */
  return pstResult;
}
