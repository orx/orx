/**
 * @file core/orxEvent.c
 */

/***************************************************************************
 begin                : 01/09/2005
 author               : (C) Arcallians
 email                : cursor@arcallians.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "core/orxEvent.h"
#include "memory/orxMemory.h"
#include "utils/orxHashTable.h"
#include "utils/orxQueue.h"


/***************************************************************************
 * Static data and definitions                                             *
 ***************************************************************************/

/*
 * Event module status flag definition.
 */
#define orxEVENT_KU32_FLAG_NONE              0x00000000
#define orxEVENT_KU32_FLAG_READY             0x00000001

/** Define to minimize code writing for module initialization test.*/
#define orxEVENT_ASSERT_MODULE_ISNT_INITIALIZED orxASSERT((sstEvent.u32Flags & orxEVENT_KU32_FLAG_READY) == orxEVENT_KU32_FLAG_READY);

/*
 * Event module state structure.
 */
typedef struct __orxEvent_STATIC_t
{
  /* Control flags */
  orxU32 u32Flags;
} orxEVENT_STATIC;


/*
 * Static data
 */
orxSTATIC orxEVENT_STATIC sstEvent;

/***************************************************************************
 * Private definitions                                                     *
 ***************************************************************************/
struct __orxEVENT_MANAGER_t
{
	/** Message queue.*/
	orxQUEUE* pstMessageQueue;
	/** Callback hash table.*/
	orxHASHTABLE* pstCallbackTable;
};



/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/
/**
 *  Create an event manager.
 */
orxEVENT_MANAGER *orxEventManager_Create(orxU16 _u16Number, orxU16 _u16HandlerNumber)
{
	/** Assert the module is initialized.*/
	orxEVENT_ASSERT_MODULE_ISNT_INITIALIZED
	
	orxEVENT_MANAGER* pstManager   = orxMemory_Allocate(sizeof(orxEVENT_MANAGER), orxMEMORY_TYPE_MAIN);
	orxQUEUE* pstMessageQueue      = orxQueue_Create(_u16Number);
	orxHASHTABLE* pstCallbackTable = orxHashTable_Create(_u16HandlerNumber, orxHASHTABLE_KU32_FLAGS_NONE, orxMEMORY_TYPE_MAIN);
	
	if( (pstManager==orxNULL) || (pstMessageQueue==orxNULL) || (pstCallbackTable==orxNULL))
	{
		if(pstManager!=orxNULL)
		{
			orxMemory_Free(pstManager);
		}
		if(pstMessageQueue!=orxNULL)
		{
			orxQueue_Delete(pstMessageQueue);
		}
		if(pstCallbackTable!=orxNULL)
		{
			orxHashTable_Delete(pstCallbackTable);
		}
		return orxNULL;
	}

	orxQueue_Clean(pstMessageQueue);
	orxHashTable_Clear(pstCallbackTable);
	
	pstManager->pstMessageQueue  = pstMessageQueue;
	pstManager->pstCallbackTable = pstCallbackTable;
	return pstManager;
}

/**
 *  Delete an event manager.
 */
orxVOID orxEventManager_Delete(orxEVENT_MANAGER* _pstEventManager)
{
	/** Assert the module is initialized.*/
	orxEVENT_ASSERT_MODULE_ISNT_INITIALIZED
    /** Assert the Event manager is ok.*/
    orxASSERT(_pstEventManager!=orxNULL);
    
	orxQueue_Delete(_pstEventManager->pstMessageQueue);
	orxHashTable_Delete(_pstEventManager->pstCallbackTable);
	orxMemory_Free(_pstEventManager);	
}


/** 
 * Register an event callback function.
 */
orxVOID orxEventManager_RegisterHandler(orxEVENT_MANAGER* _pstEventManager, orxEVENT_MESSAGE_TYPE _u16Type, orxEVENT_CB _cbHandler)
{
	/** Assert the module is initialized.*/
	orxEVENT_ASSERT_MODULE_ISNT_INITIALIZED
    /** Assert the Event manager is ok.*/
    orxASSERT(_pstEventManager!=orxNULL);
    /** Assert the other params.*/
    orxASSERT(_u16Type!=0);
    orxASSERT(_cbHandler!=orxNULL);

	orxHashTable_Set(_pstEventManager->pstCallbackTable, _u16Type, (orxVOID*)_cbHandler);
}

