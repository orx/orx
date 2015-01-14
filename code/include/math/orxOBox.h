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
 * @file orxOBox.h
 * @date 03/10/2008
 * @author iarwain@orx-project.org
 *
 * @todo
 */

/**
 * @addtogroup orxOBox
 * 
 * Oriented box module
 * Module that handles oriented boxes
 *
 * @{
 */


#ifndef _orxOBOX_H_
#define _orxOBOX_H_

#include "orxInclude.h"

#include "math/orxVector.h"


/** Public oriented box structure
*/
typedef struct __orxOBOX_t
{
    orxVECTOR vPosition;/**< Position vector  : 12 */
    orxVECTOR vPivot;   /**< Pivot vector     : 24 */
    orxVECTOR vX;       /**< X axis vector    : 36 */
    orxVECTOR vY;       /**< Y axis vector    : 48 */
    orxVECTOR vZ;       /**< Z axis vector    : 60 */

} orxOBOX;


/* *** OBox inlined functions *** */


/** Sets 2D oriented box values
 * @param[out]  _pstRes                       OBox to set
 * @param[in]   _pvWorldPosition              World space position vector
 * @param[in]   _pvPivot                      Pivot vector
 * @param[in]   _pvSize                       Size vector
 * @param[in]   _fAngle                       Z-axis angle (radians)
 * @return      orxOBOX / orxNULL
 */
static orxINLINE orxOBOX *                    orxOBox_2DSet(orxOBOX *_pstRes, const orxVECTOR *_pvWorldPosition, const orxVECTOR *_pvPivot, const orxVECTOR *_pvSize, orxFLOAT _fAngle)
{
  orxFLOAT fCos, fSin;

  /* Checks */
  orxASSERT(_pstRes != orxNULL);
  orxASSERT(_pvWorldPosition != orxNULL);
  orxASSERT(_pvPivot != orxNULL);

  /* Gets cosine and sine */
  if(_fAngle == orxFLOAT_0)
  {
    fCos = orxFLOAT_1;
    fSin = orxFLOAT_0;
  }
  else
  {
    fCos = orxMath_Cos(_fAngle);
    fSin = orxMath_Sin(_fAngle);
  }

  /* Sets axis */
  orxVector_Set(&(_pstRes->vX), fCos * _pvSize->fX, fSin * _pvSize->fX, orxFLOAT_0);
  orxVector_Set(&(_pstRes->vY), -fSin * _pvSize->fY, fCos * _pvSize->fY, orxFLOAT_0);
  orxVector_Set(&(_pstRes->vZ), orxFLOAT_0, orxFLOAT_0, _pvSize->fZ);

  /* Sets pivot */
  orxVector_Set(&(_pstRes->vPivot), (fCos * _pvPivot->fX) - (fSin * _pvPivot->fY), (fSin * _pvPivot->fX) + (fCos * _pvPivot->fY), _pvPivot->fZ);

  /* Sets box position */
  orxVector_Copy(&(_pstRes->vPosition), _pvWorldPosition);

  /* Done! */
  return _pstRes;
}

/** Copies an OBox onto another one
 * @param[out]  _pstDst                       OBox to copy to (destination)
 * @param[in]   _pstSrc                       OBox to copy from (destination)
 * @return      Destination OBox
 */
static orxINLINE orxOBOX *                    orxOBox_Copy(orxOBOX *_pstDst, const orxOBOX *_pstSrc)
{
  /* Checks */
  orxASSERT(_pstDst != orxNULL);
  orxASSERT(_pstSrc != orxNULL);

  /* Copies it */
  orxMemory_Copy(_pstDst, _pstSrc, sizeof(orxOBOX));

  /* Done! */
  return _pstDst;
}

/** Gets OBox center position
 * @param[in]   _pstOp                        Concerned OBox
 * @param[out]  _pvRes                        Center position
 * @return      Center position vector
 */
static orxINLINE orxVECTOR *                  orxOBox_GetCenter(const orxOBOX *_pstOp, orxVECTOR *_pvRes)
{
  /* Checks */
  orxASSERT(_pstOp != orxNULL);
  orxASSERT(_pvRes != orxNULL);

  /* Gets box center */
  orxVector_Add(_pvRes, orxVector_Add(_pvRes, &(_pstOp->vX), &(_pstOp->vY)), &(_pstOp->vZ));
  orxVector_Mulf(_pvRes, _pvRes, orx2F(0.5f));
  orxVector_Sub(_pvRes, orxVector_Add(_pvRes, _pvRes, &(_pstOp->vPosition)), &(_pstOp->vPivot));

  /* Done! */
  return _pvRes;
}

/** Moves an OBox
 * @param[out]  _pstRes                       OBox where to store result
 * @param[in]   _pstOp                        OBox to move
 * @param[in]   _pvMove                       Move vector
 * @return      Moved OBox
 */
