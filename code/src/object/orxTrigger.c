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
 * @file orxTrigger.c
 * @date 05/07/2024
 * @author iarwain@orx-project.org
 *
 */


#include "object/orxTrigger.h"

#include "core/orxCommand.h"
#include "core/orxConfig.h"
#include "core/orxEvent.h"
#include "core/orxResource.h"
#include "debug/orxDebug.h"
#include "debug/orxProfiler.h"
#include "memory/orxBank.h"
#include "memory/orxMemory.h"
#include "object/orxObject.h"
#include "object/orxStructure.h"
#include "utils/orxHashTable.h"
#include "utils/orxString.h"

#ifdef __orxMSVC__

  #pragma warning(push)
  #pragma warning(disable : 4200)

  #include <malloc.h>

#endif /* __orxMSVC__ */


/** Module flags
 */
#define orxTRIGGER_KU32_STATIC_FLAG_NONE              0x00000000

#define orxTRIGGER_KU32_STATIC_FLAG_READY             0x00000001

#define orxTRIGGER_KU32_STATIC_MASK_ALL               0xFFFFFFFF


/** Flags
 */
#define orxTRIGGER_KU32_FLAG_NONE                     0x00000000  /**< No flags */

#define orxTRIGGER_KU32_FLAG_ENABLED                  0x10000000  /**< Enabled flag */

#define orxTRIGGER_KU32_MASK_ALL                      0xFFFFFFFF  /**< All mask */

/** Set flags
 */
#define orxTRIGGER_SET_KU32_FLAG_NONE                 0x00000000  /**< No flag */

#define orxTRIGGER_SET_KU32_FLAG_CACHED               0x10000000  /**< Cached flag */

#define orxTRIGGER_SET_KU32_FLAG_BACKUP               0x10000000  /**< Backup flag */

#define orxTRIGGER_SET_KU32_MASK_ALL                  0xFFFFFFFF  /**< All mask */


/** Misc defines
 */
#define orxTRIGGER_KU32_TABLE_SIZE                    256         /**< Table size */
#define orxTRIGGER_KU32_BANK_SIZE                     256         /**< Bank size */

#define orxTRIGGER_KU32_BUFFER_SIZE                   1024        /**< Buffer size */

#define orxTRIGGER_KU32_SET_NUMBER                    16          /**< Set number */

#define orxTRIGGER_KZ_CONFIG_KEEP_IN_CACHE            "KeepInCache"


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Trigger Set Event
 */
typedef struct __orxTRIGGER_SET_EVENT_t
{
  orxSTRINGID               stID;                     /**< Event ID : 8 */
  const orxSTRING           zValue;                   /**< Event value : 12 / 16 */
  orxU32                    u32StopDepth;             /**< Event stop depth : 16 / 20 */

} orxTRIGGER_SET_EVENT;

/** Trigger Set
 */
typedef struct __orxTRIGGER_SET_t
{
  orxSTRINGID               stID;                     /**< ID : 8 */
  const orxSTRING           zReference;               /**< Set reference : 12 / 16 */
  orxU32                    u32RefCount;              /**< Reference count : 16 / 20 */
  orxU32                    u32EventCount;            /**< Event count : 20 / 24 */
  orxU32                    u32Flags;                 /**< Flags: 24 / 28 */
  orxTRIGGER_SET_EVENT      astEventList[0];          /**< Set event list */

} orxTRIGGER_SET;

/** Trigger structure
 */
struct __orxTRIGGER_t
{
  orxSTRUCTURE              stStructure;              /**< Public structure, first structure member : 32 / 64 */
  orxTRIGGER_SET           *pastSetList[orxTRIGGER_KU32_SET_NUMBER]; /**< Trigger set list : 64 / 128 */
};

/** Static structure
 */
typedef struct __orxTRIGGER_STATIC_t
{
  orxHASHTABLE             *pstSetTable;              /**< Set hash table */
  const orxSTRING           zCurrentEvent;            /**< Current event */
  const orxSTRING          *azCurrentRefinementList;  /**< Current list of refinements */
  orxU32                    u32CurrentRefinementListCount; /**< Current list of refinements' size */
  orxU32                    u32Flags;                 /**< Control flags */

} orxTRIGGER_STATIC;


/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** Static data
 */
static orxTRIGGER_STATIC sstTrigger;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

static orxBOOL orxFASTCALL orxTrigger_Count(const orxSTRING _zKeyName, const orxSTRING _zSectionName, void *_pContext)
{
  orxU32 *pu32Counter;

  /* Gets counter */
  pu32Counter = (orxU32 *)_pContext;

  /* Increases it */
  (*pu32Counter) += (orxU32)orxConfig_GetListCount(_zKeyName);

  /* Done! */
  return orxTRUE;
}

