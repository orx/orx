/**
 * @file orxTest_Event.c
 * 
 * Event Test Program
 * 
 */
 
 /***************************************************************************
 orxTest_Event.c
 Event Test Program
 
 begin                : 12/10/2005
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

#include "orxInclude.h"
#include "debug/orxTest.h"
#include "core/orxEvent.h"
#include "io/orxTextIO.h"

/******************************************************
 * DEFINES
 ******************************************************/

#define orxTEST_EVENT_KU32_EVENT_MAX_NUMBER  200
#define orxTEST_EVENT_KU32_EVENT_MAX_HANDLER 40

/******************************************************
 * TYPES AND STRUCTURES
 ******************************************************/

typedef struct __orxTEST_EVENT_t
{
	orxEVENT_MANAGER *pstEventManager;	/** The event manager.*/
	orxU32 u32MaxEvent;
	orxU32 u32MaxHandler;
} orxTEST_EVENT;

orxSTATIC orxTEST_EVENT sstTest_Event;



/******************************************************
 * Handler functions.
 ******************************************************/
orxVOID orxTest_Event_Handler_1(orxEVENT_MESSAGE_TYPE u16Type, orxEVENT_MESSAGE_LIFETIME s16Time, orxVOID *pData)
{
	orxTextIO_PrintLn("(1) Handle message %04hu : %hd - %p", u16Type, s16Time, pData);
}

orxVOID orxTest_Event_Handler_2(orxEVENT_MESSAGE_TYPE u16Type, orxEVENT_MESSAGE_LIFETIME s16Time, orxVOID *pData)
{
	orxTextIO_PrintLn("(2) Handle message %04hu : %hd - %p", u16Type, s16Time, pData);
}

orxVOID orxTest_Event_Handler_3(orxEVENT_MESSAGE_TYPE u16Type, orxEVENT_MESSAGE_LIFETIME s16Time, orxVOID *pData)
{
	orxTextIO_PrintLn("(3) Handle message %04hu : %hd - %p", u16Type, s16Time, pData);
}

/******************************************************
 * TEST FUNCTIONS
 ******************************************************/

/** Display informations about this test module
 */
orxVOID orxTest_Event_Infos()
{
	orxTextIO_PrintLn("This module is able to manage a event loop manually.");
	orxTextIO_PrintLn("You have 3 handler functions to simulate real actions.");
	orxTextIO_PrintLn("You can use %d handlers for %d messages (maximum).", sstTest_Event.u32MaxHandler, sstTest_Event.u32MaxEvent);
}

/** Display module state
 */
orxVOID orxTest_Event_DisplayState()
{
	/** Log message queue content : .*/	
    orxQUEUE_ITEM* pstItem;
    orxQUEUE_ITEM* pstLastItem;
    orxTextIO_PrintLn("%hu messages :", orxQueue_GetItemNumber(sstTest_Event.pstEventManager->pstMessageQueue));
    pstItem = orxQueue_GetFirstItem(sstTest_Event.pstEventManager->pstMessageQueue);
    pstLastItem = orxQueue_GetLastItem(sstTest_Event.pstEventManager->pstMessageQueue);
    while(pstItem!=orxNULL && pstItem<=pstLastItem)
    {
    	orxTextIO_PrintLn("%p", pstItem);
      orxU32 u32ID = orxQueueItem_GetID(pstItem);
      orxTextIO_PrintLn("  [0x%04x - %d : %p]", orxEVENT_MESSAGE_GET_TYPE(u32ID),
      										orxEVENT_MESSAGE_GET_LIFETIME(u32ID),
	    							      orxQueueItem_GetExtraData(pstItem));
      pstItem = orxQueue_GetNextItem(sstTest_Event.pstEventManager->pstMessageQueue, pstItem);
    }

	/** Log handler content : .*/
  	orxU32 u32Key;
  	orxVOID* pData;
  	orxVOID* pIter;  	
  	pIter = orxHashMap_FindFirst(sstTest_Event.pstEventManager->pstCallbackTable, &u32Key, &pData);
  	if(pIter==NULL)
	    orxTextIO_PrintLn("No handler registered.");
	else
	{
	    orxTextIO_PrintLn("Handler registered :");
		while(pIter!=NULL)
		{
	      orxTextIO_PrintLn("  [%u : %p]", u32Key, pData);
		  pIter = orxHashMap_FindNext(sstTest_Event.pstEventManager->pstCallbackTable, pIter, &u32Key, &pData);
		}
	}
	
}

