/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2015 Orx-Project
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
 * @file orxSoundPointer.c
 * @date 20/07/2008
 * @author iarwain@orx-project.org
 *
 */


#include "sound/orxSoundPointer.h"

#include "debug/orxDebug.h"
#include "debug/orxProfiler.h"
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
#define orxSOUNDPOINTER_HOLDER_KU32_FLAG_AUTO_PAUSE 0x00000001  /**< Auto-pause flag */
#define orxSOUNDPOINTER_HOLDER_KU32_FLAG_AUTO_PLAY  0x00000002  /**< Auto-play flag */

#define orxSOUNDPOINTER_HOLDER_KU32_MASK_ALL        0xFFFFFFFF  /**< All mask */


/** Misc defines
 */
#define orxSOUNDPOINTER_KU32_BANK_SIZE              128         /**< Bank size */


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** SoundPointer holder structure
 */
typedef struct __orxSOUNDPOINTER_HOLDER_t
{
  orxSOUND *pstSound;                                                       /**< Sound reference : 4/8 */
  orxU32    u32Flags;                                                       /**< Flags : 8/12 */

} orxSOUNDPOINTER_HOLDER;

/** SoundPointer structure
 */
struct __orxSOUNDPOINTER_t
{
  orxSTRUCTURE            stStructure;                                      /**< Public structure, first structure member : 32 */
  orxSOUNDPOINTER_HOLDER  astSoundList[orxSOUNDPOINTER_KU32_SOUND_NUMBER];  /**< Sound list : 48 */
  orxU32                  u32LastAddedIndex;                                /**< Last added sound index : 52 */
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

/** Event handler
 */
static orxSTATUS orxFASTCALL orxSoundPointer_EventHandler(const orxEVENT *_pstEvent)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Depending on event ID */
  switch(_pstEvent->eID)
  {
    /* Pause / disable */
    case orxOBJECT_EVENT_PAUSE:
    case orxOBJECT_EVENT_DISABLE:
    {
      orxSOUNDPOINTER *pstSoundPointer;

      /* Gets sound pointer */
      pstSoundPointer = orxOBJECT_GET_STRUCTURE(orxOBJECT(_pstEvent->hSender), SOUNDPOINTER);

      /* Valid? */
      if(pstSoundPointer != orxNULL)
      {
        orxU32 i;

        /* For all Sounds */
        for(i = 0; i < orxSOUNDPOINTER_KU32_SOUND_NUMBER; i++)
        {
          /* Valid? */
          if(pstSoundPointer->astSoundList[i].pstSound != orxNULL)
          {
            /* Is playing? */
            if(orxSound_GetStatus(pstSoundPointer->astSoundList[i].pstSound) == orxSOUND_STATUS_PLAY)
            {
              /* Pauses it */
              orxSound_Pause(pstSoundPointer->astSoundList[i].pstSound);

              /* Updates its status */
              orxFLAG_SET(pstSoundPointer->astSoundList[i].u32Flags, orxSOUNDPOINTER_HOLDER_KU32_FLAG_AUTO_PAUSE, orxSOUNDPOINTER_HOLDER_KU32_FLAG_NONE);
            }
          }
        }
      }

      break;
    }

    /* Unpause / enable */
    case orxOBJECT_EVENT_UNPAUSE:
    case orxOBJECT_EVENT_ENABLE:
    {
      orxSOUNDPOINTER *pstSoundPointer;

      /* Gets sound pointer */
      pstSoundPointer = orxOBJECT_GET_STRUCTURE(orxOBJECT(_pstEvent->hSender), SOUNDPOINTER);

      /* Valid? */
      if(pstSoundPointer != orxNULL)
      {
        orxU32 i;

        /* For all Sounds */
        for(i = 0; i < orxSOUNDPOINTER_KU32_SOUND_NUMBER; i++)
        {
          /* Valid? */
          if(pstSoundPointer->astSoundList[i].pstSound != orxNULL)
          {
            /* Is auto-paused? */
            if(orxFLAG_TEST(pstSoundPointer->astSoundList[i].u32Flags, orxSOUNDPOINTER_HOLDER_KU32_FLAG_AUTO_PAUSE))
            {
              /* Resumes it */
              orxSound_Play(pstSoundPointer->astSoundList[i].pstSound);

              /* Updates its status */
              orxFLAG_SET(pstSoundPointer->astSoundList[i].u32Flags, orxSOUNDPOINTER_HOLDER_KU32_FLAG_NONE, orxSOUNDPOINTER_HOLDER_KU32_FLAG_AUTO_PAUSE);
            }
          }
        }
      }

      break;
    }

    default:
    {
      break;
    }
  }

