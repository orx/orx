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
 * @file orxMemory.h
 * @date 02/04/2005
 * @author iarwain@orx-project.org
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
#include "debug/orxDebug.h"

#include <string.h>

/** Memory barrier macros */
#if defined(__orxGCC__) || defined(__orxLLVM__)
  #define orxMEMORY_BARRIER()                             __sync_synchronize()
  #define orxHAS_MEMORY_BARRIER
#elif defined(__orxMSVC__)
  #ifdef __orx64__
    #define orxMEMORY_BARRIER()                           __faststorefence()
#else /* __orx64__ */
    #define orxMEMORY_BARRIER()     \
    {                               \
      long lBarrier;                \
      _InterlockedOr(&lBarrier, 0); \
    }
#endif /* __orx64__ */
  #define orxHAS_MEMORY_BARRIER
#else
  #define orxMEMORY_BARRIER()
  #undef orxHAS_MEMORY_BARRIER

  #warning !!WARNING!! This compiler does not have any builtin hardware memory barrier.
#endif


/** Memory tracking macros */
#ifdef __orxPROFILER__
  #define orxMEMORY_TRACK(TYPE, SIZE, ALLOCATE)           orxMemory_Track(orxMEMORY_TYPE_##TYPE, SIZE, ALLOCATE)
#else /* __orxPROFILER__ */
  #define orxMEMORY_TRACK(TYPE, SIZE, ALLOCATE)
#endif /* __orxPROFILER__ */


/** Memory type
 */
typedef enum __orxMEMORY_TYPE_t
{
  orxMEMORY_TYPE_MAIN = 0,                                /**< Main memory type */
  orxMEMORY_TYPE_AUDIO,                                   /**< Audio memory type */
  orxMEMORY_TYPE_CONFIG,                                  /**< Config memory */
  orxMEMORY_TYPE_DEBUG,                                   /**< Debug memory */
  orxMEMORY_TYPE_PHYSICS,                                 /**< Physics memory type */
  orxMEMORY_TYPE_SYSTEM,                                  /**< System memory type */
  orxMEMORY_TYPE_TEMP,                                    /**< Temporary / scratch memory */
  orxMEMORY_TYPE_TEXT,                                    /**< Text memory */
  orxMEMORY_TYPE_VIDEO,                                   /**< Video memory type */

  orxMEMORY_TYPE_NUMBER,                                  /**< Number of memory type */

  orxMEMORY_TYPE_NONE = orxENUM_NONE                      /**< Invalid memory type */

} orxMEMORY_TYPE;


/** Setups the memory module
 */
extern orxDLLAPI void orxFASTCALL                         orxMemory_Setup();

/** Inits the memory module
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                    orxMemory_Init();

/** Exits from the memory module
 */
extern orxDLLAPI void orxFASTCALL                         orxMemory_Exit();

/** Allocates some memory in the system and returns a pointer to it
 * @param[in]  _u32Size  Size of the memory to allocate
 * @param[in]  _eMemType Memory zone where data will be allocated
 * @return  returns a pointer to the memory allocated, or orxNULL if an error has occurred
 */
extern orxDLLAPI void *orxFASTCALL                        orxMemory_Allocate(orxU32 _u32Size, orxMEMORY_TYPE _eMemType);

/** Reallocates a previously allocated memory block, with the given new size and returns a pointer to it
 * If possible, it'll keep the current pointer and extend the memory block, if not it'll allocate a new block,
 * copy the data over and deallocates the original block
 * @param[in]  _pMem      Memory block to reallocate
 * @param[in]  _u32Size   Size of the memory to allocate
 * @param[in]  _eMemType  Memory zone where data will be allocated
 * @return  returns a pointer to the reallocated memory block or orxNULL if an error has occurred
 */
extern orxDLLAPI void *orxFASTCALL                        orxMemory_Reallocate(void *_pMem, orxU32 _u32Size, orxMEMORY_TYPE _eMemType);

/** Frees some memory allocated with orxMemory_Allocate
 * @param[in]  _pMem     Pointer to the memory allocated by orx
 */
extern orxDLLAPI void orxFASTCALL                         orxMemory_Free(void *_pMem);


/** Copies a part of memory into another one
 * @param[out] _pDest    Destination pointer
 * @param[in]  _pSrc     Pointer of memory from where data are read
 * @param[in]  _u32Size  Size of data
 * @return returns a pointer to _pDest
 * @note if _pSrc and _pDest overlap, use orxMemory_Move instead
 */
