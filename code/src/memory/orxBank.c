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
 * @file orxBank.c
 * @date 02/04/2005
 * @author iarwain@orx-project.org
 *
 */


#include "memory/orxBank.h"
#include "debug/orxDebug.h"
#include "debug/orxProfiler.h"
#include "math/orxMath.h"
#include "utils/orxLinkList.h"


#define orxBANK_KU32_STATIC_FLAG_NONE         0x00000000

#define orxBANK_KU32_STATIC_FLAG_READY        0x00000001

#define orxBANK_KU32_STATIC_MASK_ALL          0xFFFFFFFF

#define orxBANK_KU32_TAG_SIZE                 8


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Bank structure
 */
struct __orxBANK_t
{
  orxLINKLIST_NODE  stNode;                 /**< Linklist node */
  orxU32           *au32CellMap;            /**< Cell map */
  void            **apstSegmentData;        /**< Segment data */
  orxU32           *au32SegmentFree;        /**< Segment free */
  orxU32            u32CellSize;            /**< Cell size */
  orxU32            u32SegmentSize;         /**< Segment size */
  orxU32            u32CellCount;           /**< Allocated cells count */
  orxU32            u32SegmentCount;        /**< Segment count */
  orxU32            u32Flags;               /**< Flags set for the memory bank */
  orxMEMORY_TYPE    eMemType;               /**< Memory type that will be used by the memory allocation */
};

/** Static structure
 */
typedef struct __orxBANK_STATIC_t
{
  orxLINKLIST       stBankList;             /**< Bank linklist */
  orxU32            u32CacheLineSize;       /**< Cache line size */
  orxU32            u32Flags;               /**< Flags set by the memory module */

} orxBANK_STATIC;


/***************************************************************************
 * Module global variable                                                  *
 ***************************************************************************/

/** Static data
 */
static orxBANK_STATIC sstBank;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

/** Adds a new segment
 * @param[in] _pstBank    Concerned bank
 * @return orxSTATUS_SUCCESS/orxSTATUS_FAILURE
 */
static orxINLINE orxSTATUS orxBank_AddSegment(orxBANK *_pstBank)
{
  void    **apstNewSegmentData;
  orxU32    u32NewSegmentCount;
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Profiles */
  orxPROFILER_PUSH_MARKER("orxBank_AddSegment");

  /* Gets new segment count */
  u32NewSegmentCount = _pstBank->u32SegmentCount + 1;

  /* Allocates new segment entry */
  apstNewSegmentData = (void **)orxMemory_Reallocate(_pstBank->apstSegmentData, u32NewSegmentCount * sizeof(void *), orxMEMORY_TYPE_SYSTEM);

  /* Success? */
  if(apstNewSegmentData != orxNULL)
  {
    orxU32 *au32NewCellMap;
    orxU32  u32MapSize;

    /* Stores it */
    _pstBank->apstSegmentData = apstNewSegmentData;

    /* Inits it */
    apstNewSegmentData[_pstBank->u32SegmentCount] = orxNULL;

    /* Gets segment map size */
    u32MapSize = (orxU32)orxALIGN(_pstBank->u32SegmentSize, 32) >> 5;

    /* Allocates new cell map */
    au32NewCellMap = (orxU32 *)orxMemory_Reallocate(_pstBank->au32CellMap, u32NewSegmentCount * u32MapSize * sizeof(orxU32), orxMEMORY_TYPE_SYSTEM);

    /* Success? */
    if(au32NewCellMap != orxNULL)
    {
      orxU32 *au32NewSegmentFree;

      /* Stores it */
      _pstBank->au32CellMap = au32NewCellMap;

      /* Inits it */
      orxMemory_Set(au32NewCellMap + _pstBank->u32SegmentCount * u32MapSize, 0xFF, u32MapSize * sizeof(orxU32));

      /* Allocates new segment free */
      au32NewSegmentFree = (orxU32 *)orxMemory_Reallocate(_pstBank->au32SegmentFree, u32NewSegmentCount * sizeof(orxU32), orxMEMORY_TYPE_SYSTEM);

      /* Success? */
      if(au32NewSegmentFree != orxNULL)
      {
        orxU32 u32DataSize;

        /* Stores it */
        _pstBank->au32SegmentFree = au32NewSegmentFree;

        /* Inits it */
        au32NewSegmentFree[_pstBank->u32SegmentCount] = _pstBank->u32SegmentSize;

        /* Gets segment data size */
        u32DataSize = _pstBank->u32SegmentSize * _pstBank->u32CellSize + sstBank.u32CacheLineSize - 1;

        /* Allocates new segment data */
        apstNewSegmentData[_pstBank->u32SegmentCount] = (void *)orxMemory_Allocate(u32DataSize, _pstBank->eMemType);

        /* Success? */
        if(apstNewSegmentData[_pstBank->u32SegmentCount] != orxNULL)
        {
          /* Updates segment count */
          _pstBank->u32SegmentCount++;

          /* Updates result */
          eResult = orxSTATUS_SUCCESS;
        }
      }
    }
  }

  /* Failure? */
  if(eResult != orxSTATUS_SUCCESS)
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_MEMORY, "Couldn't add new segment to bank, aborting.");

    /* Do not clear any successful re-allocations, they'll be cleared when the bank is deleted */
  }

  /* Profiles */
  orxPROFILER_POP_MARKER();

  /* Done! */
  return eResult;
}

