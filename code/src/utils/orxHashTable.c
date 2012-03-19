/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2012 Orx-Project
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 *    1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 *
 *    2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 *    3. This notice may not be removed or altered from any source
 *    distribution.
 */

/**
 * @file orxHashTable.c
 * @date 05/05/2005
 * @author cursor@arcallians.org
 *
 */


#include "utils/orxHashTable.h"
#include "memory/orxBank.h"
#include "debug/orxDebug.h"
#include "utils/orxString.h"


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Hash table cell definition.*/
typedef struct __orxHASHTABLE_CELL_t
{
  orxU32                        u32Key;                       /**< Key element of a hash table : 4 */
  void                         *pData;                        /**< Address of data : 8 */
  struct __orxHASHTABLE_CELL_t *pstNext;                      /**< Next cell with the same index : 12 */

} orxHASHTABLE_CELL;

/** Hash Table */
struct __orxHASHTABLE_t
{
  orxBANK            *pstBank;                                /**< Bank where are stored cells : 4 */
  orxU32              u32Counter;                             /**< Hashtable item counter : 8 */
  orxU32              u32Size;                                /**< Hashtable size : 12 */
  orxHASHTABLE_CELL  *apstCell[1];                            /**< Hash table */
};


/***************************************************************************
 * Module global variable                                                  *
 ***************************************************************************/

/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

/** Find a row in a hash table.
 * @param[in] _pstHashTable The hash table where search.
 * @param[in] _u32Key Key to find.
 * @return index associated to the given key.
 */
static orxINLINE orxU32 orxHashTable_FindIndex(const orxHASHTABLE *_pstHashTable, orxU32 _u32Key)
{
  /* Checks */
  orxASSERT(_pstHashTable != orxNULL);

  /* Computes the hash index */
  return(_u32Key & (_pstHashTable->u32Size - 1));
}

/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** @name HashTable creation/destruction.
 * @{ */
/** Create a new hash table and return it.
 * @param[in] _u32NbKey    Number of keys that will be inserted.
 * @param[in] _u32Flags    Flags used by the hash table
 * @param[in] _eMemType    Memory type to use
 * @return Returns the hashtable pointer or orxNULL if failed.
 */
orxHASHTABLE *orxFASTCALL orxHashTable_Create(orxU32 _u32NbKey, orxU32 _u32Flags, orxMEMORY_TYPE _eMemType)
{
  orxHASHTABLE *pstHashTable = orxNULL; /* New created hash table */
  orxU32        u32Size;
  orxU32        u32Flags;               /* Flags used for bank creation */

  /* Checks */
  orxASSERT(_eMemType < orxMEMORY_TYPE_NUMBER);
  orxASSERT(_u32NbKey > 0);

  /* Gets Power of Two size */
  u32Size = orxMath_GetNextPowerOfTwo(_u32NbKey);

  /* Allocate memory for a hash table */
  pstHashTable = (orxHASHTABLE *)orxMemory_Allocate(sizeof(orxHASHTABLE) + ((u32Size - 1) * sizeof(orxHASHTABLE_CELL *)), _eMemType);

  /* Enough memory ? */
  if(pstHashTable != orxNULL)
  {
    /* Set flags */
    if(_u32Flags == orxHASHTABLE_KU32_FLAG_NOT_EXPANDABLE)
    {
      u32Flags = orxBANK_KU32_FLAG_NOT_EXPANDABLE;
    }
    else
    {
      u32Flags = orxBANK_KU32_FLAG_NONE;
    }

    /* Clean values */
    orxMemory_Zero(pstHashTable, sizeof(orxHASHTABLE) + ((u32Size - 1) * sizeof(orxHASHTABLE_CELL *)));

    /* Allocate bank for cells */
    pstHashTable->pstBank = orxBank_Create((orxU16)u32Size, sizeof(orxHASHTABLE_CELL), u32Flags, _eMemType);

    /* Correct bank allocation ? */
    if(pstHashTable->pstBank == orxNULL)
    {
      /* Allocation problem, returns orxNULL */
      orxMemory_Free(pstHashTable);
      pstHashTable = orxNULL;
    }

    /* Stores its size */
    pstHashTable->u32Size = u32Size;
  }

  return pstHashTable;
}

/** Delete a hash table.
 * @param[in] _pstHashTable  Hash table to delete.
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxHashTable_Delete(orxHASHTABLE *_pstHashTable)
{
  /* Checks */
  orxASSERT(_pstHashTable != orxNULL);

  /* Clear hash table (unallocate cells) */
  orxHashTable_Clear(_pstHashTable);

  /* Free bank */
  orxBank_Delete(_pstHashTable->pstBank);

  /* Unallocate memory */
  orxMemory_Free(_pstHashTable);

  /* Done ! */
  return orxSTATUS_SUCCESS;
}

