/**
 * @file orxHashTable.c
 * 
 * Memory Hash table manipulation module
 * 
 */
 
 /***************************************************************************
 orxHashTable.c
 Hash table manipulation module
 
 begin                : 05/05/2005
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

#include "utils/orxHashTable.h"
#include "memory/orxBank.h"
#include "debug/orxDebug.h"
#include "io/orxTextIO.h"

#define orxHASHTABLE_KU32_FLAG_NONE   0x00000000  /**< No flags have been set */
#define orxHASHTABLE_KU32_FLAG_READY  0x00000001  /**< The module has been initialized */

#define orxHASHTABLE_KU32_INDEX_SIZE  256

/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Hash table cell definition.*/
typedef struct __orxHASHTABLE_CELL_t
{
  orxU32   u32Key;                        /**< Key element of a hash table. */
  orxVOID  *pData;                        /**< Address of data. */
  struct __orxHASHTABLE_CELL_t *pstNext;  /**< Next cell with the same index. */
} orxHASHTABLE_CELL;

/** Hash Table */
struct __orxHASHTABLE_t
{
  orxHASHTABLE_CELL *apstCell[orxHASHTABLE_KU32_INDEX_SIZE]; /**< Hash table */
  orxBANK *pstBank;                                          /**< Bank where are stored cells */
};

/** Module static structure */
typedef struct __orxHASHTABLE_STATIC_t
{
  orxU32 u32Flags;      /**< Module flags */
} orxHASHTABLE_STATIC;

/***************************************************************************
 * Module global variable                                                  *
 ***************************************************************************/
orxSTATIC orxHASHTABLE_STATIC sstHashTable;

/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

/** Find a row in a hash table.
 * @param _pstHashTable The hash table where search.
 * @param _u32Key Key to find.
 * @return index associated to the given key.
 */
orxSTATIC orxINLINE orxU32 orxHashTable_FindIndex(orxCONST orxHASHTABLE *_pstHashTable, orxU32 _u32Key)
{
  /* Module initialized ? */
  orxASSERT((sstHashTable.u32Flags & orxHASHTABLE_KU32_FLAG_READY) == orxHASHTABLE_KU32_FLAG_READY);

  /* Correct parameters ? */
  orxASSERT(_pstHashTable != orxNULL);
  
  /* Compute the hash index */
  return (_u32Key %  orxHASHTABLE_KU32_INDEX_SIZE);
}

/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/***************************************************************************
 orxHashTable_Setup
 HashTable module setup.

 returns: nothing
 ***************************************************************************/
orxVOID orxHashTable_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_HASHTABLE, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_HASHTABLE, orxMODULE_ID_BANK);

  return;
}

/** @name Module management.
 * @{ */
/** Initialize HashTable Module
 * @return Returns the initialization status.
 */
orxSTATUS orxHashTable_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILED;

  /* Not already Initialized? */
  if(!(sstHashTable.u32Flags & orxHASHTABLE_KU32_FLAG_READY))
  {
    /* Module not already initialized ? */
    orxASSERT(!(sstHashTable.u32Flags & orxHASHTABLE_KU32_FLAG_READY));
  
    /* Cleans static controller */
    orxMemory_Set(&sstHashTable, 0, sizeof(orxHASHTABLE_STATIC));
  
    /* Set module as ready */
    sstHashTable.u32Flags = orxHASHTABLE_KU32_FLAG_READY;
    
    /* Success */
    eResult = orxSTATUS_SUCCESS;
  }
  else
  {
    /* !!! MSG !!! */

    /* Already initialized */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Successfull initialization */
  return eResult;
}

/** Exit HashTable module
 */
orxVOID orxHashTable_Exit()
{
  /* Module initialized ? */
  if ((sstHashTable.u32Flags & orxHASHTABLE_KU32_FLAG_READY) == orxHASHTABLE_KU32_FLAG_READY)
  {
    /* Module not ready now */
    sstHashTable.u32Flags = orxHASHTABLE_KU32_FLAG_NONE;
  }
  
  return;
}

