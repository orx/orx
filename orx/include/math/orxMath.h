/** 
 * \file orxMath.h
 * 
 * Math Module.
 * Gathers all math related includes.
 * Has base math functions.
 * 
 * \todo
 * Adds self base math functions instead of common ones.
 */


/***************************************************************************
 orxMath.h
 Math module
 
 begin                : 30/03/2005
 author               : (C) Arcallians
 email                : iarwain@arcallians.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _orxMATH_H_
#define _orxMATH_H_


#include "orxInclude.h"

/* *** Maths related includes *** */
#include <math.h>                   /* Common maths / should be removed later */

#include "math/orxVector.h"         /* Vectors */


/* *** Public macro definition. *** */

#define orxMIN(A, B)              (((A) > (B)) ? (B) : (A))
#define orxMAX(A, B)              (((A) < (B)) ? (B) : (A))

#define orxCLAMP(V, MIN, MAX)     orxMAX(orxMIN(V, MAX), MIN)

#define orxABS(V)                 (((V) < 0) ? -(V) : (V))
#define orxFABS(V)                (((V) < 0.0f) ? -(V) : (V))

#define orxSWAP32(A, B)                                       \
do                                                            \
{                                                             \
  *((orxU32 *)&(A)) = *((orxU32 *)&(A)) ^ *((orxU32 *)&(B));  \
  *((orxU32 *)&(B)) = *((orxU32 *)&(A)) ^ *((orxU32 *)&(B));  \
  *((orxU32 *)&(A)) = *((orxU32 *)&(A)) ^ *((orxU32 *)&(B));  \
} while(orxFALSE);

#define orxF2U(V)                 ((orxU32)  (V))
#define orxF2S(V)                 ((orxS32)  (V))
#define orxU2F(V)                 ((orxFLOAT)(V))
#define orxS2F(V)                 ((orxFLOAT)(V))


/*** Math Definitions ***/
#define KF_MATH_SQRT2             1.414213562f


#endif /* _orxMATH_H_ */
