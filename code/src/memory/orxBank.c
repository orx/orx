/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2011 Orx-Project
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
 * @author bestel@arcallians.org
 *
 */


#include "memory/orxBank.h"
#include "debug/orxDebug.h"
#include "debug/orxProfiler.h"
#include "utils/orxLinkList.h"
#include "utils/orxString.h"


#define orxBANK_KU32_STATIC_FLAG_NONE         0x00000000  /**< No flags have been set */
#define orxBANK_KU32_STATIC_FLAG_READY        0x00000001  /**< The module has been initialized */


#ifdef __orxBANK_ALIGN__

  #if defined(__orxIPHONE__) || defined(__orxANDROID__) || defined(__orxANDROID_NATIVE__)

    #define orxBANK_KU32_CACHE_LINE_SIZE        32

  #else /* __orxIPHONE__ || __orxANDROID__ || __orxANDROID_NATIVE__ */

    #define orxBANK_KU32_CACHE_LINE_SIZE        64

  #endif /* __orxIPHONE__ || __orxANDROID__ || __orxANDROID_NATIVE__ */

#else /* __orxBANK_ALIGN__ */

  #define orxBANK_KU32_CACHE_LINE_SIZE          8

#endif /* __orxBANK_ALIGN__ */


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/
#ifdef __orxMSVC__
  #pragma warning(disable : 4200)
#endif /* __orxMSVC__ */
typedef struct __orxBANK_SEGMENT_t
{
  void                       *pSegmentData;     /**< Pointer address on the head of the segment data cells */
  struct __orxBANK_SEGMENT_t *pstNext;          /**< Pointer on the next segment */
  orxU32                      u32NbFree;        /**< Number of free elements in the segment */
  orxU32                      au32CellAllocationMap[0]; /**< List of bits that represents free and used elements in the segment */

} orxBANK_SEGMENT;
#ifdef __orxMSVC__
  #pragma warning(default : 4200)
#endif /* __orxMSVC__ */

struct __orxBANK_t
{
  orxLINKLIST_NODE  stNode;                 /**< Linklist node */
  orxBANK_SEGMENT  *pstFirstSegment;        /**< First segment used in the bank */
  orxU32            u32Counter;             /**< Number of allocated cells */
  orxU32            u32ElemSize;            /**< Size of a cell */
  orxU16            u16NbCellPerSegments;   /**< Number of cells per banks */
  orxU16            u16SizeSegmentBitField; /**< Number of u32 (4 bytes) to represent a segment */
  orxU32            u32Flags;               /**< Flags set for the memory bank */
  orxMEMORY_TYPE    eMemType;               /**< Memory type that will be used by the memory allocation */
};

typedef struct __orxBANK_STATIC_t
{
  orxLINKLIST       stBankList;             /**< Bank linklist */
  orxU32            u32Flags;               /**< Flags set by the memory module */

} orxBANK_STATIC;

/***************************************************************************
 * Module global variable                                                  *
 ***************************************************************************/
static orxBANK_STATIC sstBank;

/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

/** Create a new segment of memory and returns a pointer on it
 * @param[in] _pstBank    Concerned bank
 * @return  returns a pointer on the memory segment (orxNULL if an error occured)
 */
