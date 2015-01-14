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
 * @file orxVector.h
 * @date 30/03/2005
 * @author iarwain@orx-project.org
 *
 * @todo
 */

/**
 * @addtogroup orxVector
 *
 * Vector module
 * Module that handles vectors and basic structures based on them
 *
 * @{
 */


#ifndef _orxVECTOR_H_
#define _orxVECTOR_H_

#include "orxInclude.h"

#include "debug/orxDebug.h"
#include "memory/orxMemory.h"
#include "math/orxMath.h"


/** Public structure definition
 */
typedef struct __orxVECTOR_t
{
  /** Coordinates : 12 */
  union
  {
    orxFLOAT fX;              /**< First coordinate in the cartesian space */
    orxFLOAT fRho;            /**< First coordinate in the spherical space */
    orxFLOAT fR;              /**< First coordinate in the RGB color space */
    orxFLOAT fH;              /**< First coordinate in the HSL/HSV color spaces */
  };

  union
  {
    orxFLOAT fY;              /**< Second coordinate in the cartesian space */
    orxFLOAT fTheta;          /**< Second coordinate in the spherical space */
    orxFLOAT fG;              /**< Second coordinate in the RGB color space */
    orxFLOAT fS;              /**< Second coordinate in the HSL/HSV color spaces */
  };

  union
  {
    orxFLOAT fZ;              /**< Third coordinate in the cartesian space */
    orxFLOAT fPhi;            /**< Third coordinate in the spherical space */
    orxFLOAT fB;              /**< Third coordinate in the RGB color space */
    orxFLOAT fL;              /**< Third coordinate in the HSL color space */
    orxFLOAT fV;              /**< Third coordinate in the HSV color space */
  };

} orxVECTOR;


/* *** Vector inlined functions *** */


/** Sets vector XYZ values (also work for other coordinate system)
 * @param[in]   _pvVec                        Concerned vector
 * @param[in]   _fX                           First coordinate value
 * @param[in]   _fY                           Second coordinate value
 * @param[in]   _fZ                           Third coordinate value
 * @return      Vector
 */
static orxINLINE orxVECTOR *                  orxVector_Set(orxVECTOR *_pvVec, orxFLOAT _fX, orxFLOAT _fY, orxFLOAT _fZ)
{
  /* Checks */
  orxASSERT(_pvVec != orxNULL);

  /* Stores values */
  _pvVec->fX = _fX;
  _pvVec->fY = _fY;
  _pvVec->fZ = _fZ;

  /* Done ! */
  return _pvVec;
}

/** Sets all the vector coordinates with the given value
 * @param[in]   _pvVec                        Concerned vector
 * @param[in]   _fValue                       Value to set
 * @return      Vector
 */
static orxINLINE orxVECTOR *                  orxVector_SetAll(orxVECTOR *_pvVec, orxFLOAT _fValue)
{
  /* Done ! */
  return(orxVector_Set(_pvVec, _fValue, _fValue, _fValue));
}

/** Copies a vector onto another one
 * @param[in]   _pvDst                        Vector to copy to (destination)
 * @param[in]   _pvSrc                        Vector to copy from (source)
 * @return      Destination vector
 */
static orxINLINE orxVECTOR *                  orxVector_Copy(orxVECTOR *_pvDst, const orxVECTOR *_pvSrc)
{
  /* Checks */
  orxASSERT(_pvDst != orxNULL);
  orxASSERT(_pvSrc != orxNULL);

  /* Copies it */
  orxMemory_Copy(_pvDst, _pvSrc, sizeof(orxVECTOR));

  /* Done! */
  return _pvDst;
}

/** Adds vectors and stores result in a third one
 * @param[out]  _pvRes                        Vector where to store result (can be one of the two operands)
 * @param[in]   _pvOp1                        First operand
 * @param[in]   _pvOp2                        Second operand
 * @return      Resulting vector (Op1 + Op2)
 */
static orxINLINE orxVECTOR *                  orxVector_Add(orxVECTOR *_pvRes, const orxVECTOR *_pvOp1, const orxVECTOR *_pvOp2)
{
  /* Checks */
  orxASSERT(_pvRes != orxNULL);
  orxASSERT(_pvOp1 != orxNULL);
  orxASSERT(_pvOp2 != orxNULL);

  /* Adds all */
  _pvRes->fX = _pvOp1->fX + _pvOp2->fX;
  _pvRes->fY = _pvOp1->fY + _pvOp2->fY;
  _pvRes->fZ = _pvOp1->fZ + _pvOp2->fZ;

  /* Done! */
  return _pvRes;
}

