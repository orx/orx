/**
 * @file orxBank.c
 * 
 * Memory Bank allocation / Unallocation module
 * 
 * @todo
 * @todo create a private function for segment allocation / unallocation
 * Optimizations
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

#define orxBANK_KU32_FLAG_NONE  0x00000000  /**< No flags have been set */
#define orxBANK_KU32_FLAG_READY 0x00000001  /**< The module has been initialized */

#define orxBANK_KU32_UNALLOCATION_HYSTERESYS  5 /**< The next segment will be unallocated when the current */
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
  orxBANK_SEGMENT *pstCurrentSegment; /**< Current used segment for allocation */
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
 * @param _u32NbElem  (IN)  Number of elements per segments
 * @param _u32Size    (IN)  Size of an element
 * @param _eMemType   (IN)  Memory type where the datas will be allocated
 * @return  returns a pointer on the memory segment (orxNULL if an error occured)
 */
orxBANK_SEGMENT *orxBank_SegmentCreate(orxBANK *_pstBank)
{
  orxBANK_SEGMENT *pstSegment;  /* Pointer on the segment of memory */

  /* Module initialized ? */
  orxASSERT((sstBank.u32Flags & orxBANK_KU32_FLAG_READY) == orxBANK_KU32_FLAG_READY);

  /* Correct parameters ? */
  orxASSERT(_pstBank != orxNULL);
  
  /* Allocate a new segent of memory */
  pstSegment = orxMemory_Allocate(sizeof(orxBANK_SEGMENT), _pstBank->eMemType);
  if (pstSegment != orxNULL)
  {
    /* Set initial segment values */
    orxMemory_Set(pstSegment, 0, sizeof(orxBANK_SEGMENT));
    pstSegment->pstNext   = orxNULL;
    pstSegment->u32NbFree = _pstBank->u32NbCellPerSegments;
    
    /* Allocate memory for bitfields */
    pstSegment->pu32FreeElemBits = orxMemory_Allocate(_pstBank->u32SizeSegmentBitField * sizeof(orxU32), _pstBank->eMemType);
    if (pstSegment->pu32FreeElemBits != orxNULL)
    {
      /* Allocate memory for datas */
      pstSegment->pSegmentDatas = orxMemory_Allocate(_pstBank->u32SizeSegmentBitField * sizeof(_pstBank->u32ElemSize), _pstBank->eMemType);
      if (pstSegment->pSegmentDatas != orxNULL)
      {
        /* Initialize bitfields memory */
        orxMemory_Set(pstSegment->pu32FreeElemBits, 0, _pstBank->u32SizeSegmentBitField * sizeof(orxU32));

        /* Initialize datas memory */
        orxMemory_Set(pstSegment->pSegmentDatas, 0, _pstBank->u32SizeSegmentBitField * sizeof(_pstBank->u32ElemSize));
      }
      else
      {
        /* Not enough memory for datas, cancel segment allocation */
        /* Free bitfields */
        orxMemory_Free(pstSegment->pu32FreeElemBits);
        orxMemory_Free(pstSegment);
        pstSegment = orxNULL;
      }
    }
    else
    {
      /* Can't allocate enouch memory : Free all banks datas and set pBank as orxNULL) */
      orxMemory_Free(pstSegment);
      pstSegment = orxNULL;
    }
  }
  
  return pstSegment;
}

/** Free a segment of memory (and recursivly all next segments)
 * @param _pstSegment   (IN)  Allocated segment of memory
 */
orxVOID orxBank_SegmentDestroy(orxBANK_SEGMENT *_pstSegment)
{
  /* Module initialized ? */
  orxASSERT((sstBank.u32Flags & orxBANK_KU32_FLAG_READY) == orxBANK_KU32_FLAG_READY);

  /* Correct parameters ? */
  orxASSERT(_pstSegment != orxNULL);
  
  /* Is there another semgnets linked to this one ? */
  if (_pstSegment->pstNext != orxNULL)
  {
    /* Yes, free it */
    orxBank_SegmentDestroy(_pstSegment->pstNext);
  }

  /* Free the current segment */
  orxMemory_Free(_pstSegment->pSegmentDatas);
  orxMemory_Free(_pstSegment->pu32FreeElemBits);
  orxMemory_Free(_pstSegment);
}