static orxINLINE orxBANK_SEGMENT *orxBank_CreateSegment(const orxBANK *_pstBank)
{
  orxBANK_SEGMENT *pstSegment;  /* Pointer on the segment of memory */
  orxU32 u32BaseSegmentSize;    /* Base size of segment allocation */

  /* Profiles */
  orxPROFILER_PUSH_MARKER("orxBank_CreateSegment");

  /* Module initialized ? */
  orxASSERT((sstBank.u32Flags & orxBANK_KU32_STATIC_FLAG_READY) == orxBANK_KU32_STATIC_FLAG_READY);

  /* Correct parameters ? */
  orxASSERT(_pstBank != orxNULL);

  /* Compute the base aligned segment size */
  u32BaseSegmentSize = sizeof(orxBANK_SEGMENT) + _pstBank->u16SizeSegmentBitField * sizeof(orxU32);
  u32BaseSegmentSize = orxALIGN(u32BaseSegmentSize, orxBANK_KU32_CACHE_LINE_SIZE);

  /* Allocates a new segment of memory */
  pstSegment = (orxBANK_SEGMENT *)orxMemory_Allocate(u32BaseSegmentSize + (_pstBank->u16NbCellPerSegments * _pstBank->u32ElemSize), _pstBank->eMemType);
  if(pstSegment != orxNULL)
  {
    /* Set initial segment values */
    orxMemory_Zero(pstSegment, u32BaseSegmentSize + (_pstBank->u16NbCellPerSegments * _pstBank->u32ElemSize));
    pstSegment->pstNext               = orxNULL;
    pstSegment->u32NbFree             = _pstBank->u16NbCellPerSegments;
    pstSegment->pSegmentData          = (void *)(((orxU8 *)pstSegment) + u32BaseSegmentSize);
  }

  /* Profiles */
  orxPROFILER_POP_MARKER();

  /* Done! */
  return pstSegment;
}

/** Returns the segment where is stored _pCell
 * @param[in] _pstBank  Bank that stores segments
 * @param[in] _pCell    Cell stored by the segment to find
 * @return  The segment where is stored _pCell (orxNULL if not found)
 */
