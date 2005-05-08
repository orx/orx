/**
 * @file orxBank.c
 * 
 * Memory Hash map manipulation module
 * 
 * @todo All !
 */
 
 /***************************************************************************
 orxHashMap.c
 Hash map manipulation module
 
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

#include "utils/orxHashMap.h"
#include "memory/orxBank.h"
#include "debug/orxDebug.h"
#include "utils/orxString.h"

#define orxHASHMAP_KU32_FLAG_NONE   0x00000000  /**< No flags have been set */
#define orxHASHMAP_KU32_FLAG_READY  0x00000001  /**< The module has been initialized */

#define orxHASHMAP_KU32_INDEX_SIZE  256

/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Hash map cell definition.*/
typedef struct __orxHASHMAP_CELL_t
{
  orxU32   u32Key;                      /**< Key element of a hash map. */
  orxVOID  *pData;                      /**< Address of data. */
  struct __orxHASHMAP_CELL_t *pstNext;  /**< Next cell with the same index. */
} orxHASHMAP_CELL;

/** Hash Map */
struct __orxHASHMAP_t
{
  orxHASHMAP_CELL *apstCell[orxHASHMAP_KU32_INDEX_SIZE]; /**< Hash table */
  orxBANK *pstBank;                                      /**< Bank where are stored cells */
};

/** Module static structure */
typedef struct __orxHASHMAP_STATIC_t
{
  orxU32 u32Flags;      /**< Module flags */
} orxHASHMAP_STATIC;

/***************************************************************************
 * Module global variable                                                  *
 ***************************************************************************/
orxSTATIC orxHASHMAP_STATIC sstHashMap;

/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

/** Find a row in a hash map.
 * @param _pstHashMap The hash map where search.
 * @param _u32Key Key to find.
 * @return index associated to the given key.
 */
orxINLINE orxU32 orxHashMap_FindIndex(orxHASHMAP *_pstHashMap, orxU32 _u32Key)
{
  /* Module initialized ? */
  orxASSERT((sstHashMap.u32Flags & orxHASHMAP_KU32_FLAG_READY) == orxHASHMAP_KU32_FLAG_READY);

  /* Correct parameters ? */
  orxASSERT(_pstHashMap != orxNULL);
  
  /* Compute the hash index */
  return (_u32Key %  orxHASHMAP_KU32_INDEX_SIZE);
}

/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/


/** @name Module management.
 * @{ */
/** Initialize HashMap Module
 * @return Returns the initialization status.
 */
orxSTATUS orxHashMap_Init()
{
  /* Module not already initialized ? */
  orxASSERT(!(sstHashMap.u32Flags & orxHASHMAP_KU32_FLAG_READY));

  /* Cleans static controller */
  orxMemory_Set(&sstHashMap, 0, sizeof(orxHASHMAP_STATIC));

  /* Set module as ready */
  sstHashMap.u32Flags = orxHASHMAP_KU32_FLAG_READY;
  
  /* Successfull initialization */
  return orxSTATUS_SUCCESS;
}

/** Exit HashMap module
 */
orxVOID orxHashMap_Exit()
{
  /* Module initialized ? */
  orxASSERT((sstHashMap.u32Flags & orxHASHMAP_KU32_FLAG_READY) == orxHASHMAP_KU32_FLAG_READY);
  
  /* Module not ready now */
  sstHashMap.u32Flags = orxHASHMAP_KU32_FLAG_NONE;
}

/** @name HashMap creation/destruction.
 * @{ */
/** Create a new hash map and return it.
 * @param _u32NbKey   (IN) Number of keys that will be inserted.
 * @param _u32Flags   (IN) Flags used by the hash map
 * @param _eMemType   (IN) Memory type to use
 * @return Returns the hashmap pointer or orxNULL if failed.
 */
