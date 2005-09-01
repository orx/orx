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
 * @section mathset_intervals_intro Intro
 * orxINTERVAL_FLOAT and orxINTERVAL_INT32 are two structures to manupulate mathematical intervals for float and integer respectivly.
 * Intervals are parts of a set (R reals ou Z integers).
 * They are marked by a min and max coordinates.
 * As real set is continuous, you can mark if each coord is included or excluded for real intervals.
 *
 * @section mathset_intervals_use Using
 * @subsection mathset_intervals_use_construct Construction
 * You can construct intervals directly or via orxIntervalFloat_Set/orxIntervalInt32_Set or orxIntervalFloat_Copy/orxIntervalInt32_Copy.
 * @code
 * orxINTERVAL_FLOAT stFloatFoo = { 0.0, 1.0, orxINTERVALFLOAT_ALL_INCLUDED};
 * orxIntervalFloat_Set(&stFloatFoo, 0.0, 1.0, orxINTERVALFLOAT_ALL_INCLUDED};
 *
 * orxINTERVAL_INT32 stInt32Foo = { 0, 10};
 * orxIntervalInt32_Set(&stInt32Foo, 0, 10);
 * @endcode
 *
 *
 * @{
 */

#ifndef _orxSETS_H_
#define _orxSETS_H_

#include "orxInclude.h"
#include "utils/orxLinkList.h"


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
#define orxINTERVALFLOAT_MAX_INCLUDED           0x00000100  /**< Max is included. */
#define orxINTERVALFLOAT_ALL_INCLUDED           0x00000101  /**< Min and max are included. */

#define orxINTERVALFLOAT_MIN_SHIFT              0           /**< Shifting of minimum flags. */
#define orxINTERVALFLOAT_MAX_SHIFT              8           /**< Shifting of maximum flags. */

/** Float-based interval structure. */
typedef struct __orxINTERVAL_FLOAT_t
{
    /** Coordinates : the min and the max. */
    orxFLOAT fMin, fMax;
    /** Flags to say if min and max are included or exluded.*/
    orxU32 u32Flags;
}
orxINTERVAL_FLOAT;


/** Test if the min point of a float-based interval is less than another min point.
 * @param _stInterv1 First interval.
 * @param _stInterv2 Second interval.
 * @return True if the min point of the first float-based interval is less than the second min point.
 */
orxINLINE orxBOOL orxIntervalFloat_IsPointMinLessMin(orxINTERVAL_FLOAT _stInterv1, orxINTERVAL_FLOAT _stInterv2)
{
    return (_stInterv1.fMin<_stInterv2.fMin) ||
           ( (_stInterv1.fMin==_stInterv2.fMin) && 
             orxFLAG32_TEST(_stInterv1.u32Flags, orxINTERVALFLOAT_MIN_INCLUDED) &&
             !orxFLAG32_TEST(_stInterv2.u32Flags, orxINTERVALFLOAT_MIN_INCLUDED) );
}

/** Test if the min point of a float-based interval is less than another max point.
 * @param _stInterv1 First interval.
 * @param _stInterv2 Second interval.
 * @return True if the min point of the first float-based interval is less than the second max point.
 */
orxINLINE orxBOOL orxIntervalFloat_IsPointMinLessMax(orxINTERVAL_FLOAT _stInterv1, orxINTERVAL_FLOAT _stInterv2)
{
    return _stInterv1.fMin<_stInterv2.fMax;
}

/** Test if the max point of a float-based interval is less than another min point.
 * @param _stInterv1 First interval.
 * @param _stInterv2 Second interval.
 * @return True if the max point of the first float-based interval is less than the second min point.
 */
orxINLINE orxBOOL orxIntervalFloat_IsPointMaxLessMin(orxINTERVAL_FLOAT _stInterv1, orxINTERVAL_FLOAT _stInterv2)
{
    return (_stInterv1.fMax<_stInterv2.fMin) ||
           ( (_stInterv1.fMax==_stInterv2.fMin) && !(orxFLAG32_TEST(_stInterv1.u32Flags, orxINTERVALFLOAT_MAX_INCLUDED)&&orxFLAG32_TEST(_stInterv2.u32Flags, orxINTERVALFLOAT_MIN_INCLUDED)) );
}

/** Test if the max point of a float-based interval is less than another max point.
 * @param _stInterv1 First interval.
 * @param _stInterv2 Second interval.
 * @return True if the max point of the first float-based interval is less than the second max point.
 */
orxINLINE orxBOOL orxIntervalFloat_IsPointMaxLessMax(orxINTERVAL_FLOAT _stInterv1, orxINTERVAL_FLOAT _stInterv2)
{
    return (_stInterv1.fMax<_stInterv2.fMax) ||
           ( (_stInterv1.fMax==_stInterv2.fMax) && 
             !orxFLAG32_TEST(_stInterv1.u32Flags, orxINTERVALFLOAT_MAX_INCLUDED) &&
             orxFLAG32_TEST(_stInterv2.u32Flags, orxINTERVALFLOAT_MAX_INCLUDED) );
}


/** Test if the min point of a float-based interval is greater than another min point.
 * @param _stInterv1 First interval.
 * @param _stInterv2 Second interval.
 * @return True if the min point of the first float-based interval is greater than the second min point.
 */
orxINLINE orxBOOL orxIntervalFloat_IsPointMinGreaterMin(orxINTERVAL_FLOAT _stInterv1, orxINTERVAL_FLOAT _stInterv2)
{
    return (_stInterv1.fMin>_stInterv2.fMin) ||
           ( (_stInterv1.fMin==_stInterv2.fMin) && 
             !orxFLAG32_TEST(_stInterv1.u32Flags, orxINTERVALFLOAT_MIN_INCLUDED) &&
             orxFLAG32_TEST(_stInterv2.u32Flags, orxINTERVALFLOAT_MIN_INCLUDED));
}

/** Test if the min point of a float-based interval is greater than another max point.
 * @param _stInterv1 First interval.
 * @param _stInterv2 Second interval.
 * @return True if the min point of the first float-based interval is greater than the second max point.
 */
orxINLINE orxBOOL orxIntervalFloat_IsPointMinGreaterMax(orxINTERVAL_FLOAT _stInterv1, orxINTERVAL_FLOAT _stInterv2)
{
    return (_stInterv1.fMin>_stInterv2.fMax) ||
           ( (_stInterv1.fMin==_stInterv2.fMax) && 
             !(orxFLAG32_TEST(_stInterv1.u32Flags, orxINTERVALFLOAT_MIN_INCLUDED) && orxFLAG32_TEST(_stInterv2.u32Flags, orxINTERVALFLOAT_MAX_INCLUDED)));
}

/** Test if the max point of a float-based interval is greater than another min point.
 * @param _stInterv1 First interval.
 * @param _stInterv2 Second interval.
 * @return True if the max point of the first float-based interval is greater than the second min point.
 */
orxINLINE orxBOOL orxIntervalFloat_IsPointMaxGreaterMin(orxINTERVAL_FLOAT _stInterv1, orxINTERVAL_FLOAT _stInterv2)
{
    return (_stInterv1.fMax>_stInterv2.fMin);
}

/** Test if the max point of a float-based interval is greater than another max point.
 * @param _stInterv1 First interval.
 * @param _stInterv2 Second interval.
 * @return True if the max point of the first float-based interval is greater than the second max point.
 */
orxINLINE orxBOOL orxIntervalFloat_IsPointMaxGreaterMax(orxINTERVAL_FLOAT _stInterv1, orxINTERVAL_FLOAT _stInterv2)
{
    return (_stInterv1.fMax>_stInterv2.fMax) ||
           ( (_stInterv1.fMax==_stInterv2.fMax) && 
             orxFLAG32_TEST(_stInterv1.u32Flags, orxINTERVALFLOAT_MAX_INCLUDED) && 
             !orxFLAG32_TEST(_stInterv2.u32Flags, orxINTERVALFLOAT_MAX_INCLUDED));
}
 

/** Direct object creation.
 * @param _fMin Minimum value to set.
 * @param _fMax Maximum value to set.
 * @param _bMinIncluded Flag indicating if minimum is included or excluded.
 * @param _bMaxIncluded Flag indicating if maximum is included or excluded.
 * @return a copy of the interval.
 */
orxINLINE orxINTERVAL_FLOAT orxIntervalFloat(orxFLOAT _fMin, orxFLOAT _fMax, orxBOOL _bMinIncluded, orxBOOL _bMaxIncluded)
{
    orxINTERVAL_FLOAT stInterval;
    stInterval.fMin = _fMin;
    stInterval.fMax = _fMax;
    stInterval.u32Flags = (_bMinIncluded?orxINTERVALFLOAT_MIN_INCLUDED:orxINTERVALFLOAT_ALL_EXCLUDED)|
                          (_bMaxIncluded?orxINTERVALFLOAT_MAX_INCLUDED:orxINTERVALFLOAT_ALL_EXCLUDED);
    return stInterval;
}

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
    _pstInterval->u32Flags = (_bMinIncluded?orxINTERVALFLOAT_MIN_INCLUDED:orxINTERVALFLOAT_ALL_EXCLUDED)|
                             (_bMaxIncluded?orxINTERVALFLOAT_MAX_INCLUDED:orxINTERVALFLOAT_ALL_EXCLUDED);
}


