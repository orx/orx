/**
 * @file utils/orxQueue.h
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
 * @addtogroup Queue
 * @{
 */

#ifndef _orxQUEUE_H_
#define _orxQUEUE_H_

#include "orxInclude.h"


/** Item queued. */
typedef struct __orxQUEUE_ITEM_t		orxQUEUE_ITEM;

/** Return a queued item identifier.
 * @param _pstItem Adress of the item.
 * @return Identifier of the item.
 */
extern orxU32 orxQueueItem_GetID(orxQUEUE_ITEM* _pstItem);

/** Return an item extra data adress.
 * @param _pstItem Adress of the Item.
 * @return Extra data adress of the item.
 */
extern orxVOID* orxQueueItem_GetExtraData(orxQUEUE_ITEM* _pstItem);


/** Queue structure. */
typedef struct __orxQUEUE_t		orxQUEUE;


/** Create a queue.
 * @param _u16Number Number of item the queue can store.
 * @return Address of the queue structure.
 */
extern orxQUEUE *orxQueue_Create(orxU16 _u16Number);

/** Delete a queue.
 * @warning All the content will be lost.
 * @param _pstQueue Queue to delete.
 */
extern orxVOID orxQueue_Delete(orxQUEUE* _pstQueue);

/** Clean a queue.
 * @param _pstQueue Address of queue to clean.
 */
extern orxVOID	orxQueue_Clean(orxQUEUE* _pstQueue);

/** Get the number of item stored in the queue.
 * @param _pstQueue Address of queue.
 * @return Number of items currently stored.
 */
extern orxU16 orxQueue_GetItemNumber(orxQUEUE* _pstQueue);

/** Get the number of item places allocated for the queue.
 * @param _pstQueue Address of queue.
 * @return Number of item places allocated.
 */
extern orxU16 orxQueue_GetAllocatedNumber(orxQUEUE* _pstQueue);

/** Resize the queue to allocate another number of items.
 * @warning If used item number is greater than wanted allocation, extra items will be deleted.
 * @param _pstQueue Address of the queue.
 * @param _u16Size New size in number of items.
 */
extern orxVOID	orxQueue_Resize(orxQUEUE* _pstQueue, orxU16 _u16Size);

/** Add an item to the queue.
 * If the item queue is full, the last item will be lost.
 * @param _pstQueue Queue.
 * @param _u32ID Identifier of the item.
 * @param _pData	Adress of extra data.
 * @return Address of the item.
 */
extern orxQUEUE_ITEM* orxQueue_AddItem(orxQUEUE* _pstQueue, orxU32 _u32ID, orxVOID* _pData);

/** Remove an item from a queue.
 * @param _pstQueue Queue.
 * @param _pstItem Item to remove.
 */
extern void orxQueue_RemoveItem(orxQUEUE* _pstQueue, orxQUEUE_ITEM* _pstItem);

/** Find a message in the queue.
 * @param _pstQueue Queue.
 * @param _u32ID Identifier of the Item.
 * @return The first item or orxNULL if not found.
 */
extern orxQUEUE_ITEM* orxQueue_FindItem(orxQUEUE* _pstQueue, orxU32 _u32ID);

/** Return the first item of a queue.
 * @param _pstQueue Queue.
 * @return Address of the first Item or orxNULL if no item in the queue.
 */
extern orxQUEUE_ITEM* orxQueue_GetFirstItem(orxQUEUE* _pstQueue);

/** Return the last item of a queue.
 * @param _pstQueue Queue.
 * @return Address of the last item or orxNULL if no item in the queue.
 */
extern orxQUEUE_ITEM* orxQueue_GetLastItem(orxQUEUE* _pstQueue);

/** Return the next item in a queue.
 * @param _pstQueue Queue.
 * @param _pstItem Item to search the nextone.
 * @return Address of next Item, orxNULL if not found.
 */
extern orxQUEUE_ITEM* orxQueue_GetNextItem(orxQUEUE* _pstQueue, orxQUEUE_ITEM* _pstItem);

/** Return the previous item in a queue.
 * @param _pstQueue Queue.
 * @param _pstItem Item to search the previousone.
 * @return Address of previous Item or orxNULL if not found.
 */
extern orxQUEUE_ITEM* orxQueue_GetPreviousItem(orxQUEUE* _pstQueue, orxQUEUE_ITEM* _pstItem);

 
/** Initialize Queue Module
 */
extern orxSTATUS orxDLLAPI orxQueue_Init();

/** Exit Queue module
 */
extern orxVOID orxDLLAPI orxQueue_Exit();

#endif /*_orxQUEUE_H_*/
/* @} */