static orxINLINE void *                                   orxMemory_Copy(void *_pDest, const void *_pSrc, orxU32 _u32Size)
{
  /* Checks */
  orxASSERT(_pDest != orxNULL);
  orxASSERT(_pSrc != orxNULL);

  /* Done! */
  return((void *)memcpy(_pDest, _pSrc, (size_t)_u32Size));
}

/** Moves a part of memory into another one
 * @param[out] _pDest   Destination pointer
 * @param[in]  _pSrc    Pointer of memory from where data are read
 * @param[in]  _u32Size Size of data
 * @return returns a pointer to _pDest
 */
static orxINLINE void *                                   orxMemory_Move(void *_pDest, void *_pSrc, orxU32 _u32Size)
{
  /* Checks */
  orxASSERT(_pDest != orxNULL);
  orxASSERT(_pSrc != orxNULL);

  /* Done! */
  return((void *)memmove(_pDest, _pSrc, (size_t)_u32Size));
}

/** Compares two parts of memory
 * @param[in]  _pMem1   First part to test
 * @param[in]  _pMem2   Second part to test
 * @param[in]  _u32Size Size of data to test
 * @return returns a value less than, equal to or greater than 0 if the content of _pMem1 is respectively smaller, equal or greater than _pMem2's
 */
static orxINLINE orxU32                                   orxMemory_Compare(const void *_pMem1, const void *_pMem2, orxU32 _u32Size)
{
  /* Checks */
  orxASSERT(_pMem1 != orxNULL);
  orxASSERT(_pMem2 != orxNULL);

  /* Done! */
  return((orxU32)memcmp(_pMem1, _pMem2, (size_t)_u32Size));
}

/** Fills a part of memory with _u32Data
 * @param[out] _pDest   Destination pointer
 * @param[in]  _u8Data  Values of the data that will fill the memory
 * @param[in]  _u32Size Size of data
 * @return returns a pointer to _pDest
 */
static orxINLINE void *                                   orxMemory_Set(void *_pDest, orxU8 _u8Data, orxU32 _u32Size)
{
  /* Checks */
  orxASSERT(_pDest != orxNULL);

  /* Done! */
  return((void *)memset(_pDest, _u8Data, (size_t)_u32Size));
}

/** Fills a part of memory with zeroes
 * @param[out] _pDest   Destination pointer
 * @param[in]  _u32Size Size of data
 * @return returns a pointer to _pDest
 */
static orxINLINE void *                                   orxMemory_Zero(void *_pDest, orxU32 _u32Size)
{
  /* Checks */
  orxASSERT(_pDest != orxNULL);

  /* Done! */
  return((void *)memset(_pDest, 0, (size_t)_u32Size));
}


/** Gets memory type literal name
 * @param[in] _eMemType               Concerned memory type
 * @return Memory type name / orxSTRING_EMPTY
 */
extern orxDLLAPI const orxSTRING orxFASTCALL              orxMemory_GetTypeName(orxMEMORY_TYPE _eMemType);


/** Gets L1 data cache line size
 * @return Cache line size
 */
extern orxDLLAPI orxU32 orxFASTCALL                       orxMemory_GetCacheLineSize();


#ifdef __orxPROFILER__

/** Gets memory usage for a given type
 * @param[in] _eMemType               Concerned memory type
 * @param[out] _pu32Count             Current memory allocation count
 * @param[out] _pu32PeakCount         Peak memory allocation count
 * @param[out] _pu32Size              Current memory allocation size
 * @param[out] _pu32PeakSize          Peak memory allocation size
 * @param[out] _pu32OperationCount    Total number of memory operations (malloc/free)
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                    orxMemory_GetUsage(orxMEMORY_TYPE _eMemType, orxU32 *_pu32Count, orxU32 *_pu32PeakCount, orxU32 *_pu32Size, orxU32 *_pu32PeakSize, orxU32 *_pu32OperationCount);


/** Tracks (external) memory allocation
 * @param[in] _eMemType               Concerned memory type
 * @param[in] _u32Size                Size to track, in bytes
 * @param[in] _bAllocate              orxTRUE if allocate, orxFALSE if free
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                    orxMemory_Track(orxMEMORY_TYPE _eMemType, orxU32 _u32Size, orxBOOL _bAllocate);

#endif /* __orxPROFILER__  */

#endif /* _orxMEMORY_H_ */

/** @} */