/** Substracts vectors and stores result in a third one
 * @param[out]  _pvRes                        Vector where to store result (can be one of the two operands)
 * @param[in]   _pvOp1                        First operand
 * @param[in]   _pvOp2                        Second operand
 * @return      Resulting vector (Op1 - Op2)
 */
static orxINLINE orxVECTOR *                  orxVector_Sub(orxVECTOR *_pvRes, const orxVECTOR *_pvOp1, const orxVECTOR *_pvOp2)
{
  /* Checks */
  orxASSERT(_pvRes != orxNULL);
  orxASSERT(_pvOp1 != orxNULL);
  orxASSERT(_pvOp2 != orxNULL);

  /* Adds all */
  _pvRes->fX = _pvOp1->fX - _pvOp2->fX;
  _pvRes->fY = _pvOp1->fY - _pvOp2->fY;
  _pvRes->fZ = _pvOp1->fZ - _pvOp2->fZ;

  /* Done! */
  return _pvRes;
}

/** Multiplies a vector by an orxFLOAT and stores result in another one
 * @param[out]  _pvRes                        Vector where to store result (can be the operand)
 * @param[in]   _pvOp1                        First operand
 * @param[in]   _fOp2                         Second operand
 * @return      Resulting vector
 */
static orxINLINE orxVECTOR *                  orxVector_Mulf(orxVECTOR *_pvRes, const orxVECTOR *_pvOp1, orxFLOAT _fOp2)
{
  /* Checks */
  orxASSERT(_pvRes != orxNULL);
  orxASSERT(_pvOp1 != orxNULL);

  /* Muls all */
  _pvRes->fX = _pvOp1->fX * _fOp2;
  _pvRes->fY = _pvOp1->fY * _fOp2;
  _pvRes->fZ = _pvOp1->fZ * _fOp2;

  /* Done! */
  return _pvRes;
}

/** Multiplies a vector by another vector and stores result in a third one
 * @param[out]  _pvRes                        Vector where to store result (can be one of the two operands)
 * @param[in]   _pvOp1                        First operand
 * @param[in]   _pvOp2                        Second operand
 * @return      Resulting vector (Op1 * Op2)
 */
static orxINLINE orxVECTOR *                  orxVector_Mul(orxVECTOR *_pvRes, const orxVECTOR *_pvOp1, const orxVECTOR *_pvOp2)
{
  /* Checks */
  orxASSERT(_pvRes != orxNULL);
  orxASSERT(_pvOp1 != orxNULL);
  orxASSERT(_pvOp2 != orxNULL);

  /* Muls all */
  _pvRes->fX = _pvOp1->fX * _pvOp2->fX;
  _pvRes->fY = _pvOp1->fY * _pvOp2->fY;
  _pvRes->fZ = _pvOp1->fZ * _pvOp2->fZ;

  /* Done! */
  return _pvRes;
}

/** Divides a vector by an orxFLOAT and stores result in another one
 * @param[out]  _pvRes                        Vector where to store result (can be the operand)
 * @param[in]   _pvOp1                        First operand
 * @param[in]   _fOp2                         Second operand
 * @return      Resulting vector
 */
static orxINLINE orxVECTOR *                  orxVector_Divf(orxVECTOR *_pvRes, const orxVECTOR *_pvOp1, orxFLOAT _fOp2)
{
  register orxFLOAT fRecCoef;

  /* Checks */
  orxASSERT(_pvRes != orxNULL);
  orxASSERT(_pvOp1 != orxNULL);
  orxASSERT(_fOp2 != orxFLOAT_0);

  /* Gets reciprocal coef */
  fRecCoef = orxFLOAT_1 / _fOp2;

  /* Muls all */
  _pvRes->fX = _pvOp1->fX * fRecCoef;
  _pvRes->fY = _pvOp1->fY * fRecCoef;
  _pvRes->fZ = _pvOp1->fZ * fRecCoef;

  /* Done! */
  return _pvRes;
}