orxHASHMAP *orxHashMap_Create(orxU32 _u32NbKey, orxU32 _u32Flags, orxMEMORY_TYPE _eMemType)
{
  orxHASHMAP *pstHashMap = orxNULL; /* New created hash map */
  orxU32 u32Flags;                  /* Flags used for bank creation */
    
  /* Module initialized ? */
  orxASSERT((sstHashMap.u32Flags & orxHASHMAP_KU32_FLAG_READY) == orxHASHMAP_KU32_FLAG_READY);

  /* Correct parameters ? */
  orxASSERT(_eMemType < orxMEMORY_TYPE_NUMBER);
  
  /* (hash can hold at least one key) */
  orxASSERT(_u32NbKey > 0);

  /* Allocate memory for a hash table */
  pstHashMap = (orxHASHMAP *)orxMemory_Allocate(sizeof(orxHASHMAP), _eMemType);
  
  /* Enough memory ? */
  if (pstHashMap != orxNULL)
  {
    /* Set flags */
    if (_u32Flags == orxHASHMAP_KU32_FLAGS_NOT_EXPANDABLE)
    {
      u32Flags = orxBANK_KU32_FLAGS_NOT_EXPANDABLE;
    }
    else
    {
      u32Flags = orxBANK_KU32_FLAGS_NONE;
    }
    
    /* Clean values */
    orxMemory_Set(pstHashMap, 0, sizeof(orxHASHMAP));
  
    /* Allocate bank for cells */
    pstHashMap->pstBank = orxBank_Create(_u32NbKey, sizeof(orxHASHMAP_CELL), u32Flags, _eMemType);
    
    /* Correct bank allocation ? */
    if (pstHashMap->pstBank == orxNULL)
    {
      /* Allocation problem, returns orxNULL */
      orxMemory_Free(pstHashMap);
      pstHashMap = orxNULL;
    }
  }
  
  return pstHashMap;
}

/** Delete a hash map.
 * @param _pstHashMap (IN) Hash map to delete.
 */
orxVOID orxHashMap_Delete(orxHASHMAP *_pstHashMap)
{
  /* Module initialized ? */
  orxASSERT((sstHashMap.u32Flags & orxHASHMAP_KU32_FLAG_READY) == orxHASHMAP_KU32_FLAG_READY);

  /* Correct parameters ? */
  orxASSERT(_pstHashMap != orxNULL);
  
  /* Clear hash table (unallocate cells) */
  orxHashMap_Clear(_pstHashMap);
  
  /* Free bank */
  orxBank_Delete(_pstHashMap->pstBank);
  
  /* Unallocate memory */
  orxMemory_Free(_pstHashMap);
  
  /* Done ! */
}

/** Clear a hash map.
 * @param _pstHashMap (IN) Hash map to clear.
 * @return Returns the status of the operation.
 */
orxSTATUS orxHashMap_Clear(orxHASHMAP *_pstHashMap)
{
  orxSTATUS eStatus = orxSTATUS_SUCCESS; /* Returned status */
  
  /* Module initialized ? */
  orxASSERT((sstHashMap.u32Flags & orxHASHMAP_KU32_FLAG_READY) == orxHASHMAP_KU32_FLAG_READY);

  /* Correct parameters ? */
  orxASSERT(_pstHashMap != orxNULL);

  /* Clear the memory bank */
  orxBank_Clear(_pstHashMap->pstBank);
  
  /* Clear the hash */
  orxMemory_Set(_pstHashMap->apstCell, 0, sizeof(_pstHashMap->apstCell));
  
  /* Done ! */
  return eStatus;
}

/** @name HashMap key manipulation.
 * @{ */
/** Find an item in a hash map.
 * @param _pstHashMap (IN) The hash map where search.
 * @param _u32Key     (IN) Key to find.
 * @return The Element associated to the key or orxNULL if not found.
 */
