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
#include "utils/orxLinkList.h"
#include "utils/orxString.h"

#define orxBANK_KU32_STATIC_FLAG_NONE         0x00000000  /**< No flags have been set */
#define orxBANK_KU32_STATIC_FLAG_READY        0x00000001  /**< The module has been initialized */


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/
typedef struct __orxBANK_SEGMENT_t
{
  orxU32                     *pu32CellAllocationMap; /**< List of bits that represents free and used elements in the segment */
  void                       *pSegmentData;     /**< Pointer address on the head of the segment data cells */
  struct __orxBANK_SEGMENT_t *pstNext;          /**< Pointer on the next segment */
  orxU16                      u16NbFree;        /**< Number of free elements in the segment */

} orxBANK_SEGMENT;

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
  orxU32 u32SegmentSize;        /* Size of segment allocation */

  /* Module initialized ? */
  orxASSERT((sstBank.u32Flags & orxBANK_KU32_STATIC_FLAG_READY) == orxBANK_KU32_STATIC_FLAG_READY);

  /* Correct parameters ? */
  orxASSERT(_pstBank != orxNULL);

  /* Compute the segment size */
  u32SegmentSize = sizeof(orxBANK_SEGMENT) +                                /* Size of the structure */
                   _pstBank->u16SizeSegmentBitField * sizeof(orxU32) +      /* Size of bitfields */
                   _pstBank->u16NbCellPerSegments * _pstBank->u32ElemSize;  /* Size of stored Data */

  /* Allocate a new segent of memory */
  pstSegment = (orxBANK_SEGMENT *)orxMemory_Allocate(u32SegmentSize, _pstBank->eMemType);
  if(pstSegment != orxNULL)
  {
    /* Set initial segment values */
    orxMemory_Zero(pstSegment, u32SegmentSize);
    pstSegment->pstNext               = orxNULL;
    pstSegment->u16NbFree             = _pstBank->u16NbCellPerSegments;
    pstSegment->pu32CellAllocationMap = (orxU32 *)(((orxU8 *)pstSegment) + sizeof(orxBANK_SEGMENT));
    pstSegment->pSegmentData          = (void *)(((orxU8 *)pstSegment->pu32CellAllocationMap) + (_pstBank->u16SizeSegmentBitField * sizeof(orxU32)));
  }

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
    pstBank->u32ElemSize              = _u32Size;
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
  void *pCell = orxNULL;   /* Returned cell */
  orxBANK_SEGMENT *pstCurrentSegment;

  /* Module initialized ? */
  orxASSERT((sstBank.u32Flags & orxBANK_KU32_STATIC_FLAG_READY) == orxBANK_KU32_STATIC_FLAG_READY);

  /* Correct parameters ? */
  orxASSERT(_pstBank != orxNULL);

  /* If the current segment has free cells, try to go on the first segment with free cell found */
  pstCurrentSegment = _pstBank->pstFirstSegment;
  while((pstCurrentSegment->pstNext != orxNULL) && (pstCurrentSegment->u16NbFree == 0))
  {
    pstCurrentSegment = pstCurrentSegment->pstNext;
  }

  /* Is there a free space in the current segment ? (If no, try to expand it if allowed) */
  if((pstCurrentSegment->u16NbFree == 0) && (!(_pstBank->u32Flags & orxBANK_KU32_FLAG_NOT_EXPANDABLE)))
  {
    /* No, Try to allocate a new segment */
    pstCurrentSegment->pstNext = orxBank_CreateSegment(_pstBank);

    /* Correct segment allocation ? */
    if(pstCurrentSegment->pstNext != orxNULL)
    {
      /* Set the new allocated segment as the current selected one */
      pstCurrentSegment = pstCurrentSegment->pstNext;
    }
  }

  /* Does the current segment has free nodes ? (yes : the allocation was correct (if there was), else returns orxNULL) */
  if(pstCurrentSegment->u16NbFree > 0)
  {
    orxU32  u32BitIndex, u32MapPartIndex;
    orxBOOL bFound;

    /* For all map parts */
    for(u32BitIndex = 0, u32MapPartIndex = 0, bFound = orxFALSE; !bFound && (u32MapPartIndex < (orxU32)_pstBank->u16SizeSegmentBitField); u32MapPartIndex++)
    {
      orxS32 s32MapPartValue;

      /* Gets the available slots */
      s32MapPartValue = ~(pstCurrentSegment->pu32CellAllocationMap[u32MapPartIndex]);

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
      pCell = (void *)(((orxU8 *)pstCurrentSegment->pSegmentData) + (_pstBank->u32ElemSize * u32MapPartIndex << 5) + (_pstBank->u32ElemSize * u32BitIndex));

      /* Decrease the number of free elements */
      pstCurrentSegment->u16NbFree--;

      /* Updates bank counter */
      _pstBank->u32Counter++;

      /* Set the bit as used */
      ((orxU32*)(pstCurrentSegment->pu32CellAllocationMap))[u32MapPartIndex] |= 1 << u32BitIndex;
      
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

        orxASSERT(r == (orxU32)(_pstBank->u16NbCellPerSegments - pstCurrentSegment->u16NbFree));
      }
#endif /* __orxMEMORY_DEBUG__ */
    }
  }

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

  /* Module initialized ? */
  orxASSERT((sstBank.u32Flags & orxBANK_KU32_STATIC_FLAG_READY) == orxBANK_KU32_STATIC_FLAG_READY);

  /* Correct parameters ? */
  orxASSERT(_pstBank != orxNULL);
  orxASSERT(_pCell != orxNULL);

  /* Look for the segment associated to this cell */
  pstSegment = orxBank_GetSegment(_pstBank, _pCell);
  orxASSERT(pstSegment != orxNULL);

  /* (The address of _pCell can not be smaller than the adress of pstSegment->pSegmentData */
  orxASSERT(_pCell >= pstSegment->pSegmentData);

  /* Retrieve the cell index in the bitfield computing position with cell address */
  u32CellIndex    = (orxU32)((orxU8 *)_pCell - (orxU8 *)pstSegment->pSegmentData) / _pstBank->u32ElemSize;
  u32Index32Bits  = u32CellIndex >> 5;
  u32IndexBit     = u32CellIndex & 31;

  /* Set cell as Free */
  pstSegment->pu32CellAllocationMap[u32Index32Bits] &= ~(1 << u32IndexBit);

  /* Increase the number of free elements */
  pstSegment->u16NbFree++;

  /* Updates bank counter */
  _pstBank->u32Counter--;
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
    pstSegment->u16NbFree = _pstBank->u16NbCellPerSegments;
    orxMemory_Zero(pstSegment->pu32CellAllocationMap, _pstBank->u16SizeSegmentBitField * sizeof(orxU32));
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
    if(pstSegment->u16NbFree == _pstBank->u16NbCellPerSegments)
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
  orxBANK_SEGMENT *pstSegment;  /* Segment associated to the cell */
  orxU32 u32Index32Bits;        /* Index of 32 the bits data */
  orxS32 s32IndexBit;           /* Index of the bit in u32Index32Bits */
  orxU32 u32CellIndex;          /* Difference in pointers address */

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
      u32Index32Bits  = 0;
      s32IndexBit     = 0;
    }
    else
    {
      /* Gets segment */
      pstSegment = orxBank_GetSegment(_pstBank, _pCell);

      /* Get a valid segment ? */
      orxASSERT(pstSegment != orxNULL);

      /* Compute the cell bit index */
      u32CellIndex    = (orxU32)((orxU8 *)_pCell - (orxU8 *)pstSegment->pSegmentData) / _pstBank->u32ElemSize;
      u32Index32Bits  = u32CellIndex >> 5;
      s32IndexBit     = (u32CellIndex & 31) + 1;
    }

    /* Loop on bank segments while not found */
    for(;pstSegment != orxNULL; pstSegment = pstSegment->pstNext, u32Index32Bits = 0)
    {
      /* Loop on segment bitfields while not found */
      for(;u32Index32Bits < _pstBank->u16SizeSegmentBitField; u32Index32Bits++, s32IndexBit = 0)
      {
        orxU32 u32Mask;

        /* Loop on bits while not found */
        for(u32Mask = pstSegment->pu32CellAllocationMap[u32Index32Bits] >> s32IndexBit; (u32Mask != 0) && (s32IndexBit < 32); u32Mask >>= 1, s32IndexBit++)
        {
          /* Found? */
          if(u32Mask & (orxU32)1)
          {
            /* The cell is on pSegment, on the bitfield u32Index32Bits and on the bit u32IndexBit */
            return (void *)(((orxU8 *)pstSegment->pSegmentData) + (((u32Index32Bits << 5) + s32IndexBit) * _pstBank->u32ElemSize));
          }
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
  if(pstSegment->pu32CellAllocationMap[u32CellIndex >> 5] & (1 << (u32CellIndex & 31)))
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
    for(u32Index = _u32Index, pstSegment = _pstBank->pstFirstSegment; (u32Index > (orxU32)_pstBank->u16NbCellPerSegments) && (pstSegment != orxNULL); u32Index -= (orxU32)_pstBank->u16NbCellPerSegments, pstSegment = pstSegment->pstNext);

    /* Is segment valid? */
    if(pstSegment != orxNULL)
    {
      /* Checks */
      orxASSERT((u32Index >> 5) < _pstBank->u16SizeSegmentBitField);

      /* Is cell allocated? */
      if(pstSegment->pu32CellAllocationMap[u32Index >> 5] & (1 << (u32Index & 31)))
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
    orxDEBUG_PRINT(orxDEBUG_LEVEL_MEMORY, "* u16NbFree = %u", pstSegment->u16NbFree);
    orxDEBUG_PRINT(orxDEBUG_LEVEL_MEMORY, "* pstNext = %x", pstSegment->pstNext);
    orxDEBUG_PRINT(orxDEBUG_LEVEL_MEMORY, "* pSegmentData = %x", pstSegment->pSegmentData);

    for(u32Index1 = 0; u32Index1 < _pstBank->u16SizeSegmentBitField; u32Index1++)
    {
      orxCHAR zBuffer[4096];

      orxMemory_Zero(zBuffer, 4096 * sizeof(orxCHAR));

      orxString_Print(zBuffer, "Bits : %02d (%x) = ", u32Index1, pstSegment->pu32CellAllocationMap[u32Index1]);

      for(u32Index2 = 0; u32Index2 < 4; u32Index2++)
      {
        orxString_Print(zBuffer, "[");
        for(u32Index3 = 0; u32Index3 < 8; u32Index3++)
        {
          if((pstSegment->pu32CellAllocationMap[u32Index1] & (orxU32)(1 << ((u32Index2 << 3) + u32Index3))) == (orxU32)(1 << ((u32Index2 << 3) + u32Index3)))
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
