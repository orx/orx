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
 * @file orxEvent.c
 * @date 20/05/2008
 * @author iarwain@orx-project.org
 *
 */


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
  orxU32            u32IDFlags;

} orxEVENT_HANDLER_INFO;

/** Event handler storage
 */
typedef struct __orxEVENT_HANDLER_STORAGE_t
{
  orxU8       au8HandledIDList[32];
  orxLINKLIST stList;
  orxBANK    *pstBank;

} orxEVENT_HANDLER_STORAGE;

/** Static structure
 */
typedef struct __orxEVENT_STATIC_t
{
  orxU32                    u32Flags;                 /**< Control flags */
  orxS32                    s32EventSendCount;        /**< Event send count */
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
    orxEVENT_HANDLER_STORAGE *pstStorage;

    /* Deletes hashtable */
    orxHashTable_Delete(sstEvent.pstHandlerStorageTable);

    /* For all storages */
    for(pstStorage = (orxEVENT_HANDLER_STORAGE *)orxBank_GetNext(sstEvent.pstHandlerStorageBank, orxNULL);
        pstStorage != orxNULL;
        pstStorage = (orxEVENT_HANDLER_STORAGE *)orxBank_GetNext(sstEvent.pstHandlerStorageBank, pstStorage))
    {
      /* Deletes its bank */
      orxBank_Delete(pstStorage->pstBank);
    }

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
      /* Clears it */
      orxMemory_Zero(pstStorage, sizeof(orxEVENT_HANDLER_STORAGE));

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
    orxEVENT_HANDLER_INFO *pstInfo, *pstNextInfo;

    /* For all handlers */
    for(pstInfo = (orxEVENT_HANDLER_INFO *)orxLinkList_GetFirst(&(pstStorage->stList));
        pstInfo != orxNULL;
        pstInfo = pstNextInfo)
    {
      /* Gets next info */
      pstNextInfo = (orxEVENT_HANDLER_INFO *)orxLinkList_GetNext(&(pstInfo->stNode));

      /* Marked for deletion? */
      if(pstInfo->pfnHandler == orxNULL)
      {
        /* Removes it from list */
        orxLinkList_Remove(&(pstInfo->stNode));

        /* For all IDs */
        for(orxU32 i = 0; i < 32; i++)
        {
          orxU32 u32ID;

          /* Gets it */
          u32ID = (orxU32)(1 << i);

          /* Should be removed? */
          if(orxFLAG_TEST(pstInfo->u32IDFlags, u32ID))
          {
            /* Checks */
            orxASSERT(pstStorage->au8HandledIDList[i] > 0);

            /* Updates ID tracking */
            pstStorage->au8HandledIDList[i]--;
          }
        }

        /* Frees it */
        orxBank_Free(pstStorage->pstBank, pstInfo);
      }
    }

    /* Allocates a new handler info */
    pstInfo = (orxEVENT_HANDLER_INFO *)orxBank_Allocate(pstStorage->pstBank);

    /* Valid? */
    if(pstInfo != orxNULL)
    {
      /* Clears it */
      orxMemory_Zero(pstInfo, sizeof(orxEVENT_HANDLER_INFO));

      /* Stores its handler */
      pstInfo->pfnHandler = _pfnEventHandler;

      /* Stores context */
      pstInfo->pContext = _pContext;

      /* Inits its ID flags */
      pstInfo->u32IDFlags = orxEVENT_KU32_MASK_ID_ALL;

      /* Adds it to the list */
      if(orxLinkList_AddEnd(&(pstStorage->stList), &(pstInfo->stNode)) != orxSTATUS_FAILURE)
      {
        /* For all IDs */
        for(orxU32 i = 0; i < 32; i++)
        {
          /* Checks */
          orxASSERT(pstStorage->au8HandledIDList[i] < 255);

          /* Updates ID tracking */
          pstStorage->au8HandledIDList[i]++;
        }

        /* Updates result */
        eResult = orxSTATUS_SUCCESS;
      }
      else
      {
        /* Frees handler info */
        orxBank_Free(pstStorage->pstBank, pstInfo);
      }
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
        /* Marks it for deletion */
        pstInfo->pfnHandler = orxNULL;

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

/** Sets an event handler's ID flags (use orxEVENT_GET_FLAG(ID) in order to get the flag that matches an ID)
 * @param[in] _pfnEventHandler      Concerned event handler, must have been previously added for the given type
 * @param[in] _eEventType           Concerned type of event
 * @param[in] _pContext             Context of the handler to update, orxNULL for updating all occurrences regardless of their context
 * @param[in] _u32AddIDFlags        ID flags to add
 * @param[in] _u32RemoveIDFlags     ID flags to remove
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxEvent_SetHandlerIDFlags(orxEVENT_HANDLER _pfnEventHandler, orxEVENT_TYPE _eEventType, void *_pContext, orxU32 _u32AddIDFlags, orxU32 _u32RemoveIDFlags)
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
       || (_pContext == pstInfo->pContext)))
      {
        /* For all IDs */
        for(orxU32 i = 0; i < 32; i++)
        {
          orxU32 u32ID;

          /* Gets it */
          u32ID = (orxU32)(1 << i);

          /* Should be removed? */
          if(orxFLAG_TEST(_u32RemoveIDFlags, u32ID) && orxFLAG_TEST(pstInfo->u32IDFlags, u32ID))
          {
            /* Shouldn't be added back? */
            if(!orxFLAG_TEST(_u32AddIDFlags, u32ID))
            {
              /* Checks */
              orxASSERT(pstStorage->au8HandledIDList[i] > 0);

              /* Updates ID tracking */
              pstStorage->au8HandledIDList[i]--;
            }
          }
          /* Should be added? */
          else if(orxFLAG_TEST(_u32AddIDFlags, u32ID) && !orxFLAG_TEST(pstInfo->u32IDFlags, u32ID))
          {
            /* Checks */
            orxASSERT(pstStorage->au8HandledIDList[i] < 255);

            /* Updates ID tracking */
            pstStorage->au8HandledIDList[i]++;
          }
        }

        /* Updates its ID flags */
        orxFLAG_SET(pstInfo->u32IDFlags, _u32AddIDFlags, _u32RemoveIDFlags);

        /* Updates result */
        eResult = orxSTATUS_SUCCESS;
      }
    }
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

  /* Profiles */
  orxPROFILER_PUSH_MARKER("orxEvent_Send");

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstEvent.u32Flags, orxEVENT_KU32_STATIC_FLAG_READY));
  orxASSERT(_pstEvent != orxNULL);
  orxASSERT((_pstEvent->eID >= 0) && (_pstEvent->eID < 32));

  /* Gets corresponding storage */
  pstStorage = (_pstEvent->eType < orxEVENT_TYPE_CORE_NUMBER) ? sstEvent.astCoreHandlerStorageList[_pstEvent->eType] : (orxEVENT_HANDLER_STORAGE *)orxHashTable_Get(sstEvent.pstHandlerStorageTable, _pstEvent->eType);

  /* Valid? */
  if(pstStorage != orxNULL)
  {
    /* Should handle this ID? */
    if(pstStorage->au8HandledIDList[_pstEvent->eID] != 0)
    {
      orxEVENT_HANDLER_INFO  *pstInfo, *pstNextInfo;
      orxU32                  u32IDFlag, u32CurrentThread;

      /* Get its ID flag */
      u32IDFlag = orxEVENT_GET_FLAG(_pstEvent->eID);

      /* Gets current thread */
      u32CurrentThread = orxThread_GetCurrent();

      /* Main thread? */
      if(u32CurrentThread == orxTHREAD_KU32_MAIN_THREAD_ID)
      {
        /* Updates event send count */
        sstEvent.s32EventSendCount++;
      }

      /* For all handlers */
      for(pstInfo = (orxEVENT_HANDLER_INFO *)orxLinkList_GetFirst(&(pstStorage->stList));
          pstInfo != orxNULL;
          pstInfo = pstNextInfo)
      {
        /* Gets next info */
        pstNextInfo = (orxEVENT_HANDLER_INFO *)orxLinkList_GetNext(&(pstInfo->stNode));

        /* Should process? */
        if(orxFLAG_TEST(pstInfo->u32IDFlags, u32IDFlag))
        {
          /* Not marked for deletion? */
          if(pstInfo->pfnHandler != orxNULL)
          {
            /* Stores context */
            _pstEvent->pContext = pstInfo->pContext;

            /* Calls it */
            eResult = (pstInfo->pfnHandler)(_pstEvent);
          }

          /* Marked for deletion? */
          if(pstInfo->pfnHandler == orxNULL)
          {
            /* Removes it from list */
            orxLinkList_Remove(&(pstInfo->stNode));

            /* For all IDs */
            for(orxU32 i = 0; i < 32; i++)
            {
              orxU32 u32ID;

              /* Gets it */
              u32ID = (orxU32)(1 << i);

              /* Should be removed? */
              if(orxFLAG_TEST(pstInfo->u32IDFlags, u32ID))
              {
                /* Checks */
                orxASSERT(pstStorage->au8HandledIDList[i] > 0);

                /* Updates ID tracking */
                pstStorage->au8HandledIDList[i]--;
              }
            }

            /* Frees it */
            orxBank_Free(pstStorage->pstBank, pstInfo);
          }

          /* Should stop? */
          if(eResult == orxSTATUS_FAILURE)
          {
            break;
          }
        }
      }

      /* Clears context */
      _pstEvent->pContext = orxNULL;

      /* Main thread? */
      if(u32CurrentThread == orxTHREAD_KU32_MAIN_THREAD_ID)
      {
        /* Updates event send count */
        sstEvent.s32EventSendCount--;
      }
    }
  }

  /* Profiles */
  orxPROFILER_POP_MARKER();

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
  bResult = (sstEvent.s32EventSendCount != 0) ? orxTRUE : orxFALSE;

  /* Done! */
  return bResult;
}
