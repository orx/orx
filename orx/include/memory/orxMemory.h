/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/**
 * @file
 * @date 02/04/2005
 * @author (C) Arcallians
 * 
 * @todo Optimizations
 * @todo Manage the memory instead of use the access to OS allocation
 * @todo manage memory alignment
 * @todo Allows to specifiy memory starting address & maximum size for each memory type
 */
 
/**
 * @addtogroup Memory
 * Memory allocation / Unallocation module
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


/** Memory module setup */
extern orxDLLAPI orxVOID                orxMemory_Setup();
/** Initialize memory allocation module
 * @todo Really initialize the memory to be managed by the module and not OS
 */
extern orxDLLAPI orxSTATUS              orxMemory_Init();
/** Uninitialize memory allocation module
 */
extern orxDLLAPI orxVOID                orxMemory_Exit();

/** Allocate a portion of memory in the system and returns a pointer on it
 * @param[in]  _u32Size  Size of the memory to allocate
 * @param[in]  _eMemType Memory zone where datas will be allocated
 * @return  returns a pointer on the memory allocated, or orxNULL if an error has occured
 * @todo Use the memory managed by orxMemory (initialized with orxMemory_Init())
 */
extern orxDLLAPI orxVOID *orxFASTCALL   orxMemory_Allocate(orxU32 _u32Size, orxMEMORY_TYPE _eMemType);

/** Free a portion of memory allocated with orxMemory_Allocate
 * @param[in]  _pMem     Pointer on the memory allocated by orx
 * @todo Use the memory managed by orxMemory (not OS)
 */
extern orxDLLAPI orxVOID orxFASTCALL    orxMemory_Free(orxVOID *_pMem);

/** Get the an aligned data size
 * @param[in]  _u32OriginalValue Original value (ex: 70)
 * @param[in]  _u32AlignValue    Align size (The value has to be a power of 2 and > 0) (ex : 32)
 * @return the aligned _u32OriginalValue on _u32AlignValue (ex : will return 96 for previous values)
 */
extern orxDLLAPI orxU32 orxFASTCALL     orxMemory_GetAlign(orxU32 _u32OriginalValue, orxU32 _u32AlignValue);

/** Copy a portion of memory into another one
 * @param[out] _pDest    Destination pointer
 * @param[in]  _pSrc     Pointer of memory from where data are read
 * @param[in]  _u32Size  Size of data
 * @return returns a pointer on _pDest
 * @note if _pSrc and _pDest overlap, use orxMemory_Move instead
 */
extern orxDLLAPI orxVOID *orxFASTCALL   orxMemory_Copy(orxVOID *_pDest, orxCONST orxVOID *_pSrc, orxU32 _u32Size);

/** Copy a portion of memory into another one
 * @param[out] _pDest   Destination pointer
 * @param[in]  _pSrc    Pointer of memory from where data are read
 * @param[in]  _u32Size Size of data
 * @return returns a pointer on _pDest
 */
extern orxDLLAPI orxVOID *orxFASTCALL   orxMemory_Move(orxVOID *_pDest, orxVOID *_pSrc, orxU32 _u32Size);

/** Compare two portion of memory
 * @param[in]  _pMem1   First potion to test
 * @param[in]  _pMem2   Second portion to test
 * @param[in]  _u32Size Size of data to test
 * @return returns a velue less, equals or greater that 0 if _pMem1 is respectively smaller, equal or greater than _pMem2
 */
extern orxDLLAPI orxU32 orxFASTCALL     orxMemory_Compare(orxCONST orxVOID *_pMem1, orxCONST orxVOID *_pMem2, orxU32 _u32Size);

/** Fill a portion of memory with _u32Data
 * @param[out] _pDest   Destination pointer
 * @param[in]  _u8Data  Values of the data that will fill the memory
 * @param[in]  _u32Size Size of data
 * @return returns a pointer on _pDest
 */
extern orxDLLAPI orxVOID *orxFASTCALL   orxMemory_Set(orxVOID *_pDest, orxU8 _u8Data, orxU32 _u32Size);

/** Realloc a portion of memory if the already allocated memory is not suffisant.
 * @param[in] _pMem	   Memory to reallocate.
 * @param[in] _u32Size Wanted size.
 * @return The pointer reallocated.
 */
extern orxDLLAPI orxVOID *orxFASTCALL   orxMemory_Reallocate(orxVOID *_pMem, orxU32 _u32Size);


#endif /* _orxMEMORY_H_ */

/** @} */
