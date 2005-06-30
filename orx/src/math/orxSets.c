/**
 * @file math/orxSets.c
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

#include "math/orxSets.h"


/***************************************************************************
 * Module global variable                                                  *
 ***************************************************************************/
orxSTATIC orxBANK *spstFloatIntervalBank;
orxSTATIC orxBANK *spstInt32IntervalBank;


/***************************************************************************
 orxMathSets_Init
 Inits the mathematical set system.

 returns: orxSTATUS_SUCCESS/orxSTATUS_FAILED
          (Always success car can not failed.)
 ***************************************************************************/
orxSTATUS orxMathSets_Init()
{
    spstFloatIntervalBank = orxBank_Create(10, sizeof(orxINTERVAL_FLOAT_NODE), 0, orxMEMORY_TYPE_MAIN);
    /** @todo add the bank allocation for int32-based intervals.*/
    /* Done! */
    return orxSTATUS_SUCCESS;
}

/***************************************************************************
 orxMathSets_Exit
 Exits from the link list system.

 returns: orxVOID
 ***************************************************************************/
orxVOID orxMathSets_Exit()
{
    orxBank_Delete(spstFloatIntervalBank);
    /** @todo add the bank deletion for int32-based intervals.*/
    return;
}



/***************************************************************************
 orxIntervalFloat_Extand
 Extand an interval to a value.

 returns: orxVOID
 ***************************************************************************/
 orxVOID orxFASTCALL orxIntervalFloat_Extand(orxINTERVAL_FLOAT *_pstInterval, orxFLOAT _fValue)
 {
     if (_pstInterval->fMin > _fValue)
     {
         _pstInterval->fMin = _fValue;
     }
     else if (_pstInterval->fMax < _fValue)
     {
         _pstInterval->fMax = _fValue;
     }
 }


/***************************************************************************
 orxIntervalInt32_Extand
 Extand an interval to a value.

 returns: orxVOID
 ***************************************************************************/
 orxVOID orxFASTCALL orxIntervalInt32_Extand(orxINTERVAL_INT32 *_pstInterval, orxS32 _i32Value)
 {
     if (_pstInterval->i32Min > _i32Value)
     {
         _pstInterval->i32Min = _i32Value;
     }
     else if (_pstInterval->i32Max < _i32Value)
     {
         _pstInterval->i32Max = _i32Value;
     }
}



/** Allocate a new node based on an interval. */
orxINTERVAL_FLOAT_NODE* orxSetNodeFloat_AllocateNode(orxINTERVAL_FLOAT _stInterval, orxU32 _u32ExtDataType, orxHANDLE _hExtData)
{
    orxINTERVAL_FLOAT_NODE *pstNode = (orxINTERVAL_FLOAT_NODE*)orxBank_Allocate(spstFloatIntervalBank);
    pstNode->stInterval.fMin = _stInterval.fMin;
    pstNode->stInterval.fMax = _stInterval.fMax;
    pstNode->stInterval.u32Flags = _stInterval.u32Flags;
    pstNode->u32ExtDataType = _u32ExtDataType;
    pstNode->hExtData = _hExtData;
    return pstNode;
}



/** Clear a float-based set. */
orxVOID orxFASTCALL orxSetFloat_Clear(orxSET_FLOAT *_pstSet)
{
    

    /** Only clear the internal list. */
    orxLinkList_Clean(&(_pstSet->sIntervalList));
}

/** Add an interval to a float-based set.
 * @param _pstSet Target set.
 * @param _pstInterv Interval to add.
 */