/** Divides a vector by another vector and stores result in a third one
 * @param[out]  _pvRes                        Vector where to store result (can be one of the two operands)
 * @param[in]   _pvOp1                        First operand
 * @param[in]   _pvOp2                        Second operand
 * @return      Resulting vector (Op1 / Op2)
 */
static orxINLINE orxVECTOR *                  orxVector_Div(orxVECTOR *_pvRes, const orxVECTOR *_pvOp1, const orxVECTOR *_pvOp2)
{
  /* Checks */
  orxASSERT(_pvRes != orxNULL);
  orxASSERT(_pvOp1 != orxNULL);
  orxASSERT(_pvOp2 != orxNULL);

  /* Divs all */
  _pvRes->fX = _pvOp1->fX / _pvOp2->fX;
  _pvRes->fY = _pvOp1->fY / _pvOp2->fY;
  _pvRes->fZ = _pvOp1->fZ / _pvOp2->fZ;

  /* Done! */
  return _pvRes;
}

/** Lerps from one vector to another one using a coefficient
 * @param[out]  _pvRes                        Vector where to store result (can be one of the two operands)
 * @param[in]   _pvOp1                        First operand
 * @param[in]   _pvOp2                        Second operand
 * @param[in]   _fOp                          Lerp coefficient parameter
 * @return      Resulting vector
 */
static orxINLINE orxVECTOR *                  orxVector_Lerp(orxVECTOR *_pvRes, const orxVECTOR *_pvOp1, const orxVECTOR *_pvOp2, orxFLOAT _fOp)
{
  /* Checks */
  orxASSERT(_pvRes != orxNULL);
  orxASSERT(_pvOp1 != orxNULL);
  orxASSERT(_pvOp2 != orxNULL);
  orxASSERT(_fOp >= orxFLOAT_0);

  /* Lerps all*/
  _pvRes->fX = orxLERP(_pvOp1->fX, _pvOp2->fX, _fOp);
  _pvRes->fY = orxLERP(_pvOp1->fY, _pvOp2->fY, _fOp);
  _pvRes->fZ = orxLERP(_pvOp1->fZ, _pvOp2->fZ, _fOp);

  /* Done! */
  return _pvRes;
}

/** Gets minimum between two vectors
 * @param[out]  _pvRes                        Vector where to store result (can be one of the two operands)
 * @param[in]   _pvOp1                        First operand
 * @param[in]   _pvOp2                        Second operand
 * @return      Resulting vector MIN(Op1, Op2)
 */
static orxINLINE orxVECTOR *                  orxVector_Min(orxVECTOR *_pvRes, const orxVECTOR *_pvOp1, const orxVECTOR *_pvOp2)
{
  /* Checks */
  orxASSERT(_pvRes != orxNULL);
  orxASSERT(_pvOp1 != orxNULL);
  orxASSERT(_pvOp2 != orxNULL);

  /* Gets all mins */
  _pvRes->fX = orxMIN(_pvOp1->fX, _pvOp2->fX);
  _pvRes->fY = orxMIN(_pvOp1->fY, _pvOp2->fY);
  _pvRes->fZ = orxMIN(_pvOp1->fZ, _pvOp2->fZ);

  /* Done! */
  return _pvRes;
}

/** Gets maximum between two vectors
 * @param[out]  _pvRes                        Vector where to store result (can be one of the two operands)
 * @param[in]   _pvOp1                        First operand
 * @param[in]   _pvOp2                        Second operand
 * @return      Resulting vector MAX(Op1, Op2)
 */
static orxINLINE orxVECTOR *                  orxVector_Max(orxVECTOR *_pvRes, const orxVECTOR *_pvOp1, const orxVECTOR *_pvOp2)
{
  /* Checks */
  orxASSERT(_pvRes != orxNULL);
  orxASSERT(_pvOp1 != orxNULL);
  orxASSERT(_pvOp2 != orxNULL);

  /* Gets all maxs */
  _pvRes->fX = orxMAX(_pvOp1->fX, _pvOp2->fX);
  _pvRes->fY = orxMAX(_pvOp1->fY, _pvOp2->fY);
  _pvRes->fZ = orxMAX(_pvOp1->fZ, _pvOp2->fZ);

  /* Done! */
  return _pvRes;
}