/** Deletes all the banks
 */
static orxINLINE void orxBank_DeleteAll()
{
  orxBANK *pstBank;

  /* Gets first bank */
  pstBank = (orxBANK *)orxLinkList_GetFirst(&(sstBank.stBankList));

  /* Non empty? */
  while(pstBank != orxNULL)
  {
    /* Deletes it */
    orxBank_Delete(pstBank);

    /* Gets first bank */
    pstBank = (orxBANK *)orxLinkList_GetFirst(&(sstBank.stBankList));
  }

  /* Done! */
  return;
}


/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** Setups the bank module
 */
void orxFASTCALL orxBank_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_BANK, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_BANK, orxMODULE_ID_PROFILER);

  return;
}

/** Inits the bank Module
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxBank_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Not already Initialized? */
  if(!(sstBank.u32Flags & orxBANK_KU32_STATIC_FLAG_READY))
  {
    /* Cleans static controller */
    orxMemory_Zero(&sstBank, sizeof(orxBANK_STATIC));

    /* Gets cache line size */
    sstBank.u32CacheLineSize = orxMemory_GetCacheLineSize();

    /* Inits flags */
    sstBank.u32Flags = orxBANK_KU32_STATIC_FLAG_READY;

    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_MEMORY, "Tried to initialize bank module when it was already initialized.");

    /* Already initialized */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Done */
  return eResult;
}

/** Exits from the bank module
 */
void orxFASTCALL orxBank_Exit()
{
  /* Initialized? */
  if((sstBank.u32Flags & orxBANK_KU32_STATIC_FLAG_READY) == orxBANK_KU32_STATIC_FLAG_READY)
  {
    /* Deletes all banks */
    orxBank_DeleteAll();

    /* Updates flags */
    sstBank.u32Flags &= ~orxBANK_KU32_STATIC_FLAG_READY;
  }

  /* Done! */
  return;
}

/** Creates a memory bank
 * @param[in] _u32Count   Number of cells per segments
 * @param[in] _u32Size    Size of a cell
 * @param[in] _u32Flags   Bank flags
 * @param[in] _eMemType   Memory type
 * @return orxBANK / orxNULL
 */
