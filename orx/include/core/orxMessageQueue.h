/**
 * @file core/orxMessageQueue.h
 */

/***************************************************************************
 begin                : 18/08/2005
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

/**
 * @addtogroup MessageQueue
 * @{
 */

#ifndef _orxMESSAGEQUEUE_H_
#define _orxMESSAGEQUEUE_H_

#include "orxInclude.h"


/** Message lifetime countdown type.*/
typedef orxS16 orxMESSAGE_LIFETIME;

/** Message owner type.*/
typedef orxU16 orxMESSAGE_OWNER;

/** Message identifier.*/
typedef orxU32 orxMESSAGE_IDENTIFIER;

/** Identifier construcor.
 * @param LIFETIME Message lifetime.
 * @param OWNER Message owner.
 * @return Message identifier.*/
#define orxMESSAGE_CONSTRUCT_IDENTIFIER(LIFETIME, OWNER) ((OWNER<<16)|LIFETIME)

/** Lifetime reader.
 * @param ID Message identifier.
 * @return Message lifetime.*/
#define orxMESSAGE_GET_LIFETIME(ID) ((orxS16)(ID&0x0000FFFF))

/** Owner reader.
 * @param ID Message identifier.
 * @return Message owner.*/
#define orxMESSAGE_GET_OWNER(ID) ((orxU16)((ID&0xFFFF0000)>>16))


/** Message structure.*/
typedef struct __orxMESSAGE_t orxMESSAGE;
/** Message queue structure. */
typedef struct __orxMESSAGE_QUEUE_t		orxMESSAGE_QUEUE;

/** Return a message identifier.
 * @param _pstMessage Adress of the message.
 * @return Identifier of the message.
 */
extern orxMESSAGE_IDENTIFIER orxMessage_GetIdentifier(orxMESSAGE* _pstMessage);

/** Return a message extra data adress.
 * @param _pstMessage Adress of the message.
 * @return Extra data adress of the message.
 */
extern orxVOID* orxMessage_GetExtraData(orxMESSAGE* _pstMessage);



/** Create a message queue.
 * @param _u16MessageNumber Number of message the queue can store.
 * @return Address of the message queue structure.
 */
extern orxMESSAGE_QUEUE *orxMessageQueue_Create(orxU16 _u16MessageNumber);

/** Delete a message queue.
 * @warning All the content will be lost.
 * @param _pstQueue Message queue to delete.
 */
extern orxVOID orxMessageQueue_Delete(orxMESSAGE_QUEUE* _pstQueue);

/** Clean a message queue.
 * @param _pstQueue Address of message queue to clean.
 */
extern orxVOID	orxMessageQueue_Clean(orxMESSAGE_QUEUE* _pstQueue);

/** Get the number of messages stored in the message queue.
 * @param _pstQueue Address of message queue.
 * @return Number of message currently stored.
 */
extern orxU16 orxMessageQueue_GetMessageNumber(orxMESSAGE_QUEUE* _pstQueue);

/** Get the number of messages places allocated for the message queue.
 * @param _pstQueue Address of message queue.
 * @return Number of message places allocated.
 */
extern orxU16 orxMessageQueue_GetAllocatedNumber(orxMESSAGE_QUEUE* _pstQueue);

/** Resize the memssage queue to allocate another number of messages.
 * @warning If used message number is greater than wanted allocation, extra messages will be deleted.
 * @warning Use it with precaution as it is timefull.
 * @param _pstQueue Address of message queue.
 * @param _u16Size New size in number of messages.
 */
extern orxVOID	orxMessageQueue_Resize(orxMESSAGE_QUEUE* _pstQueue, orxU16 _u16Size);

/** Add a message to the queue.
 * If the message queue is full, the last message will be lost.
 * @param _pstQueue Message queue.
 * @param _u32ID Identifier of the message.
 * @param _pData	Adress of extra data.
 * @return Address of the message.
 */
extern orxMESSAGE* orxMessageQueue_AddMessage(orxMESSAGE_QUEUE* _pstQueue, orxMESSAGE_IDENTIFIER _u32ID, orxVOID* _pData);

/** Remove a message from a message queue.
 * @param _pstQueue Message queue.
 * @param _pstMessage Message to remove.
 */
extern void orxMessageQueue_RemoveMessage(orxMESSAGE_QUEUE* _pstQueue, orxMESSAGE* _pstMessage);

/** Find a message in the queue.
 * @param _pstQueue Message queue.
 * @param _u32ID Identifier of the message.
 * @return The first message or orxULL if not found.
 */
extern orxMESSAGE* orxMessageQueue_FindMessage(orxMESSAGE_QUEUE* _pstQueue, orxMESSAGE_IDENTIFIER _u32ID);

/** Return the first message of a message queue.
 * @param _pstQueue Message queue.
 * @return Address of the first message or orxNULL if no message in the queue.
 */
extern orxMESSAGE* orxMessageQueue_GetFirstMessage(orxMESSAGE_QUEUE* _pstQueue);

/** Return the last message of a message queue.
 * @param _pstQueue Message queue.
 * @return Address of the last message or orxNULL if no message in the queue.
 */
extern orxMESSAGE* orxMessageQueue_GetLastMessage(orxMESSAGE_QUEUE* _pstQueue);

/** Return the next message in a message queue.
 * @warning No integrity test are done, compare it with the result of orxMessageQueue_GetLastMessage.
 * @param _pstMessage Message to search the nextone.
 * @return Address of next message.
 */
extern orxMESSAGE* orxMessageQueue_GetNextMessage(orxMESSAGE* _pstMessage);

/** Return the previous message in a message queue.
 * @warning No integrity test are done, compare it with the result of orxMessageQueue_GetFirstMessage.
 * @param _pstMessage Message to search the previousone.
 * @return Address of previous message.
 */
extern orxMESSAGE* orxMessageQueue_GetPreviousMessage(orxMESSAGE* _pstMessage);

 
/** Initialize Message Module
 */
extern orxSTATUS orxDLLAPI orxMessageQueue_Init();

/** Exit Message module
 */
extern orxVOID orxDLLAPI orxMessageQueue_Exit();

#endif /*_orxMESSAGEQUEUE_H_*/
/* @} */
