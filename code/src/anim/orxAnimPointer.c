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
 * @file orxAnimPointer.h
 * @date 03/03/2004
 * @author iarwain@orx-project.org
 *
 */


#include "anim/orxAnimPointer.h"

#include "core/orxConfig.h"
#include "core/orxEvent.h"
#include "debug/orxDebug.h"
#include "debug/orxProfiler.h"
#include "math/orxMath.h"
#include "memory/orxMemory.h"
#include "core/orxClock.h"
#include "object/orxObject.h"


#ifdef __orxMSVC__

  #pragma warning(disable : 4311 4312)

#endif /* __orxMSVC__ */


/** Module flags
 */
#define orxANIMPOINTER_KU32_STATIC_FLAG_NONE          0x00000000  /**< No flags */

#define orxANIMPOINTER_KU32_STATIC_FLAG_READY         0x00000001  /**< Ready flag */


/** orxANIMPOINTER flags
 */
#define orxANIMPOINTER_KU32_FLAG_NONE                 0x00000000  /**< No flags */

#define orxANIMPOINTER_KU32_FLAG_HAS_CURRENT_ANIM     0x00000001  /**< Has current animation flag */
#define orxANIMPOINTER_KU32_FLAG_ANIMSET              0x00000010  /**< Has animset flag */
#define orxANIMPOINTER_KU32_FLAG_LINK_TABLE           0x00000020  /**< Has link table flag */
#define orxANIMPOINTER_KU32_FLAG_PAUSED               0x00000040  /**< Pause flag */
#define orxANIMPOINTER_KU32_FLAG_INTERNAL             0x10000000  /**< Internal structure handling flag  */

#define orxANIMPOINTER_KU32_MASK_FLAGS                0xFFFFFFFF  /**< Flags ID mask */


/** Misc defines
 */
#define orxANIMPOINTER_KZ_CONFIG_FREQUENCY            "Frequency"
#define orxANIMPOINTER_KU32_BANK_SIZE                 128         /**< Bank size */
#define orxANIMPOINTER_KF_FREQUENCY_DEFAULT           1.0         /**< Default animation frequency */


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** AnimPointer structure
 */
struct __orxANIMPOINTER_t
{
  orxSTRUCTURE            stStructure;                /**< Public structure, first structure member : 32 */
  orxANIMSET             *pstAnimSet;                 /**< Referenced AnimationSet : 20 */
  orxANIMSET_LINK_TABLE  *pstLinkTable;               /**< Link table pointer : 24 */
  orxU32                  u32CurrentAnim;             /**< Current animation ID : 28 */
  orxU32                  u32TargetAnim;              /**< Target animation ID : 32 */
  orxFLOAT                fCurrentAnimTime;           /**< Current Time (Relative to current animation) : 26 */
  orxFLOAT                fTime;                      /**< Current Time (Absolute) : 40 */
  orxFLOAT                fFrequency;                 /**< Current animation frequency : 44 */
  orxU32                  u32CurrentKey;              /**< Current animation key : 48 */
  orxU32                  u32LoopCounter;             /**< Current animation loop counter : 52 */
};


/** Static structure
 */
typedef struct __orxANIMPOINTER_STATIC_t
{
  orxU32 u32Flags;                                    /**< Control flags : 4 */

} orxANIMPOINTER_STATIC;


/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** Static data
 */
static orxANIMPOINTER_STATIC sstAnimPointer;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

/** Deletes all AnimPointers
 */
static orxINLINE void orxAnimPointer_DeleteAll()
{
  register orxANIMPOINTER *pstAnimPointer;

  /* Gets first anim pointer */
  pstAnimPointer = orxANIMPOINTER(orxStructure_GetFirst(orxSTRUCTURE_ID_ANIMPOINTER));

  /* Non empty? */
  while(pstAnimPointer != orxNULL)
  {
    /* Deletes AnimPointer */
    orxAnimPointer_Delete(pstAnimPointer);

    /* Gets first Animation Set */
    pstAnimPointer = orxANIMPOINTER(orxStructure_GetFirst(orxSTRUCTURE_ID_ANIMPOINTER));
  }

  return;
}

/** Sends custom events from an animation between two timestamps
 * @param[in]   _pstAnim        Concerned animation
 * @param[in]   _pstOwner       Event's owner
 * @param[in]   _fStartTime     Start time, excluded
 * @param[in]   _fEndTime       End time, included
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
static orxINLINE void orxAnimPointer_SendCustomEvents(orxANIM *_pstAnim, const orxSTRUCTURE *_pstOwner, orxFLOAT _fStartTime, orxFLOAT _fEndTime)
{
  const orxANIM_CUSTOM_EVENT *pstCustomEvent;
  orxANIM_EVENT_PAYLOAD       stPayload;

  /* Checks */
  orxSTRUCTURE_ASSERT(_pstAnim);
  orxSTRUCTURE_ASSERT(_pstOwner);
  orxASSERT(_fEndTime >= _fStartTime);

  /* Inits event payload */
  orxMemory_Zero(&stPayload, sizeof(orxANIM_EVENT_PAYLOAD));
  stPayload.pstAnim   = _pstAnim;
  stPayload.zAnimName = orxAnim_GetName(_pstAnim);

  /* For all events to send */
  for(pstCustomEvent = orxAnim_GetNextEvent(_pstAnim, _fStartTime);
      (pstCustomEvent != orxNULL) && (pstCustomEvent->fTimeStamp <= _fEndTime);
      pstCustomEvent = orxAnim_GetNextEvent(_pstAnim, pstCustomEvent->fTimeStamp))
  {
    /* Updates event payload */
    stPayload.stCustom.zName  = pstCustomEvent->zName;
    stPayload.stCustom.fValue = pstCustomEvent->fValue;
    stPayload.stCustom.fTime  = pstCustomEvent->fTimeStamp;

    /* Sends event */
    orxEVENT_SEND(orxEVENT_TYPE_ANIM, orxANIM_EVENT_CUSTOM_EVENT, _pstOwner, _pstOwner, &stPayload);
  }
}