orxBANK *orxFASTCALL orxBank_Create(orxU32 _u32Count, orxU32 _u32Size, orxU32 _u32Flags, orxMEMORY_TYPE _eMemType)
{
  orxBANK *pstResult;

  /* Checks */
  orxASSERT(sstBank.u32Flags & orxBANK_KU32_STATIC_FLAG_READY);
  orxASSERT(_eMemType < orxMEMORY_TYPE_NUMBER);
  orxASSERT(_u32Count > 0);
  orxASSERT(_u32Size > 0);

  /* Creates bank */
  pstResult = (orxBANK *)orxMemory_Allocate(sizeof(orxBANK), orxMEMORY_TYPE_SYSTEM);

  /* Success? */
  if(pstResult != orxNULL)
  {
    orxU32 u32Size;

    /* Clears it */
    orxMemory_Zero(pstResult, sizeof(orxBANK));

    /* Add it to the list */
    orxLinkList_AddEnd(&(sstBank.stBankList), &(pstResult->stNode));

    /* Gets full cell size */
    u32Size = _u32Size + orxBANK_KU32_TAG_SIZE;

    /* Inits it */
    pstResult->u32CellSize    = (u32Size > sstBank.u32CacheLineSize)
                              ? (orxU32)orxALIGN(u32Size, sstBank.u32CacheLineSize)
                              : (orxMath_IsPowerOfTwo(u32Size) == orxFALSE)
                                ? orxMath_GetNextPowerOfTwo(u32Size)
                                : u32Size;
    pstResult->u32SegmentSize = _u32Count;
    pstResult->u32Flags       = _u32Flags;
    pstResult->eMemType       = _eMemType;

    /* Can't add the first segment? */
    if(orxBank_AddSegment(pstResult) == orxSTATUS_FAILURE)
    {
      /* Deletes it */
      orxBank_Delete(pstResult);

      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_MEMORY, "Couldn't add first bank segment, aborting.");

      /* Updates result */
      pstResult = orxNULL;
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_MEMORY, "Couldn't create new bank.");
  }

  /* Done! */
  return pstResult;
}

/** Deletes a bank
 * @param[in] _pstBank    Concerned bank
 */
void orxFASTCALL orxBank_Delete(orxBANK *_pstBank)
{
  /* Checks */
  orxASSERT(sstBank.u32Flags & orxBANK_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBank != orxNULL);

  /* Removes it from the list */
  orxLinkList_Remove(&(_pstBank->stNode));

  /* Has segments data? */
  if(_pstBank->apstSegmentData != orxNULL)
  {
    orxU32 i;

    /* For all segments */
    for(i = 0; i < _pstBank->u32SegmentCount; i++)
    {
      /* Is present? */
      if(_pstBank->apstSegmentData[i] != orxNULL)
      {
        /* Frees it */
        orxMemory_Free(_pstBank->apstSegmentData[i]);
      }
    }

    /* Frees segment data */
    orxMemory_Free(_pstBank->apstSegmentData);
  }

  /* Frees cell map */
  orxMemory_Free(_pstBank->au32CellMap);

  /* Frees segment free */
  orxMemory_Free(_pstBank->au32SegmentFree);

  /* Frees bank */
  orxMemory_Free(_pstBank);

  /* Done! */
  return;
}

/** Allocates a new cell from the bank
 * @param[in] _pstBank    Concerned bank
 * @return a new cell of memory (orxNULL if no allocation possible)
 */
void *orxFASTCALL orxBank_Allocate(orxBANK *_pstBank)
{
  orxU32 u32Dummy;

  /* Done! */
  return orxBank_AllocateIndexed(_pstBank, &u32Dummy, orxNULL);
}

/** Allocates a new cell from the bank and returns its index
 * @param[in] _pstBank        Concerned bank
 * @param[out] _pu32ItemIndex Will be set with the allocated item index
 * @param[out] _ppPrevious    If non-null, will contain previous neighbor if found
 * @return a new cell of memory (orxNULL if no allocation possible)
 */
