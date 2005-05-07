/**
 * @file orxHashMap.h
 * 
 * Manipulation of hash maps.
 * Use memory bank to reduce allocation.
 * 
 */ 
 
 /***************************************************************************
 orxHashMap.h
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
 
#ifndef _orxHASHMAP_H_
#define _orxHASHMAP_H_
 
#include "orxInclude.h"
#include "memory/orxMemory.h"

/* Internal hash map structure */
typedef struct __orxHASHMAP_t orxHASHMAP;

/* Define flags */
#define orxHASHMAP_KU32_FLAGS_NONE            0x00000000  /**< No flags (default behaviour) */
#define orxHASHMAP_KU32_FLAGS_NOT_EXPANDABLE  0x00000001  /**< The Hash map will not be expandable */

/** @name Module management.
 * @{ */
/** Initialize HashMap Module
 * @return Returns the initialization status.
 */
extern orxDLLAPI orxSTATUS orxHashMap_Init();

/** Exit HashMap module
 */
extern orxDLLAPI orxVOID orxHashMap_Exit();
/** @} */

/** @name HashMap creation/destruction.
 * @{ */
/** Create a new hash map and return it.
 * @param _u32NbKey   (IN) Number of keys that will be inserted.
 * @param _u32Flags   (IN) Flags used by the hash map
 * @param _eMemType   (IN) Memory type to use
 * @return Returns the hashmap pointer or orxNULL if failed.
 */
extern orxDLLAPI orxHASHMAP *orxHashMap_Create(orxU32 _u32NbKey, orxU32 _u32Flags, orxMEMORY_TYPE _eMemType);

/** Delete a hash map.
 * @param _pstHashMap (IN) Hash map to delete.
 */
extern orxDLLAPI orxVOID orxHashMap_Delete(orxHASHMAP *_pstHashMap);

/** Clear a hash map.
 * @param _pstHashMap (IN) Hash map to clear.
 * @return Returns the status of the operation.
 */
extern orxDLLAPI orxSTATUS orxHashMap_Clear(orxHASHMAP *_pstHashMap);
/** @} */


/** @name HashMap key manipulation.
 * @{ */
/** Find an item in a hash map.
 * @param _pstHashMap (IN) The hash map where search.
 * @param _u32Key     (IN) Key to find.
 * @return The Element associated to the key or orxNULL if not found.
 */
extern orxDLLAPI orxVOID *orxHashMap_Get(orxHASHMAP *_pstHashMap, orxU32 _u32Key);

/** Add an item value.
 * @param _pstHashMap The hash map where set.
 * @param _u32Key     (IN) Key to assign.
 * @param _pData      (IN) Data to assign.
 * @return Returns the status of the operation. (fails if key already used)
 */
extern orxDLLAPI orxSTATUS orxHashMap_Add(orxHASHMAP *_pstHashMap, orxU32 _u32Key, orxVOID *_pData);

/** Remove an item.
 * @param _pstHashMap (IN) The hash map where remove.
 * @param _u32Key     (IN) Key to remove.
 * @return Returns the status of the operation.
 */
extern orxDLLAPI orxSTATUS orxHashMap_Remove(orxHASHMAP *_pstHashMap, orxU32 _u32Key);
/** @} */

#endif /* _orxHASHMAP_H_ */
