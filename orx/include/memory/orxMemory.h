/**
 * @file orxMemory.h
 * 
 * Memoray allocation / Unallocation module
 * 
 * @todo Optimizations
 * @todo Manage the memory instead of use the access to OS allocation
 * @todo manage memory alignment
 */
 
 /***************************************************************************
 orxMemory.h
 Memory allocation / unallocation module
 
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
 
#ifndef _orxMEMORY_H_
#define _orxMEMORY_H_
 
#include "orxInclude.h"
 
typedef enum __orxMEMORY_TYPE_t
{
  orxMEMORY_TYPE_MAIN = 0,
  
  orxMEMORY_TYPE_NUMBER,
  
  orxMEMORY_TYPE_NONE = orxENUM_NONE

} orxMEMORY_TYPE;


/** Initialize memory allocation module
 * @todo Really initialize the memory to be managed by the module and not OS
 */
extern orxDLLAPI orxSTATUS orxMemory_Init();

/** Uninitialize memory allocation module
 */
extern orxDLLAPI orxVOID orxMemory_Exit();

/** Allocate a portion of memory in the system and returns a pointer on it
 * @param _u32Size  (IN)  size of the memory to allocate
 * @param _eMemType (IN)  Memory zone where datas will be allocated
 * @return  returns a pointer on the memory allocated, or orxNULL if an error has occured
 * @todo Use the memory managed by orxMemory (initialized with orxMemory_Init())
 */
extern orxDLLAPI orxVOID *orxMemory_Allocate(orxU32 _u32Size, orxMEMORY_TYPE _eMemType);

/** Free a portion of memory allocated with orxMemory_Allocate
 * @param _pMem     (IN)  Pointer on the memory allocated by orx
 * @todo Use the memory managed by orxMemory (not OS)
 */
extern orxDLLAPI orxVOID orxMemory_Free(orxVOID *_pMem);

/** Get the an aligned data size
 * @param _u32OriginalValue (ex: 70)
 * @param _u32AlignValue (The value has to be a power of 2 and > 0) (ex : 32)
 * @return the aligned _u32OriginalValue on _u32AlignValue (ex : will return 96 for previous values)
 */
extern orxDLLAPI orxU32 orxMemory_GetAlign(orxU32 _u32OriginalValue, orxU32 _u32AlignValue);

/** Copy a portion of memory into another one
 * @param _pDest    (OUT) Destination pointer
 * @param _pSrc     (IN)  Pointer of memory from where data are read
 * @param _u32Size  (IN)  Size of data
 * @return returns a pointer on _pDest
 * @note if _pSrc and _pDest overlap, use orxMemory_Move instead
 */
extern orxDLLAPI orxVOID *orxMemory_Copy(orxVOID *_pDest, orxCONST orxVOID *_pSrc, orxU32 _u32Size);

/** Copy a portion of memory into another one
 * @param _pDest    (OUT) Destination pointer
 * @param _pSrc     (IN)  Pointer of memory from where data are read
 * @param _u32Size  (IN)  Size of data
 * @return returns a pointer on _pDest
 */
extern orxDLLAPI orxVOID *orxMemory_Move(orxVOID *_pDest, orxVOID *_pSrc, orxU32 _u32Size);

/** Compare two portion of memory
 * @param _pMem1    (IN)  First potion to test
 * @param _pMem2    (IN)  Second portion to test
 * @param _u32Size  (IN)  Size of data to test
 * @return returns a velue less, equals or greater that 0 if _pMem1 is respectively smaller, equal or greater than _pMem2
 */
extern orxDLLAPI orxU32 orxMemory_Compare(orxCONST orxVOID *_pMem1, orxCONST orxVOID *_pMem2, orxU32 _u32Size);

/** Fill a portion of memory with _u32Data
 * @param _pDest    (OUT) Destination pointer
 * @param _u8Data   (IN)  Values of the data that will fill the memory
 * @param _u32Size  (IN)  Size of data
 * @return returns a pointer on _pDest
 */
extern orxDLLAPI orxVOID *orxMemory_Set(orxVOID *_pDest, orxU8 _u8Data, orxU32 _u32Size);

#endif /* _orxMEMORY_H_ */
