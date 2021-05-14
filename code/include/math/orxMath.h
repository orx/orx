/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2021 Orx-Project
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 *    1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 *
 *    2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 *    3. This notice may not be removed or altered from any source
 *    distribution.
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
#include "debug/orxDebug.h"

/** Maths related includes
 */
#include <math.h>
#ifdef __orxMSVC__
  #include <intrin.h>
#endif /* __orxMSVC__ */


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

/** Inits the random seed
 * @param[in]   _u32Seed                        Value to use as seed for random number generation
 */
extern orxDLLAPI void orxFASTCALL     orxMath_InitRandom(orxU32 _u32Seed);

/** Gets a random orxFLOAT value
 * @param[in]   _fMin                           Minimum boundary (inclusive)
 * @param[in]   _fMax                           Maximum boundary (exclusive)
 * @return      Random value
 */
extern orxDLLAPI orxFLOAT orxFASTCALL orxMath_GetRandomFloat(orxFLOAT _fMin, orxFLOAT _fMax);

/** Gets a random orxFLOAT value using step increments
 * @param[in]   _fMin                           Minimum boundary (inclusive)
 * @param[in]   _fMax                           Maximum boundary (inclusive)
 * @param[in]   _fStep                          Step value, must be strictly positive
 * @return      Random value
 */
extern orxDLLAPI orxFLOAT orxFASTCALL orxMath_GetSteppedRandomFloat(orxFLOAT _fMin, orxFLOAT _fMax, orxFLOAT _fStep);

/** Gets a random orxU32 value
 * @param[in]   _u32Min                         Minimum boundary (inclusive)
 * @param[in]   _u32Max                         Maximum boundary (inclusive)
 * @return      Random value
 */
extern orxDLLAPI orxU32 orxFASTCALL   orxMath_GetRandomU32(orxU32 _u32Min, orxU32 _u32Max);

/** Gets a random U32 value using step increments
 * @param[in]   _u32Min                         Minimum boundary (inclusive)
 * @param[in]   _u32Max                         Maximum boundary (inclusive)
 * @param[in]   _u32Step                        Step value, must be strictly positive
 * @return      Random value
 */
extern orxDLLAPI orxU32 orxFASTCALL   orxMath_GetSteppedRandomU32(orxU32 _u32Min, orxU32 _u32Max, orxU32 _u32Step);

/** Gets a random orxS32 value
 * @param[in]   _s32Min                         Minimum boundary (inclusive)
 * @param[in]   _s32Max                         Maximum boundary (inclusive)
 * @return      Random value
 */
extern orxDLLAPI orxS32 orxFASTCALL   orxMath_GetRandomS32(orxS32 _s32Min, orxS32 _s32Max);

/** Gets a random S32 value using step increments
 * @param[in]   _s32Min                         Minimum boundary (inclusive)
 * @param[in]   _s32Max                         Maximum boundary (inclusive)
 * @param[in]   _s32Step                        Step value, must be strictly positive
 * @return      Random value
 */
extern orxDLLAPI orxS32 orxFASTCALL   orxMath_GetSteppedRandomS32(orxS32 _s32Min, orxS32 _s32Max, orxS32 _s32Step);

/** Gets a random orxU64 value
 * @param[in]   _u64Min                         Minimum boundary (inclusive)
 * @param[in]   _u64Max                         Maximum boundary (inclusive)
 * @return      Random value
 */
extern orxDLLAPI orxU64 orxFASTCALL   orxMath_GetRandomU64(orxU64 _u64Min, orxU64 _u64Max);

/** Gets a random U64 value using step increments
 * @param[in]   _u64Min                         Minimum boundary (inclusive)
 * @param[in]   _u64Max                         Maximum boundary (inclusive)
 * @param[in]   _u64Step                        Step value, must be strictly positive
 * @return      Random value
 */
extern orxDLLAPI orxU64 orxFASTCALL   orxMath_GetSteppedRandomU64(orxU64 _u64Min, orxU64 _u64Max, orxU64 _u64Step);

/** Gets a random orxS64 value
 * @param[in]   _s64Min                         Minimum boundary (inclusive)
 * @param[in]   _s64Max                         Maximum boundary (inclusive)
 * @return      Random value
 */
