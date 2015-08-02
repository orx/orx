/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2015 Orx-Project
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
 * @author bestel@arcallians.org
 *
 */


#include "memory/orxMemory.h"
#include "debug/orxDebug.h"


#define USE_DL_PREFIX
#define USE_BUILTIN_FFS 1
#define USE_LOCKS 1
#undef _GNU_SOURCE

#if defined(__orxIOS__) && defined(__orxLLVM__)
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wshorten-64-to-32"
#endif /* __orxIOS__ && __orxLLVM__ */

#include "malloc.c"

#if defined(__orxIOS__) && defined(__orxLLVM__)
  #pragma clang diagnostic pop
#endif /* __orxIOS__ && __orxLLVM__ */

#define orxMEMORY_KU32_STATIC_FLAG_NONE         0x00000000  /**< No flags have been set */
#define orxMEMORY_KU32_STATIC_FLAG_READY        0x00000001  /**< The module has been initialized */

#define orxMEMORY_KU32_DEFAULT_CACHE_LINE_SIZE  8

#define orxMEMORY_KZ_LITERAL_PREFIX             "MEM_"


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

typedef struct __orxMEMORY_TRACKER_t
{
  orxU32 u32Counter, u32PeakCounter;
  orxU32 u32Size, u32PeakSize;
  orxU32 u32OperationCounter;

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
  size_t stLineSize = 0, stSizeOfLineSize;

  /* Size of variable */
  stSizeOfLineSize = sizeof(stLineSize);

  /* Gets cache line size */
  sysctlbyname("hw.cachelinesize", &stLineSize, &stSizeOfLineSize, 0, 0);

  /* Done! */
  return (orxU32)(stLineSize != 0) ? (orxU32)stLineSize : 32;
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
  return (orxU32)32;
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

    /* Module initialized */
    sstMemory.u32Flags = orxMEMORY_KU32_STATIC_FLAG_READY;

    /* Success */
    eResult = orxSTATUS_SUCCESS;
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
    /* Module uninitialized */
    sstMemory.u32Flags = orxMEMORY_KU32_STATIC_FLAG_NONE;
  }

  return;
}

/** Allocates a portion of memory in the system and returns a pointer on it
 * @param[in] _u32Size    size of the memory to allocate
 * @param[in] _eMemType   Memory zone where data will be allocated
 * @return  returns a pointer on the memory allocated, or orxNULL if an error has occurred
 */
void *orxFASTCALL orxMemory_Allocate(orxU32 _u32Size, orxMEMORY_TYPE _eMemType)
{
  void *pResult;

  /* Checks */
  orxASSERT((sstMemory.u32Flags & orxMEMORY_KU32_STATIC_FLAG_READY) == orxMEMORY_KU32_STATIC_FLAG_READY);
  orxASSERT(_eMemType < orxMEMORY_TYPE_NUMBER);

#ifdef __orxPROFILER__

  /* Allocates memory */
  pResult = dlmalloc((size_t)(_u32Size + sizeof(orxMEMORY_TYPE)));

  /* Success? */
  if(pResult != NULL)
  {
    size_t uMemoryChunkSize;

    /* Tags memory chunk */
    *(orxMEMORY_TYPE *)pResult = _eMemType;

    /* Gets memory chunk size */
    uMemoryChunkSize = dlmalloc_usable_size(pResult);

    /* Updates memory tracker */
    orxMemory_Track(_eMemType, (orxU32)(uMemoryChunkSize - sizeof(orxMEMORY_TYPE)), orxTRUE);

    /* Updates result */
    pResult = (orxU8 *)pResult + sizeof(orxMEMORY_TYPE);
  }

#else /* __orxPROFILER__ */

  /* Allocates memory */
  pResult = dlmalloc((size_t)_u32Size);

#endif /* __orxPROFILER__ */

  /* Done! */
  return pResult;
}

