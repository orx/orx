/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2021 Orx-Project
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
 * @file orxBank.h
 * @date 02/04/2005
 * @author iarwain@orx-project.org
 *
 * @todo
 */

/**
 * @addtogroup orxBank
 *
 * Bank module
 * Bank are used to allocate some memory.
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

/** Setups the bank module
 */
extern orxDLLAPI void orxFASTCALL           orxBank_Setup();

/** Inits the bank Module
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL      orxBank_Init();

/** Exits from the bank module
 */
extern orxDLLAPI void orxFASTCALL           orxBank_Exit();

/** Creates a memory bank
 * @param[in] _u32Count   Number of cells per segments
 * @param[in] _u32Size    Size of a cell
 * @param[in] _u32Flags   Bank flags
 * @param[in] _eMemType   Memory type
 * @return orxBANK / orxNULL
 */
extern orxDLLAPI orxBANK *orxFASTCALL       orxBank_Create(orxU32 _u32Count, orxU32 _u32Size, orxU32 _u32Flags, orxMEMORY_TYPE _eMemType);

/** Deletes a bank
 * @param[in] _pstBank    Concerned bank
 */
extern orxDLLAPI void orxFASTCALL           orxBank_Delete(orxBANK *_pstBank);

/** Allocates a new cell from the bank
 * @param[in] _pstBank    Concerned bank
 * @return a new cell of memory (orxNULL if no allocation possible)
 */
extern orxDLLAPI void *orxFASTCALL          orxBank_Allocate(orxBANK *_pstBank);

/** Allocates a new cell from the bank and returns its index
 * @param[in] _pstBank        Concerned bank
 * @param[out] _pu32ItemIndex Will be set with the allocated item index
 * @param[out] _ppPrevious    If non-null, will contain previous neighbor if found
 * @return a new cell of memory (orxNULL if no allocation possible)
 */
extern orxDLLAPI void *orxFASTCALL          orxBank_AllocateIndexed(orxBANK *_pstBank, orxU32 *_pu32ItemIndex, void **_ppPrevious);

/** Frees an allocated cell
 * @param[in] _pstBank    Concerned bank
 * @param[in] _pCell      Pointer to the cell to free
 */
extern orxDLLAPI void orxFASTCALL           orxBank_Free(orxBANK *_pstBank, void *_pCell);

/** Frees an allocated cell at a given index
 * @param[in] _pstBank    Concerned bank
 * @param[in] _u32Index   Index of the cell to free
 */
extern orxDLLAPI void orxFASTCALL           orxBank_FreeAtIndex(orxBANK *_pstBank, orxU32 _u32Index);

/** Frees all allocated cell from a bank
 * @param[in] _pstBank    Concerned bank
 */
extern orxDLLAPI void orxFASTCALL           orxBank_Clear(orxBANK *_pstBank);

/** Compacts a bank by removing all its trailing unused segments
 * @param[in] _pstBank    Concerned bank
 */
extern orxDLLAPI void orxFASTCALL           orxBank_Compact(orxBANK *_pstBank);

/** Compacts all banks by removing all their unused segments
 */
extern orxDLLAPI void orxFASTCALL           orxBank_CompactAll();

/** Gets the next cell
 * @param[in] _pstBank    Concerned bank
 * @param[in] _pCell      Pointer to the current cell of memory, orxNULL to get the first one
 * @return The next cell if found, orxNULL otherwise
 */
extern orxDLLAPI void *orxFASTCALL          orxBank_GetNext(const orxBANK *_pstBank, const void *_pCell);

/** Gets the cell's index
 * @param[in] _pstBank    Concerned memory bank
 * @param[in] _pCell      Cell of which we want the index
 * @return The index of the given cell
 */
extern orxDLLAPI orxU32 orxFASTCALL         orxBank_GetIndex(const orxBANK *_pstBank, const void *_pCell);

/** Gets the cell at given index, orxNULL is the cell isn't allocated
 * @param[in] _pstBank    Concerned memory bank
 * @param[in] _u32Index   Index of the cell to retrieve
 * @return The cell at the given index if allocated, orxNULL otherwise
 */
extern orxDLLAPI void *orxFASTCALL          orxBank_GetAtIndex(const orxBANK *_pstBank, orxU32 _u32Index);

/** Gets the bank allocated cell count
 * @param[in] _pstBank    Concerned bank
 * @return Number of allocated cells
 */
extern orxDLLAPI orxU32 orxFASTCALL         orxBank_GetCount(const orxBANK *_pstBank);

#endif /* _orxBANK_H_ */

/** @} */
