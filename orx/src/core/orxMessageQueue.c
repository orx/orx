/**
 * @file core/orxMessageQueue.c
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


#include "core/orxMessageQueue.h"
#include "memory/orxMemory.h"
#include "math/orxMath.h"
#include "debug/orxDebug.h"

#include "io/orxTextIO.h"


/*
 * Message module status flag definition.
 */
#define orxMESSAGEQUEUE_KU32_FLAG_NONE              0x00000000
#define orxMESSAGEQUEUE_KU32_FLAG_READY             0x00000001

/*
 * Message module state structure.
 */
typedef struct __orxMESSAGEQUEUE_MODULE_STATE_t
{
  /* Control flags */
  orxU32 u32Flags;
} orxMESSAGEQUEUE_MODULE_STATE;


/*
 * Static data
 */
orxSTATIC orxMESSAGEQUEUE_MODULE_STATE sstMessageQueue;



/** Message association structure. */
struct __orxMESSAGE_t
{
	/** Message identifier.*/
	orxMESSAGE_IDENTIFIER u32Identifier;
	/** Message extra data.*/
	orxVOID* pExtraData;
};


/** Message queue structure. */
struct __orxMESSAGE_QUEUE_t
{
	/** Number of allocated message.*/
	orxU16	u16Alloc;
	/** Number of used messages.*/
	orxU16 u16Used;
	/** Address of message array.*/
	orxMESSAGE* pastMessages;
};


/***************************************************************************
 * Private functions                                                        *
 ***************************************************************************/

/**
 *  Clear a message content.
 * @param _pstMessage Message to clear.
 */
orxINLINE orxVOID orxMessage_Clear(orxMESSAGE* _pstMessage)
{
	_pstMessage->pExtraData    = orxNULL;
	_pstMessage->u32Identifier = 0x00000000;
}

/**
 * Set a message content.
 * @param _pstMessage Message to set.
 * @param _u32ID Identifier.
 * @param _pData Extra data.
 */
orxINLINE orxVOID orxMessage_Set(orxMESSAGE* _pstMessage, orxMESSAGE_IDENTIFIER _u32ID, orxVOID* _pData)
{
	_pstMessage->u32Identifier = _u32ID;
	_pstMessage->pExtraData    = _pData;
}

/**
 * Copy the message content to another.
 * @param _pstSourceMessage Source message.
 * @param _pstTargetMessage Target Message.
 */
orxINLINE orxVOID orxMessage_Copy(orxMESSAGE* _pstSourceMessage, orxMESSAGE* _pstTargetMessage)
{
	orxMemory_Copy(_pstTargetMessage, _pstSourceMessage, sizeof(orxMESSAGE));
}


/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/
 
/** 
 * orxMessageQueue_Init
 **/
orxSTATUS orxMessageQueue_Init()
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /** @note Depend of Memory, maths and debug modules.*/
  /** @todo Ajouter le code d'initialisation.*/

  /* Initialized? */
  if(eResult == orxSTATUS_SUCCESS)
  {
    /* Inits Flags */
    sstMessageQueue.u32Flags = orxMESSAGEQUEUE_KU32_FLAG_READY;
  }
  else
  {
    /* !!! MSG !!! */
  }

  /* Done! */
  return eResult;
}

/**
 * orxMessageQueue_Exit
 **/
orxVOID orxMessageQueue_Exit()
{
  /* Initialized? */
  if(sstMessageQueue.u32Flags & orxMESSAGEQUEUE_KU32_FLAG_READY)
  {
    /* Updates flags */
    sstMessageQueue.u32Flags &= ~orxMESSAGEQUEUE_KU32_FLAG_READY;
  }
  else
  {
    /* !!! MSG !!! */
  }

  return;
}

/**
 *  Return a message identifier.
 */
orxMESSAGE_IDENTIFIER orxMessage_GetIdentifier(orxMESSAGE* _pstMessage)
{
	return _pstMessage->u32Identifier;
}


/**
 *  Return a message extra data adress.
 */
orxVOID* orxMessage_GetExtraData(orxMESSAGE* _pstMessage)
{
	return _pstMessage->pExtraData;
}



/**
 * Create a message queue.
 */
