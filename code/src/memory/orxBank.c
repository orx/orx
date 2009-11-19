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
 * @file orxBank.c
 * @date 02/04/2005
 * @author bestel@arcallians.org
 *
 */


#include "memory/orxBank.h"
#include "debug/orxDebug.h"
#include "utils/orxString.h"

#define orxBANK_KU32_STATIC_FLAG_NONE         0x00000000  /**< No flags have been set */
#define orxBANK_KU32_STATIC_FLAG_READY        0x00000001  /**< The module has been initialized */

#define orxBANK_KU32_UNALLOCATION_HYSTERESIS  5           /**< The next segment will be unallocated when the current */
                                                          /**< segment will have orxBANK_KU32_UNALLOCATION_HYSTERESYS */
                                                          /**< free cells */

/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/
typedef struct __orxBANK_SEGMENT_t
{
  orxU32                     *pu32FreeElemBits; /**< List of bits that represents free and used elements in the segment */
  void                    *pSegmentData;      /**< Pointer address on the head of the segment data cells */
  struct __orxBANK_SEGMENT_t *pstNext;          /**< Pointer on the next segment */
  orxU16                      u16NbFree;        /**< Number of free elements in the segment */

} orxBANK_SEGMENT;

struct __orxBANK_t
{
  orxBANK_SEGMENT  *pstFirstSegment;        /**< First segment used in the bank */
  orxU32            u32Flags;               /**< Flags set for the memory bank */
  orxMEMORY_TYPE    eMemType;               /**< Memory type that will be used by the memory allocation */
  orxU32            u32ElemSize;            /**< Size of a cell */
  orxU16            u16NbCellPerSegments;   /**< Number of cells per banks */
  orxU16            u16SizeSegmentBitField; /**< Number of u32 (4 bytes) to represent a segment */
  orxU32            u32Counter;             /**< Number of allocated cells */
};

typedef struct __orxBANK_STATIC_t
{
  orxU32 u32Flags;                      /**< Flags set by the memory module */

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
static orxINLINE orxBANK_SEGMENT *orxBank_SegmentCreate(const orxBANK *_pstBank)
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
    pstSegment->pstNext           = orxNULL;
    pstSegment->u16NbFree         = _pstBank->u16NbCellPerSegments;
    pstSegment->pu32FreeElemBits  = (orxU32 *)(((orxU8 *)pstSegment) + sizeof(orxBANK_SEGMENT));
    pstSegment->pSegmentData     = (void *)(((orxU8 *)pstSegment->pu32FreeElemBits) + (_pstBank->u16SizeSegmentBitField * sizeof(orxU32)));
  }

  return pstSegment;
}

/** Free a segment of memory (and recursivly all next segments)
 * @param[in] _pstSegment  Allocated segment of memory
 */