  /* Done! */
  return eResult;
}

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

  /* Done! */
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
  orxSOUNDPOINTER  *pstSoundPointer;
  orxSTATUS         eResult = orxSTATUS_SUCCESS;

  /* Profiles */
  orxPROFILER_PUSH_MARKER("orxSoundPointer_Update");

  /* Checks */
  orxASSERT(sstSoundPointer.u32Flags & orxSOUNDPOINTER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstStructure);
  orxSTRUCTURE_ASSERT(_pstCaller);

  /* Gets sound pointer */
  pstSoundPointer = orxSOUNDPOINTER(_pstStructure);

  /* Is enabled? */
  if(orxSoundPointer_IsEnabled(pstSoundPointer) != orxFALSE)
  {
    orxU32 i;

    /* For all Sounds */
    for(i = 0; i < orxSOUNDPOINTER_KU32_SOUND_NUMBER; i++)
    {
      orxSOUND *pstSound;

      /* Gets sound */
      pstSound = pstSoundPointer->astSoundList[i].pstSound;

      /* Valid? */
      if(pstSound != orxNULL)
      {
        /* Should auto play? */
        if(orxFLAG_TEST(pstSoundPointer->astSoundList[i].u32Flags, orxSOUNDPOINTER_HOLDER_KU32_FLAG_AUTO_PLAY))
        {
          /* Plays it */
          orxSound_Play(pstSound);

          /* Updates its flags */
          orxFLAG_SET(pstSoundPointer->astSoundList[i].u32Flags, orxSOUNDPOINTER_HOLDER_KU32_FLAG_NONE, orxSOUNDPOINTER_HOLDER_KU32_FLAG_AUTO_PLAY);
        }

        /* Is sound stopped? */
        if(orxSound_GetStatus(pstSound) == orxSOUND_STATUS_STOP)
        {
          /* Removes it */
          orxSoundPointer_RemoveSound(pstSoundPointer, pstSound);
        }
        else
        {
          /* Delegates update to the sound */
          orxStructure_Update(pstSound, _pstCaller, _pstClockInfo);
        }
      }
    }
  }

  /* Profiles */
  orxPROFILER_POP_MARKER();

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
  orxModule_AddDependency(orxMODULE_ID_SOUNDPOINTER, orxMODULE_ID_PROFILER);
  orxModule_AddDependency(orxMODULE_ID_SOUNDPOINTER, orxMODULE_ID_CLOCK);
  orxModule_AddDependency(orxMODULE_ID_SOUNDPOINTER, orxMODULE_ID_EVENT);
  orxModule_AddDependency(orxMODULE_ID_SOUNDPOINTER, orxMODULE_ID_SOUND);

  /* Done! */
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

    /* Adds event handler */
    eResult = orxEvent_AddHandler(orxEVENT_TYPE_OBJECT, orxSoundPointer_EventHandler);

    /* Success? */
    if(eResult != orxSTATUS_FAILURE)
    {
      /* Registers structure type */
      eResult = orxSTRUCTURE_REGISTER(SOUNDPOINTER, orxSTRUCTURE_STORAGE_TYPE_LINKLIST, orxMEMORY_TYPE_MAIN, orxSOUNDPOINTER_KU32_BANK_SIZE, &orxSoundPointer_Update);

      /* Initialized? */
      if(eResult != orxSTATUS_FAILURE)
      {
        /* Inits Flags */
        sstSoundPointer.u32Flags = orxSOUNDPOINTER_KU32_STATIC_FLAG_READY;
      }
      else
      {
        /* Removes event handler */
        orxEvent_RemoveHandler(orxEVENT_TYPE_OBJECT, orxSoundPointer_EventHandler);

        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "Failed to register structure for Sound Pointer module.");
      }
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "Failed to add event handler.");
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
    /* Removes event handler */
    orxEvent_RemoveHandler(orxEVENT_TYPE_OBJECT, orxSoundPointer_EventHandler);

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

  /* Done! */
  return;
}