/** Clamps a vector between two others
 * @param[out]  _pvRes                        Vector where to store result (can be the operand)
 * @param[in]   _pvOp                         Vector to clamp
 * @param[in]   _pvMin                        Minimum boundary
 * @param[in]   _pvMax                        Maximum boundary
 * @return      Resulting vector CLAMP(Op, MIN, MAX)
 */
static orxINLINE orxVECTOR *                  orxVector_Clamp(orxVECTOR *_pvRes, const orxVECTOR *_pvOp, const orxVECTOR *_pvMin, const orxVECTOR *_pvMax)
{
  /* Checks */
  orxASSERT(_pvRes != orxNULL);
  orxASSERT(_pvOp != orxNULL);
  orxASSERT(_pvMin != orxNULL);
  orxASSERT(_pvMax != orxNULL);

  /* Gets all clamped values */
  _pvRes->fX = orxCLAMP(_pvOp->fX, _pvMin->fX, _pvMax->fX);
  _pvRes->fY = orxCLAMP(_pvOp->fY, _pvMin->fY, _pvMax->fY);
  _pvRes->fZ = orxCLAMP(_pvOp->fZ, _pvMin->fZ, _pvMax->fZ);

  /* Done! */
  return _pvRes;
}

/** Negates a vector and stores result in another one
 * @param[out]  _pvRes                        Vector where to store result (can be the operand)
 * @param[in]   _pvOp                         Vector to negates
 * @return      Resulting vector (-Op)
 */
static orxINLINE orxVECTOR *                  orxVector_Neg(orxVECTOR *_pvRes, const orxVECTOR *_pvOp)
{
  /* Checks */
  orxASSERT(_pvRes != orxNULL);
  orxASSERT(_pvOp != orxNULL);

  /* Negates all */
  _pvRes->fX = -(_pvOp->fX);
  _pvRes->fY = -(_pvOp->fY);
  _pvRes->fZ = -(_pvOp->fZ);

  /* Done! */
  return _pvRes;
}

/** Gets reciprocal (1.0 /) vector and stores the result in another one
 * @param[out]  _pvRes                        Vector where to store result (can be the operand)
 * @param[in]   _pvOp                         Input value
 * @return      Resulting vector (1 / Op)
 */
static orxINLINE orxVECTOR *                  orxVector_Rec(orxVECTOR *_pvRes, const orxVECTOR *_pvOp)
{
  /* Checks */
  orxASSERT(_pvRes != orxNULL);
  orxASSERT(_pvOp != orxNULL);

  /* Reverts all */
  _pvRes->fX = orxFLOAT_1 / _pvOp->fX;
  _pvRes->fY = orxFLOAT_1 / _pvOp->fY;
  _pvRes->fZ = orxFLOAT_1 / _pvOp->fZ;

  /* Done! */
  return _pvRes;
}

/** Gets floored vector and stores the result in another one
 * @param[out]  _pvRes                        Vector where to store result (can be the operand)
 * @param[in]   _pvOp                         Input value
 * @return      Resulting vector Floor(Op)
 */
static orxINLINE orxVECTOR *                  orxVector_Floor(orxVECTOR *_pvRes, const orxVECTOR *_pvOp)
{
  /* Checks */
  orxASSERT(_pvRes != orxNULL);
  orxASSERT(_pvOp != orxNULL);

  /* Reverts all */
  _pvRes->fX = orxMath_Floor(_pvOp->fX);
  _pvRes->fY = orxMath_Floor(_pvOp->fY);
  _pvRes->fZ = orxMath_Floor(_pvOp->fZ);

  /* Done! */
  return _pvRes;
}

/** Gets rounded vector and stores the result in another one
 * @param[out]  _pvRes                        Vector where to store result (can be the operand)
 * @param[in]   _pvOp                         Input value
 * @return      Resulting vector Round(Op)
 */
static orxINLINE orxVECTOR *                  orxVector_Round(orxVECTOR *_pvRes, const orxVECTOR *_pvOp)
{
  /* Checks */
  orxASSERT(_pvRes != orxNULL);
  orxASSERT(_pvOp != orxNULL);

  /* Reverts all */
  _pvRes->fX = orxMath_Round(_pvOp->fX);
  _pvRes->fY = orxMath_Round(_pvOp->fY);
  _pvRes->fZ = orxMath_Round(_pvOp->fZ);

  /* Done! */
  return _pvRes;
}

