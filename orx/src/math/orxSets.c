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
 orxMathSets_Init
 Inits the mathematical set system.

 returns: orxSTATUS_SUCCESS/orxSTATUS_FAILED
          (Always success car can not failed.)
 ***************************************************************************/
orxSTATUS orxLinkList_Init()
{
  /* Done! */
  return orxSTATUS_SUCCESS;
}

/***************************************************************************
 orxLinkList_Exit
 Exits from the link list system.

 returns: orxVOID
 ***************************************************************************/
orxVOID orxLinkList_Exit()
{
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
orxVOID orxFASTCALL orxSetFloat_Add(orxSET_FLOAT *_pstSet, orxINTERVAL_FLOAT *_pstInterval)
{
    /** @todo .*/
}

 
/** Substract an interval from a float-based set.
 * @param _pstSet Target set.
 * @param _pstInterv Interval to substract.
 */
orxVOID orxFASTCALL orxSetFloat_Sub(orxSET_FLOAT *_pstSet, orxINTERVAL_FLOAT *_pstInterval)
{
    /** @todo .*/
}

/** Test if a float value is in the set.
 * @param _pstSet Set to test.
 * @param _fValue Value to test.
 * @return True if _fValue is in _psSet.
 */
orxBOOL orxFASTCALL orxSetFloat_TestValue(orxSET_FLOAT *_pstSet, orxFLOAT *_fValue)
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
extern orxINTERVAL_FLOAT *orxFASTCALL orxSetFloat_FindValueInterval(orxSET_FLOAT *_pstSet, orxFLOAT *_fValue);

/** Return the address of the attached list of interval.
 * @param _pstSet Set from witch extract the list.
 * @return Address of the attached list.
 */
extern orxLINKLIST *orxFASTCALL orxSetFloat_GetIntervalList(orxSET_FLOAT *_pstSet);