/** Creates an empty SoundPointer
 * @return      Created orxSOUNDPOINTER / orxNULL
 */
orxSOUNDPOINTER *orxFASTCALL orxSoundPointer_Create()
{
  orxSOUNDPOINTER *pstResult;

  /* Checks */
  orxASSERT(sstSoundPointer.u32Flags & orxSOUNDPOINTER_KU32_STATIC_FLAG_READY);

  /* Creates SoundPointer */
  pstResult = orxSOUNDPOINTER(orxStructure_Create(orxSTRUCTURE_ID_SOUNDPOINTER));

  /* Created? */
  if(pstResult != orxNULL)
  {
    /* Clears last added sound index */
    pstResult->u32LastAddedIndex = orxU32_UNDEFINED;

    /* Inits flags */
    orxStructure_SetFlags(pstResult, orxSOUNDPOINTER_KU32_FLAG_ENABLED, orxSOUNDPOINTER_KU32_MASK_ALL);

    /* Increases counter */
    orxStructure_IncreaseCounter(pstResult);
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

  /* Decreases counter */
  orxStructure_DecreaseCounter(_pstSoundPointer);

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
          /* Removes its owner */
          orxStructure_SetOwner(_pstSoundPointer->astSoundList[i].pstSound, orxNULL);

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
    /* Referenced by others */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
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

  /* Done! */
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

/** Sets volume of all related sounds
 * @param[in] _pstSoundPointer      Concerned SoundPointer
 * @param[in] _fVolume        Desired volume (0.0 - 1.0)
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxSoundPointer_SetVolume(orxSOUNDPOINTER *_pstSoundPointer, orxFLOAT _fVolume)
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
      /* Sets its volume */
      orxSound_SetVolume(pstSound, _fVolume);
    }
  }

  /* Done! */
  return eResult;
}

