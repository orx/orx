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
#include "debug/orxDebug.h"


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
	/** Manipulation flags of queue.*/
	orxU32 u32ManipFlags;
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
orxEVENT_MANAGER *orxEventManager_Create(orxU16 _u16Number, orxU16 _u16HandlerNumber, orxU32 _u32Flags)
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
	pstManager->u32ManipFlags = _u32Flags;
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
 *  Set the flags of event manager.
 */
orxVOID orxEventManager_SetFlags(orxEVENT_MANAGER* _pstEventManager, orxU32 _u32Flags)
{
	/** Assert the module is initialized.*/
	orxEVENT_ASSERT_MODULE_ISNT_INITIALIZED
    /** Assert the Event manager is ok.*/
    orxASSERT(_pstEventManager!=orxNULL);
    
    _pstEventManager->u32ManipFlags = _u32Flags;
}

/**
 *  Retrieve the flags of the event manager.
 */
orxU32  orxEventManager_GetFlags(orxEVENT_MANAGER* _pstEventManager)
{
	/** Assert the module is initialized.*/
	orxEVENT_ASSERT_MODULE_ISNT_INITIALIZED
    /** Assert the Event manager is ok.*/
    orxASSERT(_pstEventManager!=orxNULL);
    
    return _pstEventManager->u32ManipFlags;
}

/** 
 * Register an event callback function.
 */
orxVOID orxEventManager_RegisterHandler(orxEVENT_MANAGER* _pstEventManager, orxEVENT_MESSAGE_TYPE _u16Type, orxEVENT_FUNCTION _pfnHandler)
{
	/** Assert the module is initialized.*/
	orxEVENT_ASSERT_MODULE_ISNT_INITIALIZED
    /** Assert the Event manager is ok.*/
    orxASSERT(_pstEventManager!=orxNULL);
    /** Assert the other params.*/
    orxASSERT(_u16Type!=0);
    orxASSERT(_pfnHandler!=orxNULL);

	orxHashTable_Set(_pstEventManager->pstCallbackTable, _u16Type, (orxVOID*)_pfnHandler);
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
orxVOID orxEventManager_ProcessEvent(orxEVENT_MANAGER* _pstEventManager, orxS16 _s16Ticks)
{
	/** Assert the module is initialized.*/
	orxEVENT_ASSERT_MODULE_ISNT_INITIALIZED
    /** Assert the Event manager is ok.*/
    orxASSERT(_pstEventManager!=orxNULL);
    
    orxBOOL bRemoveNegative = orxFLAG32_TEST(_pstEventManager->u32ManipFlags, orxEVENT_MANAGER_MANIPULATION_REMOVE_NEGATIVE_LIFETIME_EVENT);
    orxBOOL bRemoveUnprocessed = orxFLAG32_TEST(_pstEventManager->u32ManipFlags, orxEVENT_MANAGER_MANIPULATION_REMOVE_UNPROCESSED);
    orxBOOL bPartialProcess = orxFLAG32_TEST(_pstEventManager->u32ManipFlags, orxEVENT_MANAGER_MANIPULATION_PARTIAL_PROCESS);
    
    orxQUEUE_ITEM* pstCurrentItem = orxQueue_GetFirstItem(_pstEventManager->pstMessageQueue);
    orxQUEUE_ITEM* pstItem;
    
    while(pstCurrentItem!=orxNULL)
    {
    	orxU16 u16Type = orxEVENT_MESSAGE_GET_TYPE(orxQueueItem_GetID(pstItem));
    	orxS16 s16Life = orxEVENT_MESSAGE_GET_LIFETIME(orxQueueItem_GetID(pstItem));
    	orxVOID* pData = orxQueueItem_GetExtraData(pstItem);
    	
    	orxBOOL bProcessed = orxFALSE;
    	
    	/** Decrement lifetime.*/
    	if(s16Life!=orxEVENT_MESSAGE_LIFETIME_CONSTANT)
    	{
	    	s16Life -= _s16Ticks;
    	}
    	
    	/** Intend to process it if lifetime is correct.*/
    	if(!bRemoveNegative || (bRemoveNegative && s16Life>=0))
    	{
	    	/** Find handler and process event if any.*/
	    	orxEVENT_FUNCTION pfnEventHandler = (orxEVENT_FUNCTION)orxHashTable_Get(_pstEventManager->pstCallbackTable, u16Type);
	    	if(pfnEventHandler != orxNULL)
	    	{
	    		pfnEventHandler(u16Type, s16Life, pData);
	    		bProcessed = orxTRUE;
	    	}
	    	
	    	/** If not processed, restore new lifetime.*/
	    	if(!bProcessed && !bRemoveUnprocessed)
	    	{
	    		orxQueueItem_SetID(pstItem, orxEVENT_MESSAGE_GET_ID(u16Type, s16Life));
	    	}
    	}
    	
    	
    	/** Find next item to process according to total/partial process config.*/
    	pstItem = pstCurrentItem;
    	if(bPartialProcess)
    	{
    		/** the next item with a different type.*/
    		while(orxEVENT_MESSAGE_GET_TYPE(orxQueueItem_GetID(pstCurrentItem))==u16Type)
    		{
		    	pstCurrentItem = orxQueue_GetNextItem(_pstEventManager->pstMessageQueue, pstCurrentItem);
		    	if(pstCurrentItem==orxNULL)
		    		break;
    		}
    	}
    	else
    	{
	    	pstCurrentItem = orxQueue_GetNextItem(_pstEventManager->pstMessageQueue, pstCurrentItem);
	    	/** Suppress if processed, unused or if olded.*/
	    	if(bProcessed || (bRemoveNegative && s16Life<0))
	    	{
		    	orxQueue_RemoveItem(_pstEventManager->pstMessageQueue, pstItem);
	    	}
    	}
    }
    
    /** Clear the queue if not saving no processed events.*/
    if(bRemoveUnprocessed)
    {
    	orxQueue_Clean(_pstEventManager->pstMessageQueue);
    }
}

/**
 *  Update the event manager.
 * @param _pstContext Context. Here it is orxEVENT_MANAGER address.
 **/
orxVOID orxEventManager_Update(orxCONST orxCLOCK_INFO *_pstClockInfo, orxVOID *_pstContext)
{
	/** Assert the module is initialized.*/
	orxEVENT_ASSERT_MODULE_ISNT_INITIALIZED
    /** Assert the params is ok.*/
    orxASSERT(_pstClockInfo!=orxNULL);
    orxASSERT(_pstContext!=orxNULL);
	
	/** @todo More _pstClockInfo->u32DT asserts or live verifs.*/
	orxEventManager_ProcessEvent((orxEVENT_MANAGER*)_pstContext, (orxS16)_pstClockInfo->u32DT);
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
  if (orxDEPEND_INIT(Memory) &&
      orxDEPEND_INIT(Clock) &&
      orxDEPEND_INIT(HashTable) &&
  	  orxDEPEND_INIT(Queue))
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
  orxDEPEND_EXIT(Queue);
  orxDEPEND_EXIT(HashTable);
  orxDEPEND_EXIT(Clock);
  orxDEPEND_EXIT(Memory);

  return;
}