static orxINLINE orxBANK_SEGMENT *orxBank_GetSegment(const orxBANK *_pstBank, const void *_pCell)
{
  orxBANK_SEGMENT  *pstSegment, *pstResult = orxNULL;
  orxU32            u32Size;

  /* Module initialized ? */
  orxASSERT((sstBank.u32Flags & orxBANK_KU32_STATIC_FLAG_READY) == orxBANK_KU32_STATIC_FLAG_READY);

  /* Correct parameters ? */
  orxASSERT(_pstBank != orxNULL);
  orxASSERT(_pCell != orxNULL);

  /* For all segments */
  for(pstSegment = _pstBank->pstFirstSegment, u32Size = _pstBank->u32ElemSize * (orxU32)_pstBank->u16NbCellPerSegments;
      pstSegment != orxNULL;
      pstSegment = pstSegment->pstNext)
  {
    /* Is cell in segment? */
    if((_pCell >= pstSegment->pSegmentData)
    && (((orxU32)((orxU8 *)_pCell - (orxU8 *)pstSegment->pSegmentData)) < u32Size))
    {
      /* Updates result */
      pstResult = pstSegment;

      break;
    }
  }

  /* Done! */
  return pstResult;
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

/** Initialize Bank Module
 */
orxSTATUS orxFASTCALL orxBank_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Not already Initialized? */
  if(!(sstBank.u32Flags & orxBANK_KU32_STATIC_FLAG_READY))
  {
    /* Cleans static controller */
    orxMemory_Zero(&sstBank, sizeof(orxBANK_STATIC));

    /* Set module has ready */
    sstBank.u32Flags = orxBANK_KU32_STATIC_FLAG_READY;

    /* Success */
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

/** Exit bank module
 */
void orxFASTCALL orxBank_Exit()
{
  /* Module initialized ? */
  if((sstBank.u32Flags & orxBANK_KU32_STATIC_FLAG_READY) == orxBANK_KU32_STATIC_FLAG_READY)
  {
    /* Module not ready now */
    sstBank.u32Flags = orxBANK_KU32_STATIC_FLAG_NONE;
  }

  return;
}

/** Create a new bank in memory and returns a pointer on it
 * @param[in] _u16NbElem  Number of elements per segments
 * @param[in] _u32Size    Size of an element
 * @param[in] _u32Flags   Flags set for this bank
 * @param[in] _eMemType   Memory type where the Data will be allocated
 * @return  returns a pointer on the memory bank
 */
orxBANK *orxFASTCALL orxBank_Create(orxU16 _u16NbElem, orxU32 _u32Size, orxU32 _u32Flags, orxMEMORY_TYPE _eMemType)
{
  orxBANK *pstBank = orxNULL; /* New bank */

  /* Module initialized ? */
  orxASSERT((sstBank.u32Flags & orxBANK_KU32_STATIC_FLAG_READY) == orxBANK_KU32_STATIC_FLAG_READY);

  /* Correct parameters ? */
  orxASSERT(_eMemType < orxMEMORY_TYPE_NUMBER);
  orxASSERT(_u16NbElem > 0);
  orxASSERT(_u32Size > 0);

  /* Allocate the bank */
  pstBank = (orxBANK *)orxMemory_Allocate(sizeof(orxBANK), _eMemType);

  /* Memory allocated ? */
  if(pstBank != orxNULL)
  {
    /* Set initial values */
    orxMemory_Zero(pstBank, sizeof(orxBANK));
    pstBank->u32Counter               = 0;
    pstBank->u32ElemSize              = (_u32Size > orxBANK_KU32_CACHE_LINE_SIZE)
                                        ? orxALIGN(_u32Size, orxBANK_KU32_CACHE_LINE_SIZE)
                                        : (orxMath_IsPowerOfTwo(_u32Size) == orxFALSE)
                                          ? orxMath_GetNextPowerOfTwo(_u32Size)
                                          : _u32Size;
    pstBank->u32Flags                 = _u32Flags;
    pstBank->u16NbCellPerSegments     = _u16NbElem;
    pstBank->eMemType                 = _eMemType;

    /* Compute the necessary number of 32 bits packs */
    pstBank->u16SizeSegmentBitField   = orxALIGN32(_u16NbElem) >> 5;

    /* Allocate the first segment, and select it as current */
    pstBank->pstFirstSegment          = orxBank_CreateSegment(pstBank);

    /* Success? */
    if(pstBank->pstFirstSegment != orxNULL)
    {
      /* Add it to the list */
      orxLinkList_AddEnd(&(sstBank.stBankList), &(pstBank->stNode));
    }
    else
    {
      /* Can't allocate segment, cancel bank allocation */
      orxMemory_Free(pstBank);
      pstBank = orxNULL;
    }
  }

  return pstBank;
}

/** Free a portion of memory allocated with orxMemory_Allocate
 * @param[in] _pstBank    Pointer on the memory bank allocated by orx
 */
void orxFASTCALL orxBank_Delete(orxBANK *_pstBank)
{
  orxBANK_SEGMENT *pstSegment, *pstSegmentToDelete;

  /* Module initialized ? */
  orxASSERT((sstBank.u32Flags & orxBANK_KU32_STATIC_FLAG_READY) == orxBANK_KU32_STATIC_FLAG_READY);

  /* Correct parameters ? */
  orxASSERT(_pstBank != orxNULL);

  /* Removes it from the list */
  orxLinkList_Remove(&(_pstBank->stNode));

  /* Deletes all segments */
  for(pstSegment = _pstBank->pstFirstSegment;
      pstSegment != orxNULL;
      pstSegmentToDelete = pstSegment, pstSegment = pstSegment->pstNext, orxMemory_Free(pstSegmentToDelete));

  /* Completly Free Bank */
  orxMemory_Free(_pstBank);
}

/** Allocate a new cell from the bank
 * @param[in] _pstBank    Pointer on the memory bank to use
 * @return a new cell of memory (orxNULL if no allocation possible)
 */
void *orxFASTCALL orxBank_Allocate(orxBANK *_pstBank)
{
  orxU32 u32Dummy;

  /* Done! */
  return orxBank_AllocateIndexed(_pstBank, &u32Dummy);
}

/** Allocates a new cell from the bank and returns its index
 * @param[in] _pstBank        Pointer on the memory bank to use
 * @param[out] _pu32ItemIndex Will be set with the allocated item index
 * @return a new cell of memory (orxNULL if no allocation possible)
 */
void *orxFASTCALL orxBank_AllocateIndexed(orxBANK *_pstBank, orxU32 *_pu32ItemIndex)
{
  orxU32            u32SegmentIndex;
  orxBANK_SEGMENT  *pstCurrentSegment;
  void             *pCell = orxNULL;   /* Returned cell */

  /* Profiles */
  orxPROFILER_PUSH_MARKER("orxBank_Allocate");

  /* Module initialized ? */
  orxASSERT((sstBank.u32Flags & orxBANK_KU32_STATIC_FLAG_READY) == orxBANK_KU32_STATIC_FLAG_READY);

  /* Correct parameters ? */
  orxASSERT(_pstBank != orxNULL);
  orxASSERT(_pu32ItemIndex != orxNULL);

  /* Finds the first segment with empty space */
  for(u32SegmentIndex = 0, pstCurrentSegment = _pstBank->pstFirstSegment;
      (pstCurrentSegment->pstNext != orxNULL) && (pstCurrentSegment->u32NbFree == 0);
      u32SegmentIndex++, pstCurrentSegment = pstCurrentSegment->pstNext);

  /* Is there a free space in the current segment ? (If no, try to expand it if allowed) */
  if((pstCurrentSegment->u32NbFree == 0) && (!(_pstBank->u32Flags & orxBANK_KU32_FLAG_NOT_EXPANDABLE)))
  {
    /* No, Try to allocate a new segment */
    pstCurrentSegment->pstNext = orxBank_CreateSegment(_pstBank);

    /* Correct segment allocation ? */
    if(pstCurrentSegment->pstNext != orxNULL)
    {
      /* Set the new allocated segment as the current selected one */
      pstCurrentSegment = pstCurrentSegment->pstNext;
      u32SegmentIndex++;
    }
  }

  /* Does the current segment has free nodes ? (yes : the allocation was correct (if there was), else returns orxNULL) */
  if(pstCurrentSegment->u32NbFree > 0)
  {
    orxU32  u32BitIndex, u32MapPartIndex;
    orxBOOL bFound;

    /* For all map parts */
    for(u32BitIndex = 0, u32MapPartIndex = 0, bFound = orxFALSE; !bFound && (u32MapPartIndex < (orxU32)_pstBank->u16SizeSegmentBitField); u32MapPartIndex++)
    {
      orxS32 s32MapPartValue;

      /* Gets the available slots */
      s32MapPartValue = ~(pstCurrentSegment->au32CellAllocationMap[u32MapPartIndex]);

      /* Any free one? */
      if(s32MapPartValue != 0)
      {
        /* Gets free cell's map part index */
        u32BitIndex = orxMath_GetTrailingZeroCount(s32MapPartValue);

        /* Found ! */
        bFound = orxTRUE;

        break;
      }
    }

    /* If bFound is false, It means that there are no more free segments that we can allocate.
     * It can be volunteer (orxBANK_KU32_FLAG_NOT_EXPANDABLE) or a problem in the code => assert
     */
    orxASSERT(bFound || ((_pstBank->u32Flags & orxBANK_KU32_FLAG_NOT_EXPANDABLE) == orxBANK_KU32_FLAG_NOT_EXPANDABLE));

    /* Found a free element ? */
    if(bFound)
    {
      /* Get the pointer on the cell according to index value and cells size */
      pCell = (void *)(((orxU8 *)pstCurrentSegment->pSegmentData) + _pstBank->u32ElemSize * ((u32MapPartIndex << 5) + u32BitIndex));

      /* Decrease the number of free elements */
      pstCurrentSegment->u32NbFree--;

      /* Updates bank counter */
      _pstBank->u32Counter++;

      /* Set the bit as used */
      ((orxU32 *)(pstCurrentSegment->au32CellAllocationMap))[u32MapPartIndex] |= 1 << u32BitIndex;

      /* Updates item ID */
      *_pu32ItemIndex = (u32SegmentIndex * (orxU32)_pstBank->u16NbCellPerSegments) + (u32MapPartIndex << 5) + u32BitIndex;

#ifdef __orxMEMORY_DEBUG__
      {
        orxU32 r = 0, i, j;

        for(i = 0; i < _pstBank->u16SizeSegmentBitField; i++)
        {
          for(j = 0; j < 32; j++)
          {
            if((1 << j) & pstCurrentSegment->pu32CellAllocationMap[i])
            {
              r++;
            }
          }
        }

        orxASSERT(r == (orxU32)(_pstBank->u16NbCellPerSegments - pstCurrentSegment->u32NbFree));
      }
#endif /* __orxMEMORY_DEBUG__ */
    }
  }

  /* Profiles */
  orxPROFILER_POP_MARKER();

  /* Done! */
  return pCell;
}

/** Free an allocated cell
 * @param[in] _pstBank    Bank of memory from where _pCell has been allocated
 * @param[in] _pCell      Pointer on the cell to free
 * @return a new cell of memory (orxNULL if no allocation possible)
 */
void orxFASTCALL orxBank_Free(orxBANK *_pstBank, void *_pCell)
{
  orxBANK_SEGMENT *pstSegment;  /* Segment associated to the cell */
  orxU32 u32Index32Bits;        /* Index of 32 the bits data */
  orxU32 u32IndexBit;           /* Index of the bit in u32Index32Bits */
  orxU32 u32CellIndex;          /* Difference in pointers address */

  /* Profiles */
  orxPROFILER_PUSH_MARKER("orxBank_Free");

  /* Module initialized ? */
  orxASSERT((sstBank.u32Flags & orxBANK_KU32_STATIC_FLAG_READY) == orxBANK_KU32_STATIC_FLAG_READY);

  /* Correct parameters ? */
  orxASSERT(_pstBank != orxNULL);
  orxASSERT(_pCell != orxNULL);

  /* Look for the segment associated to this cell */
  pstSegment = orxBank_GetSegment(_pstBank, _pCell);
  orxASSERT(pstSegment != orxNULL);

  /* (The address of _pCell can not be smaller than the address of pstSegment->pSegmentData */
  orxASSERT(_pCell >= pstSegment->pSegmentData);

  /* Retrieve the cell index in the bitfield computing position with cell address */
  u32CellIndex    = (orxU32)((orxU8 *)_pCell - (orxU8 *)pstSegment->pSegmentData) / _pstBank->u32ElemSize;
  u32Index32Bits  = u32CellIndex >> 5;
  u32IndexBit     = u32CellIndex & 31;

  /* Set cell as Free */
  pstSegment->au32CellAllocationMap[u32Index32Bits] &= ~(1 << u32IndexBit);

  /* Increase the number of free elements */
  pstSegment->u32NbFree++;

  /* Updates bank counter */
  _pstBank->u32Counter--;

  /* Profiles */
  orxPROFILER_POP_MARKER();

  /* Done! */
  return;
}

/** Free all allocated cell from a bank
 * @param[in] _pstBank    Bank of memory to clear
 */
void orxFASTCALL orxBank_Clear(orxBANK *_pstBank)
{
  orxBANK_SEGMENT *pstSegment;

  /* Module initialized ? */
  orxASSERT((sstBank.u32Flags & orxBANK_KU32_STATIC_FLAG_READY) == orxBANK_KU32_STATIC_FLAG_READY);

  /* Correct parameters ? */
  orxASSERT(_pstBank != orxNULL);

  /* For all segments */
  for(pstSegment = _pstBank->pstFirstSegment;
      pstSegment != orxNULL;
      pstSegment = pstSegment->pstNext)
  {
    /* Clears it */
    pstSegment->u32NbFree = (orxU32)_pstBank->u16NbCellPerSegments;
    orxMemory_Zero(pstSegment->au32CellAllocationMap, _pstBank->u16SizeSegmentBitField * sizeof(orxU32));
  }

  /* Clears bank counter */
  _pstBank->u32Counter = 0;
}

/** Compacts a bank by removing all its unused segments
 * @param[in] _pstBank    Bank of memory to compact
 */
void orxFASTCALL orxBank_Compact(orxBANK *_pstBank)
{
  orxBANK_SEGMENT *pstSegment, *pstPreviousSegment;

  /* Checks */
  orxASSERT(sstBank.u32Flags & orxBANK_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBank != orxNULL);

  /* For all segments */
  for(pstPreviousSegment = _pstBank->pstFirstSegment, pstSegment = pstPreviousSegment->pstNext;
      pstSegment != orxNULL;
      pstPreviousSegment = pstSegment, pstSegment = pstSegment->pstNext)
  {
    /* Is empty? */
    if(pstSegment->u32NbFree == (orxU32)_pstBank->u16NbCellPerSegments)
    {
      /* Updates previous segment's next */
      pstPreviousSegment->pstNext = pstSegment->pstNext;

      /* Frees it */
      orxMemory_Free(pstSegment);

      /* Reverts back to previous */
      pstSegment = pstPreviousSegment;
    }
  }
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

/** Get the next cell
 * @param[in] _pstBank    Bank of memory from where _pCell has been allocated
 * @param[in] _pCell      Pointer on the current cell of memory
 * @return The next cell. If _pCell is orxNULL, the first cell will be returned. Returns orxNULL when no more cell can be returned.
 */
void *orxFASTCALL orxBank_GetNext(const orxBANK *_pstBank, const void *_pCell)
{
  orxBANK_SEGMENT *pstSegment;
  orxU32          u32ByteIndex, u32Mask;

  /* Module initialized ? */
  orxASSERT((sstBank.u32Flags & orxBANK_KU32_STATIC_FLAG_READY) == orxBANK_KU32_STATIC_FLAG_READY);

  /* Correct parameters ? */
  orxASSERT(_pstBank != orxNULL);

  /* Has elements */
  if(_pstBank->u32Counter > 0)
  {
    /* Look for the segment associated to this cell */
    if(_pCell == orxNULL)
    {
      pstSegment = _pstBank->pstFirstSegment;

      /* Get the first bit index */
      u32ByteIndex  = 0;
      u32Mask       = 0xFFFFFFFF;
    }
    else
    {
      orxU32 u32CellIndex;

      /* Gets segment */
      pstSegment = orxBank_GetSegment(_pstBank, _pCell);

      /* Get a valid segment ? */
      orxASSERT(pstSegment != orxNULL);

      /* Compute the cell bit index */
      u32CellIndex    = (orxU32)((orxU8 *)_pCell - (orxU8 *)pstSegment->pSegmentData) / _pstBank->u32ElemSize;
      u32ByteIndex    = u32CellIndex >> 5;
      u32Mask         = ((u32CellIndex & 31) == 31) ? 0 : ~((1 << ((u32CellIndex & 31) + 1)) - 1);
    }

    /* Loop on bank segments while not found */
    for(;pstSegment != orxNULL; pstSegment = pstSegment->pstNext, u32ByteIndex = 0)
    {
      /* Loop on segment bitfields while not found */
      for(;u32ByteIndex < _pstBank->u16SizeSegmentBitField; u32ByteIndex++, u32Mask = 0xFFFFFFFF)
      {
        orxU32 u32MaskedMap;

        /* Gets masked map */
        u32MaskedMap = pstSegment->au32CellAllocationMap[u32ByteIndex] & u32Mask;

        /* Has remaining elements? */
        if(u32MaskedMap != 0)
        {
          /* The cell is on pSegment, on the bitfield u32Index32Bits and on the bit u32IndexBit */
          return (void *)(((orxU8 *)pstSegment->pSegmentData) + (((u32ByteIndex << 5) + orxMath_GetTrailingZeroCount(u32MaskedMap)) * _pstBank->u32ElemSize));
        }
      }
    }
  }

  /* Not found */
  return orxNULL;
}

/** Gets the cell's index
 * @param[in] _pstBank    Concerned memory bank
 * @param[in] _pCell      Cell of which we want the index
 * @return The index of the given cell if allocated, orxU32_UNDEFINED otherwise
 */
orxU32 orxFASTCALL orxBank_GetIndex(const orxBANK *_pstBank, const void *_pCell)
{
  orxBANK_SEGMENT  *pstSegment;
  orxU32            u32CellIndex, u32Size, u32SegmentCounter, u32Result = orxU32_UNDEFINED;

  /* Checks */
  orxASSERT((sstBank.u32Flags & orxBANK_KU32_STATIC_FLAG_READY) == orxBANK_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBank != orxNULL);
  orxASSERT(_pCell != orxNULL);

  /* For all segments */
  for(pstSegment = _pstBank->pstFirstSegment, u32Size = _pstBank->u32ElemSize * (orxU32)_pstBank->u16NbCellPerSegments, u32SegmentCounter = 0;
      pstSegment != orxNULL;
      pstSegment = pstSegment->pstNext, u32SegmentCounter++)
  {
    /* Is cell in segment? */
    if((_pCell >= pstSegment->pSegmentData)
    && (((orxU32)((orxU8 *)_pCell - (orxU8 *)pstSegment->pSegmentData)) < u32Size))
    {
      /* Stops */
      break;
    }
  }

  /* Checks */
  orxASSERT(pstSegment != orxNULL);

  /* Updates result */
  u32CellIndex = (orxU32)((orxU8 *)_pCell - (orxU8 *)pstSegment->pSegmentData) / _pstBank->u32ElemSize;

  /* Valid? */
  if(pstSegment->au32CellAllocationMap[u32CellIndex >> 5] & (1 << (u32CellIndex & 31)))
  {
    /* Updates result */
    u32Result = (u32SegmentCounter * (orxU32)_pstBank->u16NbCellPerSegments) + u32CellIndex;
  }

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
  void *pResult = orxNULL;

  /* Checks */
  orxASSERT((sstBank.u32Flags & orxBANK_KU32_STATIC_FLAG_READY) == orxBANK_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBank != orxNULL);

  /* Non empty? */
  if(_pstBank->u32Counter > 0)
  {
    orxBANK_SEGMENT  *pstSegment;
    orxU32            u32Index;

    /* Skips all segments till we hit the right one */
    for(u32Index = _u32Index, pstSegment = _pstBank->pstFirstSegment; (u32Index >= (orxU32)_pstBank->u16NbCellPerSegments) && (pstSegment != orxNULL); u32Index -= (orxU32)_pstBank->u16NbCellPerSegments, pstSegment = pstSegment->pstNext);

    /* Is segment valid? */
    if(pstSegment != orxNULL)
    {
      /* Checks */
      orxASSERT((u32Index >> 5) < _pstBank->u16SizeSegmentBitField);

      /* Is cell allocated? */
      if(pstSegment->au32CellAllocationMap[u32Index >> 5] & (1 << (u32Index & 31)))
      {
        /* Updates result */
        pResult = (void *)(((orxU8 *)pstSegment->pSegmentData) + (u32Index * _pstBank->u32ElemSize));
      }
    }
  }

  /* Done! */
  return pResult;
}

/** Gets the bank allocated cell counter
 * @param[in] _pstBank    Concerned bank
 * @return Number of allocated cells
 */
orxU32 orxFASTCALL orxBank_GetCounter(const orxBANK *_pstBank)
{
  /* Checks */
  orxASSERT((sstBank.u32Flags & orxBANK_KU32_STATIC_FLAG_READY) == orxBANK_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstBank != orxNULL);

  /* Done! */
  return _pstBank->u32Counter;
}

/*******************************************************************************
 * DEBUG FUNCTION
 ******************************************************************************/

#ifdef __orxDEBUG__

/** Print the content of a chunk bank
 * @param[in] _pstBank    Bank's pointer
 */
void orxFASTCALL orxBank_DebugPrint(const orxBANK *_pstBank)
{
  orxBANK_SEGMENT *pstSegment;  /* Pointer on the current segment */
  orxU32 u32Index1, u32Index2, u32Index3;

  /* Module initialized ? */
  orxASSERT((sstBank.u32Flags & orxBANK_KU32_STATIC_FLAG_READY) == orxBANK_KU32_STATIC_FLAG_READY);

  /* Correct parameters ? */
  orxASSERT(_pstBank != orxNULL);

  orxDEBUG_PRINT(orxDEBUG_LEVEL_MEMORY, "\n\n\n********* Bank (%x) *********", _pstBank);
  orxDEBUG_PRINT(orxDEBUG_LEVEL_MEMORY, "* u16NbCellPerSegments = %u", _pstBank->u16NbCellPerSegments);
  orxDEBUG_PRINT(orxDEBUG_LEVEL_MEMORY, "* u32ElemSize = %u", _pstBank->u32ElemSize);
  orxDEBUG_PRINT(orxDEBUG_LEVEL_MEMORY, "* u32Flags = %x", _pstBank->u32Flags);
  orxDEBUG_PRINT(orxDEBUG_LEVEL_MEMORY, "* eMemType = %u", _pstBank->eMemType);
  orxDEBUG_PRINT(orxDEBUG_LEVEL_MEMORY, "* u16SizeSegmentBitField = %u", _pstBank->u16SizeSegmentBitField);

  orxDEBUG_PRINT(orxDEBUG_LEVEL_MEMORY, "\n* **** SEGMENTS ******");

  pstSegment = _pstBank->pstFirstSegment;

  while(pstSegment != orxNULL)
  {
    orxDEBUG_PRINT(orxDEBUG_LEVEL_MEMORY, "\n* ** Segment (%x) ***", pstSegment);
    orxDEBUG_PRINT(orxDEBUG_LEVEL_MEMORY, "* u32NbFree = %u", pstSegment->u32NbFree);
    orxDEBUG_PRINT(orxDEBUG_LEVEL_MEMORY, "* pstNext = %x", pstSegment->pstNext);
    orxDEBUG_PRINT(orxDEBUG_LEVEL_MEMORY, "* pSegmentData = %x", pstSegment->pSegmentData);

    for(u32Index1 = 0; u32Index1 < _pstBank->u16SizeSegmentBitField; u32Index1++)
    {
      orxCHAR zBuffer[4096];

      orxMemory_Zero(zBuffer, 4096 * sizeof(orxCHAR));

      orxString_Print(zBuffer, "Bits : %02d (%x) = ", u32Index1, pstSegment->au32CellAllocationMap[u32Index1]);

      for(u32Index2 = 0; u32Index2 < 4; u32Index2++)
      {
        orxString_Print(zBuffer, "[");
        for(u32Index3 = 0; u32Index3 < 8; u32Index3++)
        {
          if((pstSegment->au32CellAllocationMap[u32Index1] & (orxU32)(1 << ((u32Index2 << 3) + u32Index3))) == (orxU32)(1 << ((u32Index2 << 3) + u32Index3)))
          {
            orxString_Print(zBuffer, "1");
          }
          else
          {
            orxString_Print(zBuffer, "0");
          }

          if(u32Index3 == 3)
          {
            orxString_Print(zBuffer, " ");
          }
        }
        orxString_Print(zBuffer, "]");
      }

      orxDEBUG_PRINT(orxDEBUG_LEVEL_MEMORY, "%s", zBuffer);
    }

    /* Go to next segment */
    pstSegment = pstSegment->pstNext;
  }
}

#endif /* __orxDEBUG__ */
