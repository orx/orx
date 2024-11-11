/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008- Orx-Project
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
 * @file orxMath.c
 * @date 27/11/2008
 * @author iarwain@orx-project.org
 *
 */


#include "math/orxMath.h"


/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

static orxU64 su64State;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

static orxINLINE orxU64 orxMath_SplitMix64()
{
  orxU64 u64Result;

  u64Result = su64State;
  su64State = su64State + 0x9E3779B97F4A7C15;
  u64Result = (u64Result ^ (u64Result >> 30)) * 0xBF58476D1CE4E5B9;
  u64Result = (u64Result ^ (u64Result >> 27)) * 0x94D049BB133111EB;

  /* Done! */
  return(u64Result ^ (u64Result >> 31));
}


/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** Inits the random seed
 * @param[in]   _u32Seed                        Value to use as seed for random number generation
 */
void orxFASTCALL orxMath_InitRandom(orxU32 _u32Seed)
{
  /* Checks */
  orxASSERT(_u32Seed != 0);

  /* Inits random seed */
  su64State = (orxU64)_u32Seed * (orxU64)_u32Seed;
  
  /* Done! */
  return;
}

/** Gets a random orxFLOAT value
 * @param[in]   _fMin                           Minimum boundary (inclusive)
 * @param[in]   _fMax                           Maximum boundary (exclusive)
 * @return      Random value
 */
orxFLOAT orxFASTCALL orxMath_GetRandomFloat(orxFLOAT _fMin, orxFLOAT _fMax)
{
  union
  {
    orxU32    u32Value;
    orxFLOAT  fValue;
  } stSwap;
  orxFLOAT fResult;

  /* Gets next random number (as float) */
  stSwap.u32Value = (((orxU32)orxMath_SplitMix64()) >> 9) | 0x3f800000;

  /* Updates result */
  fResult = _fMin + (stSwap.fValue - orxFLOAT_1) * (_fMax - _fMin);

  /* Done! */
  return fResult;
}

/** Gets a random orxFLOAT value using step increments
 * @param[in]   _fMin                           Minimum boundary (inclusive)
 * @param[in]   _fMax                           Maximum boundary (inclusive)
 * @param[in]   _fStep                          Step value, must be strictly positive
 * @return      Random value
 */
orxFLOAT orxFASTCALL orxMath_GetSteppedRandomFloat(orxFLOAT _fMin, orxFLOAT _fMax, orxFLOAT _fStep)
{
  union
  {
    orxU32    u32Value;
    orxFLOAT  fValue;
  } stSwap;
  orxFLOAT fTemp, fResult;

  /* Checks */
  orxASSERT(_fStep >= orxMATH_KF_TINY_EPSILON);

  /* Gets next random number (as float) */
  stSwap.u32Value = (((orxU32)orxMath_SplitMix64()) >> 9) | 0x3f800000;

  /* Updates result */
  fTemp   = orxMath_Abs(_fMax - _fMin);
  fTemp   = (stSwap.fValue - orxFLOAT_1) * (fTemp + _fStep - orxMath_Mod(fTemp, _fStep) - orxMATH_KF_EPSILON);
  fResult = orxMIN(_fMin, _fMax) + (fTemp - orxMath_Mod(fTemp, _fStep));

  /* Done! */
  return fResult;
}

/** Gets a random orxU32 value
 * @param[in]   _u32Min                         Minimum boundary (inclusive)
 * @param[in]   _u32Max                         Maximum boundary (inclusive)
 * @return      Random value
 */
orxU32 orxFASTCALL orxMath_GetRandomU32(orxU32 _u32Min, orxU32 _u32Max)
{
  orxU32 u32Temp, u32Result;

  /* Gets next random number */
  u32Temp = (orxU32)orxMath_SplitMix64();

  /* Updates result */
  u32Result = _u32Min + (u32Temp % ((_u32Max - _u32Min) + 1));

  /* Done! */
  return u32Result;
}

/** Gets a random U32 value using step increments
 * @param[in]   _u32Min                         Minimum boundary (inclusive)
 * @param[in]   _u32Max                         Maximum boundary (inclusive)
 * @param[in]   _u32Step                        Step value, must be strictly positive
 * @return      Random value
 */
orxU32 orxFASTCALL orxMath_GetSteppedRandomU32(orxU32 _u32Min, orxU32 _u32Max, orxU32 _u32Step)
{
  orxU32 u32Temp, u32Result;

  /* Checks */
  orxASSERT(_u32Step > 0);

  /* Gets next random number */
  u32Temp = (orxU32)orxMath_SplitMix64();

  /* Updates result */
  u32Temp   = u32Temp % ((_u32Max - _u32Min) + 1);
  u32Result = _u32Min + (u32Temp - (u32Temp % _u32Step));

  /* Done! */
  return u32Result;
}

/** Gets a random orxS32 value
 * @param[in]   _s32Min                         Minimum boundary (inclusive)
 * @param[in]   _s32Max                         Maximum boundary (inclusive)
 * @return      Random value
 */
