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

#include <stdlib.h>


void orxFASTCALL orxMath_InitRandom(orxU32 _u32Seed)
{
  /* Inits random seed */
  srand(_u32Seed);
}

/** Gets a random orxFLOAT value
 * @param[in]   _fMin                           Minimum boundary (inclusive)
 * @param[in]   _fMax                           Maximum boundary (inclusive)
 * @return      Random value
 */
orxFLOAT orxFASTCALL orxMath_GetRandomFloat(orxFLOAT _fMin, orxFLOAT _fMax)
{
  return((orx2F(rand()) * (orx2F(1.0f / RAND_MAX)) * (_fMax - _fMin)) + _fMin);
}

/** Gets a random orxU32 value
 * @param[in]   _u32Min                         Minimum boundary (inclusive)
 * @param[in]   _u32Max                         Maximum boundary (inclusive)
 * @return      Random value
 */
orxU32 orxFASTCALL orxMath_GetRandomU32(orxU32 _u32Min, orxU32 _u32Max)
{
  orxU32 u32Rand;

  /* Gets raw random number */
  u32Rand = rand();

  /* Done! */
  return (u32Rand == RAND_MAX) ? _u32Max : (orxF2U((orx2F(u32Rand) * (orx2F(1.0f / RAND_MAX)) * (orxU2F(_u32Max) + orxFLOAT_1 - orxU2F(_u32Min))) + orxS2F(_u32Min)));
}

/** Gets a random orxS32 value
 * @param[in]   _s32Min                         Minimum boundary (inclusive)
 * @param[in]   _s32Max                         Maximum boundary (inclusive)
 * @return      Random value
 */
orxS32 orxFASTCALL orxMath_GetRandomS32(orxS32 _s32Min, orxS32 _s32Max)
{
  orxU32 u32Rand;

  /* Gets raw random number */
  u32Rand = rand();

  /* Done! */
  return (u32Rand == RAND_MAX) ? _s32Max : (orxF2S((orx2F(u32Rand) * (orx2F(1.0f / RAND_MAX)) * (orxS2F(_s32Max) + orxFLOAT_1 - orxS2F(_s32Min))) + orxS2F(_s32Min)));
}
