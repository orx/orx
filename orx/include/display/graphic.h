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
 * Later on, add a texture cache system for rotated/scaled texture.
 * This cache system should be activated or not by user.
 * Adds 3D system (Not planned yet).
 */


/***************************************************************************
 graphic.h
 Graphic module
 
 begin                : 08/12/2003
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


#ifndef _GRAPHIC_H_
#define _GRAPHIC_H_

#include "orxInclude.h"

#include "display/orxTexture.h"
#include "object/orxStructure.h"


/** Graphic ID Flags. */
#define GRAPHIC_KU32_ID_FLAG_NONE            0x00000000  /**< No flags */
#define GRAPHIC_KU32_ID_FLAG_ANTIALIAS       0x00000010  /**< Antialiasing graphic ID flag */
#define GRAPHIC_KU32_ID_FLAG_2D              0x00000020  /**< 2D type graphic ID flag  */
#define GRAPHIC_KU32_ID_FLAG_ANIM            0x00000040  /**< Graphic has Animation data */
#define GRAPHIC_KU32_ID_FLAG_RENDERED        0x10000000  /**< Graphic is rendered during the current frame */


/** Internal graphic structure. */
typedef struct st_graphic_t graphic_st_graphic;


/** Inits the graphic system. */
extern orxU32                      graphic_init();
/** Exits from the graphic system. */
extern orxVOID                     graphic_exit();

/** Creates an empty graphic. */
extern graphic_st_graphic         *graphic_create();
/** Deletes a graphic. */
extern orxVOID                     graphic_delete(graphic_st_graphic *_pst_graphic);

/** Links a structure to a graphic. */
extern orxVOID                     graphic_struct_link(graphic_st_graphic *_pst_graphic, orxSTRUCTURE *_pstStructure);
/** Unlinks a structure from a graphic, given its structure ID.*/
extern orxVOID                     graphic_struct_unlink(graphic_st_graphic *_pst_graphic, orxSTRUCTURE_ID _eStructureID);

/** Cleans all graphic render status */
extern orxVOID                     graphic_render_status_clean();
/** Test graphic render status (TRUE : clean / orxFALSE : dirty)*/
extern orxBOOL                     graphic_render_status_ok(graphic_st_graphic *_pst_graphic);

/** !!! Warning : Graphic accessors don't test parameter validity !!! */

/** Structure used by a graphic get accessor, given its structure ID. Structure must be cast correctly. */
extern orxSTRUCTURE        *graphic_struct_get(graphic_st_graphic *_pst_graphic, orxSTRUCTURE_ID _eStructureID);

/** Graphic current 2D data get accessor. */
extern orxTEXTURE         *graphic_2d_data_get(graphic_st_graphic *_pst_graphic);

/** Graphic current data size get accessor. */
extern orxVOID                     graphic_2d_size_get(graphic_st_graphic *_pst_graphic, orxVEC *_pst_coord);
/** Graphic current data ref coord get accessor. */
extern orxVOID                     graphic_2d_ref_coord_get(graphic_st_graphic *_pst_graphic, orxVEC *_pst_coord);
/** Graphic maximum size get accessor (used for object bounding boxes). */
extern orxVOID                     graphic_2d_max_size_get(graphic_st_graphic *_pst_graphic, orxVEC *_pst_coord);

/** Graphic flag test accessor. */
extern orxBOOL                     graphic_flag_test(graphic_st_graphic *_pst_graphic, orxU32 _u32Flags);
/** Graphic flag get/set accessor. */
extern orxVOID                     graphic_flag_set(graphic_st_graphic *_pst_graphic, orxU32 _u32AddFlags, orxU32 _u32RemoveFlags);

#endif /* _GRAPHIC_H_ */