/** Gets vector squared size
 * @param[in]   _pvOp                         Input vector
 * @return      Vector's squared size
 */
static orxINLINE orxFLOAT                     orxVector_GetSquareSize(const orxVECTOR *_pvOp)
{
  register orxFLOAT fResult;

  /* Checks */
  orxASSERT(_pvOp != orxNULL);

  /* Updates result */
  fResult = (_pvOp->fX * _pvOp->fX) + (_pvOp->fY * _pvOp->fY) + (_pvOp->fZ * _pvOp->fZ);

  /* Done! */
  return fResult;
}

/** Gets vector size
 * @param[in]   _pvOp                         Input vector
 * @return      Vector's size
 */
static orxINLINE orxFLOAT                     orxVector_GetSize(const orxVECTOR *_pvOp)
{
  register orxFLOAT fResult;

  /* Checks */
  orxASSERT(_pvOp != orxNULL);

  /* Updates result */
  fResult = orxMath_Sqrt((_pvOp->fX * _pvOp->fX) + (_pvOp->fY * _pvOp->fY) + (_pvOp->fZ * _pvOp->fZ));

  /* Done! */
  return fResult;
}

/** Gets squared distance between 2 positions
 * @param[in]   _pvOp1                        First position
 * @param[in]   _pvOp2                        Second position
 * @return      Squared distance
 */
static orxINLINE orxFLOAT                     orxVector_GetSquareDistance(const orxVECTOR *_pvOp1, const orxVECTOR *_pvOp2)
{
  orxVECTOR   vTemp;
  register orxFLOAT fResult;

  /* Checks */
  orxASSERT(_pvOp1 != orxNULL);
  orxASSERT(_pvOp2  != orxNULL);

  /* Gets distance vector */
  orxVector_Sub(&vTemp, _pvOp2, _pvOp1);

  /* Updates result */
  fResult = orxVector_GetSquareSize(&vTemp);

  /* Done! */
  return fResult;
}

/** Gets distance between 2 positions
 * @param[in]   _pvOp1                        First position
 * @param[in]   _pvOp2                        Second position
 * @return      Distance
 */
static orxINLINE orxFLOAT                     orxVector_GetDistance(const orxVECTOR *_pvOp1, const orxVECTOR *_pvOp2)
{
  orxVECTOR   vTemp;
  register orxFLOAT fResult;

  /* Checks */
  orxASSERT(_pvOp1 != orxNULL);
  orxASSERT(_pvOp2  != orxNULL);

  /* Gets distance vector */
  orxVector_Sub(&vTemp, _pvOp2, _pvOp1);

  /* Updates result */
  fResult = orxVector_GetSize(&vTemp);

  /* Done! */
  return fResult;
}

/** Normalizes a vector
 * @param[out]  _pvRes                        Vector where to store result (can be the operand)
 * @param[in]   _pvOp                         Vector to normalize
 * @return      Normalized vector
 */
static orxINLINE orxVECTOR *                  orxVector_Normalize(orxVECTOR *_pvRes, const orxVECTOR *_pvOp)
{
  register orxFLOAT fOp;

  /* Checks */
  orxASSERT(_pvRes != orxNULL);
  orxASSERT(_pvOp != orxNULL);

  /* Gets squared size */
  fOp = (_pvOp->fX * _pvOp->fX) + (_pvOp->fY * _pvOp->fY) + (_pvOp->fZ * _pvOp->fZ);

  /* Gets reciprocal size */
  fOp = orxFLOAT_1 / (orxMATH_KF_TINY_EPSILON + orxMath_Sqrt(fOp));

  /* Updates result */
  _pvRes->fX = fOp * _pvOp->fX;
  _pvRes->fY = fOp * _pvOp->fY;
  _pvRes->fZ = fOp * _pvOp->fZ;

  /* Done! */
  return _pvRes;
}

/** Rotates a 2D vector (along Z-axis)
 * @param[out]  _pvRes                        Vector where to store result (can be the operand)
 * @param[in]   _pvOp                         Vector to rotate
 * @param[in]   _fAngle                       Angle of rotation (radians)
 * @return      Rotated vector
 */