/** Sets pitch of all related sounds
 * @param[in] _pstSoundPointer      Concerned SoundPointer
 * @param[in] _fPitch         Desired pitch
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxSoundPointer_SetPitch(orxSOUNDPOINTER *_pstSoundPointer, orxFLOAT _fPitch)
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

    /* Gets it */
    pstSound = _pstSoundPointer->astSoundList[i].pstSound;

    /* Valid? */
    if(pstSound != orxNULL)
    {
      /* Sets its pitch */
      orxSound_SetPitch(pstSound, _fPitch);
    }
  }

  /* Done! */
  return eResult;
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

  /* Not found? */
  if(u32Index == orxSOUNDPOINTER_KU32_SOUND_NUMBER)
  {
    orxFLOAT  fShortestDuration;
    orxU32    u32ShortestIndex;

    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "No free slot to play sound <%s>, replacing shortest one.", orxSound_GetName(_pstSound));

    /* Gets first index */
    u32ShortestIndex = (_pstSoundPointer->u32LastAddedIndex == 0) ? 1 : 0;

    /* Gets its duration */
    fShortestDuration = orxSound_GetDuration(_pstSoundPointer->astSoundList[u32ShortestIndex].pstSound);

    /* For all other sounds */
    for(u32Index = u32ShortestIndex + 1; (u32Index < orxSOUNDPOINTER_KU32_SOUND_NUMBER); u32Index++)
    {
      /* Not the latest added one? */
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
    orxSOUND_EVENT_PAYLOAD  stPayload;
    orxOBJECT              *pstOwner;

    /* Increases its reference counter */
    orxStructure_IncreaseCounter(_pstSound);

    /* Adds it to holder */
    _pstSoundPointer->astSoundList[u32Index].pstSound = _pstSound;

    /* Updates its flags */
    orxFLAG_SET(_pstSoundPointer->astSoundList[u32Index].u32Flags, orxSOUNDPOINTER_HOLDER_KU32_FLAG_AUTO_PLAY, orxSOUNDPOINTER_HOLDER_KU32_MASK_ALL);

    /* Stores it as last added sound */
    _pstSoundPointer->u32LastAddedIndex = u32Index;

    /* Gets its owner object */
    pstOwner = orxOBJECT(orxStructure_GetOwner(_pstSoundPointer));

    /* Valid? */
    if(pstOwner != orxNULL)
    {
      orxVECTOR vPosition;

      /* Updates its position */
      orxSound_SetPosition(_pstSound, orxObject_GetWorldPosition(pstOwner, &vPosition));
    }

    /* Inits event payload */
    orxMemory_Zero(&stPayload, sizeof(orxSOUND_EVENT_PAYLOAD));
    stPayload.pstSound    = _pstSound;
    stPayload.zSoundName  = orxSound_GetName(_pstSound);

    /* Sends event */
    orxEVENT_SEND(orxEVENT_TYPE_SOUND, orxSOUND_EVENT_START, pstOwner, pstOwner, &stPayload);

    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
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
        orxSOUND_EVENT_PAYLOAD  stPayload;
        orxSTRUCTURE           *pstOwner;

        /* Inits event payload */
        orxMemory_Zero(&stPayload, sizeof(orxSOUND_EVENT_PAYLOAD));
        stPayload.pstSound    = pstSound;
        stPayload.zSoundName  = orxSound_GetName(pstSound);

        /* Gets owner */
        pstOwner = orxStructure_GetOwner(_pstSoundPointer);

        /* Sends event */
        orxEVENT_SEND(orxEVENT_TYPE_SOUND, orxSOUND_EVENT_STOP, pstOwner, pstOwner, &stPayload);

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
          /* Removes its owner */
          orxStructure_SetOwner(pstSound, orxNULL);

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

/** Removes all sounds
 * @param[in]   _pstSoundPointer    Concerned SoundPointer
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxSoundPointer_RemoveAllSounds(orxSOUNDPOINTER *_pstSoundPointer)
{
  orxU32    i;
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(sstSoundPointer.u32Flags & orxSOUNDPOINTER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstSoundPointer);

  /* For all slots */
  for(i = 0; i < orxSOUNDPOINTER_KU32_SOUND_NUMBER; i++)
  {
    orxSOUND *pstSound;

    /* Gets sound */
    pstSound = _pstSoundPointer->astSoundList[i].pstSound;

    /* Valid? */
    if(pstSound != orxNULL)
    {
      orxSOUND_EVENT_PAYLOAD  stPayload;
      orxSTRUCTURE           *pstOwner;

      /* Inits event payload */
      orxMemory_Zero(&stPayload, sizeof(orxSOUND_EVENT_PAYLOAD));
      stPayload.pstSound    = pstSound;
      stPayload.zSoundName  = orxSound_GetName(pstSound);

      /* Gets owner */
      pstOwner = orxStructure_GetOwner(_pstSoundPointer);

      /* Sends event */
      orxEVENT_SEND(orxEVENT_TYPE_SOUND, orxSOUND_EVENT_STOP, pstOwner, pstOwner, &stPayload);

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
        /* Removes its owner */
        orxStructure_SetOwner(pstSound, orxNULL);

        /* Deletes it */
        orxSound_Delete(pstSound);
      }

      /* Updates result */
      eResult = orxSTATUS_SUCCESS;
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

    /* For all other sounds */
    for(u32Index = u32ShortestIndex + 1; (u32Index < orxSOUNDPOINTER_KU32_SOUND_NUMBER); u32Index++)
    {
      /* Not the latest added one? */
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
      orxSOUND_EVENT_PAYLOAD  stPayload;
      orxOBJECT              *pstOwner;

      /* Increases its reference counter */
      orxStructure_IncreaseCounter(pstSound);

      /* Adds it to holder */
      _pstSoundPointer->astSoundList[u32Index].pstSound = pstSound;

      /* Updates its owner */
      orxStructure_SetOwner(pstSound, _pstSoundPointer);

      /* Updates its flags */
      orxFLAG_SET(_pstSoundPointer->astSoundList[u32Index].u32Flags, orxSOUNDPOINTER_HOLDER_KU32_FLAG_INTERNAL | orxSOUNDPOINTER_HOLDER_KU32_FLAG_AUTO_PLAY, orxSOUNDPOINTER_HOLDER_KU32_MASK_ALL);

      /* Stores it as last added sound */
      _pstSoundPointer->u32LastAddedIndex = u32Index;

      /* Gets its owner object */
      pstOwner = orxOBJECT(orxStructure_GetOwner(_pstSoundPointer));

      /* Valid? */
      if(pstOwner != orxNULL)
      {
         orxVECTOR vPosition;

         /* Updates its position */
         orxSound_SetPosition(pstSound, orxObject_GetWorldPosition(pstOwner, &vPosition));
      }

      /* Inits event payload */
      orxMemory_Zero(&stPayload, sizeof(orxSOUND_EVENT_PAYLOAD));
      stPayload.pstSound    = pstSound;
      stPayload.zSoundName  = orxSound_GetName(pstSound);

      /* Sends event */
      orxEVENT_SEND(orxEVENT_TYPE_SOUND, orxSOUND_EVENT_START, pstOwner, pstOwner, &stPayload);

      /* Updates result */
      eResult = orxSTATUS_SUCCESS;
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

/** Removes a sound using its config ID
 * @param[in]   _pstSoundPointer    Concerned SoundPointer
 * @param[in]   _zSoundConfigID     Config ID of the sound to remove
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxSoundPointer_RemoveSoundFromConfig(orxSOUNDPOINTER *_pstSoundPointer, const orxSTRING _zSoundConfigID)
{
  orxU32    i, u32ID;
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(sstSoundPointer.u32Flags & orxSOUNDPOINTER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstSoundPointer);
  orxASSERT((_zSoundConfigID != orxNULL) && (_zSoundConfigID != orxSTRING_EMPTY));

  /* Gets sound ID */
  u32ID = orxString_ToCRC(_zSoundConfigID);

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
      if(orxString_ToCRC(orxSound_GetName(pstSound)) == u32ID)
      {
        orxSOUND_EVENT_PAYLOAD  stPayload;
        orxSTRUCTURE           *pstOwner;

        /* Inits event payload */
        orxMemory_Zero(&stPayload, sizeof(orxSOUND_EVENT_PAYLOAD));
        stPayload.pstSound    = pstSound;
        stPayload.zSoundName  = orxSound_GetName(pstSound);

        /* Gets owner */
        pstOwner = orxStructure_GetOwner(_pstSoundPointer);

        /* Sends event */
        orxEVENT_SEND(orxEVENT_TYPE_SOUND, orxSOUND_EVENT_STOP, pstOwner, pstOwner, &stPayload);

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
          /* Removes its owner */
          orxStructure_SetOwner(pstSound, orxNULL);

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
