/**
 * \file orxAnimSet.h
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
 orxAnimSet.h
 Animation Set module
 
 begin                : 13/02/2004
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


#ifndef _orxANIMSET_H_
#define _orxANIMSET_H_

#include "orxInclude.h"

#include "anim/orxAnim.h"


/** AnimationSet ID Flags. */
#define orxANIMSET_KU32_ID_FLAG_NONE            0x00000000  /**< No flags. */

#define orxANIMSET_KU32_ID_FLAG_REFERENCE_LOCK  0x01000000  /**< If there's already a reference on it, the AnimationSet is locked for changes. */
#define orxANIMSET_KU32_ID_FLAG_LINK_STATIC     0x10000000  /**< If not static, animpointer should duplicate the link table upon linking/creation. */

/** AnimationSet Link Flags. */
#define orxANIMSET_KU32_LINK_FLAG_NONE          0x00000000  /**< No flags. */

#define orxANIMSET_KU32_LINK_FLAG_LOOP_COUNTER  0x10000000  /**< Animation link uses a counter */
#define orxANIMSET_KU32_LINK_FLAG_PRIORITY      0x20000000  /**< Animation link has priority */

/** AnimationSet defines. */
#define orxANIMSET_KU32_MAX_ANIM_NUMBER         128         /**< Maximum number of animations for an animation set structure */


/** Internal AnimationSet structure. */
typedef struct __orxANIM_SET_t                  orxANIM_SET;

/** Internal Link Table structure. */
typedef struct __orxANIM_SET_LINK_TABLE_t       orxANIM_SET_LINK_TABLE;

/** Inits the AnimationSet system. */
extern orxSTATUS                                orxAnimSet_Init();
/** Exits from the AnimationSet system. */
extern orxVOID                                  orxAnimSet_Exit();

/** Creates an empty AnimationSet, given a storage size (<= orxANIMSET_KU32_MAX_ANIM_NUMBER). */
extern orxANIM_SET                             *orxAnimSet_Create(orxU32 _u32Size);
/** Deletes an AnimationSet. */
extern orxSTATUS                                orxAnimSet_Delete(orxANIM_SET *_pstAnimset);

/** Adds a reference on an AnimationSet. */
extern orxVOID                                  orxAnimSet_AddReference(orxANIM_SET *_pstAnimset);
/** Removes a reference from an AnimationSet. */
extern orxVOID                                  orxAnimSet_RemoveReference(orxANIM_SET *_pstAnimset);

/** Creates a copy of an AnimationSet Link Table. */
extern orxANIM_SET_LINK_TABLE                  *orxAnimSet_DuplicateLinkTable(orxANIM_SET *_pstAnimset);
/** Deletes a Link Table. */
extern orxVOID                                  orxAnimSet_DeleteLinkTable(orxANIM_SET_LINK_TABLE *_pstLinkTable);

/** Adds an Animation to an AnimationSet. */
extern orxHANDLE                                orxAnimSet_AddAnim(orxANIM_SET *_pstAnimset, orxANIM *_pstAnim);
/** Removes an Animation from an AnimationSet, given its handle.*/
extern orxSTATUS                                orxAnimSet_RemoveAnim(orxANIM_SET *_pstAnimset, orxHANDLE _hAnimHandle);
/** Remove  all referenced animations from an AnimSet. */
extern orxSTATUS                                orxAnimSet_RemoveAllAnims(orxANIM_SET *_pstAnimset);

/** Adds a link between 2 Animations. */
extern orxHANDLE                                orxAnimSet_AddLink(orxANIM_SET *_pstAnimset, orxHANDLE _hSrcAnim, orxHANDLE _hDstAnim);
/** Removes a link given its handle. */
extern orxSTATUS                                orxAnimSet_RemoveLink(orxANIM_SET *_pstAnimset, orxHANDLE _hLinkHandle);
/** Gets a direct link between two animations (if none, result is orxU32_Undefined). */
extern orxHANDLE                                orxAnimSet_GetLink(orxANIM_SET *_pstAnimset, orxHANDLE _hSrcAnim, orxHANDLE _hDstAnim);
/** Computes all link relations. */
extern orxSTATUS                                orxAnimSet_ComputeLinks(orxANIM_SET *_pstAnimset);

/** Sets a link property at the given value. */
extern orxSTATUS                                orxAnimSet_SetLinkProperty(orxANIM_SET *_pstAnimset, orxHANDLE _hLinkHandle, orxU32 _u32Property, orxU32 _u32Value);
/** Gets a link property (orxU32_Undefined if something's wrong). */
extern orxU32                                   orxAnimSet_GetLinkProperty(orxANIM_SET *_pstAnimset, orxHANDLE _hLinkHandle, orxU32 _u32Property);

/** Computes active animation given current and destination Animation handle & a relative timestamp.
 * \param   _pstAnimset    AnimationSet container.
 * \param   _hSrcAnim Source animation (current) handle.
 * \param   _hDstAnim Destination animation handle. If none (auto mode), set to undefined.
 * \param   _pu32Time  Pointer to the current timestamp relative to the source animation (time elapsed since the beginning of this anim) : writable.
 * \param   _pstLinkTable Animation Pointer link table. (Updated if AnimationSet link table isn't static, when using loop counters for example.)
 * \return Current animation handle. If it's not the source one, _pu32Time will contain the new timestamp, relative to the new animation.
*/
extern orxHANDLE                                orxAnimSet_ComputeAnim(orxANIM_SET *_pstAnimset, orxHANDLE _hSrcAnim, orxHANDLE _hDstAnim, orxU32 *_pu32Time, orxANIM_SET_LINK_TABLE *_pstLinkTable);


/** *** AnimationSet accessors *** */


/** AnimationSet Animation get accessor, given its handle. */
extern orxANIM                                 *orxAnimSet_GetAnim(orxANIM_SET *_pstAnimset, orxHANDLE _hAnimHandle);

/** AnimationSet Animation storage size get accessor. */
extern orxU32                                   orxAnimSet_GetAnimStorageSize(orxANIM_SET *_pstAnimset);
/** AnimationSet Animation counter get accessor. */
extern orxU32                                   orxAnimSet_GetAnimCounter(orxANIM_SET *_pstAnimset);

/** AnimationSet flag test accessor. */
extern orxBOOL                                  orxAnimSet_TestFlag(orxANIM_SET *_pstAnimset, orxU32 _u32Flag);
/** AnimationSet flag get/set accessor. */
extern orxVOID                                  orxAnimSet_SetFlag(orxANIM_SET *_pstAnimset, orxU32 _u32AddFlags, orxU32 _u32RemoveFlags);

#endif /* _orxANIMSET_H_ */