/** Computes current Anim for the given time
 * @param[in]   _pstAnimPointer               Concerned AnimPointer
 * @param[in]   _fDT                          Delta time
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
static orxINLINE orxSTATUS orxAnimPointer_Compute(orxANIMPOINTER *_pstAnimPointer, orxFLOAT _fDT)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxSTRUCTURE_ASSERT(_pstAnimPointer);
  orxASSERT(orxStructure_TestFlags(_pstAnimPointer, orxANIMPOINTER_KU32_FLAG_ANIMSET) != orxFALSE);

  /* Not Paused? */
  if(orxStructure_TestFlags(_pstAnimPointer, orxANIMPOINTER_KU32_FLAG_PAUSED) == orxFALSE)
  {
    /* Has current animation */
    if(orxStructure_TestFlags(_pstAnimPointer, orxANIMPOINTER_KU32_FLAG_HAS_CURRENT_ANIM) != orxFALSE)
    {
      orxBOOL               bCut, bClearTarget;
      orxU32                u32NewAnim;
      orxFLOAT              fTimeBackup, fEventStartTime;
      orxSTRUCTURE         *pstOwner;
      orxANIM_EVENT_PAYLOAD stPayload;

      /* Gets owner */
      pstOwner = orxStructure_GetOwner(_pstAnimPointer);

      /* Gets event start time */
      fEventStartTime = _pstAnimPointer->fCurrentAnimTime;

      /* Updates Times */
      _pstAnimPointer->fTime += _fDT * _pstAnimPointer->fFrequency;
      _pstAnimPointer->fCurrentAnimTime += _fDT * _pstAnimPointer->fFrequency;

      do
      {
        /* Gets a backup of current time */
        fTimeBackup = _pstAnimPointer->fCurrentAnimTime;

        /* Computes & updates anim*/
        u32NewAnim = orxAnimSet_ComputeAnim(_pstAnimPointer->pstAnimSet, _pstAnimPointer->u32CurrentAnim, _pstAnimPointer->u32TargetAnim, &(_pstAnimPointer->fCurrentAnimTime), _pstAnimPointer->pstLinkTable, &bCut, &bClearTarget);

        /* Inits event payload */
        orxMemory_Zero(&stPayload, sizeof(orxANIM_EVENT_PAYLOAD));
        stPayload.pstAnim   = orxAnimSet_GetAnim(_pstAnimPointer->pstAnimSet, _pstAnimPointer->u32CurrentAnim);
        stPayload.zAnimName = orxAnim_GetName(stPayload.pstAnim);

        /* Change happened? */
        if(u32NewAnim != _pstAnimPointer->u32CurrentAnim)
        {
          orxU32 u32TargetAnim;

          /* Not cut? */
          if(bCut == orxFALSE)
          {
            orxFLOAT fAnimLength;

            /* Gets anim length */
            fAnimLength = orxAnim_GetLength(stPayload.pstAnim);

            /* In scope? */
            if(fEventStartTime < fAnimLength)
            {
              /* Sends custom events */
              orxAnimPointer_SendCustomEvents(stPayload.pstAnim, pstOwner, fEventStartTime, fAnimLength);
            }
          }

          /* Updates current anim ID */
          _pstAnimPointer->u32CurrentAnim = u32NewAnim;

          /* Clears current key */
          _pstAnimPointer->u32CurrentKey = 0;

          /* Clears loop counter */
          _pstAnimPointer->u32LoopCounter = 0;

          /* Stores target anim */
          u32TargetAnim = _pstAnimPointer->u32TargetAnim;

          /* Sends event */
          orxEVENT_SEND(orxEVENT_TYPE_ANIM, (bCut != orxFALSE) ? orxANIM_EVENT_CUT : orxANIM_EVENT_STOP, pstOwner, pstOwner, &stPayload);

          /* No new anim? */
          if(_pstAnimPointer->u32CurrentAnim == orxU32_UNDEFINED)
          {
            /* Cleans target anim */
            _pstAnimPointer->u32TargetAnim = orxU32_UNDEFINED;

            /* Updates flags */
            orxStructure_SetFlags(_pstAnimPointer, orxANIMPOINTER_KU32_FLAG_NONE, orxANIMPOINTER_KU32_FLAG_HAS_CURRENT_ANIM);

            /* Should clear target? */
            if(bClearTarget != orxFALSE)
            {
              /* Removes it */
              _pstAnimPointer->u32TargetAnim = orxU32_UNDEFINED;
            }
          }
          else
          {
            /* Not modified during callback? */
            if((_pstAnimPointer->u32CurrentAnim == u32NewAnim)
            && (_pstAnimPointer->u32TargetAnim == u32TargetAnim))
            {
              /* Inits event payload */
              stPayload.pstAnim   = orxAnimSet_GetAnim(_pstAnimPointer->pstAnimSet, _pstAnimPointer->u32CurrentAnim);
              stPayload.zAnimName = orxAnim_GetName(stPayload.pstAnim);

              /* Sends event */
              orxEVENT_SEND(orxEVENT_TYPE_ANIM, orxANIM_EVENT_START, pstOwner, pstOwner, &stPayload);

              /* Should clear target? */
              if(bClearTarget != orxFALSE)
              {
                /* Removes it */
                _pstAnimPointer->u32TargetAnim = orxU32_UNDEFINED;
              }
            }
          }

          /* Updates event start time */
          fEventStartTime = orx2F(-1.0f);
        }
        else
        {
          /* Looped? */
          if(_pstAnimPointer->fCurrentAnimTime < fTimeBackup)
          {
            orxFLOAT  fAnimLength;
            orxU32    u32CurrentAnim, u32TargetAnim;

            /* Gets anim length */
            fAnimLength = orxAnim_GetLength(stPayload.pstAnim);

            /* In scope? */
            if(fEventStartTime < fAnimLength)
            {
              /* Sends custom events */
              orxAnimPointer_SendCustomEvents(stPayload.pstAnim, pstOwner, fEventStartTime, fAnimLength);
            }

            /* Stores current and target anims */
            u32CurrentAnim  = _pstAnimPointer->u32CurrentAnim;
            u32TargetAnim   = _pstAnimPointer->u32TargetAnim;

            /* Updates loop counter */
            _pstAnimPointer->u32LoopCounter++;

            /* Updates payload */
            stPayload.stLoop.u32Counter = _pstAnimPointer->u32LoopCounter;

            /* Sends it */
            orxEVENT_SEND(orxEVENT_TYPE_ANIM, orxANIM_EVENT_LOOP, pstOwner, pstOwner, &stPayload);

            /* Not modified during event? */
            if((_pstAnimPointer->u32CurrentAnim == u32CurrentAnim)
            && (_pstAnimPointer->u32TargetAnim == u32TargetAnim))
            {
              /* Should clear target? */
              if(bClearTarget != orxFALSE)
              {
                /* Removes it */
                _pstAnimPointer->u32TargetAnim = orxU32_UNDEFINED;
              }
            }

            /* Updates event start time */
            fEventStartTime = orx2F(-1.0f);
          }
        }
      } while(_pstAnimPointer->fCurrentAnimTime > orxAnim_GetLength(stPayload.pstAnim));

      /* Has current anim? */
      if(orxStructure_TestFlags(_pstAnimPointer, orxANIMPOINTER_KU32_FLAG_HAS_CURRENT_ANIM) != orxFALSE)
      {
        orxANIM  *pstAnim;
        orxU32    u32BackupKey;

        /* Gets current anim */
        pstAnim = orxAnimSet_GetAnim(_pstAnimPointer->pstAnimSet, _pstAnimPointer->u32CurrentAnim);

        /* Backups current key */
        u32BackupKey = _pstAnimPointer->u32CurrentKey;

        /* Updates current anim */
        eResult = orxAnim_Update(pstAnim, _pstAnimPointer->fCurrentAnimTime, &(_pstAnimPointer->u32CurrentKey));

        /* New key? */
        if(_pstAnimPointer->u32CurrentKey != u32BackupKey)
        {
          /* Inits event payload */
          orxMemory_Zero(&stPayload, sizeof(orxANIM_EVENT_PAYLOAD));
          stPayload.pstAnim   = orxAnimSet_GetAnim(_pstAnimPointer->pstAnimSet, _pstAnimPointer->u32CurrentAnim);
          stPayload.zAnimName = orxAnim_GetName(stPayload.pstAnim);

          /* Sends it */
          orxEVENT_SEND(orxEVENT_TYPE_ANIM, orxANIM_EVENT_UPDATE, pstOwner, pstOwner, &stPayload);
        }

        /* Sends custom events */
        orxAnimPointer_SendCustomEvents(pstAnim, pstOwner, fEventStartTime, _pstAnimPointer->fCurrentAnimTime);
      }
    }
    else
    {
      /* Can't process */
      eResult = orxSTATUS_FAILURE;
    }
  }

  /* Done! */
  return eResult;
}