/** @name HashTable creation/destruction.
 * @{ */
/** Create a new hash table and return it.
 * @param _u32NbKey   (IN) Number of keys that will be inserted.
 * @param _u32Flags   (IN) Flags used by the hash table
 * @param _eMemType   (IN) Memory type to use
 * @return Returns the hashtable pointer or orxNULL if failed.
 */
orxHASHTABLE *orxFASTCALL orxHashTable_Create(orxU32 _u32NbKey, orxU32 _u32Flags, orxMEMORY_TYPE _eMemType)
{
  orxHASHTABLE *pstHashTable = orxNULL; /* New created hash table */
  orxU32 u32Flags;                  /* Flags used for bank creation */
    
  /* Module initialized ? */
  orxASSERT((sstHashTable.u32Flags & orxHASHTABLE_KU32_FLAG_READY) == orxHASHTABLE_KU32_FLAG_READY);

  /* Correct parameters ? */
  orxASSERT(_eMemType < orxMEMORY_TYPE_NUMBER);
  
  /* (hash can hold at least one key) */
  orxASSERT(_u32NbKey > 0);

  /* Allocate memory for a hash table */
  pstHashTable = (orxHASHTABLE *)orxMemory_Allocate(sizeof(orxHASHTABLE), _eMemType);
  
  /* Enough memory ? */
  if (pstHashTable != orxNULL)
  {
    /* Set flags */
    if (_u32Flags == orxHASHTABLE_KU32_FLAGS_NOT_EXPANDABLE)
    {
      u32Flags = orxBANK_KU32_FLAGS_NOT_EXPANDABLE;
    }
    else
    {
      u32Flags = orxBANK_KU32_FLAGS_NONE;
    }
    
    /* Clean values */
    orxMemory_Set(pstHashTable, 0, sizeof(orxHASHTABLE));
  
    /* Allocate bank for cells */
    pstHashTable->pstBank = orxBank_Create(_u32NbKey, sizeof(orxHASHTABLE_CELL), u32Flags, _eMemType);
    
    /* Correct bank allocation ? */
    if (pstHashTable->pstBank == orxNULL)
    {
      /* Allocation problem, returns orxNULL */
      orxMemory_Free(pstHashTable);
      pstHashTable = orxNULL;
    }
  }
  
  return pstHashTable;
}

/** Delete a hash table.
 * @param _pstHashTable (IN) Hash table to delete.
 */
orxVOID orxFASTCALL orxHashTable_Delete(orxHASHTABLE *_pstHashTable)
{
  /* Module initialized ? */
  orxASSERT((sstHashTable.u32Flags & orxHASHTABLE_KU32_FLAG_READY) == orxHASHTABLE_KU32_FLAG_READY);

  /* Correct parameters ? */
  orxASSERT(_pstHashTable != orxNULL);
  
  /* Clear hash table (unallocate cells) */
  orxHashTable_Clear(_pstHashTable);
  
  /* Free bank */
  orxBank_Delete(_pstHashTable->pstBank);
  
  /* Unallocate memory */
  orxMemory_Free(_pstHashTable);
  
  /* Done ! */
}

/** Clear a hash table.
 * @param _pstHashTable (IN) Hash table to clear.
 */
orxVOID orxFASTCALL orxHashTable_Clear(orxHASHTABLE *_pstHashTable)
{
  /* Module initialized ? */
  orxASSERT((sstHashTable.u32Flags & orxHASHTABLE_KU32_FLAG_READY) == orxHASHTABLE_KU32_FLAG_READY);

  /* Correct parameters ? */
  orxASSERT(_pstHashTable != orxNULL);

  /* Clear the memory bank */
  orxBank_Clear(_pstHashTable->pstBank);
  
  /* Clear the hash */
  orxMemory_Set(_pstHashTable->apstCell, 0, sizeof(_pstHashTable->apstCell));
}

/** @name HashTable key manipulation.
 * @{ */