/** Copy an interval to an other.
 * @param _stInterTgt Interval to set.
 * @param _pstInterSrc Source insteval.
 */
orxINLINE orxVOID orxIntervalFloat_Copy(orxINTERVAL_FLOAT _stInterSrc, orxINTERVAL_FLOAT *_pstInterTgt)
{
    _pstInterTgt->fMin = _stInterSrc.fMin;
    _pstInterTgt->fMax = _stInterSrc.fMax;
    _pstInterTgt->u32Flags = _stInterSrc.u32Flags;
}

/** Validate an interval.
 * Verify if the Min value is lower than the max and swap them if needed.
 * @param _pstInterval Interval to verify.
 */
extern orxVOID orxIntervalFloat_Validate(orxINTERVAL_FLOAT *_pstInterval);

/** Swap two interval content.
 * @param _pstInter1 First interval.
 * @param _pstInter2 Second interval.
 */
extern orxVOID orxIntervalFloat_Swap(orxINTERVAL_FLOAT *_pstInter1, orxINTERVAL_FLOAT *_pstInter2);

/** Test if two intervals are identicals.
 * @param _stInter1 First interval.
 * @param _stInter2 Second interval.
 * @return true if they are identical.
 */
orxINLINE orxBOOL orxIntervalFloat_AreEgual(orxINTERVAL_FLOAT _stInter1, orxINTERVAL_FLOAT _stInter2)
{
    return (_stInter1.fMin==_stInter2.fMin)&&(_stInter1.fMax==_stInter2.fMax)&&(_stInter1.u32Flags==_stInter2.u32Flags);
}
 
