/**
 * @file orxTest_Queue.c
 * 
 * Message queue Test Program
 * 
 */
 
 /***************************************************************************
 orxTest_Queue.c
 Queue Test Program
 
 begin                : 22/08/2005
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
#include "utils/orxTest.h"
#include "utils/orxQueue.h"
#include "io/orxTextIO.h"


/******************************************************
 * GLOBAL OBJECTS.
 ******************************************************/
orxSTATIC orxQUEUE* spstQueue = orxNULL;


/******************************************************
 * TEST FUNCTIONS
 ******************************************************/
/** Display informations about this test module
 */
orxVOID orxTest_Queue_Infos()
{
  orxTextIO_PrintLn("Queue test module :");
  orxTextIO_PrintLn("This will alloc and manipulate one queue.");
  orxTextIO_PrintLn("You must create it before use it.");
}

/** Display test module contents and states.
 */
orxVOID orxTest_Queue_DisplayContent()
{
	if(spstQueue==orxNULL)
	    orxTextIO_PrintLn("Queue not created.");
	else
	{
		orxTextIO_PrintLn("Queue of %hu / %hu", orxQueue_GetItemNumber(spstQueue), orxQueue_GetAllocatedNumber(spstQueue));
		orxQUEUE_ITEM* pstItem = orxQueue_GetFirstItem(spstQueue);
	    orxQUEUE_ITEM* pstLastItem = orxQueue_GetLastItem(spstQueue);
	    while(pstItem<=pstLastItem)
	    {
	    	orxTextIO_PrintLn("  [0x%08lx : %p]", orxQueueItem_GetID(pstItem),
	    										  orxQueueItem_GetExtraData(pstItem));
	    	pstItem = orxQueue_GetNextItem(spstQueue, pstItem);
	    }
	}
}

/** Create the queue.
 */
orxVOID orxTest_Queue_Create()
{	
	if(spstQueue!=orxNULL)
	{
		orxTextIO_PrintLn("Queue already created, delete it or use resize function.");
	}
	else
	{
		orxS32 s32Number;
		orxTextIO_ReadS32InRange(&s32Number, 10, 1, 65535, "Maximum number of item the queue will store ? ", orxTRUE);
		spstQueue = orxQueue_Create((orxU16)s32Number);
		if(spstQueue==orxNULL)
			orxTextIO_PrintLn("An error has occure during the queue creation.");
	}	
}

/** Delete the queue.
 */
orxVOID orxTest_Queue_Delete()
{	
	if(spstQueue==orxNULL)
	{
		orxTextIO_PrintLn("Queue not created, create it before delete it.");
	}
	else
	{
		orxQueue_Delete(spstQueue);
		spstQueue = orxNULL;
	}	
}

/** Resize the queue.
 */
orxVOID orxTest_Queue_Resize()
{	
	if(spstQueue==orxNULL)
	{
		orxTextIO_PrintLn("Queue not created, create it before rezise it.");
	}
	else
	{
		orxS32 s32Number;
		orxTextIO_ReadS32InRange(&s32Number, 10, 1, 65535, "Maximum number of items the queue will store ? ", orxTRUE);
		orxQueue_Resize(spstQueue, (orxU16)s32Number);
	}
}

/** Add an item in the queue.
 */
orxVOID orxTest_Queue_AddItem()
{
	if(spstQueue==orxNULL)
	{
		orxTextIO_PrintLn("Queue not created, create it before use it.");
	}
	else
	{
		orxS32 s32Number;
		orxU32 u32ID;
		orxTextIO_ReadS32(&s32Number, 16, "ID ? 0x", orxTRUE);
		u32ID = (orxU32)s32Number;
		orxTextIO_ReadS32(&s32Number, 16, "Extra data ? 0x", orxTRUE);
		
		orxQueue_AddItem(spstQueue, u32ID, (orxVOID*)s32Number);
	}
}

/** Remove an item from the queue.
 */
orxVOID orxTest_Queue_RemoveItem()
{
	if(spstQueue==orxNULL)
	{
		orxTextIO_PrintLn("Queue not created, create it before use it.");
	}
	else
	{
		orxS32 s32Number;
		orxTextIO_ReadS32(&s32Number, 16, "Item identifier ? 0x", orxTRUE);
		orxQUEUE_ITEM* pstItem = orxQueue_FindItem(spstQueue, (orxU32)s32Number);
		if(pstItem==orxNULL)
			orxTextIO_PrintLn("Item not found in the queue.");
		else
			orxQueue_RemoveItem(spstQueue, pstItem);
	}	
}

/******************************************************
 * DYNAMIC LIBRARY ENTRY POINTS.
 ******************************************************/
orxVOID orxTest_Queue_Init()
{
	/* Initialize bank module */
	orxDEPEND_INIT(Queue);
	  
	/* Register test functions */
	orxTest_Register("Queue", "Display module informations", orxTest_Queue_Infos);
	orxTest_Register("Queue", "Display test module content", orxTest_Queue_DisplayContent);
	orxTest_Register("Queue", "Create queue", orxTest_Queue_Create);
	orxTest_Register("Queue", "Delete queue", orxTest_Queue_Delete);
	orxTest_Register("Queue", "Resize  queue", orxTest_Queue_Resize);
	orxTest_Register("Queue", "Add an item", orxTest_Queue_AddItem);
	orxTest_Register("Queue", "Remove an item", orxTest_Queue_RemoveItem);
}

orxVOID orxTest_Queue_Exit()
{
	/** Destroy the global queue if not done by the user.*/
	if(spstQueue!=orxNULL)
	{
		orxQueue_Delete(spstQueue);
		spstQueue = orxNULL;
	}
		
	/* Uninitialize module */
	orxDEPEND_EXIT(Queue);
}

orxTEST_DEFINE_ENTRY_POINT(orxTest_Queue_Init, orxTest_Queue_Exit)
