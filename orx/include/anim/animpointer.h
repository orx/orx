/**
 * \file animpointer.h
 * 
 * Animation (Pointer) Module.
 * Allows to creates and handle Animation Set Pointers.
 * It consists of a structure containing pointers and counters referenced to an Animation Set.
 * It also contains functions for handling and accessing animations of the referenced Animation Set.
 * Animation Pointers are structures.
 * They thus can be referenced by Graphics.
 * 
 * \todo
 * Everything :)
 * X Add/Remove "update-all function" clock registering in animpointer_init / animpointer_exit.
 * V Add current anim, timestamp, dest anim, animset ref, link_table ref in structure.
 * V Duplicate link table on creation if animset is non link-static.
 * V Lock animset & update ref counter upon creation, unlock it upon destruction if updated ref counter is 0.
 * V Optimized aimpointer update : will only happen if graphic is displayed on screen.
 * - Add clocks registering (using different frequencies) for an animpointer update.
 * - Add optimization with above system, to compute only if graphic is tagged as rendered.
 */


/***************************************************************************
 animpointer.h
 Animation Pointer module
 
 begin                : 03/03/2004
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


#ifndef _ANIMPOINTER_H_
#define _ANIMPOINTER_H_

#include "include.h"

#include "anim/animset.h"


/** AnimationPointer Link Flags. */
#define ANIMPOINTER_KUL_ID_FLAG_PAUSE           0x00010000  /**< Animation link has priority */

/** AnimationPointer defines. */
#define ANIMPOINTER_KI_ANIM_MAX_NUMBER          64          /**< Maximum number of animations for an animation set structure */


/** Internal AnimationPointer structure. */
typedef struct st_animpointer_t animpointer_st_animpointer;


/** Inits the AnimationPointer system. */
extern uint32                       animpointer_init();
/** Exits from the AnimationPointer system. */
extern void                         animpointer_exit();

/** Creates an empty AnimationPointer, given its AnimationSet reference. */
extern animpointer_st_animpointer  *animpointer_create(animset_st_animset *_pst_animset);
/** Deletes an AnimationPointer. */
extern void                         animpointer_delete(animpointer_st_animpointer *_pst_animpointer);

/** Gets the referenced Animation Set. */
extern animset_st_animset          *animpointer_animset_get(animpointer_st_animpointer *_pst_animpointer);

/** Computes animation for the given AnimationPointer using the given TimeStamp. */
extern uint32                       animpointer_compute(animpointer_st_animpointer *_pst_animpointer, uint32 _u32_timedt);

/** Updates all AnimationPointer according to the given TimeStamp. */
extern uint32                       animpointer_update_all(uint32 _u32_timedt);


/** !!! Warning : AnimationPointer accessors don't test parameter validity !!! */


/** AnimationPointer current Animation get accessor. */
extern anim_st_anim                *animpointer_anim_get(animpointer_st_animpointer *_pst_animpointer);
/** AnimationPointer current Animation TimeStamp get accessor. */
extern uint32                       animpointer_time_get(animpointer_st_animpointer *_pst_animpointer);

/** AnimationPointer current Animation set accessor. */
extern uint32                       animpointer_anim_set(animpointer_st_animpointer *_pst_animpointer, uint32 _u32_anim_id);
/** AnimationPointer current Animation TimeStamp set accessor. */
extern uint32                       animpointer_time_set(animpointer_st_animpointer *_pst_animpointer, uint32 _u32_timestamp);

/** AnimationPointer Frequency get accessor. */
extern float                        animpointer_frequency_get(animpointer_st_animpointer *_pst_animpointer);
/** AnimationPointer Frequency set accessor. */
extern uint32                       animpointer_frequency_set(animpointer_st_animpointer *_pst_animpointer, float _f_frequency);

/** AnimationPointer flag test accessor. */
extern bool                         animpointer_flag_test(animpointer_st_animpointer *_pst_animpointer, uint32 _u32_flag);
/** AnimationPointer flag get/set accessor. */
extern void                         animpointer_flag_set(animpointer_st_animpointer *_pst_animpointer, uint32 _u32_add_flags, uint32 _u32_remove_flags);

#endif /* _ANIMPOINTER_H_ */
