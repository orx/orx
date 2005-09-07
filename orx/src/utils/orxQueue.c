/**
 * @file utils/orxQueue.c
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


#include "utils/orxQueue.h"
#include "memory/orxMemory.h"
#include "math/orxMath.h"
#include "debug/orxDebug.h"

/*
 * Queue module status flag definition.
 */
#define orxQUEUE_KU32_FLAG_NONE              0x00000000
#define orxQUEUE_KU32_FLAG_READY             0x00000001

/*
 * Queue module state structure.
 */
typedef struct __orxQUEUE_STATIC_t
{
  /* Control flags */
  orxU32 u32Flags;
} orxQUEUE_STATIC;


/*
 * Static data
 */
orxSTATIC orxQUEUE_STATIC sstQueue;



/** Queues item association structure. */
struct __orxQUEUE_ITEM_t
{
	/** Queued item identifier.*/
	orxU32 u32ID;
	/** Queued item extra data.*/
	orxVOID* pExtraData;
};


/** Queue structure. */
struct __orxQUEUE_t
{
	/** Number of allocated item.*/
	orxU16	u16Alloc;
	/** Number of used item.*/
	orxU16 u16Used;
	/** Address of item array.*/
	orxQUEUE_ITEM* pastItems;
};


/***************************************************************************
 * Private functions                                                        *
 ***************************************************************************/

/**
 *  Clear a item content.
 * @param _pstItem Item to clear.
 */
orxSTATIC orxINLINE orxVOID orxQueueItem_Clear(orxQUEUE_ITEM* _pstItem)
{
  /* Module initialized ? */
  orxASSERT((sstQueue.u32Flags & orxQUEUE_KU32_FLAG_READY) == orxQUEUE_KU32_FLAG_READY);

  /* Correct parameters ? */
  orxASSERT(_pstItem != orxNULL);

	_pstItem->pExtraData = orxNULL;
	_pstItem->u32ID      = 0x00000000;
}

/**
 * Set an item content.
 * @param _pstItem Item to set.
 * @param _u32ID Identifier.
 * @param _pData Extra data.
 */
orxSTATIC orxINLINE orxVOID orxQueueItem_Set(orxQUEUE_ITEM* _pstItem, orxU32 _u32ID, orxVOID* _pData)
{
  /* Module initialized ? */
  orxASSERT((sstQueue.u32Flags & orxQUEUE_KU32_FLAG_READY) == orxQUEUE_KU32_FLAG_READY);

  /* Correct parameters ? */
  orxASSERT(_pstItem != orxNULL);

	_pstItem->u32ID      = _u32ID;
	_pstItem->pExtraData = _pData;
}

/**
 * Copy the item content to another.
 * @param _pstSourceItem Source item.
 * @param _pstTargetItem Target item.
 */
orxSTATIC orxINLINE orxVOID orxQueueItem_Copy(orxQUEUE_ITEM* _pstSourceItem, orxQUEUE_ITEM* _pstTargetItem)
{
  /* Module initialized ? */
  orxASSERT((sstQueue.u32Flags & orxQUEUE_KU32_FLAG_READY) == orxQUEUE_KU32_FLAG_READY);

  /* Correct parameters ? */
  orxASSERT(_pstSourceItem != orxNULL);
  orxASSERT(_pstTargetItem != orxNULL);

	orxMemory_Copy(_pstTargetItem, _pstSourceItem, sizeof(orxQUEUE_ITEM));
}


/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/
 
/** 
 * orxQueue_Init
 **/
