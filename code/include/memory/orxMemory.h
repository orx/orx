/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2013 Orx-Project
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
 * @file orxMemory.h
 * @date 02/04/2005
 * @author bestel@arcallians.org
 *
 * @todo
 * - Manage memory alignment
 * - Allow to specify memory starting address & maximum size for each memory type
 */

/**
 * @addtogroup orxMemory
 *
 * Memory module
 * Module that handles all low level allocation/free requests
 *
 * @{
 */


#ifndef _orxMEMORY_H_
#define _orxMEMORY_H_


#include "orxInclude.h"


typedef enum __orxMEMORY_TYPE_t
{
  orxMEMORY_TYPE_MAIN = 0,              /**< Main memory type */

  orxMEMORY_TYPE_VIDEO,                 /**< Video memory type */

  orxMEMORY_TYPE_CONFIG,                /**< Config memory */
  orxMEMORY_TYPE_TEXT,                  /**< Text memory */

  orxMEMORY_TYPE_AUDIO,                 /**< Audio memory type */

  orxMEMORY_TYPE_PHYSICS,               /**< Physics memory type */

  orxMEMORY_TYPE_TEMP,                  /**< Temporary / scratch memory */

  orxMEMORY_TYPE_NUMBER,                /**< Number of memory type */

  orxMEMORY_TYPE_NONE = orxENUM_NONE    /**< Invalid memory type */

} orxMEMORY_TYPE;


/** Setups the memory module
 */
extern orxDLLAPI void orxFASTCALL       orxMemory_Setup();

/** Inits the memory module
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL  orxMemory_Init();

/** Exits from the memory module
 */
extern orxDLLAPI void orxFASTCALL       orxMemory_Exit();

/** Allocates a portion of memory in the system and returns a pointer on it
 * @param[in]  _u32Size  Size of the memory to allocate
 * @param[in]  _eMemType Memory zone where datas will be allocated
 * @return  returns a pointer on the memory allocated, or orxNULL if an error has occured
 */
extern orxDLLAPI void *orxFASTCALL      orxMemory_Allocate(orxU32 _u32Size, orxMEMORY_TYPE _eMemType);

/** Frees a portion of memory allocated with orxMemory_Allocate
 * @param[in]  _pMem     Pointer on the memory allocated by orx
 */
extern orxDLLAPI void orxFASTCALL       orxMemory_Free(void *_pMem);

/** Copies a portion of memory into another one
 * @param[out] _pDest    Destination pointer
 * @param[in]  _pSrc     Pointer of memory from where data are read
 * @param[in]  _u32Size  Size of data
 * @return returns a pointer on _pDest
 * @note if _pSrc and _pDest overlap, use orxMemory_Move instead
 */
extern orxDLLAPI void *orxFASTCALL      orxMemory_Copy(void *_pDest, const void *_pSrc, orxU32 _u32Size);

/** Moves a portion of memory into another one
 * @param[out] _pDest   Destination pointer
 * @param[in]  _pSrc    Pointer of memory from where data are read
 * @param[in]  _u32Size Size of data
 * @return returns a pointer on _pDest
 */
extern orxDLLAPI void *orxFASTCALL      orxMemory_Move(void *_pDest, void *_pSrc, orxU32 _u32Size);

/** Compares two portions of memory
 * @param[in]  _pMem1   First potion to test
 * @param[in]  _pMem2   Second portion to test
 * @param[in]  _u32Size Size of data to test
 * @return returns a value less, equals or greater that 0 if _pMem1 is respectively smaller, equal or greater than _pMem2
 */
extern orxDLLAPI orxU32 orxFASTCALL     orxMemory_Compare(const void *_pMem1, const void *_pMem2, orxU32 _u32Size);

/** Fills a portion of memory with _u32Data
 * @param[out] _pDest   Destination pointer
 * @param[in]  _u8Data  Values of the data that will fill the memory
 * @param[in]  _u32Size Size of data
 * @return returns a pointer on _pDest
 */
extern orxDLLAPI void *orxFASTCALL      orxMemory_Set(void *_pDest, orxU8 _u8Data, orxU32 _u32Size);

/** Fills a portion of memory with zeroes
 * @param[out] _pDest   Destination pointer
 * @param[in]  _u32Size Size of data
 * @return returns a pointer on _pDest
 */
extern orxDLLAPI void *orxFASTCALL      orxMemory_Zero(void *_pDest, orxU32 _u32Size);

/** Reallocates a portion of memory if the already allocated memory is not suffisant.
 * @param[in] _pMem    Memory to reallocate.
 * @param[in] _u32Size Wanted size.
 * @return The pointer reallocated.
 */
extern orxDLLAPI void *orxFASTCALL      orxMemory_Reallocate(void *_pMem, orxU32 _u32Size);

#ifdef __orxPROFILER__

/** Gets memory usage for a given type
 * @param[in] _eMemType               Concerned memory type
 * @param[out] _pu32Counter           Current memory allocation counter
 * @param[out] _pu32PeakCounter       Peak memory allocation counter
 * @param[out] _pu32Size              Current memory allocation size
 * @param[out] _pu32PeakSize          Peak memory allocation size
 * @param[out] _pu32OperationCounter  Total number of memory operations (malloc/free)
 * @return The pointer reallocated.
 */
extern orxDLLAPI orxSTATUS orxFASTCALL  orxMemory_GetUsage(orxMEMORY_TYPE _eMemType, orxU32 *_pu32Counter, orxU32 *_pu32PeakCounter, orxU32 *_pu32Size, orxU32 *_pu32PeakSize, orxU32 *_pu32OperationCounter);

#endif /* __orxPROFILER__  */

#endif /* _orxMEMORY_H_ */

/** @} */