void *orxFASTCALL orxBank_AllocateIndexed(orxBANK *_pstBank, orxU32 *_pu32ItemIndex, void **_ppPrevious)
{
  orxU32  u32SegmentIndex, u32CellIndex, u32MapSize;
  void   *pResult = orxNULL;

  /* Profiles */
  orxPROFILER_PUSH_MARKER("orxBank_AllocateIndexed");

  /* Checks */
  orxASSERT(sstBank.u32Flags & orxBANK_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBank != orxNULL);
  orxASSERT(_pu32ItemIndex != orxNULL);

  /* Gets segment map size */
  u32MapSize = (orxU32)orxALIGN(_pstBank->u32SegmentSize, 32) >> 5;

  /* Has room? */
  if(_pstBank->u32CellCount < _pstBank->u32SegmentCount * _pstBank->u32SegmentSize)
  {
    orxU32 *pu32MapEntry;

    /* Finds non-full segment */
    for(u32SegmentIndex = 0; (u32SegmentIndex < _pstBank->u32SegmentCount) && (_pstBank->au32SegmentFree[u32SegmentIndex] == 0); u32SegmentIndex++)
      ;

    /* Checks */
    orxASSERT(u32SegmentIndex < _pstBank->u32SegmentCount);

    /* For all cell map entries */
    for(u32CellIndex = 0, pu32MapEntry = _pstBank->au32CellMap + u32SegmentIndex * u32MapSize;
        u32CellIndex < _pstBank->u32SegmentSize;
        u32CellIndex += 32, pu32MapEntry++)
    {
      /* Found free cell? */
      if(*pu32MapEntry != 0)
      {
        /* Updates cell index */
        u32CellIndex += orxMath_GetTrailingZeroCount(*pu32MapEntry);

        /* Stops */
        break;
      }
    }

    /* Checks */
    orxASSERT(u32CellIndex < _pstBank->u32SegmentSize);
    orxASSERT(*pu32MapEntry & (1 << (u32CellIndex & 31)));

    /* Updates map */
    *pu32MapEntry &= ~(1 << (u32CellIndex & 31));
  }
  else
  {
    /* Can add a new segment? */
    if(!orxFLAG_TEST(_pstBank->u32Flags, orxBANK_KU32_FLAG_NOT_EXPANDABLE)
    && (orxBank_AddSegment(_pstBank) != orxSTATUS_FAILURE))
    {
      /* Gets indices */
      u32SegmentIndex = _pstBank->u32SegmentCount - 1;
      u32CellIndex    = 0;

      /* Updates map */
      _pstBank->au32CellMap[u32SegmentIndex * u32MapSize] &= ~1;
    }
    else
    {
      /* Clears indices */
      u32SegmentIndex = u32CellIndex = orxU32_UNDEFINED;
    }
  }

  /* Success? */
  if(u32SegmentIndex != orxU32_UNDEFINED)
  {
    orxU32 u32ItemIndex;

    /* Checks */
    orxASSERT(_pstBank->au32SegmentFree[u32SegmentIndex] != 0);
    orxASSERT(_pstBank->u32CellCount < _pstBank->u32SegmentCount * _pstBank->u32SegmentSize);

    /* Updates segment free */
    _pstBank->au32SegmentFree[u32SegmentIndex]--;

    /* Updates cell count */
    _pstBank->u32CellCount++;

    /* Updates item index */
    *_pu32ItemIndex = u32ItemIndex = u32SegmentIndex * _pstBank->u32SegmentSize + u32CellIndex;

    /* Was previous cell requested? */
    if(_ppPrevious != orxNULL)
    {
      /* Updates it */
      *_ppPrevious = (u32CellIndex == 0)
                   ? (u32SegmentIndex == 0)
                     ? orxNULL
                     : (void*)((orxU8 *)orxALIGN(_pstBank->apstSegmentData[u32SegmentIndex - 1], sstBank.u32CacheLineSize) + (_pstBank->u32SegmentSize - 1) * _pstBank->u32CellSize + orxBANK_KU32_TAG_SIZE)
                   : (void*)((orxU8 *)orxALIGN(_pstBank->apstSegmentData[u32SegmentIndex], sstBank.u32CacheLineSize) + (u32CellIndex - 1) * _pstBank->u32CellSize + orxBANK_KU32_TAG_SIZE);
    }

    /* Updates result */
    pResult = (void *)((orxU8 *)orxALIGN(_pstBank->apstSegmentData[u32SegmentIndex], sstBank.u32CacheLineSize) + u32CellIndex * _pstBank->u32CellSize + orxBANK_KU32_TAG_SIZE);

    /* Stores its index */
    *(orxU32 *)((orxU8 *)pResult - orxBANK_KU32_TAG_SIZE) = u32ItemIndex;
  }

  /* Profiles */
  orxPROFILER_POP_MARKER();

  /* Done! */
  return pResult;
}

