/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2022 Orx-Project
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
 * @file orxMemory.c
 * @date 02/04/2005
 * @author iarwain@orx-project.org
 *
 */


#include "memory/orxMemory.h"
#include "debug/orxDebug.h"


#ifdef __orxLLVM__
  #if defined(__orxMAC__) || defined(__orxIOS__)
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wunknown-attributes"
  #endif /* __orxMAC__ || __orxIOS__ */

  #if defined(__orxANDROID__) || defined(__orxANDROID_NATIVE__) || defined(__orxIOS__)
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wstatic-in-inline"
  #endif /* __orxANDROID__ || __orxANDROID_NATIVE__ || __orxIOS__ */
#endif /* __orxLLVM__ */

#ifdef __orxIOS__
  #define ENABLE_PRELOAD 1
#endif /* __orxIOS__ */

#include "rpmalloc.c"

#ifdef __orxIOS__
  #undef ENABLE_PRELOAD
#endif /* __orxIOS__ */

#ifdef __orxLLVM__
  #if defined(__orxANDROID__) || defined(__orxANDROID_NATIVE__) || defined(__orxIOS__)
    #pragma clang diagnostic pop
  #endif /* __orxANDROID__ || __orxANDROID_NATIVE__ || __orxIOS__ */

  #if defined(__orxMAC__) || defined(__orxIOS__)
    #pragma clang diagnostic pop
  #endif /* __orxMAC__ || __orxIOS__ */
#endif /* __orxLLVM__ */

#define orxMEMORY_KU32_STATIC_FLAG_NONE         0x00000000  /**< No flags have been set */
#define orxMEMORY_KU32_STATIC_FLAG_READY        0x00000001  /**< The module has been initialized */

#define orxMEMORY_KU32_DEFAULT_CACHE_LINE_SIZE  64
#define orxMEMORY_KU32_TAG_SIZE                 16

#define orxMEMORY_KZ_LITERAL_PREFIX             "MEM_"


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

typedef struct __orxMEMORY_TRACKER_t
{
  orxU64 u64Count, u64PeakCount;
  orxU64 u64Size, u64PeakSize;
  orxU64 u64OperationCount;

} orxMEMORY_TRACKER;

typedef struct __orxMEMORY_STATIC_t
{
#ifdef __orxPROFILER__

  orxMEMORY_TRACKER astMemoryTrackerList[orxMEMORY_TYPE_NUMBER];

#endif /* __orxPROFILER__ */

  orxU32 u32Flags;   /**< Flags set by the memory module */

} orxMEMORY_STATIC;


/***************************************************************************
 * Module global variable                                                  *
 ***************************************************************************/

static orxMEMORY_STATIC sstMemory;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

#if defined(__orxWINDOWS__)

  #ifdef __orxMSVC__

static orxINLINE orxU32 orxMemory_CacheLineSize()
{
  SYSTEM_LOGICAL_PROCESSOR_INFORMATION *astProcessorInfoList;
  orxU32                                u32InfoListSize = 0, u32Result = orxMEMORY_KU32_DEFAULT_CACHE_LINE_SIZE, i, u32Number;

  /* Requests total size of processors info */
  GetLogicalProcessorInformation(0, (PDWORD)&u32InfoListSize);

  /* Allocates info list */
  astProcessorInfoList = (SYSTEM_LOGICAL_PROCESSOR_INFORMATION *)orxMemory_Allocate(u32InfoListSize, orxMEMORY_TYPE_TEMP);

  /* Gets processors info */
  GetLogicalProcessorInformation(astProcessorInfoList, (PDWORD)&u32InfoListSize);

  /* For all processor info */
  for(i = 0, u32Number = u32InfoListSize / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
      i < u32Number;
      i++)
  {
    /* Found first level cache info? */
    if((astProcessorInfoList[i].Relationship == RelationCache)
    && (astProcessorInfoList[i].Cache.Level == 1))
    {
      /* Updates result */
      u32Result = astProcessorInfoList[i].Cache.LineSize;

      break;
    }
  }

  /* Frees info list */
  orxMemory_Free(astProcessorInfoList);

  /* Done! */
  return u32Result;
}

  #else /* __orxMSVC__ */

static orxINLINE orxU32 orxMemory_CacheLineSize()
{
  /* Done! */
  return orxMEMORY_KU32_DEFAULT_CACHE_LINE_SIZE;
}

  #endif /* __orxMSVC__ */

#elif defined(__orxMAC__) || defined(__orxIOS__)

#include <sys/sysctl.h>

static orxINLINE orxU32 orxMemory_CacheLineSize()
{
  size_t usLineSize = 0, usSizeOfLineSize;

  /* Gets size of variable */
  usSizeOfLineSize = sizeof(usLineSize);

  /* Gets cache line size */
  sysctlbyname("hw.cachelinesize", &usLineSize, &usSizeOfLineSize, 0, 0);

  /* Done! */
  return (orxU32)((usLineSize != 0) ? usLineSize : orxMEMORY_KU32_DEFAULT_CACHE_LINE_SIZE);
}

