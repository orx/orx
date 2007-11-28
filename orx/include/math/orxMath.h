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


// !!! TEMP : Till own functions are defined !!!
#ifdef __orxMSVC__
  #define sinf(X)                 (orxFLOAT)sin((orxFLOAT)(X))
  #define cosf(X)                 (orxFLOAT)cos((orxFLOAT)(X))
  #define floorf(X)               (orxFLOAT)floor((orxFLOAT)(X))
#endif /* __orxMSVC__ */


/* *** Public macro definition. *** */

#define orxMIN(A, B)              (((A) > (B)) ? (B) : (A))
#define orxMAX(A, B)              (((A) < (B)) ? (B) : (A))

#define orxCLAMP(V, MIN, MAX)     orxMAX(orxMIN(V, MAX), MIN)
#define orxCIRCULAR_CLAMP_INC_MIN(V, MIN, MAX)  \
do                                              \
{                                               \
  while((V) < (MIN))                            \
  {                                             \
    (V) += ((MAX) - (MIN));                     \
  }                                             \
  while((V) >= (MAX))                           \
  {                                             \
    (V) -= ((MAX) - (MIN));                     \
  }                                             \
} while(orxFALSE);

#define orxCIRCULAR_CLAMP_INC_MAX(V, MIN, MAX)  \
do                                              \
{                                               \
  while((V) <= (MIN))                           \
  {                                             \
    (V) += ((MAX) - (MIN));                     \
  }                                             \
  while((V) > (MAX))                            \
  {                                             \
    (V) -= ((MAX) - (MIN));                     \
  }                                             \
} while(orxFALSE);

#define orxABS(V)                 (((V) < 0) ? -(V) : (V))
#define orxFABS(V)                (((V) < orxFLOAT_0) ? -(V) : (V))

#define orxFRAND_INIT(SEED)       (srand(orxF2U(SEED)));
#define orxFRAND(MIN, MAX)        ((orx2F(rand()) * (orxFLOAT_1 / orx2F(RAND_MAX)) * (MAX - MIN)) + MIN)

#define orxF2U(V)                 ((orxU32)  (V))
#define orxF2S(V)                 ((orxS32)  (V))
#define orxU2F(V)                 ((orxFLOAT)(V))
#define orxS2F(V)                 ((orxFLOAT)(V))


/*** Math Definitions ***/

#define orxMATH_KF_SQRT_2         orx2F(1.414213562f)
#define orxMATH_KF_PI             orx2F(3.141592654f)
#define orxMATH_KF_PI_BY_2        orx2F(1.570796327f)
#define orxMATH_KF_DEG_TO_RAD     orx2F(orxMATH_KF_PI / 180.0f)
#define orxMATH_KF_RAD_TO_DEG     orx2F(180.0f / orxMATH_KF_PI)

#endif /* _orxMATH_H_ */
