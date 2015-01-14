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
 * @file orxHashTable.h
 * @date 05/05/2005
 * @author cursor@arcallians.org
 *
 * @todo
 */

/**
 * @addtogroup orxHashTable
 * 
 * Hashtable module
 * Module that handles Hashtables
 *
 * @{
 */

 
#ifndef _orxHASHTABLE_H_
#define _orxHASHTABLE_H_
 
#include "orxInclude.h"
#include "memory/orxMemory.h"

/* Internal hash table structure */
typedef struct __orxHASHTABLE_t orxHASHTABLE;

/* Define flags */
#define orxHASHTABLE_KU32_FLAG_NONE             0x00000000  /**< No flags (default behaviour) */
#define orxHASHTABLE_KU32_FLAG_NOT_EXPANDABLE   0x00000001  /**< The Hash table will not be expandable */


/** @name HashTable creation/destruction.
 * @{ */
/** Create a new hash table and return it.
 * @param[in] _u32NbKey    Number of keys that will be inserted.
 * @param[in] _u32Flags    Flags used by the hash table
 * @param[in] _eMemType    Memory type to use
 * @return Returns the hashtable pointer or orxNULL if failed.
 */
extern orxDLLAPI orxHASHTABLE *orxFASTCALL      orxHashTable_Create(orxU32 _u32NbKey, orxU32 _u32Flags, orxMEMORY_TYPE _eMemType);

/** Delete a hash table.
 * @param[in] _pstHashTable  Hash table to delete.
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxHashTable_Delete(orxHASHTABLE *_pstHashTable);

/** Clear a hash table.
 * @param[in] _pstHashTable  Hash table to clear.
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxHashTable_Clear(orxHASHTABLE *_pstHashTable);
/** @} */

/** Gets a hash table item counter
 * @param[in] _pstHashTable         Concerned hash table
 * @return    Item number
 */
extern orxDLLAPI orxU32 orxFASTCALL             orxHashTable_GetCounter(const orxHASHTABLE *_pstHashTable);

/** @name HashTable key manipulation.
 * @{ */
/** Find an item in a hash table.
 * @param[in] _pstHashTable  The hash table where search.
 * @param[in] _u32Key      Key to find.
 * @return The Element associated to the key or orxNULL if not found.
 */
extern orxDLLAPI void *orxFASTCALL              orxHashTable_Get(const orxHASHTABLE *_pstHashTable, orxU32 _u32Key);

/** Set an item value.
 * @param[in] _pstHashTable The hash table where set.
 * @param[in] _u32Key      Key to assign.
 * @param[in] _pData       Data to assign.
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxHashTable_Set(orxHASHTABLE *_pstHashTable, orxU32 _u32Key, void *_pData);


/** Add an item value.
 * @param[in] _pstHashTable The hash table where set.
 * @param[in] _u32Key      Key to assign.
 * @param[in] _pData       Data to assign.
 * @return Returns the status of the operation. (fails if key already used)
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxHashTable_Add(orxHASHTABLE *_pstHashTable, orxU32 _u32Key, void *_pData);

/** Remove an item.
 * @param[in] _pstHashTable  The hash table where remove.
 * @param[in] _u32Key      Key to remove.
 * @return Returns the status of the operation.
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxHashTable_Remove(orxHASHTABLE *_pstHashTable, orxU32 _u32Key);
/** @} */


/** @name HashTable iteration.
 * Used to iterate on all elements of the hashtable.
 * @{ */
/** Gets the next item in the hashtable and returns an iterator for next search
 * @param[in]   _pstHashTable   Concerned HashTable
 * @param[in]   _hIterator      Iterator from previous search or orxHANDLE_UNDEFINED/orxNULL for a new search
 * @param[out]  _pu32Key        Current element key
 * @param[out]  _ppData         Current element data
 * @return Iterator for next element if an element has been found, orxHANDLE_UNDEFINED otherwise
 */
extern orxDLLAPI orxHANDLE orxFASTCALL          orxHashTable_GetNext(const orxHASHTABLE *_pstHashTable, orxHANDLE _hIterator, orxU32 *_pu32Key, void **_ppData);
/** @} */

/** Optimizes a hashtable for read accesses (minimizes number of cache misses upon collisions)
 * @param[in] _pstHashTable HashTable to optimize
 * @return orxSTATUS_SUCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxHashTable_Optimize(orxHASHTABLE *_pstHashTable);


#endif /* _orxHASHTABLE_H_ */

/** @} */
