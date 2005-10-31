/***************************************************************************
 *                                                                         *
 *   This library is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License           *
 *   as published by the Free Software Foundation; either version 2.1      *
 *   of the License, or (at your option) any later version.                *
 *                                                                         *
 ***************************************************************************/

/**
 * @file orxAnimSet.h
 * @date 13/02/2004
 * @author (C) Arcallians
 * 
 * @todo 
 * - Rewrite with new Graphic/Anim system
 * - Optimize the link animation graph handling & structures.
 * - Clean & simplify internal structures.
 */

/**
 * @addtogroup AnimSet
 * 
 * Animation (Set) Module.
 * Allows to creates and handle Sets of Animations.
 * It consists of a structure containing animations and their relations.
 * It also contains functions for handling and accessing them.
 * Animation Sets are structures.
 * They thus can be referenced by Animation Pointers.
 *
 * @{
 */


#ifndef _orxANIMSET_H_
#define _orxANIMSET_H_


#include "orxInclude.h"

#include "anim/orxAnim.h"


/** AnimSet ID Flags
 */
#define orxANIMSET_KU32_ID_FLAG_NONE                0x00000000  /**< No flags. */

#define orxANIMSET_KU32_ID_FLAG_REFERENCE_LOCK      0x01000000  /**< If there's already a reference on it, the AnimSet is locked for changes. */
#define orxANIMSET_KU32_ID_FLAG_LINK_STATIC         0x10000000  /**< If not static, animpointer should duplicate the link table upon linking/creation. */

/** AnimSet Link Flags
 */
#define orxANIMSET_KU32_LINK_FLAG_NONE              0x00000000  /**< No flags. */

#define orxANIMSET_KU32_LINK_FLAG_LOOP_COUNTER      0x10000000  /**< Animation link uses a counter */
#define orxANIMSET_KU32_LINK_FLAG_PRIORITY          0x20000000  /**< Animation link has priority */

/** AnimSet defines
 */
#define orxANIMSET_KU32_MAX_ANIM_NUMBER             128         /**< Maximum number of animations for an animation set structure */


/** Internal AnimSet structure
 */
typedef struct __orxANIMSET_t                       orxANIMSET;

/** Internal Link Table structure
 */
typedef struct __orxANIMSET_LINK_TABLE_t            orxANIMSET_LINK_TABLE;


/** AnimSet module setup
 */
extern orxDLLAPI orxVOID                            orxAnimSet_Setup();

/** Inits the AnimSet module
 */
extern orxDLLAPI orxSTATUS                          orxAnimSet_Init();

/** Exits from the AnimSet module
 */
extern orxDLLAPI orxVOID                            orxAnimSet_Exit();


/** Creates an empty AnimSet, given a storage size (<= orxANIMSET_KU32_MAX_ANIM_NUMBER). */
extern orxDLLAPI orxANIMSET *orxFASTCALL            orxAnimSet_Create(orxU32 _u32Size);
/** Deletes an AnimSet. */
extern orxDLLAPI orxSTATUS orxFASTCALL              orxAnimSet_Delete(orxANIMSET *_pstAnimSet);

/** Adds a reference on an AnimSet. */
extern orxDLLAPI orxVOID orxFASTCALL                orxAnimSet_AddReference(orxANIMSET *_pstAnimSet);
/** Removes a reference from an AnimSet. */
extern orxDLLAPI orxVOID orxFASTCALL                orxAnimSet_RemoveReference(orxANIMSET *_pstAnimSet);

/** Creates a copy of an AnimSet Link Table. */
extern orxDLLAPI orxANIMSET_LINK_TABLE *orxFASTCALL orxAnimSet_DuplicateLinkTable(orxCONST orxANIMSET *_pstAnimSet);
/** Deletes a Link Table. */
extern orxDLLAPI orxVOID orxFASTCALL                orxAnimSet_DeleteLinkTable(orxANIMSET_LINK_TABLE *_pstLinkTable);