/** Clear a hash table.
 * @param[in] _pstHashTable  Hash table to clear.
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxHashTable_Clear(orxHASHTABLE *_pstHashTable)
{
  /* Checks */
  orxASSERT(_pstHashTable != orxNULL);

  /* Clear the memory bank */
  orxBank_Clear(_pstHashTable->pstBank);

  /* Clear the hash */
  orxMemory_Zero(_pstHashTable->apstCell, _pstHashTable->u32Size * sizeof(orxHASHTABLE_CELL *));

  /* Clears counter */
  _pstHashTable->u32Counter = 0;

  /* Done! */
  return orxSTATUS_SUCCESS;
}

/** Gets a hash table item counter
 * @param[in] _pstHashTable         Concerned hash table
 * @return    Item number
 */
orxU32 orxFASTCALL orxHashTable_GetCounter(orxHASHTABLE *_pstHashTable)
{
  orxU32 u32Result = orxU32_UNDEFINED;

  /* Checks */
  orxASSERT(_pstHashTable != orxNULL);

  /* Updates result */
  u32Result = _pstHashTable->u32Counter;

  /* Done! */
  return u32Result;
}

/** @name HashTable key manipulation.
 * @{ */
/** Find an item in a hash table.
 * @param[in] _pstHashTable  The hash table where search.
 * @param[in] _u32Key      Key to find.
 * @return The Element associated to the key or orxNULL if not found.
 */
