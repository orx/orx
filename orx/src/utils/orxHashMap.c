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

#define orxHASHMAP_INIT_BANK_HASHMAP_ELEM       32     /**< Initial bank hashmap element number. */
#define orxHASHMAP_INIT_BANK_HASHMAP_ROWELEM    256    /**< Initial bank hashmap row element number. */

#define orxHASHMAP_INDEX_NUMBER                 256    /**< Indexing size.*/

/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

typedef struct __orxHASHMAP_ROW_t
{
    orxHASHMAP_CELL stCell;         /**< Cell of data. */
    __orxHASHMAP_ROW_t *pstNext;    /**< Next row of data . */
} orxHASHMAP_ROW, *orxHASHMAP_PROW;



typedef struct __orxHASHMAP_t
{
    orxHASHMAP_PROW astRows[orxHASHMAP_INDEX_NUMBER];     /**< Rows of data. */
} *orxHASHMAP;


/***************************************************************************
 * Module global variable                                                  *
 ***************************************************************************/
orxSTATIC orxBANK* spstHashMapBank = 0;
orxSTATIC orxBANK* spstHashMapRowBank = 0;

/***************************************************************************
 * Private functions                                                        *
 ***************************************************************************/

/** Find a row in a hash map.
 * @param _pstHashMap The hash map where search.
 * @param _u32Key Key to find.
 * @return address of the row.
 */
orxSTATIC orxHASHMAP_ROW* orxHashMap_FindRow(orxHASHMAP _pstHashMap, orxU32 _u32Key)
{
    orxHASHMAP_ROW* pstRow = _pstHashMap->astRows[_u32Key%orxHASHMAP_INDEX_NUMBER];

    while (pstRow!=0)
    {
        if (pstRow->stCell.u32Key==_u32Key)
            break;
        pstRow = pstRow->pstNext;
    }

    return pstRow;
}


/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/


/* Initialize HashMap Module */
orxSTATUS orxHashMap_Init()
{
    /** @todo Set the correct flags.*/
    spstHashMapBank = orxBank_Create(orxHASHMAP_INIT_BANK_HASHMAP_ELEM, sizeof(__orxHASHMAP_t), orxBANK_KU32_FLAGS_NONE, orxMEMORY_TYPE_MAIN);
    spstHashMapRowBank = orxBank_Create(orxHASHMAP_INIT_BANK_HASHMAP_ROWELEM, sizeof(__orxHASHMAP_ROW_t), orxBANK_KU32_FLAGS_NONE, orxMEMORY_TYPE_MAIN);
    /* Module successfully initialized */
    return orxSTATUS_SUCCESS;
}

/* Exit HashMap module */
orxVOID orxHashMap_Exit()
{
    orxBank_Delete(spstHashMapBank);
    orxBank_Delete(spstHashMapRowBank);
}



/* Create a new hash map and return it. */
orxHASHMAP orxHashMap_Create(orxU32 _u32InitNbRows)
{
    /* Allocate hash map. */
    orxHASHMAP pstMap = orxBank_Allocate(spstHashMapBank);

    /* Nulling all pointers.*/
    orxU32 u32Index;
    for (u32Index=0; u32Index<orxHASHMAP_INDEX_NUMBER; u32Index++)
        pstMap->astRows[u32Index] = 0;

    return pstMap;
}

/* Delete a hash map. */
orxVOID orxHashMap_Delete(orxHASHMAP _pstHashMap)
{
    orxHashMap_Clear(_pstHashMap);
    orxBank_Free(spstHashMapBank, _pstHashMap);
}

/* Clear a hash map. */
orxVOID orxHashMap_Clear(orxHASHMAP _pstHashMap)
{
    /* Nulling all pointers.*/
    orxU32 u32Index;
    for (u32Index=0; u32Index<orxHASHMAP_INDEX_NUMBER; u32Index++)
    {
        orxHASHMAP_ROW *pstNext, *pstRow = pstMap->astRows[u32Index];
        if (pstRow!=0)
        {
            pstMap->astRows[u32Index] = 0;
            do
            {
                pstNext = pstRow->pstNext;
                orxBank_Free(spstHashMapRowBank, pstNext);
            }
            while (pstNext!=0)
        }
    }
}


/* Find an item in a hash map. */
orxHASHMAP_CELL* orxHashMap_FindKey(orxHASHMAP _pstHashMap, orxU32 _u32Key)
{
    orxHASHMAP_ROW* pstRow = orxHashMap_FindRow(_pstHashMap, _u32Key);
    return pstRow!=0?&pstRow->stCell:0;
}

/* Set an item value. */
orxVOID orxHashMap_SetKey(orxHASHMAP _pstHashMap, orxU32 _u32Key, orxVOID *_pValue)
{
    orxHASHMAP_ROW* pstRow = orxHashMap_FindRow(_pstHashMap, _u32Key);
    if (pstRow!=0)
        pstRow->stCell.pData = _pValue);
    else
    {
        pstRow = orxBank_Allocate(spstHashMapRowBank);
        pstRow->stCell.u32Key = _u32Key;
        pstRow->stCell.pData  = _pValue;
        pstRow->pstNext       = _pstHashMap->astRows[_u32Key%orxHASHMAP_INDEX_NUMBER];
        _pstHashMap->astRows[_u32Key%orxHASHMAP_INDEX_NUMBER] = pstRow;
    }
}

/* Remove an item. */
orxVOID orxHashMap_RemoveKey(orxHASHMAP _pstHashMap, orxU32 _u32Key)
{
    orxHASHMAP_ROW* pstRow = _pstHashMap->astRows[_u32Key%orxHASHMAP_INDEX_NUMBER];

    if (pstRow!=0)
    {
        if (pstRow->stCell.u32Key==_u32Key)
        {
            _pstHashMap->astRows[_u32Key%orxHASHMAP_INDEX_NUMBER] = pstRow->pstNext;
        }
        else
        {
            while (pstRow->pstNext!=0 && pstRow->pstNext->stCell.u32Key!=_u32Key)
                pstRow = pstRow->pstNext;
            if (pstRow->pstNext!=0 && pstRow->pstNext->stCell.u32Key==_u32Key)
            {
                orxHASHMAP_ROW* pstCurrent = pstRow;
                pstRow = pstRow->pstNext;
                pstCurrent->pstNext = pstRow->pstNext;
            }
        }
        if (pstRow!=0)
            orxBank_Free(spstHashMapRowBank, pstRow);
    }
}


