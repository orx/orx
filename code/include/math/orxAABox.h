/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2013 Orx-Project
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
 * @file orxAABox.h
 * @date 03/10/2008
 * @author iarwain@orx-project.org
 *
 * @todo
 */

/**
 * @addtogroup orxAABox
 * 
 * Axis-aligned box module
 * Module that handles axis-aligned boxes
 *
 * @{
 */


#ifndef _orxAABOX_H_
#define _orxAABOX_H_

#include "orxInclude.h"

#include "math/orxVector.h"


/** Public structure definition
 */
typedef struct __orxAABOX_t
{
  orxVECTOR vTL; /**< Top left corner vector : 12 */
  orxVECTOR vBR; /**< Bottom right corner vector : 24 */

} orxAABOX;


/* *** AABox inlined functions *** */


/** Reorders AABox corners
 * @param[in]   _pstBox                       Box to reorder
 * @return      Reordered AABox
 */
static orxINLINE orxAABOX *                   orxAABox_Reorder(orxAABOX *_pstBox)
{
  /* Checks */
  orxASSERT(_pstBox != orxNULL);

  /* Reorders coordinates so as to have upper left & bottom right box corners */

  /* X coord */
  if(_pstBox->vTL.fX > _pstBox->vBR.fX)
  {
     orxFLOAT fTemp;

     /* Swaps */
     fTemp           = _pstBox->vTL.fX;
     _pstBox->vTL.fX = _pstBox->vBR.fX;
     _pstBox->vBR.fX = fTemp;
  }

  /* Y coord */
  if(_pstBox->vTL.fY > _pstBox->vBR.fY)
  {
    orxFLOAT fTemp;

    /* Swaps */
    fTemp           = _pstBox->vTL.fY;
    _pstBox->vTL.fY = _pstBox->vBR.fY;
    _pstBox->vBR.fY = fTemp;
  }

  /* Z coord */
  if(_pstBox->vTL.fZ > _pstBox->vBR.fZ)
  {
    orxFLOAT fTemp;

    /* Swaps */
    fTemp           = _pstBox->vTL.fZ;
    _pstBox->vTL.fZ = _pstBox->vBR.fZ;
    _pstBox->vBR.fZ = fTemp;
  }

  /* Done! */
  return _pstBox;
}

/** Sets axis aligned box values
 * @param[out]  _pstRes                       AABox to set
 * @param[in]   _pvTL                         Top left corner
 * @param[in]   _pvBR                         Bottom right corner
 * @return      orxAABOX / orxNULL
 */
static orxINLINE orxAABOX *                   orxAABox_Set(orxAABOX *_pstRes, const orxVECTOR *_pvTL, const orxVECTOR *_pvBR)
{
  /* Checks */
  orxASSERT(_pstRes != orxNULL);
  orxASSERT(_pvTL != orxNULL);
  orxASSERT(_pvBR != orxNULL);

  /* Sets values */
  orxVector_Copy(&(_pstRes->vTL), _pvTL);
  orxVector_Copy(&(_pstRes->vBR), _pvBR);

  /* Reorders corners */
  orxAABox_Reorder(_pstRes);

  /* Done! */
  return _pstRes;
}

/** Is position inside axis aligned box test
 * @param[in]   _pstBox                       Box to test against position
 * @param[in]   _pvPosition                   Position to test against the box
 * @return      orxTRUE if position is inside the box, orxFALSE otherwise
 */
static orxINLINE orxBOOL                      orxAABox_IsInside(const orxAABOX *_pstBox, const orxVECTOR *_pvPosition)
{
  register orxBOOL bResult = orxFALSE;

  /* Checks */
  orxASSERT(_pstBox != orxNULL);
  orxASSERT(_pvPosition != orxNULL);

  /* Z intersected? */
  if((_pvPosition->fZ >= _pstBox->vTL.fZ)
  && (_pvPosition->fZ <= _pstBox->vBR.fZ))
  {
    /* X intersected? */
    if((_pvPosition->fX >= _pstBox->vTL.fX)
    && (_pvPosition->fX <= _pstBox->vBR.fX))
    {
      /* Y intersected? */
      if((_pvPosition->fY >= _pstBox->vTL.fY)
      && (_pvPosition->fY <= _pstBox->vBR.fY))
      {
        /* Intersects */
        bResult = orxTRUE;
      }
    }
  }

  /* Done! */
  return bResult;
}

/** Tests axis aligned box intersection
 * @param[in]   _pstBox1                      First box operand
 * @param[in]   _pstBox2                      Second box operand
 * @return      orxTRUE if boxes intersect, orxFALSE otherwise
 */