orxMESSAGE_QUEUE *orxMessageQueue_Create(orxU16 _u16MessageNumber)
{
	/** Assert the number of message is superior to 0.*/
	orxASSERT(_u16MessageNumber>0);
	
	orxMESSAGE_QUEUE* pstQueue = (orxMESSAGE_QUEUE*) orxMemory_Allocate(sizeof(orxMESSAGE_QUEUE), orxMEMORY_TYPE_MAIN);
	pstQueue->pastMessages = (orxMESSAGE*) orxMemory_Allocate(sizeof(orxMESSAGE)*_u16MessageNumber, orxMEMORY_TYPE_MAIN);
	pstQueue->u16Alloc = _u16MessageNumber;
	pstQueue->u16Used = 0;
	orxMessageQueue_Clean(pstQueue);
	return pstQueue;
}

/**
 *  Delete a message queue.
 */
orxVOID orxMessageQueue_Delete(orxMESSAGE_QUEUE* _pstQueue)
{
	/** Assert the queue is exists.*/
	orxASSERT(_pstQueue!=NULL);

	if(_pstQueue->pastMessages!=orxNULL)
		orxMemory_Free(_pstQueue->pastMessages);
	orxMemory_Free(_pstQueue);
}


/**
 * Clean a message queue.
 */
orxVOID	orxMessageQueue_Clean(orxMESSAGE_QUEUE* _pstQueue)
{
	/** Assert the queue is exists.*/
	orxASSERT(_pstQueue!=NULL);

	orxMemory_Set(_pstQueue->pastMessages, 0, sizeof(_pstQueue->pastMessages));
	_pstQueue->u16Used = 0;
}

/**
 *  Get the number of messages stored in the message queue.
 */
orxU16 orxMessageQueue_GetMessageNumber(orxMESSAGE_QUEUE* _pstQueue)
{
	/** Assert the queue is exists.*/
	orxASSERT(_pstQueue!=NULL);
	return _pstQueue->u16Used;
}


/**
 *  Get the number of messages places allocated for the message queue.
 */
orxU16 orxMessageQueue_GetAllocatedNumber(orxMESSAGE_QUEUE* _pstQueue)
{
	/** Assert the queue is exists.*/
	orxASSERT(_pstQueue!=NULL);
	return _pstQueue->u16Alloc;
}


/**
 *  Resize the memssage queue to allocate another number of messages.
 */
orxVOID	orxMessageQueue_Resize(orxMESSAGE_QUEUE* _pstQueue, orxU16 _u16Size)
{
	/** Assert the queue is exists.*/
	orxASSERT(_pstQueue!=NULL);
	/** Assert the number of message is superior to 0.*/
	orxASSERT(_u16Size>0);
	
	orxMESSAGE* pstMessage = (orxMESSAGE*) orxMemory_Allocate(sizeof(orxMESSAGE)*_u16Size, orxMEMORY_TYPE_MAIN);
	orxMemory_Set(pstMessage, 0, sizeof(pstMessage));
	_pstQueue->u16Alloc = _u16Size;
	
	_pstQueue->u16Used = orxMIN(_u16Size, _pstQueue->u16Used);
	orxMemory_Copy(pstMessage, _pstQueue->pastMessages, sizeof(orxMESSAGE)*_pstQueue->u16Used);
	orxMemory_Free(_pstQueue->pastMessages);
	
	_pstQueue->pastMessages = pstMessage;
}

/** 
 * Add a message to the queue.
 */