extern orxDLLAPI orxS64 orxFASTCALL   orxMath_GetRandomS64(orxS64 _s64Min, orxS64 _s64Max);

/** Gets a random S64 value using step increments
 * @param[in]   _s64Min                         Minimum boundary (inclusive)
 * @param[in]   _s64Max                         Maximum boundary (inclusive)
 * @param[in]   _s64Step                        Step value, must be strictly positive
 * @return      Random value
 */
extern orxDLLAPI orxS64 orxFASTCALL   orxMath_GetSteppedRandomS64(orxS64 _s64Min, orxS64 _s64Max, orxS64 _s64Step);

/** Gets the current random seeds
 * @param[out]  _au32Seeds                      Current seeds
 */
extern orxDLLAPI void orxFASTCALL     orxMath_GetRandomSeeds(orxU32 _au32Seeds[4]);

/** Sets (replaces) the current random seeds
 * @param[in]   _au32Seeds                      Seeds to set
 */
extern orxDLLAPI void orxFASTCALL     orxMath_SetRandomSeeds(const orxU32 _au32Seeds[4]);


/*** Inlined functions *** */

/** Gets the count of bit in an orxU32
 * @param[in]   _u32Value                       Value to process
 * @return      Number of bits that are set in the value
 */
static orxINLINE orxU32               orxMath_GetBitCount(orxU32 _u32Value)
{
  orxU32 u32Result;

#ifdef __orxMSVC__

  /* Uses intrinsic */
  u32Result = __popcnt(_u32Value);

#else /* __orxMSVC__ */

  /* Uses intrinsic */
  u32Result = (orxU32)__builtin_popcount(_u32Value);

#endif /* __orxMSVC__ */

  /* Done! */
  return u32Result;
}

/** Gets the count of trailing zeros in an orxU32
 * @param[in]   _u32Value                       Value to process
 * @return      Number of trailing zeros
 */
static orxINLINE orxU32               orxMath_GetTrailingZeroCount(orxU32 _u32Value)
{
  orxU32 u32Result;

  /* Checks */
  orxASSERT(_u32Value != 0);

#ifdef __orxMSVC__

  /* Uses intrinsic */
  _BitScanForward((unsigned long *)&u32Result, _u32Value);

#else /* __orxMSVC__ */

  /* Uses intrinsic */
  u32Result = (orxU32)__builtin_ctz(_u32Value);

#endif /* __orxMSVC__ */

  /* Done! */
  return u32Result;
}

/** Gets the count of trailing zeros in an orxU64
 * @param[in]   _u64Value                       Value to process
 * @return      Number of trailing zeros
 */
static orxINLINE orxU32               orxMath_GetTrailingZeroCount64(orxU64 _u64Value)
{
  orxU32 u32Result;

  /* Checks */
  orxASSERT(_u64Value != 0);

#ifdef __orxMSVC__

  #ifdef __orx64__

  /* Uses intrinsic */
  _BitScanForward64((unsigned long *)&u32Result, _u64Value);

  #else /* __orx64__ */

  /* Updates result */
  u32Result = ((_u64Value & 0xFFFFFFFFULL) == 0) ? orxMath_GetTrailingZeroCount((orxU32)(_u64Value >> 32)) + 32 : orxMath_GetTrailingZeroCount((orxU32)_u64Value);

  #endif /* __orx64__ */

#else /* __orxMSVC__ */

  /* Uses intrinsic */
  u32Result = (orxU32)__builtin_ctzll(_u64Value);

#endif /* __orxMSVC__ */

  /* Done! */
  return u32Result;
}

/** Is value a power of two?
 * @param[in]   _u32Value                       Value to test
 * @return      orxTRUE / orxFALSE
 */
static orxINLINE orxBOOL              orxMath_IsPowerOfTwo(orxU32 _u32Value)
{
  orxBOOL bResult;

  /* Updates result */
  bResult = ((_u32Value & (_u32Value - 1)) == 0) ? orxTRUE : orxFALSE;

  /* Done! */
  return bResult;
}

/** Gets next power of two of an orxU32
 * @param[in]   _u32Value                       Value to process
 * @return      If _u32Value is already a power of two, returns it, otherwise the next power of two
 */
