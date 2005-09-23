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
typedef struct __orxANIMSET_t                   orxANIMSET;

/** Internal Link Table structure. */
typedef struct __orxANIMSET_LINK_TABLE_t        orxANIMSET_LINK_TABLE;


/** AnimSet module setup. */
extern orxDLLAPI orxVOID                        orxAnimSet_Setup();
/** Inits the AnimationSet system. */
extern orxDLLAPI orxSTATUS                      orxAnimSet_Init();
/** Exits from the AnimationSet system. */
extern orxDLLAPI orxVOID                        orxAnimSet_Exit();

/** Creates an empty AnimationSet, given a storage size (<= orxANIMSET_KU32_MAX_ANIM_NUMBER). */
extern orxDLLAPI orxANIMSET *                   orxAnimSet_Create(orxU32 _u32Size);
/** Deletes an AnimationSet. */
extern orxDLLAPI orxSTATUS                      orxAnimSet_Delete(orxANIMSET *_pstAnimset);

/** Adds a reference on an AnimationSet. */
extern orxDLLAPI orxVOID                        orxAnimSet_AddReference(orxANIMSET *_pstAnimset);
/** Removes a reference from an AnimationSet. */
extern orxDLLAPI orxVOID                        orxAnimSet_RemoveReference(orxANIMSET *_pstAnimset);

/** Creates a copy of an AnimationSet Link Table. */
extern orxDLLAPI orxANIMSET_LINK_TABLE *        orxAnimSet_DuplicateLinkTable(orxANIMSET *_pstAnimset);
/** Deletes a Link Table. */
extern orxDLLAPI orxVOID                        orxAnimSet_DeleteLinkTable(orxANIMSET_LINK_TABLE *_pstLinkTable);

/** Adds an Animation to an AnimationSet. */
extern orxDLLAPI orxHANDLE                      orxAnimSet_AddAnim(orxANIMSET *_pstAnimset, orxANIM *_pstAnim);
/** Removes an Animation from an AnimationSet, given its handle.*/
extern orxDLLAPI orxSTATUS                      orxAnimSet_RemoveAnim(orxANIMSET *_pstAnimset, orxHANDLE _hAnimHandle);
/** Remove  all referenced animations from an AnimSet. */
extern orxDLLAPI orxSTATUS                      orxAnimSet_RemoveAllAnims(orxANIMSET *_pstAnimset);

/** Adds a link between 2 Animations. */
extern orxDLLAPI orxHANDLE                      orxAnimSet_AddLink(orxANIMSET *_pstAnimset, orxHANDLE _hSrcAnim, orxHANDLE _hDstAnim);
/** Removes a link given its handle. */
extern orxDLLAPI orxSTATUS                      orxAnimSet_RemoveLink(orxANIMSET *_pstAnimset, orxHANDLE _hLinkHandle);
/** Gets a direct link between two animations (if none, result is orxU32_Undefined). */
extern orxDLLAPI orxHANDLE                      orxAnimSet_GetLink(orxANIMSET *_pstAnimset, orxHANDLE _hSrcAnim, orxHANDLE _hDstAnim);
/** Computes all link relations. */
extern orxDLLAPI orxSTATUS                      orxAnimSet_ComputeLinks(orxANIMSET *_pstAnimset);

/** Sets a link property at the given value. */
extern orxDLLAPI orxSTATUS                      orxAnimSet_SetLinkProperty(orxANIMSET *_pstAnimset, orxHANDLE _hLinkHandle, orxU32 _u32Property, orxU32 _u32Value);
/** Gets a link property (orxU32_Undefined if something's wrong). */
extern orxDLLAPI orxU32                         orxAnimSet_GetLinkProperty(orxANIMSET *_pstAnimset, orxHANDLE _hLinkHandle, orxU32 _u32Property);

/** Computes active animation given current and destination Animation handle & a relative timestamp.
 * \param   _pstAnimset    AnimationSet container.
 * \param   _hSrcAnim Source animation (current) handle.
 * \param   _hDstAnim Destination animation handle. If none (auto mode), set to undefined.
 * \param   _pu32Time  Pointer to the current timestamp relative to the source animation (time elapsed since the beginning of this anim) : writable.
 * \param   _pstLinkTable Animation Pointer link table. (Updated if AnimationSet link table isn't static, when using loop counters for example.)
 * \return Current animation handle. If it's not the source one, _pu32Time will contain the new timestamp, relative to the new animation.
*/
extern orxDLLAPI orxHANDLE                      orxAnimSet_ComputeAnim(orxANIMSET *_pstAnimset, orxHANDLE _hSrcAnim, orxHANDLE _hDstAnim, orxU32 *_pu32Time, orxANIMSET_LINK_TABLE *_pstLinkTable);


/** *** AnimationSet accessors *** */


/** AnimationSet Animation get accessor, given its handle. */
extern orxDLLAPI orxANIM *                      orxAnimSet_GetAnim(orxANIMSET *_pstAnimset, orxHANDLE _hAnimHandle);

/** AnimationSet Animation storage size get accessor. */
extern orxDLLAPI orxU32                         orxAnimSet_GetAnimStorageSize(orxANIMSET *_pstAnimset);
/** AnimationSet Animation counter get accessor. */
extern orxDLLAPI orxU32                         orxAnimSet_GetAnimCounter(orxANIMSET *_pstAnimset);

/** AnimationSet flag test accessor. */
extern orxDLLAPI orxBOOL                        orxAnimSet_TestFlags(orxANIMSET *_pstAnimset, orxU32 _u32Flag);
/** AnimationSet flag get/set accessor. */
extern orxDLLAPI orxVOID                        orxAnimSet_SetFlags(orxANIMSET *_pstAnimset, orxU32 _u32AddFlags, orxU32 _u32RemoveFlags);


#endif /* _orxANIMSET_H_ */
