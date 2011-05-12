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


static orxS32 ss32RandomSeed = 0;


void orxFASTCALL orxMath_InitRandom(orxS32 _s32Seed)
{
  /* Inits random seed */
  ss32RandomSeed = _s32Seed;;
}

/** Gets a random orxFLOAT value
 * @param[in]   _fMin                           Minimum boundary (inclusive)
 * @param[in]   _fMax                           Maximum boundary (inclusive)
 * @return      Random value
 */
orxFLOAT orxFASTCALL orxMath_GetRandomFloat(orxFLOAT _fMin, orxFLOAT _fMax)
{
  orxFLOAT  fResult;
 
  /* Updates seed */
  ss32RandomSeed *= 48271;
 
  /* Updates result */
  fResult = orxS2F(ss32RandomSeed) / orx2F(0x80000000);
  fResult = orx2F(0.5f) * (fResult * (_fMax - _fMin) + _fMax + _fMin);
 
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
  orxU32    u32Result;
  orxFLOAT  fTemp;
 
  /* Updates seed */
  ss32RandomSeed *= 48271;
 
  /* Updates result */
  fTemp     = orxS2F(ss32RandomSeed & 0x7FFFFFFF) / orx2F(0x80000000);
  u32Result = _u32Min + orxF2U(fTemp * (orx2F(0.9f) + orxU2F(_u32Max - _u32Min)));
 
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
  orxS32    s32Result;
  orxFLOAT  fTemp;
 
  /* Updates seed */
  ss32RandomSeed *= 48271;
 
  /* Updates result */
  fTemp     = orxS2F(ss32RandomSeed & 0x7FFFFFFF) / orx2F(0x80000000);
  s32Result = _s32Min + orxF2S(fTemp * (orx2F(0.9f) + orxS2F(_s32Max - _s32Min)));

  /* Done! */
  return s32Result;
}