static orxBOOL orxFASTCALL orxTrigger_AddEvent(const orxSTRING _zKeyName, const orxSTRING _zSectionName, void *_pContext)
{
  orxSTRINGID     stEventID;
  orxTRIGGER_SET *pstTriggerSet;
  const orxCHAR  *pcSrc;
  orxCHAR        *pcDst;
  orxU32          u32StopDepth, u32Depth;
  orxS32          s32ListCount, i;
  orxCHAR         acBuffer[orxTRIGGER_KU32_BUFFER_SIZE];

  /* Gets trigger set */
  pstTriggerSet = (orxTRIGGER_SET *)_pContext;

  /* For all characters */
  for(pcSrc = _zKeyName, pcDst = acBuffer, u32StopDepth = 0, u32Depth = 0;
      (*pcSrc != orxCHAR_NULL) && (pcDst - acBuffer < sizeof(acBuffer) - 1);
      pcSrc++)
  {
    /* Depending on character */
    switch(*pcSrc)
    {
      case orxTRIGGER_KC_STOP_MARKER:
      {
        /* Checks */
        if((u32StopDepth != 0) || (u32Depth == 0))
        {
          orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "[%s] Invalid stop markers found for event <%s>.", orxString_GetFromID(pstTriggerSet->stID), _zKeyName);
        }

        /* Updates stop depth */
        u32StopDepth = u32Depth;

        break;
      }

      case orxTRIGGER_KC_SEPARATOR:
      {
        /* Updates depth */
        u32Depth++;

        /* Fall through */
      }

      default:
      {
        /* Copies it */
        *(pcDst++) = *pcSrc;

        break;
      }
    }
  }

  /* Terminates buffer */
  *pcDst = orxCHAR_NULL;

  /* Gets event ID */
  stEventID = orxString_Hash(acBuffer);

  /* For all associated values */
  for(i = 0, s32ListCount = orxConfig_GetListCount(_zKeyName);
      i < s32ListCount;
      i++, (pstTriggerSet->u32EventCount)++)
  {
    /* Stores it */
    pstTriggerSet->astEventList[pstTriggerSet->u32EventCount].stID          = stEventID;
    pstTriggerSet->astEventList[pstTriggerSet->u32EventCount].zValue        = orxString_Store(orxConfig_GetListString(_zKeyName, i));
    pstTriggerSet->astEventList[pstTriggerSet->u32EventCount].u32StopDepth  = u32StopDepth;
  }

  /* Done! */
  return orxTRUE;
}

/** Creates a set
 */
static orxINLINE orxTRIGGER_SET *orxTrigger_CreateSet(const orxSTRING _zConfigID)
{
  orxTRIGGER_SET *pstResult = orxNULL;

  /* Pushes section */
  if((orxConfig_HasSection(_zConfigID) != orxFALSE)
  && (orxConfig_PushSection(_zConfigID) != orxSTATUS_FAILURE))
  {
    orxU32 u32EventCount = 0;

    /* Gets event count */
    orxConfig_ForAllKeys(orxTrigger_Count, orxTRUE, &u32EventCount);

    /* Valid? */
    if(u32EventCount > 0)
    {
      /* Allocates set */
      pstResult = (orxTRIGGER_SET *)orxMemory_Allocate(sizeof(orxTRIGGER_SET) + (u32EventCount * sizeof(orxTRIGGER_SET_EVENT)), orxMEMORY_TYPE_MAIN);

      /* Valid? */
      if(pstResult != orxNULL)
      {
        /* Stores its ID */
        pstResult->stID = orxString_GetID(orxConfig_GetCurrentSection());

        /* Inits its event count */
        pstResult->u32EventCount = 0;

        /* Adds it to reference table */
        if(orxHashTable_Set(sstTrigger.pstSetTable, pstResult->stID, pstResult) != orxSTATUS_FAILURE)
        {
          orxU32 u32Flags = orxTRIGGER_SET_KU32_FLAG_NONE;

          /* Adds all events */
          orxConfig_ForAllKeys(orxTrigger_AddEvent, orxTRUE, pstResult);

          /* Checks */
          orxASSERT(pstResult->u32EventCount == u32EventCount);

          /* Stores its reference */
          pstResult->zReference = orxString_GetFromID(pstResult->stID);

          /* Updates its ref count */
          pstResult->u32RefCount = 1;

          /* Should keep in cache? */
          if(orxConfig_GetBool(orxTRIGGER_KZ_CONFIG_KEEP_IN_CACHE) != orxFALSE)
          {
            /* Increases its reference count to keep it in cache table */
            pstResult->u32RefCount++;

            /* Updates flags */
            u32Flags |= orxTRIGGER_SET_KU32_FLAG_CACHED;
          }

          /* Stores flags */
          pstResult->u32Flags = u32Flags;
        }
        else
        {
          /* Logs message */
          orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Failed to add set to hashtable.");

          /* Deletes it */
          orxMemory_Free(pstResult);

          /* Updates result */
          pstResult = orxNULL;
        }
      }
      else
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Couldn't create Trigger set [%s]: memory allocation failure.", _zConfigID);
      }
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Couldn't create Trigger set [%s]: config section is empty.", _zConfigID);
    }

    /* Pops previous section */
    orxConfig_PopSection();
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Couldn't create Trigger set [%s]: config section not found.", _zConfigID);
  }

  /* Done! */
  return pstResult;
}