/** Test if two intervals are different.
 * @param _stInter1 First interval.
 * @param _stInter2 Second interval.
 * @return true if they are different.
 */
orxINLINE orxBOOL orxIntervalFloat_AreDifferent(orxINTERVAL_FLOAT _stInter1, orxINTERVAL_FLOAT _stInter2)
{
    return (_stInter1.fMin!=_stInter2.fMin)||(_stInter1.fMax!=_stInter2.fMax)||(_stInter1.u32Flags!=_stInter2.u32Flags);
}
 
/** Test if an interval is less than another.
 * @param _stInter1 First interval.
 * @param _stInter2 Second interval.
 * @return true if the first is less than the second.
 */
orxINLINE orxBOOL orxIntervalFloat_IsLess(orxINTERVAL_FLOAT _stInter1, orxINTERVAL_FLOAT _stInter2)
{
    return orxIntervalFloat_IsPointMaxLessMin(_stInter1, _stInter2);
}

/** Test if an interval is less or bottom-throw than another.
 * @param _stInter1 First interval.
 * @param _stInter2 Second interval.
 * @return true if the first is less or bottom-throw than the second.
 */
orxBOOL orxIntervalFloat_IsLessOrBottomThrow(orxINTERVAL_FLOAT _stInter1, orxINTERVAL_FLOAT _stInter2)
{
    return orxIntervalFloat_IsPointMinLessMin(_stInter1, _stInter2) &&
            orxIntervalFloat_IsPointMaxLessMax(_stInter1, _stInter2);
}

