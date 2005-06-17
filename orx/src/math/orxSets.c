/**
 * @file orxSets.c
 * 
 * Mathematical sets
 * 
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




/** Extand an interval to a value.*/
 orxVOID orxFASTCALL orxIntervalFloat_Extand(orxINTERVAL_FLOAT *_psInterval, orxFLOAT _fValue)
 {
     if (_psInterval->fMin > _fValue)
     {
         _psInterval->fMin = _fValue;
     }
     else if (_psInterval->fMax < _fValue)
     {
         _psInterval->fMax = _fValue;
     }
 }


/** Extand an interval to a value.*/
 orxVOID orxFASTCALL orxIntervalInt32_Extand(orxINTERVAL_INT32 *_psInterval, orxS32 _i32Value)
 {
     if (_psInterval->i32Min > _i32Value)
     {
         _psInterval->i32Min = _i32Value;
     }
     else if (_psInterval->i32Max < _i32Value)
     {
         _psInterval->i32Max = _i32Value;
     }
}

