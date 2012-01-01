/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2011 Orx-Project
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
  register orxU32 u32Temp;

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
 * @param[in]   _s32Seed                        Value to use as seed for random number generation
 */
void orxFASTCALL orxMath_InitRandom(orxS32 _s32Seed)
{
  /* Inits random seed */
  su32X = (orxU32)_s32Seed;
  su32Y = su32X * (orxU32)_s32Seed;
  su32Z = su32Y * (orxU32)_s32Seed;
  su32W = su32Z * (orxU32)_s32Seed;
}

/** Gets a random orxFLOAT value
 * @param[in]   _fMin                           Minimum boundary (inclusive)
 * @param[in]   _fMax                           Maximum boundary (exclusive)
 * @return      Random value
 */
orxFLOAT orxFASTCALL orxMath_GetRandomFloat(orxFLOAT _fMin, orxFLOAT _fMax)
{
  orxU32   u32Temp;
  orxFLOAT fResult;

  /* Gets next random number */
  u32Temp = orxMath_Xor128();

  /* Updates result */
  *((orxU32 *)&fResult) = (u32Temp >> 9) | 0x3f800000;
  fResult               = _fMin + (fResult - orxFLOAT_1) * (_fMax - _fMin);

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
