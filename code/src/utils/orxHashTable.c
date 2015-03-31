/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2015 Orx-Project
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
#include "debug/orxProfiler.h"
#include "utils/orxString.h"


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Hash table cell definition.*/
typedef struct __orxHASHTABLE_CELL_t
{
  orxU64                        u64Key;                       /**< Key element of a hash table : 8 */
  void                         *pData;                        /**< Address of data : 12 / 16 */
  struct __orxHASHTABLE_CELL_t *pstNext;                      /**< Next cell with the same index : 16 / 20 */

} orxHASHTABLE_CELL;

/** Hash Table */
#ifdef __orxMSVC__
  #pragma warning(disable : 4200)
#endif /* __orxMSVC__ */

struct __orxHASHTABLE_t
{
  orxBANK            *pstBank;                                /**< Bank where are stored cells : 4 */
  orxU32              u32Counter;                             /**< Hashtable item counter : 8 */
  orxU32              u32Size;                                /**< Hashtable size : 12 */
  orxHASHTABLE_CELL  *apstCell[0];                            /**< Hash table */
};

#ifdef __orxMSVC__
  #pragma warning(default : 4200)
#endif /* __orxMSVC__ */


/***************************************************************************
 * Module global variable                                                  *
 ***************************************************************************/

/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

/** Find a row in a hash table.
 * @param[in] _pstHashTable The hash table where search.
 * @param[in] _u64Key Key to find.
 * @return index associated to the given key.
 */
