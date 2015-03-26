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
 * @file orxFXPointer.c
 * @date 30/06/2008
 * @author iarwain@orx-project.org
 *
 */


#include "object/orxFXPointer.h"

#include "debug/orxDebug.h"
#include "debug/orxProfiler.h"
#include "memory/orxMemory.h"
#include "core/orxClock.h"
#include "core/orxEvent.h"
#include "object/orxStructure.h"
#include "object/orxObject.h"


/** Module flags
 */
#define orxFXPOINTER_KU32_STATIC_FLAG_NONE      0x00000000

#define orxFXPOINTER_KU32_STATIC_FLAG_READY     0x00000001

#define orxFXPOINTER_KU32_STATIC_MASK_ALL       0xFFFFFFFF


/** Flags
 */
#define orxFXPOINTER_KU32_FLAG_NONE             0x00000000  /**< No flags */

#define orxFXPOINTER_KU32_FLAG_ENABLED          0x10000000  /**< Enabled flag */

#define orxFXPOINTER_KU32_MASK_ALL              0xFFFFFFFF  /**< All mask */


/** Holder flags
 */
#define orxFXPOINTER_HOLDER_KU32_FLAG_NONE      0x00000000  /**< No flags */

#define orxFXPOINTER_HOLDER_KU32_FLAG_INTERNAL  0x10000000  /**< Internal flag */
#define orxFXPOINTER_HOLDER_KU32_FLAG_PLAYED    0x20000000  /**< Played flag */

#define orxFXPOINTER_HOLDER_KU32_MASK_ALL       0xFFFFFFFF  /**< All mask */


/** Misc defines
 */
#define orxFXPOINTER_KU32_BANK_SIZE             1024        /**< Bank size */


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** FXPointer holder structure
 */
typedef struct __orxFXPOINTER_HOLDER_t
{
  orxFX    *pstFX;                                          /**< FX reference : 4 */
  orxFLOAT  fStartTime;                                     /**< Start time : 8 */
  orxU32    u32Flags;                                       /**< Flags : 12 */

} orxFXPOINTER_HOLDER;

/** FXPointer structure
 */
struct __orxFXPOINTER_t
{
  orxSTRUCTURE            stStructure;                            /**< Public structure, first structure member : 32 */
  orxFXPOINTER_HOLDER     astFXList[orxFXPOINTER_KU32_FX_NUMBER]; /**< FX list : 112 */
  orxFLOAT                fTime;                                  /**< Time stamp : 116 */
};

/** Static structure
 */
typedef struct __orxFXPOINTER_STATIC_t
{
  orxU32 u32Flags;                                          /**< Control flags */

} orxFXPOINTER_STATIC;


/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** Static data
 */
static orxFXPOINTER_STATIC sstFXPointer;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

/** Deletes all the FXPointers
 */
static orxINLINE void orxFXPointer_DeleteAll()
{
  orxFXPOINTER *pstFXPointer;

  /* Gets first FXPointer */
  pstFXPointer = orxFXPOINTER(orxStructure_GetFirst(orxSTRUCTURE_ID_FXPOINTER));

  /* Non empty? */
  while(pstFXPointer != orxNULL)
  {
    /* Deletes it */
    orxFXPointer_Delete(pstFXPointer);

    /* Gets first FXPointer */
    pstFXPointer = orxFXPOINTER(orxStructure_GetFirst(orxSTRUCTURE_ID_FXPOINTER));
  }

  return;
}