static orxINLINE orxOBOX *                    orxOBox_Move(orxOBOX *_pstRes, const orxOBOX *_pstOp, const orxVECTOR *_pvMove)
{
  /* Checks */
  orxASSERT(_pstRes != orxNULL);
  orxASSERT(_pstOp != orxNULL);
  orxASSERT(_pvMove != orxNULL);

  /* Updates result */
  orxVector_Add(&(_pstRes->vPosition), &(_pstOp->vPosition), _pvMove);

  /* Done! */
  return _pstRes;
}

/** Rotates in 2D an OBox
 * @param[out]  _pstRes                       OBox where to store result
 * @param[in]   _pstOp                        OBox to rotate (its Z-axis vector will be unchanged)
 * @param[in]   _fAngle                       Z-axis rotation angle (radians)
 * @return      Rotated OBox
 */
static orxINLINE orxOBOX *                    orxOBox_2DRotate(orxOBOX *_pstRes, const orxOBOX *_pstOp, orxFLOAT _fAngle)
{
  register orxFLOAT fSin, fCos;

  /* Checks */
  orxASSERT(_pstRes != orxNULL);
  orxASSERT(_pstOp != orxNULL);

  /* Gets cos & sin of angle */
  if(_fAngle == orxFLOAT_0)
  {
    fCos = orxFLOAT_1;
    fSin = orxFLOAT_0;
  }
  else
  {
    fCos = orxMath_Cos(_fAngle);
    fSin = orxMath_Sin(_fAngle);
  }

  /* Updates axis */
  orxVector_Set(&(_pstRes->vX), (fCos * _pstOp->vX.fX) - (fSin * _pstOp->vX.fY), (fSin * _pstOp->vX.fX) + (fCos * _pstOp->vX.fY), _pstOp->vX.fZ);
  orxVector_Set(&(_pstRes->vY), (fCos * _pstOp->vY.fX) - (fSin * _pstOp->vY.fY), (fSin * _pstOp->vY.fX) + (fCos * _pstOp->vY.fY), _pstOp->vY.fZ);

  /* Updates pivot */
  orxVector_Set(&(_pstRes->vPivot), (fCos * _pstOp->vPivot.fX) - (fSin * _pstOp->vPivot.fY), (fSin * _pstOp->vPivot.fX) + (fCos * _pstOp->vPivot.fY), _pstOp->vPivot.fZ);

  /* Done! */
  return _pstRes;
}

/** Is position inside oriented box test
 * @param[in]   _pstBox                       Box to test against position
 * @param[in]   _pvPosition                   Position to test against the box
 * @return      orxTRUE if position is inside the box, orxFALSE otherwise
 */
static orxINLINE orxBOOL                      orxOBox_IsInside(const orxOBOX *_pstBox, const orxVECTOR *_pvPosition)
{
  register orxBOOL bResult = orxFALSE;
  orxFLOAT            fProj;
  orxVECTOR           vToPos;

  /* Checks */
  orxASSERT(_pstBox != orxNULL);
  orxASSERT(_pvPosition != orxNULL);

  /* Gets origin to position vector */
  orxVector_Sub(&vToPos, _pvPosition, orxVector_Sub(&vToPos, &(_pstBox->vPosition), &(_pstBox->vPivot)));

  /* Z-axis test */
  if(((fProj = orxVector_Dot(&vToPos, &(_pstBox->vZ))) >= orxFLOAT_0)
  && (fProj <= orxVector_GetSquareSize(&(_pstBox->vZ))))
  {
    /* X-axis test */
    if(((fProj = orxVector_Dot(&vToPos, &(_pstBox->vX))) >= orxFLOAT_0)
    && (fProj <= orxVector_GetSquareSize(&(_pstBox->vX))))
    {
      /* Y-axis test */
      if(((fProj = orxVector_Dot(&vToPos, &(_pstBox->vY))) >= orxFLOAT_0)
      && (fProj <= orxVector_GetSquareSize(&(_pstBox->vY))))
      {
        /* Updates result */
        bResult = orxTRUE;
      }
    }
  }

  /* Done! */
  return bResult;
}

/** Is 2D position inside oriented box test
 * @param[in]   _pstBox                       Box to test against position
 * @param[in]   _pvPosition                   Position to test against the box (no Z-test)
 * @return      orxTRUE if position is inside the box, orxFALSE otherwise
 */
