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
 *   This library is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License           *
 *   as published by the Free Software Foundation; either version 2.1      *
 *   of the License, or (at your option) any later version.                *
 *                                                                         *
 ***************************************************************************/

#include "core/orxEvent.h"

#include "debug/orxDebug.h"
#include "memory/orxMemory.h"

#include "io/orxTextIO.h"


/***************************************************************************
 * Static data and definitions                                             *
 ***************************************************************************/

/*
 * Event module status flag definition.
 */
#define orxEVENT_KU32_STATIC_FLAG_NONE          0x00000000
#define orxEVENT_KU32_STATIC_FLAG_READY         0x00000001


#define orxEVENT_KU32_FLAG_EVENT_DELETED		    0xFFFFFFFF

#define orxEVENT_KU32_EVENT_MESSAGE_QUEUE_SIZE	    0x00001000
#define orxEVENT_KU32_EVENT_CALLBACK_TABLE_SIZE	    0x00000100


/** Define to minimize code writing for module initialization test.*/
#define orxEVENT_ASSERT_MODULE_NOT_INITIALIZED  orxASSERT((sstEvent.u32Flags & orxEVENT_KU32_STATIC_FLAG_READY)  ==  orxEVENT_KU32_STATIC_FLAG_READY)

/*
 * Event module state structure.
 */
typedef struct __orxEVENT_STATIC_t
{
  /* Control flags */
  orxU32 u32Flags;
  
  /** Manipulation flags of queue.*/
  orxU32 u32ManipFlags;
  /** Message queue.*/
  orxQUEUE *pstMessageQueue;
  /** Callback hash table.*/
  orxHASHTABLE *pstCallbackTable;
} orxEVENT_STATIC;


/*
 * Static data
 */
orxSTATIC orxEVENT_STATIC sstEvent;


/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/**
 *  Set the manipulation flags
 */
orxVOID orxFASTCALL orxEvent_SetManipulationFlags(orxU32 _u32Flags)
{
	/** Assert the module is initialized.*/
	orxEVENT_ASSERT_MODULE_NOT_INITIALIZED;

	sstEvent.u32ManipFlags = _u32Flags;
}

/**
 *  Retrieve the manipulation flags.
 */
orxU32  orxFASTCALL orxEvent_GetManipulationFlags()
{
	/** Assert the module is initialized.*/
	orxEVENT_ASSERT_MODULE_NOT_INITIALIZED;

	return sstEvent.u32ManipFlags;
}

/** 
 * Register an event callback function.
 */
orxVOID orxFASTCALL orxEvent_RegisterHandler(orxEVENT_MESSAGE_TYPE _u16Type, orxEVENT_FUNCTION _pfnHandler)
{
	/** Assert the module is initialized.*/
	orxEVENT_ASSERT_MODULE_NOT_INITIALIZED;

	/** Assert the other params.*/
	orxASSERT(_u16Type != 0);
	orxASSERT(_pfnHandler != orxNULL);

	orxHashTable_Set(sstEvent.pstCallbackTable, _u16Type, (orxVOID *)_pfnHandler);
}

/**
 *  Add an event.
 */
orxVOID orxFASTCALL orxEvent_Add(orxEVENT_MESSAGE_TYPE _u16Type, orxEVENT_MESSAGE_LIFETIME _s16Life, orxVOID* _pExtraData)
{
	/** Assert the module is initialized.*/
	orxEVENT_ASSERT_MODULE_NOT_INITIALIZED;
	
	/** Assert the other params.*/
	orxASSERT(_u16Type != 0);
	orxASSERT(_s16Life > 0);
  
	orxQueue_AddItem(sstEvent.pstMessageQueue, orxEVENT_MESSAGE_GET_ID(_u16Type, _s16Life), _pExtraData);
}


/**
 * Retrieve event number.
 */
orxU16 orxFASTCALL orxEvent_GetCount()
{
	/** Assert the module is initialized.*/
	orxEVENT_ASSERT_MODULE_NOT_INITIALIZED;

	return orxQueue_GetItemNumber(sstEvent.pstMessageQueue);
}

/**
 *  Process the events.
 */
