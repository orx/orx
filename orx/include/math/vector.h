#ifndef _COORD_H_
#define _COORD_H_

#include "include.h"


/** Public structure definition. */
typedef struct st_coord_t
{
  /** Integer coordinates : 12 */
  int32 s32_x, s32_y, s32_z;              

  /* 4 extra bytes of padding : 16 */
  uint8 auc_unused[4];
} coord_st_coord;


/** Tells if coord is null (Warning : null != (0, 0, 0)) */
extern bool coord_is_null(coord_st_coord *_pst_coord);
/** Nullify a coord. */
extern void coord_reset(coord_st_coord *_pst_coord);
/** Sets coord values. */
extern void coord_set(coord_st_coord *_pst_coord, int32 _i_x, int32 _i_y, int32 _i_z);

/** Creates a coord with given values. */
extern coord_st_coord *coord_create(int32 _i_x, int32 _i_y, int32 _i_z);
/** Deletes a coord. */
extern void coord_delete(coord_st_coord *_pst_coord);

/** Copies coord values into another one. */
extern void coord_copy(coord_st_coord *_pst_dest, coord_st_coord *_pst_src);

/** Adds coords and stores result in a third one. */
extern void coord_add(coord_st_coord *_pst_result, coord_st_coord *_pst_op1, coord_st_coord *_pst_op2);
/** Subs a coord from another one and stores result in a third one. */
extern void coord_sub(coord_st_coord *_pst_result, coord_st_coord *_pst_op1, coord_st_coord *_pst_op2);
/** Muls a coord by a float and stores result in another one. */
extern void coord_mul(coord_st_coord *_pst_result, coord_st_coord *_pst_op1, float _f_op2);
/** Divs a coord by a float and stores result in another one. */
extern void coord_div(coord_st_coord *_pst_result, coord_st_coord *_pst_op1, float _f_op2);
/** Rotates a coord using a float angle (RAD) and stores result in another one. */
extern void coord_rotate(coord_st_coord *_pst_result, coord_st_coord *_pst_op1, float _f_op2);
/** Negates a coord and stores result in another one. */
extern void coord_neg(coord_st_coord *_pst_result, coord_st_coord *_pst_op);

/** Reorders axis aligned box corners (result is real upper left & bottom right corners). */
extern void coord_aabox_reorder(coord_st_coord *_pst_box_ul, coord_st_coord *_pst_box_br);
/** Tests axis aligned box intersection given corners (if corners are not sorted, test won't work). */
extern bool coord_aabox_intersection_test(coord_st_coord *_pst_box1_ul, coord_st_coord *_pst_box1_br, coord_st_coord *_pst_box2_ul, coord_st_coord *_pst_box2_br);

#endif /* _COORD_H_ */
