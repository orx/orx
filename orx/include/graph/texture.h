/** 
 * \file texture.h
 * 
 * Texture Module.
 * Allows to creates and handle textures.
 * Textures are 2D structures used by objects.
 * They thus can be referenced by other structures.
 * 
 * \todo
 * Add structure reference counter test before deletion.
 */


/***************************************************************************
 textures.h
 Texture module
 
 begin                : 07/12/2003
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


#ifndef _TEXTURE_H_
#define _TEXTURE_H_

#include "include.h"

#include "math/coord.h"


/** Internal texture structure. */
typedef struct st_texture_t texture_st_texture;


/** Inits the texture system. */
extern uint32               texture_init();
/** Exits from the texture system. */
extern void                 texture_exit();

/** Creates an empty texture. */
extern texture_st_texture  *texture_create();
/** Creates a texture from a bitmap (Warning : use a different bitmap for each texture). */
extern texture_st_texture  *texture_create_from_bitmap(graph_st_bitmap *_pst_bitmap);
/** Deletes a texture (Warning : it deletes referenced bitmap too!). */
extern void                 texture_delete(texture_st_texture *_pst_texture);

/** Links a bitmap to a texture (Warning : use a different bitmap for each texture). */
extern void                 texture_bitmap_link(texture_st_texture *_pst_texture, graph_st_bitmap *_pst_bitmap);
/** Unlinks a bitmap from a texture (Warning : it deletes it). */
extern void                 texture_bitmap_unlink(texture_st_texture *_pst_texture);

/** !!! Warning : Texture accessors don't parameter validity !!! */

/** Corresponding bitmap get accessor. */
extern graph_st_bitmap     *texture_bitmap_get(texture_st_texture *_pst_texture);

/** Texture size get accessor. */
extern void                 texture_size_get(texture_st_texture *_pst_texture, coord_st_coord *_pst_coord);

/** Reference coordinates set accessor (used for rendering purpose). */
extern void                 texture_ref_coord_set(texture_st_texture *_pst_texture, coord_st_coord *_pst_coord);
/** Reference coordinates get accessor (used for rendering purpose). */
extern void                 texture_ref_coord_get(texture_st_texture *_pst_texture, coord_st_coord *_pst_coord);

#endif /* _TEXTURE_H_ */