/** Test if an interval is greater tha another.
 * @param _stInter1 First interval.
 * @param _stInter2 Second interval.
 * @return true if the first is greater than the second.
 */
orxINLINE orxBOOL orxIntervalFloat_IsGreater(orxINTERVAL_FLOAT _stInter1, orxINTERVAL_FLOAT _stInter2)
{
    return orxIntervalFloat_IsPointMinGreaterMax(_stInter1, _stInter2);
}
 
 /** Test if an interval is greater or top-throw than another.
 * @param _stInter1 First interval.
 * @param _stInter2 Second interval.
 * @return true if the first is greater or top-throw than the second.
 */
orxINLINE orxBOOL orxIntervalFloat_IsGreaterOrTopThrow(orxINTERVAL_FLOAT _stInter1, orxINTERVAL_FLOAT _stInter2)
{
    return orxIntervalFloat_IsPointMinGreaterMin(_stInter1, _stInter2) &&
           orxIntervalFloat_IsPointMaxGreaterMax(_stInter1, _stInter2);
}

 /** Test if an interval is strictly in another.
 * @param _stInter1 First interval.
 * @param _stInter2 Second interval.
 * @return true if the first is in the second.
 */
orxINLINE orxBOOL orxIntervalFloat_IsIn(orxINTERVAL_FLOAT _stInter1, orxINTERVAL_FLOAT _stInter2)
{
    return !orxIntervalFloat_IsPointMinLessMin(_stInter1, _stInter2) &&
           !orxIntervalFloat_IsPointMaxGreaterMax(_stInter1, _stInter2);
}

/** Test if a float value is in an interval.
 * @param _stInter Interval to test.
 * @param _fValue Value to search.
 * @return true if the value is present in the interval.
 */
 orxINLINE orxBOOL orxIntervalFloat_HasValue(const orxINTERVAL_FLOAT _stInter, orxFLOAT _fValue)
 {
     return ((_fValue>_stInter.fMin) && (_fValue<_stInter.fMax)) ||
              ((_fValue==_stInter.fMin) && (_stInter.u32Flags&orxINTERVALFLOAT_MIN_INCLUDED)) ||
              ((_fValue==_stInter.fMax) && (_stInter.u32Flags&orxINTERVALFLOAT_MAX_INCLUDED)) ;
              
 }

/** Extand an interval to a value.
 * @param _pstInterval Interval to extand.
 * @param _fValue Value to used to extand the interval.
 * @param _bIncluded Set if value is included or not.
 */
extern orxVOID orxFASTCALL orxIntervalFloat_Extand(orxINTERVAL_FLOAT *_pstInterval, orxFLOAT _fValue, orxBOOL _bIncluded);
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

/** Direct object creation
 * @param _s32Min Minimum value to set.
 * @param _s32Max Maximum value to set.
 * @return Direct copy of the interval
 */
