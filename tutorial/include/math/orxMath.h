/* Orx - Portable Game Engine
 *
 * Orx is the legal property of its developers, whose names
 * are listed in the COPYRIGHT file distributed 
 * with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

/**
 * @file orxMath.h
 * @date 30/03/2005
 * @author iarwain@orx-project.org
 *
 * @todo
 */

/**
 * @addtogroup orxMath
 * 
 * Math module
 * Contains all base math functions
 *
 * @{
 */


#ifndef _orxMATH_H_
#define _orxMATH_H_


#include "orxInclude.h"

/** Maths related includes
 */
#include <math.h>
#include <stdlib.h>


/** Public macro
 */

/** Lerps between two values given a parameter T [0, 1]
 * @param[in]   A                               First value (will be selected for T = 0)
 * @param[in]   B                               Second value (will be selected for T = 1)
 * @param[in]   T                               Lerp coefficient parameter [0, 1]
 * @return      Lerped value
 */
#define orxLERP(A, B, T)          ((A) + ((T) * ((B) - (A))))


/** Gets minimum between two values
 * @param[in]   A                               First value
 * @param[in]   B                               Second value
 * @return      Minimum between A & B
 */
#define orxMIN(A, B)              (((A) > (B)) ? (B) : (A))

/** Gets maximum between two values
 * @param[in]   A                               First value
 * @param[in]   B                               Second value
 * @return      Maximum between A & B
 */
#define orxMAX(A, B)              (((A) < (B)) ? (B) : (A))

/** Gets clamped value between two boundaries
 * @param[in]   V                               Value to clamp
 * @param[in]   MIN                             Minimum boundary
 * @param[in]   MAX                             Maximum boundary
 * @return      Clamped value between MIN & MAX
 */
#define orxCLAMP(V, MIN, MAX)     orxMAX(orxMIN(V, MAX), MIN)

/** Gets circularly clamped (for ring spaces) value between two boundaries [MIN, MAX[
 * @param[in]   V                               Value to clamp
 * @param[in]   MIN                             Minimum boundary
 * @param[in]   MAX                             Maximum boundary
 * @return      Circularly clamped value between MIN & MAX
 */
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
} while(0)

/** Gets circularly clamped (for ring spaces) value between two boundaries ]MIN, MAX]
 * @param[in]   V                               Value to clamp
 * @param[in]   MIN                             Minimum boundary
 * @param[in]   MAX                             Maximum boundary
 * @return      Circularly clamped value between MIN & MAX
 */
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
} while(0)

/** Converts an orxFLOAT to an orxU32
 * @param[in]   V                               Value to convert
 * @return      Converted value
 */
#define orxF2U(V)                 ((orxU32)  (V))

/** Converts an orxFLOAT to an orxS32
 * @param[in]   V                               Value to convert
 * @return      Converted value
 */
#define orxF2S(V)                 ((orxS32)  (V))

/** Converts an orxU32 to an orxFLOAT
 * @param[in]   V                               Value to convert
 * @return      Converted value
 */
#define orxU2F(V)                 ((orxFLOAT)(V))

/** Converts an orxS32 to an orxFLOAT
 * @param[in]   V                               Value to convert
 * @return      Converted value
 */
#define orxS2F(V)                 ((orxFLOAT)(V))


/*** Module functions *** */

/** Inits the random seed with an orxFLOAT
 * @param[in]   _u32Seed                        Value to use as seed for random number generation
 */
extern orxDLLAPI void orxFASTCALL     orxMath_InitRandom(orxU32 _u32Seed);

/** Gets a random orxFLOAT value
 * @param[in]   _fMin                           Minimum boundary
 * @param[in]   _fMax                           Maximum boundary
 * @return      Random value
 */
extern orxDLLAPI orxFLOAT orxFASTCALL orxMath_GetRandomFloat(orxFLOAT _fMin, orxFLOAT _fMax);

/** Gets a random orxU32 value
 * @param[in]   _u32Min                         Minimum boundary
 * @param[in]   _u32Max                         Maximum boundary
 * @return      Random value
 */
extern orxDLLAPI orxU32 orxFASTCALL   orxMath_GetRandomU32(orxU32 _u32Min, orxU32 _u32Max);

/** Gets a random orxS32 value
 * @param[in]   _s32Min                         Minimum boundary
 * @param[in]   _s32Max                         Maximum boundary
 * @return      Random value
 */
extern orxDLLAPI orxS32 orxFASTCALL   orxMath_GetRandomS32(orxS32 _s32Min, orxS32 _s32Max);


/*** Inlined functions *** */

/** Gets the counts of bit in an orxU32
 * @param[in]   _u32Value                       Value to process
 * @return      Number of bits that are set in the value
 */
static orxINLINE orxU32 orxMath_GetBitCount(register orxU32 _u32Value)
{
  _u32Value -= ((_u32Value >> 1) & 0x55555555);
  _u32Value = (((_u32Value >> 2) & 0x33333333) + (_u32Value & 0x33333333));
  _u32Value = (((_u32Value >> 4) + _u32Value) & 0x0f0f0f0f);
  _u32Value += (_u32Value >> 8);
  _u32Value += (_u32Value >> 16);
  return(_u32Value & 0x0000003f);
}


/*** Math Definitions ***/