/** Updates the AnimPointer (Callback for generic structure update calling)
 * @param[in]   _pstStructure                 Generic Structure or the concerned AnimPointer
 * @param[in]   _pstCaller                    Structure of the caller
 * @param[in]   _pstClockInfo                 Clock info used for time updates
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
static orxSTATUS orxFASTCALL orxAnimPointer_Update(orxSTRUCTURE *_pstStructure, const orxSTRUCTURE *_pstCaller, const orxCLOCK_INFO *_pstClockInfo)
{
  register orxANIMPOINTER *pstAnimPointer;

  /* Profiles */
  orxPROFILER_PUSH_MARKER("orxAnimPointer_Update");

  /* Gets pointer */
  pstAnimPointer = orxANIMPOINTER(_pstStructure);

  /* Checks */
  orxSTRUCTURE_ASSERT(pstAnimPointer);

  /* Computes animation pointer */
  orxAnimPointer_Compute(pstAnimPointer, _pstClockInfo->fDT);

  /* Profiles */
  orxPROFILER_POP_MARKER();

  /* Done! */
  return orxSTATUS_SUCCESS;
}


/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** AnimPointer module setup
 */
void orxFASTCALL orxAnimPointer_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_ANIMPOINTER, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_ANIMPOINTER, orxMODULE_ID_CLOCK);
  orxModule_AddDependency(orxMODULE_ID_ANIMPOINTER, orxMODULE_ID_CONFIG);
  orxModule_AddDependency(orxMODULE_ID_ANIMPOINTER, orxMODULE_ID_PROFILER);
  orxModule_AddDependency(orxMODULE_ID_ANIMPOINTER, orxMODULE_ID_EVENT);
  orxModule_AddDependency(orxMODULE_ID_ANIMPOINTER, orxMODULE_ID_ANIMSET);
  orxModule_AddDependency(orxMODULE_ID_ANIMPOINTER, orxMODULE_ID_ANIM);

  return;
}