static orxINLINE orxVECTOR *                  orxVector_2DRotate(orxVECTOR *_pvRes, const orxVECTOR *_pvOp, orxFLOAT _fAngle)
{
  /* Checks */
  orxASSERT(_pvRes != orxNULL);
  orxASSERT(_pvOp != orxNULL);

  /* PI/2? */
  if(_fAngle == orxMATH_KF_PI_BY_2)
  {
    /* Updates result */
    orxVector_Set(_pvRes, -_pvOp->fY, _pvOp->fX, _pvOp->fZ);
  }
  /* -PI/2? */
  else if(_fAngle == -orxMATH_KF_PI_BY_2)
  {
    /* Updates result */
    orxVector_Set(_pvRes, _pvOp->fY, -_pvOp->fX, _pvOp->fZ);
  }
  /* Any other angle */
  else
  {
    register orxFLOAT fSin, fCos;

    /* Gets cos & sin of angle */
    fCos = orxMath_Cos(_fAngle);
    fSin = orxMath_Sin(_fAngle);

    /* Updates result */
    orxVector_Set(_pvRes, (fCos * _pvOp->fX) - (fSin * _pvOp->fY), (fSin * _pvOp->fX) + (fCos * _pvOp->fY), _pvOp->fZ);
  }

  /* Done! */
  return _pvRes;
}

/** Is vector null?
 * @param[in]   _pvOp                         Vector to test
 * @return      orxTRUE if vector's null, orxFALSE otherwise
 */
static orxINLINE orxBOOL                      orxVector_IsNull(const orxVECTOR *_pvOp)
{
  orxBOOL bResult;

  /* Checks */
  orxASSERT(_pvOp != orxNULL);

  /* Updates result */
  bResult = ((_pvOp->fX == orxFLOAT_0) && (_pvOp->fY == orxFLOAT_0) && (_pvOp->fZ == orxFLOAT_0)) ? orxTRUE : orxFALSE;

  /* Done! */
  return bResult;
}

/** Are vectors equal?
 * @param[in]   _pvOp1                        First vector to compare
 * @param[in]   _pvOp2                        Second vector to compare
 * @return      orxTRUE if both vectors are equal, orxFALSE otherwise
 */
static orxINLINE orxBOOL                      orxVector_AreEqual(const orxVECTOR *_pvOp1, const orxVECTOR *_pvOp2)
{
  orxBOOL bResult;

  /* Checks */
  orxASSERT(_pvOp1  != orxNULL);
  orxASSERT(_pvOp2  != orxNULL);

  /* Updates result */
  bResult = ((_pvOp1->fX == _pvOp2->fX) && (_pvOp1->fY == _pvOp2->fY) && (_pvOp1->fZ == _pvOp2->fZ)) ? orxTRUE : orxFALSE;

  /* Done! */
  return bResult;
}

/** Transforms a cartesian vector into a spherical one
 * @param[out]  _pvRes                        Vector where to store result (can be the operand)
 * @param[in]   _pvOp                         Vector to transform
 * @return      Transformed vector
 */