static orxINLINE orxBOOL                      orxAABox_TestIntersection(const orxAABOX *_pstBox1, const orxAABOX *_pstBox2)
{
  register orxBOOL bResult = orxFALSE;

  /* Checks */
  orxASSERT(_pstBox1 != orxNULL);
  orxASSERT(_pstBox2 != orxNULL);

  /* Z intersected? */
  if((_pstBox2->vBR.fZ >= _pstBox1->vTL.fZ)
  && (_pstBox2->vTL.fZ <= _pstBox1->vBR.fZ))
  {
    /* X intersected? */
    if((_pstBox2->vBR.fX >= _pstBox1->vTL.fX)
    && (_pstBox2->vTL.fX <= _pstBox1->vBR.fX))
    {
      /* Y intersected? */
      if((_pstBox2->vBR.fY >= _pstBox1->vTL.fY)
      && (_pstBox2->vTL.fY <= _pstBox1->vBR.fY))
      {
        /* Intersects */
        bResult = orxTRUE;
      }
    }
  }

  /* Done! */
  return bResult;
}

/** Tests axis aligned box 2D intersection (no Z-axis test)
 * @param[in]   _pstBox1                      First box operand
 * @param[in]   _pstBox2                      Second box operand
 * @return      orxTRUE if boxes intersect in 2D, orxFALSE otherwise
 */
static orxINLINE orxBOOL                      orxAABox_Test2DIntersection(const orxAABOX *_pstBox1, const orxAABOX *_pstBox2)
{
  register orxBOOL bResult = orxFALSE;

  /* Checks */
  orxASSERT(_pstBox1 != orxNULL);
  orxASSERT(_pstBox2 != orxNULL);

  /* X intersected? */
  if((_pstBox2->vBR.fX >= _pstBox1->vTL.fX)
  && (_pstBox2->vTL.fX <= _pstBox1->vBR.fX))
  {
    /* Y intersected? */
    if((_pstBox2->vBR.fY >= _pstBox1->vTL.fY)
    && (_pstBox2->vTL.fY <= _pstBox1->vBR.fY))
    {
      /* Intersects */
      bResult = orxTRUE;
    }
  }

  /* Done! */
  return bResult;
}

/** Copies an AABox onto another one
 * @param[out]   _pstDst                      AABox to copy to (destination)
 * @param[in]   _pstSrc                       AABox to copy from (destination)
 * @return      Destination AABox
 */
static orxINLINE orxAABOX *                   orxAABox_Copy(orxAABOX *_pstDst, const orxAABOX *_pstSrc)
{
  /* Checks */
  orxASSERT(_pstDst != orxNULL);
  orxASSERT(_pstSrc != orxNULL);

  /* Copies it */
  orxMemory_Copy(_pstDst, _pstSrc, sizeof(orxAABOX));

  /* Done! */
  return _pstDst;
}

/** Moves an AABox
 * @param[out]  _pstRes                       AABox where to store result
 * @param[in]   _pstOp                        AABox to move
 * @param[in]   _pvMove                       Move vector
 * @return      Moved AABox
 */
static orxINLINE orxAABOX *                   orxAABox_Move(orxAABOX *_pstRes, const orxAABOX *_pstOp, const orxVECTOR *_pvMove)
{
  /* Checks */
  orxASSERT(_pstRes != orxNULL);
  orxASSERT(_pstOp != orxNULL);
  orxASSERT(_pvMove != orxNULL);

  /* Updates result */
  orxVector_Add(&(_pstRes->vTL), &(_pstOp->vTL), _pvMove);
  orxVector_Add(&(_pstRes->vBR), &(_pstOp->vBR), _pvMove);

  /* Done! */
  return _pstRes;
}

/** Gets AABox center position
 * @param[in]   _pstOp                        Concerned AABox
 * @param[out]  _pvRes                        Center position
 * @return      Center position vector
 */
static orxINLINE orxVECTOR *                  orxAABox_GetCenter(const orxAABOX *_pstOp, orxVECTOR *_pvRes)
{
  /* Checks */
  orxASSERT(_pstOp != orxNULL);
  orxASSERT(_pvRes != orxNULL);

  /* Gets box center */
  orxVector_Add(_pvRes, &(_pstOp->vTL), &(_pstOp->vBR));
  orxVector_Mulf(_pvRes, _pvRes, orx2F(0.5f));

  /* Done! */
  return _pvRes;
}

#endif /* _orxAABOX_H_ */

/** @} */
