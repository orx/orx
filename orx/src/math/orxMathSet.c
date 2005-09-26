/**
 * @file math/orxMathSet.c
 * @brief Mathematical sets
 * @warning In development, use it at your own risks.
 */

/***************************************************************************
orxSets.h
Mathematical sets

begin                : 15/06/2005
author               : (C) Arcallians
email                : cursor@arcallians.org
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "math/orxMathSet.h"
#include "memory/orxBank.h"

#define orxMATHSET_KU32_FLAG_NONE  0x00000000  /**< No flags have been set */
#define orxMATHSET_KU32_FLAG_READY 0x00000001  /**< The module has been initialized */

#define orxMATHSET_KU32_BANK_SIZE 10           /**< Banks' size */

/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/
typedef struct __orxMATHSET_STATIC_t
{
  orxU32 u32Flags;               /**< Module state flag.*/
  orxBANK *pstFloatIntervalBank; /**< Module bank for float-based intervals.*/
  orxBANK *pstInt32IntervalBank; /**< Module bank for s32-based intervals.*/
} orxMATHSET_STATIC;

/***************************************************************************
 * Module global variable                                                  *
 ***************************************************************************/
orxSTATIC orxMATHSET_STATIC sstMathSet;

/***************************************************************************
 orxMathSet_Setup
 MathSet module setup.

 returns: nothing
 ***************************************************************************/
orxVOID orxMathSet_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_MATHSET, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_MATHSET, orxMODULE_ID_BANK);
  orxModule_AddDependency(orxMODULE_ID_MATHSET, orxMODULE_ID_LINKLIST);

  return;
}

/***************************************************************************
 orxMathSets_Init
 Inits the mathematical set system.
 
 returns: orxSTATUS_SUCCESS/orxSTATUS_FAILED
          (Always success car can not failed.)
 ***************************************************************************/
orxSTATUS orxMathSet_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILED;

  /* Not already Initialized? */
  if(!(sstMathSet.u32Flags & orxMATHSET_KU32_FLAG_READY))
  {
    /* Cleans static variable */
    orxMemory_Set(&sstMathSet, 0, sizeof(orxMATHSET_STATIC));
    
    sstMathSet.pstFloatIntervalBank = orxBank_Create(orxMATHSET_KU32_BANK_SIZE, sizeof(orxINTERVAL_FLOAT_NODE), 0, orxMEMORY_TYPE_MAIN);
    
    if (sstMathSet.pstFloatIntervalBank != orxNULL)
    {
      sstMathSet.pstInt32IntervalBank = orxBank_Create(orxMATHSET_KU32_BANK_SIZE, sizeof(orxINTERVAL_INT32_NODE), 0, orxMEMORY_TYPE_MAIN);
      
      /* Success ? */
      if (sstMathSet.pstInt32IntervalBank != orxNULL)
      {
        eResult = orxSTATUS_SUCCESS;
      }
      else
      {
        orxBank_Delete(sstMathSet.pstFloatIntervalBank);
      }
    }
  }
  else
  {
    /* !!! MSG !!! */

    /* Already initialized */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Done! */
  return eResult;
}

/***************************************************************************
 orxLinkList_Exit
 Exits from the link list system.
 
 returns: orxVOID
 ***************************************************************************/
orxVOID orxMathSet_Exit()
{
  /* Initialized? */
  if(sstMathSet.u32Flags & orxMATHSET_KU32_FLAG_READY)
  {
    orxBank_Delete(sstMathSet.pstFloatIntervalBank);
    orxBank_Delete(sstMathSet.pstInt32IntervalBank);
    
    /* Not ready now */
    sstMathSet.u32Flags = orxMATHSET_KU32_FLAG_NONE;
  }
  else
  {
    /* !!! MSG !!! */
  }
  
  /* Done */
  return;
}





/* Validate an interval. */
orxVOID orxIntervalFloat_Validate(orxINTERVAL_FLOAT *_pstInterval)
{
  /* Module intialized ? */
  orxASSERT(sstMathSet.u32Flags & orxMATHSET_KU32_FLAG_READY);
  
  /* Correct parameters ? */
  orxASSERT(_pstInterval != orxNULL);

  if (_pstInterval->fMin >_pstInterval->fMax)
  {
    orxSWAP32(_pstInterval->fMin, _pstInterval->fMax);
    
    if (_pstInterval->u32Flags==orxINTERVALFLOAT_MIN_INCLUDED)
    {
      _pstInterval->u32Flags = orxINTERVALFLOAT_MAX_INCLUDED;
    }
    else if (_pstInterval->u32Flags==orxINTERVALFLOAT_MAX_INCLUDED)
    {
      _pstInterval->u32Flags = orxINTERVALFLOAT_MIN_INCLUDED;
    }
  }
}

