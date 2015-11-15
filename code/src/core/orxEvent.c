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
 * @file orxEvent.c
 * @date 20/05/2008
 * @author iarwain@orx-project.org
 *
 */


#include "orxInclude.h"

#include "core/orxEvent.h"
#include "core/orxThread.h"
#include "debug/orxDebug.h"
#include "debug/orxProfiler.h"
#include "memory/orxBank.h"
#include "utils/orxHashTable.h"
#include "utils/orxLinkList.h"


/** Module flags
 */
#define orxEVENT_KU32_STATIC_FLAG_NONE    0x00000000  /**< No flags */

#define orxEVENT_KU32_STATIC_FLAG_READY   0x00000001  /**< Ready flag */

#define orxEVENT_KU32_STATIC_MASK_ALL     0xFFFFFFFF  /**< All mask */


/** Misc defines
 */
#define orxEVENT_KU32_HANDLER_TABLE_SIZE  64
#define orxEVENT_KU32_HANDLER_BANK_SIZE   32
#define orxEVENT_KU32_STORAGE_BANK_SIZE   32


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Event handler info structure
 */
typedef struct __orxEVENT_HANDLER_INFO_t
{
  orxLINKLIST_NODE  stNode;
  orxEVENT_HANDLER  pfnHandler;
  void             *pContext;

} orxEVENT_HANDLER_INFO;

/** Event handler storage
 */
typedef struct __orxEVENT_HANDLER_STORAGE_t
{
  orxLINKLIST stList;
  orxBANK    *pstBank;

} orxEVENT_HANDLER_STORAGE;

/** Static structure
 */
typedef struct __orxEVENT_STATIC_t
{
  orxU32                    u32Flags;                 /**< Control flags */
  orxS32                    s32EventSendCounter;      /**< Event send counter */
  orxHASHTABLE             *pstHandlerStorageTable;   /**< Handler storage table */
  orxBANK                  *pstHandlerStorageBank;    /**< Handler storage bank */
  orxEVENT_HANDLER_STORAGE *astCoreHandlerStorageList[orxEVENT_TYPE_CORE_NUMBER]; /**< Core handler storage list */

} orxEVENT_STATIC;


/***************************************************************************
 * Module global variable                                                  *
 ***************************************************************************/

static orxEVENT_STATIC sstEvent;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/


/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** Event module setup
 */
void orxFASTCALL orxEvent_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_EVENT, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_EVENT, orxMODULE_ID_BANK);
  orxModule_AddDependency(orxMODULE_ID_EVENT, orxMODULE_ID_PROFILER);
  orxModule_AddDependency(orxMODULE_ID_EVENT, orxMODULE_ID_THREAD);

  /* Done! */
  return;
}

