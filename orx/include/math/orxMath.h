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
 *   This library is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License           *
 *   as published by the Free Software Foundation; either version 2.1      *
 *   of the License, or (at your option) any later version.                *
 *                                                                         *
 ***************************************************************************/

#ifndef _orxMATH_H_
#define _orxMATH_H_


#include "orxInclude.h"

/* *** Maths related includes *** */
#include <math.h>                   /* Common maths / should be removed later */


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

#define orxFABS(V)                ((*(orxU32 *)&(V)) & 0x7FFFFFFFL)

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
#define orxMATH_KF_PI_BY_4        orx2F(0.785398163f)
#define orxMATH_KF_DEG_TO_RAD     orx2F(orxMATH_KF_PI / 180.0f)
#define orxMATH_KF_RAD_TO_DEG     orx2F(180.0f / orxMATH_KF_PI)


/*** Trigonometric function ***/

orxSTATIC orxINLINE orxFLOAT orxMath_Cos(orxFLOAT _fOp)
{
  orxREGISTER orxFLOAT fResult;

#ifdef __orxMSVC__
  
  /* Updates result */
  fResult = cosf(_fOp);

#else /* __orxMSVC__ */

  /* Updates result */
  fResult = cosf(_fOp);

#endif /* __orxMSVC__ */

  /* Done! */
  return fResult;
}

orxSTATIC orxINLINE orxFLOAT orxMath_Sin(orxFLOAT _fOp)
{
  orxREGISTER orxFLOAT fResult;

#ifdef __orxMSVC__
  
  /* Updates result */
  fResult = sinf(_fOp);

#else /* __orxMSVC__ */

  /* Updates result */
  fResult = sinf(_fOp);

#endif /* __orxMSVC__ */

  /* Done! */
  return fResult;
}

orxSTATIC orxINLINE orxFLOAT orxMath_Tan(orxFLOAT _fOp)
{
  orxREGISTER orxFLOAT fResult;

#ifdef __orxMSVC__
  
  /* Updates result */
  fResult = tanf(_fOp);

#else /* __orxMSVC__ */

  /* Updates result */
  fResult = tan(_fOp);

#endif /* __orxMSVC__ */

  /* Done! */
  return fResult;
}

orxSTATIC orxINLINE orxFLOAT orxMath_ACos(orxFLOAT _fOp)
{
  orxREGISTER orxFLOAT fResult;

#ifdef __orxMSVC__
  
  /* Updates result */
  fResult = acosf(_fOp);

#else /* __orxMSVC__ */

  /* Updates result */
  fResult = acosf(_fOp);

#endif /* __orxMSVC__ */

  /* Done! */
  return fResult;
}

orxSTATIC orxINLINE orxFLOAT orxMath_ATan(orxFLOAT _fOp1, orxFLOAT _fOp2)
{
  orxREGISTER orxFLOAT fResult;

#ifdef __orxMSVC__
  
  /* Updates result */
  fResult = atan2f(_fOp1, _fOp2);

#else /* __orxMSVC__ */

  /* Updates result */
  fResult = atan2f(_fOp1, _fOp2);

#endif /* __orxMSVC__ */

  /* Done! */
  return fResult;
}


/*** Misc functions ***/

orxSTATIC orxINLINE orxFLOAT orxMath_Sqrt(orxFLOAT _fOp)
{
  orxREGISTER orxFLOAT fResult;

#ifdef __orxMSVC__
  
  /* Updates result */
  fResult = sqrtf(_fOp);

#else /* __orxMSVC__ */

  /* Updates result */
  fResult = sqrtf(_fOp);

#endif /* __orxMSVC__ */

  /* Done! */
  return fResult;
}

orxSTATIC orxINLINE orxFLOAT orxMath_Floor(orxFLOAT _fOp)
{
  orxREGISTER orxFLOAT fResult;

#ifdef __orxMSVC__
  
  /* Updates result */
  fResult = floorf(_fOp);

#else /* __orxMSVC__ */

  /* Updates result */
  fResult = floorf(_fOp);

#endif /* __orxMSVC__ */

  /* Done! */
  return fResult;
}

orxSTATIC orxINLINE orxFLOAT orxMath_Ceil(orxFLOAT _fOp)
{
  orxREGISTER orxFLOAT fResult;

#ifdef __orxMSVC__
  
  /* Updates result */
  fResult = ceilf(_fOp);

#else /* __orxMSVC__ */

  /* Updates result */
  fResult = ceilf(_fOp);

#endif /* __orxMSVC__ */

  /* Done! */
  return fResult;
}

orxSTATIC orxINLINE orxFLOAT orxMath_Round(orxFLOAT _fOp)
{
  orxREGISTER orxFLOAT fResult;

#ifdef __orxMSVC__
  
  /* Updates result */
  fResult = (fmodf(_fOp, orxFLOAT_1) >= orx2F(0.5f)) ? ceilf(_fOp) : floorf(_fOp);

#else /* __orxMSVC__ */

  /* Updates result */
  fResult = rintf(_fOp);

#endif /* __orxMSVC__ */

  /* Done! */
  return fResult;
}

#endif /* _orxMATH_H_ */