#elif defined(__orxLINUX__)

#include <unistd.h>

static orxINLINE orxU32 orxMemory_CacheLineSize()
{
  /* Done! */
  return (orxU32)sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
}

#elif defined(__orxANDROID__)

static orxINLINE orxU32 orxMemory_CacheLineSize()
{
  /* Done! */
  return orxMEMORY_KU32_DEFAULT_CACHE_LINE_SIZE;
}

#else

static orxINLINE orxU32 orxMemory_CacheLineSize()
{
  /* Done! */
  return orxMEMORY_KU32_DEFAULT_CACHE_LINE_SIZE;
}

#endif


/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** Memory module setup
 */
void orxFASTCALL orxMemory_Setup()
{
  /* Adds module dependencies */

  /* Done! */
  return;
}

/** Initializes memory allocation module
 */
orxSTATUS orxFASTCALL orxMemory_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Not already Initialized? */
  if(!(sstMemory.u32Flags & orxMEMORY_KU32_STATIC_FLAG_READY))
  {
    /* Cleans static controller */
    orxMemory_Zero(&sstMemory, sizeof(orxMEMORY_STATIC));

    /* Initializes rpmalloc */
    if(rpmalloc_initialize() == 0)
    {
      /* Module initialized */
      sstMemory.u32Flags = orxMEMORY_KU32_STATIC_FLAG_READY;

      /* Success */
      eResult = orxSTATUS_SUCCESS;
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_MEMORY, "Failed to initialize rpmalloc, aborting.");

      /* Already initialized */
      eResult = orxSTATUS_SUCCESS;
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_MEMORY, "Tried to initialize memory module when it was already initialized.");

    /* Already initialized */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Done */
  return eResult;
}

/** Exits from the memory module
 */
void orxFASTCALL orxMemory_Exit()
{
  /* Module initialized ? */
  if((sstMemory.u32Flags & orxMEMORY_KU32_STATIC_FLAG_READY) == orxMEMORY_KU32_STATIC_FLAG_READY)
  {
    /* Finalizes rpmalloc */
    rpmalloc_finalize();

    /* Module uninitialized */
    sstMemory.u32Flags = orxMEMORY_KU32_STATIC_FLAG_NONE;
  }

  return;
}

/** Allocates some memory in the system and returns a pointer to it
 * @param[in] _u32Size    size of the memory to allocate
 * @param[in] _eMemType   Memory zone where data will be allocated
 * @return  returns a pointer to the memory allocated, or orxNULL if an error has occurred
 */
void *orxFASTCALL orxMemory_Allocate(orxU32 _u32Size, orxMEMORY_TYPE _eMemType)
{
  void *pResult;

  /* Checks */
  orxASSERT((sstMemory.u32Flags & orxMEMORY_KU32_STATIC_FLAG_READY) == orxMEMORY_KU32_STATIC_FLAG_READY);
  orxASSERT(_eMemType < orxMEMORY_TYPE_NUMBER);

#ifdef __orxPROFILER__

  /* Allocates memory */
  pResult = rpmalloc((size_t)(_u32Size + orxMEMORY_KU32_TAG_SIZE));

  /* Success? */
  if(pResult != NULL)
  {
    size_t uMemoryChunkSize;

    /* Tags memory chunk */
    *(orxMEMORY_TYPE *)pResult = _eMemType;

    /* Gets memory chunk size */
    uMemoryChunkSize = rpmalloc_usable_size(pResult);

    /* Updates memory tracker */
    orxMemory_Track(_eMemType, (orxU32)(uMemoryChunkSize - orxMEMORY_KU32_TAG_SIZE), orxTRUE);

    /* Updates result */
    pResult = (orxU8 *)pResult + orxMEMORY_KU32_TAG_SIZE;
  }

#else /* __orxPROFILER__ */

  /* Allocates memory */
  pResult = rpmalloc((size_t)_u32Size);

#endif /* __orxPROFILER__ */

  /* Done! */
  return pResult;
}

/** Reallocates a previously allocated memory block, with the given new size and returns a pointer to it
 * If possible, it'll keep the current pointer and extend the memory block, if not it'll allocate a new block,
 * copy the data over and deallocates the original block
 * @param[in]  _pMem      Memory block to reallocate
 * @param[in]  _u32Size   Size of the memory to allocate
 * @param[in]  _eMemType  Memory zone where data will be allocated
 * @return  returns a pointer to the reallocated memory block or orxNULL if an error has occurred
 */
