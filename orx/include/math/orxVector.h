/** 
 * \file orxVector.h
 * 
 * Vector module.
 * Handles vectors.
 * 
 * \todo
 * Extract box code into box module
 * Adds rotate function with matrix module, when it's done
 * Gets it intrinsic depending on platform.
 * All handling functions.
 */


/***************************************************************************
 orxVector.h
 Vector module
 
 begin                : 30/03/2005
 author               : (C) Arcallians
 email                : iarwain@arcallians.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef _orxVECTOR_H_
#define _orxVECTOR_H_

#include "orxInclude.h"

#include "debug/orxDebug.h"
#include "memory/orxMemory.h"
#include "math/orxMath.h"


/** Public structure definition. */
typedef struct __orxVECTOR_t
{
  /** Coordinates : 12 */
  union
  {
    orxFLOAT fX;
    orxFLOAT fRho;
  };

  union
  {
    orxFLOAT fY;
    orxFLOAT fTheta;
  };

  union
  {
    orxFLOAT fZ;
    orxFLOAT fPhi;
  };

} orxVECTOR;


/** Rotates a coord using a orxFLOAT angle (RAD), an axis and stores result in another one. */
extern orxDLLAPI orxVECTOR *orxFASTCALL       orxVector_Rot(orxVECTOR *_pvRes, orxCONST orxVECTOR *_pvOp, orxCONST orxVECTOR *_pvAxis, orxFLOAT _fAngle);

/** Reorders axis aligned box corners (result is real upper left & bottom right corners). */
extern orxDLLAPI orxVOID orxFASTCALL          orxVector_ReorderAABox(orxVECTOR *_pvULBox, orxVECTOR *_pvBRBox);


/* *** Vector inlined functions *** */


/** Tests axis aligned box intersection given corners (if corners are not sorted, test won't work). */
orxSTATIC orxINLINE orxBOOL                   orxVector_TestAABoxIntersection(orxCONST orxVECTOR *_pvULBox1, orxCONST orxVECTOR *_pvBRBox1, orxCONST orxVECTOR *_pvULBox2, orxCONST orxVECTOR *_pvBRBox2)
{
  orxREGISTER orxBOOL bResult = orxFALSE;

  /* Checks */
  orxASSERT(_pvULBox1 != orxNULL);
  orxASSERT(_pvBRBox1 != orxNULL);
  orxASSERT(_pvULBox2 != orxNULL);
  orxASSERT(_pvBRBox2 != orxNULL);

  /* Warning : Corners should be sorted otherwise test won't work! */

  /* Z intersected? */
  if((_pvBRBox2->fZ >= _pvULBox1->fZ)
  && (_pvULBox2->fZ <= _pvBRBox1->fZ))
  {
    /* X intersected? */
    if((_pvBRBox2->fX >= _pvULBox1->fX)
    && (_pvULBox2->fX <= _pvBRBox1->fX))
    {
      /* Y intersected? */
      if((_pvBRBox2->fY >= _pvULBox1->fY)
      && (_pvULBox2->fY <= _pvBRBox1->fY))
      {
        /* Intersects */
        bResult = orxTRUE;
      }
    }
  }

  /* Done! */
  return bResult;
}

/** Sets vector x / y / z / w values. */
orxSTATIC orxINLINE orxVECTOR                *orxVector_Set(orxVECTOR *_pvVec, orxFLOAT _fX, orxFLOAT _fY, orxFLOAT _fZ)
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

/** Sets value in all vector fields. */
orxSTATIC orxINLINE orxVECTOR                *orxVector_SetAll(orxVECTOR *_pvVec, orxFLOAT _fValue)
{
  /* Done ! */
  return(orxVector_Set(_pvVec, _fValue, _fValue, _fValue));
}

/** Copies vector values into another one. */
orxSTATIC orxINLINE orxVOID                   orxVector_Copy(orxVECTOR *_pvDst, orxCONST orxVECTOR *_pvSrc)
{
  /* Checks */
  orxASSERT(_pvDst != orxNULL);
  orxASSERT(_pvSrc != orxNULL);

  /* Copies it */
  orxMemory_Copy(_pvDst, _pvSrc, sizeof(orxVECTOR));

  /* Done! */
  return;
}