/** Frees an allocated cell
 * @param[in] _pstBank    Concerned bank
 * @param[in] _pCell      Pointer to the cell to free
 */
void orxFASTCALL orxBank_Free(orxBANK *_pstBank, void *_pCell)
{
  orxU32 u32SegmentIndex, u32CellIndex, u32MapSize;

  /* Profiles */
  orxPROFILER_PUSH_MARKER("orxBank_Free");

  /* Checks */
  orxASSERT(sstBank.u32Flags & orxBANK_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBank != orxNULL);
  orxASSERT(_pCell != orxNULL);

  /* Gets indices */
  u32SegmentIndex = *(orxU32 *)((orxU8 *)_pCell - orxBANK_KU32_TAG_SIZE) / _pstBank->u32SegmentSize;
  u32CellIndex    = *(orxU32 *)((orxU8 *)_pCell - orxBANK_KU32_TAG_SIZE) % _pstBank->u32SegmentSize;

  /* Gets segment map size */
  u32MapSize = (orxU32)orxALIGN(_pstBank->u32SegmentSize, 32) >> 5;

  /* Checks */
  orxASSERT(!(_pstBank->au32CellMap[u32SegmentIndex * u32MapSize + (u32CellIndex >> 5)] & (1 << (u32CellIndex & 31))));

  /* Marks cell as free */
  _pstBank->au32CellMap[u32SegmentIndex * u32MapSize + (u32CellIndex >> 5)] |= 1 << (u32CellIndex & 31);

  /* Updates segment free count */
  _pstBank->au32SegmentFree[u32SegmentIndex]++;

  /* Updates bank count */
  _pstBank->u32CellCount--;

  /* Profiles */
  orxPROFILER_POP_MARKER();

  /* Done! */
  return;
}

/** Frees an allocated cell at a given index
 * @param[in] _pstBank    Concerned bank
 * @param[in] _u32Index   Index of the cell to free
 */
void orxFASTCALL orxBank_FreeAtIndex(orxBANK *_pstBank, orxU32 _u32Index)
{
  orxU32 u32SegmentIndex, u32CellIndex, u32MapSize;

  /* Profiles */
  orxPROFILER_PUSH_MARKER("orxBank_FreeAtIndex");

  /* Checks */
  orxASSERT((sstBank.u32Flags & orxBANK_KU32_STATIC_FLAG_READY) == orxBANK_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBank != orxNULL);
  orxASSERT(_u32Index < _pstBank->u32SegmentCount * _pstBank->u32SegmentSize);
  orxASSERT(_pstBank->au32SegmentFree[_u32Index / _pstBank->u32SegmentSize] < _pstBank->u32SegmentSize);
  orxASSERT(_pstBank->u32CellCount != 0);

  /* Gets segment map size */
  u32MapSize = (orxU32)orxALIGN(_pstBank->u32SegmentSize, 32) >> 5;

  /* Gets indices */
  u32SegmentIndex = _u32Index / _pstBank->u32SegmentSize;
  u32CellIndex    = _u32Index % _pstBank->u32SegmentSize;

  /* Checks */
  orxASSERT(!(_pstBank->au32CellMap[u32SegmentIndex * u32MapSize + (u32CellIndex >> 5)] & (1 << (u32CellIndex & 31))));

  /* Marks cell as free */
  _pstBank->au32CellMap[u32SegmentIndex * u32MapSize + (u32CellIndex >> 5)] |= 1 << (u32CellIndex & 31);

  /* Updates segment free count */
  _pstBank->au32SegmentFree[u32SegmentIndex]++;

  /* Updates bank count */
  _pstBank->u32CellCount--;

  /* Profiles */
  orxPROFILER_POP_MARKER();

  /* Done! */
  return;
}

/** Frees all allocated cell from a bank
 * @param[in] _pstBank    Concerned bank
 */
