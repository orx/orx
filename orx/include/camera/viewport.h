/** 
 * \file viewport.h
 * 
 * Viewport Module.
 * Allows to creates and handle viewports.
 * Viewports are structures associated to cameras and used for rendering.
 * They thus can be referenced by other structures.
 * 
 */


/***************************************************************************
 viewport.h
 Texture module
 
 begin                : 14/12/2003
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


#ifndef _VIEWPORT_H_
#define _VIEWPORT_H_

#include "include.h"

#include "camera/camera.h"
#include "graph/texture.h"


/** Viewport alingment defines */
#define VIEWPORT_KUL_FLAG_ALIGN_CENTER  0x00000000
#define VIEWPORT_KUL_FLAG_ALIGN_LEFT    0x10000000
#define VIEWPORT_KUL_FLAG_ALIGN_RIGHT   0x20000000
#define VIEWPORT_KUL_FLAG_ALIGN_TOP     0x40000000
#define VIEWPORT_KUL_FLAG_ALIGN_BOTTOM  0x80000000


/** Internal viewport structure. */
typedef struct st_viewport_t viewport_st_viewport;

/** Inits the viewport system. */
extern uint32                 viewport_init();
/** Ends the viewport system. */
extern void                   viewport_exit();

/** Creates an empty viewport. */
extern viewport_st_viewport  *viewport_create();
/** Deletes a viewport. */
extern void                   viewport_delete(viewport_st_viewport *_pst_viewport);

/** !!! Warning : viewport accessors don't test parameter validity !!! */

/** Viewport alignment set accessor (flags must be OR'ed). */
extern void                   viewport_align_set(viewport_st_viewport *_pst_viewport, uint32 _u32_align);

/** Viewport on screen camera position get accessor. */
extern void                   viewport_on_screen_camera_position_get(viewport_st_viewport *_pst_viewport, coord_st_coord *_pst_position);

/** Viewport surface set accessor. */
extern void                   viewport_surface_set(viewport_st_viewport *_pst_viewport, texture_st_texture *_pst_surface);
/** Viewport surface get accessor. */
extern texture_st_texture    *viewport_surface_get(viewport_st_viewport *_pst_viewport);

/** Viewport activate accessor. */
extern void                   viewport_activate(viewport_st_viewport *_pst_viewport, bool _b_activation);
/** Viewport is active accessor. */
extern bool                   viewport_active(viewport_st_viewport *_pst_viewport);

/** Viewport clip get accessor. */
extern void                   viewport_clip_get(viewport_st_viewport * _pst_viewport, coord_st_coord *_pst_position, coord_st_coord *_pst_size);

/** Viewport camera set accessor. */
extern void                   viewport_camera_set(viewport_st_viewport *_pst_viewport, camera_st_camera *_pst_camera);
/** Viewport position set accessor. */
extern void                   viewport_position_set(viewport_st_viewport *_pst_viewport, coord_st_coord *_pst_position);
/** Viewport size set accessor. */
extern void                   viewport_size_set(viewport_st_viewport *_pst_viewport, coord_st_coord *_pst_size);

/** Viewport camera get accessor. */
extern camera_st_camera      *viewport_camera_get(viewport_st_viewport *_pst_viewport);
/** Viewport position get accessor. */
extern void                   viewport_position_get(viewport_st_viewport *_pst_viewport, coord_st_coord *_pst_position);
/** Viewport size get accessor. */
extern void                   viewport_size_get(viewport_st_viewport *_pst_viewport, coord_st_coord *_pst_size);

#endif /* _VIEWPORT_H_ */