/** Returns the segment where is stored _pCell
 * @param _pstBank    (IN)  Bank that stores segments
 * @param _pCell      (IN)  Cell stored by the segment to find
 * @return  The segment where is stored _pCell (orxNULL if not found)
 */
orxBANK_SEGMENT *orxBank_GetSegment(orxBANK *_pstBank, orxVOID *_pCell)
{
  orxBANK_SEGMENT *pstSegment;          /* Returned segment */
  orxVOID *pStartAddress;               /* Start address of the segment */
  orxVOID *pEndAddress;                 /* End address of the segment */
  orxBOOL bFound;                       /* orxTRUE when segment found */
  
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
    pEndAddress   = pStartAddress + (_pstBank->u32ElemSize * _pstBank->u32NbCellPerSegments);
    
    /* Is cell in segment data range ? */
    if ((_pCell >= pStartAddress) && (_pCell < pEndAddress))
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
  
  /* Module not already initialized ? */
  orxASSERT(!(sstBank.u32Flags & orxBANK_KU32_FLAG_READY));

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
  
  /* Module successfully initialized */
  return orxSTATUS_SUCCESS;
}

/** Exit bank module
 */
orxVOID orxBank_Exit()
{
  /* Module initialized ? */
  orxASSERT((sstBank.u32Flags & orxBANK_KU32_FLAG_READY) == orxBANK_KU32_FLAG_READY);
  
  /* Module not ready now */
  sstBank.u32Flags = orxBANK_KU32_FLAG_NONE;
}

/** Create a new bank in memory and returns a pointer on it
 * @param _u32NbElem  (IN)  Number of elements per segments
 * @param _u32Size    (IN)  Size of an element
 * @param _u32Flags   (IN)  Flags set for this bank
 * @param _eMemType   (IN)  Memory type where the datas will be allocated
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
    pstBank->u32SizeSegmentBitField   = (((((_u32NbElem / 32) * 32) + (_u32NbElem % 32)) / 32) * sizeof(orxU32));
//    pstBank->u32SizeSegmentBitField   = (((_u32NbElem + 31) &  31) / 32) * sizeof(orxU32);
    
    /* Allocate the first segment, and select it as current */
    pstBank->pstFirstSegment    = orxBank_SegmentCreate(pstBank);
    pstBank->pstCurrentSegment  = pstBank->pstFirstSegment;
    
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
 * @param _pstBank      (IN)  Pointer on the memory bank allocated by orx
 */
orxVOID orxBank_Destroy(orxBANK *_pstBank)
{
  /* Module initialized ? */
  orxASSERT((sstBank.u32Flags & orxBANK_KU32_FLAG_READY) == orxBANK_KU32_FLAG_READY);

  /* Correct parameters ? */
  orxASSERT(_pstBank != orxNULL);
  
  /* Destroy segment(s) (This segment is the first one, it can't be orxNULL) */
  orxBank_SegmentDestroy(_pstBank->pstFirstSegment);
  
  /* Completly Free Bank */
  orxMemory_Free(_pstBank);
  
}

/** Allocate a new cell from the bank
 * @param _pstBank     (IN) Pointer on the memory bank to use
 * @return a new cell of memory (orxNULL if no allocation possible)
 */
orxVOID *orxBank_Allocate(orxBANK *_pstBank)
{
  orxVOID *pCell = orxNULL;   /* Returned cell */
  
  /* Module initialized ? */
  orxASSERT((sstBank.u32Flags & orxBANK_KU32_FLAG_READY) == orxBANK_KU32_FLAG_READY);

  /* Correct parameters ? */
  orxASSERT(_pstBank != orxNULL);
  
  /* Is there a free space in the current segment ? */
  if (_pstBank->pstCurrentSegment->u32NbFree == 0)
  {
    /* Try to allocate a new segment */
    _pstBank->pstCurrentSegment->pstNext = orxBank_SegmentCreate(_pstBank);
    
    /* Correct segment allocation ? */
    if (_pstBank->pstCurrentSegment->pstNext != orxNULL)
    {
      /* Set the new allocated segment as the current selected one */
      _pstBank->pstCurrentSegment = _pstBank->pstCurrentSegment->pstNext;
    }
  }
  
  /* Is the current segment has free nodes ? (yes : the allocation was correct, else returns orxNULL) */
  if (_pstBank->pstCurrentSegment->u32NbFree > 0)
  {
    /* Look for the first free cell */
    /* TODO */
    
  }

  return pCell;
}