/** Inits the AnimPointer module
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxAnimPointer_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Not already Initialized? */
  if(!(sstAnimPointer.u32Flags & orxANIMPOINTER_KU32_STATIC_FLAG_READY))
  {
    /* Cleans static controller */
    orxMemory_Zero(&sstAnimPointer, sizeof(orxANIMPOINTER_STATIC));

    /* Registers structure type */
    eResult = orxSTRUCTURE_REGISTER(ANIMPOINTER, orxSTRUCTURE_STORAGE_TYPE_LINKLIST, orxMEMORY_TYPE_MAIN, orxANIMPOINTER_KU32_BANK_SIZE, &orxAnimPointer_Update);
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_ANIM, "AnimPointer module already initialized.");

    /* Already initialized */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Initialized? */
  if(eResult != orxSTATUS_FAILURE)
  {
    /* Inits Flags */
    sstAnimPointer.u32Flags = orxANIMPOINTER_KU32_STATIC_FLAG_READY;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_ANIM, "Failed to register AnimPointer module.");
  }

  /* Done! */
  return eResult;
}

/** Exits from the AnimPointer module
 */
void orxFASTCALL orxAnimPointer_Exit()
{
  /* Initialized? */
  if(sstAnimPointer.u32Flags & orxANIMPOINTER_KU32_STATIC_FLAG_READY)
  {
    /* Deletes animpointer list */
    orxAnimPointer_DeleteAll();

    /* Unregisters structure type */
    orxStructure_Unregister(orxSTRUCTURE_ID_ANIMPOINTER);

    /* Updates flags */
    sstAnimPointer.u32Flags &= ~orxANIMPOINTER_KU32_STATIC_FLAG_READY;

  }

  return;
}

/** Creates an empty AnimPointer
 * @param[in]   _pstAnimSet                   AnimationSet reference
 * @return      Created orxANIMPOINTER / orxNULL
 */
orxANIMPOINTER *orxFASTCALL orxAnimPointer_Create(orxANIMSET *_pstAnimSet)
{
  orxANIMPOINTER *pstAnimPointer;

  /* Checks */
  orxASSERT(sstAnimPointer.u32Flags & orxANIMPOINTER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstAnimSet);

  /* Creates animpointer */
  pstAnimPointer = orxANIMPOINTER(orxStructure_Create(orxSTRUCTURE_ID_ANIMPOINTER));

  /* Was allocated? */
  if(pstAnimPointer != orxNULL)
  {
    /* Stores animset */
    pstAnimPointer->pstAnimSet = _pstAnimSet;

    /* Adds a reference on the animset */
    orxAnimSet_AddReference(_pstAnimSet);

    /* Inits flags */
    orxStructure_SetFlags(pstAnimPointer, orxANIMPOINTER_KU32_FLAG_ANIMSET | orxANIMPOINTER_KU32_FLAG_HAS_CURRENT_ANIM, orxANIMPOINTER_KU32_MASK_FLAGS);

    /* Inits value */
    pstAnimPointer->u32CurrentAnim    = 0;
    pstAnimPointer->fCurrentAnimTime  = orxFLOAT_0;
    pstAnimPointer->fFrequency        = orxANIMPOINTER_KF_FREQUENCY_DEFAULT;
    pstAnimPointer->fTime             = orxFLOAT_0;
    pstAnimPointer->u32TargetAnim     = orxU32_UNDEFINED;
    pstAnimPointer->u32CurrentKey     = 0;
    pstAnimPointer->u32LoopCounter    = 0;

    /* Is animset link table non-static? */
    if(orxStructure_TestFlags(_pstAnimSet, orxANIMSET_KU32_FLAG_LINK_STATIC) == orxFALSE)
    {
      /* Stores link table */
      pstAnimPointer->pstLinkTable = orxAnimSet_CloneLinkTable(_pstAnimSet);

      /* Updates flags */
      orxStructure_SetFlags(pstAnimPointer, orxANIMPOINTER_KU32_FLAG_LINK_TABLE, orxANIMPOINTER_KU32_FLAG_NONE);
    }

    /* Increases counter */
    orxStructure_IncreaseCounter(pstAnimPointer);
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_ANIM, "Failed creating Anim pointer.");

    /* Not created */
    pstAnimPointer = orxNULL;
  }

  /* Done! */
  return pstAnimPointer;
}

/** Creates an animation pointer from config
 * @param[in]   _zConfigID                    Config ID
 * @return      orxANIMPOINTER / orxNULL
 */
