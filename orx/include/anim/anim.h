/**
 * \file anim.h
 * 
 * Animation (Data) Module.
 * Allows to creates and handle Animations data.
 * It consists of a structure containing data for a single animation
 * and functions for handling and accessing them.
 * Animations are structures.
 * They thus can be referenced by Animation Sets (animset) Module.
 * 
 * \todo
 * Optimizations
 */


/***************************************************************************
 anim.h
 Animation Data module
 
 begin                : 11/02/2004
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


#ifndef _ANIM_H_
#define _ANIM_H_

#include "include.h"

#include "graph/texture.h"
#include "object/structure.h"


/** Animation ID Flags. */
#define ANIM_KUL_ID_FLAG_2D               0x00010000  /**< 2D type animation ID flag */

/** Animation defines. */
#define ANIM_KI_TEXTURE_MAX_NUMBER        256         /**< Maximum number of texture for an animation structure */


/** Internal Animation structure. */
typedef struct st_anim_t anim_st_anim;


/** Inits the Animation system. */
extern uint32               anim_init();
/** Exits from the Animation system. */
extern void                 anim_exit();

/** Creates an empty Animation, given its id type and storage size (<= ANIM_KI_TEXTURE_MAX_NUMBER). */
extern anim_st_anim        *anim_create(uint32 _u32_id_flag, uint32 _u32_size);
/** Deletes an Animation. */
extern void                 anim_delete(anim_st_anim *_pst_anim);

/** Adds a texture to a 2D Animation. */
extern uint32               anim_2d_texture_add(anim_st_anim *_pst_anim, texture_st_texture *_pst_texture, uint32 _u32_timestamp);
/** Removes last added texture from a 2D Animation.*/
extern uint32               anim_2d_texture_remove(anim_st_anim *_pst_anim);
/** Cleans all referenced textures from a 2D Animation. */
extern void                 anim_2d_texture_clean(anim_st_anim *_pst_anim);

/** Computes active 2D texture given a timestamp. */
extern texture_st_texture  *anim_2d_texture_compute(anim_st_anim *_pst_anim, uint32 _u32_timestamp);


/** !!! Warning : Animation accessors don't test parameter validity !!! */


/** Animation 2D texture get accessor, given its index. */
extern texture_st_texture  *anim_2d_texture_get(anim_st_anim *_pst_anim, uint32 _u32_index);

/** Animation 2D texture storage size get accessor. */
extern uint32               anim_2d_texture_storage_size_get(anim_st_anim *_pst_anim);
/** Animation 2D texture counter get accessor. */
extern uint32               anim_2d_texture_counter_get(anim_st_anim *_pst_anim);

/** Animation duration get accessor. */
extern uint32               anim_duration_get(anim_st_anim *_pst_anim);

/** Animation flag test accessor. */
extern bool                 anim_flag_test(anim_st_anim *_pst_anim, uint32 _u32_flag);
/** Animation flag get/set accessor. */
extern void                 anim_flag_set(anim_st_anim *_pst_anim, uint32 _u32_add_flags, uint32 _u32_remove_flags);

#endif /* _ANIM_H_ */
