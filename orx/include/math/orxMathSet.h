/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/**
 * @file math/orxSets.h
 * @date 15/06/2005
 * @author (C) Arcallians
 * @brief Mathematical set manipulation.
 * @warning In development, use it at your own risks.
 */

/**
 * @addtogroup MathematicalSets
 * 
 * @{
 */

#ifndef _orxSETS_H_
#define _orxSETS_H_

#include "orxInclude.h"
#include "utils/orxLinkList.h"
#include "memory/orxBank.h"

/**
 * @name Module state.
 * @{
 */

/** Inits the object system. */
extern orxSTATUS                      orxMathSet_Init();
/** Ends the object system. */
extern orxVOID                        orxMathSet_Exit();

/** @} */


/** @name Float interval.
 * @{ 
 */

/* Define flags */
#define orxINTERVALFLOAT_ALL_EXCLUDED           0x00000000  /**< Min and max are exluded. */       
#define orxINTERVALFLOAT_MIN_INCLUDED           0x00000001  /**< Min is included. */
#define orxINTERVALFLOAT_MAX_INCLUDED           0x00000010  /**< Max is included. */
#define orxINTERVALFLOAT_ALL_INCLUDED           0x00000011  /**< Min and max are included. */

/** Float-based interval structure. */
typedef struct __orxINTERVAL_FLOAT_t
{
    /** Coordinates : the min and the max. */
    orxFLOAT fMin, fMax;
    /** Flags to say if min and max are included or exluded.*/
    orxU32 u32Flags;
}
orxINTERVAL_FLOAT;


/** Set an interval.
 * @param _pstInterval Interval to set.
 * @param _fMin Minimum value to set.
 * @param _fMax Maximum value to set.
 * @param _bMinIncluded Flag indicating if minimum is included or excluded.
 * @param _bMaxIncluded Flag indicating if maximum is included or excluded.
 */
orxINLINE orxVOID orxIntervalFloat_Set(orxINTERVAL_FLOAT *_pstInterval, orxFLOAT _fMin, orxFLOAT _fMax, orxBOOL _bMinIncluded, orxBOOL _bMaxIncluded)
{
    _pstInterval->fMin = _fMin;
    _pstInterval->fMax = _fMax;
    _pstInterval->u32Flags = (_bMinIncluded&orxINTERVALFLOAT_MIN_INCLUDED)|(_bMaxIncluded&orxINTERVALFLOAT_MAX_INCLUDED);
}


/** Copy an interval to an other.
 * @param _pstInterTgt Interval to set.
 * @param _pstInterSrc Source insteval.
 */
orxINLINE orxVOID orxIntervalFloat_Copy(orxINTERVAL_FLOAT *_pstInterSrc, orxINTERVAL_FLOAT *_pstInterTgt)
{
    _pstInterTgt->fMin = _pstInterSrc->fMin;
    _pstInterTgt->fMax = _pstInterSrc->fMax;
    _pstInterTgt->u32Flags = _pstInterSrc->u32Flags;
}

/** Validate an interval.
 * Verify if the Min value is lower than the max and swap them if needed.
 * @param _pstInterval Interval to verify.
 */
orxINLINE orxVOID orxIntervalFloat_Validate(orxINTERVAL_FLOAT *_pstInterval)
{
    if (_pstInterval->fMin >_pstInterval->fMax)
    {
        orxSWAP32(_pstInterval->fMin, _pstInterval->fMax);
        _pstInterval->u32Flags = (_pstInterval->u32Flags&orxINTERVALFLOAT_ALL_INCLUDED)|((_pstInterval->u32Flags&orxINTERVALFLOAT_MIN_INCLUDED)<<8)|((_pstInterval->u32Flags&orxINTERVALFLOAT_MAX_INCLUDED)>>8);
    }
}

/** Swap two interval content.
 * @param _pstInter1 First interval.
 * @param _pstInter2 Second interval.
 */