orxANIMPOINTER *orxFASTCALL orxAnimPointer_CreateFromConfig(const orxSTRING _zConfigID)
{
  orxANIMPOINTER *pstResult = orxNULL;

  /* Checks */
  orxASSERT(sstAnimPointer.u32Flags & orxANIMPOINTER_KU32_STATIC_FLAG_READY);

  /* Pushes section */
  if((orxConfig_HasSection(_zConfigID) != orxFALSE)
  && (orxConfig_PushSection(_zConfigID) != orxSTATUS_FAILURE))
  {
    orxANIMSET *pstAnimSet;

    /* Creates animset from config */
    pstAnimSet = orxAnimSet_CreateFromConfig(_zConfigID);

    /* Valid? */
    if(pstAnimSet != orxNULL)
    {
      /* Creates animation pointer from it */
      pstResult = orxAnimPointer_Create(pstAnimSet);

      /* Valid? */
      if(pstResult != orxNULL)
      {
        /* Has frequency? */
        if(orxConfig_HasValue(orxANIMPOINTER_KZ_CONFIG_FREQUENCY) != orxFALSE)
        {
          /* Updates animation pointer frequency */
          orxAnimPointer_SetFrequency(pstResult, orxConfig_GetFloat(orxANIMPOINTER_KZ_CONFIG_FREQUENCY));
        }

        /* Sets AnimSet's owner */
        orxStructure_SetOwner(pstAnimSet, pstResult);

        /* Updates status flags */
        orxStructure_SetFlags(pstResult, orxANIMPOINTER_KU32_FLAG_INTERNAL, orxANIMPOINTER_KU32_FLAG_NONE);
      }
      else
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_ANIM, "Failed creating Anim pointer.");

        /* Deletes created anim set */
        orxAnimSet_Delete(pstAnimSet);
      }
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

/** Deletes an AnimPointer
 * @param[in]   _pstAnimPointer               AnimPointer to delete
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxAnimPointer_Delete(orxANIMPOINTER *_pstAnimPointer)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstAnimPointer.u32Flags & orxANIMPOINTER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstAnimPointer);

  /* Decreases counter */
  orxStructure_DecreaseCounter(_pstAnimPointer);

  /* Not referenced? */
  if(orxStructure_GetRefCounter(_pstAnimPointer) == 0)
  {
    /* Has an animset? */
    if(orxStructure_TestFlags(_pstAnimPointer, orxANIMPOINTER_KU32_FLAG_ANIMSET) != orxFALSE)
    {
      /* Removes the reference from the animset */
      orxAnimSet_RemoveReference(_pstAnimPointer->pstAnimSet);

      /* Was internally allocated? */
      if(orxStructure_TestFlags(_pstAnimPointer, orxANIMPOINTER_KU32_FLAG_INTERNAL) != orxFALSE)
      {
        /* Removes its owner */
        orxStructure_SetOwner(_pstAnimPointer->pstAnimSet, orxNULL);

        /* Deletes animset */
        orxAnimSet_Delete(_pstAnimPointer->pstAnimSet);
      }
    }

    /* Has a link table? */
    if(orxStructure_TestFlags(_pstAnimPointer, orxANIMPOINTER_KU32_FLAG_LINK_TABLE) != orxFALSE)
    {
      /* Deletes it */
      orxAnimSet_DeleteLinkTable(_pstAnimPointer->pstLinkTable);
    }

    /* Deletes structure */
    orxStructure_Delete(_pstAnimPointer);
  }
  else
  {
    /* Referenced by others */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Gets the referenced AnimationSet
 * @param[in]   _pstAnimPointer               Concerned AnimPointer
 * @return      Referenced orxANIMSET
 */
orxANIMSET *orxFASTCALL orxAnimPointer_GetAnimSet(const orxANIMPOINTER *_pstAnimPointer)
{
  orxANIMSET *pstAnimSet = orxNULL;

  /* Checks */
  orxASSERT(sstAnimPointer.u32Flags & orxANIMPOINTER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstAnimPointer);

  /* Has an animset? */
  if(orxStructure_TestFlags(_pstAnimPointer, orxANIMPOINTER_KU32_FLAG_ANIMSET) != orxFALSE)
  {
    pstAnimSet = _pstAnimPointer->pstAnimSet;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_ANIM, "Anim pointer does not have Anim set.");
  }

  /* Done! */
  return pstAnimSet;
}

/** AnimPointer current Animation get accessor
 * @param[in]   _pstAnimPointer               Concerned AnimPointer
 * @return      Current Animation ID
 */
orxU32 orxFASTCALL orxAnimPointer_GetCurrentAnim(const orxANIMPOINTER *_pstAnimPointer)
{
  orxU32 u32Anim = orxU32_UNDEFINED;

  /* Checks */
  orxASSERT(sstAnimPointer.u32Flags & orxANIMPOINTER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstAnimPointer);

  /* Has anim? */
  if((orxStructure_TestFlags(_pstAnimPointer, orxANIMPOINTER_KU32_FLAG_ANIMSET) != orxFALSE)
  && (orxStructure_TestFlags(_pstAnimPointer, orxANIMPOINTER_KU32_FLAG_HAS_CURRENT_ANIM) != orxFALSE))
  {
    u32Anim = _pstAnimPointer->u32CurrentAnim;
  }

  /* Done! */
  return u32Anim;
}


/** AnimPointer target Animation get accessor
 * @param[in]   _pstAnimPointer               Concerned AnimPointer
 * @return      Target Animation ID
 */
orxU32 orxFASTCALL orxAnimPointer_GetTargetAnim(const orxANIMPOINTER *_pstAnimPointer)
{
  orxU32 u32Anim = orxU32_UNDEFINED;

  /* Checks */
  orxASSERT(sstAnimPointer.u32Flags & orxANIMPOINTER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstAnimPointer);

  /* Has anim? */
  if((orxStructure_TestFlags(_pstAnimPointer, orxANIMPOINTER_KU32_FLAG_ANIMSET) != orxFALSE)
  && (orxStructure_TestFlags(_pstAnimPointer, orxANIMPOINTER_KU32_FLAG_HAS_CURRENT_ANIM) != orxFALSE))
  {
    u32Anim = _pstAnimPointer->u32TargetAnim;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_ANIM, "Anim pointer does not have a current anim.");
  }

  /* Done! */
  return u32Anim;
}