/** Frees a portion of memory allocated with orxMemory_Allocateate
 * @param[in] _pMem       Pointer on the memory allocated by orx
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
    _pMem = (orxU8 *)_pMem - sizeof(orxMEMORY_TYPE);

    /* Gets memory type from memory chunk tag */
    eMemType = *(orxMEMORY_TYPE *)_pMem;

    /* Gets memory chunk size */
    uMemoryChunkSize = dlmalloc_usable_size(_pMem);

    /* Updates memory tracker */
    orxMemory_Track(eMemType, (orxU32)(uMemoryChunkSize - sizeof(orxMEMORY_TYPE)), orxFALSE);
  }

#endif /* __orxPROFILER__ */

  /* System call to free memory */
  dlfree(_pMem);

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
 * @param[out] _pu32Counter     Current memory allocation counter
 * @param[out] _pu32PeakCounter Peak memory allocation counter
 * @param[out] _pu32Size        Current memory allocation size
 * @param[out] _pu32PeakSize    Peak memory allocation size
 * @param[out] _pu32OperationCounter  Total number of memory operations (malloc/free)
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxMemory_GetUsage(orxMEMORY_TYPE _eMemType, orxU32 *_pu32Counter, orxU32 *_pu32PeakCounter, orxU32 *_pu32Size, orxU32 *_pu32PeakSize, orxU32 *_pu32OperationCounter)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstMemory.u32Flags & orxMEMORY_KU32_STATIC_FLAG_READY) == orxMEMORY_KU32_STATIC_FLAG_READY);

  /* Valid? */
  if(_eMemType < orxMEMORY_TYPE_NUMBER)
  {
    /* Asked for current counter? */
    if(_pu32Counter != orxNULL)
    {
      /* Updates it */
      *_pu32Counter = sstMemory.astMemoryTrackerList[_eMemType].u32Counter;
    }

    /* Asked for peak counter? */
    if(_pu32PeakCounter != orxNULL)
    {
      /* Updates it */
      *_pu32PeakCounter = sstMemory.astMemoryTrackerList[_eMemType].u32PeakCounter;
    }

    /* Asked for current size? */
    if(_pu32Size != orxNULL)
    {
      /* Updates it */
      *_pu32Size = sstMemory.astMemoryTrackerList[_eMemType].u32Size;
    }

    /* Asked for peak size? */
    if(_pu32PeakSize != orxNULL)
    {
      /* Updates it */
      *_pu32PeakSize = sstMemory.astMemoryTrackerList[_eMemType].u32PeakSize;
    }

    /* Asked for total operation counter? */
    if(_pu32OperationCounter != orxNULL)
    {
      /* Updates it */
      *_pu32OperationCounter = sstMemory.astMemoryTrackerList[_eMemType].u32OperationCounter;
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
 * @param[in] _s32Size                Size to track, in bytes
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
      /* Updates counters */
      sstMemory.astMemoryTrackerList[_eMemType].u32Size += _u32Size;
      sstMemory.astMemoryTrackerList[_eMemType].u32Counter++;
      if(sstMemory.astMemoryTrackerList[_eMemType].u32Counter > sstMemory.astMemoryTrackerList[_eMemType].u32PeakCounter)
      {
        sstMemory.astMemoryTrackerList[_eMemType].u32PeakCounter = sstMemory.astMemoryTrackerList[_eMemType].u32Counter;
      }
      if(sstMemory.astMemoryTrackerList[_eMemType].u32Size > sstMemory.astMemoryTrackerList[_eMemType].u32PeakSize)
      {
        sstMemory.astMemoryTrackerList[_eMemType].u32PeakSize = sstMemory.astMemoryTrackerList[_eMemType].u32Size;
      }
    }
    else
    {
      /* Updates counters */
      sstMemory.astMemoryTrackerList[_eMemType].u32Size -= _u32Size;
      sstMemory.astMemoryTrackerList[_eMemType].u32Counter--;
    }

    /* Updates operation counter */
    sstMemory.astMemoryTrackerList[_eMemType].u32OperationCounter++;
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