orxS32 orxFASTCALL orxMath_GetRandomS32(orxS32 _s32Min, orxS32 _s32Max)
{
  orxU32 u32Temp;
  orxS32 s32Result;

  /* Gets next random number */
  u32Temp = (orxU32)orxMath_SplitMix64();

  /* Updates result */
  s32Result = _s32Min + (u32Temp % ((_s32Max - _s32Min) + 1));

  /* Done! */
  return s32Result;
}

/** Gets a random S32 value using step increments
 * @param[in]   _s32Min                         Minimum boundary (inclusive)
 * @param[in]   _s32Max                         Maximum boundary (inclusive)
 * @param[in]   _s32Step                        Step value, must be strictly positive
 * @return      Random value
 */
orxS32 orxFASTCALL orxMath_GetSteppedRandomS32(orxS32 _s32Min, orxS32 _s32Max, orxS32 _s32Step)
{
  orxU32 u32Temp;
  orxS32 s32Result;

  /* Checks */
  orxASSERT(_s32Step > 0);

  /* Gets next random number */
  u32Temp = (orxU32)orxMath_SplitMix64();

  /* Updates result */
  u32Temp   = u32Temp % ((_s32Max - _s32Min) + 1);
  s32Result = _s32Min + (u32Temp - (u32Temp % _s32Step));

  /* Done! */
  return s32Result;
}

/** Gets a random orxU64 value
 * @param[in]   _u64Min                         Minimum boundary (inclusive)
 * @param[in]   _u64Max                         Maximum boundary (inclusive)
 * @return      Random value
 */
orxU64 orxFASTCALL orxMath_GetRandomU64(orxU64 _u64Min, orxU64 _u64Max)
{
  orxU64 u64Temp, u64Result;

  /* Gets next random number */
  u64Temp = orxMath_SplitMix64();

  /* Updates result */
  u64Result = _u64Min + (u64Temp % ((_u64Max - _u64Min) + 1));

  /* Done! */
  return u64Result;
}

/** Gets a random U64 value using step increments
 * @param[in]   _u64Min                         Minimum boundary (inclusive)
 * @param[in]   _u64Max                         Maximum boundary (inclusive)
 * @param[in]   _u64Step                        Step value, must be strictly positive
 * @return      Random value
 */
orxU64 orxFASTCALL orxMath_GetSteppedRandomU64(orxU64 _u64Min, orxU64 _u64Max, orxU64 _u64Step)
{
  orxU64 u64Temp, u64Result;

  /* Checks */
  orxASSERT(_u64Step > 0);

  /* Gets next random number */
  u64Temp = orxMath_SplitMix64();

  /* Updates result */
  u64Temp   = u64Temp % ((_u64Max - _u64Min) + 1);
  u64Result = _u64Min + (u64Temp - (u64Temp % _u64Step));

  /* Done! */
  return u64Result;
}

/** Gets a random orxS64 value
 * @param[in]   _s64Min                         Minimum boundary (inclusive)
 * @param[in]   _s64Max                         Maximum boundary (inclusive)
 * @return      Random value
 */
orxS64 orxFASTCALL orxMath_GetRandomS64(orxS64 _s64Min, orxS64 _s64Max)
{
  orxU64 u64Temp;
  orxS64 s64Result;

  /* Gets next random number */
  u64Temp = orxMath_SplitMix64();

  /* Updates result */
  s64Result = _s64Min + (u64Temp % ((_s64Max - _s64Min) + 1));

  /* Done! */
  return s64Result;
}

/** Gets a random S64 value using step increments
 * @param[in]   _s64Min                         Minimum boundary (inclusive)
 * @param[in]   _s64Max                         Maximum boundary (inclusive)
 * @param[in]   _s64Step                        Step value, must be strictly positive
 * @return      Random value
 */
orxS64 orxFASTCALL orxMath_GetSteppedRandomS64(orxS64 _s64Min, orxS64 _s64Max, orxS64 _s64Step)
{
  orxU64 u64Temp;
  orxS64 s64Result;

  /* Checks */
  orxASSERT(_s64Step > 0);

  /* Gets next random number */
  u64Temp = orxMath_SplitMix64();

  /* Updates result */
  u64Temp   = u64Temp % ((_s64Max - _s64Min) + 1);
  s64Result = _s64Min + (u64Temp - (u64Temp % _s64Step));

  /* Done! */
  return s64Result;
}

/** Gets the current random seeds
 * @param[out]  _au32Seeds[4]                   Current seeds
 */
void orxFASTCALL orxMath_GetRandomSeeds(orxU32 _au32Seeds[4])
{
  /* Gets all seeds */
  _au32Seeds[0] = (orxU32)su64State;
  _au32Seeds[1] = (orxU32)(su64State >> 32);
  _au32Seeds[2] = 0;
  _au32Seeds[3] = 0;

  /* Done! */
  return;
}

/** Sets (replaces) the current random seeds
 * @param[in]   _au32Seeds[4]                   Seeds to set
 */
void orxFASTCALL orxMath_SetRandomSeeds(const orxU32 _au32Seeds[4])
{
  /* Restores state */
  su64State = (((orxU64)_au32Seeds[1]) << 32) | (orxU64)_au32Seeds[0];

  /* Done! */
  return;
}