void orxFASTCALL orxBank_Clear(orxBANK *_pstBank)
{
  orxU32 i, u32MapSize;

  /* Checks */
  orxASSERT(sstBank.u32Flags & orxBANK_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBank != orxNULL);

  /* Gets segment map size */
  u32MapSize = (orxU32)orxALIGN(_pstBank->u32SegmentSize, 32) >> 5;

  /* Clears cell map */
  orxMemory_Set(_pstBank->au32CellMap, 0xFF, _pstBank->u32SegmentCount * u32MapSize * sizeof(orxU32));

  /* For all segments */
  for(i = 0; i < _pstBank->u32SegmentCount; i++)
  {
    /* Resets its free */
    _pstBank->au32SegmentFree[i] = _pstBank->u32SegmentSize;
  }

  /* Clears cell count */
  _pstBank->u32CellCount = 0;

  /* Done! */
  return;
}

/** Compacts a bank by removing all its trailing unused segments
 * @param[in] _pstBank    Concerned bank
 */
void orxFASTCALL orxBank_Compact(orxBANK *_pstBank)
{
  orxU32 u32SegmentIndex, u32LastSegmentIndex;

  /* For all segments */
  for(u32SegmentIndex = 0, u32LastSegmentIndex = 0;
      u32SegmentIndex < _pstBank->u32SegmentCount;
      u32SegmentIndex++)
  {
    /* Not empty? */
    if(_pstBank->au32SegmentFree[u32SegmentIndex] != _pstBank->u32SegmentSize)
    {
      /* Updates last index */
      u32LastSegmentIndex = u32SegmentIndex;
    }
  }

  /* Should compact? */
  if(u32LastSegmentIndex + 1 < _pstBank->u32SegmentCount)
  {
    void  **apstNewSegmentData;
    orxU32  u32NewSegmentCount, i;

    /* Gets new segment count */
    u32NewSegmentCount = u32LastSegmentIndex + 1;

    /* Frees unused segment data */
    for(i = u32NewSegmentCount; i < _pstBank->u32SegmentCount; i++)
    {
      /* Frees it */
      orxMemory_Free(_pstBank->apstSegmentData[i]);
      _pstBank->apstSegmentData[i] = orxNULL;
    }

    /* Allocates new segment entry */
    apstNewSegmentData = (void **)orxMemory_Reallocate(_pstBank->apstSegmentData, u32NewSegmentCount * sizeof(void *), orxMEMORY_TYPE_SYSTEM);

    /* Success? */
    if(apstNewSegmentData != orxNULL)
    {
      orxU32 *au32NewCellMap;
      orxU32  u32MapSize;

      /* Stores it */
      _pstBank->apstSegmentData = apstNewSegmentData;

      /* Gets segment map size */
      u32MapSize = (orxU32)orxALIGN(_pstBank->u32SegmentSize, 32) >> 5;

      /* Allocates new cell map */
      au32NewCellMap = (orxU32 *)orxMemory_Reallocate(_pstBank->au32CellMap, u32NewSegmentCount * u32MapSize * sizeof(orxU32), orxMEMORY_TYPE_SYSTEM);

      /* Success? */
      if(au32NewCellMap != orxNULL)
      {
        orxU32 *au32NewSegmentFree;

        /* Stores it */
        _pstBank->au32CellMap = au32NewCellMap;

        /* Allocates new segment free */
        au32NewSegmentFree = (orxU32 *)orxMemory_Reallocate(_pstBank->au32SegmentFree, u32NewSegmentCount * sizeof(orxU32), orxMEMORY_TYPE_SYSTEM);

        /* Success? */
        if(au32NewSegmentFree != orxNULL)
        {
          /* Stores it */
          _pstBank->au32SegmentFree = au32NewSegmentFree;
        }
      }
    }

    /* Updates segment count */
    _pstBank->u32SegmentCount = u32NewSegmentCount;
  }

  /* Done! */
  return;
}

/** Compacts all banks by removing all their unused segments
 */
void orxFASTCALL orxBank_CompactAll()
{
  orxBANK *pstBank;

  /* Checks */
  orxASSERT(sstBank.u32Flags & orxBANK_KU32_STATIC_FLAG_READY);

  /* For all banks */
  for(pstBank = (orxBANK *)orxLinkList_GetFirst(&(sstBank.stBankList));
      pstBank != orxNULL;
      pstBank = (orxBANK *)orxLinkList_GetNext(&(pstBank->stNode)))
  {
    /* Compacts it */
    orxBank_Compact(pstBank);
  }

  /* Done! */
  return;
}

