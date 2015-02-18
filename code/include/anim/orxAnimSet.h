/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2015 Orx-Project
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 *    1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 *
 *    2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 *    3. This notice may not be removed or altered from any source
 *    distribution.
 */

/**
 * @file orxAnimSet.h
 * @date 13/02/2004
 * @author iarwain@orx-project.org
 *
 * @todo
 */

/**
 * @addtogroup orxAnimSet
 *
 * Animation (Set) Module.
 * Allows to creates and handle Sets of Anims.
 * It consists of a structure containing Anims and their relations.
 * It also contains functions for handling and accessing them.
 * AnimSets are structures.
 * They thus can be referenced by Anim Pointers.
 *
 * @{
 */


#ifndef _orxANIMSET_H_
#define _orxANIMSET_H_


#include "orxInclude.h"

#include "anim/orxAnim.h"


/** AnimSet flags
 */
#define orxANIMSET_KU32_FLAG_NONE                   0x00000000  /**< No flags. */

#define orxANIMSET_KU32_FLAG_REFERENCE_LOCK         0x00100000  /**< If there's already a reference on it, the AnimSet is locked for changes. */
#define orxANIMSET_KU32_FLAG_LINK_STATIC            0x00200000  /**< If not static, animpointer should duplicate the link table upon linking/creation. */

/** AnimSet Link Flags
 */
#define orxANIMSET_KU32_LINK_FLAG_NONE              0x00000000  /**< No flags. */

#define orxANIMSET_KU32_LINK_FLAG_LOOP_COUNTER      0x10000000  /**< Anim link uses a counter */
#define orxANIMSET_KU32_LINK_FLAG_PRIORITY          0x20000000  /**< Anim link has priority */
#define orxANIMSET_KU32_LINK_FLAG_IMMEDIATE_CUT     0x40000000  /**< Anim link should update with an immediate cut */
#define orxANIMSET_KU32_LINK_FLAG_CLEAR_TARGET      0x80000000  /**< Anim link should clear target when used */

/** AnimSet defines
 */
#define orxANIMSET_KU32_MAX_ANIM_NUMBER             128         /**< Maximum number of Anims for an Anim set structure */


/** Internal AnimSet structure
 */
typedef struct __orxANIMSET_t                       orxANIMSET;

/** Internal Link Table structure
 */
typedef struct __orxANIMSET_LINK_TABLE_t            orxANIMSET_LINK_TABLE;


/** AnimSet module setup
 */
extern orxDLLAPI void orxFASTCALL                   orxAnimSet_Setup();

/** Inits the AnimSet module
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL              orxAnimSet_Init();

/** Exits from the AnimSet module
 */
extern orxDLLAPI void orxFASTCALL                   orxAnimSet_Exit();


/** Creates an empty AnimSet
 * @param[in]		_u32Size														Storage size
 * return 			Created orxANIMSET / orxNULL
 */
extern orxDLLAPI orxANIMSET *orxFASTCALL            orxAnimSet_Create(orxU32 _u32Size);

/** Creates an animation set from config
 * @param[in]   _zConfigID                    Config ID
 * @return      orxANIMSET / orxNULL
 */
extern orxDLLAPI orxANIMSET *orxFASTCALL            orxAnimSet_CreateFromConfig(const orxSTRING _zConfigID);

/** Deletes an AnimSet
 * @param[in]		_pstAnimSet													AnimSet to delete
 */
extern orxDLLAPI orxSTATUS orxFASTCALL              orxAnimSet_Delete(orxANIMSET *_pstAnimSet);

/** Adds a reference to an AnimSet
 * @param[in]		_pstAnimSet													Concerned AnimSet
 */
extern orxDLLAPI void orxFASTCALL                   orxAnimSet_AddReference(orxANIMSET *_pstAnimSet);

/** Removes a reference from an AnimSet
 * @param[in]		_pstAnimSet													Concerned AnimSet
 */