static orxINLINE orxVECTOR *                  orxVector_FromCartesianToSpherical(orxVECTOR *_pvRes, const orxVECTOR *_pvOp)
{
  /* Checks */
  orxASSERT(_pvRes != orxNULL);
  orxASSERT(_pvOp != orxNULL);

  /* Is operand vector null? */
  if((_pvOp->fX == orxFLOAT_0)
  && (_pvOp->fY == orxFLOAT_0)
  && (_pvOp->fZ == orxFLOAT_0))
  {
    /* Updates result vector */
    _pvRes->fRho = _pvRes->fTheta = _pvRes->fPhi = orxFLOAT_0;
  }
  else
  {
    orxFLOAT fRho, fTheta, fPhi;

    /* Z = 0? */
    if(_pvOp->fZ == orxFLOAT_0)
    {
      /* X = 0? */
      if(_pvOp->fX == orxFLOAT_0)
      {
        /* Gets absolute value */
        fRho = orxMath_Abs(_pvOp->fY);
      }
      /* X != 0 and Y = 0? */
      else if(_pvOp->fY == orxFLOAT_0)
      {
        /* Gets absolute value */
        fRho = orxMath_Abs(_pvOp->fX);
      }
      /* X != 0 and Y != 0 */
      else
      {
        /* Computes rho */
        fRho = orxMath_Sqrt((_pvOp->fX * _pvOp->fX) + (_pvOp->fY * _pvOp->fY));
      }

      /* Sets phi */
      fPhi = orxMATH_KF_PI_BY_2;
    }
    else
    {
      /* X = 0 and Y = 0? */
      if((_pvOp->fX == orxFLOAT_0) && (_pvOp->fY == orxFLOAT_0))
      {
        /* Z < 0? */
        if(_pvOp->fZ < orxFLOAT_0)
        {
          /* Gets absolute value */
          fRho = orxMath_Abs(_pvOp->fZ);

          /* Sets phi */
          fPhi = orxMATH_KF_PI;
        }
        else
        {
          /* Sets rho */
          fRho = _pvOp->fZ;

          /* Sets phi */
          fPhi = orxFLOAT_0;
        }
      }
      else
      {
        /* Computes rho */
        fRho = orxMath_Sqrt(orxVector_GetSquareSize(_pvOp));

        /* Computes phi */
        fPhi = orxMath_ACos(_pvOp->fZ / fRho);
      }
    }

    /* Computes theta */
    fTheta = orxMath_ATan(_pvOp->fY, _pvOp->fX);

    /* Updates result */
    _pvRes->fRho    = fRho;
    _pvRes->fTheta  = fTheta;
    _pvRes->fPhi    = fPhi;
  }

  /* Done! */
  return _pvRes;
}

/** Transforms a spherical vector into a cartesian one
 * @param[out]  _pvRes                        Vector where to store result (can be the operand)
 * @param[in]   _pvOp                         Vector to transform
 * @return      Transformed vector
 */
static orxINLINE orxVECTOR *                  orxVector_FromSphericalToCartesian(orxVECTOR *_pvRes, const orxVECTOR *_pvOp)
{
  orxFLOAT fSinPhi, fCosPhi, fSinTheta, fCosTheta, fRho;

  /* Checks */
  orxASSERT(_pvRes != orxNULL);
  orxASSERT(_pvOp != orxNULL);

  /* Stores rho */
  fRho = _pvOp->fRho;

  /* Gets sine & cosine */
  fSinTheta = orxMath_Sin(_pvOp->fTheta);
  fCosTheta = orxMath_Cos(_pvOp->fTheta);
  fSinPhi   = orxMath_Sin(_pvOp->fPhi);
  fCosPhi   = orxMath_Cos(_pvOp->fPhi);
  if(orxMath_Abs(fSinTheta) < orxMATH_KF_EPSILON)
  {
    fSinTheta = orxFLOAT_0;
  }
  if(orxMath_Abs(fCosTheta) < orxMATH_KF_EPSILON)
  {
    fCosTheta = orxFLOAT_0;
  }
  if(orxMath_Abs(fSinPhi) < orxMATH_KF_EPSILON)
  {
    fSinPhi = orxFLOAT_0;
  }
  if(orxMath_Abs(fCosPhi) < orxMATH_KF_EPSILON)
  {
    fCosPhi = orxFLOAT_0;
  }

  /* Updates result */
  _pvRes->fX = fRho * fCosTheta * fSinPhi;
  _pvRes->fY = fRho * fSinTheta * fSinPhi;
  _pvRes->fZ = fRho * fCosPhi;

  /* Done! */
  return _pvRes;
}

/** Gets dot product of two vectors
 * @param[in]   _pvOp1                      First operand
 * @param[in]   _pvOp2                      Second operand
 * @return      Dot product
 */
static orxINLINE orxFLOAT                   orxVector_Dot(const orxVECTOR *_pvOp1, const orxVECTOR *_pvOp2)
{
  orxFLOAT fResult;

  /* Checks */
  orxASSERT(_pvOp1 != orxNULL);
  orxASSERT(_pvOp2  != orxNULL);

  /* Updates result */
  fResult = (_pvOp1->fX * _pvOp2->fX) + (_pvOp1->fY * _pvOp2->fY) + (_pvOp1->fZ * _pvOp2->fZ);

  /* Done! */
  return fResult;
}

/** Gets 2D dot product of two vectors
 * @param[in]   _pvOp1                      First operand
 * @param[in]   _pvOp2                      Second operand
 * @return      2D dot product
 */