static orxINLINE orxU32 orxHashTable_FindIndex(const orxHASHTABLE *_pstHashTable, orxU64 _u64Key)
{
  /* Checks */
  orxASSERT(_pstHashTable != orxNULL);

  /* Computes the hash index */
  return((orxU32)_u64Key & (_pstHashTable->u32Size - 1));
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
  orxHASHTABLE *pstHashTable;
  orxU32        u32Size;
  orxU32        u32Flags;

  /* Checks */
  orxASSERT(_eMemType < orxMEMORY_TYPE_NUMBER);
  orxASSERT(_u32NbKey > 0);

  /* Gets Power of Two size */
  u32Size = orxMath_GetNextPowerOfTwo(_u32NbKey);

  /* Allocate memory for a hash table */
  pstHashTable = (orxHASHTABLE *)orxMemory_Allocate(sizeof(orxHASHTABLE) + (u32Size * sizeof(orxHASHTABLE_CELL *)), _eMemType);

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
    orxMemory_Zero(pstHashTable, sizeof(orxHASHTABLE) + (u32Size * sizeof(orxHASHTABLE_CELL *)));

    /* Allocate bank for cells */
    pstHashTable->pstBank = orxBank_Create((orxU16)u32Size, sizeof(orxHASHTABLE_CELL), u32Flags, _eMemType);

    /* Correct bank allocation ? */
    if(pstHashTable->pstBank != orxNULL)
    {
      /* Stores its size */
      pstHashTable->u32Size = u32Size;
    }
    else
    {
      /* Allocation problem, returns orxNULL */
      orxMemory_Free(pstHashTable);
      pstHashTable = orxNULL;
    }
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
orxU32 orxFASTCALL orxHashTable_GetCounter(const orxHASHTABLE *_pstHashTable)
{
  orxU32 u32Result;

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
 * @param[in] _u64Key      Key to find.
 * @return The Element associated to the key or orxNULL if not found.
 */
void *orxFASTCALL orxHashTable_Get(const orxHASHTABLE *_pstHashTable, orxU64 _u64Key)
{
  orxU32              u32Index;
  orxHASHTABLE_CELL  *pstCell = orxNULL;

  /* Profiles */
  orxPROFILER_PUSH_MARKER("orxHashTable_Get");

  /* Checks */
  orxASSERT(_pstHashTable != orxNULL);

  /* Gets the index from the key */
  u32Index = orxHashTable_FindIndex(_pstHashTable, _u64Key);

  /* Finds the corresponding cell */
  for(pstCell = _pstHashTable->apstCell[u32Index];
      (pstCell != orxNULL) && (pstCell->u64Key != _u64Key);
      pstCell = pstCell->pstNext);

  /* Profiles */
  orxPROFILER_POP_MARKER();

  /* Done! */
  return (pstCell != orxNULL) ? pstCell->pData : orxNULL;
}

/** Set an item value.
 * @param[in] _pstHashTable The hash table where set.
 * @param[in] _u64Key      Key to assign.
 * @param[in] _pData       Data to assign.
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxHashTable_Set(orxHASHTABLE *_pstHashTable, orxU64 _u64Key, void *_pData)
{
  orxU32              u32Index;
  orxHASHTABLE_CELL  *pstCell = orxNULL;

  /* Profiles */
  orxPROFILER_PUSH_MARKER("orxHashTable_Set");

  /* Checks */
  orxASSERT(_pstHashTable != orxNULL);

  /* Gets the index from the key */
  u32Index = orxHashTable_FindIndex(_pstHashTable, _u64Key);

  /* Finds the corresponding cell */
  for(pstCell = _pstHashTable->apstCell[u32Index];
      (pstCell != orxNULL) && (pstCell->u64Key != _u64Key);
      pstCell = pstCell->pstNext);

  /* Found ? */
  if(pstCell != orxNULL)
  {
    /* Stores data */
    pstCell->pData = _pData;
  }
  else
  {
    /* Creates a new cell */
    pstCell = (orxHASHTABLE_CELL *)orxBank_Allocate(_pstHashTable->pstBank);

    /* Success? */
    if(pstCell != orxNULL)
    {
      /* Inits cell */
      pstCell->u64Key   = _u64Key;
      pstCell->pData    = _pData;
      pstCell->pstNext  = _pstHashTable->apstCell[u32Index];

      /* Updates counter */
      _pstHashTable->u32Counter++;

      /* Inserts it */
      _pstHashTable->apstCell[u32Index] = pstCell;
    }
  }

  /* Profiles */
  orxPROFILER_POP_MARKER();

  /* Done! */
  return orxSTATUS_SUCCESS;
}


/** Add an item value.
 * @param[in] _pstHashTable The hash table where set.
 * @param[in] _u64Key      Key to assign.
 * @param[in] _pData       Data to assign.
 * @return Returns the status of the operation. (fails if key already used)
 */
orxSTATUS orxFASTCALL orxHashTable_Add(orxHASHTABLE *_pstHashTable, orxU64 _u64Key, void *_pData)
{
  orxU32              u32Index;
  orxHASHTABLE_CELL  *pstCell;
  orxSTATUS           eStatus = orxSTATUS_FAILURE;

  /* Profiles */
  orxPROFILER_PUSH_MARKER("orxHashTable_Add");

  /* Checks */
  orxASSERT(_pstHashTable != orxNULL);
  orxASSERT(_pData != orxNULL);

  /* Gets the index from the key */
  u32Index = orxHashTable_FindIndex(_pstHashTable, _u64Key);

  /* Finds the corresponding cell */
  for(pstCell = _pstHashTable->apstCell[u32Index];
      (pstCell != orxNULL) && (pstCell->u64Key != _u64Key);
      pstCell = pstCell->pstNext);

  /* Not found? */
  if(pstCell == orxNULL)
  {
    /* Creates a new cell */
    pstCell = (orxHASHTABLE_CELL *)orxBank_Allocate(_pstHashTable->pstBank);

    /* Success? */
    if(pstCell != orxNULL)
    {
      /* Inits cell */
      pstCell->u64Key   = _u64Key;
      pstCell->pData    = _pData;
      pstCell->pstNext  = _pstHashTable->apstCell[u32Index];

      /* Inserts it */
      _pstHashTable->apstCell[u32Index] = pstCell;

      /* Updates counter */
      _pstHashTable->u32Counter++;

      /* Updates result */
      eStatus = orxSTATUS_SUCCESS;
    }
  }

  /* Profiles */
  orxPROFILER_POP_MARKER();

  /* Done! */
  return eStatus;
}

/** Remove an item.
 * @param[in] _pstHashTable  The hash table where remove.
 * @param[in] _u64Key      Key to remove.
 * @return Returns the status of the operation.
 */
orxSTATUS orxFASTCALL orxHashTable_Remove(orxHASHTABLE *_pstHashTable, orxU64 _u64Key)
{
  orxU32 u32Index;                        /* Hash table index */
  orxHASHTABLE_CELL *pstCell;             /* Cell used to traverse */
  orxSTATUS eStatus = orxSTATUS_FAILURE;  /* Status to return */

  /* Profiles */
  orxPROFILER_PUSH_MARKER("orxHashTable_Remove");

  /* Checks */
  orxASSERT(_pstHashTable != orxNULL);

  /* Get the index from the key */
  u32Index = orxHashTable_FindIndex(_pstHashTable, _u64Key);

  pstCell = _pstHashTable->apstCell[u32Index];

  /* Is the first key is the key to remove ? */
  if(pstCell != orxNULL)
  {
    if(pstCell->u64Key == _u64Key)
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
      while(pstCell->pstNext != orxNULL && pstCell->pstNext->u64Key != _u64Key)
      {
        /* Try with next cell */
        pstCell = pstCell->pstNext;
      }

      /* Cell found ? (key should be on the next cell) */
      if(pstCell->pstNext != orxNULL)
      {
        orxHASHTABLE_CELL *pstRemoveCell;

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

  /* Profiles */
  orxPROFILER_POP_MARKER();

  /* Done! */
  return eStatus;
}

/*******************************************************************************
 * Search functions
 ******************************************************************************/

/** Gets the next item in the hashtable and returns an iterator for next search
 * @param[in]   _pstHashTable   Concerned HashTable
 * @param[in]   _hIterator      Iterator from previous search or orxHANDLE_UNDEFINED/orxNULL for a new search
 * @param[out]  _pu64Key        Current element key
 * @param[out]  _ppData         Current element data
 * @return Iterator for next element if an element has been found, orxHANDLE_UNDEFINED otherwise
 */
orxHANDLE orxFASTCALL orxHashTable_GetNext(const orxHASHTABLE *_pstHashTable, orxHANDLE _hIterator, orxU64 *_pu64Key, void **_ppData)
{
  orxHASHTABLE_CELL  *pstCell;
  orxU32              u32Index;
  orxHANDLE           hResult;

  /* Checks */
  orxASSERT(_pstHashTable != orxNULL);

  /* Has iterator? */
  if((_hIterator != orxNULL) && (_hIterator != orxHANDLE_UNDEFINED))
  {
    /* Gets current cell */
    pstCell = (orxHASHTABLE_CELL *)_hIterator;

    /* Gets start index */
    u32Index = orxHashTable_FindIndex(_pstHashTable, pstCell->u64Key) + 1;

    /* Updates temporary result */
    pstCell = pstCell->pstNext;
  }
  else
  {
    /* Starts a new search */
    u32Index  = 0;
    pstCell   = orxNULL;
  }

  /* Finds next head cell if needed */
  for(; (pstCell == orxNULL) && (u32Index < _pstHashTable->u32Size); pstCell = _pstHashTable->apstCell[u32Index++]);

  /* Found? */
  if(pstCell != orxNULL)
  {
    /* Asked for key? */
    if(_pu64Key != orxNULL)
    {
      /* Updates it */
      *_pu64Key = pstCell->u64Key;
    }

    /* Asked for data? */
    if(_ppData != orxNULL)
    {
      /* Updates it */
      *_ppData = pstCell->pData;
    }

    /* Updates result */
    hResult = (orxHANDLE)pstCell;
  }
  else
  {
    /* Updates result */
    hResult = orxHANDLE_UNDEFINED;
  }

  /* Done! */
  return hResult;
}

/** Optimizes a hashtable for read accesses (minimizes number of cache misses upon collisions)
 * @param[in] _pstHashTable HashTable to optimize
 * @return orxSTATUS_SUCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxHashTable_Optimize(orxHASHTABLE *_pstHashTable)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(_pstHashTable != orxNULL);

  /* Has elements? */
  if(_pstHashTable->u32Counter > 0)
  {
    orxHASHTABLE_CELL *astWorkBuffer;

    /* Allocates work buffer */
    astWorkBuffer = (orxHASHTABLE_CELL *)orxMemory_Allocate(_pstHashTable->u32Counter * sizeof(orxHASHTABLE_CELL), orxMEMORY_TYPE_TEMP);

    /* Valid? */
    if(astWorkBuffer != orxNULL)
    {
      orxU64              u64KeyIndex;
      orxU32              u32BufferIndex, i;
      orxHASHTABLE_CELL  *pstCell;

      /* For all cells */
      for(i = 0, u64KeyIndex = 0, u32BufferIndex = 0, pstCell = orxNULL; i < _pstHashTable->u32Counter; i++, u32BufferIndex++)
      {
        /* Linked cell? */
        if((pstCell != orxNULL) && (pstCell->pstNext != orxNULL))
        {
          /* Gets next in line */
          pstCell = pstCell->pstNext;
        }
        else
        {
          /* Finds next head cell */
          do{pstCell = _pstHashTable->apstCell[u64KeyIndex++];} while(pstCell == orxNULL);
        }

        /* Stores it */
        orxMemory_Copy(&astWorkBuffer[u32BufferIndex], pstCell, sizeof(orxHASHTABLE_CELL));
      }

      /* Clears bank */
      orxBank_Clear(_pstHashTable->pstBank);

      /* For all ordered cells */
      for(i = 0, pstCell = orxNULL; i < _pstHashTable->u32Counter; i++)
      {
        orxHASHTABLE_CELL *pstPreviousCell;

        /* Allocates new cell */
        pstPreviousCell = pstCell;
        pstCell         = (orxHASHTABLE_CELL *)orxBank_Allocate(_pstHashTable->pstBank);

        /* Checks */
        orxASSERT(pstCell != orxNULL);

        /* Stores its data */
        orxMemory_Copy(pstCell, &astWorkBuffer[i], sizeof(orxHASHTABLE_CELL));

        /* Chained? */
        if((pstPreviousCell != orxNULL) && (pstPreviousCell->pstNext != orxNULL))
        {
          /* Updates chaining */
          pstPreviousCell->pstNext = pstCell;
        }
        else
        {
          /* Updates head pointer */
          _pstHashTable->apstCell[orxHashTable_FindIndex(_pstHashTable, pstCell->u64Key)] = pstCell;
        }
      }

      /* Clears work buffer */
      orxMemory_Free(astWorkBuffer);
    }
    else
    {
      /* Updates result */
      eResult = orxSTATUS_FAILURE;
    }
  }

  /* Done! */
  return eResult;
}
