/** 
 * \file camera.h
 * 
 * Camera Module.
 * Allows to creates and handle cameras.
 * Camera are structures used to render graphic (2D/3D) objects.
 * They thus can be referenced by other structures.
 * 
 * \todo
 * Optimize view list update.
 * Adds 3d system (Not planned yet).
 */


/***************************************************************************
 camera.h
 Texture module
 
 begin                : 10/12/2003
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


#ifndef _CAMERA_H_
#define _CAMERA_H_

#include "include.h"

#include "math/coord.h"
#include "object/frame.h"
#include "object/object.h"


/** Camera ID Flags. */
#define CAMERA_KUL_ID_FLAG_2D             0x00000010


/** Internal camera structure. */
typedef struct st_camera_t camera_st_camera;

/** Public camera view list structure. */
typedef struct st_view_list_t camera_st_view_list;


/** Inits the camera system. */
extern uint32               camera_init();
/** Exits from the camera system. */
extern void                 camera_exit();

/** Creates an empty camera. */
extern camera_st_camera    *camera_create();
/** Deletes a camera. */
extern void                 camera_delete(camera_st_camera *_pst_camera);

/** Updates camera view list. */
extern void                 camera_view_list_update(camera_st_camera *_pst_camera);

/** !!! Warning : Camera accessors don't test parameter validity !!! */

/** Gets camera first view list. */
extern camera_st_view_list *camera_view_list_first_get(camera_st_camera *_pst_camera);
/** Gets camera next view list. */
extern camera_st_view_list *camera_view_list_next_get(camera_st_camera *_pst_camera);

/** Gets camera view list number. */
extern int32                camera_view_list_number_get(camera_st_camera *_pst_camera);
/** Gets view list screen frame. */
extern frame_st_frame      *camera_view_list_screen_frame_get(camera_st_view_list *_pst_view_list);
/** Gets view list object. */
extern object_st_object    *camera_view_list_object_get(camera_st_view_list *_pst_view_list);

/** Camera 2D position set accessor. */
extern void                 camera_2d_position_set(camera_st_camera *_pst_camera, coord_st_coord *_pst_position);
/** Camera 2D size set accessor. */
extern void                 camera_2d_size_set(camera_st_camera *_pst_camera, coord_st_coord *_pst_size);
/** Camera 2D rotation set accessor. */
extern void                 camera_2d_rotation_set(camera_st_camera *_pst_camera, float _f_rotation);
/** Camera 2D zoom set accessor. */
extern void                 camera_2d_zoom_set(camera_st_camera *_pst_camera, float _f_zoom);

/** Camera link set accessor. */
extern void                 camera_link_set(camera_st_camera *_pst_camera, object_st_object *_pst_object);
/** Camera limit set accessor (Upper left & Bottom right corners positions). */
extern void                 camera_limit_set(camera_st_camera *_pst_camera, coord_st_coord *_pst_ul, coord_st_coord *_pst_br);
/** Camera limit reset accessor (Removes all position limits). */
extern void                 camera_limit_reset(camera_st_camera *_pst_camera);

/** Camera on screen position set accessor. */
extern void                 camera_on_screen_position_set(camera_st_camera *_pst_camera, coord_st_coord *_pst_position);

/** Camera 2D position get accessor. */
extern void                 camera_2d_position_get(camera_st_camera *_pst_camera, coord_st_coord *_pst_position);
/** Camera 2D size get accessor. */
extern void                 camera_2d_size_get(camera_st_camera *_pst_camera, coord_st_coord *_pst_size);
/** Camera 2D rotation get accessor. */
extern float                camera_2d_rotation_get(camera_st_camera *_pst_camera);
/** Camera 2D zoom get accessor. */
extern float                camera_2d_zoom_get(camera_st_camera *_pst_camera);
/** Camera link get accessor. */

extern object_st_object    *camera_link_get(camera_st_camera *_pst_camera);
/** Camera limit get accessor (Upper left & Bottom right corners positions). */
extern void                 camera_limit_get(camera_st_camera *_pst_camera, coord_st_coord *_pst_ul, coord_st_coord *_pst_br);
/** Camera on screen position get accessor. */
extern void                 camera_on_screen_position_get(camera_st_camera *_pst_camera, coord_st_coord *_pst_position);

/** Camera flag test accessor. */
extern bool                 camera_flag_test(camera_st_camera *_pst_camera, uint32 _u32_flag);
/** Camera flag get/set accessor. */
extern void                 camera_flag_set(camera_st_camera *_pst_camera, uint32 _u32_add_flags, uint32 _u32_remove_flags);

#endif /* _CAMERA_H_ */