void *orxFASTCALL orxMemory_Reallocate(void *_pMem, orxU32 _u32Size, orxMEMORY_TYPE _eMemType)
{
  void *pResult;

  /* Checks */
  orxASSERT((sstMemory.u32Flags & orxMEMORY_KU32_STATIC_FLAG_READY) == orxMEMORY_KU32_STATIC_FLAG_READY);

  /* Null? */
  if(_pMem == orxNULL)
  {
    /* Allocates it */
    pResult = orxMemory_Allocate(_u32Size, _eMemType);
  }
  else
  {

#ifdef __orxPROFILER__

    orxMEMORY_TYPE eMemType;
    size_t         uMemoryChunkSize;

    /* Updates pointer */
    _pMem = (orxU8 *)_pMem - orxMEMORY_KU32_TAG_SIZE;

    /* Gets current memory type from memory chunk tag */
    eMemType = *(orxMEMORY_TYPE *)_pMem;

    /* Gets memory chunk size */
    uMemoryChunkSize = rpmalloc_usable_size(_pMem);

    /* Reallocates memory */
    pResult = rprealloc(_pMem, (size_t)(_u32Size + orxMEMORY_KU32_TAG_SIZE));

    /* Success? */
    if(pResult != NULL)
    {
      /* Tags memory chunk */
      *(orxMEMORY_TYPE *)pResult = _eMemType;

      /* Updates memory tracker */
      orxMemory_Track(eMemType, (orxU32)(uMemoryChunkSize - orxMEMORY_KU32_TAG_SIZE), orxFALSE);
      orxMemory_Track(_eMemType, (orxU32)(rpmalloc_usable_size(pResult) - orxMEMORY_KU32_TAG_SIZE), orxTRUE);

      /* Updates result */
      pResult = (orxU8 *)pResult + orxMEMORY_KU32_TAG_SIZE;
    }

#else /* __orxPROFILER__ */

    /* Reallocates memory */
    pResult = rprealloc(_pMem, (size_t)_u32Size);

#endif /* __orxPROFILER__ */
  }

  /* Done! */
  return pResult;
}

/** Frees some memory allocated with orxMemory_Allocate
 * @param[in] _pMem       Pointer to the memory allocated by orx
 */
void orxFASTCALL orxMemory_Free(void *_pMem)
{
  /* Checks */
  orxASSERT((sstMemory.u32Flags & orxMEMORY_KU32_STATIC_FLAG_READY) == orxMEMORY_KU32_STATIC_FLAG_READY);

#ifdef __orxPROFILER__

  /* Valid? */
  if(_pMem != NULL)
  {
    orxMEMORY_TYPE eMemType;
    size_t         uMemoryChunkSize;

    /* Updates pointer */
    _pMem = (orxU8 *)_pMem - orxMEMORY_KU32_TAG_SIZE;

    /* Gets memory type from memory chunk tag */
    eMemType = *(orxMEMORY_TYPE *)_pMem;

    /* Gets memory chunk size */
    uMemoryChunkSize = rpmalloc_usable_size(_pMem);

    /* Updates memory tracker */
    orxMemory_Track(eMemType, (orxU32)(uMemoryChunkSize - orxMEMORY_KU32_TAG_SIZE), orxFALSE);
  }

#endif /* __orxPROFILER__ */

  /* System call to free memory */
  rpfree(_pMem);

  return;
}

/** Gets memory type literal name
 * @param[in] _eMemType               Concerned memory type
 * @return Memory type name
 */
const orxSTRING orxFASTCALL orxMemory_GetTypeName(orxMEMORY_TYPE _eMemType)
{
  const orxSTRING zResult;

#define orxMEMORY_DECLARE_TYPE_NAME(TYPE) case orxMEMORY_TYPE_##TYPE: zResult = orxMEMORY_KZ_LITERAL_PREFIX#TYPE; break

  /* Checks */
  orxASSERT((sstMemory.u32Flags & orxMEMORY_KU32_STATIC_FLAG_READY) == orxMEMORY_KU32_STATIC_FLAG_READY);
  orxASSERT(_eMemType < orxMEMORY_TYPE_NUMBER);

  /* Depending on type */
  switch(_eMemType)
  {
    orxMEMORY_DECLARE_TYPE_NAME(MAIN);
    orxMEMORY_DECLARE_TYPE_NAME(VIDEO);
    orxMEMORY_DECLARE_TYPE_NAME(CONFIG);
    orxMEMORY_DECLARE_TYPE_NAME(TEXT);
    orxMEMORY_DECLARE_TYPE_NAME(AUDIO);
    orxMEMORY_DECLARE_TYPE_NAME(PHYSICS);
    orxMEMORY_DECLARE_TYPE_NAME(SYSTEM);
    orxMEMORY_DECLARE_TYPE_NAME(TEMP);
    orxMEMORY_DECLARE_TYPE_NAME(DEBUG);
    default:
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_MEMORY, "No name defined for memory type #%d.", _eMemType);

      /* Updates result */
      zResult = orxSTRING_EMPTY;
    }
  }

  /* Done! */
  return zResult;
}

