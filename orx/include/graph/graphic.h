/**
 * \file graphic.h
 * 
 * Graphic Module.
 * Allows to creates and handle 2D/3D Graphics.
 * They are used as texture/animation container, with associated properties.
 * 2D Graphics are used by objects.
 * They thus can be referenced by objects as structures.
 * 
 * \todo
 * Adds animation system when it's done.
 * Adds dirty status update with animation.
 * Later on, add a texture cache system for rotated/scaled texture.
 * This cache system should be activated or not by user.
 * Adds 3D system (No planned yet).
 */


/***************************************************************************
 graphic.h
 Graphic module
 
 begin                : 08/12/2003
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


#ifndef _GRAPHIC_H_
#define _GRAPHIC_H_

#include "include.h"

#include "graph/texture.h"
#include "object/structure.h"


/** Graphic ID Flags. */
#define GRAPHIC_KUL_ID_FLAG_NONE            0x00000000  /**< No flags */
#define GRAPHIC_KUL_ID_FLAG_ANTIALIAS       0x00000010  /**< Antialiasing graphic ID flag */
#define GRAPHIC_KUL_ID_FLAG_2D              0x00000020  /**< 2D type graphic ID flag  */
#define GRAPHIC_KUL_ID_FLAG_ANIM            0x00000040  /**< Graphic has Animation data */
#define GRAPHIC_KUL_ID_FLAG_RENDERED        0x10000000  /**< Graphic is rendered during the current frame */


/** Internal graphic structure. */
typedef struct st_graphic_t graphic_st_graphic;


/** Inits the graphic system. */
extern uint32                   graphic_init();
/** Exits from the graphic system. */
extern void                     graphic_exit();

/** Creates an empty graphic. */
extern graphic_st_graphic      *graphic_create();
/** Deletes a graphic. */
extern void                     graphic_delete(graphic_st_graphic *_pst_graphic);

/** Links a structure to a graphic. */
extern void                     graphic_struct_link(graphic_st_graphic *_pst_graphic, structure_st_struct *_pst_struct);
/** Unlinks structures from a graphic, given their structure IDs (IDs can be OR'ed).*/
extern void                     graphic_struct_unlink(graphic_st_graphic *_pst_graphic, uint32 _u32_struct_id);

/** Cleans all graphic render status */
extern void                     graphic_render_status_clean();
/** Test graphic render status (TRUE : clean / FALSE : dirty)*/
extern bool                     graphic_render_status_ok(graphic_st_graphic *_pst_graphic);

/** !!! Warning : Graphic accessors don't test parameter validity !!! */

/** Structure used by a graphic get accessor, given its structure ID. Structure must be cast correctly. */
extern structure_st_struct     *graphic_struct_get(graphic_st_graphic *_pst_graphic, uint32 _u32_struct_id);

/** Graphic current 2D data get accessor. */
extern texture_st_texture      *graphic_2d_data_get(graphic_st_graphic *_pst_graphic);

/** Graphic current data size get accessor. */
extern void                     graphic_2d_size_get(graphic_st_graphic *_pst_graphic, coord_st_coord *_pst_coord);
/** Graphic current data ref coord get accessor. */
extern void                     graphic_2d_ref_coord_get(graphic_st_graphic *_pst_graphic, coord_st_coord *_pst_coord);
/** Graphic maximum size get accessor (used for object bounding boxes). */
extern void                     graphic_2d_max_size_get(graphic_st_graphic *_pst_graphic, coord_st_coord *_pst_coord);

/** Graphic flag test accessor. */
extern bool                     graphic_flag_test(graphic_st_graphic *_pst_graphic, uint32 _u32_flag);
/** Graphic flag get/set accessor. */
extern void                     graphic_flag_set(graphic_st_graphic *_pst_graphic, uint32 _u32_add_flags, uint32 _u32_remove_flags);

#endif /* _GRAPHIC_H_ */
