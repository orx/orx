/**
 * @file orxTest_MessageQueue.c
 * 
 * Message queue Test Program
 * 
 */
 
 /***************************************************************************
 orxTest_MessageQueue.c
 Message Queue Test Program
 
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
#include "core/orxMessageQueue.h"
#include "io/orxTextIO.h"


/******************************************************
 * GLOBAL OBJECTS.
 ******************************************************/
static orxMESSAGE_QUEUE* spstMessageQueue = orxNULL;


/******************************************************
 * TEST FUNCTIONS
 ******************************************************/
/** Display informations about this test module
 */
orxVOID orxTest_MessageQueue_Infos()
{
  orxTextIO_PrintLn("Message queue test module :");
  orxTextIO_PrintLn("This will alloc and manipulate one message queue.");
  orxTextIO_PrintLn("You must create it before use it.");
}

/** Display test module contents and states.
 */
orxVOID orxTest_MessageQueue_DisplayContent()
{
	if(spstMessageQueue==orxNULL)
	    orxTextIO_PrintLn("Message queue not created.");
	else
	{
		orxTextIO_PrintLn("Message queue of %hu / %hu", orxMessageQueue_GetMessageNumber(spstMessageQueue), orxMessageQueue_GetAllocatedNumber(spstMessageQueue));
		orxMESSAGE* pstMessage = orxMessageQueue_GetFirstMessage(spstMessageQueue);
	    orxMESSAGE* pstLastMessage = orxMessageQueue_GetLastMessage(spstMessageQueue);
	    while(pstMessage<=pstLastMessage)
	    {
	    	orxTextIO_PrintLn("  [0x%08lx : %p]", orxMessage_GetIdentifier(pstMessage),
	    										  orxMessage_GetExtraData(pstMessage));
	    	pstMessage = orxMessageQueue_GetNextMessage(pstMessage);
	    }
	}
}

/** Create the message queue.
 */
orxVOID orxTest_MessageQueue_Create()
{	
	if(spstMessageQueue!=orxNULL)
	{
		orxTextIO_PrintLn("Message queue already created, delete it or use resize function.");
	}
	else
	{
		orxS32 s32Number;
		orxTextIO_ReadS32InRange(&s32Number, 10, 1, 65535, "Maximum number of message the message queue will store ? ", orxTRUE);
		spstMessageQueue = orxMessageQueue_Create((orxU16)s32Number);
		if(spstMessageQueue==orxNULL)
			orxTextIO_PrintLn("An error has occure during the message queue creation.");
	}	
}

/** Delete the message queue.
 */
orxVOID orxTest_MessageQueue_Delete()
{	
	if(spstMessageQueue==orxNULL)
	{
		orxTextIO_PrintLn("Message queue not created, create it before delete it.");
	}
	else
	{
		orxMessageQueue_Delete(spstMessageQueue);
		spstMessageQueue = orxNULL;
	}	
}

/** Resize the message queue.
 */
orxVOID orxTest_MessageQueue_Resize()
{	
	if(spstMessageQueue==orxNULL)
	{
		orxTextIO_PrintLn("Message queue not created, create it before rezise it.");
	}
	else
	{
		orxS32 s32Number;
		orxTextIO_ReadS32InRange(&s32Number, 10, 1, 65535, "Maximum number of message the message queue will store ? ", orxTRUE);
		orxMessageQueue_Resize(spstMessageQueue, (orxU16)s32Number);
	}
}

/** Add a message in the queue.
 */
orxVOID orxTest_MessageQueue_AddMessage()
{
	if(spstMessageQueue==orxNULL)
	{
		orxTextIO_PrintLn("Message queue not created, create it before use it.");
	}
	else
	{
		orxS32 s32Number;
		orxS16 s16Time;
		orxU16 u16Owner;
		orxTextIO_ReadS32InRange(&s32Number, 16, 1, 65535, "Owner identifier ? 0x", orxTRUE);
		u16Owner = (orxU16)s32Number;
		orxTextIO_ReadS32InRange(&s32Number, 10, -32766, 32767, "Time life ? ", orxTRUE);
		s16Time = (orxS16)s32Number;
		orxTextIO_ReadS32(&s32Number, 16, "Extra data ? 0x", orxTRUE);
		
		orxMessageQueue_AddMessage(spstMessageQueue, orxMESSAGE_CONSTRUCT_IDENTIFIER(s16Time, u16Owner), (orxVOID*)s32Number);
	}
}

/** Remove a message from the queue.
 */
orxVOID orxTest_MessageQueue_RemoveMessage()
{
	if(spstMessageQueue==orxNULL)
	{
		orxTextIO_PrintLn("Message queue not created, create it before use it.");
	}
	else
	{
		orxS32 s32Number;
		orxTextIO_ReadS32(&s32Number, 16, "Message identifier ? 0x", orxTRUE);
		orxMESSAGE* pstMessage = orxMessageQueue_FindMessage(spstMessageQueue, (orxMESSAGE_IDENTIFIER)s32Number);
		if(pstMessage==orxNULL)
			orxTextIO_PrintLn("Message not found in the queue.");
		else
			orxMessageQueue_RemoveMessage(spstMessageQueue, pstMessage);
	}	
}

/******************************************************
 * DYNAMIC LIBRARY ENTRY POINTS.
 ******************************************************/
orxVOID orxTest_MessageQueue_Init()
{
	/* Initialize bank module */
	orxMAIN_INIT_MODULE(MessageQueue);
	  
	/* Register test functions */
	orxTest_Register("MessageQueue", "Display module informations", orxTest_MessageQueue_Infos);
	orxTest_Register("MessageQueue", "Display test module content", orxTest_MessageQueue_DisplayContent);
	orxTest_Register("MessageQueue", "Create message queue", orxTest_MessageQueue_Create);
	orxTest_Register("MessageQueue", "Delete message queue", orxTest_MessageQueue_Delete);
	orxTest_Register("MessageQueue", "Resize message queue", orxTest_MessageQueue_Resize);
	orxTest_Register("MessageQueue", "Add a message", orxTest_MessageQueue_AddMessage);
	orxTest_Register("MessageQueue", "Remove a message", orxTest_MessageQueue_RemoveMessage);
}

orxVOID orxTest_MessageQueue_Exit()
{
	/** Destroy the global message queue if not done by the user.*/
	if(spstMessageQueue!=orxNULL)
	{
		orxMessageQueue_Delete(spstMessageQueue);
		spstMessageQueue = orxNULL;
	}
		
	/* Uninitialize module */
	orxMAIN_EXIT_MODULE(MessageQueue);
}

orxTEST_DEFINE_ENTRY_POINT(orxTest_MessageQueue_Init, orxTest_MessageQueue_Exit)


