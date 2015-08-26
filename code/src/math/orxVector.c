/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2015 Orx-Project
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
 * @file orxVector.c
 * @date 27/04/2005
 * @author iarwain@orx-project.org
 *
 */


#include "math/orxVector.h"


/* *** Vector functions *** */

/** Computes an interpolated point on a cubic Bezier curve segment for a given parameter
 * @param[out]  _pvRes                      Vector where to store result
 * @param[in]   _pvPoint1                   First point for this curve segment
 * @param[in]   _pvPoint2                   First control point for this curve segment
 * @param[in]   _pvPoint3                   Second control point for this curve segment
 * @param[in]   _pvPoint4                   Last point for this curve segment
 * @param[in]   _fT                         Interpolation parameter in [0.0, 1.0]
 * @return      Interpolated point on the cubic Bezier curve segment
 */
orxVECTOR *orxFASTCALL orxVector_Bezier(orxVECTOR *_pvRes, const orxVECTOR *_pvPoint1, const orxVECTOR *_pvPoint2, const orxVECTOR *_pvPoint3, const orxVECTOR *_pvPoint4, orxFLOAT _fT)
{
  orxFLOAT fT2, fT3, f1MT, f1MT2, f1MT3, fW1, fW2, fW3, fW4;

  /* Checks */
  orxASSERT(_pvRes != orxNULL);
  orxASSERT(_pvPoint1 != orxNULL);
  orxASSERT(_pvPoint2 != orxNULL);
  orxASSERT(_pvPoint3 != orxNULL);
  orxASSERT(_pvPoint4 != orxNULL);
  orxASSERT((_fT >= orxFLOAT_0) && (_fT <= orxFLOAT_1));

  /* Computes weights */
  fT2   = _fT * _fT;
  fT3   = fT2 * _fT;
  f1MT  = orxFLOAT_1 - _fT;
  f1MT2 = f1MT * f1MT;
  f1MT3 = f1MT2 * f1MT;
  fW1   = f1MT3;
  fW2   = orx2F(3.0f) * f1MT2 * _fT;
  fW3   = orx2F(3.0f) * f1MT * fT2;
  fW4   = fT3;

  /* Gets intermediate results */
  orxVector_Set(_pvRes,
                fW1 * _pvPoint1->fX + fW2 * _pvPoint2->fX + fW3 * _pvPoint3->fX + fW4 * _pvPoint4->fX,
                fW1 * _pvPoint1->fY + fW2 * _pvPoint2->fY + fW3 * _pvPoint3->fY + fW4 * _pvPoint4->fY,
                fW1 * _pvPoint1->fZ + fW2 * _pvPoint2->fZ + fW3 * _pvPoint3->fZ + fW4 * _pvPoint4->fZ);

  /* Done! */
  return _pvRes;
}

/** Computes an interpolated point on a Catmull-Rom curve segment for a given parameter
 * @param[out]  _pvRes                      Vector where to store result
 * @param[in]   _pvPoint1                   First control point for this curve segment
 * @param[in]   _pvPoint2                   Second control point for this curve segment
 * @param[in]   _pvPoint3                   Third control point for this curve segment
 * @param[in]   _pvPoint4                   Fourth control point for this curve segment
 * @param[in]   _fT                         Interpolation parameter in [0.0, 1.0]
 * @return      Interpolated point on the Catmull-Rom curve segment
 */
orxVECTOR *orxFASTCALL orxVector_CatmullRom(orxVECTOR *_pvRes, const orxVECTOR *_pvPoint1, const orxVECTOR *_pvPoint2, const orxVECTOR *_pvPoint3, const orxVECTOR *_pvPoint4, orxFLOAT _fT)
{
  orxFLOAT fT2, fT3, fW1, fW2, fW3, fW4;

  /* Checks */
  orxASSERT(_pvRes != orxNULL);
  orxASSERT(_pvPoint1 != orxNULL);
  orxASSERT(_pvPoint2 != orxNULL);
  orxASSERT(_pvPoint3 != orxNULL);
  orxASSERT(_pvPoint4 != orxNULL);
  orxASSERT((_fT >= orxFLOAT_0) && (_fT <= orxFLOAT_1));

  /* Computes weights */
  fT2 = _fT * _fT;
  fT3 = fT2 * _fT;
  fW1 = orx2F(2.0f) * fT2 - fT3 - _fT;
  fW2 = orx2F(3.0f) * fT3 - orx2F(5.0f) * fT2 + orx2F(2.0f);
  fW3 = orx2F(4.0f) * fT2 - orx2F(3.0f) * fT3 + _fT;
  fW4 = fT3 - fT2;

  /* Gets intermediate results */
  orxVector_Set(_pvRes,
                orx2F(0.5f) * (fW1 * _pvPoint1->fX + fW2 * _pvPoint2->fX + fW3 * _pvPoint3->fX + fW4 * _pvPoint4->fX),
                orx2F(0.5f) * (fW1 * _pvPoint1->fY + fW2 * _pvPoint2->fY + fW3 * _pvPoint3->fY + fW4 * _pvPoint4->fY),
                orx2F(0.5f) * (fW1 * _pvPoint1->fZ + fW2 * _pvPoint2->fZ + fW3 * _pvPoint3->fZ + fW4 * _pvPoint4->fZ));

  /* Done! */
  return _pvRes;
}


/* *** Vector constants *** */

const orxVECTOR orxVECTOR_X       = {{orx2F(1.0f)}, {orx2F(0.0f)}, {orx2F(0.0f)}};
const orxVECTOR orxVECTOR_Y       = {{orx2F(0.0f)}, {orx2F(1.0f)}, {orx2F(0.0f)}};
const orxVECTOR orxVECTOR_Z       = {{orx2F(0.0f)}, {orx2F(0.0f)}, {orx2F(1.0f)}};

const orxVECTOR orxVECTOR_0       = {{orx2F(0.0f)}, {orx2F(0.0f)}, {orx2F(0.0f)}};
const orxVECTOR orxVECTOR_1       = {{orx2F(1.0f)}, {orx2F(1.0f)}, {orx2F(1.0f)}};

const orxVECTOR orxVECTOR_RED     = {{orx2F(1.0f)}, {orx2F(0.0f)}, {orx2F(0.0f)}};
const orxVECTOR orxVECTOR_GREEN   = {{orx2F(0.0f)}, {orx2F(1.0f)}, {orx2F(0.0f)}};
const orxVECTOR orxVECTOR_BLUE    = {{orx2F(0.0f)}, {orx2F(0.0f)}, {orx2F(1.0f)}};

const orxVECTOR orxVECTOR_YELLOW  = {{orx2F(1.0f)}, {orx2F(1.0f)}, {orx2F(0.0f)}};
const orxVECTOR orxVECTOR_CYAN    = {{orx2F(0.0f)}, {orx2F(1.0f)}, {orx2F(1.0f)}};
const orxVECTOR orxVECTOR_MAGENTA = {{orx2F(1.0f)}, {orx2F(0.0f)}, {orx2F(1.0f)}};

const orxVECTOR orxVECTOR_BLACK   = {{orx2F(0.0f)}, {orx2F(0.0f)}, {orx2F(0.0f)}};
const orxVECTOR orxVECTOR_WHITE   = {{orx2F(1.0f)}, {orx2F(1.0f)}, {orx2F(1.0f)}};