/** AnimPointer current Animation name get accessor
 * @param[in]   _pstAnimPointer               Concerned AnimPointer
 * @return      Current Animation name
 */
const orxSTRING orxFASTCALL orxAnimPointer_GetCurrentAnimName(const orxANIMPOINTER *_pstAnimPointer)
{
  const orxSTRING zAnimName = orxSTRING_EMPTY;

  /* Checks */
  orxASSERT(sstAnimPointer.u32Flags & orxANIMPOINTER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstAnimPointer);

  /* Has anim? */
  if((orxStructure_TestFlags(_pstAnimPointer, orxANIMPOINTER_KU32_FLAG_ANIMSET) != orxFALSE)
  && (orxStructure_TestFlags(_pstAnimPointer, orxANIMPOINTER_KU32_FLAG_HAS_CURRENT_ANIM) != orxFALSE))
  {
    orxANIM *pstAnim;

    /* Gets it */
    pstAnim = orxAnimSet_GetAnim(_pstAnimPointer->pstAnimSet, _pstAnimPointer->u32CurrentAnim);

    /* Valid? */
    if(pstAnim != orxNULL)
    {
      /* Updates result */
      zAnimName = orxAnim_GetName(pstAnim);
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_ANIM, "Anim pointer does not have a current anim.");
  }

  /* Done! */
  return zAnimName;
}

/** AnimPointer target Animation name get accessor
 * @param[in]   _pstAnimPointer               Concerned AnimPointer
 * @return      Target Animation name / orxSTRING_EMPTY
 */
const orxSTRING orxFASTCALL orxAnimPointer_GetTargetAnimName(const orxANIMPOINTER *_pstAnimPointer)
{
  const orxSTRING zAnimName = orxSTRING_EMPTY;

  /* Checks */
  orxASSERT(sstAnimPointer.u32Flags & orxANIMPOINTER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstAnimPointer);

  /* Has anim? */
  if((orxStructure_TestFlags(_pstAnimPointer, orxANIMPOINTER_KU32_FLAG_ANIMSET) != orxFALSE)
  && (orxStructure_TestFlags(_pstAnimPointer, orxANIMPOINTER_KU32_FLAG_HAS_CURRENT_ANIM) != orxFALSE))
  {
    orxANIM *pstAnim;

    /* Gets it */
    pstAnim = orxAnimSet_GetAnim(_pstAnimPointer->pstAnimSet, _pstAnimPointer->u32TargetAnim);

    /* Valid? */
    if(pstAnim != orxNULL)
    {
      /* Updates result */
      zAnimName = orxAnim_GetName(pstAnim);
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_ANIM, "Anim pointer does not have a current anim.");
  }

  /* Done! */
  return zAnimName;
}

/** AnimPointer current anim data get accessor
 * @param[in]   _pstAnimPointer               Concerned AnimPointer
 * @return      Current anim data / orxNULL
 */
orxSTRUCTURE *orxFASTCALL orxAnimPointer_GetCurrentAnimData(const orxANIMPOINTER *_pstAnimPointer)
{
  orxU32        u32AnimID;
  orxSTRUCTURE *pstResult = orxNULL;

  /* Checks */
  orxASSERT(sstAnimPointer.u32Flags & orxANIMPOINTER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstAnimPointer);

  /* Gets current anim ID */
  u32AnimID = orxAnimPointer_GetCurrentAnim(_pstAnimPointer);

  /* Valid? */
  if(u32AnimID != orxU32_UNDEFINED)
  {
    orxANIM *pstAnim;

    /* Gets anim */
    pstAnim = orxAnimSet_GetAnim(orxAnimPointer_GetAnimSet(_pstAnimPointer), u32AnimID);

    /* Valid? */
    if(pstAnim != orxNULL)
    {
      /* Gets data */
      pstResult = orxAnim_GetKeyData(pstAnim, _pstAnimPointer->u32CurrentKey);
    }
  }

  /* Done! */
  return pstResult;
}

/** AnimPointer current Time get accessor
 * @param[in]   _pstAnimPointer               Concerned AnimPointer
 * @return      Current time
 */