orxVOID *orxHashMap_Get(orxHASHMAP *_pstHashMap, orxU32 _u32Key)
{
  orxU32 u32Index;                    /* Hash map index */
  orxHASHMAP_CELL *pstCell = orxNULL; /* Cell used to traverse */
  
  /* Module initialized ? */
  orxASSERT((sstHashMap.u32Flags & orxHASHMAP_KU32_FLAG_READY) == orxHASHMAP_KU32_FLAG_READY);

  /* Correct parameters ? */
  orxASSERT(_pstHashMap != orxNULL);

  /* Get the index from the key */
  u32Index = orxHashMap_FindIndex(_pstHashMap, _u32Key);
  
  /* Traverse to find the key */
  pstCell = _pstHashMap->apstCell[u32Index];
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

/** Add an item value.
 * @param _pstHashMap The hash map where set.
 * @param _u32Key     (IN) Key to assign.
 * @param _pData      (IN) Data to assign.
 * @return Returns the status of the operation. (fails if key already used)
 */
orxSTATUS orxHashMap_Add(orxHASHMAP *_pstHashMap, orxU32 _u32Key, orxVOID *_pData)
{
  orxU32 u32Index;                      /* Hash index */
  orxHASHMAP_CELL *pstCell;             /* New cell to add */
  orxSTATUS eStatus = orxSTATUS_FAILED; /* Status to return */
  
  /* Module initialized ? */
  orxASSERT((sstHashMap.u32Flags & orxHASHMAP_KU32_FLAG_READY) == orxHASHMAP_KU32_FLAG_READY);

  /* Correct parameters ? */
  orxASSERT(_pstHashMap != orxNULL);
  
  /* Can't add a NULL pointer, else Get will returns orxNULL and it won't be possible to detect errors
   * Maybe that this behaviour should be different ?
   */
  orxASSERT(_pData != orxNULL);
  
  /* The key must not exist */
  if (orxHashMap_Get(_pstHashMap, _u32Key) == orxNULL)
  {
    /* Get the hash map index */
    u32Index = orxHashMap_FindIndex(_pstHashMap, _u32Key);
    
    /* Allocate a new cell if possible */
    pstCell = (orxHASHMAP_CELL *)orxBank_Allocate(_pstHashMap->pstBank);
    
    /* If allocation succeed, insert the new cell */
    if (pstCell != orxNULL)
    {
      /* Set datas */
      pstCell->u32Key = _u32Key;
      pstCell->pData  = _pData;
      pstCell->pstNext = _pstHashMap->apstCell[u32Index];
      
      /* Insert it */
      _pstHashMap->apstCell[u32Index] = pstCell;
      eStatus = orxSTATUS_SUCCESS;
    }
  }

  return eStatus;
}

/** Remove an item.
 * @param _pstHashMap (IN) The hash map where remove.
 * @param _u32Key     (IN) Key to remove.
 * @return Returns the status of the operation.
 */
orxSTATUS orxHashMap_Remove(orxHASHMAP *_pstHashMap, orxU32 _u32Key)
{
  orxU32 u32Index;                          /* Hash map index */
  orxHASHMAP_CELL *pstCell = orxNULL;       /* Cell used to traverse */
  orxHASHMAP_CELL *pstRemoveCell = orxNULL; /* Cell to remove */
  orxSTATUS eStatus = orxSTATUS_FAILED; /* Status to return */
  
  /* Module initialized ? */
  orxASSERT((sstHashMap.u32Flags & orxHASHMAP_KU32_FLAG_READY) == orxHASHMAP_KU32_FLAG_READY);

  /* Correct parameters ? */
  orxASSERT(_pstHashMap != orxNULL);

  /* Get the index from the key */
  u32Index = orxHashMap_FindIndex(_pstHashMap, _u32Key);
  
  pstCell = _pstHashMap->apstCell[u32Index];

  /* Is the first key is the key to remove ? */
  if (pstCell != orxNULL)
  {
    if (pstCell->u32Key == _u32Key)
    {
      /* The first cell has to be removed */
      _pstHashMap->apstCell[u32Index] = pstCell->pstNext;
      orxBank_Free(_pstHashMap->pstBank, pstCell);
      
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
        orxBank_Free(_pstHashMap->pstBank, pstRemoveCell);
        
        /* Operation succeed */
        eStatus = orxSTATUS_SUCCESS;
      }
    }
  }
  
  return eStatus;
}

/*******************************************************************************
 * DEBUG FUNCTION
 ******************************************************************************/

/** Print the content of a Hash map
 * @param _pstHashMap (IN) Hash map to display
 */
orxVOID orxHashMap_DebugPrint(orxHASHMAP *_pstHashMap)
{
  orxHASHMAP_CELL *pstCell = orxNULL;
  orxU32 u32Index;
    
  /* Module initialized ? */
  orxASSERT((sstHashMap.u32Flags & orxHASHMAP_KU32_FLAG_READY) == orxHASHMAP_KU32_FLAG_READY);

  /* Correct parameters ? */
  orxASSERT(_pstHashMap != orxNULL);

  orxString_PrintLn("\n\n\n********* HashMap (%x) *********", _pstHashMap);

  for (u32Index = 0; u32Index < orxHASHMAP_KU32_INDEX_SIZE; u32Index++)
  {
    orxString_Print("[%3d]-->", u32Index);
    pstCell = _pstHashMap->apstCell[u32Index];
    
    while (pstCell != orxNULL)
    {
      orxString_Print("(%u/%d)-->", pstCell->u32Key, (orxS32)pstCell->pData);
      pstCell = pstCell->pstNext;
    }
    
    orxString_PrintLn("NULL");
  }
}
