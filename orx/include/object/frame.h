/** 
 * \file frame.h
 * 
 * Frame Module.
 * Allows to creates and handle frames.
 * Frames are structures used to give position, orientation & scale to objects.
 * They thus can be referenced by other structures.
 * 
 * \todo
 * Use matrix instead of disjoint pos vector/angle float/scale float for frame data structure.
 * 3D system (later).
 */


/***************************************************************************
 frame.h
 frame module
 
 begin                : 02/12/2003
 author               : (C) Gdp
 email                : iarwain@ifrance.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _FRAME_H_
#define _FRAME_H_

#include "include.h"

#include "math/coord.h"


/** Internal frame structure. */
typedef struct st_frame_t frame_st_frame;


/** Inits the frame system. */
extern uint32           frame_init();
/** Ends the frame system. */
extern void             frame_exit();

/** Creates a frame. */
extern frame_st_frame  *frame_create();
/** Deletes a frame. */
extern void             frame_delete(frame_st_frame *_pst_frame);

/** Cleans all frames render status */
extern void             frame_render_status_clean();
/** Test frame render status (TRUE : clean / FALSE : dirty)*/
extern bool             frame_render_status_ok(frame_st_frame *_pst_frame);

/** Does frame use differential scrolling? */
extern bool             frame_differential_scrolling_use(frame_st_frame *_pst_frame);
/** Sets frame differential scrolling values (X & Y axis / 0.0 : none) */
extern void             frame_differential_scrolling_set(frame_st_frame * _pst_frame, float _f_x_axis, float _f_y_axis);
/** Gets frame differential scrolling values (X & Y axis / 0.0 : none) */
extern void             frame_differential_scrolling_get(frame_st_frame * _pst_frame, float *_pf_x_axis, float *_pf_y_axis);

/** Sets a frame parent. */
extern void             frame_parent_set(frame_st_frame *_pst_frame, frame_st_frame * _pst_parent);

/** Sets 2D local position */
extern void             frame_2d_position_set(frame_st_frame *_pst_frame, coord_st_coord *_pst_coord);
/** Sets 2D local rotation */
extern void             frame_2d_rotation_set(frame_st_frame *_pst_frame, float _f_angle);
/** Sets 2D local scale */
extern void             frame_2d_scale_set(frame_st_frame *_pst_frame, float _f_scale);

/** Gets 2D position global/local according to param */
extern void             frame_2d_position_get(frame_st_frame *_pst_frame, coord_st_coord *_pst_coord, bool _b_local);
/** Gets 2D global rotation global/local according to param */
extern float            frame_2d_rotation_get(frame_st_frame *_pst_frame, bool _b_local);
/** Gets 2D scale global/local according to param */
extern float            frame_2d_scale_get(frame_st_frame *_pst_frame, bool _b_local);

/** Computes a frame global data using parent's global & self local ones. Result is stored in another frame. */
extern void             frame_global_data_compute(frame_st_frame *_pst_dest_frame, frame_st_frame *_pst_src_frame, frame_st_frame *_pst_parent_frame);

#endif /* _FRAME_H_ */
