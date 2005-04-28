/** 
 * \file orxVec.h
 * 
 * Vector module.
 * Handles vectors.
 * 
 * \todo
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


/** Public structure definition. */
typedef struct __orxVEC_t
{
  /** Coordinates : 16 */
  orxFLOAT fX, fY, fZ, fW;

} orxVEC;


/** Sets vector x / y / z / w values. */
orxSTATIC orxINLINE orxVEC *orxCONST    orxVec_Set4(orxVEC *_pvVec, orxFLOAT _fX, orxFLOAT _fY, orxFLOAT _fZ, orxFLOAT _fW)
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
orxSTATIC orxINLINE orxVEC *orxCONST    orxVec_Set3(orxVEC *_pvVec, orxFLOAT _fX, orxFLOAT _fY, orxFLOAT _fZ)
{
  /* Done ! */
  return(orxVec_Set4(_pvVec, _fX, _fY, _fZ, orx2F(0.0f)));
}

/** Sets value in all vector fields. */
orxSTATIC orxINLINE orxVEC *orxCONST    orxVec_SetAll(orxVEC *_pvVec, orxFLOAT _fValue)
{
  /* Done ! */
  return(orxVec_Set4(_pvVec, _fValue, _fValue, _fValue, _fValue));
}

/** Sets coord values. */
extern orxVOID coord_set(orxVEC *_pst_coord, orxS32 _i_x, orxS32 _i_y, orxS32 _i_z);

/** Creates a coord with given values. */
extern orxVEC *coord_create(orxS32 _i_x, orxS32 _i_y, orxS32 _i_z);
/** Deletes a coord. */
extern orxVOID coord_delete(orxVEC *_pst_coord);

/** Copies coord values into another one. */
extern orxVOID coord_copy(orxVEC *_pst_dest, orxVEC *_pst_src);

/** Adds coords and stores result in a third one. */
extern orxVOID coord_add(orxVEC *_pst_result, orxVEC *_pst_op1, orxVEC *_pst_op2);
/** Subs a coord from another one and stores result in a third one. */
extern orxVOID coord_sub(orxVEC *_pst_result, orxVEC *_pst_op1, orxVEC *_pst_op2);
/** Muls a coord by a orxFLOAT and stores result in another one. */
extern orxVOID coord_mul(orxVEC *_pst_result, orxVEC *_pst_op1, orxFLOAT _f_op2);
/** Divs a coord by a orxFLOAT and stores result in another one. */
extern orxVOID coord_div(orxVEC *_pst_result, orxVEC *_pst_op1, orxFLOAT _f_op2);
/** Rotates a coord using a orxFLOAT angle (RAD) and stores result in another one. */
extern orxVOID coord_rotate(orxVEC *_pst_result, orxVEC *_pst_op1, orxFLOAT _f_op2);
/** Negates a coord and stores result in another one. */
extern orxVOID coord_neg(orxVEC *_pst_result, orxVEC *_pst_op);

/** Reorders axis aligned box corners (result is real upper left & bottom right corners). */
extern orxVOID coord_aabox_reorder(orxVEC *_pst_box_ul, orxVEC *_pst_box_br);
/** Tests axis aligned box intersection given corners (if corners are not sorted, test won't work). */
extern orxBOOL coord_aabox_intersection_test(orxVEC *_pst_box1_ul, orxVEC *_pst_box1_br, orxVEC *_pst_box2_ul, orxVEC *_pst_box2_br);


#endif /* _orxVEC_H_ */
