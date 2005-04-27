/** 
 * \file orxVector.h
 * 
 * Vector module.
 * Handles vectors.
 * 
 * \todo
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


/** Public structure definition. */
typedef struct __orxVEC_t
{
  /** Integer coordinates : 16 */
  orxFLOAT fX, fY, fZ, fW;

} orxVEC;


/** Tells if coord is null (Warning : null != (0, 0, 0)) */
extern orxBOOL coord_is_null(orxVEC *_pst_coord);
/** Nullify a coord. */
extern orxVOID coord_reset(orxVEC *_pst_coord);
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


#endif /* _orxVECTOR_H_ */