orxINLINE orxVOID orxIntervalFloat_Swap(orxINTERVAL_FLOAT *_pstInter1, orxINTERVAL_FLOAT *_pstInter2)
{
     orxSWAP32(_pstInter1->fMin, _pstInter2->fMin);
     orxSWAP32(_pstInter1->fMax, _pstInter2->fMax);
     orxSWAP32(_pstInter1->u32Flags, _pstInter2->u32Flags);
}

/** Test if two intervals are identicals.
 * @param _pstInter1 First interval.
 * @param _pstInter2 Second interval.
 * @return true if they are identical.
 */
 orxINLINE orxBOOL orxIntervalFloat_AreEgual(orxINTERVAL_FLOAT *_pstInter1, orxINTERVAL_FLOAT *_pstInter2)
 {
     return (_pstInter1->fMin==_pstInter2->fMin)&&(_pstInter1->fMax==_pstInter2->fMax)&&(_pstInter1->u32Flags==_pstInter2->u32Flags);
 }
 
/** Test if two intervals are different.
 * @param _pstInter1 First interval.
 * @param _pstInter2 Second interval.
 * @return true if they are different.
 */
 orxINLINE orxBOOL orxIntervalFloat_AreDifferent(orxINTERVAL_FLOAT *_pstInter1, orxINTERVAL_FLOAT *_pstInter2)
 {
     return (_pstInter1->fMin!=_pstInter2->fMin)||(_pstInter1->fMax!=_pstInter2->fMax)||(_pstInter1->u32Flags!=_pstInter2->u32Flags);
 }
 
 /** Test if an interval is less than another.
 * @param _pstInter1 First interval.
 * @param _pstInter2 Second interval.
 * @return true if the first is less than the second.
 */
 orxINLINE orxBOOL orxIntervalFloat_IsLess(orxINTERVAL_FLOAT *_pstInter1, orxINTERVAL_FLOAT *_pstInter2)
 {
     return (_pstInter1->fMax<_pstInter2->fMin)||((_pstInter1->fMax==_pstInter2->fMin)&&((_pstInter1->u32Flags&orxINTERVALFLOAT_MAX_INCLUDED)!=(_pstInter2->u32Flags&orxINTERVALFLOAT_MIN_INCLUDED)));
 }

 /** Test if an interval is less or bottom-throw than another.
 * @param _pstInter1 First interval.
 * @param _pstInter2 Second interval.
 * @return true if the first is less or bottom-throw than the second.
 */
 orxINLINE orxBOOL orxIntervalFloat_IsLessOrBottomThrow(orxINTERVAL_FLOAT *_pstInter1, orxINTERVAL_FLOAT *_pstInter2)
 {
     return (_pstInter1->fMin<_pstInter2->fMin)&&(_pstInter1->fMax<_pstInter2->fMax);
 }

 /** Test if an interval is greater tha another.
 * @param _pstInter1 First interval.
 * @param _pstInter2 Second interval.
 * @return true if the first is greater than the second.
 */
 orxINLINE orxBOOL orxIntervalFloat_IsGreater(orxINTERVAL_FLOAT *_pstInter1, orxINTERVAL_FLOAT *_pstInter2)
 {
     return (_pstInter1->fMin>_pstInter2->fMax)||((_pstInter1->fMin==_pstInter2->fMax)&&((_pstInter1->u32Flags&orxINTERVALFLOAT_MIN_INCLUDED)!=(_pstInter2->u32Flags&orxINTERVALFLOAT_MAX_INCLUDED)));
 }
 
 /** Test if an interval is greater or top-throw than another.
 * @param _pstInter1 First interval.
 * @param _pstInter2 Second interval.
 * @return true if the first is greater or top-throw than the second.
 */
 orxINLINE orxBOOL orxIntervalFloat_IsGreaterOrTopThrow(orxINTERVAL_FLOAT *_pstInter1, orxINTERVAL_FLOAT *_pstInter2)
 {
     return (_pstInter1->fMin>_pstInter2->fMin)&&(_pstInter1->fMax>_pstInter2->fMax);
 }
 
 /** Test if an interval is strictly in another.
 * @param _pstInter1 First interval.
 * @param _pstInter2 Second interval.
 * @return true if the first is in the second.
 */
 orxINLINE orxBOOL orxIntervalFloat_IsIn(orxINTERVAL_FLOAT *_pstInter1, orxINTERVAL_FLOAT *_pstInter2)
 {
     return ( (_pstInter1->fMin>_pstInter2->fMin) || ((_pstInter1->fMin==_pstInter2->fMin)&&((_pstInter1->u32Flags&orxINTERVALFLOAT_MIN_INCLUDED)==(_pstInter2->u32Flags&orxINTERVALFLOAT_MIN_INCLUDED)))) &&
             ((_pstInter1->fMax<_pstInter2->fMax) || ((_pstInter1->fMin==_pstInter2->fMin)&&((_pstInter1->u32Flags&orxINTERVALFLOAT_MIN_INCLUDED)==(_pstInter2->u32Flags&orxINTERVALFLOAT_MIN_INCLUDED)))) ;
 }