extern orxDLLAPI void orxFASTCALL                   orxAnimSet_RemoveReference(orxANIMSET *_pstAnimSet);

/** Clones an AnimSet Link Table
 * @param[in]		_pstAnimSet													AnimSet to clone
 * @return An internally allocated clone of the AnimSet
 */
extern orxDLLAPI orxANIMSET_LINK_TABLE *orxFASTCALL orxAnimSet_CloneLinkTable(const orxANIMSET *_pstAnimSet);

/** Deletes a Link table
 * @param[in]		_pstLinkTable											  Link table to delete (should have been created using the clone function)
 */
extern orxDLLAPI void orxFASTCALL                   orxAnimSet_DeleteLinkTable(orxANIMSET_LINK_TABLE *_pstLinkTable);


/** Adds an Anim to an AnimSet
 * @param[in]		_pstAnimSet													Concerned AnimSet
 * @param[in]		_pstAnim														Anim to add
 * @return Anim ID in the specified AnimSet
 */
extern orxDLLAPI orxU32 orxFASTCALL                 orxAnimSet_AddAnim(orxANIMSET *_pstAnimSet, orxANIM *_pstAnim);

/** Removes an Anim from an AnimSet
 * @param[in]		_pstAnimSet													Concerned AnimSet
 * @param[in]		_u32AnimID												  ID of the anim to remove
 * @return 			orxSTATUS_SUCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL              orxAnimSet_RemoveAnim(orxANIMSET *_pstAnimSet, orxU32 _u32AnimID);

/** Removes all Anim from the AnimSet
 * @param[in]		_pstAnimSet													Concerned AnimSet
 * @return 			orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL              orxAnimSet_RemoveAllAnims(orxANIMSET *_pstAnimSet);


/** Adds a link between two Anims of the AnimSet
 * @param[in]		_pstAnimSet													Concerned AnimSet
 * @param[in]		_u32SrcAnim                         Source Anim of the link
 * @param[in]		_u32DstAnim                         Destination Anim of the link
 * @return 			ID of the created link / orxU32_UNDEFINED
 */
extern orxDLLAPI orxU32 orxFASTCALL                 orxAnimSet_AddLink(orxANIMSET *_pstAnimSet, orxU32 _u32SrcAnim, orxU32 _u32DstAnim);

/** Removes a link from the AnimSet
 * @param[in]		_pstAnimSet													Concerned AnimSet
 * @param[in]		_u32LinkID                          ID of the link
 * @return			orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL              orxAnimSet_RemoveLink(orxANIMSET *_pstAnimSet, orxU32 _u32LinkID);

/** Gets a direct link between two Anims, if exists
 * @param[in]		_pstAnimSet													Concerned AnimSet
 * @param[in]		_u32SrcAnim                         ID of the source Anim
 * @param[in]		_u32DstAnim                         ID of the destination Anim
 * @return			ID of the direct link, orxU32_UNDEFINED if none
 */
extern orxDLLAPI orxU32 orxFASTCALL                 orxAnimSet_GetLink(const orxANIMSET *_pstAnimSet, orxU32 _u32SrcAnim, orxU32 _u32DstAnim);

/** Sets a link property
 * @param[in]		_pstAnimSet													Concerned AnimSet
 * @param[in]		_u32LinkID												  ID of the concerned link
 * @param[in]		_u32Property												ID of the property to set
 * @param[in]		_u32Value                           Value of the property to set
 * @return			orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL              orxAnimSet_SetLinkProperty(orxANIMSET *_pstAnimSet, orxU32 _u32LinkID, orxU32 _u32Property, orxU32 _u32Value);

/** Gets a link property
 * @param[in]		_pstAnimSet													Concerned AnimSet
 * @param[in]		_u32LinkID 												  ID of the concerned link
 * @param[in]		_u32Property												ID of the property to get
 * @return			Property value / orx32_Undefined
 */