/* Swap two interval content. */
orxVOID orxIntervalFloat_Swap(orxINTERVAL_FLOAT *_pstInter1, orxINTERVAL_FLOAT *_pstInter2)
{
  /* Module intialized ? */
  orxASSERT(sstMathSet.u32Flags & orxMATHSET_KU32_FLAG_READY);
  
  /* Correct parameters ? */
  orxASSERT(_pstInter1 != orxNULL);
  orxASSERT(_pstInter2 != orxNULL);

  orxSWAP32(_pstInter1->fMin, _pstInter2->fMin);
  orxSWAP32(_pstInter1->fMax, _pstInter2->fMax);
  orxSWAP32(_pstInter1->u32Flags, _pstInter2->u32Flags);
}

/* Extand an interval to a value.*/
orxVOID orxFASTCALL orxIntervalFloat_Extand(orxINTERVAL_FLOAT *_pstInterval, orxFLOAT _fValue, orxBOOL _bIncluded)
{
  /* Module intialized ? */
  orxASSERT(sstMathSet.u32Flags & orxMATHSET_KU32_FLAG_READY);
  
  /* Correct parameters ? */
  orxASSERT(_pstInterval != orxNULL);

  if (_pstInterval->fMin > _fValue)
  {
    _pstInterval->fMin = _fValue;
    if (_bIncluded)
    {
      orxFLAG32_SET(_pstInterval->u32Flags, orxINTERVALFLOAT_MIN_INCLUDED, orxINTERVALFLOAT_MIN_INCLUDED);
    }
  }
  else if (_pstInterval->fMax < _fValue)
  {
    _pstInterval->fMax = _fValue;
    if (_bIncluded)
    {
      orxFLAG32_SET(_pstInterval->u32Flags, orxINTERVALFLOAT_MAX_INCLUDED, orxINTERVALFLOAT_MAX_INCLUDED);
    }
  }
  else if ((_pstInterval->fMin==_fValue)&&_bIncluded&&!orxFLAG32_TEST(_pstInterval->u32Flags,orxINTERVALFLOAT_MIN_INCLUDED))
  {
    orxFLAG32_SET(_pstInterval->u32Flags, orxINTERVALFLOAT_MIN_INCLUDED, orxINTERVALFLOAT_MIN_INCLUDED);
  }
  else if ((_pstInterval->fMax==_fValue)&&_bIncluded&&!orxFLAG32_TEST(_pstInterval->u32Flags,orxINTERVALFLOAT_MAX_INCLUDED))
  {
    orxFLAG32_SET(_pstInterval->u32Flags, orxINTERVALFLOAT_MAX_INCLUDED, orxINTERVALFLOAT_MAX_INCLUDED);
  }    
}


/***************************************************************************
 orxIntervalInt32_Extand
 Extand an interval to a value.
 
 returns: orxVOID
 ***************************************************************************/
orxVOID orxFASTCALL orxIntervalInt32_Extand(orxINTERVAL_INT32 *_pstInterval, orxS32 _s32Value)
{
  /* Module intialized ? */
  orxASSERT(sstMathSet.u32Flags & orxMATHSET_KU32_FLAG_READY);
  
  /* Correct parameters ? */
  orxASSERT(_pstInterval != orxNULL);

  if (_pstInterval->s32Min > _s32Value)
  {
    _pstInterval->s32Min = _s32Value;
  }
  else if (_pstInterval->s32Max < _s32Value)
  {
    _pstInterval->s32Max = _s32Value;
  }
}


/**
 * Float-based mathematical set.
 */

