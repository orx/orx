/* Orx - Portable Game Engine
 *
 * Orx is the legal property of its developers, whose names
 * are listed in the COPYRIGHT file distributed 
 * with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

/**
 * @file orxBank.h
 * @date 02/04/2005
 * @author bestel@arcallians.org
 *
 * @todo
 * - Optimizations 
 */

/**
 * @addtogroup Memory
 * 
 * Bank module
 * Bank are used to allocate a portion of memory.
 * Applications can get cell from this memory and use it. If the number of
 * allocations requests become bigger than the bank size, a new segment of memory
 * is automatically allocated.
 * Memory bank can be used to try to reduce memory fragmentation.
 *
 * @{
 */

 
#ifndef _orxBANK_H_
#define _orxBANK_H_

 
#include "orxInclude.h"
#include "memory/orxMemory.h"

 
/* Internal Bank structure */
typedef struct __orxBANK_t orxBANK;

/* Define flags */
#define orxBANK_KU32_FLAG_NONE              0x00000000  /**< No flags (default behaviour) */
#define orxBANK_KU32_FLAG_NOT_EXPANDABLE    0x00000001  /**< The bank will not be expandable */

/** Bank module setup
 */
extern orxDLLAPI orxVOID                    orxBank_Setup();

/** Initialize Bank Module
 */
extern orxDLLAPI orxSTATUS                  orxBank_Init();

/** Exit bank module
 */
extern orxDLLAPI orxVOID                    orxBank_Exit();

/** Create a new bank in memory and returns a pointer on it
 * @param[in] _u16NbElem  Number of elements per segments
 * @param[in] _u32Size    Size of an element
 * @param[in] _u32Flags   Flags set for this bank
 * @param[in] _eMemType   Memory type where the datas will be allocated
 * @return  returns a pointer on the memory bank
 */
extern orxDLLAPI orxBANK *orxFASTCALL       orxBank_Create(orxU16 _u16NbElem, orxU32 _u32Size, orxU32 _u32Flags, orxMEMORY_TYPE _eMemType);

/** Free a portion of memory allocated with orxMemory_Allocate
 * @param[in] _pstBank    Pointer on the memory bank allocated by orx
 */
extern orxDLLAPI orxVOID orxFASTCALL        orxBank_Delete(orxBANK *_pstBank);

/** Allocate a new cell from the bank
 * @param[in] _pstBank    Pointer on the memory bank to use
 * @return a new cell of memory (orxNULL if no allocation possible)
 */
extern orxDLLAPI orxVOID *orxFASTCALL       orxBank_Allocate(orxBANK *_pstBank);

/** Free an allocated cell
 * @param[in] _pstBank    Bank of memory from where _pCell has been allocated
 * @param[in] _pCell      Pointer on the cell to free
 * @return a new cell of memory (orxNULL if no allocation possible)
 */
extern orxDLLAPI orxVOID orxFASTCALL        orxBank_Free(orxBANK *_pstBank, orxVOID *_pCell);

/** Free all allocated cell from a bank
 * @param[in] _pstBank    Bank of memory to clear
 */
extern orxDLLAPI orxVOID orxFASTCALL        orxBank_Clear(orxBANK *_pstBank);

/** Get the next cell
 * @param[in] _pstBank    Bank of memory from where _pCell has been allocated
 * @param[in] _pCell      Pointer on the current cell of memory
 * @return The next cell. If _pCell is orxNULL, the first cell will be returned. Returns orxNULL when no more cell can be returned.
 */
extern orxDLLAPI orxVOID *orxFASTCALL       orxBank_GetNext(orxCONST orxBANK *_pstBank, orxCONST orxVOID *_pCell);

/*******************************************************************************
 * DEBUG FUNCTION
 ******************************************************************************/

/** Print the content of a chunk bank
 * @param[in] _pstBank    Bank's pointer
 */
extern orxDLLAPI orxVOID orxFASTCALL        orxBank_DebugPrint(orxCONST orxBANK *_pstBank);

#endif /* _orxBANK_H_ */

/** @} */
