/***************************************************************************
 orxVector.c
 Vector module
 
 begin                : 27/04/2005
 author               : (C) Arcallians
 email                : iarwain@arcallians.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This library is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License           *
 *   as published by the Free Software Foundation; either version 2.1      *
 *   of the License, or (at your option) any later version.                *
 *                                                                         *
 ***************************************************************************/


#include "math/orxVector.h"


/***************************************************************************
 ***************************************************************************
 ******                       LOCAL FUNCTIONS                         ******
 ***************************************************************************
 ***************************************************************************/


/***************************************************************************
 ***************************************************************************
 ******                       PUBLIC FUNCTIONS                        ******
 ***************************************************************************
 ***************************************************************************/

/***************************************************************************
 orxVector_2DRot
 Rotates a coord using a orxFLOAT angle (RAD), an axis and stores result in another one.

 returns: Rotated vector
 ***************************************************************************/
orxVECTOR *orxFASTCALL orxVector_2DRot(orxVECTOR *_pvRes, orxCONST orxVECTOR *_pvOp, orxFLOAT _fAngle)
{
  orxREGISTER orxFLOAT fSin, fCos;

  /* Checks */
  orxASSERT(_pvRes  != orxNULL);
  orxASSERT(_pvOp != orxNULL);

  /* Gets cos & sin of angle */
  fCos = orxMath_Cos(_fAngle);
  fSin = orxMath_Sin(_fAngle);

  /* Updates result */
  orxVector_Set(_pvRes, (fCos * _pvOp->fX) - (fSin * _pvOp->fY), (fSin * _pvOp->fX) + (fCos * _pvOp->fY), _pvOp->fZ);

  /* Done! */
  return _pvRes;
}

/***************************************************************************
 orxAABox_Reorder
 Reorders axis aligned box corners (result is real upper left & bottom right corners).

 returns: Nothing
 ***************************************************************************/
orxVOID orxFASTCALL orxAABox_Reorder(orxAABOX *_pstBox)
{
  /* Checks */
  orxASSERT(_pstBox != orxNULL);

/* Reorders coordinates so as to have upper left & bottom right box corners */

  /* Z coord */
  if(_pstBox->vTL.fZ > _pstBox->vBR.fZ)
  {
    /* Swaps */
    orxSWAP32(_pstBox->vTL.fZ, _pstBox->vBR.fZ);
  }

  /* Y coord */
  if(_pstBox->vTL.fY > _pstBox->vBR.fY)
  {
    /* Swaps */
    orxSWAP32(_pstBox->vTL.fY, _pstBox->vBR.fY);
  }

  /* X coord */
  if(_pstBox->vTL.fX > _pstBox->vBR.fX)
  {
    /* Swaps */
    orxSWAP32(_pstBox->vTL.fX, _pstBox->vBR.fX);
  }

  /* Done! */
  return;
}