static orxINLINE orxFLOAT                   orxVector_2DDot(const orxVECTOR *_pvOp1, const orxVECTOR *_pvOp2)
{
  orxFLOAT fResult;

  /* Checks */
  orxASSERT(_pvOp1 != orxNULL);
  orxASSERT(_pvOp2  != orxNULL);

  /* Updates result */
  fResult = (_pvOp1->fX * _pvOp2->fX) + (_pvOp1->fY * _pvOp2->fY);

  /* Done! */
  return fResult;
}

/** Gets cross product of two vectors
 * @param[out]  _pvRes                       Vector where to store result
 * @param[in]   _pvOp1                      First operand
 * @param[in]   _pvOp2                      Second operand
 * @return      Cross product orxVECTOR / orxNULL
 */
static orxINLINE orxVECTOR *                orxVector_Cross(orxVECTOR *_pvRes, const orxVECTOR *_pvOp1, const orxVECTOR *_pvOp2)
{
  orxFLOAT fTemp1, fTemp2;

  /* Checks */
  orxASSERT(_pvRes != orxNULL);
  orxASSERT(_pvOp1 != orxNULL);
  orxASSERT(_pvOp2  != orxNULL);

  /* Computes cross product */
  fTemp1      = (_pvOp1->fY * _pvOp2->fZ) - (_pvOp1->fZ * _pvOp2->fY);
  fTemp2      = (_pvOp1->fZ * _pvOp2->fX) - (_pvOp1->fX * _pvOp2->fZ);
  _pvRes->fZ  = (_pvOp1->fX * _pvOp2->fY) - (_pvOp1->fY * _pvOp2->fX);
  _pvRes->fY  = fTemp2;
  _pvRes->fX  = fTemp1;

  /* Done! */
  return _pvRes;
}


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
extern orxDLLAPI orxVECTOR *orxFASTCALL orxVector_Bezier(orxVECTOR *_pvRes, const orxVECTOR *_pvPoint1, const orxVECTOR *_pvPoint2, const orxVECTOR *_pvPoint3, const orxVECTOR *_pvPoint4, orxFLOAT _fT);

/** Computes an interpolated point on a Catmull-Rom curve segment for a given parameter
 * @param[out]  _pvRes                      Vector where to store result
 * @param[in]   _pvPoint1                   First control point for this curve segment
 * @param[in]   _pvPoint2                   Second control point for this curve segment
 * @param[in]   _pvPoint3                   Third control point for this curve segment
 * @param[in]   _pvPoint4                   Fourth control point for this curve segment
 * @param[in]   _fT                         Interpolation parameter in [0.0, 1.0]
 * @return      Interpolated point on the Catmull-Rom curve segment
 */
extern orxDLLAPI orxVECTOR *orxFASTCALL orxVector_CatmullRom(orxVECTOR *_pvRes, const orxVECTOR *_pvPoint1, const orxVECTOR *_pvPoint2, const orxVECTOR *_pvPoint3, const orxVECTOR *_pvPoint4, orxFLOAT _fT);


/* *** Vector constants *** */

extern orxDLLAPI const orxVECTOR orxVECTOR_X;      /**< X-Axis unit vector */
extern orxDLLAPI const orxVECTOR orxVECTOR_Y;      /**< Y-Axis unit vector */
extern orxDLLAPI const orxVECTOR orxVECTOR_Z;      /**< Z-Axis unit vector */

extern orxDLLAPI const orxVECTOR orxVECTOR_0;      /**< Null vector */
extern orxDLLAPI const orxVECTOR orxVECTOR_1;      /**< Vector filled with 1s */

extern orxDLLAPI const orxVECTOR orxVECTOR_RED;    /**< Red color vector */
extern orxDLLAPI const orxVECTOR orxVECTOR_GREEN;  /**< Green color vector */
extern orxDLLAPI const orxVECTOR orxVECTOR_BLUE;   /**< Blue color vector */
extern orxDLLAPI const orxVECTOR orxVECTOR_BLACK;  /**< Black color vector */
extern orxDLLAPI const orxVECTOR orxVECTOR_WHITE;  /**< White color vector */

#endif /* _orxVECTOR_H_ */

/** @} */