/** Deletes a set
 */
static orxINLINE void orxTrigger_DeleteSet(orxTRIGGER_SET *_pstSet)
{
  /* Decreases count */
  _pstSet->u32RefCount--;

  /* Not referenced? */
  if(_pstSet->u32RefCount == 0)
  {
    /* Has an ID? */
    if((_pstSet->zReference != orxNULL)
    && (_pstSet->zReference != orxSTRING_EMPTY))
    {
      /* Removes it from the table */
      orxHashTable_Remove(sstTrigger.pstSetTable, _pstSet->stID);
    }

    /* Deletes it */
    orxMemory_Free(_pstSet);
  }

  /* Done! */
  return;
}

/** Event handler
 */
static orxSTATUS orxFASTCALL orxTrigger_EventHandler(const orxEVENT *_pstEvent)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Resource event? */
  if(_pstEvent->eType == orxEVENT_TYPE_RESOURCE)
  {
    orxRESOURCE_EVENT_PAYLOAD *pstPayload;

    /* Gets payload */
    pstPayload = (orxRESOURCE_EVENT_PAYLOAD *)_pstEvent->pstPayload;

    /* Is config group? */
    if(pstPayload->stGroupID == orxString_Hash(orxCONFIG_KZ_RESOURCE_GROUP))
    {
      orxHANDLE             hIterator;
      orxTRIGGER_SET *pstEventSet;

      /* For all sets */
      for(hIterator = orxHashTable_GetNext(sstTrigger.pstSetTable, orxHANDLE_UNDEFINED, orxNULL, (void **)&pstEventSet);
          hIterator != orxHANDLE_UNDEFINED;
          hIterator = orxHashTable_GetNext(sstTrigger.pstSetTable, hIterator, orxNULL, (void **)&pstEventSet))
      {
        /* Match origin? */
        if(orxConfig_GetOriginID(pstEventSet->zReference) == pstPayload->stNameID)
        {
          orxTRIGGER           *pstTrigger;
          orxTRIGGER_SET *pstNewEventSet;
          orxSTRINGID           stID;
          orxU32                u32Count, u32Flags;
          const orxSTRING       zReference;

          /* Backups count, ID, flags & reference */
          u32Count    = pstEventSet->u32RefCount;
          stID        = pstEventSet->stID;
          u32Flags    = orxFLAG_GET(pstEventSet->u32Flags, orxTRIGGER_SET_KU32_FLAG_BACKUP);
          zReference  = pstEventSet->zReference;

          /* Deletes it (but keeps its reference in the hashtable to prevent infinite loop upon table changes) */
          orxMemory_Free(pstEventSet);

          /* Creates new set */
          pstNewEventSet = orxTrigger_CreateSet(zReference);

          /* Success? */
          if(pstNewEventSet != orxNULL)
          {
            /* Restores its count */
            pstNewEventSet->u32RefCount = u32Count;

            /* Restores its flags */
            orxFLAG_SET(pstNewEventSet->u32Flags, u32Flags, orxTRIGGER_SET_KU32_FLAG_BACKUP);
          }
          else
          {
            /* Removes old reference from the table */
            orxHashTable_Remove(sstTrigger.pstSetTable, stID);
          }

          /* For all triggers */
          for(pstTrigger = orxTRIGGER(orxStructure_GetFirst(orxSTRUCTURE_ID_TRIGGER));
              pstTrigger != orxNULL;
              pstTrigger = orxTRIGGER(orxStructure_GetNext(pstTrigger)))
          {
            orxU32 u32Index;

            /* For all its sets */
            for(u32Index = 0; u32Index < orxTRIGGER_KU32_SET_NUMBER; u32Index++)
            {
              /* Matches? */
              if(pstTrigger->pastSetList[u32Index] == pstEventSet)
              {
                /* Updates its data */
                pstTrigger->pastSetList[u32Index] = pstNewEventSet;
              }
            }
          }
        }
      }
    }
  }
  /* Object event */
  else
  {
    orxTRIGGER *pstTrigger;

    /* Checks */
    orxASSERT(_pstEvent->eType == orxEVENT_TYPE_OBJECT);

    /* Gets trigger */
    pstTrigger = orxOBJECT_GET_STRUCTURE(orxOBJECT(_pstEvent->hSender), TRIGGER);

    /* Valid? */
    if(pstTrigger != orxNULL)
    {
      /* Depending on event ID */
      switch(_pstEvent->eID)
      {
        /* Disable */
        case orxOBJECT_EVENT_DISABLE:
        {
          /* Disables it */
          orxStructure_SetFlags(pstTrigger, orxTRIGGER_KU32_FLAG_NONE, orxTRIGGER_KU32_FLAG_ENABLED);

          break;
        }

        /* Enable */
        case orxOBJECT_EVENT_ENABLE:
        {
          /* Enables it */
          orxStructure_SetFlags(pstTrigger, orxTRIGGER_KU32_FLAG_ENABLED, orxTRIGGER_KU32_FLAG_NONE);

          break;
        }

        default:
        {
          break;
        }
      }
    }
  }

  /* Done! */
  return eResult;
}