/** Find an item in a hash table.
 * @param _pstHashTable (IN) The hash table where search.
 * @param _u32Key     (IN) Key to find.
 * @return The Element associated to the key or orxNULL if not found.
 */
orxVOID *orxFASTCALL orxHashTable_Get(orxCONST orxHASHTABLE *_pstHashTable, orxU32 _u32Key)
{
  orxU32 u32Index;                    /* Hash table index */
  orxHASHTABLE_CELL *pstCell = orxNULL; /* Cell used to traverse */
  
  /* Module initialized ? */
  orxASSERT((sstHashTable.u32Flags & orxHASHTABLE_KU32_FLAG_READY) == orxHASHTABLE_KU32_FLAG_READY);

  /* Correct parameters ? */
  orxASSERT(_pstHashTable != orxNULL);

  /* Get the index from the key */
  u32Index = orxHashTable_FindIndex(_pstHashTable, _u32Key);
  
  /* Traverse to find the key */
  pstCell = _pstHashTable->apstCell[u32Index];
  while (pstCell != orxNULL && pstCell->u32Key != _u32Key)
  {
    /* Try with next cell */
    pstCell = pstCell->pstNext;
  }
  
  /* Cell found ? */
  if (pstCell != orxNULL)
  {
    /* Returns associated datas */
    return pstCell->pData;
  }
  else
  {
    /* Key not found, returns orxNULL */
    return orxNULL;
  }
}

/** Set an item value.
 * @param _pstHashTable The hash table where set.
 * @param _u32Key     (IN) Key to assign.
 * @param _pData      (IN) Data to assign.
 */
orxVOID orxFASTCALL orxHashTable_Set(orxHASHTABLE *_pstHashTable, orxU32 _u32Key, orxVOID *_pData)
{
  orxU32 u32Index;                    /* Hash table index */
  orxHASHTABLE_CELL *pstCell = orxNULL; /* Cell used to traverse */
  
  /* Module initialized ? */
  orxASSERT((sstHashTable.u32Flags & orxHASHTABLE_KU32_FLAG_READY) == orxHASHTABLE_KU32_FLAG_READY);

  /* Correct parameters ? */
  orxASSERT(_pstHashTable != orxNULL);

  /* Get the index from the key */
  u32Index = orxHashTable_FindIndex(_pstHashTable, _u32Key);
  
  /* Traverse to find the key */
  pstCell = _pstHashTable->apstCell[u32Index];
  while (pstCell != orxNULL && pstCell->u32Key != _u32Key)
  {
    /* Try with next cell */
    pstCell = pstCell->pstNext;
  }
  
  /* Cell found ? */
  if (pstCell != orxNULL)
  {
    /* Set associated datas */
    pstCell->pData = _pData;
  }
  else
  {
    /* Allocate a new cell if possible */
    pstCell = (orxHASHTABLE_CELL *)orxBank_Allocate(_pstHashTable->pstBank);
    
    /* If allocation succeed, insert the new cell */
    if (pstCell != orxNULL)
    {
      /* Set datas */
      pstCell->u32Key = _u32Key;
      pstCell->pData  = _pData;
      pstCell->pstNext = _pstHashTable->apstCell[u32Index];
      
      /* Insert it */
      _pstHashTable->apstCell[u32Index] = pstCell;
    }
  }
}


/** Add an item value.
 * @param _pstHashTable The hash table where set.
 * @param _u32Key     (IN) Key to assign.
 * @param _pData      (IN) Data to assign.
 * @return Returns the status of the operation. (fails if key already used)
 */