/** Allocate a new node based on an interval. */
orxINTERVAL_FLOAT_NODE* orxSetNodeFloat_AllocateNode(orxINTERVAL_FLOAT _stInterval, orxU32 _u32ExtDataType, orxHANDLE _hExtData)
{
  orxINTERVAL_FLOAT_NODE *pstNode;
  
  /* Module intialized ? */
  orxASSERT(sstMathSet.u32Flags & orxMATHSET_KU32_FLAG_READY);
  
  pstNode = (orxINTERVAL_FLOAT_NODE*)orxBank_Allocate(sstMathSet.pstFloatIntervalBank);
  
  if (pstNode != orxNULL)
  {  
    pstNode->stInterval.fMin = _stInterval.fMin;
    pstNode->stInterval.fMax = _stInterval.fMax;
    pstNode->stInterval.u32Flags = _stInterval.u32Flags;
    pstNode->u32ExtDataType = _u32ExtDataType;
    pstNode->hExtData = _hExtData;
  }
  else
  {
    /* MSG !!! */
  }
  
  return pstNode;
}



/** Clear a float-based set. */
orxVOID orxFASTCALL orxSetFloat_Clear(orxSET_FLOAT *_pstSet)
{
    /** Free all allocated node.*/
    orxINTERVAL_FLOAT_NODE *pstNode = orxNULL;
    orxLINKLIST *pstList = orxSetFloat_GetIntervalList(_pstSet);
    pstNode = (orxINTERVAL_FLOAT_NODE*) orxLinkList_GetFirst((orxLINKLIST*) pstList);
    while (pstNode!=NULL)
    {
      orxBank_Free(sstMathSet.pstFloatIntervalBank, pstNode);
    }

    /** Clear the internal list. */
    orxLinkList_Clean(&(_pstSet->sIntervalList));
}

/** Add an interval to a float-based set.
 * @param _pstSet Target set.
 * @param _pstInterv Interval to add.
 */
orxVOID orxFASTCALL orxSetFloat_Add(orxSET_FLOAT *_pstSet, orxINTERVAL_FLOAT _stInterval)
{
  orxLINKLIST *pstList;

  orxINTERVAL_FLOAT_NODE *pstNodeFirst = orxNULL,
                         *pstNodeLast  = orxNULL,
                         *pstNodeTemp  = orxNULL;

  /* Module intialized ? */
  orxASSERT(sstMathSet.u32Flags & orxMATHSET_KU32_FLAG_READY);
  
  /* Correct parameters ? */
  orxASSERT(_pstSet != orxNULL);

  pstList = orxSetFloat_GetIntervalList(_pstSet);
  pstNodeLast  = (orxINTERVAL_FLOAT_NODE*) orxLinkList_GetLast((orxLINKLIST*) pstList);
  pstNodeFirst = (orxINTERVAL_FLOAT_NODE*) orxLinkList_GetFirst((orxLINKLIST*) pstList);

  /** Search greatest interval before _strInterval.*/
  while ( (pstNodeTemp!=NULL) && orxIntervalFloat_IsLess(*orxSetNodeFloat_GetInterval((orxINTERVAL_FLOAT_NODE*)pstNodeTemp), _stInterval))
  {
    pstNodeFirst = pstNodeTemp;
    pstNodeTemp = (orxINTERVAL_FLOAT_NODE*) orxLinkList_GetNext((orxLINKLIST_NODE*) pstNodeTemp);
  }

  /** Search littlest interval after _strInterval.*/
  while ( (pstNodeTemp!=NULL) && orxIntervalFloat_IsGreater(*orxSetNodeFloat_GetInterval((orxINTERVAL_FLOAT_NODE*)pstNodeTemp), _stInterval))
  {
    pstNodeLast = pstNodeTemp;
    pstNodeTemp = (orxINTERVAL_FLOAT_NODE*) orxLinkList_GetPrevious((orxLINKLIST_NODE*) pstNodeTemp);
  }

  /** Grow the _stInterval if across other intervals. */
  pstNodeTemp = (orxINTERVAL_FLOAT_NODE*) orxLinkList_GetNext((orxLINKLIST_NODE*) pstNodeFirst);
  if (pstNodeTemp!=NULL && pstNodeTemp!=pstNodeLast)
  {
    orxIntervalFloat_Extand(&_stInterval, pstNodeTemp->stInterval.fMin, orxFLAG32_TEST(pstNodeTemp->stInterval.u32Flags, orxINTERVALFLOAT_MIN_INCLUDED));
  }

  pstNodeTemp = (orxINTERVAL_FLOAT_NODE*) orxLinkList_GetPrevious((orxLINKLIST_NODE*) pstNodeLast);
  if (pstNodeTemp!=NULL && pstNodeTemp!=pstNodeFirst)
  {
    orxIntervalFloat_Extand(&_stInterval, pstNodeTemp->stInterval.fMax, orxFLAG32_TEST(pstNodeTemp->stInterval.u32Flags, orxINTERVALFLOAT_MAX_INCLUDED));
  }

  /** Insert the current interval. */
  pstNodeTemp = orxSetNodeFloat_AllocateNode(_stInterval, 0, orxNULL);
  orxLinkList_AddAfter((orxLINKLIST_NODE*) pstNodeFirst, (orxLINKLIST_NODE*) pstNodeTemp);

  /** Remove all undeeded nodes. */
  pstNodeFirst = pstNodeTemp;
  pstNodeTemp = (orxINTERVAL_FLOAT_NODE*) orxLinkList_GetNext((orxLINKLIST_NODE*) pstNodeFirst);
  while (pstNodeTemp!=orxNULL && pstNodeTemp!=pstNodeLast)
  {
    orxLinkList_Remove((orxLINKLIST_NODE*) pstNodeTemp);
    orxBank_Free(sstMathSet.pstFloatIntervalBank, pstNodeTemp);
    pstNodeTemp = (orxINTERVAL_FLOAT_NODE*)  orxLinkList_GetNext((orxLINKLIST_NODE*) pstNodeFirst);
  }
}