/** Updates the FXPointer (Callback for generic structure update calling)
 * @param[in]   _pstStructure                 Generic Structure or the concerned Body
 * @param[in]   _pstCaller                    Structure of the caller
 * @param[in]   _pstClockInfo                 Clock info used for time updates
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
static orxSTATUS orxFASTCALL orxFXPointer_Update(orxSTRUCTURE *_pstStructure, const orxSTRUCTURE *_pstCaller, const orxCLOCK_INFO *_pstClockInfo)
{
  orxFXPOINTER *pstFXPointer;
  orxOBJECT    *pstObject;
  orxSTATUS     eResult = orxSTATUS_SUCCESS;

  /* Profiles */
  orxPROFILER_PUSH_MARKER("orxFXPointer_Update");

  /* Checks */
  orxASSERT(sstFXPointer.u32Flags & orxFXPOINTER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstStructure);
  orxSTRUCTURE_ASSERT(_pstCaller);

  /* Gets FXPointer */
  pstFXPointer = orxFXPOINTER(_pstStructure);

  /* Gets calling object */
  pstObject = orxOBJECT(_pstCaller);

  /* Is enabled? */
  if(orxFXPointer_IsEnabled(pstFXPointer) != orxFALSE)
  {
    orxFLOAT      fLastTime;
    orxU32        i;
    orxSTRUCTURE *pstOwner;

    /* Gets owner */
    pstOwner = orxStructure_GetOwner(pstFXPointer);

    /* Backups last time */
    fLastTime = pstFXPointer->fTime;

    /* Computes its new time cursor */
    pstFXPointer->fTime += _pstClockInfo->fDT;

    /* For all FXs */
    for(i = 0; i < orxFXPOINTER_KU32_FX_NUMBER; i++)
    {
      orxFX *pstFX;

      /* Gets FX */
      pstFX = pstFXPointer->astFXList[i].pstFX;

      /* Valid? */
      if(pstFX != orxNULL)
      {
        orxFLOAT fFXLocalStartTime, fFXLocalEndTime;

        /* Gets FX local times */
        fFXLocalStartTime = fLastTime - pstFXPointer->astFXList[i].fStartTime;
        fFXLocalEndTime   = pstFXPointer->fTime - pstFXPointer->astFXList[i].fStartTime;

        /* Is the FX reached? */
        if(fFXLocalEndTime >= orxFLOAT_0)
        {
          /* Is the first time? */
          if(!orxFLAG_TEST(pstFXPointer->astFXList[i].u32Flags, orxFXPOINTER_HOLDER_KU32_FLAG_PLAYED))
          {
            orxFX_EVENT_PAYLOAD stPayload;

            /* Inits event payload */
            orxMemory_Zero(&stPayload, sizeof(orxFX_EVENT_PAYLOAD));
            stPayload.pstFX   = pstFX;
            stPayload.zFXName = orxFX_GetName(pstFX);

            /* Sends event */
            orxEVENT_SEND(orxEVENT_TYPE_FX, orxFX_EVENT_START, pstOwner, pstOwner, &stPayload);
          }

          /* Updates its status */
          orxFLAG_SET(pstFXPointer->astFXList[i].u32Flags, orxFXPOINTER_HOLDER_KU32_FLAG_PLAYED, orxFXPOINTER_HOLDER_KU32_FLAG_NONE);

          /* Applies FX from last time to now */
          if(orxFX_Apply(pstFX, pstObject, fFXLocalStartTime, fFXLocalEndTime) == orxSTATUS_FAILURE)
          {
            orxFX_EVENT_PAYLOAD stPayload;

            /* Inits event payload */
            orxMemory_Zero(&stPayload, sizeof(orxFX_EVENT_PAYLOAD));
            stPayload.pstFX   = pstFX;
            stPayload.zFXName = orxFX_GetName(pstFX);

            /* Is a looping FX? */
            if(orxFX_IsLooping(pstFX) != orxFALSE)
            {
              /* Sends event */
              orxEVENT_SEND(orxEVENT_TYPE_FX, orxFX_EVENT_LOOP, pstOwner, pstOwner, &stPayload);

              /* Updates its start time */
              pstFXPointer->astFXList[i].fStartTime = pstFXPointer->fTime;
            }
            else
            {
              /* Decreases its reference counter */
              orxStructure_DecreaseCounter(pstFX);

              /* Removes its reference */
              pstFXPointer->astFXList[i].pstFX = orxNULL;

              /* Sends event */
              orxEVENT_SEND(orxEVENT_TYPE_FX, orxFX_EVENT_STOP, pstOwner, pstOwner, &stPayload);

              /* Sends event */
              orxEVENT_SEND(orxEVENT_TYPE_FX, orxFX_EVENT_REMOVE, pstOwner, pstOwner, &stPayload);

              /* Is internal? */
              if(orxFLAG_TEST(pstFXPointer->astFXList[i].u32Flags, orxFXPOINTER_HOLDER_KU32_FLAG_INTERNAL))
              {
                /* Removes its owner */
                orxStructure_SetOwner(pstFX, orxNULL);

                /* Deletes it */
                orxFX_Delete(pstFX);
              }
            }
          }
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

/** FXPointer module setup
 */
void orxFASTCALL orxFXPointer_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_FXPOINTER, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_FXPOINTER, orxMODULE_ID_STRUCTURE);
  orxModule_AddDependency(orxMODULE_ID_FXPOINTER, orxMODULE_ID_PROFILER);
  orxModule_AddDependency(orxMODULE_ID_FXPOINTER, orxMODULE_ID_CLOCK);
  orxModule_AddDependency(orxMODULE_ID_FXPOINTER, orxMODULE_ID_CONFIG);
  orxModule_AddDependency(orxMODULE_ID_FXPOINTER, orxMODULE_ID_EVENT);
  orxModule_AddDependency(orxMODULE_ID_FXPOINTER, orxMODULE_ID_FX);

  return;
}

/** Inits the FXPointer module
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxFXPointer_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Not already Initialized? */
  if(!(sstFXPointer.u32Flags & orxFXPOINTER_KU32_STATIC_FLAG_READY))
  {
    /* Cleans static controller */
    orxMemory_Zero(&sstFXPointer, sizeof(orxFXPOINTER_STATIC));

    /* Registers structure type */
    eResult = orxSTRUCTURE_REGISTER(FXPOINTER, orxSTRUCTURE_STORAGE_TYPE_LINKLIST, orxMEMORY_TYPE_MAIN, orxFXPOINTER_KU32_BANK_SIZE, &orxFXPointer_Update);

    /* Initialized? */
    if(eResult != orxSTATUS_FAILURE)
    {
      /* Inits Flags */
      sstFXPointer.u32Flags = orxFXPOINTER_KU32_STATIC_FLAG_READY;
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Failed to register linked list structure.");
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Tried to load FX pointer module when it was already initialized.");

    /* Already initialized */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Done! */
  return eResult;
}

/** Exits from the FXPointer module
 */
void orxFASTCALL orxFXPointer_Exit()
{
  /* Initialized? */
  if(sstFXPointer.u32Flags & orxFXPOINTER_KU32_STATIC_FLAG_READY)
  {
    /* Deletes FXPointer list */
    orxFXPointer_DeleteAll();

    /* Unregisters structure type */
    orxStructure_Unregister(orxSTRUCTURE_ID_FXPOINTER);

    /* Updates flags */
    sstFXPointer.u32Flags &= ~orxFXPOINTER_KU32_STATIC_FLAG_READY;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Tried to exit FX pointer module when it wasn't initialized.");
  }

  return;
}

/** Creates an empty FXPointer
 * @return      Created orxFXPOINTER / orxNULL
 */
orxFXPOINTER *orxFASTCALL orxFXPointer_Create()
{
  orxFXPOINTER *pstResult;

  /* Checks */
  orxASSERT(sstFXPointer.u32Flags & orxFXPOINTER_KU32_STATIC_FLAG_READY);

  /* Creates FXPointer */
  pstResult = orxFXPOINTER(orxStructure_Create(orxSTRUCTURE_ID_FXPOINTER));

  /* Created? */
  if(pstResult != orxNULL)
  {
    /* Inits flags */
    orxStructure_SetFlags(pstResult, orxFXPOINTER_KU32_FLAG_ENABLED, orxFXPOINTER_KU32_MASK_ALL);

    /* Increases counter */
    orxStructure_IncreaseCounter(pstResult);
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Failed to create FX pointer structure.");
  }

  /* Done! */
  return pstResult;
}

/** Deletes an FXPointer
 * @param[in] _pstFXPointer     Concerned FXPointer
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxFXPointer_Delete(orxFXPOINTER *_pstFXPointer)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstFXPointer.u32Flags & orxFXPOINTER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstFXPointer);

  /* Decreases counter */
  orxStructure_DecreaseCounter(_pstFXPointer);

  /* Not referenced? */
  if(orxStructure_GetRefCounter(_pstFXPointer) == 0)
  {
    orxU32 i;

    /* For all FXs */
    for(i = 0; i < orxFXPOINTER_KU32_FX_NUMBER; i++)
    {
      /* Valid? */
      if(_pstFXPointer->astFXList[i].pstFX != orxNULL)
      {
        /* Decreases its reference counter */
        orxStructure_DecreaseCounter(_pstFXPointer->astFXList[i].pstFX);

        /* Is internal? */
        if(orxFLAG_TEST(_pstFXPointer->astFXList[i].u32Flags, orxFXPOINTER_HOLDER_KU32_FLAG_INTERNAL))
        {
          /* Removes its owner */
          orxStructure_SetOwner(_pstFXPointer->astFXList[i].pstFX, orxNULL);

          /* Deletes it */
          orxFX_Delete(_pstFXPointer->astFXList[i].pstFX);
        }
      }
    }

    /* Deletes structure */
    orxStructure_Delete(_pstFXPointer);
  }
  else
  {
    /* Referenced by others */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Enables/disables an FXPointer
 * @param[in]   _pstFXPointer Concerned FXPointer
 * @param[in]   _bEnable      enable / disable
 */
void orxFASTCALL    orxFXPointer_Enable(orxFXPOINTER *_pstFXPointer, orxBOOL _bEnable)
{
  /* Checks */
  orxASSERT(sstFXPointer.u32Flags & orxFXPOINTER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstFXPointer);

  /* Enable? */
  if(_bEnable != orxFALSE)
  {
    /* Updates status flags */
    orxStructure_SetFlags(_pstFXPointer, orxFXPOINTER_KU32_FLAG_ENABLED, orxFXPOINTER_KU32_FLAG_NONE);
  }
  else
  {
    /* Updates status flags */
    orxStructure_SetFlags(_pstFXPointer, orxFXPOINTER_KU32_FLAG_NONE, orxFXPOINTER_KU32_FLAG_ENABLED);
  }

  return;
}

/** Is FXPointer enabled?
 * @param[in]   _pstFXPointer Concerned FXPointer
 * @return      orxTRUE if enabled, orxFALSE otherwise
 */
orxBOOL orxFASTCALL orxFXPointer_IsEnabled(const orxFXPOINTER *_pstFXPointer)
{
  /* Checks */
  orxASSERT(sstFXPointer.u32Flags & orxFXPOINTER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstFXPointer);

  /* Done! */
  return(orxStructure_TestFlags(_pstFXPointer, orxFXPOINTER_KU32_FLAG_ENABLED));
}

/** Adds an FX
 * @param[in]   _pstFXPointer Concerned FXPointer
 * @param[in]   _pstFX        FX to add
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxFXPointer_AddFX(orxFXPOINTER *_pstFXPointer, orxFX *_pstFX)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(sstFXPointer.u32Flags & orxFXPOINTER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstFXPointer);
  orxSTRUCTURE_ASSERT(_pstFX);

  /* Adds FX */
  eResult = orxFXPointer_AddDelayedFX(_pstFXPointer, _pstFX, orxFLOAT_0);

  /* Done! */
  return eResult;
}

/** Adds a delayed FX
 * @param[in]   _pstFXPointer Concerned FXPointer
 * @param[in]   _pstFX        FX to add
 * @param[in]   _fDelay       Delay time
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxFXPointer_AddDelayedFX(orxFXPOINTER *_pstFXPointer, orxFX *_pstFX, orxFLOAT _fDelay)
{
  orxU32    u32Index;
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(sstFXPointer.u32Flags & orxFXPOINTER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstFXPointer);
  orxSTRUCTURE_ASSERT(_pstFX);
  orxASSERT(_fDelay >= orxFLOAT_0);

  /* Finds an empty slot */
  for(u32Index = 0; (u32Index < orxFXPOINTER_KU32_FX_NUMBER) && (_pstFXPointer->astFXList[u32Index].pstFX != orxNULL); u32Index++);

  /* Found? */
  if(u32Index < orxFXPOINTER_KU32_FX_NUMBER)
  {
    orxSTRUCTURE       *pstOwner;
    orxFX_EVENT_PAYLOAD stPayload;

    /* Gets owner */
    pstOwner = orxStructure_GetOwner(_pstFXPointer);

    /* Increases its reference counter */
    orxStructure_IncreaseCounter(_pstFX);

    /* Adds it to holder */
    _pstFXPointer->astFXList[u32Index].pstFX = _pstFX;

    /* Inits its start time */
    _pstFXPointer->astFXList[u32Index].fStartTime = _pstFXPointer->fTime + _fDelay;

    /* Updates its flags */
    orxFLAG_SET(_pstFXPointer->astFXList[u32Index].u32Flags, orxFXPOINTER_HOLDER_KU32_FLAG_NONE, orxFXPOINTER_HOLDER_KU32_MASK_ALL);

    /* Inits event payload */
    orxMemory_Zero(&stPayload, sizeof(orxFX_EVENT_PAYLOAD));
    stPayload.pstFX   = _pstFX;
    stPayload.zFXName = orxFX_GetName(_pstFX);

    /* Sends event */
    orxEVENT_SEND(orxEVENT_TYPE_FX, orxFX_EVENT_ADD, pstOwner, pstOwner, &stPayload);

    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "No available slots for FX.");

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Removes an FX
 * @param[in]   _pstFXPointer Concerned FXPointer
 * @param[in]   _pstFX        FX to remove
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxFXPointer_RemoveFX(orxFXPOINTER *_pstFXPointer, orxFX *_pstFX)
{
  orxU32        i;
  orxSTRUCTURE *pstOwner;
  orxSTATUS     eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(sstFXPointer.u32Flags & orxFXPOINTER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstFXPointer);
  orxSTRUCTURE_ASSERT(_pstFX);

  /* Gets owner */
  pstOwner = orxStructure_GetOwner(_pstFXPointer);

  /* For all slots */
  for(i = 0; i < orxFXPOINTER_KU32_FX_NUMBER; i++)
  {
    orxFX *pstFX;

    /* Gets FX */
    pstFX = _pstFXPointer->astFXList[i].pstFX;

    /* Valid? */
    if(pstFX != orxNULL)
    {
      /* Found? */
      if(pstFX == _pstFX)
      {
        orxFX_EVENT_PAYLOAD stPayload;

        /* Decreases its reference counter */
        orxStructure_DecreaseCounter(pstFX);

        /* Removes its reference */
        _pstFXPointer->astFXList[i].pstFX = orxNULL;

        /* Inits event payload */
        orxMemory_Zero(&stPayload, sizeof(orxFX_EVENT_PAYLOAD));
        stPayload.pstFX   = pstFX;
        stPayload.zFXName = orxFX_GetName(pstFX);

        /* Sends event */
        orxEVENT_SEND(orxEVENT_TYPE_FX, orxFX_EVENT_REMOVE, pstOwner, pstOwner, &stPayload);

        /* Is internal? */
        if(orxFLAG_TEST(_pstFXPointer->astFXList[i].u32Flags, orxFXPOINTER_HOLDER_KU32_FLAG_INTERNAL))
        {
          /* Removes its owner */
          orxStructure_SetOwner(pstFX, orxNULL);

          /* Deletes it */
          orxFX_Delete(pstFX);
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

/** Adds an FX using its config ID
 * @param[in]   _pstFXPointer Concerned FXPointer
 * @param[in]   _zFXConfigID  Config ID of the FX to add
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxFXPointer_AddFXFromConfig(orxFXPOINTER *_pstFXPointer, const orxSTRING _zFXConfigID)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(sstFXPointer.u32Flags & orxFXPOINTER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstFXPointer);
  orxASSERT((_zFXConfigID != orxNULL) && (_zFXConfigID != orxSTRING_EMPTY));

  /* Adds FX */
  eResult = orxFXPointer_AddDelayedFXFromConfig(_pstFXPointer, _zFXConfigID, orxFLOAT_0);

  /* Done! */
  return eResult;
}

/** Adds a unique FX using its config ID
 * @param[in]   _pstFXPointer Concerned FXPointer
 * @param[in]   _zFXConfigID  Config ID of the FX to add
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxFXPointer_AddUniqueFXFromConfig(orxFXPOINTER *_pstFXPointer, const orxSTRING _zFXConfigID)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(sstFXPointer.u32Flags & orxFXPOINTER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstFXPointer);
  orxASSERT((_zFXConfigID != orxNULL) && (_zFXConfigID != orxSTRING_EMPTY));

  /* Adds FX */
  eResult = orxFXPointer_AddUniqueDelayedFXFromConfig(_pstFXPointer, _zFXConfigID, orxFLOAT_0);

  /* Done! */
  return eResult;
}

/** Adds a delayed FX using its config ID
 * @param[in]   _pstFXPointer Concerned FXPointer
 * @param[in]   _zFXConfigID  Config ID of the FX to add
 * @param[in]   _fDelay       Delay time
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxFXPointer_AddDelayedFXFromConfig(orxFXPOINTER *_pstFXPointer, const orxSTRING _zFXConfigID, orxFLOAT _fDelay)
{
  orxU32    u32Index;
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(sstFXPointer.u32Flags & orxFXPOINTER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstFXPointer);
  orxASSERT((_zFXConfigID != orxNULL) && (_zFXConfigID != orxSTRING_EMPTY));
  orxASSERT(_fDelay >= orxFLOAT_0);

  /* Finds an empty slot */
  for(u32Index = 0; (u32Index < orxFXPOINTER_KU32_FX_NUMBER) && (_pstFXPointer->astFXList[u32Index].pstFX != orxNULL); u32Index++);

  /* Found? */
  if(u32Index < orxFXPOINTER_KU32_FX_NUMBER)
  {
    orxFX *pstFX;

    /* Creates FX */
    pstFX = orxFX_CreateFromConfig(_zFXConfigID);

    /* Valid? */
    if(pstFX != orxNULL)
    {
      orxSTRUCTURE       *pstOwner;
      orxFX_EVENT_PAYLOAD stPayload;

      /* Gets owner */
      pstOwner = orxStructure_GetOwner(_pstFXPointer);

      /* Increases its reference counter */
      orxStructure_IncreaseCounter(pstFX);

      /* Adds it to holder */
      _pstFXPointer->astFXList[u32Index].pstFX = pstFX;

      /* Inits its start time */
      _pstFXPointer->astFXList[u32Index].fStartTime = _pstFXPointer->fTime + _fDelay;

      /* Updates its owner */
      orxStructure_SetOwner(pstFX, _pstFXPointer);

      /* Updates its flags */
      orxFLAG_SET(_pstFXPointer->astFXList[u32Index].u32Flags, orxFXPOINTER_HOLDER_KU32_FLAG_INTERNAL, orxFXPOINTER_HOLDER_KU32_MASK_ALL);

      /* Inits event payload */
      orxMemory_Zero(&stPayload, sizeof(orxFX_EVENT_PAYLOAD));
      stPayload.pstFX   = pstFX;
      stPayload.zFXName = orxFX_GetName(pstFX);

      /* Sends event */
      orxEVENT_SEND(orxEVENT_TYPE_FX, orxFX_EVENT_ADD, pstOwner, pstOwner, &stPayload);

      /* Updates result */
      eResult = orxSTATUS_SUCCESS;
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Loading FX <%s> from config failed.", _zFXConfigID);

      /* Updates result */
      eResult = orxSTATUS_FAILURE;
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "Failed to find an empty slot to put FX <%s> into.", _zFXConfigID);

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Adds a unique delayed FX using its config ID
 * @param[in]   _pstFXPointer Concerned FXPointer
 * @param[in]   _zFXConfigID  Config ID of the FX to add
 * @param[in]   _fDelay       Delay time
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxFXPointer_AddUniqueDelayedFXFromConfig(orxFXPOINTER *_pstFXPointer, const orxSTRING _zFXConfigID, orxFLOAT _fDelay)
{
  orxU32    i, u32ID;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstFXPointer.u32Flags & orxFXPOINTER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstFXPointer);
  orxASSERT((_zFXConfigID != orxNULL) && (_zFXConfigID != orxSTRING_EMPTY));
  orxASSERT(_fDelay >= orxFLOAT_0);

  /* Gets ID */
  u32ID = orxString_ToCRC(_zFXConfigID);

  /* For all slots */
  for(i = 0; i < orxFXPOINTER_KU32_FX_NUMBER; i++)
  {
    orxFX *pstFX;

    /* Gets FX */
    pstFX = _pstFXPointer->astFXList[i].pstFX;

    /* Valid? */
    if(pstFX != orxNULL)
    {
      /* Found? */
      if(orxString_ToCRC(orxFX_GetName(pstFX)) == u32ID)
      {
        /* Updates result */
        eResult = orxSTATUS_FAILURE;

        break;
      }
    }
  }

  /* Success? */
  if(eResult != orxSTATUS_FAILURE)
  {
    /* Adds delayed FX */
    eResult = orxFXPointer_AddDelayedFXFromConfig(_pstFXPointer, _zFXConfigID, _fDelay);
  }

  /* Done! */
  return eResult;
}

/** Removes an FX using its config ID
 * @param[in]   _pstFXPointer Concerned FXPointer
 * @param[in]   _zFXConfigID  Config ID of the FX to remove
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxFXPointer_RemoveFXFromConfig(orxFXPOINTER *_pstFXPointer, const orxSTRING _zFXConfigID)
{
  orxU32        i, u32ID;
  orxSTRUCTURE *pstOwner;
  orxSTATUS     eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(sstFXPointer.u32Flags & orxFXPOINTER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstFXPointer);
  orxASSERT((_zFXConfigID != orxNULL) && (_zFXConfigID != orxSTRING_EMPTY));

  /* Gets owner */
  pstOwner = orxStructure_GetOwner(_pstFXPointer);

  /* Gets ID */
  u32ID = orxString_ToCRC(_zFXConfigID);

  /* For all slots */
  for(i = 0; i < orxFXPOINTER_KU32_FX_NUMBER; i++)
  {
    orxFX *pstFX;

    /* Gets FX */
    pstFX = _pstFXPointer->astFXList[i].pstFX;

    /* Valid? */
    if(pstFX != orxNULL)
    {
      /* Found? */
      if(orxString_ToCRC(orxFX_GetName(pstFX)) == u32ID)
      {
        orxFX_EVENT_PAYLOAD stPayload;

        /* Decreases its reference counter */
        orxStructure_DecreaseCounter(pstFX);

        /* Removes its reference */
        _pstFXPointer->astFXList[i].pstFX = orxNULL;

        /* Inits event payload */
        orxMemory_Zero(&stPayload, sizeof(orxFX_EVENT_PAYLOAD));
        stPayload.pstFX   = pstFX;
        stPayload.zFXName = orxFX_GetName(pstFX);

        /* Sends event */
        orxEVENT_SEND(orxEVENT_TYPE_FX, orxFX_EVENT_REMOVE, pstOwner, pstOwner, &stPayload);

        /* Is internal? */
        if(orxFLAG_TEST(_pstFXPointer->astFXList[i].u32Flags, orxFXPOINTER_HOLDER_KU32_FLAG_INTERNAL))
        {
          /* Removes its owner */
          orxStructure_SetOwner(pstFX, orxNULL);

          /* Deletes it */
          orxFX_Delete(pstFX);
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

/** Synchronizes FX times with an other orxFXPointer if they share common FXs
 * @param[in]   _pstFXPointer Concerned FXPointer
 * @param[in]   _pstModel     Model FX pointer to use for synchronization
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxFXPointer_Synchronize(orxFXPOINTER *_pstFXPointer, const orxFXPOINTER *_pstModel)
{
  orxS32    i;
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(sstFXPointer.u32Flags & orxFXPOINTER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstFXPointer);
  orxSTRUCTURE_ASSERT(_pstModel);

  /* For all FXs */
  for(i = 0; i < orxFXPOINTER_KU32_FX_NUMBER; i++)
  {
    orxFXPOINTER_HOLDER *pstFX;

    /* Gets it */
    pstFX = &_pstFXPointer->astFXList[i];

    /* Valid? */
    if(pstFX != orxNULL)
    {
      orxS32 j;

      /* For all FXs on model */
      for(j = 0; j < orxFXPOINTER_KU32_FX_NUMBER; j++)
      {
        const orxFXPOINTER_HOLDER *pstModelFX;

        /* Gets it */
        pstModelFX = &_pstModel->astFXList[j];

        /* Valid? */
        if(pstModelFX != orxNULL)
        {
          /* Matches? */
          if(pstModelFX->pstFX == pstFX->pstFX)
          {
            /* Synchronizes start time */
            pstFX->fStartTime = pstModelFX->fStartTime;

            /* Updates result */
            eResult = orxSTATUS_SUCCESS;

            break;
          }
        }
      }

      /* Not found? */
      if(j == orxFXPOINTER_KU32_FX_NUMBER)
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Couldn't synchronize FX <%s> as it wasn't found on model.", orxFX_GetName(pstFX->pstFX));
      }
    }
  }

  /* Succes? */
  if(eResult != orxSTATUS_FAILURE)
  {
    /* Updates global time */
    _pstFXPointer->fTime = _pstModel->fTime;
  }

  /* Done! */
  return eResult;
}

/** FXPointer time get accessor
 * @param[in]   _pstFXPointer Concerned FXPointer
 * @return      orxFLOAT
 */
orxFLOAT orxFASTCALL orxFXPointer_GetTime(const orxFXPOINTER *_pstFXPointer)
{
  orxFLOAT fResult;

  /* Checks */
  orxASSERT(sstFXPointer.u32Flags & orxFXPOINTER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstFXPointer);

  /* Updates result */
  fResult = _pstFXPointer->fTime;

  /* Done! */
  return fResult;
}

/** FXPointer time set accessor
 * @param[in]   _pstFXPointer Concerned FXPointer
 * @param[in]   _fTime        Time to set
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxFXPointer_SetTime(orxFXPOINTER *_pstFXPointer, orxFLOAT _fTime)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(sstFXPointer.u32Flags & orxFXPOINTER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstFXPointer);

  /* Valid? */
  if(_fTime >= orxFLOAT_0)
  {
    /* Stores timestamp */
    _pstFXPointer->fTime = _fTime;

    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }
  else
  {
    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}