/** Test if a float value is in an interval.
 * @param _pstInter Interval to test.
 * @param _fValue Value to search.
 * @return true if the value is present in the interval.
 */
 orxINLINE orxBOOL orxIntervalFloat_HasValue(const orxINTERVAL_FLOAT *_pstInter, orxFLOAT _fValue)
 {
     return ( (_fValue>_pstInter->fMin) || (_fValue<_pstInter->fMax) ||
              ((_fValue==_pstInter->fMin) && (_pstInter->u32Flags&orxINTERVALFLOAT_MIN_INCLUDED)) ||
              ((_fValue==_pstInter->fMax) && (_pstInter->u32Flags&orxINTERVALFLOAT_MAX_INCLUDED)) );
              
 }

/** Extand an interval to a value.
 * @param _pstInterval Interval to extand.
 * @param _fValue Value to used to extand the interval.
 */
extern orxVOID orxFASTCALL orxIntervalFloat_Extand(orxINTERVAL_FLOAT *_pstInterval, orxFLOAT _fValue);
/** @} */




/** @name Integer interval.
 * @{ 
 */

/** Int32-based interval structure. */
typedef struct __orxINTERVAL_INT32_t
{
    /** Coordinates : the min and the max. */
    orxS32 s32Min, s32Max;

}
orxINTERVAL_INT32;


/** Set an interval.
 * @param _pstInterval Interval to set.
 * @param _s32Min Minimum value to set.
 * @param _s32Max Maximum value to set.
 */
orxINLINE orxVOID orxIntervalInt32_Set(orxINTERVAL_INT32 *_pstInterval, orxS32 _s32Min, orxS32 _s32Max)
{
    _pstInterval->s32Min = _s32Min;
    _pstInterval->s32Max = _s32Max;
}

/** Copy an interval to an other.
 * @param _pstInterTgt Interval to set.
 * @param _pstInterSrc Source insteval.
 */
orxINLINE orxVOID orxIntervalInt32_Copy(orxINTERVAL_INT32 *_pstInterSrc, orxINTERVAL_INT32 *_pstInterTgt)
{
    _pstInterTgt->s32Min = _pstInterSrc->s32Min;
    _pstInterTgt->s32Max = _pstInterSrc->s32Max;
}

/** Validate an interval.
 * Verify if the Min value is lower than the max and swap them if needed.
 * @param _pstInterval Interval to verify.
 */
orxINLINE orxVOID orxIntervalInt32_Validate(orxINTERVAL_INT32 *_pstInterval)
{
    if (_pstInterval->s32Min >_pstInterval->s32Max)
    {
        orxSWAP32(_pstInterval->s32Min, _pstInterval->s32Max);
    }
}