/** Substract an interval from a float-based set.
 * @param _pstSet Target set.
 * @param _stInterv Interval to substract.
 */
orxVOID orxFASTCALL orxSetFloat_Sub(orxSET_FLOAT *_pstSet, orxINTERVAL_FLOAT _stInterval)
{
    orxINTERVAL_FLOAT_NODE *pstNodeFirst = orxNULL,
                           *pstNodeLast  = orxNULL,
                           *pstNodeTemp  = orxNULL;

    orxLINKLIST *pstList = orxSetFloat_GetIntervalList(_pstSet);
    pstNodeLast  = (orxINTERVAL_FLOAT_NODE*) orxLinkList_GetLast((orxLINKLIST*) pstList);
    pstNodeFirst = (orxINTERVAL_FLOAT_NODE*) orxLinkList_GetFirst((orxLINKLIST*) pstList);
    
    /** Search greatest interval before or bottom-throw _strInterval.*/
    while ( (pstNodeFirst!=NULL) && orxIntervalFloat_IsLessOrBottomThrow(*orxSetNodeFloat_GetInterval((orxINTERVAL_FLOAT_NODE*)pstNodeFirst), _stInterval))
        pstNodeFirst = (orxINTERVAL_FLOAT_NODE*) orxLinkList_GetNext((orxLINKLIST_NODE*) pstNodeFirst);

    /** Search littlest interval after or top-throw _strInterval.*/
    while ( (pstNodeLast!=NULL) && orxIntervalFloat_IsGreaterOrTopThrow(*orxSetNodeFloat_GetInterval((orxINTERVAL_FLOAT_NODE*)pstNodeLast), _stInterval))
        pstNodeLast = (orxINTERVAL_FLOAT_NODE*) orxLinkList_GetPrevious((orxLINKLIST_NODE*) pstNodeLast);

    /** Shrink intervals witch are across the _stInterval. */
    if (pstNodeFirst!=NULL && pstNodeLast!=NULL && pstNodeFirst==pstNodeLast)
    {
        pstNodeTemp = orxSetNodeFloat_AllocateNode(pstNodeFirst->stInterval, 0, orxNULL);
        orxIntervalFloat_Set(&pstNodeFirst->stInterval, pstNodeFirst->stInterval.fMin, _stInterval.fMin, pstNodeFirst->stInterval.u32Flags, !_stInterval.u32Flags);
        orxIntervalFloat_Set(&pstNodeTemp->stInterval, _stInterval.fMax, pstNodeFirst->stInterval.fMax, !_stInterval.u32Flags, pstNodeFirst->stInterval.u32Flags);
        pstNodeLast = pstNodeTemp;
        orxLinkList_AddAfter((orxLINKLIST_NODE*) pstNodeFirst, (orxLINKLIST_NODE*) pstNodeTemp);
    }
    else
    {
        /** If the interval before is across _stInterval, shrink it.*/
        if (pstNodeFirst!=NULL && !orxIntervalFloat_IsLess(*orxSetNodeFloat_GetInterval((orxINTERVAL_FLOAT_NODE*)pstNodeFirst), _stInterval))
            orxIntervalFloat_Set(&pstNodeFirst->stInterval, pstNodeFirst->stInterval.fMin, _stInterval.fMin, pstNodeFirst->stInterval.u32Flags, !_stInterval.u32Flags);
        /** If the interval before is across _stInterval, shrink it.*/    
        if (pstNodeLast!=NULL && !orxIntervalFloat_IsGreaterOrTopThrow(*orxSetNodeFloat_GetInterval((orxINTERVAL_FLOAT_NODE*)pstNodeLast), _stInterval))
            orxIntervalFloat_Set(&pstNodeLast->stInterval, _stInterval.fMax, pstNodeLast->stInterval.fMax, !_stInterval.u32Flags, pstNodeLast->stInterval.u32Flags);
    }
    
    /** Remove all undeeded nodes. */
    pstNodeTemp = (orxINTERVAL_FLOAT_NODE*) orxLinkList_GetNext((orxLINKLIST_NODE*) pstNodeFirst);
    while (pstNodeTemp!=orxNULL && pstNodeTemp!=pstNodeLast)
    {
        orxLinkList_Remove((orxLINKLIST_NODE*) pstNodeTemp);
        orxBank_Free(sstMathSet.pstFloatIntervalBank, pstNodeTemp);
        pstNodeTemp = (orxINTERVAL_FLOAT_NODE*)  orxLinkList_GetNext((orxLINKLIST_NODE*) pstNodeFirst);
    }
}