/** Adds vectors and stores result in a third one. */
orxSTATIC orxINLINE orxVECTOR                *orxVector_Add(orxVECTOR *_pvRes, orxCONST orxVECTOR *_pvOp1, orxCONST orxVECTOR *_pvOp2)
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

/** Subs vectors and stores result in a third one. */
orxSTATIC orxINLINE orxVECTOR                *orxVector_Sub(orxVECTOR *_pvRes, orxCONST orxVECTOR *_pvOp1, orxCONST orxVECTOR *_pvOp2)
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

/** Muls a vector by an orxFLOAT and stores result in another one. */
orxSTATIC orxINLINE orxVECTOR                *orxVector_Mulf(orxVECTOR *_pvRes, orxCONST orxVECTOR *_pvOp1, orxFLOAT _fOp2)
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

/** Muls a vector by another vector and stores result in a third one. */
orxSTATIC orxINLINE orxVECTOR                *orxVector_Mul(orxVECTOR *_pvRes, orxCONST orxVECTOR *_pvOp1, orxCONST orxVECTOR *_pvOp2)
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

/** Divs a vector by an orxFLOAT and stores result in another one. */
orxSTATIC orxINLINE orxVECTOR                *orxVector_Divf(orxVECTOR *_pvRes, orxCONST orxVECTOR *_pvOp1, orxFLOAT _fOp2)
{
  orxREGISTER orxFLOAT fInvCoef;

  /* Checks */
  orxASSERT(_pvRes != orxNULL);
  orxASSERT(_pvOp1 != orxNULL);
  orxASSERT(_fOp2 != orxFLOAT_0);

  /* Gets coef */
  fInvCoef = orxFLOAT_1 / _fOp2;

  /* Muls all */
  _pvRes->fX = _pvOp1->fX * fInvCoef;
  _pvRes->fY = _pvOp1->fY * fInvCoef;
  _pvRes->fZ = _pvOp1->fZ * fInvCoef;

  /* Done! */
  return _pvRes;
}

/** Divs a vector by another vector and stores result in a third one. */
orxSTATIC orxINLINE orxVECTOR                *orxVector_Div(orxVECTOR *_pvRes, orxCONST orxVECTOR *_pvOp1, orxCONST orxVECTOR *_pvOp2)
{
  /* Checks */
  orxASSERT(_pvRes != orxNULL);
  orxASSERT(_pvOp1 != orxNULL);
  orxASSERT(_pvOp2 != orxNULL);
  orxASSERT(_pvOp2->fX != orxFLOAT_0);
  orxASSERT(_pvOp2->fY != orxFLOAT_0);
  orxASSERT(_pvOp2->fZ != orxFLOAT_0);

  /* Divs all */
  _pvRes->fX = _pvOp1->fX / _pvOp2->fX;
  _pvRes->fY = _pvOp1->fY / _pvOp2->fY;
  _pvRes->fZ = _pvOp1->fZ / _pvOp2->fZ;

  /* Done! */
  return _pvRes;
}

/** Negates a vector and stores result in another one. */
orxSTATIC orxINLINE orxVECTOR                *orxVector_Neg(orxVECTOR *_pvRes, orxCONST orxVECTOR *_pvOp)
{
  /* Checks */
  orxASSERT(_pvRes != orxNULL);
  orxASSERT(_pvOp  != orxNULL);

  /* Negates all */
  _pvRes->fX = -(_pvOp->fX);
  _pvRes->fY = -(_pvOp->fY);
  _pvRes->fZ = -(_pvOp->fZ);

  /* Done! */
  return _pvRes;
}

