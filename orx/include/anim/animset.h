/**
 * \file animset.h
 * 
 * Animation (Set) Module.
 * Allows to creates and handle Sets of Animations.
 * It consists of a structure containing animations and their relations.
 * It also contains functions for handling and accessing them.
 * Animation Sets are structures.
 * They thus can be referenced by Animation Pointers.
 * 
 * \todo
 * Optimize the link animation graph handling & structures.
 * Clean & simplify internal structures.
 */


/***************************************************************************
 animset.h
 Animation Set module
 
 begin                : 13/02/2004
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


#ifndef _ANIMSET_H_
#define _ANIMSET_H_

#include "include.h"

#include "anim/anim.h"


/** AnimationSet ID Flags. */
#define ANIMSET_KUL_ID_FLAG_REFERENCE_LOCK  0x01000000  /**< If there's already a reference on it, the AnimationSet is locked for changes. */
#define ANIMSET_KUL_ID_FLAG_LINK_STATIC     0x10000000  /**< If not static, animpointer should duplicate the link table upon linking/creation. */

/** AnimationSet Link Flags. */
#define ANIMSET_KUL_LINK_FLAG_LOOP_COUNTER  0x10000000  /**< Animation link uses a counter */
#define ANIMSET_KUL_LINK_FLAG_PRIORITY      0x20000000  /**< Animation link has priority */

/** AnimationSet defines. */
#define ANIMSET_KI_ANIM_MAX_NUMBER          128         /**< Maximum number of animations for an animation set structure */


/** Internal AnimationSet structure. */
typedef struct st_animset_t animset_st_animset;

/** Internal Link Table structure. */
typedef struct st_link_table_t animset_st_link_table;

/** Inits the AnimationSet system. */
extern uint32                 animset_init();
/** Exits from the AnimationSet system. */
extern void                   animset_exit();

/** Creates an empty AnimationSet, given a storage size (<= ANIMSET_KI_ANIM_MAX_NUMBER). */
extern animset_st_animset    *animset_create(uint32 _u32_size);
/** Deletes an AnimationSet. */
extern void                   animset_delete(animset_st_animset *_pst_animset);

/** Adds a reference on an AnimationSet. */
extern void                   animset_reference_add(animset_st_animset *_pst_animset);
/** Removes a reference from an AnimationSet. */
extern void                   animset_reference_remove(animset_st_animset *_pst_animset);

/** Creates a copy of an AnimationSet Link Table. */
extern animset_st_link_table *animset_link_table_duplicate(animset_st_animset *_pst_animset);
/** Deletes a Link Table. */
extern void                   animset_link_table_delete(animset_st_link_table *_pst_link_table);

/** Adds an Animation to an AnimationSet. */
extern uint32                 animset_anim_add(animset_st_animset *_pst_animset, anim_st_anim *_pst_anim);
/** Removes an Animation from an AnimationSet, given its ID.*/
extern uint32                 animset_anim_remove(animset_st_animset *_pst_animset, uint32 _u32_anim_id);
/** Cleans all referenced Animations from an AnimationSet. */
extern uint32                 animset_anim_clean(animset_st_animset *_pst_animset);

/** Adds a link between 2 Animations. */
extern uint32                 animset_link_add(animset_st_animset *_pst_animset, uint32 _u32_anim_src, uint32 _u32_anim_dst);
/** Removes a link given its ID. */
extern uint32                 animset_link_remove(animset_st_animset *_pst_animset, uint32 _u32_link_id);
/** Gets a direct link between two animations (if none, result is KUL_UNDEFINED). */
extern uint32                 animset_link_get(animset_st_animset *_pst_animset, uint32 _u32_anim_src, uint32 _u32_anim_dst);
/** Computes all link relations. */
extern uint32                 animset_link_compute(animset_st_animset *_pst_animset);

/** Sets a link property at the given value. */
extern uint32                 animset_link_property_set(animset_st_animset *_pst_animset, uint32 _u32_link_id, uint32 _u32_property, uint32 _u32_value);
/** Gets a link property (KUL_UNDEFINED if something's wrong). */
extern uint32                 animset_link_property_get(animset_st_animset *_pst_animset, uint32 _u32_link_id, uint32 _u32_property);

/** Computes active animation given current and destination Animation ID & a relative timestamp.
 * \param   _pst_animset    AnimationSet container.
 * \param   _u32_src_anim_id Source animation (current) ID.
 * \param   _u32_dst_anim_id Destination animation ID . If none (auto mode), set to KUL_UNDEFINED.
 * \param   _pu32_timestamp  Pointer to the current timestamp relative to the source animation (time elapsed since the beginning of this anim) : writable.
 * \param   _pst_link_table Animation Pointer link table. (Updated if AnimationSet link table isn't static, when using loop counters for example.)
 * \return Current animation ID. If it's not the source one, _pu32_timestamp will contain the new timestamp, relative to the nez animation.
*/
extern uint32                 animset_anim_compute(animset_st_animset *_pst_animset, uint32 _u32_src_anim_id, uint32 _u32_dst_anim_id, uint32 *_pu32_timestamp, animset_st_link_table *_pst_link_table);


/** !!! Warning : AnimationSet accessors don't test parameter validity !!! */


/** AnimationSet Animation get accessor, given its ID. */
extern anim_st_anim          *animset_anim_get(animset_st_animset *_pst_animset, uint32 _u32_id);

/** AnimationSet Animation storage size get accessor. */
extern uint32                 animset_anim_storage_size_get(animset_st_animset *_pst_animset);
/** AnimationSet Animation counter get accessor. */
extern uint32                 animset_anim_counter_get(animset_st_animset *_pst_animset);

/** AnimationSet flag test accessor. */
extern bool                   animset_flag_test(animset_st_animset *_pst_animset, uint32 _u32_flag);
/** AnimationSet flag get/set accessor. */
extern void                   animset_flag_set(animset_st_animset *_pst_animset, uint32 _u32_add_flags, uint32 _u32_remove_flags);

#endif /* _ANIMSET_H_ */