/** Return the interval corresponding to a value if any.
 * @param _pstSet Set where to search.
 * @param _fValue Value to search.
 * @return Address of the interval corresponding to the value param or NULL if not found.
 */
orxINTERVAL_FLOAT_NODE *orxFASTCALL orxSetFloat_FindValueIntervalNode(orxSET_FLOAT *_pstSet, orxFLOAT _fValue)
{
    orxINTERVAL_FLOAT_NODE *pstNode = orxNULL;
    orxLINKLIST *pstList = orxSetFloat_GetIntervalList(_pstSet);
    pstNode = (orxINTERVAL_FLOAT_NODE*) orxLinkList_GetFirst((orxLINKLIST*) pstList);

    while (pstNode!=orxNULL && !orxIntervalFloat_HasValue(pstNode->stInterval, _fValue))
        pstNode = (orxINTERVAL_FLOAT_NODE*) orxLinkList_GetNext((orxLINKLIST_NODE*) pstNode);
    return pstNode;
}



/**
 * Int32-based mathematical set.
 */


/** Allocate a new node based on an interval. */
orxINTERVAL_INT32_NODE* orxSetNodeInt32_AllocateNode(orxINTERVAL_INT32 _stInterval, orxU32 _u32ExtDataType, orxHANDLE _hExtData)
{
    orxINTERVAL_INT32_NODE *pstNode = (orxINTERVAL_INT32_NODE*)orxBank_Allocate(sstMathSet.pstInt32IntervalBank);
    pstNode->stInterval.s32Min = _stInterval.s32Min;
    pstNode->stInterval.s32Max = _stInterval.s32Max;
    pstNode->u32ExtDataType = _u32ExtDataType;
    pstNode->hExtData = _hExtData;
    return pstNode;
}



/** Clear a integer-based set. */
orxVOID orxFASTCALL orxSetInt32_Clear(orxSET_INT32 *_pstSet)
{
    /** Free all allocated node.*/
    orxINTERVAL_INT32_NODE *pstNode = orxNULL;
    orxLINKLIST *pstList = orxSetInt32_GetIntervalList(_pstSet);
    pstNode = (orxINTERVAL_INT32_NODE*) orxLinkList_GetFirst((orxLINKLIST*) pstList);
    while (pstNode!=orxNULL)
    {
        orxBank_Free(sstMathSet.pstInt32IntervalBank, pstNode);
    }

    /** Clear the internal list. */
    orxLinkList_Clean(&(_pstSet->sIntervalList));
}

/** Add an interval to a integer-based set.
 * @param _pstSet Target set.
 * @param _pstInterv Interval to add.
 */