static orxINLINE orxU32               orxMath_GetNextPowerOfTwo(orxU32 _u32Value)
{
  orxU32 u32Result;

  /* Non-zero? */
  if(_u32Value != 0)
  {
    /* Updates result */
    u32Result = _u32Value - 1;
    u32Result = u32Result | (u32Result >> 1);
    u32Result = u32Result | (u32Result >> 2);
    u32Result = u32Result | (u32Result >> 4);
    u32Result = u32Result | (u32Result >> 8);
    u32Result = u32Result | (u32Result >> 16);
    u32Result++;
  }
  else
  {
    /* Updates result */
    u32Result = 1;
  }

  /* Done! */
  return u32Result;
}

/** Gets smooth stepped value between two extrema
 * @param[in]   _fMin                           Minimum value
 * @param[in]   _fMax                           Maximum value
 * @param[in]   _fValue                         Value to process
 * @return      0.0 if _fValue <= _fMin, 1.0 if _fValue >= _fMax, smoothed value between 0.0 & 1.0 otherwise
 */
static orxINLINE orxFLOAT             orxMath_SmoothStep(orxFLOAT _fMin, orxFLOAT _fMax, orxFLOAT _fValue)
{
  orxFLOAT fTemp, fResult;

  /* Gets normalized and clamped value */
  fTemp = (_fValue - _fMin) / (_fMax - _fMin);
  fTemp = orxCLAMP(fTemp, orxFLOAT_0, orxFLOAT_1);

  /* Gets smoothed result */
  fResult = fTemp * fTemp * (orx2F(3.0f) - (orx2F(2.0f) * fTemp));

  /* Done! */
  return fResult;
}

/** Gets smoother stepped value between two extrema
 * @param[in]   _fMin                           Minimum value
 * @param[in]   _fMax                           Maximum value
 * @param[in]   _fValue                         Value to process
 * @return      0.0 if _fValue <= _fMin, 1.0 if _fValue >= _fMax, smooth(er)ed value between 0.0 & 1.0 otherwise
 */
static orxINLINE orxFLOAT             orxMath_SmootherStep(orxFLOAT _fMin, orxFLOAT _fMax, orxFLOAT _fValue)
{
  orxFLOAT fTemp, fResult;

  /* Gets normalized and clamped value */
  fTemp = (_fValue - _fMin) / (_fMax - _fMin);
  fTemp = orxCLAMP(fTemp, orxFLOAT_0, orxFLOAT_1);

  /* Gets smoothed result */
  fResult = fTemp * fTemp * fTemp * (fTemp * ((fTemp * orx2F(6.0f)) - orx2F(15.0f)) + orx2F(10.0f));

  /* Done! */
  return fResult;
}


/*** Math Definitions ***/

#define orxMATH_KF_SQRT_2             orx2F(1.414213562f)           /**< Sqrt(2) constant */
#define orxMATH_KF_EPSILON            orx2F(0.0001f)                /**< Epsilon constant */
#define orxMATH_KF_TINY_EPSILON       orx2F(1.0e-037f)              /**< Tiny epsilon */
#define orxMATH_KF_2_PI               orx2F(6.283185307f)           /**< 2 PI constant */
#define orxMATH_KF_PI                 orx2F(3.141592654f)           /**< PI constant */
#define orxMATH_KF_PI_BY_2            orx2F(1.570796327f)           /**< PI / 2 constant */
#define orxMATH_KF_PI_BY_4            orx2F(0.785398163f)           /**< PI / 4 constant */
#define orxMATH_KF_DEG_TO_RAD         orx2F(3.141592654f / 180.0f)  /**< Degree to radian conversion constant */
#define orxMATH_KF_RAD_TO_DEG         orx2F(180.0f / 3.141592654f)  /**< Radian to degree conversion constant */


/*** Trigonometric function ***/

/** Gets a sine
 * @param[in]   _fOp                            Input radian angle value
 * @return      Sine of the given angle
 */
static orxINLINE orxFLOAT             orxMath_Sin(orxFLOAT _fOp)
{
  orxFLOAT fResult;

  /* Updates result */
  fResult = sinf(_fOp);

  /* Done! */
  return fResult;
}