orxFLOAT orxFASTCALL orxAnimPointer_GetCurrentTime(const orxANIMPOINTER *_pstAnimPointer)
{
  register orxFLOAT fResult = orxFLOAT_0;

  /* Checks */
  orxASSERT(sstAnimPointer.u32Flags & orxANIMPOINTER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstAnimPointer);

  /* Has anim? */
  if(orxStructure_TestFlags(_pstAnimPointer, orxANIMPOINTER_KU32_FLAG_HAS_CURRENT_ANIM) != orxFALSE)
  {
    /* Gets time */
    fResult = _pstAnimPointer->fCurrentAnimTime;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_ANIM, "Anim pointer does not have a current anim.");
  }

  /* Done! */
  return fResult;
}

/** AnimPointer Frequency get accessor
 * @param[in]   _pstAnimPointer               Concerned AnimPointer
 * @return      AnimPointer frequency
 */
orxFLOAT orxFASTCALL orxAnimPointer_GetFrequency(const orxANIMPOINTER *_pstAnimPointer)
{
  /* Checks */
  orxASSERT(sstAnimPointer.u32Flags & orxANIMPOINTER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstAnimPointer);

  /* Gets frequency */
  return _pstAnimPointer->fFrequency;
}

/** AnimPointer current Animation set accessor
 * @param[in]   _pstAnimPointer               Concerned AnimPointer
 * @param[in]   _u32AnimID                    Animation ID to set
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxAnimPointer_SetCurrentAnim(orxANIMPOINTER *_pstAnimPointer, orxU32 _u32AnimID)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstAnimPointer.u32Flags & orxANIMPOINTER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstAnimPointer);

  /* Has Animset? */
  if(orxStructure_TestFlags(_pstAnimPointer, orxANIMPOINTER_KU32_FLAG_ANIMSET) != orxFALSE)
  {
    /* In range? */
    if(_u32AnimID < orxAnimSet_GetAnimCounter(_pstAnimPointer->pstAnimSet))
    {
      orxANIM_EVENT_PAYLOAD stPayload;
      orxANIM              *pstAnim;
      orxSTRUCTURE         *pstOwner;
      orxU32                u32CurrentAnim;

      /* Clears event payload */
      orxMemory_Zero(&stPayload, sizeof(orxANIM_EVENT_PAYLOAD));

      /* Gets owner */
      pstOwner = orxStructure_GetOwner(_pstAnimPointer);

      /* Stores current anim */
      u32CurrentAnim = _pstAnimPointer->u32CurrentAnim;

      /* Stores ID */
      _pstAnimPointer->u32CurrentAnim = _u32AnimID;

      /* Clears target anim */
      _pstAnimPointer->u32TargetAnim  = orxU32_UNDEFINED;

      /* Clears current key */
      _pstAnimPointer->u32CurrentKey  = 0;

      /* Clears loop counter */
      _pstAnimPointer->u32LoopCounter = 0;

      /* Has current anim? */
      if(u32CurrentAnim != orxU32_UNDEFINED)
      {
        /* Inits event payload */
        pstAnim             = orxAnimSet_GetAnim(_pstAnimPointer->pstAnimSet, u32CurrentAnim);
        stPayload.pstAnim   = pstAnim;
        stPayload.zAnimName = orxAnim_GetName(pstAnim);

        /* Sends event */
        orxEVENT_SEND(orxEVENT_TYPE_ANIM, orxANIM_EVENT_CUT, pstOwner, pstOwner, &stPayload);
      }

      /* Inits event payload */
      pstAnim             = orxAnimSet_GetAnim(_pstAnimPointer->pstAnimSet, _pstAnimPointer->u32CurrentAnim);
      stPayload.pstAnim   = pstAnim;
      stPayload.zAnimName = orxAnim_GetName(pstAnim);

      /* Sends event */
      orxEVENT_SEND(orxEVENT_TYPE_ANIM, orxANIM_EVENT_START, pstOwner, pstOwner, &stPayload);

      /* Updates flags */
      orxStructure_SetFlags(_pstAnimPointer, orxANIMPOINTER_KU32_FLAG_HAS_CURRENT_ANIM, orxANIMPOINTER_KU32_FLAG_NONE);

      /* Computes animpointer from start */
      eResult = orxAnimPointer_SetTime(_pstAnimPointer, orxFLOAT_0);
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_ANIM, "%d is not a valid ID for the anim pointer.", _u32AnimID);

      /* Can't process */
      eResult = orxSTATUS_FAILURE;
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_ANIM, "Anim pointer does not have a current anim.");

    /* Can't process */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** AnimPointer target Animation set accessor
 * @param[in]   _pstAnimPointer               Concerned AnimPointer
 * @param[in]   _u32AnimID                    Animation ID to set / orxU32_UNDEFINED
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxAnimPointer_SetTargetAnim(orxANIMPOINTER *_pstAnimPointer, orxU32 _u32AnimID)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstAnimPointer.u32Flags & orxANIMPOINTER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstAnimPointer);

  /* Has Animset? */
  if(orxStructure_TestAllFlags(_pstAnimPointer, orxANIMPOINTER_KU32_FLAG_ANIMSET | orxANIMPOINTER_KU32_FLAG_HAS_CURRENT_ANIM) != orxFALSE)
  {
    /* New value? */
    if(_pstAnimPointer->u32TargetAnim != _u32AnimID)
    {
      /* Removes target anim? */
      if(_u32AnimID == orxU32_UNDEFINED)
      {
        /* Removes it */
        _pstAnimPointer->u32TargetAnim = orxU32_UNDEFINED;

        /* Computes animpointer */
        eResult = orxAnimPointer_Compute(_pstAnimPointer, orxFLOAT_0);
      }
      /* In range? */
      else if(_u32AnimID < orxAnimSet_GetAnimCounter(_pstAnimPointer->pstAnimSet))
      {
        /* Stores ID */
        _pstAnimPointer->u32TargetAnim = _u32AnimID;

        /* Computes animpointer */
        eResult = orxAnimPointer_Compute(_pstAnimPointer, orxFLOAT_0);
      }
      else
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_ANIM, "%d is not a valid ID for the anim pointer.", _u32AnimID);

        /* Can't process */
        eResult = orxSTATUS_FAILURE;
      }
    }
  }
  else
  {
    /* Clears it */
    _pstAnimPointer->u32TargetAnim = orxU32_UNDEFINED;

    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_ANIM, "Anim pointer does not have a current anim.");

    /* Can't process */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** AnimPointer current Animation set accessor using name
 * @param[in]   _pstAnimPointer               Concerned AnimPointer
 * @param[in]   _zAnimName                    Animation name (config's name) to set
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxAnimPointer_SetCurrentAnimFromName(orxANIMPOINTER *_pstAnimPointer, const orxSTRING _zAnimName)
{
  orxU32    u32AnimID;
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(sstAnimPointer.u32Flags & orxANIMPOINTER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstAnimPointer);

  /* Gets corresponding anim ID */
  u32AnimID = ((_zAnimName != orxNULL) && (_zAnimName != orxSTRING_EMPTY)) ? orxAnimSet_GetAnimIDFromName(_pstAnimPointer->pstAnimSet, _zAnimName) : orxU32_UNDEFINED;

  /* Valid? */
  if(u32AnimID != orxU32_UNDEFINED)
  {
    /* Sets current anim */
    eResult = orxAnimPointer_SetCurrentAnim(_pstAnimPointer, u32AnimID);
  }

  /* Done! */
  return eResult;
}

/** AnimPointer target Animation set accessor using name
 * @param[in]   _pstAnimPointer               Concerned AnimPointer
 * @param[in]   _zAnimName                    Animation name (config's name) to set / orxNULL
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxAnimPointer_SetTargetAnimFromName(orxANIMPOINTER *_pstAnimPointer, const orxSTRING _zAnimName)
{
  orxU32    u32AnimID;
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(sstAnimPointer.u32Flags & orxANIMPOINTER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstAnimPointer);

  /* Gets corresponding anim ID */
  u32AnimID = ((_zAnimName != orxNULL) && (_zAnimName != orxSTRING_EMPTY)) ? orxAnimSet_GetAnimIDFromName(_pstAnimPointer->pstAnimSet, _zAnimName) : orxU32_UNDEFINED;

  /* Sets target anim */
  eResult = orxAnimPointer_SetTargetAnim(_pstAnimPointer, u32AnimID);

  /* Done! */
  return eResult;
}

