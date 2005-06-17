/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/**
 * @file
 * @date 15/06/2005
 * @author (C) Arcallians
 * 
 */

/**
 * @addtogroup Mathematical sets
 * 
 * @{
 */
 
#ifndef _orxSETS_H_
#define _orxSETS_H_
 
#include "orxInclude.h"


/** @name Float interval.
 * @{ 
 */


/** Public structure definition. */
typedef struct __orxINTERVAL_FLOAT_t
{
  /** Coordinates : the min and the max. */
  orxFLOAT fMin, fMax;

} orxINTERVAL_FLOAT;


/** Set an interval.
 * @param _psInterval Interval to set.
 * @param _fMin Minimum value to set.
 * @param _fMax Maximum value to set.
 */
orxINLINE orxVOID orxIntervalFloat_Set(orxINTERVAL_FLOAT *_psInterval, orxFLOAT _fMin, orxFLOAT _fMax)
{
    _psInterval->fMin = _fMin;
    _psInterval->fMax = _fMax;
}

/** Copy an interval to an other.
 * @param _psInterTgt Interval to set.
 * @param _psInterSrc Source insteval.
 */
orxINLINE orxVOID orxIntervalFloat_Copy(orxINTERVAL_FLOAT *_psInterSrc, orxINTERVAL_FLOAT *_psInterTgt)
{
    _psInterTgt->fMin = _psInterSrc->fMin;
    _psInterTgt->fMax = _psInterSrc->fMax;
}

/** Validate an interval.
 * Verify if the Min value is lower than the max and swap them if needed.
 * @param _psInterval Interval to verify.
 */
orxINLINE orxVOID orxIntervalFloat_Validate(orxINTERVAL_FLOAT *_psInterval)
{
    orxFLOAT fTemp;
    if (_psInterval->fMin >_psInterval->fMax)
    {
        fTemp = _psInterval->fMin;
        _psInterval->fMin = _psInterval->fMax;
        _psInterval->fMax = fTemp;
    }
}

/** Extand an interval to a value.
 * @param _psInterval Interval to extand.
 * @param _fValue Value to used to extand the interval.
 */
 orxVOID orxFASTCALL orxIntervalFloat_Extand(orxINTERVAL_FLOAT *_psInterval, orxFLOAT _fValue);

/** @} */ 




/** @name Integer interval.
 * @{ 
 */


/** Public structure definition. */
typedef struct __orxINTERVAL_INT32_t
{
  /** Coordinates : the min and the max. */
  orxS32 i32Min, i32Max;

} orxINTERVAL_INT32;


/** Set an interval.
 * @param _psInterval Interval to set.
 * @param _i32Min Minimum value to set.
 * @param _i32Max Maximum value to set.
 */
orxINLINE orxVOID orxIntervalInt32_Set(orxINTERVAL_INT32 *_psInterval, orxS32 _i32Min, orxS32 _i32Max)
{
    _psInterval->i32Min = _i32Min;
    _psInterval->i32Max = _i32Max;
}

/** Copy an interval to an other.
 * @param _psInterTgt Interval to set.
 * @param _psInterSrc Source insteval.
 */
orxINLINE orxVOID orxIntervalInt32_Copy(orxINTERVAL_INT32 *_psInterSrc, orxINTERVAL_INT32 *_psInterTgt)
{
    _psInterTgt->i32Min = _psInterSrc->i32Min;
    _psInterTgt->i32Max = _psInterSrc->i32Max;
}

/** Validate an interval.
 * Verify if the Min value is lower than the max and swap them if needed.
 * @param _psInterval Interval to verify.
 */
orxINLINE orxVOID orxIntervalInt32_Validate(orxINTERVAL_INT32 *_psInterval)
{
    orxS32 i32Temp;
    if (_psInterval->i32Min >_psInterval->i32Max)
    {
        i32Temp = _psInterval->i32Min;
        _psInterval->i32Min = _psInterval->i32Max;
        _psInterval->i32Max = i32Temp;
    }
}

/** Extand an interval to a value.
 * @param _psInterval Interval to extand.
 * @param _i32Value Value to used to extand the interval.
 */
 orxVOID orxFASTCALL orxIntervalInt32_Extand(orxINTERVAL_INT32 *_psInterval, orxS32 _i32Value);

/** @} */ 

#endif
/** @} */