/** Gets the next cell
 * @param[in] _pstBank    Concerned bank
 * @param[in] _pCell      Pointer to the current cell of memory, orxNULL to get the first one
 * @return The next cell if found, orxNULL otherwise
 */
void *orxFASTCALL orxBank_GetNext(const orxBANK *_pstBank, const void *_pCell)
{
  void *pResult = orxNULL;

  /* Profiles */
  orxPROFILER_PUSH_MARKER("orxBank_GetNext");

  /* Checks */
  orxASSERT((sstBank.u32Flags & orxBANK_KU32_STATIC_FLAG_READY) == orxBANK_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBank != orxNULL);

  /* Isn't empty? */
  if(_pstBank->u32CellCount != 0)
  {
    orxU32 *pu32MapEntry = orxNULL, u32Mask = 0xFFFFFFFF, u32MapSize;

    /* Gets segment map size */
    u32MapSize = (orxU32)orxALIGN(_pstBank->u32SegmentSize, 32) >> 5;

    /* No cell? */
    if(_pCell == orxNULL)
    {
      /* Selects first segment */
      pu32MapEntry = _pstBank->au32CellMap;
    }
    else
    {
      orxU32 u32SegmentIndex, u32CellIndex;

      /* Gets indices */
      u32SegmentIndex = *(orxU32 *)((orxU8 *)_pCell - orxBANK_KU32_TAG_SIZE) / _pstBank->u32SegmentSize;
      u32CellIndex    = *(orxU32 *)((orxU8 *)_pCell - orxBANK_KU32_TAG_SIZE) % _pstBank->u32SegmentSize;

      /* Is cell allocated? */
      if(!(_pstBank->au32CellMap[u32SegmentIndex * u32MapSize + (u32CellIndex >> 5)] & (1 << (u32CellIndex & 31))))
      {
        /* Gets its associated map entry */
        pu32MapEntry = _pstBank->au32CellMap + u32SegmentIndex * u32MapSize + (u32CellIndex >> 5);

        /* Updates mask */
        u32Mask = ((u32CellIndex & 31) == 31) ? 0 : ~((1 << ((u32CellIndex & 31) + 1)) - 1);
      }

      /* Checks */
      orxASSERT(pu32MapEntry != orxNULL);
    }

    /* Found? */
    if(pu32MapEntry != orxNULL)
    {
      orxU32 *pu32MapEntryEnd;

      /* For all map entries */
      for(pu32MapEntryEnd = _pstBank->au32CellMap + _pstBank->u32SegmentCount * u32MapSize;
          pu32MapEntry < pu32MapEntryEnd;
          pu32MapEntry++, u32Mask = 0xFFFFFFFF)
      {
        orxU32 u32MaskedMapEntry;

        /* Gets masked map entry */
        u32MaskedMapEntry = ~*pu32MapEntry & u32Mask;

        /* Has allocated cells? */
        if(u32MaskedMapEntry != 0)
        {
          orxU32 u32SegmentIndex, u32CellIndex;

          /* Gets associated indices */
          u32SegmentIndex = (orxU32)(pu32MapEntry - _pstBank->au32CellMap) / u32MapSize;
          u32CellIndex    = 32 * ((orxU32)(pu32MapEntry - _pstBank->au32CellMap) % u32MapSize) + orxMath_GetTrailingZeroCount(u32MaskedMapEntry);

          /* Updates result */
          pResult = (void *)((orxU8 *)orxALIGN(_pstBank->apstSegmentData[u32SegmentIndex], sstBank.u32CacheLineSize) + u32CellIndex * _pstBank->u32CellSize + orxBANK_KU32_TAG_SIZE);

          /* Stops */
          break;
        }
      }
    }
  }

  /* Profiles */
  orxPROFILER_POP_MARKER();

  /* Done! */
  return pResult;
}

/** Gets the cell's index
 * @param[in] _pstBank    Concerned memory bank
 * @param[in] _pCell      Cell of which we want the index
 * @return The index of the given cell
 */