/** Inits the event module
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxEvent_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Not already Initialized? */
  if(!orxFLAG_TEST(sstEvent.u32Flags, orxEVENT_KU32_STATIC_FLAG_READY))
  {
    /* Cleans control structure */
    orxMemory_Zero(&sstEvent, sizeof(orxEVENT_STATIC));

    /* Creates handler storage table */
    sstEvent.pstHandlerStorageTable = orxHashTable_Create(orxEVENT_KU32_HANDLER_TABLE_SIZE, orxHASHTABLE_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

    /* Success? */
    if(sstEvent.pstHandlerStorageTable != orxNULL)
    {
      /* Creates handler storage bank */
      sstEvent.pstHandlerStorageBank = orxBank_Create(orxEVENT_KU32_STORAGE_BANK_SIZE, sizeof(orxEVENT_HANDLER_STORAGE), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

      /* Success? */
      if(sstEvent.pstHandlerStorageBank != orxNULL)
      {
        /* Inits Flags */
        orxFLAG_SET(sstEvent.u32Flags, orxEVENT_KU32_STATIC_FLAG_READY, orxEVENT_KU32_STATIC_MASK_ALL);

        /* Success */
        eResult = orxSTATUS_SUCCESS;
      }
      else
      {
        /* Deletes table */
        orxHashTable_Delete(sstEvent.pstHandlerStorageTable);

        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Event module failed to create bank.");

        /* Updates result */
        eResult = orxSTATUS_FAILURE;
      }
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Event module failed to create hash table.");

      /* Updates result */
      eResult = orxSTATUS_FAILURE;
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Event module already loaded.");

    /* Already initialized */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Done! */
  return eResult;
}

/** Exits from the event module
 */
void orxFASTCALL orxEvent_Exit()
{
  /* Initialized? */
  if(orxFLAG_TEST(sstEvent.u32Flags, orxEVENT_KU32_STATIC_FLAG_READY))
  {
    /* Deletes hashtable */
    orxHashTable_Delete(sstEvent.pstHandlerStorageTable);

    /* Deletes bank */
    orxBank_Delete(sstEvent.pstHandlerStorageBank);

    /* Updates flags */
    orxFLAG_SET(sstEvent.u32Flags, orxEVENT_KU32_STATIC_FLAG_NONE, orxEVENT_KU32_STATIC_MASK_ALL);
  }

  return;
}

/** Adds an event handler
 * @param[in] _eEventType           Concerned type of event
 * @param[in] _pfnHandler           Event handler to add
 * return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxEvent_AddHandler(orxEVENT_TYPE _eEventType, orxEVENT_HANDLER _pfnEventHandler)
{
  orxSTATUS eResult;

  /* Adds handler */
  eResult = orxEvent_AddHandlerWithContext(_eEventType, _pfnEventHandler, orxNULL);

  /* Done! */
  return eResult;
}

/** Adds an event handler with user-defined context
 * @param[in] _eEventType           Concerned type of event
 * @param[in] _pfnHandler           Event handler to add
 * @param[in] _pContext             Context that will be stored in events sent to this handler
 * return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxEvent_AddHandlerWithContext(orxEVENT_TYPE _eEventType, orxEVENT_HANDLER _pfnEventHandler, void *_pContext)
{
  orxEVENT_HANDLER_STORAGE *pstStorage;
  orxSTATUS                 eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstEvent.u32Flags, orxEVENT_KU32_STATIC_FLAG_READY));
  orxASSERT(_pfnEventHandler != orxNULL);

  /* Gets corresponding storage */
  pstStorage = (_eEventType < orxEVENT_TYPE_CORE_NUMBER) ? sstEvent.astCoreHandlerStorageList[_eEventType] : (orxEVENT_HANDLER_STORAGE *)orxHashTable_Get(sstEvent.pstHandlerStorageTable, _eEventType);

  /* No storage yet? */
  if(pstStorage == orxNULL)
  {
    /* Allocates it */
    pstStorage = (orxEVENT_HANDLER_STORAGE *)orxBank_Allocate(sstEvent.pstHandlerStorageBank);

    /* Success? */
    if(pstStorage != orxNULL)
    {
      /* Creates its bank */
      pstStorage->pstBank = orxBank_Create(orxEVENT_KU32_HANDLER_BANK_SIZE, sizeof(orxEVENT_HANDLER_INFO), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

      /* Success? */
      if(pstStorage->pstBank != orxNULL)
      {
        /* Clears its list */
        orxMemory_Zero(&(pstStorage->stList), sizeof(orxLINKLIST));

        /* Is a core event handler? */
        if(_eEventType < orxEVENT_TYPE_CORE_NUMBER)
        {
          /* Stores it */
          sstEvent.astCoreHandlerStorageList[_eEventType] = pstStorage;
        }
        else
        {
          /* Tries to add it to the table */
          if(orxHashTable_Add(sstEvent.pstHandlerStorageTable, _eEventType, pstStorage) == orxSTATUS_FAILURE)
          {
            /* Deletes its bank */
            orxBank_Delete(pstStorage->pstBank);

            /* Frees storage */
            orxBank_Free(sstEvent.pstHandlerStorageBank, pstStorage);
            pstStorage = orxNULL;
          }
        }
      }
      else
      {
        /* Frees storage */
        orxBank_Free(sstEvent.pstHandlerStorageBank, pstStorage);
        pstStorage = orxNULL;
      }
    }
  }

  /* Valid? */
  if(pstStorage != orxNULL)
  {
    orxEVENT_HANDLER_INFO *pstInfo;

    /* Allocates a new handler info */
    pstInfo = (orxEVENT_HANDLER_INFO *)orxBank_Allocate(pstStorage->pstBank);

    /* Valid? */
    if(pstInfo != orxNULL)
    {
      /* Clears its node */
      orxMemory_Zero(&(pstInfo->stNode), sizeof(orxLINKLIST_NODE));

      /* Stores its handler */
      pstInfo->pfnHandler = _pfnEventHandler;

      /* Stores context */
      pstInfo->pContext = _pContext;

      /* Adds it to the list */
      eResult = orxLinkList_AddEnd(&(pstStorage->stList), &(pstInfo->stNode));
    }
  }

  /* Done! */
  return eResult;
}

/** Removes an event handler
 * @param[in] _eEventType           Concerned type of event
 * @param[in] _pfnHandler           Event handler to remove
 * return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxEvent_RemoveHandler(orxEVENT_TYPE _eEventType, orxEVENT_HANDLER _pfnEventHandler)
{
  orxSTATUS eResult;

  /* Removes first instance of handler */
  eResult = orxEvent_RemoveHandlerWithContext(_eEventType, _pfnEventHandler, (void *)_pfnEventHandler);

  /* Done! */
  return eResult;
}

/** Removes an event handler which matches given context
 * @param[in] _eEventType           Concerned type of event
 * @param[in] _pfnEventHandler      Event handler to remove
 * @param[in] _pContext             Context of the handler to remove, orxNULL for removing all occurrences regardless of their context
 * return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxEvent_RemoveHandlerWithContext(orxEVENT_TYPE _eEventType, orxEVENT_HANDLER _pfnEventHandler, void *_pContext)
{
  orxEVENT_HANDLER_STORAGE *pstStorage;
  orxSTATUS                 eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstEvent.u32Flags, orxEVENT_KU32_STATIC_FLAG_READY));
  orxASSERT(_pfnEventHandler != orxNULL);

  /* Gets corresponding storage */
  pstStorage = (_eEventType < orxEVENT_TYPE_CORE_NUMBER) ? sstEvent.astCoreHandlerStorageList[_eEventType] : (orxEVENT_HANDLER_STORAGE *)orxHashTable_Get(sstEvent.pstHandlerStorageTable, _eEventType);

  /* Valid? */
  if(pstStorage != orxNULL)
  {
    orxEVENT_HANDLER_INFO *pstInfo;

    /* For all handlers */
    for(pstInfo = (orxEVENT_HANDLER_INFO *)orxLinkList_GetFirst(&(pstStorage->stList));
        pstInfo != orxNULL;
        pstInfo = (orxEVENT_HANDLER_INFO *)orxLinkList_GetNext(&(pstInfo->stNode)))
    {
      /* Found? */
      if((pstInfo->pfnHandler == _pfnEventHandler)
      && ((_pContext == orxNULL)
       || (_pContext == _pfnEventHandler)
       || (_pContext == pstInfo->pContext)))
      {
        /* Removes it from list */
        orxLinkList_Remove(&(pstInfo->stNode));

        /* Frees it */
        orxBank_Free(pstStorage->pstBank, pstInfo);

        /* Updates result */
        eResult = orxSTATUS_SUCCESS;

        /* Should only remove one? */
        if(_pContext == _pfnEventHandler)
        {
          /* Stops */
          break;
        }
      }
    }
  }
  else
  {
    /* Defaults to success */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Done! */
  return eResult;
}

/** Sends an event
 * @param[in] _pstEvent             Event to send
 */
orxSTATUS orxFASTCALL orxEvent_Send(orxEVENT *_pstEvent)
{
  orxEVENT_HANDLER_STORAGE *pstStorage;
  orxSTATUS                 eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstEvent.u32Flags, orxEVENT_KU32_STATIC_FLAG_READY));
  orxASSERT(_pstEvent != orxNULL);

  /* Gets corresponding storage */
  pstStorage = (_pstEvent->eType < orxEVENT_TYPE_CORE_NUMBER) ? sstEvent.astCoreHandlerStorageList[_pstEvent->eType] : (orxEVENT_HANDLER_STORAGE *)orxHashTable_Get(sstEvent.pstHandlerStorageTable, _pstEvent->eType);

  /* Valid? */
  if(pstStorage != orxNULL)
  {
    /* Main thread? */
    if(orxThread_GetCurrent() == orxTHREAD_KU32_MAIN_THREAD_ID)
    {
      /* Updates event send counter */
      sstEvent.s32EventSendCounter++;
    }

    /* Has handler(s)? */
    if(orxLinkList_GetCounter(&(pstStorage->stList)) != 0)
    {
      orxEVENT_HANDLER_INFO *pstInfo;

      /* For all handlers */
      for(pstInfo = (orxEVENT_HANDLER_INFO *)orxLinkList_GetFirst(&(pstStorage->stList));
          pstInfo != orxNULL;
          pstInfo = (orxEVENT_HANDLER_INFO *)orxLinkList_GetNext(&(pstInfo->stNode)))
      {
        /* Stores context */
        _pstEvent->pContext = pstInfo->pContext;

        /* Calls it */
        if((pstInfo->pfnHandler)(_pstEvent) == orxSTATUS_FAILURE)
        {
          /* Updates result */
          eResult = orxSTATUS_FAILURE;

          break;
        }
      }

      /* Clears context */
      _pstEvent->pContext = orxNULL;
    }

    /* Main thread? */
    if(orxThread_GetCurrent() == orxTHREAD_KU32_MAIN_THREAD_ID)
    {
      /* Updates event send counter */
      sstEvent.s32EventSendCounter--;
    }
  }

  /* Done! */
  return eResult;
}

/** Sends a simple event
 * @param[in] _eEventType           Event type
 * @param[in] _eEventID             Event ID
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxEvent_SendShort(orxEVENT_TYPE _eEventType, orxENUM _eEventID)
{
  orxEVENT  stEvent;
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstEvent.u32Flags, orxEVENT_KU32_STATIC_FLAG_READY));

  /* Inits event */
  orxMemory_Zero(&stEvent, sizeof(orxEVENT));
  stEvent.eType = _eEventType;
  stEvent.eID   = _eEventID;

  /* Sends it */
  eResult = orxEvent_Send(&stEvent);

  /* Done! */
  return eResult;
}

/** Is currently sending an event?
 * @return orxTRUE / orxFALSE
 */
orxBOOL orxFASTCALL orxEvent_IsSending()
{
  orxBOOL bResult;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstEvent.u32Flags, orxEVENT_KU32_STATIC_FLAG_READY));

  /* Updates result */
  bResult = (sstEvent.s32EventSendCounter != 0) ? orxTRUE : orxFALSE;

  /* Done! */
  return bResult;
}
