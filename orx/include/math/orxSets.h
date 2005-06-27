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


/**
 * @name Module state.
 * @{
 */

/** Inits the object system. */
extern orxSTATUS                      orxMathSets_Init();
/** Ends the object system. */
extern orxVOID                        orxMathSets_Exit();

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
        _pstInterval->u32Flags = _pstInterval->u32Flags&orxINTERVALFLOAT_ALL_INCLUDED|((_pstInterval->u32Flags&&orxINTERVALFLOAT_MIN_INCLUDED)<<8)|((_pstInterval->u32Flags&&orxINTERVALFLOAT_MAX_INCLUDED)>>8);
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
    orxS32 i32Min, i32Max;

}
orxINTERVAL_INT32;


/** Set an interval.
 * @param _pstInterval Interval to set.
 * @param _i32Min Minimum value to set.
 * @param _i32Max Maximum value to set.
 */
orxINLINE orxVOID orxIntervalInt32_Set(orxINTERVAL_INT32 *_pstInterval, orxS32 _i32Min, orxS32 _i32Max)
{
    _pstInterval->i32Min = _i32Min;
    _pstInterval->i32Max = _i32Max;
}

/** Copy an interval to an other.
 * @param _pstInterTgt Interval to set.
 * @param _pstInterSrc Source insteval.
 */
orxINLINE orxVOID orxIntervalInt32_Copy(orxINTERVAL_INT32 *_pstInterSrc, orxINTERVAL_INT32 *_pstInterTgt)
{
    _pstInterTgt->i32Min = _pstInterSrc->i32Min;
    _pstInterTgt->i32Max = _pstInterSrc->i32Max;
}

/** Validate an interval.
 * Verify if the Min value is lower than the max and swap them if needed.
 * @param _pstInterval Interval to verify.
 */
orxINLINE orxVOID orxIntervalInt32_Validate(orxINTERVAL_INT32 *_pstInterval)
{
    if (_pstInterval->i32Min >_pstInterval->i32Max)
    {
        orxSWAP32(_pstInterval->i32Min, _pstInterval->i32Max);
    }
}

/** Swap two interval content.
 * @param _pstInter1 First interval.
 * @param _pstInter2 Second interval.
 */
orxINLINE orxVOID orxIntervalInt32_Swap(orxINTERVAL_INT32 *_pstInter1, orxINTERVAL_INT32 *_pstInter2)
{
    orxSWAP32(_pstInter2->i32Min, _pstInter1->i32Min);
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
typedef struct __orxINTERVAL_FLOAT_EXT_t
{
    /** Float-based interval.*/
    orxINTERVAL_FLOAT sInterval;
    /** Extended data type. */
    orxU32 u32ExtDataType;
    /** Extended data address. */
    orxHANDLE hExtData;
}
orxINTERVAL_FLOAT_EXT;

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
 * @param _pstInterv Interval to add.
 */
extern orxVOID orxFASTCALL orxSetFloat_Add(orxSET_FLOAT *_pstSet, orxINTERVAL_FLOAT *_pstInterval);
 
/** Substract an interval from a float-based set.
 * @param _pstSet Target set.
 * @param _pstInterv Interval to substract.
 */
extern orxVOID orxFASTCALL orxSetFloat_Sub(orxSET_FLOAT *_pstet, orxINTERVAL_FLOAT *_pstInterval);

/** Test if a float value is in the set.
 * @param _pstSet Set to test.
 * @param _fValue Value to test.
 * @return True if _fValue is in _psSet.
 */
extern orxBOOL orxFASTCALL orxSetFloat_TestValue(orxSET_FLOAT *_pstSet, orxFLOAT *_fValue);

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


/** @} */



#endif
/** @} */
