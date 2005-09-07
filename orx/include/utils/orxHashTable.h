/**
 * @file orxHashTable.h
 * 
 * Manipulation of hash tables.
 * 
 */ 
 
 /***************************************************************************
 orxHashTable.h
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
 
#ifndef _orxHASHTABLE_H_
#define _orxHASHTABLE_H_
 
#include "orxInclude.h"
#include "memory/orxMemory.h"

/* Internal hash table structure */
typedef struct __orxHASHTABLE_t orxHASHTABLE;

/* Define flags */
#define orxHASHTABLE_KU32_FLAGS_NONE            0x00000000  /**< No flags (default behaviour) */
#define orxHASHTABLE_KU32_FLAGS_NOT_EXPANDABLE  0x00000001  /**< The Hash table will not be expandable */


/** @name Module management.
 * @{ */
/** Initialize HashTable Module
 * @return Returns the initialization status.
 */
extern orxDLLAPI orxSTATUS                      orxHashTable_Init();
/** Exit HashTable module
 */
extern orxDLLAPI orxVOID                        orxHashTable_Exit();
/** @} */

/** @name HashTable creation/destruction.
 * @{ */
/** Create a new hash table and return it.
 * @param _u32NbKey   (IN) Number of keys that will be inserted.
 * @param _u32Flags   (IN) Flags used by the hash table
 * @param _eMemType   (IN) Memory type to use
 * @return Returns the hashtable pointer or orxNULL if failed.
 */
extern orxDLLAPI orxHASHTABLE *orxFASTCALL      orxHashTable_Create(orxU32 _u32NbKey, orxU32 _u32Flags, orxMEMORY_TYPE _eMemType);

/** Delete a hash table.
 * @param _pstHashTable (IN) Hash table to delete.
 */
extern orxDLLAPI orxVOID orxFASTCALL            orxHashTable_Delete(orxHASHTABLE *_pstHashTable);

/** Clear a hash table.
 * @param _pstHashTable (IN) Hash table to clear.
 */
extern orxDLLAPI orxVOID orxFASTCALL            orxHashTable_Clear(orxHASHTABLE *_pstHashTable);
/** @} */


/** @name HashTable key manipulation.
 * @{ */
/** Find an item in a hash table.
 * @param _pstHashTable (IN) The hash table where search.
 * @param _u32Key     (IN) Key to find.
 * @return The Element associated to the key or orxNULL if not found.
 */
extern orxDLLAPI orxVOID *orxFASTCALL           orxHashTable_Get(orxCONST orxHASHTABLE *_pstHashTable, orxU32 _u32Key);

/** Set an item value.
 * @param _pstHashTable The hash table where set.
 * @param _u32Key     (IN) Key to assign.
 * @param _pData      (IN) Data to assign.
 */
extern orxDLLAPI orxVOID orxFASTCALL            orxHashTable_Set(orxHASHTABLE *_pstHashTable, orxU32 _u32Key, orxVOID *_pData);


/** Add an item value.
 * @param _pstHashTable The hash table where set.
 * @param _u32Key     (IN) Key to assign.
 * @param _pData      (IN) Data to assign.
 * @return Returns the status of the operation. (fails if key already used)
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxHashTable_Add(orxHASHTABLE *_pstHashTable, orxU32 _u32Key, orxVOID *_pData);

/** Remove an item.
 * @param _pstHashTable (IN) The hash table where remove.
 * @param _u32Key     (IN) Key to remove.
 * @return Returns the status of the operation.
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxHashTable_Remove(orxHASHTABLE *_pstHashTable, orxU32 _u32Key);
/** @} */


/*******************************************************************************
 * DEBUG FUNCTION
 ******************************************************************************/

/** Print the content of a Hash table
 * @param _pstHashTable (IN) Hash table to display
 */
extern orxDLLAPI orxVOID orxFASTCALL            orxHashTable_DebugPrint(orxCONST orxHASHTABLE *_pstHashTable);


#endif /* _orxHASHTABLE_H_ */