orxINLINE orxINTERVAL_INT32 orxIntervalInt32(orxS32 _s32Min, orxS32 _s32Max)
{
    orxINTERVAL_INT32 stInterval;
    stInterval.s32Min = _s32Min;
    stInterval.s32Max = _s32Max;
    return stInterval;
}


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
 * @param _stInterSrc Source insteval.
 */
orxINLINE orxVOID orxIntervalInt32_Copy(orxINTERVAL_INT32 _stInterSrc, orxINTERVAL_INT32 *_pstInterTgt)
{
    _pstInterTgt->s32Min = _stInterSrc.s32Min;
    _pstInterTgt->s32Max = _stInterSrc.s32Max;
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
 * @param _stInter1 First interval.
 * @param _stInter2 Second interval.
 * @return true if they are identical.
 */
orxINLINE orxBOOL orxIntervalInt32_AreEgual(orxINTERVAL_INT32 _stInter1, orxINTERVAL_INT32 _stInter2)
{
     return (_stInter1.s32Min==_stInter2.s32Min)&&(_stInter1.s32Max==_stInter2.s32Max);
}
 
/** Test if two intervals are different.
 * @param _stInter1 First interval.
 * @param _stInter2 Second interval.
 * @return true if they are different.
 */
 orxINLINE orxBOOL orxIntervalInt32_AreDifferent(orxINTERVAL_INT32 _stInter1, orxINTERVAL_INT32 _stInter2)
 {
     return (_stInter1.s32Min!=_stInter2.s32Min)||(_stInter1.s32Min!=_stInter2.s32Min);
 }
 
 /** Test if an interval is less than another.
 * @param _stInter1 First interval.
 * @param _stInter2 Second interval.
 * @return true if the first is less than the second.
 */
 orxINLINE orxBOOL orxIntervalInt32_IsLess(orxINTERVAL_INT32 _stInter1, orxINTERVAL_INT32 _stInter2)
 {
     return _stInter1.s32Max<_stInter2.s32Min;
 }

 /** Test if an interval is less or bottom-throw than another.
 * @param _stInter1 First interval.
 * @param _stInter2 Second interval.
 * @return true if the first is less or bottom-throw than the second.
 */
 orxINLINE orxBOOL orxIntervalInt32_IsLessOrBottomThrow(orxINTERVAL_INT32 _stInter1, orxINTERVAL_INT32 _stInter2)
 {
     return (_stInter1.s32Min<_stInter2.s32Min)&&(_stInter1.s32Max<=_stInter2.s32Max);
 }

 /** Test if an interval is greater than another.
 * @param _stInter1 First interval.
 * @param _stInter2 Second interval.
 * @return true if the first is greater than the second.
 */
 orxINLINE orxBOOL orxIntervalInt32_IsGreater(orxINTERVAL_INT32 _stInter1, orxINTERVAL_INT32 _stInter2)
 {
     return (_stInter1.s32Min>_stInter2.s32Max);
 }
 
 /** Test if an interval is greater or top-throw than another.
 * @param _stInter1 First interval.
 * @param _stInter2 Second interval.
 * @return true if the first is greater or top-throw than the second.
 */
 orxINLINE orxBOOL orxIntervalInt32_IsGreaterOrTopThrow(orxINTERVAL_INT32 _stInter1, orxINTERVAL_INT32 _stInter2)
 {
     return (_stInter1.s32Max>_stInter2.s32Max)&&(_stInter1.s32Min>=_stInter2.s32Min);
 }
 
 /** Test if an interval is strictly in another.
 * @param _stInter1 First interval.
 * @param _stInter2 Second interval.
 * @return true if the first is in the second.
 */
 orxINLINE orxBOOL orxIntervalInt32_IsIn(orxINTERVAL_INT32 _stInter1, orxINTERVAL_INT32 _stInter2)
 {
     return (_stInter1.s32Min>=_stInter2.s32Min) && (_stInter1.s32Max<=_stInter2.s32Max);
 }

/** Test if an integer value is in an interval.
 * @param _stInter Interval to test.
 * @param _s32Value Value to search.
 * @return true if the value is present in the interval.
 */
 orxINLINE orxBOOL orxIntervalInt32_HasValue(const orxINTERVAL_INT32 _stInter, orxS32 _s32Value)
 {
     return (_s32Value>=_stInter.s32Min) && (_s32Value<=_stInter.s32Max);
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

/** Return the interval node corresponding to a value if any.
 * @param _pstSet Set where to search.
 * @param _fValue Value to search.
 * @return Address of the interval node corresponding to the value param or NULL if not found.
 */
extern orxINTERVAL_FLOAT_NODE *orxFASTCALL orxSetFloat_FindValueIntervalNode(orxSET_FLOAT *_pstSet, orxFLOAT _fValue);

/** Return the address of the attached list of interval.
 * @param _pstSet Set from witch extract the list.
 * @return Address of the attached list.
 */
orxINLINE orxLINKLIST *orxFASTCALL orxSetFloat_GetIntervalList(orxSET_FLOAT *_pstSet)
{
    return &(_pstSet->sIntervalList);
}


/** @} */




/** @name Integer Set.
 * Manipulation of integer-element sets.
 * @{
 */

/** Integer-based extended interval structure. */
typedef struct __orxINTERVAL_INT32_NODE_t
{
    /** Link list direct dependancy. */
    orxLINKLIST_NODE stNode;
    /** Float-based interval.*/
    orxINTERVAL_INT32 stInterval;
    /** Extended data type. */
    orxU32 u32ExtDataType;
    /** Extended data address. */
    orxHANDLE hExtData;
}
orxINTERVAL_INT32_NODE;

/** Return the interval part of a set node.
 * @return Address of the interval.
 */
orxINLINE orxINTERVAL_INT32* orxSetNodeInt32_GetInterval(orxINTERVAL_INT32_NODE* _pstNode)
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
extern orxINTERVAL_INT32_NODE* orxSetNodeInt32_AllocateNode(orxINTERVAL_INT32 _stInterval, orxU32 _u32ExtDataType, orxHANDLE _hExtData);

/** Float-based set structure. */
typedef struct __orxSET_INT32_t
{
    /** Link list of intervals.*/
    orxLINKLIST sIntervalList;
}
orxSET_INT32;

/** Clear an integer-based set.  
 * @param _pstSet Set to clear.
 */
extern orxVOID orxFASTCALL orxSetInt32_Clear(orxSET_INT32 *_pstSet);

/** Add an interval to an integer-based set.
 * @param _pstSet Target set.
 * @param _ptInterv Interval to add.
 */
extern orxVOID orxFASTCALL orxSetInt32_Add(orxSET_INT32 *_pstSet, orxINTERVAL_INT32 _stInterval);
 
/** Substract an interval from an integer-based set.
 * @param _pstSet Target set.
 * @param _stInterv Interval to substract.
 */
extern orxVOID orxFASTCALL orxSetInt32_Sub(orxSET_INT32 *_pstet, orxINTERVAL_INT32 _stInterval);

/** Return the interval corresponding to a value if any.
 * @param _pstSet Set where to search.
 * @param _s32Value Value to search.
 * @return Address of the interval node corresponding to the value param or NULL if not found.
 */
extern orxINTERVAL_INT32_NODE *orxFASTCALL orxSetInt32_FindValueIntervalNode(orxSET_INT32 *_pstSet, orxS32 _s32Value);

/** Return the address of the attached list of interval.
 * @param _pstSet Set from witch extract the list.
 * @return Address of the attached list.
 */
orxINLINE orxLINKLIST *orxFASTCALL orxSetInt32_GetIntervalList(orxSET_INT32 *_pstSet)
{
    return &(_pstSet->sIntervalList);
}

/** @} */


#endif
/** @} */