orxVOID orxFASTCALL orxSetFloat_Add(orxSET_FLOAT *_pstSet, orxINTERVAL_FLOAT _stInterval)
{
    orxINTERVAL_FLOAT_NODE *pstNodeFirst = orxNULL,
                           *pstNodeLast  = orxNULL,
                           *pstNodeTemp  = orxNULL;

    orxLINKLIST *pstList = orxSetFloat_GetIntervalList(_pstSet);
    pstNodeLast  = (orxINTERVAL_FLOAT_NODE *)orxLinkList_GetLast(pstList);
    pstNodeFirst = (orxINTERVAL_FLOAT_NODE *)orxLinkList_GetFirst(pstList);
    
    /** Search greatest interval before _strInterval.*/
    while ( (pstNodeTemp!=NULL) && orxIntervalFloat_IsLess(orxSetNodeFloat_GetInterval((orxINTERVAL_FLOAT_NODE*)pstNodeTemp), &_stInterval))
    {
        pstNodeFirst = pstNodeTemp;
        pstNodeTemp = (orxINTERVAL_FLOAT_NODE *)orxLinkList_GetNext((orxLINKLIST_NODE *)pstNodeTemp);
    }
    
    /** Search littlest interval after _strInterval.*/
    while ( (pstNodeTemp!=NULL) && orxIntervalFloat_IsGreater(orxSetNodeFloat_GetInterval((orxINTERVAL_FLOAT_NODE*)pstNodeTemp), &_stInterval))
    {
        pstNodeLast = pstNodeTemp;
        pstNodeTemp = (orxINTERVAL_FLOAT_NODE *)orxLinkList_GetPrevious((orxLINKLIST_NODE *)pstNodeTemp);
    }
    
    /** Enlarge the _stInterval if across other intervals. */
    pstNodeTemp = (orxINTERVAL_FLOAT_NODE *)orxLinkList_GetNext((orxLINKLIST_NODE *)pstNodeFirst);
    if (pstNodeTemp!=NULL && pstNodeTemp!=pstNodeLast)
    {
        orxIntervalFloat_Extand(&_stInterval, pstNodeTemp->stInterval.fMin);
    }
        
    pstNodeTemp = (orxINTERVAL_FLOAT_NODE *)orxLinkList_GetPrevious((orxLINKLIST_NODE *)pstNodeLast);
    if (pstNodeTemp!=NULL && pstNodeTemp!=pstNodeFirst)
    {
        orxIntervalFloat_Extand(&_stInterval, pstNodeTemp->stInterval.fMax);
    }
        
    /** Insert the current interval. */
    pstNodeTemp = orxSetNodeFloat_AllocateNode(_stInterval, 0, orxNULL);
    orxLinkList_AddAfter((orxLINKLIST_NODE *)pstNodeFirst, (orxLINKLIST_NODE *)pstNodeTemp);
    
    /** Remove all undeeded nodes. */
    pstNodeFirst = pstNodeTemp;
    pstNodeTemp = (orxINTERVAL_FLOAT_NODE *)orxLinkList_GetNext((orxLINKLIST_NODE *)pstNodeFirst);
    while (pstNodeTemp!=orxNULL && pstNodeTemp!=pstNodeLast)
    {
        orxLinkList_Remove((orxLINKLIST_NODE *)pstNodeTemp);
        pstNodeTemp = (orxINTERVAL_FLOAT_NODE *)orxLinkList_GetNext((orxLINKLIST_NODE *)pstNodeFirst);
    }
}


 
/** Substract an interval from a float-based set.
 * @param _pstSet Target set.
 * @param _stInterv Interval to substract.
 */
orxVOID orxFASTCALL orxSetFloat_Sub(orxSET_FLOAT *_pstSet, orxINTERVAL_FLOAT _stInterval)
{
    /** @todo .*/
}

/** Test if a float value is in the set.
 * @param _pstSet Set to test.
 * @param _fValue Value to test.
 * @return True if _fValue is in _psSet.
 */
orxBOOL orxFASTCALL orxSetFloat_TestValue(orxSET_FLOAT *_pstSet, orxFLOAT _fValue)
{
    orxFLOAT fCookie;
    orxLINKLIST_NODE * 	pNode = orxLinkList_GetFirst(&(_pstSet->sIntervalList));

/*    while (pNode!=orxNULL)
    {
        if ( ((orxINTERVAL_FLOAT*)pNode)
    }*/
}


/** Return the interval corresponding to a value if any.
 * @param _pstSet Set where to search.
 * @param _fValue Value to search.
 * @return Address of the interval corresponding to the value param or NULL if not found.
 */
extern orxINTERVAL_FLOAT *orxFASTCALL orxSetFloat_FindValueInterval(orxSET_FLOAT *_pstSet, orxFLOAT _fValue);