/** Swap two interval content.
 * @param _pstInter1 First interval.
 * @param _pstInter2 Second interval.
 */
orxINLINE orxVOID orxIntervalInt32_Swap(orxINTERVAL_INT32 *_pstInter1, orxINTERVAL_INT32 *_pstInter2)
{
    orxSWAP32(_pstInter2->s32Min, _pstInter1->s32Min);
    orxSWAP32(_pstInter2->s32Max, _pstInter1->s32Max);
}


/** Test if two intervals are identicals.
 * @param _pstInter1 First interval.
 * @param _pstInter2 Second interval.
 * @return true if they are identical.
 */
 orxINLINE orxBOOL orxIntervalInt32_AreEgual(orxINTERVAL_INT32 *_pstInter1, orxINTERVAL_INT32 *_pstInter2)
 {
     return (_pstInter1->s32Min==_pstInter2->s32Min)&&(_pstInter1->s32Max==_pstInter2->s32Max);
 }
 
/** Test if two intervals are different.
 * @param _pstInter1 First interval.
 * @param _pstInter2 Second interval.
 * @return true if they are different.
 */
 orxINLINE orxBOOL orxIntervalInt32_AreDifferent(orxINTERVAL_INT32 *_pstInter1, orxINTERVAL_INT32 *_pstInter2)
 {
     return (_pstInter1->s32Min!=_pstInter2->s32Min)||(_pstInter1->s32Min!=_pstInter2->s32Min);
 }
 
 /** Test if an interval is less than another.
 * @param _pstInter1 First interval.
 * @param _pstInter2 Second interval.
 * @return true if the first is less than the second.
 */
 orxINLINE orxBOOL orxIntervalInt32_IsLess(orxINTERVAL_INT32 *_pstInter1, orxINTERVAL_INT32 *_pstInter2)
 {
     return _pstInter1->s32Max<_pstInter2->s32Min;
 }

 /** Test if an interval is less or bottom-throw than another.
 * @param _pstInter1 First interval.
 * @param _pstInter2 Second interval.
 * @return true if the first is less or bottom-throw than the second.
 */
 orxINLINE orxBOOL orxIntervalInt32_IsLessOrBottomThrow(orxINTERVAL_INT32 *_pstInter1, orxINTERVAL_INT32 *_pstInter2)
 {
     return (_pstInter1->s32Min<_pstInter2->s32Min)&&(_pstInter1->s32Min<=_pstInter2->s32Max);
 }

 /** Test if an interval is greater than another.
 * @param _pstInter1 First interval.
 * @param _pstInter2 Second interval.
 * @return true if the first is greater than the second.
 */
 orxINLINE orxBOOL orxIntervalInt32_IsGreater(orxINTERVAL_INT32 *_pstInter1, orxINTERVAL_INT32 *_pstInter2)
 {
     return (_pstInter1->s32Min>_pstInter2->s32Max);
 }
 
 /** Test if an interval is greater or top-throw than another.
 * @param _pstInter1 First interval.
 * @param _pstInter2 Second interval.
 * @return true if the first is greater or top-throw than the second.
 */
 orxINLINE orxBOOL orxIntervalInt32_IsGreaterOrTopThrow(orxINTERVAL_INT32 *_pstInter1, orxINTERVAL_INT32 *_pstInter2)
 {
     return (_pstInter1->s32Max>_pstInter2->s32Max)&&(_pstInter1->s32Min>=_pstInter2->s32Min);
 }
 
 /** Test if an interval is strictly in another.
 * @param _pstInter1 First interval.
 * @param _pstInter2 Second interval.
 * @return true if the first is in the second.
 */
 orxINLINE orxBOOL orxIntervalInt32_IsIn(orxINTERVAL_INT32 *_pstInter1, orxINTERVAL_INT32 *_pstInter2)
 {
     return (_pstInter1->s32Min>=_pstInter2->s32Min) && (_pstInter1->s32Max<=_pstInter2->s32Max);
 }

