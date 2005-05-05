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


/** Hash map cell definition.*/
typedef struct __orxHASHMAP_CELL_t
{
    orxU32   u32Key;    /**< Key element of a hash map. */
    orxVOID  *pData;    /**< Address of data. */
} orxHASHMAP_CELL;

 
/* Internal hash map structure */
typedef struct __orxHASHMAP_t *orxHASHMAP;



/** @name Module management.
 * @{ */
/** Initialize HashMap Module
 */
extern orxDLLAPI orxSTATUS orxHashMap_Init();
/** Exit HashMap module
 */
extern orxDLLAPI orxVOID orxHashMap_Exit();
/** @} */

/** @name HashMap creation/destruction.
 * @{ */
/** Create a new hash map and return it.
 * @param _u32InitNbElem Number of rows initially allocated.
 * @return Returns the hashmap or 0 if failed.
 */
extern orxDLLAPI orxHASHMAP orxHashMap_Create(orxU32 _u32InitNbRows);
/** Delete a hash map.
 * @param _pstHashMap Hash map to delete.
 */
extern orxDLLAPI orxVOID orxHashMap_Delete(orxHASHMAP _pstHashMap);
/** Clear a hash map.
 * @param _pstHashMap Hash map to clear.
 */
extern orxDLLAPI orxVOID orxHashMap_Clear(orxHASHMAP _pstHashMap);
/** @} */


/** @name HashMap key manipulation.
 * @{ */
/** Find an item in a hash map.
 * @param _pstHashMap The hash map where search.
 * @param _u32Key Key to find.
 * @return The cell or 0 if not found.
 */
extern orxDLLAPI orxHASHMAP_CELL* orxHashMap_FindKey(orxHASHMAP _pstHashMap, orxU32 _u32Key);
/** Set an item value.
 * @param _pstHashMap The hash map where set.
 * @param _u32Key Key to assign.
 * @param _pValue Value to assign.
 */
extern orxDLLAPI orxVOID orxHashMap_SetKey(orxHASHMAP _pstHashMap, orxU32 _u32Key, orxVOID *_pValue);
/** Remove an item.
 * @param _pstHashMap The hash map where remove.
 * @param _u32Key Key to remove.
 */
extern orxDLLAPI orxVOID orxHashMap_RemoveKey(orxHASHMAP _pstHashMap, orxU32 _u32Key);
/** @} */


#endif /* _orxHASHMAP_H_ */