/** Gets L1 data cache line size
 * @return Cache line size
 */
orxU32 orxFASTCALL orxMemory_GetCacheLineSize()
{
  orxU32 u32Result;

  /* Checks */
  orxASSERT((sstMemory.u32Flags & orxMEMORY_KU32_STATIC_FLAG_READY) == orxMEMORY_KU32_STATIC_FLAG_READY);

  /* Updates result */
  u32Result = orxMemory_CacheLineSize();

  /* Done! */
  return u32Result;
}

#ifdef __orxPROFILER__

/** Gets memory usage for a given type
 * @param[in] _eMemType         Concerned memory type
 * @param[out] _pu64Count       Current memory allocation count
 * @param[out] _pu64PeakCount   Peak memory allocation count
 * @param[out] _pu64Size        Current memory allocation size
 * @param[out] _pu64PeakSize    Peak memory allocation size
 * @param[out] _pu64OperationCount Total number of memory operations (malloc/free)
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxMemory_GetUsage(orxMEMORY_TYPE _eMemType, orxU64 *_pu64Count, orxU64 *_pu64PeakCount, orxU64 *_pu64Size, orxU64 *_pu64PeakSize, orxU64 *_pu64OperationCount)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstMemory.u32Flags & orxMEMORY_KU32_STATIC_FLAG_READY) == orxMEMORY_KU32_STATIC_FLAG_READY);

  /* Valid? */
  if(_eMemType < orxMEMORY_TYPE_NUMBER)
  {
    /* Asked for current count? */
    if(_pu64Count != orxNULL)
    {
      /* Updates it */
      *_pu64Count = sstMemory.astMemoryTrackerList[_eMemType].u64Count;
    }

    /* Asked for peak count? */
    if(_pu64PeakCount != orxNULL)
    {
      /* Updates it */
      *_pu64PeakCount = sstMemory.astMemoryTrackerList[_eMemType].u64PeakCount;
    }

    /* Asked for current size? */
    if(_pu64Size != orxNULL)
    {
      /* Updates it */
      *_pu64Size = sstMemory.astMemoryTrackerList[_eMemType].u64Size;
    }

    /* Asked for peak size? */
    if(_pu64PeakSize != orxNULL)
    {
      /* Updates it */
      *_pu64PeakSize = sstMemory.astMemoryTrackerList[_eMemType].u64PeakSize;
    }

    /* Asked for total operation count? */
    if(_pu64OperationCount != orxNULL)
    {
      /* Updates it */
      *_pu64OperationCount = sstMemory.astMemoryTrackerList[_eMemType].u64OperationCount;
    }
  }
  else
  {
    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Tracks (external) memory allocation
 * @param[in] _eMemType               Concerned memory type
 * @param[in] _u32Size                Size to track, in bytes
 * @param[in] _bAllocate              orxTRUE if allocate, orxFALSE if free
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxMemory_Track(orxMEMORY_TYPE _eMemType, orxU32 _u32Size, orxBOOL _bAllocate)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstMemory.u32Flags & orxMEMORY_KU32_STATIC_FLAG_READY) == orxMEMORY_KU32_STATIC_FLAG_READY);

  /* Valid? */
  if(_eMemType < orxMEMORY_TYPE_NUMBER)
  {
    /* Allocate? */
    if(_bAllocate != orxFALSE)
    {
      /* Updates counts */
      sstMemory.astMemoryTrackerList[_eMemType].u64Size += (orxU64)_u32Size;
      sstMemory.astMemoryTrackerList[_eMemType].u64Count++;
      if(sstMemory.astMemoryTrackerList[_eMemType].u64Count > sstMemory.astMemoryTrackerList[_eMemType].u64PeakCount)
      {
        sstMemory.astMemoryTrackerList[_eMemType].u64PeakCount = sstMemory.astMemoryTrackerList[_eMemType].u64Count;
      }
      if(sstMemory.astMemoryTrackerList[_eMemType].u64Size > sstMemory.astMemoryTrackerList[_eMemType].u64PeakSize)
      {
        sstMemory.astMemoryTrackerList[_eMemType].u64PeakSize = sstMemory.astMemoryTrackerList[_eMemType].u64Size;
      }
    }
    else
    {
      /* Updates counts */
      sstMemory.astMemoryTrackerList[_eMemType].u64Size -= (orxU64)_u32Size;
      sstMemory.astMemoryTrackerList[_eMemType].u64Count--;
    }

    /* Updates operation count */
    sstMemory.astMemoryTrackerList[_eMemType].u64OperationCount++;
  }
  else
  {
    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

#endif /* __orxPROFILER__ */