orxMESSAGE* orxMessageQueue_AddMessage(orxMESSAGE_QUEUE* _pstQueue, orxMESSAGE_IDENTIFIER _u32ID, orxVOID* _pData)
{
	/** Assert the queue is exists.*/
	orxASSERT(_pstQueue!=NULL);
	
	/** If no message, add it direcly.*/
	if(_pstQueue->u16Used==0)
	{
		orxMessage_Set(_pstQueue->pastMessages, _u32ID, _pData);
		_pstQueue->u16Used = 1;
		return _pstQueue->pastMessages;
	}
	else
	{
		orxMESSAGE* pCurrentMessage = orxMessageQueue_GetLastMessage(_pstQueue);
		orxMESSAGE* pFirstMessage = orxMessageQueue_GetFirstMessage(_pstQueue);

		/** If queue is full, lost the last message.*/
		if(_pstQueue->u16Used==_pstQueue->u16Alloc)
			pCurrentMessage	= orxMessageQueue_GetPreviousMessage(pCurrentMessage);
	
		/** Find the insert position and move all messages after.*/
		while((pCurrentMessage>=pFirstMessage) && (pCurrentMessage->u32Identifier>_u32ID))
		{
			orxMessage_Copy(pCurrentMessage, orxMessageQueue_GetNextMessage(pCurrentMessage));
			pCurrentMessage = orxMessageQueue_GetPreviousMessage(pCurrentMessage);
		}
		/** Shift position.*/
		pCurrentMessage	= orxMessageQueue_GetNextMessage(pCurrentMessage);
		/** Set the inserted message.*/
		orxMessage_Set(pCurrentMessage, _u32ID, _pData);

		if(_pstQueue->u16Used<_pstQueue->u16Alloc)
			_pstQueue->u16Used++;
		return pCurrentMessage;
	}	
}

/**
 *  Remove a message from a message queue.
 */
void orxMessageQueue_RemoveMessage(orxMESSAGE_QUEUE* _pstQueue, orxMESSAGE* _pstMessage)
{
	/** Assert the queue is exists.*/
	orxASSERT(_pstQueue!=NULL);
	/** Assert the message to remove is in message queue array bounds.*/
	orxASSERT((_pstMessage>=_pstQueue->pastMessages)&&(_pstMessage<_pstQueue->pastMessages+(_pstQueue->u16Used*sizeof(orxMESSAGE))));
	
	orxMESSAGE* pLastMessage = orxMessageQueue_GetLastMessage(_pstQueue);
	while(_pstMessage < pLastMessage)
	{
		orxMessage_Copy(orxMessageQueue_GetNextMessage(_pstMessage), _pstMessage);
		_pstMessage = orxMessageQueue_GetNextMessage(_pstMessage);
	}
	orxMessage_Clear(pLastMessage);
	_pstQueue->u16Used--;
}

/**
 *  Find a message in the queue.
 */
orxMESSAGE* orxMessageQueue_FindMessage(orxMESSAGE_QUEUE* _pstQueue, orxMESSAGE_IDENTIFIER _u32ID)
{
	/** Assert the queue is exists.*/
	orxASSERT(_pstQueue!=NULL);

	orxMESSAGE* pCurrentMessage = orxMessageQueue_GetFirstMessage(_pstQueue);
	if(pCurrentMessage==orxNULL)
		return orxNULL;
	orxMESSAGE* pLastMessage = orxMessageQueue_GetLastMessage(_pstQueue);
	
	while(pCurrentMessage<=pLastMessage)
	{
		if(pCurrentMessage->u32Identifier==_u32ID)
			return pCurrentMessage;
		pCurrentMessage = orxMessageQueue_GetNextMessage(pCurrentMessage);
	}
	return orxNULL;
}


/**
 *  Return the first message of a message queue.
 */
orxMESSAGE* orxMessageQueue_GetFirstMessage(orxMESSAGE_QUEUE* _pstQueue)
{
	/** Assert the queue is exists.*/
	orxASSERT(_pstQueue!=NULL);
	if(_pstQueue->u16Used==0)
		return orxNULL;
	else
		return _pstQueue->pastMessages;
}

/**
 *  Return the last message of a message queue.
 */
orxMESSAGE* orxMessageQueue_GetLastMessage(orxMESSAGE_QUEUE* _pstQueue)
{
	/** Assert the queue is exists.*/
	orxASSERT(_pstQueue!=NULL);
	if(_pstQueue->u16Used==0)
		return orxNULL;
	else
		return _pstQueue->pastMessages + ((_pstQueue->u16Used-1)*sizeof(orxMESSAGE));
}

/**
 *  Return the next message in a message queue.
 */
orxMESSAGE* orxMessageQueue_GetNextMessage(orxMESSAGE* _pstMessage)
{
	return _pstMessage + sizeof(orxMESSAGE);
}

/**
 *  Return the previous message in a message queue.
 */
orxMESSAGE* orxMessageQueue_GetPreviousMessage(orxMESSAGE* _pstMessage)
{
	return _pstMessage - sizeof(orxMESSAGE);
}