orxVOID orxFASTCALL orxEvent_Process(orxS16 _s16Ticks)
{
  orxQUEUE_ITEM *pstCurrentItem;
  orxBOOL bRemoveNegative;
  orxBOOL bRemoveUnprocessed;
  orxBOOL bPartialProcess;
  orxQUEUE_ITEM *pstItem = orxNULL;
  orxQUEUE_ITEM *pstFirstItem = orxNULL;

  /** Assert the module is initialized.*/
  orxEVENT_ASSERT_MODULE_NOT_INITIALIZED;
  
  bRemoveNegative    = orxFLAG_TEST_ALL(sstEvent.u32ManipFlags, orxEVENT_KU32_FLAG_MANIPULATION_REMOVE_NEGATIVE_LIFETIME_EVENT);
  bRemoveUnprocessed = orxFLAG_TEST_ALL(sstEvent.u32ManipFlags, orxEVENT_KU32_FLAG_MANIPULATION_REMOVE_UNPROCESSED);
  bPartialProcess    = orxFLAG_TEST_ALL(sstEvent.u32ManipFlags, orxEVENT_KU32_FLAG_MANIPULATION_PARTIAL_PROCESS);
  
  /** Process all message.*/
  pstCurrentItem = orxQueue_GetFirstItem(sstEvent.pstMessageQueue);
  while(pstCurrentItem != orxNULL)
  {
  	orxU16 u16Type = orxEVENT_MESSAGE_GET_TYPE(orxQueueItem_GetID(pstCurrentItem));
  	orxS16 s16Life = orxEVENT_MESSAGE_GET_LIFETIME(orxQueueItem_GetID(pstCurrentItem));
  	orxVOID* pData = orxQueueItem_GetExtraData(pstCurrentItem);
  	
  	orxTextIO_PrintLn("Process event %04u (%04d - %p)", u16Type, s16Life, pData);
  	
  	/** Decrement lifetime.*/
  	if(s16Life != orxEVENT_KS16_MESSAGE_LIFETIME_CONSTANT)
  	{
    	s16Life = s16Life - _s16Ticks;
  	}
  	
  	if(bRemoveNegative && (s16Life<0))
  	{
  		/** Mark event as removed.*/
		orxQueueItem_SetID(pstCurrentItem, orxEVENT_KU32_FLAG_EVENT_DELETED);  		
  	}
  	if(!bRemoveNegative || (s16Life>=0))
  	{
	  	/** Intend to process it.*/

    	/** Find handler and process event if any.*/
    	orxEVENT_FUNCTION pfnEventHandler = (orxEVENT_FUNCTION)orxHashTable_Get(sstEvent.pstCallbackTable, u16Type);
    	if(pfnEventHandler  !=  orxNULL)
    	{
    		pfnEventHandler(u16Type, s16Life, pData);
	  		/** Mark event as removed.*/
			orxQueueItem_SetID(pstCurrentItem, orxEVENT_KU32_FLAG_EVENT_DELETED);  		
    	}    	
    	else	/** If not processed.*/
    	{
    		if(!bRemoveUnprocessed)
    		{
    			/** set the new lifetime.*/
	    		orxQueueItem_SetID(pstCurrentItem, orxEVENT_MESSAGE_GET_ID(u16Type, s16Life));
    		}
    		else
    		{
		  		/** Mark event as removed.*/
				orxQueueItem_SetID(pstCurrentItem, orxEVENT_KU32_FLAG_EVENT_DELETED);  		
    		}
    	}
  	}
  	
  	/** Find next item to process according to total/partial process config.*/
	pstCurrentItem = orxQueue_GetNextItem(sstEvent.pstMessageQueue, pstCurrentItem);
  	if(bPartialProcess && (pstCurrentItem!=orxNULL))
  	{
  		/** the next item with a different type.*/
  		while(orxEVENT_MESSAGE_GET_TYPE(orxQueueItem_GetID(pstCurrentItem)) == u16Type)
  		{
	    	pstCurrentItem = orxQueue_GetNextItem(sstEvent.pstMessageQueue, pstCurrentItem);
	    	if(pstCurrentItem == orxNULL)
	    		break;
  		}
  	}
  }
  
  /** Clear the queue if not saving no processed events.*/
  if(bRemoveUnprocessed)
  {
	orxQueue_Clean(sstEvent.pstMessageQueue);
  }
  else
  {
	  pstCurrentItem = orxQueue_GetLastItem(sstEvent.pstMessageQueue);
	  pstFirstItem = orxQueue_GetFirstItem(sstEvent.pstMessageQueue);
	  while(pstCurrentItem >= pstFirstItem)
	  {
	  	pstItem = orxQueue_GetPreviousItem(sstEvent.pstMessageQueue, pstCurrentItem);
	  	if(orxQueueItem_GetID(pstCurrentItem)==orxEVENT_KU32_FLAG_EVENT_DELETED)
		{
			orxQueue_RemoveItem(sstEvent.pstMessageQueue, pstCurrentItem);
  		}
		pstCurrentItem = pstItem;
	  }
  }
}