/** Deletes all the Triggers
 */
static orxINLINE void orxTrigger_DeleteAll()
{
  orxTRIGGER *pstTrigger;

  /* Gets first Trigger */
  pstTrigger = orxTRIGGER(orxStructure_GetFirst(orxSTRUCTURE_ID_TRIGGER));

  /* Non empty? */
  while(pstTrigger != orxNULL)
  {
    /* Deletes it */
    orxTrigger_Delete(pstTrigger);

    /* Gets first Trigger */
    pstTrigger = orxTRIGGER(orxStructure_GetFirst(orxSTRUCTURE_ID_TRIGGER));
  }

  /* Done! */
  return;
}

/** Command: GetRefinement
 */
void orxFASTCALL orxTrigger_CommandGetRefinement(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  /* Updates result */
  _pstResult->zValue = ((_u32ArgNumber < 1) || (_astArgList[0].u32Value == 0))
                       ? sstTrigger.zCurrentEvent
                       : (_astArgList[0].u32Value <= sstTrigger.u32CurrentRefinementListCount)
                         ? sstTrigger.azCurrentRefinementList[_astArgList[0].u32Value - 1]
                         : orxSTRING_EMPTY;

  /* Has stop marker? */
  if(_pstResult->zValue[0] == orxTRIGGER_KC_STOP_MARKER)
  {
    /* Skips it */
    _pstResult->zValue++;
  }

  /* Done! */
  return;
}

/** Command: GetRefinementCount
 */
void orxFASTCALL orxTrigger_CommandGetRefinementCount(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  /* Updates result */
  _pstResult->u32Value = sstTrigger.u32CurrentRefinementListCount;

  /* Done! */
  return;
}

/** Registers all the trigger commands
 */
static orxINLINE void orxTrigger_RegisterCommands()
{
  /* Command: GetRefinement */
  orxCOMMAND_REGISTER_CORE_COMMAND(Trigger, GetRefinement, "Refinement", orxCOMMAND_VAR_TYPE_STRING, 0, 1, {"Index = 0 (Event)", orxCOMMAND_VAR_TYPE_U32});
  /* Command: GetRefinementCount */
  orxCOMMAND_REGISTER_CORE_COMMAND(Trigger, GetRefinementCount, "RefinementCount", orxCOMMAND_VAR_TYPE_U32, 0, 0);

  /* Alias: GetRefinement */
  orxCommand_AddAlias("GetRefinement", "Trigger.GetRefinement", orxNULL);
  /* Alias: GetRefinementCount */
  orxCommand_AddAlias("GetRefinementCount", "Trigger.GetRefinementCount", orxNULL);

  /* Alias: : */
  orxCommand_AddAlias(":", "Trigger.GetRefinement", orxNULL);

  /* Done! */
  return;
}

/** Unregisters all the trigger commands
 */
static orxINLINE void orxTrigger_UnregisterCommands()
{
  /* Alias: GetRefinement */
  orxCommand_RemoveAlias("GetRefinement");
  /* Alias: GetRefinementCount */
  orxCommand_RemoveAlias("GetRefinementCount");

  /* Alias: : */
  orxCommand_RemoveAlias(":");

  /* Command: GetRefinement */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Trigger, GetRefinement);
  /* Command: GetRefinementCount */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Trigger, GetRefinementCount);

  /* Done! */
  return;
}


/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** Trigger module setup
 */
void orxFASTCALL orxTrigger_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_TRIGGER, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_TRIGGER, orxMODULE_ID_BANK);
  orxModule_AddDependency(orxMODULE_ID_TRIGGER, orxMODULE_ID_STRING);
  orxModule_AddDependency(orxMODULE_ID_TRIGGER, orxMODULE_ID_STRUCTURE);
  orxModule_AddDependency(orxMODULE_ID_TRIGGER, orxMODULE_ID_PROFILER);
  orxModule_AddDependency(orxMODULE_ID_TRIGGER, orxMODULE_ID_COMMAND);
  orxModule_AddDependency(orxMODULE_ID_TRIGGER, orxMODULE_ID_CONFIG);
  orxModule_AddDependency(orxMODULE_ID_TRIGGER, orxMODULE_ID_EVENT);

  /* Done! */
  return;
}