orxSTATUS orxQueue_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILED;
  
  /* Init dependencies */
  if ((orxDEPEND_INIT(Depend) &
       orxDEPEND_INIT(Memory)) == orxSTATUS_SUCCESS)
  {
    /* Not already initialized ? */
    if(!(sstQueue.u32Flags & orxQUEUE_KU32_FLAG_READY))
    {
      /* Cleans control structure */
      orxMemory_Set(&sstQueue, 0, sizeof(orxQUEUE_STATIC));
    
      /* Inits Flags */
      sstQueue.u32Flags = orxQUEUE_KU32_FLAG_READY;
      
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
 * orxQueue_Exit
 **/
orxVOID orxQueue_Exit()
{
  /* Initialized? */
  if(sstQueue.u32Flags & orxQUEUE_KU32_FLAG_READY)
  {
    /* Updates flags */
    sstQueue.u32Flags &= ~orxQUEUE_KU32_FLAG_READY;
  }
  else
  {
    /* !!! MSG !!! */
  }

  /* Exit dependencies */
  orxDEPEND_EXIT(Memory);
  orxDEPEND_EXIT(Depend);

  return;
}


/**
 *  Return an item identifier.
 */
orxU32 orxQueueItem_GetID(orxQUEUE_ITEM* _pstItem)
{
    /* Module initialized ? */
    orxASSERT((sstQueue.u32Flags & orxQUEUE_KU32_FLAG_READY) == orxQUEUE_KU32_FLAG_READY);

    /* Correct parameters ? */
    orxASSERT(_pstItem != orxNULL);

	return _pstItem->u32ID;
}


/**
 *  Return an item extra data adress.
 */
orxVOID* orxQueueItem_GetExtraData(orxQUEUE_ITEM* _pstItem)
{
    /* Module initialized ? */
    orxASSERT((sstQueue.u32Flags & orxQUEUE_KU32_FLAG_READY) == orxQUEUE_KU32_FLAG_READY);

    /* Correct parameters ? */
    orxASSERT(_pstItem != orxNULL);

	return _pstItem->pExtraData;
}

/**
 *  Modify a queued item identifier.
 */
orxVOID orxQueueItem_SetID(orxQUEUE_ITEM* _pstItem, orxU32 _u32ID)
{
   /* Module initialized ? */
    orxASSERT((sstQueue.u32Flags & orxQUEUE_KU32_FLAG_READY) == orxQUEUE_KU32_FLAG_READY);

    /* Correct parameters ? */
    orxASSERT(_pstItem != orxNULL);
    
    _pstItem->u32ID = _u32ID;
}

/**
 *  Modify an item extra data adress.
 */
orxVOID orxQueueItem_SetExtraData(orxQUEUE_ITEM* _pstItem, orxVOID* _pData)
{
   /* Module initialized ? */
    orxASSERT((sstQueue.u32Flags & orxQUEUE_KU32_FLAG_READY) == orxQUEUE_KU32_FLAG_READY);

    /* Correct parameters ? */
    orxASSERT(_pstItem != orxNULL);

	_pstItem->pExtraData = _pData;
}


/**
 * Create a queue.
 */
orxQUEUE *orxQueue_Create(orxU16 _u16Number)
{
  /* Module initialized ? */
  orxASSERT((sstQueue.u32Flags & orxQUEUE_KU32_FLAG_READY) == orxQUEUE_KU32_FLAG_READY);

	/** Assert the number of items is superior to 0.*/
	orxASSERT(_u16Number>0);
	
	orxQUEUE* pstQueue = (orxQUEUE*) orxMemory_Allocate(sizeof(orxQUEUE), orxMEMORY_TYPE_MAIN);
	pstQueue->pastItems = (orxQUEUE_ITEM*) orxMemory_Allocate(sizeof(orxQUEUE_ITEM)*_u16Number, orxMEMORY_TYPE_MAIN);
	pstQueue->u16Alloc = _u16Number;
	pstQueue->u16Used = 0;
	orxQueue_Clean(pstQueue);
	return pstQueue;
}

/**
 *  Delete a queue.
 */
orxVOID orxQueue_Delete(orxQUEUE* _pstQueue)
{
  /* Module initialized ? */
  orxASSERT((sstQueue.u32Flags & orxQUEUE_KU32_FLAG_READY) == orxQUEUE_KU32_FLAG_READY);

	/** Assert the queue is exists.*/
	orxASSERT(_pstQueue != orxNULL);

	if(_pstQueue->pastItems!=orxNULL)
    {
		orxMemory_Free(_pstQueue->pastItems);
    }
  
	orxMemory_Free(_pstQueue);
}


/**
 * Clean a queue.
 */
orxVOID	orxQueue_Clean(orxQUEUE* _pstQueue)
{
  /* Module initialized ? */
  orxASSERT((sstQueue.u32Flags & orxQUEUE_KU32_FLAG_READY) == orxQUEUE_KU32_FLAG_READY);

	/** Assert the queue is exists.*/
	orxASSERT(_pstQueue != orxNULL);

	orxMemory_Set(_pstQueue->pastItems, 0, sizeof(_pstQueue->pastItems));
	_pstQueue->u16Used = 0;
}

/**
 *  Get the number of items stored in the queue.
 */
orxU16 orxQueue_GetItemNumber(orxQUEUE* _pstQueue)
{
    /* Module initialized ? */
    orxASSERT((sstQueue.u32Flags & orxQUEUE_KU32_FLAG_READY) == orxQUEUE_KU32_FLAG_READY);

	/** Assert the queue is exists.*/
	orxASSERT(_pstQueue != orxNULL);
	return _pstQueue->u16Used;
}


/**
 *  Get the number of item places allocated for the queue.
 */
orxU16 orxQueue_GetAllocatedNumber(orxQUEUE* _pstQueue)
{
    /* Module initialized ? */
    orxASSERT((sstQueue.u32Flags & orxQUEUE_KU32_FLAG_READY) == orxQUEUE_KU32_FLAG_READY);

	/** Assert the queue is exists.*/
	orxASSERT(_pstQueue != orxNULL);
	return _pstQueue->u16Alloc;
}


/**
 *  Resize the queue to allocate another number of items.
 */
orxVOID	orxQueue_Resize(orxQUEUE* _pstQueue, orxU16 _u16Size)
{
    /* Module initialized ? */
    orxASSERT((sstQueue.u32Flags & orxQUEUE_KU32_FLAG_READY) == orxQUEUE_KU32_FLAG_READY);

	/** Assert the queue is exists.*/
	orxASSERT(_pstQueue != orxNULL);
  
	/** Assert the number of items is superior to 0.*/
	orxASSERT(_u16Size>0);
	
	orxQUEUE_ITEM* pstItem = (orxQUEUE_ITEM*) orxMemory_Allocate(sizeof(orxQUEUE_ITEM)*_u16Size, orxMEMORY_TYPE_MAIN);
	orxMemory_Set(pstItem, 0, sizeof(pstItem));
	_pstQueue->u16Alloc = _u16Size;
	
	_pstQueue->u16Used = orxMIN(_u16Size, _pstQueue->u16Used);
	orxMemory_Copy(pstItem, _pstQueue->pastItems, sizeof(orxQUEUE_ITEM)*_pstQueue->u16Used);
	orxMemory_Free(_pstQueue->pastItems);
	
	_pstQueue->pastItems = pstItem;
}

/** 
 * Add an item to the queue.
 */
orxQUEUE_ITEM* orxQueue_AddItem(orxQUEUE* _pstQueue, orxU32 _u32ID, orxVOID* _pData)
{
    /* Module initialized ? */
    orxASSERT((sstQueue.u32Flags & orxQUEUE_KU32_FLAG_READY) == orxQUEUE_KU32_FLAG_READY);

    /** Assert the queue is exists.*/
	orxASSERT(_pstQueue != orxNULL);
	
	/** If no item, add it direcly.*/
	if(_pstQueue->u16Used==0)
	{
		orxQueueItem_Set(_pstQueue->pastItems, _u32ID, _pData);
		_pstQueue->u16Used = 1;
		return _pstQueue->pastItems;
	}
	else
	{
		orxQUEUE_ITEM* pCurrentItem = orxQueue_GetLastItem(_pstQueue);
		orxQUEUE_ITEM* pFirstItem = orxQueue_GetFirstItem(_pstQueue);

		/** If queue is full, lost the last item.*/
		if(_pstQueue->u16Used==_pstQueue->u16Alloc)
			pCurrentItem	= orxQueue_GetPreviousItem(_pstQueue, pCurrentItem);
	
		/** Find the insert position and move all Item after.*/
		while((pCurrentItem>=pFirstItem) && (pCurrentItem->u32ID>_u32ID))
		{
			orxQueueItem_Copy(pCurrentItem, orxQueue_GetNextItem(_pstQueue, pCurrentItem));
			pCurrentItem = orxQueue_GetPreviousItem(_pstQueue, pCurrentItem);
		}
		/** Shift position.*/
		pCurrentItem	= orxQueue_GetNextItem(_pstQueue, pCurrentItem);
		/** Set the inserted item.*/
		orxQueueItem_Set(pCurrentItem, _u32ID, _pData);

		if(_pstQueue->u16Used<_pstQueue->u16Alloc)
        {
			_pstQueue->u16Used++;
        }
		return pCurrentItem;
	}	
}

/**
 *  Remove an item from a queue.
 */
void orxQueue_RemoveItem(orxQUEUE* _pstQueue, orxQUEUE_ITEM* _pstItem)
{
    /* Module initialized ? */
    orxASSERT((sstQueue.u32Flags & orxQUEUE_KU32_FLAG_READY) == orxQUEUE_KU32_FLAG_READY);

	/** Assert the queue is exists.*/
	orxASSERT(_pstQueue != orxNULL);
	/** Assert the item to remove is in queue array bounds.*/
	orxASSERT((_pstItem>=_pstQueue->pastItems)&&(_pstItem<_pstQueue->pastItems+(_pstQueue->u16Used*sizeof(orxQUEUE_ITEM))));
	
	orxQUEUE_ITEM* pLastItem = orxQueue_GetLastItem(_pstQueue);
	while(_pstItem < pLastItem)
	{
		orxQueueItem_Copy(orxQueue_GetNextItem(_pstQueue, _pstItem), _pstItem);
		_pstItem = orxQueue_GetNextItem(_pstQueue, _pstItem);
	}
	orxQueueItem_Clear(pLastItem);
	_pstQueue->u16Used--;
}

/**
 *  Find an item in the queue.
 */
orxQUEUE_ITEM* orxQueue_FindItem(orxQUEUE* _pstQueue, orxU32 _u32ID)
{
    /* Module initialized ? */
    orxASSERT((sstQueue.u32Flags & orxQUEUE_KU32_FLAG_READY) == orxQUEUE_KU32_FLAG_READY);

	/** Assert the queue is exists.*/
	orxASSERT(_pstQueue != orxNULL);

	orxQUEUE_ITEM* pCurrentItem = orxQueue_GetFirstItem(_pstQueue);
	if(pCurrentItem==orxNULL)
    {
        return orxNULL;
    }
	orxQUEUE_ITEM* pLastItem = orxQueue_GetLastItem(_pstQueue);
	
	while(pCurrentItem<=pLastItem)
	{
		if(pCurrentItem->u32ID==_u32ID)
    {
			return pCurrentItem;
    }
		pCurrentItem = orxQueue_GetNextItem(_pstQueue, pCurrentItem);
	}
	return orxNULL;
}


/**
 *  Return the first item of a queue.
 */
orxQUEUE_ITEM* orxQueue_GetFirstItem(orxQUEUE* _pstQueue)
{
    /* Module initialized ? */
    orxASSERT((sstQueue.u32Flags & orxQUEUE_KU32_FLAG_READY) == orxQUEUE_KU32_FLAG_READY);

	/** Assert the queue is exists.*/
	orxASSERT(_pstQueue != orxNULL);
	if(_pstQueue->u16Used==0)
		return orxNULL;
	else
		return _pstQueue->pastItems;
}

/**
 *  Return the last item of a queue.
 */
orxQUEUE_ITEM* orxQueue_GetLastItem(orxQUEUE* _pstQueue)
{
    /* Module initialized ? */
    orxASSERT((sstQueue.u32Flags & orxQUEUE_KU32_FLAG_READY) == orxQUEUE_KU32_FLAG_READY);

	/** Assert the queue is exists.*/
	orxASSERT(_pstQueue != orxNULL);
	if(_pstQueue->u16Used==0)
		return orxNULL;
	else
		return _pstQueue->pastItems + ((_pstQueue->u16Used-1)*sizeof(orxQUEUE_ITEM));}

/**
 *  Return the next item in a queue.
 */
orxQUEUE_ITEM* orxQueue_GetNextItem(orxQUEUE* _pstQueue, orxQUEUE_ITEM* _pstItem)
{
  /* Module initialized ? */
  orxASSERT((sstQueue.u32Flags & orxQUEUE_KU32_FLAG_READY) == orxQUEUE_KU32_FLAG_READY);

	/* Correct parameters ? */
  orxASSERT(_pstItem != orxNULL);
  orxASSERT(_pstQueue != orxNULL);

	/** Correct current item bound.*/
	orxASSERT(_pstItem >= _pstQueue->pastItems);
	orxASSERT(_pstItem <= (_pstQueue->pastItems + (_pstQueue->u16Used * sizeof(orxQUEUE_ITEM))));

	if(_pstItem == (_pstQueue->pastItems + (_pstQueue->u16Used * sizeof(orxQUEUE_ITEM))))
  {
    return orxNULL;
  }
  else
  {
    return(_pstItem + sizeof(orxQUEUE_ITEM));
  }
}

/**
 *  Return the previous item in a queue.
 */
orxQUEUE_ITEM* orxQueue_GetPreviousItem(orxQUEUE* _pstQueue, orxQUEUE_ITEM* _pstItem)
{
  /* Module initialized ? */
  orxASSERT((sstQueue.u32Flags & orxQUEUE_KU32_FLAG_READY) == orxQUEUE_KU32_FLAG_READY);

  /* Correct parameters ? */
  orxASSERT(_pstItem != orxNULL);
  orxASSERT(_pstQueue != orxNULL);

	/** Correct current item bound.*/
	orxASSERT(_pstItem >= _pstQueue->pastItems);
	orxASSERT(_pstItem <= (_pstQueue->pastItems + (_pstQueue->u16Used * sizeof(orxQUEUE_ITEM))));

	if(_pstItem == _pstQueue->pastItems)
  {
		return orxNULL;
  }
	else
  {
		return(_pstItem - sizeof(orxQUEUE_ITEM));
  }
}