/** Register an handler.
 */
orxVOID orxTest_Event_RegisterHandler()
{
	orxS32 s32;
	orxU16 u16ID;
	
    /* Get the ID to handle. */
    orxTextIO_ReadS32InRange(&s32, 10, 0, 65535, "Choose the ID to handle : ", orxTRUE);
    u16ID = (orxU16) s32;
    
    /* Get the num of function. */
    orxTextIO_ReadS32InRange(&s32, 10, 1, 3, "Choose the handler function (1-3, 0 to unregister) : ", orxTRUE);
	
	switch(s32)
	{
	case 1:
		orxEvent_RegisterHandler(sstTest_Event.pstEventManager, u16ID, orxTest_Event_Handler_1);
		break;
	case 2:
		orxEvent_RegisterHandler(sstTest_Event.pstEventManager, u16ID, orxTest_Event_Handler_2);
		break;
	case 3:
		orxEvent_RegisterHandler(sstTest_Event.pstEventManager, u16ID, orxTest_Event_Handler_3);
		break;
	default:
		orxEvent_RegisterHandler(sstTest_Event.pstEventManager, u16ID, orxNULL);
		break;
	}
    orxTextIO_PrintLn("Handler registered.");
	
}

/** Add an event.
 */
orxVOID orxTest_Event_AddEvent()
{
	static orxU32 s_u32Count = 0;
	s_u32Count++;
	
	orxS32 s32;
	orxU16 u16ID;
	orxS16 s16Time;
	
    /* Get the message ID. */
    orxTextIO_ReadS32InRange(&s32, 10, 0, 65535, "Choose the ID of the message : ", orxTRUE);
    u16ID = (orxU16) s32;
    /* Get the message life time. */
    orxTextIO_ReadS32InRange(&s32, 10, -32000, 32000, "Choose the message life time : ", orxTRUE);
    s16Time = (orxU16) s32;
	
	orxEvent_Add(sstTest_Event.pstEventManager, u16ID, s16Time, (orxVOID*)s_u32Count);
}

/******************************************************
 * DYNAMIC LIBRARY ENTRY POINTS
 ******************************************************/
orxVOID orxTest_Event_Init()
{
  /* Initialize bank module */
  orxModule_Init(orxMODULE_ID_EVENT);
  
  /* Register test functions */
  orxTest_Register("Event", "Display module informations", orxTest_Event_Infos);
  orxTest_Register("Event", "Display module state", orxTest_Event_DisplayState);
  orxTest_Register("Event", "(Un)Register event handler", orxTest_Event_RegisterHandler);
  orxTest_Register("Event", "Add an event", orxTest_Event_AddEvent);
  
  /* Initialize static datas */
  sstTest_Event.u32MaxEvent = orxTEST_EVENT_KU32_EVENT_MAX_NUMBER;
  sstTest_Event.u32MaxHandler = orxTEST_EVENT_KU32_EVENT_MAX_HANDLER;
  sstTest_Event.pstEventManager = orxEvent_CreateManager(orxTEST_EVENT_KU32_EVENT_MAX_NUMBER, orxTEST_EVENT_KU32_EVENT_MAX_HANDLER, orxEVENT_KU32_FLAG_MANIPULATION_STANDARD);

}

orxVOID orxTest_Event_Exit()
{
	/** Delete static data.*/
	orxEvent_DeleteManager(sstTest_Event.pstEventManager);
	sstTest_Event.pstEventManager = orxNULL;
}

orxTEST_DEFINE_ENTRY_POINT(orxTest_Event_Init, orxTest_Event_Exit)