/** Inits the Trigger module
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxTrigger_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Not already Initialized? */
  if(!(sstTrigger.u32Flags & orxTRIGGER_KU32_STATIC_FLAG_READY))
  {
    /* Cleans static controller */
    orxMemory_Zero(&sstTrigger, sizeof(orxTRIGGER_STATIC));

    /* Creates set table */
    sstTrigger.pstSetTable = orxHashTable_Create(orxTRIGGER_KU32_TABLE_SIZE, orxHASHTABLE_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

    /* Valid? */
    if(sstTrigger.pstSetTable != orxNULL)
    {
      /* Inits current event & refinement list */
      sstTrigger.zCurrentEvent                  = orxNULL;
      sstTrigger.azCurrentRefinementList        = orxNULL;
      sstTrigger.u32CurrentRefinementListCount  = 0;

      /* Registers structure type */
      eResult = orxSTRUCTURE_REGISTER(TRIGGER, orxSTRUCTURE_STORAGE_TYPE_LINKLIST, orxMEMORY_TYPE_MAIN, orxTRIGGER_KU32_BANK_SIZE, orxNULL);

      /* Initialized? */
      if(eResult != orxSTATUS_FAILURE)
      {
        /* Registers commands */
        orxTrigger_RegisterCommands();

        /* Inits Flags */
        orxFLAG_SET(sstTrigger.u32Flags, orxTRIGGER_KU32_STATIC_FLAG_READY, orxTRIGGER_KU32_STATIC_FLAG_NONE);

        /* Adds event handlers */
        orxEvent_AddHandler(orxEVENT_TYPE_RESOURCE, orxTrigger_EventHandler);
        orxEvent_AddHandler(orxEVENT_TYPE_OBJECT, orxTrigger_EventHandler);
        orxEvent_SetHandlerIDFlags(orxTrigger_EventHandler, orxEVENT_TYPE_RESOURCE, orxNULL, orxEVENT_GET_FLAG(orxRESOURCE_EVENT_ADD) | orxEVENT_GET_FLAG(orxRESOURCE_EVENT_UPDATE), orxEVENT_KU32_MASK_ID_ALL);
        orxEvent_SetHandlerIDFlags(orxTrigger_EventHandler, orxEVENT_TYPE_OBJECT, orxNULL, orxEVENT_GET_FLAG(orxOBJECT_EVENT_ENABLE) | orxEVENT_GET_FLAG(orxOBJECT_EVENT_DISABLE), orxEVENT_KU32_MASK_ID_ALL);
      }
      else
      {
        /* Deletes set table */
        orxHashTable_Delete(sstTrigger.pstSetTable);

        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Failed to register link list structure.");
      }
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Failed to create Trigger set table.");
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Tried to initialize the Trigger module when it was already initialized.");

    /* Already initialized */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Done! */
  return eResult;
}

/** Exits from the Trigger module
 */
void orxFASTCALL orxTrigger_Exit()
{
  /* Initialized? */
  if(sstTrigger.u32Flags & orxTRIGGER_KU32_STATIC_FLAG_READY)
  {
    orxTRIGGER_SET *pstEventSet;

    /* Removes event handlers */
    orxEvent_RemoveHandler(orxEVENT_TYPE_RESOURCE, orxTrigger_EventHandler);
    orxEvent_RemoveHandler(orxEVENT_TYPE_OBJECT, orxTrigger_EventHandler);

    /* Unregisters commands */
    orxTrigger_UnregisterCommands();

    /* Deletes Trigger list */
    orxTrigger_DeleteAll();

    /* Unregisters structure type */
    orxStructure_Unregister(orxSTRUCTURE_ID_TRIGGER);

    /* For all remaining sets */
    while(orxHashTable_GetNext(sstTrigger.pstSetTable, orxNULL, orxNULL, (void **)&pstEventSet) != orxHANDLE_UNDEFINED)
    {
      /* Deletes it */
      orxTrigger_DeleteSet(pstEventSet);
    }

    /* Deletes set table */
    orxHashTable_Delete(sstTrigger.pstSetTable);

    /* Updates flags */
    sstTrigger.u32Flags &= ~orxTRIGGER_KU32_STATIC_FLAG_READY;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Tried to exit from the Trigger module when it wasn't initialized.");
  }

  /* Done! */
  return;
}

/** Creates an empty Trigger
 * @return orxTrigger / orxNULL
 */
orxTRIGGER *orxFASTCALL orxTrigger_Create()
{
  orxTRIGGER *pstResult;

  /* Checks */
  orxASSERT(sstTrigger.u32Flags & orxTRIGGER_KU32_STATIC_FLAG_READY);

  /* Creates Trigger */
  pstResult = orxTRIGGER(orxStructure_Create(orxSTRUCTURE_ID_TRIGGER));

  /* Created? */
  if(pstResult != orxNULL)
  {
    /* Inits flags */
    orxStructure_SetFlags(pstResult, orxTRIGGER_KU32_FLAG_ENABLED, orxTRIGGER_KU32_MASK_ALL);

    /* Increases count */
    orxStructure_IncreaseCount(pstResult);
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Failed to create Trigger structure.");
  }

  /* Done! */
  return pstResult;
}

/** Deletes a Trigger
 * @param[in] _pstTrigger             Concerned Trigger
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxTrigger_Delete(orxTRIGGER *_pstTrigger)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstTrigger.u32Flags & orxTRIGGER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstTrigger);

  /* Decreases count */
  orxStructure_DecreaseCount(_pstTrigger);

  /* Not referenced? */
  if(orxStructure_GetRefCount(_pstTrigger) == 0)
  {
    orxTRIGGER_EVENT_PAYLOAD stPayload;
    orxSTRUCTURE             *pstOwner;
    orxU32                    i;

    /* Gets owner */
    pstOwner = orxStructure_GetOwner(_pstTrigger);

    /* Inits event payload */
    orxMemory_Zero(&stPayload, sizeof(orxTRIGGER_EVENT_PAYLOAD));
    stPayload.pstTrigger = _pstTrigger;

    /* For all sets */
    for(i = 0; i < orxTRIGGER_KU32_SET_NUMBER; i++)
    {
      /* Valid? */
      if(_pstTrigger->pastSetList[i] != orxNULL)
      {
        orxTRIGGER_SET *pstSet;

        /* Gets set */
        pstSet = _pstTrigger->pastSetList[i];

        /* Removes its reference */
        _pstTrigger->pastSetList[i] = orxNULL;

        /* Updates payload */
        stPayload.zSetName = pstSet->zReference;

        /* Sends event */
        orxEVENT_SEND(orxEVENT_TYPE_TRIGGER, orxTRIGGER_EVENT_SET_REMOVE, pstOwner, pstOwner, &stPayload);

        /* Deletes it */
        orxTrigger_DeleteSet(pstSet);
      }
    }

    /* Deletes structure */
    orxStructure_Delete(_pstTrigger);
  }
  else
  {
    /* Referenced by others */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Clears cache (if any Trigger is still in active use, it'll remain in memory until not referenced anymore)
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxTrigger_ClearCache()
{
  orxTRIGGER_SET *pstSet, *pstNewSet;
  orxHANDLE       hIterator, hNextIterator;
  orxSTATUS       eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstTrigger.u32Flags & orxTRIGGER_KU32_STATIC_FLAG_READY);

  /* For all sets */
  for(hIterator = orxHashTable_GetNext(sstTrigger.pstSetTable, orxHANDLE_UNDEFINED, orxNULL, (void **)&pstSet);
      hIterator != orxHANDLE_UNDEFINED;
      hIterator = hNextIterator, pstSet = pstNewSet)
  {
    /* Gets next set */
    hNextIterator = orxHashTable_GetNext(sstTrigger.pstSetTable, hIterator, orxNULL, (void **)&pstNewSet);

    /* Is cached? */
    if(orxFLAG_TEST(pstSet->u32Flags, orxTRIGGER_SET_KU32_FLAG_CACHED))
    {
      /* Updates its flags */
      orxFLAG_SET(pstSet->u32Flags, orxTRIGGER_SET_KU32_FLAG_NONE, orxTRIGGER_SET_KU32_FLAG_CACHED);

      /* Deletes its extra reference */
      orxTrigger_DeleteSet(pstSet);
    }
  }

  /* Done! */
  return eResult;
}

/** Enables/disables a Trigger
 * @param[in]   _pstTrigger           Concerned Trigger
 * @param[in]   _bEnable              Enable / disable
 */
void orxFASTCALL orxTrigger_Enable(orxTRIGGER *_pstTrigger, orxBOOL _bEnable)
{
  /* Checks */
  orxASSERT(sstTrigger.u32Flags & orxTRIGGER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstTrigger);

  /* Enable? */
  if(_bEnable != orxFALSE)
  {
    /* Updates status flags */
    orxStructure_SetFlags(_pstTrigger, orxTRIGGER_KU32_FLAG_ENABLED, orxTRIGGER_KU32_FLAG_NONE);
  }
  else
  {
    /* Updates status flags */
    orxStructure_SetFlags(_pstTrigger, orxTRIGGER_KU32_FLAG_NONE, orxTRIGGER_KU32_FLAG_ENABLED);
  }

  /* Done! */
  return;
}

/** Is Trigger enabled?
 * @param[in]   _pstTrigger           Concerned Trigger
 * @return      orxTRUE if enabled, orxFALSE otherwise
 */
orxBOOL orxFASTCALL orxTrigger_IsEnabled(const orxTRIGGER *_pstTrigger)
{
  /* Checks */
  orxASSERT(sstTrigger.u32Flags & orxTRIGGER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstTrigger);

  /* Done! */
  return(orxStructure_TestFlags(_pstTrigger, orxTRIGGER_KU32_FLAG_ENABLED));
}

/** Adds a set to a Trigger from config
 * @param[in]   _pstTrigger           Concerned Trigger
 * @param[in]   _zConfigID            Config ID of the set to add
 * return       orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxTrigger_AddSetFromConfig(orxTRIGGER *_pstTrigger, const orxSTRING _zConfigID)
{
  orxU32    u32Index;
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(sstTrigger.u32Flags & orxTRIGGER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstTrigger);
  orxASSERT((_zConfigID != orxNULL) && (_zConfigID != orxSTRING_EMPTY));

  /* Finds an empty set */
  for(u32Index = 0; (u32Index < orxTRIGGER_KU32_SET_NUMBER) && (_pstTrigger->pastSetList[u32Index] != orxNULL); u32Index++);

  /* Found? */
  if(u32Index < orxTRIGGER_KU32_SET_NUMBER)
  {
    orxTRIGGER_SET *pstSet;

    /* Searches for reference */
    pstSet = (orxTRIGGER_SET *)orxHashTable_Get(sstTrigger.pstSetTable, orxString_Hash(_zConfigID));

    /* Found? */
    if(pstSet != orxNULL)
    {
      /* Increases count */
      pstSet->u32RefCount++;
    }
    else
    {
      /* Creates set */
      pstSet = orxTrigger_CreateSet(_zConfigID);
    }

    /* Valid? */
    if(pstSet != orxNULL)
    {
      orxTRIGGER_EVENT_PAYLOAD  stPayload;
      orxSTRUCTURE             *pstOwner;

      /* Gets owner */
      pstOwner = orxStructure_GetOwner(_pstTrigger);

      /* Stores set */
      _pstTrigger->pastSetList[u32Index] = pstSet;

      /* Inits event payload */
      orxMemory_Zero(&stPayload, sizeof(orxTRIGGER_EVENT_PAYLOAD));
      stPayload.pstTrigger  = _pstTrigger;
      stPayload.zSetName    = pstSet->zReference;

      /* Sends event */
      orxEVENT_SEND(orxEVENT_TYPE_TRIGGER, orxTRIGGER_EVENT_SET_ADD, pstOwner, pstOwner, &stPayload);

      /* Updates result */
      eResult = orxSTATUS_SUCCESS;
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "No room in Trigger, can't add set <%s>.", _zConfigID);
  }

  /* Done! */
  return eResult;
}

/** Removes a set from a Trigger using its config ID
 * @param[in]   _pstTrigger           Concerned Trigger
 * @param[in]   _zConfigID            Config ID of the set to remove
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxTrigger_RemoveSetFromConfig(orxTRIGGER *_pstTrigger, const orxSTRING _zConfigID)
{
  orxSTRINGID   stSetID;
  orxU32        u32Index;
  orxSTRUCTURE *pstOwner;
  orxSTATUS     eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(sstTrigger.u32Flags & orxTRIGGER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstTrigger);
  orxASSERT((_zConfigID != orxNULL) && (_zConfigID != orxSTRING_EMPTY));

  /* Gets owner */
  pstOwner = orxStructure_GetOwner(_pstTrigger);

  /* Gets set ID */
  stSetID = orxString_Hash(_zConfigID);

  /* For all sets */
  for(u32Index = 0; u32Index < orxTRIGGER_KU32_SET_NUMBER; u32Index++)
  {
    /* Is defined? */
    if(_pstTrigger->pastSetList[u32Index] != orxNULL)
    {
      /* Do IDs match? */
      if(_pstTrigger->pastSetList[u32Index]->stID == stSetID)
      {
        orxTRIGGER_EVENT_PAYLOAD  stPayload;
        orxTRIGGER_SET           *pstSet;

        /* Gets set */
        pstSet = _pstTrigger->pastSetList[u32Index];

        /* Removes its reference */
        _pstTrigger->pastSetList[u32Index] = orxNULL;

        /* Inits event payload */
        orxMemory_Zero(&stPayload, sizeof(orxTRIGGER_EVENT_PAYLOAD));
        stPayload.pstTrigger  = _pstTrigger;
        stPayload.zSetName    = pstSet->zReference;

        /* Sends event */
        orxEVENT_SEND(orxEVENT_TYPE_TRIGGER, orxTRIGGER_EVENT_SET_REMOVE, pstOwner, pstOwner, &stPayload);

        /* Deletes it */
        orxTrigger_DeleteSet(pstSet);

        /* Updates result */
        eResult = orxSTATUS_SUCCESS;

        break;
      }
    }
  }

  /* Done! */
  return eResult;
}

/** Gets how many sets are defined in the trigger
 * @param[in]   _pstTrigger           Concerned Trigger
 * @return      Count of sets defined in the trigger
 */
orxU32 orxFASTCALL orxTrigger_GetCount(const orxTRIGGER *_pstTrigger)
{
  orxU32 i, u32Result = 0;

  /* Checks */
  orxASSERT(sstTrigger.u32Flags & orxTRIGGER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstTrigger);

  /* For all sets */
  for(i = 0; i < orxTRIGGER_KU32_SET_NUMBER; i++)
  {
    /* Is valid? */
    if(_pstTrigger->pastSetList[i] != orxNULL)
    {
      /* Updates result */
      u32Result++;
    }
  }

  /* Done! */
  return u32Result;
}

/** Fire a Trigger's event
 * @param[in]   _pstTrigger           Concerned Trigger
 * @param[in]   _zEvent               Event to fire
 * @param[in]   _azRefinementList     List of refinements for this event, unused if _u32Count == 0
 * @param[in]   _u32Count             Number of refinements in the list, 0 for none
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxTrigger_Fire(orxTRIGGER *_pstTrigger, const orxSTRING _zEvent, const orxSTRING *_azRefinementList, orxU32 _u32Count)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Profiles */
  orxPROFILER_PUSH_MARKER("orxTrigger_Fire");

  /* Checks */
  orxASSERT(sstTrigger.u32Flags & orxTRIGGER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstTrigger);
  orxASSERT((_zEvent != orxNULL) && (_zEvent != orxSTRING_EMPTY));
  orxASSERT((_u32Count == 0) || (_azRefinementList != orxNULL));

  /* Is enabled? */
  if(orxStructure_TestFlags(_pstTrigger, orxTRIGGER_KU32_FLAG_ENABLED))
  {
    orxTRIGGER_EVENT_PAYLOAD  stPayload;
    orxSTRUCTURE             *pstOwner;
    const orxSTRING          *azPreviousRefinementList;
    const orxSTRING           zPreviousEvent;
    orxCHAR                   acBuffer[orxTRIGGER_KU32_BUFFER_SIZE], *pc = acBuffer;
    orxS32                    i, s32StopDepth;
    orxU32                    u32PreviousRefinementListCount;

  #ifdef __orxMSVC__
    orxSTRINGID *astEventIDList = (orxSTRINGID *)_malloca((_u32Count + 1) * sizeof(orxSTRINGID));
  #else /* __orxMSVC__ */
    orxSTRINGID astEventIDList[_u32Count + 1];
  #endif /* __orxMSVC__ */

    /* Backups current event & refinement list */
    zPreviousEvent                  = sstTrigger.zCurrentEvent;
    azPreviousRefinementList        = sstTrigger.azCurrentRefinementList;
    u32PreviousRefinementListCount  = sstTrigger.u32CurrentRefinementListCount;

    /* Updates current event & refinement list */
    sstTrigger.zCurrentEvent                  = _zEvent;
    sstTrigger.azCurrentRefinementList        = _azRefinementList;
    sstTrigger.u32CurrentRefinementListCount  = _u32Count;

    /* Gets event ID */
    pc += orxString_NPrint(pc, sizeof(acBuffer), "%s", _zEvent);
    astEventIDList[0] = orxString_Hash(acBuffer);

    /* Inits event payload */
    orxMemory_Zero(&stPayload, sizeof(orxTRIGGER_EVENT_PAYLOAD));
    stPayload.pstTrigger = _pstTrigger;

    /* Gets owner */
    pstOwner = orxStructure_GetOwner(_pstTrigger);

    /* For all refinements */
    for(i = 0, s32StopDepth = 0; i < (orxS32)_u32Count; i++)
    {
      const orxSTRING zRefinement;

      /* Gets refinement & stop depth */
      zRefinement = (*_azRefinementList[i] == orxTRIGGER_KC_STOP_MARKER) ? s32StopDepth = i + 1, _azRefinementList[i] + 1 : _azRefinementList[i];

      /* Stores its ID */
      pc += orxString_NPrint(pc, (orxU32)(sizeof(acBuffer) - (pc - acBuffer)), "%c%s", orxTRIGGER_KC_SEPARATOR, zRefinement);
      astEventIDList[i + 1] = orxString_Hash(acBuffer);
    }

    /* For all refinements, in reverse order */
    for(i = (orxS32)_u32Count; (i >= 0) && (i >= s32StopDepth); i--)
    {
      orxSTRINGID stEventID;
      orxU32      u32SetIndex;

      /* Gets its ID */
      stEventID = astEventIDList[i];

      /* For all sets */
      for(u32SetIndex = 0; u32SetIndex < orxTRIGGER_KU32_SET_NUMBER; u32SetIndex++)
      {
        const orxTRIGGER_SET *pstSet;

        /* Gets it */
        pstSet = _pstTrigger->pastSetList[u32SetIndex];

        /* Valid? */
        if(pstSet != orxNULL)
        {
          orxU32 u32EventIndex;

          /* For all its events */
          for(u32EventIndex = 0; u32EventIndex < pstSet->u32EventCount; u32EventIndex++)
          {
            /* Match? */
            if(pstSet->astEventList[u32EventIndex].stID == stEventID)
            {
              /* Updates payload */
              stPayload.zSetName            = pstSet->zReference;
              stPayload.zEventName          = _zEvent;
              stPayload.zEventValue         = pstSet->astEventList[u32EventIndex].zValue;
              stPayload.azRefinementList    = _azRefinementList;
              stPayload.u32RefinementCount  = _u32Count;
              stPayload.u32RefinementIndex  = (orxU32)i;

              /* Sends event */
              orxEVENT_SEND(orxEVENT_TYPE_TRIGGER, orxTRIGGER_EVENT_FIRE, pstOwner, pstOwner, &stPayload);

              /* Updates stop depth */
              s32StopDepth = orxMAX(s32StopDepth, (orxS32)pstSet->astEventList[u32EventIndex].u32StopDepth);

              /* Updates result */
              eResult = orxSTATUS_SUCCESS;
            }
          }
        }
      }
    }

    /* Restores previous event & refinement list */
    sstTrigger.zCurrentEvent                  = zPreviousEvent;
    sstTrigger.azCurrentRefinementList        = azPreviousRefinementList;
    sstTrigger.u32CurrentRefinementListCount  = u32PreviousRefinementListCount;
  }

  /* Profiles */
  orxPROFILER_POP_MARKER();

  /* Done! */
  return eResult;
}

#ifdef __orxMSVC__

  #pragma warning(pop)

#endif /* __orxMSVC__ */
