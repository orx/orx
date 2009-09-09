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
 * @file orxMemory.h
 * @date 02/04/2005
 * @author bestel@arcallians.org
 *
 * @todo
 * - Manage memory alignment
 * - Allow to specifiy memory starting address & maximum size for each memory type
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
  orxMEMORY_TYPE_SPRITE,                /**< Sprite memory type */
  orxMEMORY_TYPE_BACKGROUND,            /**< Background memory type */
  orxMEMORY_TYPE_PALETTE,               /**< Palette memory type */

  orxMEMORY_TYPE_CONFIG,                /**< Config memory */
  orxMEMORY_TYPE_TEXT,                  /**< Text memory */

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

/** Gets the an aligned data size
 * @param[in]  _u32OriginalValue Original value (ex: 70)
 * @param[in]  _u32AlignValue    Align size (The value has to be a power of 2 and > 0) (ex : 32)
 * @return the aligned _u32OriginalValue on _u32AlignValue (ex : will return 96 for previous values)
 */
extern orxDLLAPI orxU32 orxFASTCALL     orxMemory_GetAlign(orxU32 _u32OriginalValue, orxU32 _u32AlignValue);

/** Copies a portion of memory into another one
 * @param[out] _pDest    Destination pointer
 * @param[in]  _pSrc     Pointer of memory from where data are read
 * @param[in]  _u32Size  Size of data
 * @return returns a pointer on _pDest
 * @note if _pSrc and _pDest overlap, use orxMemory_Move instead
 */
extern orxDLLAPI void *orxFASTCALL      orxMemory_Copy(void *_pDest, const void *_pSrc, orxU32 _u32Size);

/** Copies a portion of memory into another one
 * @param[out] _pDest   Destination pointer
 * @param[in]  _pSrc    Pointer of memory from where data are read
 * @param[in]  _u32Size Size of data
 * @return returns a pointer on _pDest
 */
extern orxDLLAPI void *orxFASTCALL      orxMemory_Move(void *_pDest, void *_pSrc, orxU32 _u32Size);

/** Compares two portion of memory
 * @param[in]  _pMem1   First potion to test
 * @param[in]  _pMem2   Second portion to test
 * @param[in]  _u32Size Size of data to test
 * @return returns a velue less, equals or greater that 0 if _pMem1 is respectively smaller, equal or greater than _pMem2
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

/** Reallocs a portion of memory if the already allocated memory is not suffisant.
 * @param[in] _pMem	   Memory to reallocate.
 * @param[in] _u32Size Wanted size.
 * @return The pointer reallocated.
 */
extern orxDLLAPI void *orxFASTCALL      orxMemory_Reallocate(void *_pMem, orxU32 _u32Size);

#endif /* _orxMEMORY_H_ */

/** @} */