/** Gets a cosine
 * @param[in]   _fOp                            Input radian angle value
 * @return      Cosine of the given angle
 */
static orxINLINE orxFLOAT             orxMath_Cos(orxFLOAT _fOp)
{
  orxFLOAT fResult;

  /* Updates result */
  fResult = cosf(_fOp);

  /* Done! */
  return fResult;
}

/** Gets a tangent
 * @param[in]   _fOp                            Input radian angle value
 * @return      Tangent of the given angle
 */
static orxINLINE orxFLOAT             orxMath_Tan(orxFLOAT _fOp)
{
  orxFLOAT fResult;

  /* Updates result */
  fResult = tanf(_fOp);

  /* Done! */
  return fResult;
}

/** Gets an arccosine
 * @param[in]   _fOp                            Input radian angle value
 * @return      Arccosine of the given angle
 */
static orxINLINE orxFLOAT             orxMath_ACos(orxFLOAT _fOp)
{
  orxFLOAT fResult;

  /* Updates result */
  fResult = acosf(_fOp);

  /* Done! */
  return fResult;
}

/** Gets an arcsine
 * @param[in]   _fOp                            Input radian angle value
 * @return      Arcsine of the given angle
 */
static orxINLINE orxFLOAT             orxMath_ASin(orxFLOAT _fOp)
{
  orxFLOAT fResult;

  /* Updates result */
  fResult = asinf(_fOp);

  /* Done! */
  return fResult;
}

/** Gets an arctangent
 * @param[in]   _fOp1                           First operand
 * @param[in]   _fOp2                           Second operand
 * @return      Arctangent of the given angle
 */
static orxINLINE orxFLOAT             orxMath_ATan(orxFLOAT _fOp1, orxFLOAT _fOp2)
{
  orxFLOAT fResult;

  /* Updates result */
  fResult = atan2f(_fOp1, _fOp2);

  /* Done! */
  return fResult;
}


/*** Misc functions ***/

/** Gets a square root
 * @param[in]   _fOp                            Input value
 * @return      Square root of the given value
 */
static orxINLINE orxFLOAT             orxMath_Sqrt(orxFLOAT _fOp)
{
  orxFLOAT fResult;

  /* Updates result */
  fResult = sqrtf(_fOp);

  /* Done! */
  return fResult;
}

/** Gets a floored value
 * @param[in]   _fOp                            Input value
 * @return      Floored value
 */
static orxINLINE orxFLOAT             orxMath_Floor(orxFLOAT _fOp)
{
  orxFLOAT fResult;

  /* Updates result */
  fResult = floorf(_fOp);

  /* Done! */
  return fResult;
}

/** Gets a ceiled value
 * @param[in]   _fOp                            Input value
 * @return      Ceiled value
 */
static orxINLINE orxFLOAT             orxMath_Ceil(orxFLOAT _fOp)
{
  orxFLOAT fResult;

  /* Updates result */
  fResult = ceilf(_fOp);

  /* Done! */
  return fResult;
}

/** Gets a rounded value
 * @param[in]   _fOp                            Input value
 * @return      Rounded value
 */
static orxINLINE orxFLOAT             orxMath_Round(orxFLOAT _fOp)
{
  orxFLOAT fResult;

#ifdef __orxMSVC__

  /* Updates result */
  fResult = floorf(_fOp + orx2F(0.5f));

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
static orxINLINE orxFLOAT             orxMath_Mod(orxFLOAT _fOp1, orxFLOAT _fOp2)
{
  orxFLOAT fResult;

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
static orxINLINE orxFLOAT             orxMath_Pow(orxFLOAT _fOp, orxFLOAT _fExp)
{
  orxFLOAT fResult;

  /* Updates result */
  fResult = powf(_fOp, _fExp);

  /* Done! */
  return fResult;
}

/** Gets an absolute value
 * @param[in]   _fOp                            Input value
 * @return      Absolute value
 */
static orxINLINE orxFLOAT             orxMath_Abs(orxFLOAT _fOp)
{
  orxFLOAT fResult;

  /* Updates result */
  fResult = fabsf(_fOp);

  /* Done! */
  return fResult;
}

#endif /* _orxMATH_H_ */

/** @} */