orxSTATUS orxFASTCALL orxHashTable_Add(orxHASHTABLE *_pstHashTable, orxU32 _u32Key, orxVOID *_pData)
{
  orxU32 u32Index;                      /* Hash index */
  orxHASHTABLE_CELL *pstCell;             /* New cell to add */
  orxSTATUS eStatus = orxSTATUS_FAILED; /* Status to return */
  
  /* Module initialized ? */
  orxASSERT((sstHashTable.u32Flags & orxHASHTABLE_KU32_FLAG_READY) == orxHASHTABLE_KU32_FLAG_READY);

  /* Correct parameters ? */
  orxASSERT(_pstHashTable != orxNULL);
  
  /* Can't add a NULL pointer, else Get will returns orxNULL and it won't be possible to detect errors
   * Maybe that this behaviour should be different ?
   */
  orxASSERT(_pData != orxNULL);
  
  /* The key must not exist */
  if (orxHashTable_Get(_pstHashTable, _u32Key) == orxNULL)
  {
    /* Get the hash table index */
    u32Index = orxHashTable_FindIndex(_pstHashTable, _u32Key);
    
    /* Allocate a new cell if possible */
    pstCell = (orxHASHTABLE_CELL *)orxBank_Allocate(_pstHashTable->pstBank);
    
    /* If allocation succeed, insert the new cell */
    if (pstCell != orxNULL)
    {
      /* Set datas */
      pstCell->u32Key = _u32Key;
      pstCell->pData  = _pData;
      pstCell->pstNext = _pstHashTable->apstCell[u32Index];
      
      /* Insert it */
      _pstHashTable->apstCell[u32Index] = pstCell;
      eStatus = orxSTATUS_SUCCESS;
    }
  }

  return eStatus;
}

/** Remove an item.
 * @param _pstHashTable (IN) The hash table where remove.
 * @param _u32Key     (IN) Key to remove.
 * @return Returns the status of the operation.
 */
orxSTATUS orxFASTCALL orxHashTable_Remove(orxHASHTABLE *_pstHashTable, orxU32 _u32Key)
{
  orxU32 u32Index;                          /* Hash table index */
  orxHASHTABLE_CELL *pstCell = orxNULL;       /* Cell used to traverse */
  orxHASHTABLE_CELL *pstRemoveCell = orxNULL; /* Cell to remove */
  orxSTATUS eStatus = orxSTATUS_FAILED; /* Status to return */
  
  /* Module initialized ? */
  orxASSERT((sstHashTable.u32Flags & orxHASHTABLE_KU32_FLAG_READY) == orxHASHTABLE_KU32_FLAG_READY);

  /* Correct parameters ? */
  orxASSERT(_pstHashTable != orxNULL);

  /* Get the index from the key */
  u32Index = orxHashTable_FindIndex(_pstHashTable, _u32Key);
  
  pstCell = _pstHashTable->apstCell[u32Index];

  /* Is the first key is the key to remove ? */
  if (pstCell != orxNULL)
  {
    if (pstCell->u32Key == _u32Key)
    {
      /* The first cell has to be removed */
      _pstHashTable->apstCell[u32Index] = pstCell->pstNext;
      orxBank_Free(_pstHashTable->pstBank, pstCell);
      
      /* Operation succeed */
      eStatus = orxSTATUS_SUCCESS;
    }
    else
    {
      /* Traverse to find the key */
      while (pstCell->pstNext != orxNULL && pstCell->pstNext->u32Key != _u32Key)
      {
        /* Try with next cell */
        pstCell = pstCell->pstNext;
      }
      
      /* Cell found ? (key should be on the next cell) */
      if (pstCell->pstNext != orxNULL)
      {
        /* We found it, remove this cell */
        pstRemoveCell = pstCell->pstNext;
        pstCell->pstNext = pstRemoveCell->pstNext;
        
        /* Free cell from bank */
        orxBank_Free(_pstHashTable->pstBank, pstRemoveCell);
        
        /* Operation succeed */
        eStatus = orxSTATUS_SUCCESS;
      }
    }
  }
  
  return eStatus;
}

/*******************************************************************************
 * Search functions
 ******************************************************************************/