orxU32 orxFASTCALL orxBank_GetIndex(const orxBANK *_pstBank, const void *_pCell)
{
  orxU32 u32SegmentIndex, u32CellIndex, u32MapSize, u32Result;

  /* Profiles */
  orxPROFILER_PUSH_MARKER("orxBank_GetIndex");

  /* Checks */
  orxASSERT(sstBank.u32Flags & orxBANK_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBank != orxNULL);
  orxASSERT(_pCell != orxNULL);

  /* Gets segment map size */
  u32MapSize = (orxU32)orxALIGN(_pstBank->u32SegmentSize, 32) >> 5;

  /* Gets indices */
  u32SegmentIndex = *(orxU32 *)((orxU8 *)_pCell - orxBANK_KU32_TAG_SIZE) / _pstBank->u32SegmentSize;
  u32CellIndex    = *(orxU32 *)((orxU8 *)_pCell - orxBANK_KU32_TAG_SIZE) % _pstBank->u32SegmentSize;

  /* Is cell allocated? */
  if(!(_pstBank->au32CellMap[u32SegmentIndex * u32MapSize + (u32CellIndex >> 5)] & (1 << (u32CellIndex & 31))))
  {
    /* Updates result */
    u32Result = *(orxU32 *)((orxU8 *)_pCell - orxBANK_KU32_TAG_SIZE);
  }
  else
  {
    /* Updates result */
    u32Result = orxU32_UNDEFINED;
  }

  /* Profiles */
  orxPROFILER_POP_MARKER();

  /* Done! */
  return u32Result;
}

/** Gets the cell at given index, orxNULL is the cell isn't allocated
 * @param[in] _pstBank    Concerned memory bank
 * @param[in] _u32Index   Index of the cell to retrieve
 * @return The cell at the given index if allocated, orxNULL otherwise
 */
void *orxFASTCALL orxBank_GetAtIndex(const orxBANK *_pstBank, orxU32 _u32Index)
{
  orxU32  u32SegmentIndex, u32CellIndex, u32MapSize;
  void   *pResult = orxNULL;

  /* Profiles */
  orxPROFILER_PUSH_MARKER("orxBank_GetAtIndex");

  /* Checks */
  orxASSERT((sstBank.u32Flags & orxBANK_KU32_STATIC_FLAG_READY) == orxBANK_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBank != orxNULL);
  orxASSERT(_pstBank->au32SegmentFree[_u32Index / _pstBank->u32SegmentSize] <= _pstBank->u32SegmentSize);

  /* Is within segment boundaries? */
  if(_u32Index < _pstBank->u32SegmentCount * _pstBank->u32SegmentSize)
  {
    /* Gets segment map size */
    u32MapSize = (orxU32)orxALIGN(_pstBank->u32SegmentSize, 32) >> 5;

    /* Gets indices */
    u32SegmentIndex = _u32Index / _pstBank->u32SegmentSize;
    u32CellIndex    = _u32Index % _pstBank->u32SegmentSize;

    /* Is cell allocated? */
    if(!(_pstBank->au32CellMap[u32SegmentIndex * u32MapSize + (u32CellIndex >> 5)] & (1 << (u32CellIndex & 31))))
    {
      /* Updates result */
      pResult = (void *)((orxU8 *)orxALIGN(_pstBank->apstSegmentData[u32SegmentIndex], sstBank.u32CacheLineSize) + u32CellIndex * _pstBank->u32CellSize + orxBANK_KU32_TAG_SIZE);
    }
  }

  /* Profiles */
  orxPROFILER_POP_MARKER();

  /* Done! */
  return pResult;
}

/** Gets the bank allocated cell count
 * @param[in] _pstBank    Concerned bank
 * @return Number of allocated cells
 */
orxU32 orxFASTCALL orxBank_GetCount(const orxBANK *_pstBank)
{
  /* Checks */
  orxASSERT((sstBank.u32Flags & orxBANK_KU32_STATIC_FLAG_READY) == orxBANK_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBank != orxNULL);

  /* Done! */
  return _pstBank->u32CellCount;
}