/**
 *  Add an event to the manager.
 */
orxVOID orxEventManager_AddEvent(orxEVENT_MANAGER* _pstEventManager, orxEVENT_MESSAGE_TYPE _u16Type, orxEVENT_MESSAGE_LIFETIME _s16Life, orxVOID* _pExtraData)
{
	/** Assert the module is initialized.*/
	orxEVENT_ASSERT_MODULE_ISNT_INITIALIZED
    /** Assert the Event manager is ok.*/
    orxASSERT(_pstEventManager!=orxNULL);
    /** Assert the other params.*/
    orxASSERT(_u16Type!=0);
    orxASSERT(_s16Life>0);
    
    orxQueue_AddItem(_pstEventManager->pstMessageQueue, orxEVENT_MESSAGE_GET_ID(_u16Type, _s16Life), _pExtraData);
}

/**
 *  Process the events.
 */
orxVOID orxEventManager_ProcessEvent(orxEVENT_MANAGER* _pstEventManager)
{
	/** Assert the module is initialized.*/
	orxEVENT_ASSERT_MODULE_ISNT_INITIALIZED
    /** Assert the Event manager is ok.*/
    orxASSERT(_pstEventManager!=orxNULL);
    
    while(orxQueue_GetItemNumber(_pstEventManager->pstMessageQueue)>0)
    {
    	orxQUEUE_ITEM* pstItem = orxQueue_GetFirstItem(_pstEventManager->pstMessageQueue);
    	
    	orxU16 u16Type = orxEVENT_MESSAGE_GET_TYPE(orxQueueItem_GetID(pstItem));
    	orxS16 s16Life = orxEVENT_MESSAGE_GET_LIFETIME(orxQueueItem_GetID(pstItem));
    	orxVOID* pData = orxQueueItem_GetExtraData(pstItem);
    	
    	orxEVENT_CB pEventHandler = (orxEVENT_CB) orxHashTable_Get(_pstEventManager->pstCallbackTable, u16Type);
    	if(pEventHandler!=orxNULL)
    	{
    		pEventHandler(u16Type, s16Life, pData);
    	}
    	
    	orxQueue_RemoveItem(_pstEventManager->pstMessageQueue, pstItem);
    }
}


/***************************************************************************
 * Module management                                                       *
 ***************************************************************************/
 
/** 
 * orxEvent_Init
 **/
orxSTATUS orxEvent_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILED;
  
  /* Init dependencies */
  if ((orxMAIN_INIT_MODULE(Memory) == orxSTATUS_SUCCESS) &&
      (orxMAIN_INIT_MODULE(HashTable) == orxSTATUS_SUCCESS) &&
  	  (orxMAIN_INIT_MODULE(Queue) == orxSTATUS_SUCCESS))
  {
    /* Not already initialized ? */
    if(!(sstEvent.u32Flags & orxEVENT_KU32_FLAG_READY))
    {
      /* Cleans control structure */
      orxMemory_Set(&sstEvent, 0, sizeof(orxEVENT_STATIC));
    
      /* Inits Flags */
      sstEvent.u32Flags = orxEVENT_KU32_FLAG_READY;
      
      /* Successfull Init */
      eResult = orxSTATUS_SUCCESS;
    }
    else
    {
      /* !!! MSG !!! */
    }
  }

  /* Done! */
  return eResult;
}

/**
 * orxEvent_Exit
 **/
orxVOID orxEvent_Exit()
{
  /* Initialized? */
  if(sstEvent.u32Flags & orxEVENT_KU32_FLAG_READY)
  {
    /* Updates flags */
    sstEvent.u32Flags &= ~orxEVENT_KU32_FLAG_READY;
  }
  else
  {
    /* !!! MSG !!! */
  }

  /* Exit dependencies */
  orxMAIN_EXIT_MODULE(Queue);
  orxMAIN_EXIT_MODULE(HashTable);
  orxMAIN_EXIT_MODULE(Memory);

  return;
}