extern orxDLLAPI orxU32 orxFASTCALL                 orxAnimSet_GetLinkProperty(const orxANIMSET *_pstAnimSet, orxU32 _u32LinkID, orxU32 _u32Property);

/** Computes active Anim given current and destination Anim IDs & a relative timestamp
 * @param[in]   _pstAnimSet                         Concerned AnimSet
 * @param[in]   _u32SrcAnim                         Source (current) Anim ID
 * @param[in]   _u32DstAnim                         Destination Anim ID, if none (auto mode) set it to orxU32_UNDEFINED
 * @param[in,out] _pfTime                           Pointer to the current timestamp relative to the source Anim (time elapsed since the beginning of this anim)
 * @param[in,out] _pstLinkTable                     Anim Pointer link table (updated if AnimSet link table isn't static, when using loop counters for example)
 * @param[out] _pbCut                               Animation has been cut
 * @param[out] _pbClearTarget                       Animation has requested a target clearing
 * @return Current Anim ID. If it's not the source one, _pu32Time will contain the new timestamp, relative to the new Anim
*/
extern orxDLLAPI orxU32 orxFASTCALL                 orxAnimSet_ComputeAnim(orxANIMSET *_pstAnimSet, orxU32 _u32SrcAnim, orxU32 _u32DstAnim, orxFLOAT *_pfTime, orxANIMSET_LINK_TABLE *_pstLinkTable, orxBOOL *_pbCut, orxBOOL *_pbClearTarget);

/** Finds next Anim given current and destination Anim IDs
 * @param[in]   _pstAnimSet                         Concerned AnimSet
 * @param[in]   _u32SrcAnim                         Source (current) Anim ID
 * @param[in]   _u32DstAnim                         Destination Anim ID, if none (auto mode) set it to orxU32_UNDEFINED
 * @return Next Anim ID if found, orxU32_UNDEFINED otherwise
*/
extern orxDLLAPI orxU32 orxFASTCALL                 orxAnimSet_FindNextAnim(orxANIMSET *_pstAnimSet, orxU32 _u32SrcAnim, orxU32 _u32DstAnim);

/** AnimSet Anim get accessor
 * @param[in]		_pstAnimSet													Concerned AnimSet
 * @param[in]		_u32AnimID												  Anim ID
 * @return Anim pointer / orxNULL
 */
extern orxDLLAPI orxANIM *orxFASTCALL               orxAnimSet_GetAnim(const orxANIMSET *_pstAnimSet, orxU32 _u32AnimID);

/** AnimSet Anim storage size get accessor
 * @param[in]		_pstAnimSet													Concerned AnimSet
 * @return			AnimSet Storage size / orxU32_UNDEFINED
 */
extern orxDLLAPI orxU32 orxFASTCALL                 orxAnimSet_GetAnimStorageSize(const orxANIMSET *_pstAnimSet);

/** AnimSet Anim counter get accessor
 * @param[in]		_pstAnimSet													Concerned AnimSet
 * @return			Anim counter / orxU32_UNDEFINED
 */
extern orxDLLAPI orxU32 orxFASTCALL                 orxAnimSet_GetAnimCounter(const orxANIMSET *_pstAnimSet);

/** Gets animation ID from name
 * @param[in]   _pstAnimSet                         Concerned AnimSet
 * @param[in]   _zAnimName                          Animation name (config's section)
 * @return Anim ID / orxU32_UNDEFINED
 */
extern orxDLLAPI orxU32 orxFASTCALL                 orxAnimSet_GetAnimIDFromName(const orxANIMSET *_pstAnimSet, const orxSTRING _zAnimName);

/** AnimSet name get accessor
 * @param[in]   _pstAnimSet                         Concerned AnimSet
 * @return      orxSTRING / orxSTRING_EMPTY
 */
extern orxDLLAPI const orxSTRING orxFASTCALL        orxAnimSet_GetName(const orxANIMSET *_pstAnimSet);

#endif /* _orxANIMSET_H_ */

/** @} */