static orxINLINE orxBOOL                      orxOBox_2DIsInside(const orxOBOX *_pstBox, const orxVECTOR *_pvPosition)
{
  register orxBOOL bResult = orxFALSE;
  orxFLOAT            fProj;
  orxVECTOR           vToPos;

  /* Checks */
  orxASSERT(_pstBox != orxNULL);
  orxASSERT(_pvPosition != orxNULL);

  /* Gets origin to position vector */
  orxVector_Sub(&vToPos, _pvPosition, orxVector_Sub(&vToPos, &(_pstBox->vPosition), &(_pstBox->vPivot)));

  /* X-axis test */
  if(((fProj = orxVector_Dot(&vToPos, &(_pstBox->vX))) >= orxFLOAT_0)
  && (fProj <= orxVector_GetSquareSize(&(_pstBox->vX))))
  {
    /* Y-axis test */
    if(((fProj = orxVector_Dot(&vToPos, &(_pstBox->vY))) >= orxFLOAT_0)
    && (fProj <= orxVector_GetSquareSize(&(_pstBox->vY))))
    {
      /* Updates result */
      bResult = orxTRUE;
    }
  }

  /* Done! */
  return bResult;
}

/** Tests oriented box intersection (simple Z-axis test, to use with Z-axis aligned orxOBOX)
 * @param[in]   _pstBox1                      First box operand
 * @param[in]   _pstBox2                      Second box operand
 * @return      orxTRUE if boxes intersect, orxFALSE otherwise
 */
static orxINLINE orxBOOL                      orxOBox_ZAlignedTestIntersection(const orxOBOX *_pstBox1, const orxOBOX *_pstBox2)
{
  register orxBOOL bResult;

  /* Checks */
  orxASSERT(_pstBox1 != orxNULL);
  orxASSERT(_pstBox2 != orxNULL);
  orxASSERT(_pstBox1->vZ.fX == orxFLOAT_0);
  orxASSERT(_pstBox1->vZ.fY == orxFLOAT_0);
  orxASSERT(_pstBox1->vZ.fZ >= orxFLOAT_0);

  /* Z intersected? */
  if((_pstBox2->vPosition.fZ + _pstBox2->vZ.fZ >= _pstBox1->vPosition.fZ)
  && (_pstBox2->vPosition.fZ <= _pstBox1->vPosition.fZ + _pstBox1->vZ.fZ))
  {
    orxU32          i;
    orxVECTOR       vOrigin1, vOrigin2, *pvOrigin1 = &vOrigin1, *pvOrigin2 = &vOrigin2, *pvTemp;
    const orxOBOX  *pstBox1 = _pstBox1, *pstBox2 = _pstBox2, *pstTemp;

    /* Computes boxes origins */
    vOrigin1.fX = _pstBox1->vPosition.fX - pstBox1->vPivot.fX;
    vOrigin1.fY = _pstBox1->vPosition.fY - pstBox1->vPivot.fY;
    vOrigin2.fX = _pstBox2->vPosition.fX - pstBox2->vPivot.fX;
    vOrigin2.fY = _pstBox2->vPosition.fY - pstBox2->vPivot.fY;

    /* Test each box against the other */
    for(i = 2, bResult = orxTRUE;
        i != 0;
        i--, pstTemp = pstBox1, pstBox1 = pstBox2, pstBox2 = pstTemp, pvTemp = pvOrigin1, pvOrigin1 = pvOrigin2, pvOrigin2 = pvTemp)
    {
      orxVECTOR           vToCorner[4];
      const orxVECTOR *pvAxis;
      orxU32              j;

      /* Gets to-corner vectors */
      vToCorner[0].fX = pvOrigin2->fX - pvOrigin1->fX;
      vToCorner[0].fY = pvOrigin2->fY - pvOrigin1->fY;
      vToCorner[1].fX = vToCorner[0].fX + pstBox2->vX.fX;
      vToCorner[1].fY = vToCorner[0].fY + pstBox2->vX.fY;
      vToCorner[2].fX = vToCorner[1].fX + pstBox2->vY.fX;
      vToCorner[2].fY = vToCorner[1].fY + pstBox2->vY.fY;
      vToCorner[3].fX = vToCorner[0].fX + pstBox2->vY.fX;
      vToCorner[3].fY = vToCorner[0].fY + pstBox2->vY.fY;

      /* For both axis */
      for(j = 2, pvAxis = &(pstBox1->vX);
          j != 0;
          j--, pvAxis++)
      {
        orxFLOAT  fMin, fMax, fProj;
        orxU32    k;

        /* Gets initial projected values */
        fMin = fMax = fProj = orxVector_2DDot(&vToCorner[0], pvAxis);

        /* For all remaining corners */
        for(k = 1; k < 4; k++)
        {
          /* Gets projected value */
          fProj = orxVector_2DDot(&vToCorner[k], pvAxis);

          /* Updates extrema */
          if(fProj > fMax)
          {
            fMax = fProj;
          }
          else if(fProj < fMin)
          {
            fMin = fProj;
          }
        }

        /* Not intersecting? */
        if((fMax < orxFLOAT_0)
        || (fMin > orxVector_GetSquareSize(pvAxis)))
        {
          /* Updates result */
          bResult = orxFALSE;
          break;
        }
      }
    }
  }
  else
  {
    /* Updates result */
    bResult = orxFALSE;
  }

  /* Done! */
  return bResult;
}

#endif /* _orxOBOX_H_ */

/** @} */