/**
 *  Update the event manager.
 * @param _pstContext Context. Here it is orxEVENT_MANAGER address.
 **/
orxVOID orxFASTCALL orxEvent_Update(orxCONST orxCLOCK_INFO *_pstClockInfo, orxVOID *_pstContext)
{
	/** Assert the module is initialized.*/
	orxEVENT_ASSERT_MODULE_NOT_INITIALIZED;
	/** Assert the params is ok.*/
	orxASSERT(_pstClockInfo != orxNULL);

	/** @todo More _pstClockInfo->u32DT asserts or live verifs.*/
	orxEvent_Process((orxS16)_pstClockInfo->fDT);
}


/***************************************************************************
 * Module management                                                       *
 ***************************************************************************/
 
/***************************************************************************
 orxEvent_Setup
 Event module setup.

 returns: nothing
 ***************************************************************************/
orxVOID orxEvent_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_EVENT, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_EVENT, orxMODULE_ID_CLOCK);
  orxModule_AddDependency(orxMODULE_ID_EVENT, orxMODULE_ID_HASHTABLE);
  orxModule_AddDependency(orxMODULE_ID_EVENT, orxMODULE_ID_QUEUE);

  return;
}

/** 
 * orxEvent_Init
 **/
orxSTATUS orxEvent_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;
  
  /* Not already initialized ? */
  if(!(sstEvent.u32Flags & orxEVENT_KU32_STATIC_FLAG_READY))
  {
    /* Cleans control structure */
    orxMemory_Set(&sstEvent, 0, sizeof(orxEVENT_STATIC));
  
    /* Inits Flags */
    sstEvent.u32Flags = orxEVENT_KU32_STATIC_FLAG_READY;

    sstEvent.pstMessageQueue  = orxQueue_Create(orxEVENT_KU32_EVENT_MESSAGE_QUEUE_SIZE);
    sstEvent.pstCallbackTable = orxHashTable_Create(orxEVENT_KU32_EVENT_CALLBACK_TABLE_SIZE,
    									orxHASHTABLE_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);
	
	if((sstEvent.pstMessageQueue == orxNULL) || (sstEvent.pstCallbackTable == orxNULL))
	{
		if(sstEvent.pstMessageQueue != orxNULL)
		{
			orxQueue_Delete(sstEvent.pstMessageQueue);
		}
		if(sstEvent.pstCallbackTable != orxNULL)
		{
			orxHashTable_Delete(sstEvent.pstCallbackTable);
		}
        /* Successfull Init */
        eResult = orxSTATUS_FAILURE;
    }
    else
    {
      orxQueue_Clean(sstEvent.pstMessageQueue);
      orxHashTable_Clear(sstEvent.pstCallbackTable);
      sstEvent.u32ManipFlags = orxEVENT_KU32_FLAG_MANIPULATION_STANDARD;

      /* Successfull Init */
      eResult = orxSTATUS_SUCCESS;
	}
  }
  else
  {
    /* !!! MSG !!! */

    /* Already initialized */
    eResult = orxSTATUS_SUCCESS;
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
  if(sstEvent.u32Flags & orxEVENT_KU32_STATIC_FLAG_READY)
  {
    /* Updates flags */
    sstEvent.u32Flags &= ~orxEVENT_KU32_STATIC_FLAG_READY;
    
    /* Free message queue */
    if(sstEvent.pstMessageQueue)
    {
    	orxQueue_Delete(sstEvent.pstMessageQueue);
    	sstEvent.pstMessageQueue = orxNULL;
    }
    
    /* Free callback table */
    if(sstEvent.pstCallbackTable)
    {
    	
    	orxHashTable_Delete(sstEvent.pstCallbackTable);
    	sstEvent.pstCallbackTable = orxNULL;
    }
  }
  else
  {
    /* !!! MSG !!! */
  }

  return;
}
