/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2022 Orx-Project
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

static orxU32 su32X = 123456789, su32Y = 362436069, su32Z = 521288629, su32W = 88675123;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

static orxINLINE orxU32 orxMath_Xor128()
{
  orxU32 u32Temp;

  u32Temp = su32X ^ (su32X << 11);
  su32X   = su32Y; su32Y = su32Z; su32Z = su32W;
  su32W   = su32W ^ (su32W >> 19) ^ (u32Temp ^ (u32Temp >> 8));

  /* Done! */
  return su32W;
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
  su32X = _u32Seed;
  su32Y = 362436069;
  su32Z = 521288629;
  su32W = 88675123;
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
  stSwap.u32Value = (orxMath_Xor128() >> 9) | 0x3f800000;

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
  stSwap.u32Value = (orxMath_Xor128() >> 9) | 0x3f800000;

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
  u32Temp = orxMath_Xor128();

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
  u32Temp = orxMath_Xor128();

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
  u32Temp = orxMath_Xor128();

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
  u32Temp = orxMath_Xor128();

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
  u64Temp = ((orxU64)orxMath_Xor128() << 32) | (orxU64)orxMath_Xor128();

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
  u64Temp = ((orxU64)orxMath_Xor128() << 32) | (orxU64)orxMath_Xor128();

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
  u64Temp = ((orxU64)orxMath_Xor128() << 32) | (orxU64)orxMath_Xor128();

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
  u64Temp = ((orxU64)orxMath_Xor128() << 32) | (orxU64)orxMath_Xor128();

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
  _au32Seeds[0] = su32X;
  _au32Seeds[1] = su32Y;
  _au32Seeds[2] = su32Z;
  _au32Seeds[3] = su32W;

  /* Done! */
  return;
}

/** Sets (replaces) the current random seeds
 * @param[in]   _au32Seeds[4]                   Seeds to set
 */
void orxFASTCALL orxMath_SetRandomSeeds(const orxU32 _au32Seeds[4])
{
  /* Gets all seeds */
  su32X = _au32Seeds[0];
  su32Y = _au32Seeds[1];
  su32Z = _au32Seeds[2];
  su32W = _au32Seeds[3];

  /* Done! */
  return;
}