// Find a the first item of the hashtable and return the iterator corresponding to the search.
orxVOID *orxFASTCALL orxHashMap_FindFirst(orxHASHTABLE *_pstHashTable, orxU32 *_pu32Key, orxVOID **_ppData)
{
	/* Module initialized ? */
	orxASSERT((sstHashTable.u32Flags & orxHASHTABLE_KU32_FLAG_READY) == orxHASHTABLE_KU32_FLAG_READY);
	/* Correct parameters ? */
	orxASSERT(_pstHashTable != orxNULL);

	orxU16 u16Cell = 0;
	for(u16Cell=0; u16Cell<orxHASHTABLE_KU32_INDEX_SIZE; u16Cell++)
	{
		if(_pstHashTable->apstCell[u16Cell]!=orxNULL)
		{
			if(_pu32Key!=orxNULL)
				*_pu32Key = _pstHashTable->apstCell[u16Cell]->u32Key;
			if(_ppData!=orxNULL)
				*_ppData = _pstHashTable->apstCell[u16Cell]->pData;
			return (orxVOID*)_pstHashTable->apstCell[u16Cell];
		}
	}
	return orxNULL;
}

// Find a the next item of the hashtable and return the iterator corresponding to the search.
orxVOID *orxFASTCALL orxHashMap_FindNext(orxHASHTABLE *_pstHashTable, orxVOID *_pIterator, orxU32 *_pu32Key, orxVOID **_ppData)
{
	/* Module initialized ? */
	orxASSERT((sstHashTable.u32Flags & orxHASHTABLE_KU32_FLAG_READY) == orxHASHTABLE_KU32_FLAG_READY);
	/* Correct parameters ? */
	orxASSERT(_pstHashTable != orxNULL && _pIterator != orxNULL);
	
	
	orxHASHTABLE_CELL *pIter = (orxHASHTABLE_CELL*)_pIterator;
	orxU16 u16Cell;

	if(pIter->pstNext!=NULL)
	{
		if(_pu32Key!=orxNULL)
			*_pu32Key = pIter->pstNext->u32Key;
		if(_ppData!=orxNULL)
			*_ppData = pIter->pstNext->pData;
		return (orxVOID*)pIter->pstNext;		
	}
	
	for(u16Cell=orxHashTable_FindIndex(_pstHashTable, pIter->u32Key)+1; u16Cell<orxHASHTABLE_KU32_INDEX_SIZE; u16Cell++)
	{
		if(_pstHashTable->apstCell[u16Cell]!=orxNULL)
		{
			if(_pu32Key!=orxNULL)
				*_pu32Key = _pstHashTable->apstCell[u16Cell]->u32Key;
			if(_ppData!=orxNULL)
				*_ppData = _pstHashTable->apstCell[u16Cell]->pData;
			return (orxVOID*)_pstHashTable->apstCell[u16Cell];
		}
	}
	return orxNULL;
}

/*******************************************************************************
 * DEBUG FUNCTION
 ******************************************************************************/

/** Print the content of a Hash table
 * @param _pstHashTable (IN) Hash table to display
 */
orxVOID orxFASTCALL orxHashTable_DebugPrint(orxCONST orxHASHTABLE *_pstHashTable)
{
  orxHASHTABLE_CELL *pstCell = orxNULL;
  orxU32 u32Index;
    
  /* Module initialized ? */
  orxASSERT((sstHashTable.u32Flags & orxHASHTABLE_KU32_FLAG_READY) == orxHASHTABLE_KU32_FLAG_READY);

  /* Correct parameters ? */
  orxASSERT(_pstHashTable != orxNULL);

  orxTextIO_PrintLn("\n\n\n********* HashTable (%x) *********", _pstHashTable);

  for (u32Index = 0; u32Index < orxHASHTABLE_KU32_INDEX_SIZE; u32Index++)
  {
    orxTextIO_Print("[%3d]-->", u32Index);
    pstCell = _pstHashTable->apstCell[u32Index];
    
    while (pstCell != orxNULL)
    {
      orxTextIO_Print("(%u/%d)-->", pstCell->u32Key, (orxS32)pstCell->pData);
      pstCell = pstCell->pstNext;
    }
    
    orxTextIO_PrintLn("NULL");
  }
}
