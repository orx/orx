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

#include "orxInclude.h"

#include "math/orxMath.h"


/** Internal texture structure. */
typedef struct st_texture_t orxTEXTURE;


/** Inits the texture system. */
extern orxU32               texture_init();
/** Exits from the texture system. */
extern orxVOID                 texture_exit();

/** Creates an empty texture. */
extern orxTEXTURE  *texture_create();
/** Creates a texture from a bitmap (Warning : use a different bitmap for each texture). */
extern orxTEXTURE  *texture_create_from_bitmap(graph_st_bitmap *_pstBitmap);
/** Deletes a texture (Warning : it deletes referenced bitmap too!). */
extern orxVOID                 texture_delete(orxTEXTURE *_pstTexture);

/** Links a bitmap to a texture (Warning : use a different bitmap for each texture). */
extern orxVOID                 texture_bitmap_link(orxTEXTURE *_pstTexture, graph_st_bitmap *_pstBitmap);
/** Unlinks a bitmap from a texture (Warning : it deletes it). */
extern orxVOID                 texture_bitmap_unlink(orxTEXTURE *_pstTexture);

/** !!! Warning : Texture accessors don't parameter validity !!! */

/** Corresponding bitmap get accessor. */
extern graph_st_bitmap     *texture_bitmap_get(orxTEXTURE *_pstTexture);

/** Texture size get accessor. */
extern orxVOID                 texture_size_get(orxTEXTURE *_pstTexture, orxVEC *_pst_coord);

/** Reference coordinates set accessor (used for rendering purpose). */
extern orxVOID                 texture_ref_coord_set(orxTEXTURE *_pstTexture, orxVEC *_pst_coord);
/** Reference coordinates get accessor (used for rendering purpose). */
extern orxVOID                 texture_ref_coord_get(orxTEXTURE *_pstTexture, orxVEC *_pst_coord);

#endif /* _TEXTURE_H_ */