orxVOID orxFASTCALL orxSetInt32_Add(orxSET_INT32 *_pstSet, orxINTERVAL_INT32 _stInterval)
{
    orxINTERVAL_INT32_NODE *pstNodeFirst = orxNULL,
                           *pstNodeLast  = orxNULL,
                           *pstNodeTemp  = orxNULL;

    orxLINKLIST *pstList = orxSetInt32_GetIntervalList(_pstSet);
    pstNodeLast  = (orxINTERVAL_INT32_NODE*) orxLinkList_GetLast((orxLINKLIST*) pstList);
    pstNodeFirst = (orxINTERVAL_INT32_NODE*) orxLinkList_GetFirst((orxLINKLIST*) pstList);

    /** Search greatest interval before _strInterval.*/
    while ( (pstNodeTemp!=NULL) && orxIntervalInt32_IsLess(*orxSetNodeInt32_GetInterval((orxINTERVAL_INT32_NODE*)pstNodeTemp), _stInterval))
    {
        pstNodeFirst = pstNodeTemp;
        pstNodeTemp = (orxINTERVAL_INT32_NODE*) orxLinkList_GetNext((orxLINKLIST_NODE*) pstNodeTemp);
    }

    /** Search littlest interval after _strInterval.*/
    while ( (pstNodeTemp!=NULL) && orxIntervalInt32_IsGreater(*orxSetNodeInt32_GetInterval((orxINTERVAL_INT32_NODE*)pstNodeTemp), _stInterval))
    {
        pstNodeLast = pstNodeTemp;
        pstNodeTemp = (orxINTERVAL_INT32_NODE*) orxLinkList_GetPrevious((orxLINKLIST_NODE*) pstNodeTemp);
    }

    /** Grow the _stInterval if across other intervals. */
    pstNodeTemp = (orxINTERVAL_INT32_NODE*) orxLinkList_GetNext((orxLINKLIST_NODE*) pstNodeFirst);
    if (pstNodeTemp!=NULL && pstNodeTemp!=pstNodeLast)
        orxIntervalInt32_Extand(&_stInterval, pstNodeTemp->stInterval.s32Min);

    pstNodeTemp = (orxINTERVAL_INT32_NODE*) orxLinkList_GetPrevious((orxLINKLIST_NODE*) pstNodeLast);
    if (pstNodeTemp!=NULL && pstNodeTemp!=pstNodeFirst)
        orxIntervalInt32_Extand(&_stInterval, pstNodeTemp->stInterval.s32Max);

    /** Insert the current interval. */
    pstNodeTemp = orxSetNodeInt32_AllocateNode(_stInterval, 0, orxNULL);
    orxLinkList_AddAfter((orxLINKLIST_NODE*) pstNodeFirst, (orxLINKLIST_NODE*) pstNodeTemp);

    /** Remove all undeeded nodes. */
    pstNodeFirst = pstNodeTemp;
    pstNodeTemp = (orxINTERVAL_INT32_NODE*) orxLinkList_GetNext((orxLINKLIST_NODE*) pstNodeFirst);
    while (pstNodeTemp!=orxNULL && pstNodeTemp!=pstNodeLast)
    {
        orxLinkList_Remove((orxLINKLIST_NODE*) pstNodeTemp);
        orxBank_Free(sstMathSet.pstInt32IntervalBank, pstNodeTemp);
        pstNodeTemp = (orxINTERVAL_INT32_NODE*)  orxLinkList_GetNext((orxLINKLIST_NODE*) pstNodeFirst);
    }
}



/** Substract an interval from a float-based set.
 * @param _pstSet Target set.
 * @param _stInterv Interval to substract.
 */