/** Test if an integer value is in an interval.
 * @param _pstInter Interval to test.
 * @param _s32Value Value to search.
 * @return true if the value is present in the interval.
 */
 orxINLINE orxBOOL orxIntervalInt32_HasValue(const orxINTERVAL_INT32 *_pstInter, orxS32 _s32Value)
 {
     return (_s32Value>=_pstInter->s32Min) || (_s32Value<=_pstInter->s32Max);
 }

/** Extand an interval to a value.
 * @param _pstInterval Interval to extand.
 * @param _i32Value Value to used to extand the interval.
 */
extern orxVOID orxFASTCALL orxIntervalInt32_Extand(orxINTERVAL_INT32 *_pstInterval, orxS32 _i32Value);

/** @} */



/** @name Float Set.
 * Manipulation of float-element sets.
 * @{
 */

/** Float-based extended interval structure. */
typedef struct __orxINTERVAL_FLOAT_NODE_t
{
    /** Link list direct dependancy. */
    orxLINKLIST_NODE stNode;
    /** Float-based interval.*/
    orxINTERVAL_FLOAT stInterval;
    /** Extended data type. */
    orxU32 u32ExtDataType;
    /** Extended data address. */
    orxHANDLE hExtData;
}
orxINTERVAL_FLOAT_NODE;

/** Return the interval part of a set node.
 * @return Address of the interval.
 */
orxINLINE orxINTERVAL_FLOAT* orxSetNodeFloat_GetInterval(orxINTERVAL_FLOAT_NODE* _pstNode)
{
    return &(_pstNode->stInterval);
}

/** Allocate a new node based on an interval.
 * @note Extended data is linked and not copied.
 * @param _stInterval Interval
 * @param _u32ExtDataType Type of extended data.
 * @param _hExtData Address of extanded data.
 * @return address of the allocated node.
 */
extern orxINTERVAL_FLOAT_NODE* orxSetNodeFloat_AllocateNode(orxINTERVAL_FLOAT _stInterval, orxU32 _u32ExtDataType, orxHANDLE _hExtData);

/** Float-based set structure. */
typedef struct __orxSET_FLOAT_t
{
    /** Link list of intervals.*/
    orxLINKLIST sIntervalList;
}
orxSET_FLOAT;


/** Clear a float-based set.  
 * @param _pstSet Set to clear.
 */
extern orxVOID orxFASTCALL orxSetFloat_Clear(orxSET_FLOAT *_pstSet);

/** Add an interval to a float-based set.
 * @param _pstSet Target set.
 * @param _ptInterv Interval to add.
 */
extern orxVOID orxFASTCALL orxSetFloat_Add(orxSET_FLOAT *_pstSet, orxINTERVAL_FLOAT _stInterval);
 
/** Substract an interval from a float-based set.
 * @param _pstSet Target set.
 * @param _stInterv Interval to substract.
 */
extern orxVOID orxFASTCALL orxSetFloat_Sub(orxSET_FLOAT *_pstet, orxINTERVAL_FLOAT _stInterval);

/** Return the interval corresponding to a value if any.
 * @param _pstSet Set where to search.
 * @param _fValue Value to search.
 * @return Address of the interval corresponding to the value param or NULL if not found.
 */
extern orxINTERVAL_FLOAT *orxFASTCALL orxSetFloat_FindValueInterval(orxSET_FLOAT *_pstSet, orxFLOAT _fValue);

/** Return the address of the attached list of interval.
 * @param _pstSet Set from witch extract the list.
 * @return Address of the attached list.
 */
orxINLINE orxLINKLIST *orxFASTCALL orxSetFloat_GetIntervalList(orxSET_FLOAT *_pstSet)
{
    return &(_pstSet->sIntervalList);
}


/** @} */



#endif
/** @} */
