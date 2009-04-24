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
 * @file orxMath.c
 * @date 27/11/2008
 * @author iarwain@orx-project.org
 *
 */


#include "math/orxMath.h"


void orxFASTCALL orxMath_InitRandom(orxU32 _u32Seed)
{
  /* Inits random seed */
  srand(_u32Seed);
}

/** Gets a random orxFLOAT value
 * @param[in]   _fMin                           Minimum boundary
 * @param[in]   _fMax                           Maximum boundary
 * @return      Random value
 */
orxFLOAT orxFASTCALL orxMath_GetRandomFloat(orxFLOAT _fMin, orxFLOAT _fMax)
{
  return((orx2F(rand()) * (orx2F(1.0f / RAND_MAX)) * (_fMax - _fMin)) + _fMin);
}

/** Gets a random orxU32 value
 * @param[in]   _u32Min                         Minimum boundary
 * @param[in]   _u32Max                         Maximum boundary
 * @return      Random value
 */
orxU32 orxFASTCALL orxMath_GetRandomU32(orxU32 _u32Min, orxU32 _u32Max)
{
  orxU32 u32Rand;

  /* Gets raw random number */
  u32Rand = rand();

  /* Done! */
  return (u32Rand == RAND_MAX) ? _u32Max : (orxF2U((orx2F(u32Rand) * (orx2F(1.0f / RAND_MAX)) * (orxU2F(_u32Max) + 1 - orxU2F(_u32Min))) + orxS2F(_u32Min)));
}

/** Gets a random orxS32 value
 * @param[in]   _s32Min                         Minimum boundary
 * @param[in]   _s32Max                         Maximum boundary
 * @return      Random value
 */
orxS32 orxFASTCALL orxMath_GetRandomS32(orxS32 _s32Min, orxS32 _s32Max)
{
  orxU32 u32Rand;

  /* Gets raw random number */
  u32Rand = rand();

  /* Done! */
  return (u32Rand == RAND_MAX) ? _s32Max : (orxF2S((orx2F(u32Rand) * (orx2F(1.0f / RAND_MAX)) * (orxS2F(_s32Max) + 1 - orxS2F(_s32Min))) + orxS2F(_s32Min)));
}