/** Free an allocated cell
 * @param _pstBank  (IN)  Bank of memory from where _pCell has been allocated
 * @param _pCell    (IN)  Pointer on the cell to free
 * @return a new cell of memory (orxNULL if no allocation possible)
 */
orxVOID orxBank_Free(orxBANK *_pstBank, orxVOID *_pCell)
{
  orxBANK_SEGMENT *pstSegment;  /* Segment associated to the cell */
  
  /* Module initialized ? */
  orxASSERT((sstBank.u32Flags & orxBANK_KU32_FLAG_READY) == orxBANK_KU32_FLAG_READY);

  /* Correct parameters ? */
  orxASSERT(_pCell != orxNULL);
  
  /* Look for the segment associated to this cell */
  pstSegment = orxBank_GetSegment(_pstBank, _pCell);
  orxASSERT(pstSegment != orxNULL);
  
  /* Retrieve the cell index in the bitfield computing position with cell adress */
  
  /* Set cell as Free */
}

/** Get the next cell
 * @param _pstBank  (IN)  Bank of memory from where _pCell has been allocated
 * @param _pCell    (IN)  Pointer on the current cell of memory
 * @return The next cell. If _pCell is orxNULL, the first cell will be returned.
 * @return Returns orxNULL when no more cell can be returned.
 */
orxVOID *orxBank_GetNext(orxBANK *_pstBank, orxVOID *_pCell)
{
  orxBANK_SEGMENT *pstSegment;  /* Segment associated to the cell */
  orxVOID *pCell = orxNULL;        /* Returned cell */

  /* Module initialized ? */
  orxASSERT((sstBank.u32Flags & orxBANK_KU32_FLAG_READY) == orxBANK_KU32_FLAG_READY);

  /* Look for the segment associated to this cell */
  pstSegment = orxBank_GetSegment(_pstBank, _pCell);
  orxASSERT(pstSegment != orxNULL);

  /* Loop on bit field to find the next used cell */
  /* TODO */  
  
  /* Compute adress of the cell and returns it */
  /* TODO */
  
  return pCell;
}

/** Get the previous cell
 * @param _pstBank  (IN)  Bank of memory from where _pCell has been allocated
 * @param _pCell    (IN)  Pointer on the current cell of memory
 * @return The previous cell. If _pCell is orxNULL, the last cell will be returned.
 * @return Returns orxNULL when no more cell can be returned.
 */
orxVOID *orxBank_GetPrevious(orxBANK *_pstBank, orxVOID *_pCell)
{
  orxBANK_SEGMENT *pstSegment;  /* Segment associated to the cell */
  orxVOID *pCell = orxNULL;        /* Returned cell */

  /* Module initialized ? */
  orxASSERT((sstBank.u32Flags & orxBANK_KU32_FLAG_READY) == orxBANK_KU32_FLAG_READY);

  /* Look for the segment associated to this cell */
  pstSegment = orxBank_GetSegment(_pstBank, _pCell);
  orxASSERT(pstSegment != orxNULL);
  
  /* Loop on bit field to find the previous used cell */
  /* TODO */  
  
  /* Compute adress of the cell and returns it */
  /* TODO */
  
  return pCell;
}


/*******************************************************************************
 * DEBUG FUNCTION
 ******************************************************************************/

/** Print the content of a chunk bank
 * @param _pstBank  (IN)  Bank's pointer
 */
orxVOID orxBank_DebugPrint(orxBANK *_pstBank)
{
}