orxVOID orxFASTCALL orxSetInt32_Sub(orxSET_INT32 *_pstSet, orxINTERVAL_INT32 _stInterval)
{
    orxINTERVAL_INT32_NODE *pstNodeFirst = orxNULL,
                           *pstNodeLast  = orxNULL,
                           *pstNodeTemp  = orxNULL;

    orxLINKLIST *pstList = orxSetInt32_GetIntervalList(_pstSet);
    pstNodeLast  = (orxINTERVAL_INT32_NODE*) orxLinkList_GetLast((orxLINKLIST*) pstList);
    pstNodeFirst = (orxINTERVAL_INT32_NODE*) orxLinkList_GetFirst((orxLINKLIST*) pstList);
    
    /** Search greatest interval before or bottom-throw _strInterval.*/
    while ( (pstNodeFirst!=NULL) && orxIntervalInt32_IsLessOrBottomThrow(*orxSetNodeInt32_GetInterval((orxINTERVAL_INT32_NODE*)pstNodeFirst), _stInterval))
        pstNodeFirst = (orxINTERVAL_INT32_NODE*) orxLinkList_GetNext((orxLINKLIST_NODE*) pstNodeFirst);

    /** Search littlest interval after or top-throw _strInterval.*/
    while ( (pstNodeLast!=NULL) && orxIntervalInt32_IsGreaterOrTopThrow(*orxSetNodeInt32_GetInterval((orxINTERVAL_INT32_NODE*)pstNodeLast), _stInterval))
        pstNodeLast = (orxINTERVAL_INT32_NODE*) orxLinkList_GetPrevious((orxLINKLIST_NODE*) pstNodeLast);

    /** Shrink intervals witch are across the _stInterval. */
    if (pstNodeFirst!=NULL && pstNodeLast!=NULL && pstNodeFirst==pstNodeLast)
    {
        pstNodeTemp = orxSetNodeInt32_AllocateNode(pstNodeFirst->stInterval, 0, orxNULL);
        orxIntervalInt32_Set(&pstNodeFirst->stInterval, pstNodeFirst->stInterval.s32Min, _stInterval.s32Min);
        orxIntervalInt32_Set(&pstNodeTemp->stInterval, _stInterval.s32Max, pstNodeFirst->stInterval.s32Max);
        pstNodeLast = pstNodeTemp;
        orxLinkList_AddAfter((orxLINKLIST_NODE*) pstNodeFirst, (orxLINKLIST_NODE*) pstNodeTemp);
    }
    else
    {
        /** If the interval before is across _stInterval, shrink it.*/
        if (pstNodeFirst!=NULL && !orxIntervalInt32_IsLess(*orxSetNodeInt32_GetInterval((orxINTERVAL_INT32_NODE*)pstNodeFirst), _stInterval))
            orxIntervalInt32_Set(&pstNodeFirst->stInterval, pstNodeFirst->stInterval.s32Min, _stInterval.s32Min);
        /** If the interval before is across _stInterval, shrink it.*/    
        if (pstNodeLast!=NULL && !orxIntervalInt32_IsGreaterOrTopThrow(*orxSetNodeInt32_GetInterval((orxINTERVAL_INT32_NODE*)pstNodeLast), _stInterval))
            orxIntervalInt32_Set(&pstNodeLast->stInterval, _stInterval.s32Max, pstNodeLast->stInterval.s32Max);
    }
    
    /** Remove all undeeded nodes. */
    pstNodeTemp = (orxINTERVAL_INT32_NODE*) orxLinkList_GetNext((orxLINKLIST_NODE*) pstNodeFirst);
    while (pstNodeTemp!=orxNULL && pstNodeTemp!=pstNodeLast)
    {
        orxLinkList_Remove((orxLINKLIST_NODE*) pstNodeTemp);
        orxBank_Free(sstMathSet.pstFloatIntervalBank, pstNodeTemp);
        pstNodeTemp = (orxINTERVAL_INT32_NODE*)  orxLinkList_GetNext((orxLINKLIST_NODE*) pstNodeFirst);
    }
}



/** Return the interval corresponding to a value if any.
 * @param _pstSet Set where to search.
 * @param _s32Value Value to search.
 * @return Address of the interval corresponding to the value param or NULL if not found.
 */
orxINTERVAL_INT32_NODE *orxFASTCALL orxSetInt32_FindValueIntervalNode(orxSET_INT32 *_pstSet, orxS32 _s32Value)
{
    orxINTERVAL_INT32_NODE *pstNode = orxNULL;
    orxLINKLIST *pstList = orxSetInt32_GetIntervalList(_pstSet);
    pstNode = (orxINTERVAL_INT32_NODE*) orxLinkList_GetFirst((orxLINKLIST*) pstList);

    while (pstNode!=orxNULL && !orxIntervalInt32_HasValue(pstNode->stInterval, _s32Value))
    {
        pstNode = (orxINTERVAL_INT32_NODE*) orxLinkList_GetNext((orxLINKLIST_NODE*) pstNode);
    }
    return pstNode;
}

