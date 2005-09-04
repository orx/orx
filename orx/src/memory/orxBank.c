/**
 * @file orxBank.c
 * 
 * Memory Bank allocation / Unallocation module
 * 
 * @todo Add a clever method to unallocate segments.
 * @todo Optimisation for traverse
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

#include "memory/orxBank.h"
#include "debug/orxDebug.h"
#include "io/orxTextIO.h"

#define orxBANK_KU32_FLAG_NONE  0x00000000  /**< No flags have been set */
#define orxBANK_KU32_FLAG_READY 0x00000001  /**< The module has been initialized */

#define orxBANK_KU32_UNALLOCATION_HYSTERESIS  5 /**< The next segment will be unallocated when the current */
                                                /**< segment will have orxBANK_KU32_UNALLOCATION_HYSTERESYS */
                                                /**< free cells */

/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/
typedef struct __orxBANK_SEGMENT_t
{
  orxU32 u32NbFree;                     /**< Number of free elements in the segment */
  orxU32 *pu32FreeElemBits;             /**< List of bits that represents free and used elements in the segment */
  orxVOID *pSegmentDatas;               /**< Pointer address on the head of the segment data cells */
  struct __orxBANK_SEGMENT_t *pstNext;  /**< Pointer on the next segment */
} orxBANK_SEGMENT;
 
struct __orxBANK_t
{
  orxU32 u32NbCellPerSegments;        /**< Number of cells per banks */
  orxU32 u32ElemSize;                 /**< Size of a cell */
  orxU32 u32Flags;                    /**< Flags set for the memory bank */
  orxMEMORY_TYPE eMemType;            /**< Memory type that will be used by the memory allocation */
  orxU32 u32SizeSegmentBitField;      /**< Number of u32 (4 bytes) to represent a segment */
  orxBANK_SEGMENT *pstFirstSegment;   /**< First segment used in the bank */
};

typedef struct __orxBANK_STATIC_t
{
  orxU32 u32Flags;         /**< Flags set by the memory module */
  orxU8 au8Index0[256];    /**< Array of values that defines the index position of */
                           /**< the first flag set to 0 in a bit field */
} orxBANK_STATIC;

/***************************************************************************
 * Module global variable                                                  *
 ***************************************************************************/
orxSTATIC orxBANK_STATIC sstBank;

/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

/** Create a new segment of memory and returns a pointer on it
 * @param[in] _u32NbElem  Number of elements per segments
 * @param[in] _u32Size    Size of an element
 * @param[in] _eMemType   Memory type where the datas will be allocated
 * @return  returns a pointer on the memory segment (orxNULL if an error occured)
 */
orxBANK_SEGMENT *orxBank_SegmentCreate(orxBANK *_pstBank)
{
  orxBANK_SEGMENT *pstSegment;  /* Pointer on the segment of memory */
  orxU32 u32SegmentSize;        /* Size of segment allocation */

  /* Module initialized ? */
  orxASSERT((sstBank.u32Flags & orxBANK_KU32_FLAG_READY) == orxBANK_KU32_FLAG_READY);

  /* Correct parameters ? */
  orxASSERT(_pstBank != orxNULL);
  
  /* Compute the segment size */
  u32SegmentSize = sizeof(orxBANK_SEGMENT) +                                /* Size of the structure */
                   _pstBank->u32SizeSegmentBitField * sizeof(orxU32) +      /* Size of bitfields */
                   _pstBank->u32NbCellPerSegments * _pstBank->u32ElemSize;  /* Size of stored datas */
  
  /* Allocate a new segent of memory */
  pstSegment = (orxBANK_SEGMENT *)orxMemory_Allocate(u32SegmentSize, _pstBank->eMemType);
  if (pstSegment != orxNULL)
  {
    /* Set initial segment values */
    orxMemory_Set(pstSegment, 0, u32SegmentSize);
    pstSegment->pstNext           = orxNULL;
    pstSegment->u32NbFree         = _pstBank->u32NbCellPerSegments;
    pstSegment->pu32FreeElemBits  = (orxU32 *)(((orxU8 *)pstSegment) + sizeof(orxBANK_SEGMENT));
    pstSegment->pSegmentDatas     = (orxVOID *)(((orxU8 *)pstSegment->pu32FreeElemBits) + (_pstBank->u32SizeSegmentBitField * sizeof(orxU32)));
  }
  
  return pstSegment;
}