static orxINLINE void orxBank_SegmentDelete(orxBANK_SEGMENT *_pstSegment)
{
  /* Module initialized ? */
  orxASSERT((sstBank.u32Flags & orxBANK_KU32_STATIC_FLAG_READY) == orxBANK_KU32_STATIC_FLAG_READY);

  /* Correct parameters ? */
  orxASSERT(_pstSegment != orxNULL);

  /* Are there another segments linked to this one ? */
  if(_pstSegment->pstNext != orxNULL)
  {
    /* Yes, free it */
    orxBank_SegmentDelete(_pstSegment->pstNext);
  }

  /* Free the current segment */
  orxMemory_Free(_pstSegment);
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
    /* In cell in segment? */
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
    pstBank->eMemType                 = _eMemType;
    pstBank->u32ElemSize              = _u32Size;
    pstBank->u32Flags                 = _u32Flags;
    pstBank->u16NbCellPerSegments     = _u16NbElem;
    pstBank->u32Counter               = 0;

    /* Compute the necessary number of 32 bits packs */
    pstBank->u16SizeSegmentBitField   = (orxU16)orxMemory_GetAlign(_u16NbElem, 32) >> 5;

    /* Allocate the first segment, and select it as current */
    pstBank->pstFirstSegment    = orxBank_SegmentCreate(pstBank);

    /* No allocation problem ? */
    if(pstBank->pstFirstSegment == orxNULL)
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
  /* Module initialized ? */
  orxASSERT((sstBank.u32Flags & orxBANK_KU32_STATIC_FLAG_READY) == orxBANK_KU32_STATIC_FLAG_READY);

  /* Correct parameters ? */
  orxASSERT(_pstBank != orxNULL);

  /* Delete segment(s) (This segment is the first one, it can't be orxNULL) */
  orxBank_SegmentDelete(_pstBank->pstFirstSegment);

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
    pstCurrentSegment->pstNext = orxBank_SegmentCreate(_pstBank);

    /* Correct segment allocation ? */
    if(pstCurrentSegment->pstNext != orxNULL)
    {
      /* Set the new allocated segment as the current selected one */
      pstCurrentSegment = pstCurrentSegment->pstNext;
    }
  }

  /* Is the current segment has free nodes ? (yes : the allocation was correct (if there was), else returns orxNULL) */
  if(pstCurrentSegment->u16NbFree > 0)
  {
    orxU32 u32Index32Bits;          /* Index between 0 and _pstBank->u32SizeSegmentBitField */
    orxU32 u32Index8Bits;           /* Index to traverse a 32 bit field (indexed by u32Index32Bits) */
    orxU32 u32BitResultIndex   = 0; /* Position of the free bit found */
    orxU32 u32FieldResultIndex = 0; /* Index of the bit field to use */
    orxBOOL bFound = orxFALSE;      /* Set as orxTRUE, when the bit position will be found */

    /* Look for the first free cell (use precomputed array to improve search speed) */
    /* Loop on the segment bits */
    for(u32Index32Bits = 0; !bFound && (u32Index32Bits < _pstBank->u16SizeSegmentBitField); u32Index32Bits++)
    {
      u32BitResultIndex = 0;
      for(u32Index8Bits = 0; !bFound && (u32Index8Bits < 4); u32Index8Bits++)
      {
        orxS32 s328BitsValue;

        /* Get the 8 bits to check */
        s328BitsValue = ~(((pstCurrentSegment->pu32FreeElemBits[u32Index32Bits]) >> (u32Index8Bits << 3))) & 0xFF;

        /* Get the array index value (take 8 bits from the segment) */
        /* All bits used ? */
        if(s328BitsValue != 0)
        {
          /* Gets starting index */
          u32BitResultIndex = orxMath_GetBitCount((s328BitsValue & -s328BitsValue) - 1);

          /* Compute the global position index the segment */
          u32BitResultIndex  += (u32Index8Bits << 3);
          u32FieldResultIndex = u32Index32Bits;

          /* Found ! */
          bFound = orxTRUE;
        }
      }
    }

    /* If bFound is false, It means that there are no more free segments that we can allocate.
     * It can be volunteer (orxBANK_KU32_FLAG_NOT_EXPANDABLE) or a problem in the code => assert
     */
    orxASSERT(bFound || (!bFound && ((_pstBank->u32Flags & orxBANK_KU32_FLAG_NOT_EXPANDABLE) == orxBANK_KU32_FLAG_NOT_EXPANDABLE)));

    /* Found a free element ? */
    if(bFound)
    {
      /* Get the pointer on the cell according to index value and cells size */
      pCell = (void *)(((orxU8 *)pstCurrentSegment->pSegmentData) + (_pstBank->u32ElemSize * u32FieldResultIndex << 5) + (_pstBank->u32ElemSize * u32BitResultIndex));

      /* Decrease the number of free elements */
      pstCurrentSegment->u16NbFree--;

      /* Updates bank counter */
      _pstBank->u32Counter++;

      /* Set the bit as used */
      ((orxU32*)(pstCurrentSegment->pu32FreeElemBits))[u32FieldResultIndex] |= 1 << u32BitResultIndex;
      
#ifdef __orxDEBUG__
      {
        orxU32 r = 0, i, j;

        for(i = 0; i < _pstBank->u16SizeSegmentBitField; i++)
        {
          for(j = 0; j < 32; j++)
          {
            if((1 << j) & pstCurrentSegment->pu32FreeElemBits[i])
            {
              r++;
            }
          }
        }

        orxASSERT(r == (orxU32)(_pstBank->u16NbCellPerSegments - pstCurrentSegment->u16NbFree));
      }
#endif /* __orxDEBUG__ */
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
  orxU32 u32CellIndex;          /* Difference in pointers adress */

  /* Module initialized ? */
  orxASSERT((sstBank.u32Flags & orxBANK_KU32_STATIC_FLAG_READY) == orxBANK_KU32_STATIC_FLAG_READY);

  /* Correct parameters ? */
  orxASSERT(_pstBank != orxNULL);
  orxASSERT(_pCell != orxNULL);

  /* Look for the segment associated to this cell */
  pstSegment = orxBank_GetSegment(_pstBank, _pCell);
  orxASSERT(pstSegment != orxNULL);

  /* (The adress of _pCell can not be smaller than the adress of pstSegment->pSegmentData */
  orxASSERT(_pCell >= pstSegment->pSegmentData);

  /* Retrieve the cell index in the bitfield computing position with cell adress */
  u32CellIndex    = (orxU32)((orxU8 *)_pCell - (orxU8 *)pstSegment->pSegmentData) / _pstBank->u32ElemSize;
  u32Index32Bits  = u32CellIndex >> 5;
  u32IndexBit     = u32CellIndex & 31;

  /* Set cell as Free */
  pstSegment->pu32FreeElemBits[u32Index32Bits] &= ~(1 << u32IndexBit);

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
  void *pstCell = orxNULL;

  /* Module initialized ? */
  orxASSERT((sstBank.u32Flags & orxBANK_KU32_STATIC_FLAG_READY) == orxBANK_KU32_STATIC_FLAG_READY);

  /* Correct parameters ? */
  orxASSERT(_pstBank != orxNULL);

  /* Free all elements */
  while((pstCell = orxBank_GetNext(_pstBank, orxNULL)) != orxNULL)
  {
    orxBank_Free(_pstBank, pstCell);
  }
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
  orxU32 u32CellIndex;          /* Difference in pointers adress */

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
        for(u32Mask = pstSegment->pu32FreeElemBits[u32Index32Bits] >> s32IndexBit; (u32Mask != 0) && (s32IndexBit < 32); u32Mask >>= 1, s32IndexBit++)
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

      orxString_Print(zBuffer, "Bits : %02d (%x) = ", u32Index1, pstSegment->pu32FreeElemBits[u32Index1]);

      for(u32Index2 = 0; u32Index2 < 4; u32Index2++)
      {
        orxString_Print(zBuffer, "[");
        for(u32Index3 = 0; u32Index3 < 8; u32Index3++)
        {
          if((pstSegment->pu32FreeElemBits[u32Index1] & (orxU32)(1 << ((u32Index2 << 3) + u32Index3))) == (orxU32)(1 << ((u32Index2 << 3) + u32Index3)))
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