/** Reverses a vector and stores result in another one. */
orxSTATIC orxINLINE orxVECTOR                *orxVector_Inv(orxVECTOR *_pvRes, orxCONST orxVECTOR *_pvOp)
{
  /* Checks */
  orxASSERT(_pvRes != orxNULL);
  orxASSERT(_pvOp  != orxNULL);

  /* Reverts all */
  _pvRes->fX = orxFLOAT_1 / _pvOp->fX;
  _pvRes->fY = orxFLOAT_1 / _pvOp->fY;
  _pvRes->fZ = orxFLOAT_1 / _pvOp->fZ;

  /* Done! */
  return _pvRes;
}

orxSTATIC orxINLINE orxFLOAT                  orxVector_GetSquareSize(orxCONST orxVECTOR *_pvOp)
{
  orxREGISTER orxFLOAT fResult;

  /* Checks */
  orxASSERT(_pvOp  != orxNULL);

  /* Updates result */
  fResult = (_pvOp->fX * _pvOp->fX) + (_pvOp->fY * _pvOp->fY) + (_pvOp->fZ * _pvOp->fZ);

  /* Done! */
  return fResult;
}

orxSTATIC orxINLINE orxFLOAT                  orxVector_GetSquareDistance(orxCONST orxVECTOR *_pvOp1, orxCONST orxVECTOR *_pvOp2)
{
  orxVECTOR   vTemp;
  orxREGISTER orxFLOAT fResult;

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

orxSTATIC orxINLINE orxVECTOR *               orxVector_FromCartesianToSpherical(orxVECTOR *_pvRes, orxCONST orxVECTOR *_pvOp)
{
  /* Checks */
  orxASSERT(_pvRes != orxNULL);
  orxASSERT(_pvOp  != orxNULL);

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
        /* Sets rho */
        fRho = _pvOp->fY;
      }
      /* X != 0 and Y = 0? */
      else if(_pvOp->fY == orxFLOAT_0)
      {
        /* Sets rho */
        fRho = _pvOp->fX;
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
        if(*(orxU32 *)&(_pvOp->fZ) & (orxU32)0x80000000)
        {
          orxU32 u32Temp;

          /* Gets absolute value */
          u32Temp = *(orxU32 *)&(_pvOp->fZ) & (orxU32)0x7FFFFFFF;

          /* Sets rho */
          fRho = *(orxFLOAT *)&u32Temp;

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

orxSTATIC orxINLINE orxVECTOR *               orxVector_FromSphericalToCartesian(orxVECTOR *_pvRes, orxCONST orxVECTOR *_pvOp)
{
  orxFLOAT fSinPhi, fCosPhi, fSinTheta, fCosTheta;

  /* Checks */
  orxASSERT(_pvRes != orxNULL);
  orxASSERT(_pvOp  != orxNULL);

  /* Gets sine & cosine */
  fSinTheta = orxMath_Sin(_pvOp->fTheta);
  fCosTheta = orxMath_Cos(_pvOp->fTheta);
  fSinPhi   = orxMath_Sin(_pvOp->fPhi);
  fCosPhi   = orxMath_Cos(_pvOp->fPhi);

  /* Updates result */
  _pvRes->fX = _pvOp->fRho * fCosTheta * fSinPhi;
  _pvRes->fY = _pvOp->fRho * fSinTheta * fSinPhi;
  _pvRes->fZ = _pvOp->fRho * fCosPhi;

  /* Done! */
  return _pvRes;
}


/* *** Vector constants *** */


orxSTATIC orxCONST  orxVECTOR      orxVECTOR_X    = {{orx2F(1.0f)}, {orx2F(0.0f)}, {orx2F(0.0f)}};
orxSTATIC orxCONST  orxVECTOR      orxVECTOR_Y    = {{orx2F(0.0f)}, {orx2F(1.0f)}, {orx2F(0.0f)}};
orxSTATIC orxCONST  orxVECTOR      orxVECTOR_Z    = {{orx2F(0.0f)}, {orx2F(0.0f)}, {orx2F(1.0f)}};

orxSTATIC orxCONST  orxVECTOR      orxVECTOR_0    = {{orx2F(0.0f)}, {orx2F(0.0f)}, {orx2F(0.0f)}};


#endif /* _orxVECTOR_H_ */
