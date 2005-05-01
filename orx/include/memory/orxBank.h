/**
 * @file orxBank.h
 * 
 * Allocate bank of memory.
 * Bank are used to allocate a portion of memory.
 * Applications can get cell from this memory and use it. If the number of
 * allocations requests become bigger than the bank size, a new segment of memory
 *  is automatically allocated.
 * Memory bank can be used to try to reduce memory fragmentation
 * 
 * @todo
 * Optimizations
 */ 
 
 /***************************************************************************
 orxMemory.h
 Memory Bank allocation / unallocation module
 
 begin                : 02/04/2005
 author               : (C) Arcallians
 email                : bestel@arcallians.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
 
#ifndef _orxBANK_H_
#define _orxBANK_H_
 
#include "orxInclude.h"
#include "memory/orxMemory.h"
 
/* Internal Bank structure */
typedef struct __orxBANK_t orxBANK;

/* Define flags */
#define orxBANK_KU32_FLAGS_NONE            0x00000000  /**< No flags (default behaviour) */
#define orxBANK_KU32_FLAGS_NOT_EXPANDABLE  0x00000001  /**< The bank will not be expandable */

 
/** Initialize Bank Module
 */
extern orxDLLAPI orxSTATUS orxBank_Init();

/** Exit bank module
 */
extern orxDLLAPI orxVOID orxBank_Exit();

 
/** Create a new bank in memory and returns a pointer on it
 * @param _u32NbElem  (IN)  Number of elements per segments
 * @param _u32Size    (IN)  Size of an element
 * @param _u32Flags   (IN)  Flags set for this bank
 * @param _eMemType   (IN)  Memory type where the datas will be allocated
 * @return  returns a pointer on the memory bank
 */
extern orxDLLAPI orxBANK *orxBank_Create(orxU32 _u32NbElem, orxU32 _u32Size, orxU32 _u32Flags, orxMEMORY_TYPE _eMemType);

/** Free a portion of memory allocated with orxMemory_Allocate
 * @param _pstBank    (IN)  Pointer on the memory bank allocated by orx
 */
extern orxDLLAPI orxVOID orxBank_Delete(orxBANK *_pstBank);

/** Allocate a new cell from the bank
 * @param _pstBank    (IN) Pointer on the memory bank to use
 * @return a new cell of memory (orxNULL if no allocation possible)
 */
extern orxDLLAPI orxVOID *orxBank_Allocate(orxBANK *_pstBank);

/** Free an allocated cell
 * @param _pstBank  (IN)  Bank of memory from where _pCell has been allocated
 * @param _pCell    (IN)  Pointer on the cell to free
 * @return a new cell of memory (orxNULL if no allocation possible)
 */
extern orxDLLAPI orxVOID orxBank_Free(orxBANK *_pstBank, orxVOID *_pCell);

/** Get the next cell
 * @param _pstBank  (IN)  Bank of memory from where _pCell has been allocated
 * @param _pCell    (IN)  Pointer on the current cell of memory
 * @return The next cell. If _pCell is orxNULL, the first cell will be returned.
 * @return Returns orxNULL when no more cell can be returned.
 */
extern orxDLLAPI orxVOID *orxBank_GetNext(orxBANK *_pstBank, orxVOID *_pCell);

/*******************************************************************************
 * DEBUG FUNCTION
 ******************************************************************************/

/** Print the content of a chunk bank
 * @param _pstBank  (IN)  Bank's pointer
 */
extern orxDLLAPI orxVOID orxBank_DebugPrint(orxBANK *_pstBank);
 

#endif /* _orxBANK_H_ */