#define orxMATH_KF_SQRT_2         orx2F(1.414213562f)           /**< Sqrt(2) constant */
#define orxMATH_KF_EPSILON        orx2F(0.0001f)                /**< Epsilon constant */
#define orxMATH_KF_2_PI           orx2F(6.283185307f)           /**< 2 PI constant */
#define orxMATH_KF_PI             orx2F(3.141592654f)           /**< PI constant */
#define orxMATH_KF_PI_BY_2        orx2F(1.570796327f)           /**< PI / 2 constant */
#define orxMATH_KF_PI_BY_4        orx2F(0.785398163f)           /**< PI / 4 constant */
#define orxMATH_KF_DEG_TO_RAD     orx2F(3.141592654f / 180.0f)  /**< Degree to radian conversion constant */
#define orxMATH_KF_RAD_TO_DEG     orx2F(180.0f / 3.141592654f)  /**< Radian to degree conversion constant */


/*** Trigonometric function ***/

/** Gets a cosine
 * @param[in]   _fOp                            Input radian angle value
 * @return      Cosine of the given angle
 */
static orxINLINE orxFLOAT    orxMath_Cos(orxFLOAT _fOp)
{
  register orxFLOAT fResult;

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

/** Gets a sine
 * @param[in]   _fOp                            Input radian angle value
 * @return      Sine of the given angle
 */
static orxINLINE orxFLOAT    orxMath_Sin(orxFLOAT _fOp)
{
  register orxFLOAT fResult;

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

/** Gets a tangent
 * @param[in]   _fOp                            Input radian angle value
 * @return      Tangent of the given angle
 */
static orxINLINE orxFLOAT    orxMath_Tan(orxFLOAT _fOp)
{
  register orxFLOAT fResult;

#ifdef __orxMSVC__
  
  /* Updates result */
  fResult = tanf(_fOp);

#else /* __orxMSVC__ */

  /* Updates result */
  fResult = tanf(_fOp);

#endif /* __orxMSVC__ */

  /* Done! */
  return fResult;
}

/** Gets an arccosine
 * @param[in]   _fOp                            Input radian angle value
 * @return      Arccosine of the given angle
 */
static orxINLINE orxFLOAT    orxMath_ACos(orxFLOAT _fOp)
{
  register orxFLOAT fResult;

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

/** Gets an arcsine
 * @param[in]   _fOp                            Input radian angle value
 * @return      Arcsine of the given angle
 */
static orxINLINE orxFLOAT    orxMath_ASin(orxFLOAT _fOp)
{
  register orxFLOAT fResult;

#ifdef __orxMSVC__
  
  /* Updates result */
  fResult = asinf(_fOp);

#else /* __orxMSVC__ */

  /* Updates result */
  fResult = asinf(_fOp);

#endif /* __orxMSVC__ */

  /* Done! */
  return fResult;
}

/** Gets an arctangent
 * @param[in]   _fOp1                           First operand
 * @param[in]   _fOp2                           Second operand
 * @return      Arctangent of the given angle
 */
static orxINLINE orxFLOAT    orxMath_ATan(orxFLOAT _fOp1, orxFLOAT _fOp2)
{
  register orxFLOAT fResult;

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

/** Gets a square root
 * @param[in]   _fOp                            Input value
 * @return      Square root of the given value
 */
static orxINLINE orxFLOAT    orxMath_Sqrt(orxFLOAT _fOp)
{
  register orxFLOAT fResult;

  /* Updates result */
  fResult = sqrtf(_fOp);

  /* Done! */
  return fResult;
}

/** Gets a floored value
 * @param[in]   _fOp                            Input value
 * @return      Floored value
 */
static orxINLINE orxFLOAT    orxMath_Floor(orxFLOAT _fOp)
{
  register orxFLOAT fResult;

  /* Updates result */
  fResult = floorf(_fOp);

  /* Done! */
  return fResult;
}

/** Gets a ceiled value
 * @param[in]   _fOp                            Input value
 * @return      Ceiled value
 */
static orxINLINE orxFLOAT    orxMath_Ceil(orxFLOAT _fOp)
{
  register orxFLOAT fResult;

  /* Updates result */
  fResult = ceilf(_fOp);

  /* Done! */
  return fResult;
}

/** Gets a rounded value
 * @param[in]   _fOp                            Input value
 * @return      Rounded value
 */
static orxINLINE orxFLOAT    orxMath_Round(orxFLOAT _fOp)
{
  register orxFLOAT fResult;

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

/** Gets a modulo value
 * @param[in]   _fOp1                           Input value
 * @param[in]   _fOp2                           Modulo value
 * @return      Modulo value
 */
static orxINLINE orxFLOAT    orxMath_Mod(orxFLOAT _fOp1, orxFLOAT _fOp2)
{
  register orxFLOAT fResult;

  /* Updates result */
  fResult = fmodf(_fOp1, _fOp2);

  /* Done! */
  return fResult;
}

/** Gets a powed value
 * @param[in]   _fOp                            Input value
 * @param[in]   _fExp                           Exponent value
 * @return      Powed value
 */
static orxINLINE orxFLOAT    orxMath_Pow(orxFLOAT _fOp, orxFLOAT _fExp)
{
  register orxFLOAT fResult;

  /* Updates result */
  fResult = powf(_fOp, _fExp);

  /* Done! */
  return fResult;
}

/** Gets an absolute value
 * @param[in]   _fOp                            Input value
 * @return      Absolute value
 */
static orxINLINE orxFLOAT    orxMath_Abs(orxFLOAT _fOp)
{
  /* Done! */
  return fabsf(_fOp);
}

#endif /* _orxMATH_H_ */

/** @} */
