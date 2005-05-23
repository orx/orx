/** 
 * \file orxVec.h
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
 orxVec.h
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


#ifndef _orxVEC_H_
#define _orxVEC_H_

#include "orxInclude.h"

#include "debug/orxDebug.h"
#include "math/orxMath.h"
#include "memory/orxMemory.h"


/** Public structure definition. */
typedef struct __orxVEC_t
{
  /** Coordinates : 16 */
  orxFLOAT fX, fY, fZ, fW;

} orxVEC;


/** Rotates a coord using a orxFLOAT angle (RAD), an axis and stores result in another one. */
extern orxVEC *orxFASTCALL              orxVec_Rot(orxVEC *_pvRes, orxCONST orxVEC *_pvOp, orxCONST orxVEC *_pvAxis, orxFLOAT _fAngle);

/** Reorders axis aligned box corners (result is real upper left & bottom right corners). */
extern orxVOID orxFASTCALL              orxVec_ReorderAABox(orxVEC *_pvULBox, orxVEC *_pvBRBox);

/** Tests axis aligned box intersection given corners (if corners are not sorted, test won't work). */
extern orxBOOL orxFASTCALL              orxVec_TestAABoxIntersection(orxCONST orxVEC *_pvULBox1, orxCONST orxVEC *_pvBRBox1, orxCONST orxVEC *_pvULBox2, orxCONST orxVEC *_pvBRBox2);


/* *** Vector inlined functions *** */


/** Sets vector x / y / z / w values. */
orxSTATIC orxINLINE orxVEC             *orxVec_Set4(orxVEC *_pvVec, orxFLOAT _fX, orxFLOAT _fY, orxFLOAT _fZ, orxFLOAT _fW)
{
  /* Checks */
  orxASSERT(_pvVec != orxNULL);

  /* Stores values */
  _pvVec->fX = _fX;
  _pvVec->fY = _fY;
  _pvVec->fZ = _fZ;
  _pvVec->fW = _fW;

  /* Done ! */
  return _pvVec;
}

/** Sets vector x / y / z values. */
orxSTATIC orxINLINE orxVEC             *orxVec_Set3(orxVEC *_pvVec, orxFLOAT _fX, orxFLOAT _fY, orxFLOAT _fZ)
{
  /* Done ! */
  return(orxVec_Set4(_pvVec, _fX, _fY, _fZ, orx2F(0.0f)));
}

/** Sets value in all vector fields. */
orxSTATIC orxINLINE orxVEC             *orxVec_SetAll(orxVEC *_pvVec, orxFLOAT _fValue)
{
  /* Done ! */
  return(orxVec_Set4(_pvVec, _fValue, _fValue, _fValue, _fValue));
}

/** Copies vector values into another one. */
orxSTATIC orxINLINE orxVOID             orxVec_Copy(orxVEC *_pvDst, orxCONST orxVEC *_pvSrc)
{
  /* Checks */
  orxASSERT(_pvDst != orxNULL);
  orxASSERT(_pvSrc != orxNULL);

  /* Copies it */
  orxMemory_Copy(_pvDst, _pvSrc, sizeof(orxVEC));

  /* Done! */
  return;
}

/** Adds vectors and stores result in a third one. */
orxSTATIC orxINLINE orxVEC             *orxVec_Add(orxVEC *_pvRes, orxCONST orxVEC *_pvOp1, orxCONST orxVEC *_pvOp2)
{
  /* Checks */
  orxASSERT(_pvRes != orxNULL);
  orxASSERT(_pvOp1 != orxNULL);
  orxASSERT(_pvOp2 != orxNULL);

  /* Adds all */
  _pvRes->fX = _pvOp1->fX + _pvOp2->fX;
  _pvRes->fY = _pvOp1->fY + _pvOp2->fY;
  _pvRes->fZ = _pvOp1->fZ + _pvOp2->fZ;
  _pvRes->fW = _pvOp1->fW + _pvOp2->fW;

  /* Done! */
  return _pvRes;
}

/** Muls a vector by an orxFLOAT and stores result in another one. */
orxSTATIC orxINLINE orxVEC             *orxVec_Mul(orxVEC *_pvRes, orxCONST orxVEC *_pvOp1, orxFLOAT _fOp2)
{
  /* Checks */
  orxASSERT(_pvRes != orxNULL);
  orxASSERT(_pvOp1 != orxNULL);

  /* Muls all */
  _pvRes->fX = _pvOp1->fX * _fOp2;
  _pvRes->fY = _pvOp1->fY * _fOp2;
  _pvRes->fZ = _pvOp1->fZ * _fOp2;
  _pvRes->fW = _pvOp1->fW * _fOp2;

  /* Done! */
  return _pvRes;
}

/** Negates a vector and stores result in another one. */
orxSTATIC orxINLINE orxVEC             *orxVec_Neg(orxVEC *_pvRes, orxCONST orxVEC *_pvOp)
{
  /* Checks */
  orxASSERT(_pvRes != orxNULL);
  orxASSERT(_pvOp  != orxNULL);

  /* Negates all */
  _pvRes->fX = -(_pvOp->fX);
  _pvRes->fY = -(_pvOp->fY);
  _pvRes->fZ = -(_pvOp->fZ);
  _pvRes->fW = -(_pvOp->fW);

  /* Done! */
  return _pvRes;
}

/** Reverses a vector and stores result in another one. */
orxSTATIC orxINLINE orxVEC             *orxVec_Inv(orxVEC *_pvRes, orxCONST orxVEC *_pvOp)
{
  /* Checks */
  orxASSERT(_pvRes != orxNULL);
  orxASSERT(_pvOp  != orxNULL);

  /* Negates all */
  _pvRes->fX = orx2F(1.0f) / _pvOp->fX;
  _pvRes->fY = orx2F(1.0f) / _pvOp->fY;
  _pvRes->fZ = orx2F(1.0f) / _pvOp->fZ;
  _pvRes->fW = orx2F(1.0f) / _pvOp->fW;

  /* Done! */
  return _pvRes;
}


/* *** Vector constants *** */


orxSTATIC orxCONST  orxVEC      orxVEC_X    = {orx2F(1.0f), orx2F(0.0f), orx2F(0.0f), orx2F(0.0f)};
orxSTATIC orxCONST  orxVEC      orxVEC_Y    = {orx2F(0.0f), orx2F(1.0f), orx2F(0.0f), orx2F(0.0f)};
orxSTATIC orxCONST  orxVEC      orxVEC_Z    = {orx2F(0.0f), orx2F(0.0f), orx2F(1.0f), orx2F(0.0f)};

orxSTATIC orxCONST  orxVEC      orxVEC_NULL = {orx2F(0.0f), orx2F(0.0f), orx2F(0.0f), orx2F(0.0f)};


#endif /* _orxVEC_H_ */