/** Free a segment of memory (and recursivly all next segments)
 * @param[in] _pstSegment  Allocated segment of memory
 */
orxVOID orxBank_SegmentDelete(orxBANK_SEGMENT *_pstSegment)
{
  /* Module initialized ? */
  orxASSERT((sstBank.u32Flags & orxBANK_KU32_FLAG_READY) == orxBANK_KU32_FLAG_READY);

  /* Correct parameters ? */
  orxASSERT(_pstSegment != orxNULL);
  
  /* Are there another segments linked to this one ? */
  if (_pstSegment->pstNext != orxNULL)
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
orxBANK_SEGMENT *orxBank_GetSegment(orxBANK *_pstBank, orxVOID *_pCell)
{
  orxBANK_SEGMENT *pstSegment;        /* Returned segment */
  orxU8 *pStartAddress;               /* Start address of the segment */
  orxU8 *pEndAddress;                 /* End address of the segment */
  orxBOOL bFound;                     /* orxTRUE when segment found */
  
  /* Module initialized ? */
  orxASSERT((sstBank.u32Flags & orxBANK_KU32_FLAG_READY) == orxBANK_KU32_FLAG_READY);

  /* Correct parameters ? */
  orxASSERT(_pstBank != orxNULL);
  orxASSERT(_pCell != orxNULL);
  
  /* Starts Look up with the first segment */
  pstSegment = _pstBank->pstFirstSegment;
 
  /* Traverses segment list and compare adresses */
  bFound = orxFALSE;
  while (!bFound && (pstSegment != orxNULL))
  {
    /* Set Start and End address */
    pStartAddress = pstSegment->pSegmentDatas;
    pEndAddress   = pStartAddress + (orxU32)(orxU8 *)(_pstBank->u32ElemSize * _pstBank->u32NbCellPerSegments);
    
    /* Is cell in segment data range ? */
    if (((orxU8 *)_pCell >= pStartAddress) && ((orxU8 *)_pCell < pEndAddress))
    {
      /* Yes, we found it */
      bFound = orxTRUE;
    }
    else
    {
      /* Try with the next segment */
      pstSegment = pstSegment->pstNext;
    }
  }
  
  /* Not found ? returns orxNULL */
  if (!bFound)
  {
    pstSegment = orxNULL;
  }
  
  return pstSegment;
}

/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/
/** Initialize Bank Module
 */
orxSTATUS orxBank_Init()
{
  /* Declare variables */
  orxU32 u32Index; /* Array index (values where 0 has to be found ) */
  orxU8 u8Bit;     /* Bit to test */
  orxU8 u8Test;    /* Bitfield used to do the test (bit 1 set on u8Bit) */
  orxBOOL bFound;  /* set to orxTRUE when the bit has been found */

  orxSTATUS eResult = orxSTATUS_FAILED;
  
  /* Init dependencies */
  if ((orxDEPEND_INIT(Depend) &
       orxDEPEND_INIT(Memory)) == orxSTATUS_SUCCESS)
  {
    /* Not already Initialized? */
    if(!(sstBank.u32Flags & orxBANK_KU32_FLAG_READY))
    {
      /* Cleans static controller */
      orxMemory_Set(&sstBank, 0, sizeof(orxBANK_STATIC));
      
      /* Set initial values */
      /* Compute the array of 0 index for each values between 0 and 255 */
      for (u32Index = 0; u32Index < 256; u32Index++)
      {
        /* Set the initial value of the index (not defined yet) */
        sstBank.au8Index0[u32Index] = orxU8_Undefined;
        bFound = orxFALSE;
        u8Test = 1;
        
        /* Loop on each bit to find the first equals to 0 */
        for (u8Bit = 0; !bFound && (u8Bit < 8); u8Bit++)
        {
          /* Is & between Index flag and the test flag == 0 ? (!= 0 means tested bit = 1) */
          if (((orxU8)u32Index & u8Test) == 0)
          {
            /* Bit 0 found, store the index and stop search */
            sstBank.au8Index0[u32Index] = u8Bit;
            bFound = orxTRUE;
          }
          
          /* Shift test flag bit to check the next one */
          u8Test <<= 1;
        } 
      }
      
      /* Set module has ready */
      sstBank.u32Flags = orxBANK_KU32_FLAG_READY;
      
      /* Success */
      eResult = orxSTATUS_SUCCESS;
    }
  }
  
  /* Done */
  return eResult;
}

/** Exit bank module
 */
orxVOID orxBank_Exit()
{
  /* Module initialized ? */
  if ((sstBank.u32Flags & orxBANK_KU32_FLAG_READY) == orxBANK_KU32_FLAG_READY)
  {
    /* Module not ready now */
    sstBank.u32Flags = orxBANK_KU32_FLAG_NONE;
  }

  /* Exit dependencies */
  orxDEPEND_EXIT(Memory);
  orxDEPEND_EXIT(Depend);
}

/** Create a new bank in memory and returns a pointer on it
 * @param[in] _u32NbElem  Number of elements per segments
 * @param[in] _u32Size    Size of an element
 * @param[in] _u32Flags   Flags set for this bank
 * @param[in] _eMemType   Memory type where the datas will be allocated
 * @return  returns a pointer on the memory bank
 */
orxBANK *orxBank_Create(orxU32 _u32NbElem, orxU32 _u32Size, orxU32 _u32Flags, orxMEMORY_TYPE _eMemType)
{
  /* Module initialized ? */
  orxASSERT((sstBank.u32Flags & orxBANK_KU32_FLAG_READY) == orxBANK_KU32_FLAG_READY);

  /* Correct parameters ? */
  orxASSERT(_eMemType < orxMEMORY_TYPE_NUMBER);
  orxASSERT(_u32NbElem > 0);
  orxASSERT(_u32Size > 0);
  
  /* Allocate the bank */
  orxBANK *pstBank = (orxBANK *)orxMemory_Allocate(sizeof(orxBANK), _eMemType);
  
  /* Memory allocated ? */
  if (pstBank != orxNULL)
  {
    /* Set initial values */
    orxMemory_Set(pstBank, 0, sizeof(orxBANK));
    pstBank->eMemType                 = _eMemType;
    pstBank->u32ElemSize              = _u32Size;
    pstBank->u32Flags                 = _u32Flags;
    pstBank->u32NbCellPerSegments     = _u32NbElem;

    /* Compute the necessary number of 32 bits packs */
    pstBank->u32SizeSegmentBitField   = orxMemory_GetAlign(_u32NbElem, 32) / 32;
    
    /* Allocate the first segment, and select it as current */
    pstBank->pstFirstSegment    = orxBank_SegmentCreate(pstBank);
    
    /* No allocation problem ? */
    if (pstBank->pstFirstSegment == orxNULL)
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
orxVOID orxBank_Delete(orxBANK *_pstBank)
{
  /* Module initialized ? */
  orxASSERT((sstBank.u32Flags & orxBANK_KU32_FLAG_READY) == orxBANK_KU32_FLAG_READY);

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
orxVOID *orxBank_Allocate(orxBANK *_pstBank)
{
  orxVOID *pCell = orxNULL;   /* Returned cell */
  orxBANK_SEGMENT *pstCurrentSegment;
  
  /* Module initialized ? */
  orxASSERT((sstBank.u32Flags & orxBANK_KU32_FLAG_READY) == orxBANK_KU32_FLAG_READY);

  /* Correct parameters ? */
  orxASSERT(_pstBank != orxNULL);
  
  /* If the current segment has free cells, try to go on the first segment with free cell found */
  pstCurrentSegment = _pstBank->pstFirstSegment;
  while ((pstCurrentSegment->pstNext != orxNULL) && (pstCurrentSegment->u32NbFree == 0))
  {
    pstCurrentSegment = pstCurrentSegment->pstNext;
  }
  
  /* Is there a free space in the current segment ? (If no, try to expand it if allowed) */
  if ((pstCurrentSegment->u32NbFree == 0) && (!(_pstBank->u32Flags & orxBANK_KU32_FLAGS_NOT_EXPANDABLE)))
  {
    /* No, Try to allocate a new segment */
    pstCurrentSegment->pstNext = orxBank_SegmentCreate(_pstBank);
    
    /* Correct segment allocation ? */
    if (pstCurrentSegment->pstNext != orxNULL)
    {
      /* Set the new allocated segment as the current selected one */
      pstCurrentSegment = pstCurrentSegment->pstNext;
    }
  }
  
  /* Is the current segment has free nodes ? (yes : the allocation was correct (if there was), else returns orxNULL) */
  if (pstCurrentSegment->u32NbFree > 0)
  {
    orxU32 u32Index32Bits;      /* Index between 0 and _pstBank->u32SizeSegmentBitField */
    orxU32 u32Index8Bits;       /* Index to traverse a 32 bit field (indexed by u32Index32Bits) */
    orxU32 u32BitResultIndex;   /* Position of the free bit found */
    orxU32 u32FieldResultIndex; /* Index of the bit field to use */
    orxBOOL bFound = orxFALSE;  /* Set as orxTRUE, when the bit position will be found */

    /* Look for the first free cell (use precomputed array to improve search speed) */
    /* Loop on the segment bits */
    for (u32Index32Bits = 0; !bFound && (u32Index32Bits < _pstBank->u32SizeSegmentBitField); u32Index32Bits++)
    {
      u32BitResultIndex = 0;
      for (u32Index8Bits = 0; !bFound && (u32Index8Bits < 4); u32Index8Bits++)
      {
        /* Get the 8 bits to check */
        orxU32 u328BitsValue = (pstCurrentSegment->pu32FreeElemBits[u32Index32Bits]) >> (u32Index8Bits * 4);
        u328BitsValue &= 0xFF;
        
        /* Get the array index value (take 8 bits from the segment) */
        /* All bits used ? */
        if (u328BitsValue < 0xFF)
        {
          /* Get the index (local)*/
          u32BitResultIndex = sstBank.au8Index0[u328BitsValue];
          
          /* Compute the global position index the segment */
          u32BitResultIndex  += (u32Index8Bits * 4);
          u32FieldResultIndex = u32Index32Bits;
          
          /* Found ! */
          bFound = orxTRUE;
        }
      }
    }
    
    /* If bFound is false, It means that there are no more free segments that we can allocate.
     * It can be volunteer (orxBANK_KU32_FLAGS_NOT_EXPANDABLE) or a problem in the code => assert
     */
    orxASSERT(bFound || (!bFound && ((_pstBank->u32Flags & orxBANK_KU32_FLAGS_NOT_EXPANDABLE) == orxBANK_KU32_FLAGS_NOT_EXPANDABLE)));
    
    /* Found a free element ? */
    if (bFound)
    {
      /* Get the pointer on the cell according to index value and cells size */
      pCell = (orxVOID *)(((orxU8 *)pstCurrentSegment->pSegmentDatas) + (_pstBank->u32ElemSize * u32FieldResultIndex) + (_pstBank->u32ElemSize * u32BitResultIndex));
    
      /* Decrease the number of free elements */
      pstCurrentSegment->u32NbFree--;
    
      /* Set the bit as used */
      ((orxU32)pstCurrentSegment->pu32FreeElemBits[u32FieldResultIndex]) |= 1 << u32BitResultIndex;
    }
  }

  return pCell;
}

/** Free an allocated cell
 * @param[in] _pstBank    Bank of memory from where _pCell has been allocated
 * @param[in] _pCell      Pointer on the cell to free
 * @return a new cell of memory (orxNULL if no allocation possible)
 */
orxVOID orxBank_Free(orxBANK *_pstBank, orxVOID *_pCell)
{
  orxBANK_SEGMENT *pstSegment;  /* Segment associated to the cell */
  orxU32 u32Index32Bits;        /* Index of 32 the bits data */
  orxU32 u32IndexBit;           /* Index of the bit in u32Index32Bits */
  orxU32 u32CellIndex;          /* Difference in pointers adress */
  
  /* Module initialized ? */
  orxASSERT((sstBank.u32Flags & orxBANK_KU32_FLAG_READY) == orxBANK_KU32_FLAG_READY);

  /* Correct parameters ? */
  orxASSERT(_pstBank != orxNULL);
  orxASSERT(_pCell != orxNULL);
  
  /* Look for the segment associated to this cell */
  pstSegment = orxBank_GetSegment(_pstBank, _pCell);
  orxASSERT(pstSegment != orxNULL);
  
  /* (The adress of _pCell can not be smaller than the adress of pstSegment->pSegmentDatas */
  orxASSERT(_pCell >= pstSegment->pSegmentDatas);
  
  /* Retrieve the cell index in the bitfield computing position with cell adress */
  u32CellIndex    = (orxU32)_pCell - (orxU32)pstSegment->pSegmentDatas;
  u32Index32Bits  = u32CellIndex / (32 * _pstBank->u32ElemSize);
  u32IndexBit     = (u32CellIndex % (32 * _pstBank->u32ElemSize) / _pstBank->u32ElemSize);
  
  /* Set cell as Free */
  pstSegment->pu32FreeElemBits[u32Index32Bits] &= ~(1 << u32IndexBit);
  
  /* Increase the number of free elements */
  pstSegment->u32NbFree++;
}

/** Free all allocated cell from a bank
 * @param[in] _pstBank    Bank of memory to clear
 */
orxVOID orxBank_Clear(orxBANK *_pstBank)
{
  orxVOID *pstCell = orxNULL;
  
  /* Module initialized ? */
  orxASSERT((sstBank.u32Flags & orxBANK_KU32_FLAG_READY) == orxBANK_KU32_FLAG_READY);

  /* Correct parameters ? */
  orxASSERT(_pstBank != orxNULL);
  
  /* Free all elements */
  while ((pstCell = orxBank_GetNext(_pstBank, NULL)))
  {
    orxBank_Free(_pstBank, pstCell);
  }
}

/** Get the next cell
 * @param[in] _pstBank    Bank of memory from where _pCell has been allocated
 * @param[in] _pCell      Pointer on the current cell of memory
 * @return The next cell. If _pCell is orxNULL, the first cell will be returned. Returns orxNULL when no more cell can be returned.
 */
orxVOID *orxBank_GetNext(orxBANK *_pstBank, orxVOID *_pCell)
{
  orxBANK_SEGMENT *pstSegment;  /* Segment associated to the cell */
  orxU32 u32Index32Bits;        /* Index of 32 the bits data */
  orxS32 s32IndexBit;           /* Index of the bit in u32Index32Bits */
  orxU32 u32CellIndex;          /* Difference in pointers adress */
  orxVOID *pCell = orxNULL;     /* Returned cell */
  orxBOOL bFound = orxFALSE;    /* orxTRUE when the cell will be found */

  /* Module initialized ? */
  orxASSERT((sstBank.u32Flags & orxBANK_KU32_FLAG_READY) == orxBANK_KU32_FLAG_READY);

  /* Correct parameters ? */
  orxASSERT(_pstBank != orxNULL);

  /* Look for the segment associated to this cell */
  if (_pCell == orxNULL)
  {
    pstSegment = _pstBank->pstFirstSegment;
    
    /* Get the first bit index */
    u32Index32Bits  = 0;
    s32IndexBit     = -1;
  }
  else
  {
    pstSegment = orxBank_GetSegment(_pstBank, _pCell);
    /* Get a valid segment ? */
    orxASSERT(pstSegment != orxNULL);

    /* Compute the cell bit index */
    u32CellIndex    = (orxU32)_pCell - (orxU32)pstSegment->pSegmentDatas;
    u32Index32Bits  = u32CellIndex / (32 * _pstBank->u32ElemSize);
    s32IndexBit     = (u32CellIndex % (32 * _pstBank->u32ElemSize) / _pstBank->u32ElemSize);
  }

  /* Loop on bank segments while not found */
  while (!bFound && pstSegment != orxNULL)
  {
    /* Loop on segment bitfields while not found */
    while (!bFound && (u32Index32Bits < _pstBank->u32SizeSegmentBitField))
    {
        /* Loop on bits while not found */
        while (!bFound && (s32IndexBit < 31))
        {
          s32IndexBit++;
          if ((pstSegment->pu32FreeElemBits[u32Index32Bits] & (orxU32)(1 << s32IndexBit)) == (orxU32)(1 << s32IndexBit))
          {
            /* We found it ! */
            bFound = orxTRUE;
          }
        }
        
        /* Not found ? try the next bitfield */
        if (!bFound)
        {
          u32Index32Bits++;
          s32IndexBit = -1;
        }
    }
    
    /* Not found ? try the next segment */
    if (!bFound)
    {
      pstSegment = pstSegment->pstNext;
      u32Index32Bits = 0;
    }
  }
  
  /* Compute adress of the cell if found, and returns it */
  if (bFound)
  {
    /* The cell is on pSegment, on the bitfield n° u32Index32Bits and on the bit u32IndexBit */
    pCell = (orxVOID *)(((orxU8 *)pstSegment->pSegmentDatas) + (((32 * u32Index32Bits) + s32IndexBit) * _pstBank->u32ElemSize));
  }
  
  return pCell;
}

/*******************************************************************************
 * DEBUG FUNCTION
 ******************************************************************************/

/** Print the content of a chunk bank
 * @param[in] _pstBank    Bank's pointer
 */
orxVOID orxBank_DebugPrint(orxBANK *_pstBank)
{
  orxBANK_SEGMENT *pstSegment;  /* Pointer on the current segment */
  orxU32 u32Index1, u32Index2, u32Index3;
  
  /* Module initialized ? */
  orxASSERT((sstBank.u32Flags & orxBANK_KU32_FLAG_READY) == orxBANK_KU32_FLAG_READY);

  /* Correct parameters ? */
  orxASSERT(_pstBank != orxNULL);

  orxTextIO_PrintLn("\n\n\n********* Bank (%x) *********", _pstBank);
  orxTextIO_PrintLn("* u32NbCellPerSegments = %u", _pstBank->u32NbCellPerSegments);
  orxTextIO_PrintLn("* u32ElemSize = %u", _pstBank->u32ElemSize);
  orxTextIO_PrintLn("* u32Flags = %x", _pstBank->u32Flags);
  orxTextIO_PrintLn("* eMemType = %u", _pstBank->eMemType);
  orxTextIO_PrintLn("* u32SizeSegmentBitField = %u", _pstBank->u32SizeSegmentBitField);

  orxTextIO_PrintLn("\n* **** SEGMENTS ******");

  pstSegment = _pstBank->pstFirstSegment;

  while (pstSegment != orxNULL)
  {
    orxTextIO_PrintLn("\n* ** Segment (%x) ***", pstSegment);
    orxTextIO_PrintLn("* u32NbFree = %u", pstSegment->u32NbFree);
    orxTextIO_PrintLn("* pstNext = %x", pstSegment->pstNext);
    orxTextIO_PrintLn("* pSegmentDatas = %x", pstSegment->pSegmentDatas);

    for (u32Index1 = 0; u32Index1 < _pstBank->u32SizeSegmentBitField; u32Index1++)
    {
      orxTextIO_Print("Bits : %02d (%x) = ", u32Index1, pstSegment->pu32FreeElemBits[u32Index1]);
      
      for (u32Index2 = 0; u32Index2 < 4; u32Index2++)
      {
        orxTextIO_Print("[");
        for (u32Index3 = 0; u32Index3 < 8; u32Index3++)
        {
          if ((pstSegment->pu32FreeElemBits[u32Index1] & (orxU32)(1 << ((8 * u32Index2) + u32Index3)) ) == (orxU32)(1 << ((8 * u32Index2) + u32Index3)) )
          {
            orxTextIO_Print("1");
          }
          else
          {
            orxTextIO_Print("0");
          }

          if (u32Index3 == 3)
          {
            orxTextIO_Print(" ");
          }
        }
        orxTextIO_Print("]");
      }
      
      orxTextIO_PrintLn("");
    }
    
    /* Go to next segment */
    pstSegment = pstSegment->pstNext;
  }
}