/** AnimPointer current Time accessor
 * @param[in]   _pstAnimPointer               Concerned AnimPointer
 * @param[in]   _fTime                        Time to set
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxAnimPointer_SetTime(orxANIMPOINTER *_pstAnimPointer, orxFLOAT _fTime)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(sstAnimPointer.u32Flags & orxANIMPOINTER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstAnimPointer);

  /* Stores relative timestamp */
  _pstAnimPointer->fCurrentAnimTime = _fTime;

  /* Computes animpointer */
  eResult = orxAnimPointer_Compute(_pstAnimPointer, orxFLOAT_0);

  /* Done! */
  return eResult;
}

/** AnimPointer Frequency set accessor
 * @param[in]   _pstAnimPointer               Concerned AnimPointer
 * @param[in]   _fFrequency                   Frequency to set
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxAnimPointer_SetFrequency(orxANIMPOINTER *_pstAnimPointer, orxFLOAT _fFrequency)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(sstAnimPointer.u32Flags & orxANIMPOINTER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstAnimPointer);
  orxASSERT(_fFrequency >= orxDOUBLE_0);

  /* Stores frequency */
  _pstAnimPointer->fFrequency = _fFrequency;

  /* Computes animpointer */
  eResult = orxAnimPointer_Compute(_pstAnimPointer, orxFLOAT_0);

  /* Done! */
  return eResult;
}

/** AnimPointer pause accessor
 * @param[in]   _pstAnimPointer               Concerned AnimPointer
 * @param[in]   _bPause                       Pause / Unpause
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxAnimPointer_Pause(orxANIMPOINTER *_pstAnimPointer, orxBOOL _bPause)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstAnimPointer.u32Flags & orxANIMPOINTER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstAnimPointer);

  /* Pause? */
  if(_bPause != orxFALSE)
  {
    /* Updates status flags */
    orxStructure_SetFlags(_pstAnimPointer, orxANIMPOINTER_KU32_FLAG_PAUSED, orxANIMPOINTER_KU32_FLAG_NONE);
  }
  else
  {
    /* Updates status flags */
    orxStructure_SetFlags(_pstAnimPointer, orxANIMPOINTER_KU32_FLAG_NONE, orxANIMPOINTER_KU32_FLAG_PAUSED);
  }

  /* Done! */
  return eResult;
}


#ifdef __orxMSVC__

  #pragma warning(default : 4311 4312)

#endif /* __orxMSVC__ */