void *orxFASTCALL orxHashTable_Get(const orxHASHTABLE *_pstHashTable, orxU32 _u32Key)
{
  orxU32 u32Index;                    /* Hash table index */
  orxHASHTABLE_CELL *pstCell = orxNULL; /* Cell used to traverse */

  /* Checks */
  orxASSERT(_pstHashTable != orxNULL);

  /* Get the index from the key */
  u32Index = orxHashTable_FindIndex(_pstHashTable, _u32Key);

  /* Traverse to find the key */
  pstCell = _pstHashTable->apstCell[u32Index];
  while(pstCell != orxNULL && pstCell->u32Key != _u32Key)
  {
    /* Try with next cell */
    pstCell = pstCell->pstNext;
  }

  /* Cell found ? */
  if(pstCell != orxNULL)
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
 * @param[in] _pstHashTable The hash table where set.
 * @param[in] _u32Key      Key to assign.
 * @param[in] _pData       Data to assign.
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxHashTable_Set(orxHASHTABLE *_pstHashTable, orxU32 _u32Key, void *_pData)
{
  orxU32 u32Index;                      /* Hash table index */
  orxHASHTABLE_CELL *pstCell = orxNULL; /* Cell used to traverse */

  /* Checks */
  orxASSERT(_pstHashTable != orxNULL);

  /* Get the index from the key */
  u32Index = orxHashTable_FindIndex(_pstHashTable, _u32Key);

  /* Traverse to find the key */
  pstCell = _pstHashTable->apstCell[u32Index];
  while(pstCell != orxNULL && pstCell->u32Key != _u32Key)
  {
    /* Try with next cell */
    pstCell = pstCell->pstNext;
  }

  /* Cell found ? */
  if(pstCell != orxNULL)
  {
    /* Set associated datas */
    pstCell->pData = _pData;
  }
  else
  {
    /* Allocate a new cell if possible */
    pstCell = (orxHASHTABLE_CELL *)orxBank_Allocate(_pstHashTable->pstBank);

    /* If allocation succeed, insert the new cell */
    if(pstCell != orxNULL)
    {
      /* Set datas */
      pstCell->u32Key = _u32Key;
      pstCell->pData  = _pData;
      pstCell->pstNext = _pstHashTable->apstCell[u32Index];

      /* Insert it */
      _pstHashTable->apstCell[u32Index] = pstCell;
    }
  }

  /* Done! */
  return orxSTATUS_SUCCESS;
}


/** Add an item value.
 * @param[in] _pstHashTable The hash table where set.
 * @param[in] _u32Key      Key to assign.
 * @param[in] _pData       Data to assign.
 * @return Returns the status of the operation. (fails if key already used)
 */
orxSTATUS orxFASTCALL orxHashTable_Add(orxHASHTABLE *_pstHashTable, orxU32 _u32Key, void *_pData)
{
  orxU32 u32Index;                      /* Hash index */
  orxHASHTABLE_CELL *pstCell;             /* New cell to add */
  orxSTATUS eStatus = orxSTATUS_FAILURE; /* Status to return */

  /* Checks */
  orxASSERT(_pstHashTable != orxNULL);

  /* Can't add a NULL pointer, else Get will returns orxNULL and it won't be possible to detect errors
   * Maybe that this behaviour should be different ?
   */
  orxASSERT(_pData != orxNULL);

  /* The key must not exist */
  if(orxHashTable_Get(_pstHashTable, _u32Key) == orxNULL)
  {
    /* Get the hash table index */
    u32Index = orxHashTable_FindIndex(_pstHashTable, _u32Key);

    /* Allocate a new cell if possible */
    pstCell = (orxHASHTABLE_CELL *)orxBank_Allocate(_pstHashTable->pstBank);

    /* If allocation succeed, insert the new cell */
    if(pstCell != orxNULL)
    {
      /* Set datas */
      pstCell->u32Key = _u32Key;
      pstCell->pData  = _pData;
      pstCell->pstNext = _pstHashTable->apstCell[u32Index];

      /* Insert it */
      _pstHashTable->apstCell[u32Index] = pstCell;
      eStatus = orxSTATUS_SUCCESS;

      /* Updates counter */
      _pstHashTable->u32Counter++;
    }
  }

  return eStatus;
}

/** Remove an item.
 * @param[in] _pstHashTable  The hash table where remove.
 * @param[in] _u32Key      Key to remove.
 * @return Returns the status of the operation.
 */
orxSTATUS orxFASTCALL orxHashTable_Remove(orxHASHTABLE *_pstHashTable, orxU32 _u32Key)
{
  orxU32 u32Index;                          /* Hash table index */
  orxHASHTABLE_CELL *pstCell = orxNULL;       /* Cell used to traverse */
  orxHASHTABLE_CELL *pstRemoveCell = orxNULL; /* Cell to remove */
  orxSTATUS eStatus = orxSTATUS_FAILURE; /* Status to return */

  /* Checks */
  orxASSERT(_pstHashTable != orxNULL);

  /* Get the index from the key */
  u32Index = orxHashTable_FindIndex(_pstHashTable, _u32Key);

  pstCell = _pstHashTable->apstCell[u32Index];

  /* Is the first key is the key to remove ? */
  if(pstCell != orxNULL)
  {
    if(pstCell->u32Key == _u32Key)
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
      while(pstCell->pstNext != orxNULL && pstCell->pstNext->u32Key != _u32Key)
      {
        /* Try with next cell */
        pstCell = pstCell->pstNext;
      }

      /* Cell found ? (key should be on the next cell) */
      if(pstCell->pstNext != orxNULL)
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

  /* Successful? */
  if(eStatus != orxSTATUS_FAILURE)
  {
    /* Updates counter */
    _pstHashTable->u32Counter--;
  }

  return eStatus;
}

/*******************************************************************************
 * Search functions
 ******************************************************************************/

orxHANDLE orxFASTCALL orxHashTable_FindFirst(orxHASHTABLE *_pstHashTable, orxU32 *_pu32Key, void **_ppData)
{
  orxHANDLE hResult = orxHANDLE_UNDEFINED;
  orxU32    u32Cell;

  /* Checks */
  orxASSERT(_pstHashTable != orxNULL);

  for(u32Cell = 0; u32Cell < _pstHashTable->u32Size; u32Cell++)
  {
    if(_pstHashTable->apstCell[u32Cell] != orxNULL)
    {
      if(_pu32Key != orxNULL)
      {
          *_pu32Key = _pstHashTable->apstCell[u32Cell]->u32Key;
      }
      if(_ppData!=orxNULL)
      {
          *_ppData = _pstHashTable->apstCell[u32Cell]->pData;
      }

      /* Updates result */
      hResult = (orxHANDLE)(_pstHashTable->apstCell[u32Cell]);
      break;
    }
  }

  /* Done! */
  return hResult;
}

orxHANDLE orxFASTCALL orxHashTable_FindNext(orxHASHTABLE *_pstHashTable, orxHANDLE _hIterator, orxU32 *_pu32Key, void **_ppData)
{
  orxHASHTABLE_CELL *pIter = (orxHASHTABLE_CELL*)_hIterator;
  orxU32             u32Cell;
  orxHANDLE          hResult = orxHANDLE_UNDEFINED;

  /* Checks */
  orxASSERT(_pstHashTable != orxNULL && _hIterator != orxHANDLE_UNDEFINED);

  if(pIter->pstNext != orxNULL)
  {
    if(_pu32Key != orxNULL)
    {
      *_pu32Key = pIter->pstNext->u32Key;
    }
    if(_ppData != orxNULL)
    {
      *_ppData = pIter->pstNext->pData;
    }

    /* Updates result */
    hResult = (orxHANDLE)(pIter->pstNext);
  }
  else
  {
    for(u32Cell = orxHashTable_FindIndex(_pstHashTable, pIter->u32Key) + 1; u32Cell < _pstHashTable->u32Size; u32Cell++)
    {
      if(_pstHashTable->apstCell[u32Cell]!=orxNULL)
      {
        if(_pu32Key!=orxNULL)
        {
          *_pu32Key = _pstHashTable->apstCell[u32Cell]->u32Key;
        }
        if(_ppData!=orxNULL)
        {
          *_ppData = _pstHashTable->apstCell[u32Cell]->pData;
        }

        /* Updates result */
        hResult = (orxHANDLE)(_pstHashTable->apstCell[u32Cell]);
        break;
      }
    }
  }

  /* Done! */
  return hResult;
}

/*******************************************************************************
 * DEBUG FUNCTION
 ******************************************************************************/

#ifdef __orxDEBUG__

/** Print the content of a Hash table
 * @param[in] _pstHashTable  Hash table to display
 */
void orxFASTCALL orxHashTable_DebugPrint(const orxHASHTABLE *_pstHashTable)
{
  orxHASHTABLE_CELL *pstCell = orxNULL;
  orxU32 u32Index;

  /* Checks */
  orxASSERT(_pstHashTable != orxNULL);

  orxLOG("\n\n\n********* HashTable (%x) *********", _pstHashTable);

  for(u32Index = 0; u32Index < _pstHashTable->u32Size; u32Index++)
  {
    orxCHAR zBuffer[4096];

    orxMemory_Zero(zBuffer, 4096 * sizeof(orxCHAR));

    orxString_Print(zBuffer, "[%3d]-->", u32Index);
    pstCell = _pstHashTable->apstCell[u32Index];

    while(pstCell != orxNULL)
    {
      orxString_Print(zBuffer, "(%u/%p)-->", pstCell->u32Key, pstCell->pData);
      pstCell = pstCell->pstNext;
    }

    orxLOG("%sNULL", zBuffer);
  }
}

#endif /* __orxDEBUG__ */