/** Adds an Animation to an AnimSet. */
extern orxDLLAPI orxHANDLE orxFASTCALL              orxAnimSet_AddAnim(orxANIMSET *_pstAnimSet, orxANIM *_pstAnim);
/** Removes an Animation from an AnimSet, given its handle.*/
extern orxDLLAPI orxSTATUS orxFASTCALL              orxAnimSet_RemoveAnim(orxANIMSET *_pstAnimSet, orxHANDLE _hAnimHandle);
/** Remove  all referenced animations from an AnimSet. */
extern orxDLLAPI orxSTATUS orxFASTCALL              orxAnimSet_RemoveAllAnims(orxANIMSET *_pstAnimSet);

/** Adds a link between 2 Animations. */
extern orxDLLAPI orxHANDLE orxFASTCALL              orxAnimSet_AddLink(orxANIMSET *_pstAnimSet, orxHANDLE _hSrcAnim, orxHANDLE _hDstAnim);
/** Removes a link given its handle. */
extern orxDLLAPI orxSTATUS orxFASTCALL              orxAnimSet_RemoveLink(orxANIMSET *_pstAnimSet, orxHANDLE _hLinkHandle);
/** Gets a direct link between two animations (if none, result is orxU32_Undefined). */
extern orxDLLAPI orxHANDLE orxFASTCALL              orxAnimSet_GetLink(orxCONST orxANIMSET *_pstAnimSet, orxHANDLE _hSrcAnim, orxHANDLE _hDstAnim);
/** Computes all link relations. */
extern orxDLLAPI orxSTATUS orxFASTCALL              orxAnimSet_ComputeLinks(orxANIMSET *_pstAnimSet);

/** Sets a link property at the given value. */
extern orxDLLAPI orxSTATUS orxFASTCALL              orxAnimSet_SetLinkProperty(orxANIMSET *_pstAnimSet, orxHANDLE _hLinkHandle, orxU32 _u32Property, orxU32 _u32Value);
/** Gets a link property (orxU32_Undefined if something's wrong). */
extern orxDLLAPI orxU32 orxFASTCALL                 orxAnimSet_GetLinkProperty(orxCONST orxANIMSET *_pstAnimSet, orxHANDLE _hLinkHandle, orxU32 _u32Property);

/** Computes active animation given current and destination Animation handle & a relative timestamp.
 * \param   _pstAnimSet    AnimSet container.
 * \param   _hSrcAnim Source animation (current) handle.
 * \param   _hDstAnim Destination animation handle. If none (auto mode), set to undefined.
 * \param   _pu32Time  Pointer to the current timestamp relative to the source animation (time elapsed since the beginning of this anim) : writable.
 * \param   _pstLinkTable Animation Pointer link table. (Updated if AnimSet link table isn't static, when using loop counters for example.)
 * \return Current animation handle. If it's not the source one, _pu32Time will contain the new timestamp, relative to the new animation.
*/
extern orxDLLAPI orxHANDLE orxFASTCALL              orxAnimSet_ComputeAnim(orxANIMSET *_pstAnimSet, orxHANDLE _hSrcAnim, orxHANDLE _hDstAnim, orxU32 *_pu32Time, orxANIMSET_LINK_TABLE *_pstLinkTable);

/** AnimSet Animation get accessor, given its handle. */
extern orxDLLAPI orxANIM *orxFASTCALL               orxAnimSet_GetAnim(orxCONST orxANIMSET *_pstAnimSet, orxHANDLE _hAnimHandle);

/** AnimSet Animation storage size get accessor. */
extern orxDLLAPI orxU32 orxFASTCALL                 orxAnimSet_GetAnimStorageSize(orxCONST orxANIMSET *_pstAnimSet);
/** AnimSet Animation counter get accessor. */
extern orxDLLAPI orxU32 orxFASTCALL                 orxAnimSet_GetAnimCounter(orxCONST orxANIMSET *_pstAnimSet);

/** AnimSet flag test accessor. */
extern orxDLLAPI orxBOOL orxFASTCALL                orxAnimSet_TestFlags(orxCONST orxANIMSET *_pstAnimSet, orxU32 _u32Flags);
/** AnimSet flag get/set accessor. */
extern orxDLLAPI orxVOID orxFASTCALL                orxAnimSet_SetFlags(orxANIMSET *_pstAnimSet, orxU32 _u32AddFlags, orxU32 _u32RemoveFlags);


#endif /* _orxANIMSET_H_ */


/** @} */
