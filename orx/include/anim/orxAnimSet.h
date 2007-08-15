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
 * - Optimize the link Anim graph handling & structures.
 * - Clean & simplify internal structures.
 */

/**
 * @addtogroup AnimSet
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


/** AnimSet ID Flags
 */
#define orxANIMSET_KU32_ID_FLAG_NONE                0x00000000  /**< No flags. */

#define orxANIMSET_KU32_ID_FLAG_REFERENCE_LOCK      0x01000000  /**< If there's already a reference on it, the AnimSet is locked for changes. */
#define orxANIMSET_KU32_ID_FLAG_LINK_STATIC         0x10000000  /**< If not static, animpointer should duplicate the link table upon linking/creation. */

/** AnimSet Link Flags
 */
#define orxANIMSET_KU32_LINK_FLAG_NONE              0x00000000  /**< No flags. */

#define orxANIMSET_KU32_LINK_FLAG_LOOP_COUNTER      0x10000000  /**< Anim link uses a counter */
#define orxANIMSET_KU32_LINK_FLAG_PRIORITY          0x20000000  /**< Anim link has priority */

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
extern orxDLLAPI orxVOID                            orxAnimSet_Setup();

/** Inits the AnimSet module
 */
extern orxDLLAPI orxSTATUS                          orxAnimSet_Init();

/** Exits from the AnimSet module
 */
extern orxDLLAPI orxVOID                            orxAnimSet_Exit();


/** Creates an empty AnimSet
 * @param[in]		_u32Size														Storage size
 * return 			Created orxANIMSET / orxNULL
 */
extern orxDLLAPI orxANIMSET *orxFASTCALL            orxAnimSet_Create(orxU32 _u32Size);

/** Deletes an AnimSet
 * @param[in]		_pstAnimSet													AnimSet to delete
 */
extern orxDLLAPI orxSTATUS orxFASTCALL              orxAnimSet_Delete(orxANIMSET *_pstAnimSet);

/** Adds a reference to an AnimSet
 * @param[in]		_pstAnimSet													Concerned AnimSet
 */
extern orxDLLAPI orxVOID orxFASTCALL                orxAnimSet_AddReference(orxANIMSET *_pstAnimSet);

/** Removes a reference from an AnimSet
 * @param[in]		_pstAnimSet													Concerned AnimSet
 */
extern orxDLLAPI orxVOID orxFASTCALL                orxAnimSet_RemoveReference(orxANIMSET *_pstAnimSet);

/** Clones an AnimSet Link Table
 * @param[in]		_pstAnimSet													AnimSet to clone
 * @return An internally allocated clone of the AnimSet
 */
extern orxDLLAPI orxANIMSET_LINK_TABLE *orxFASTCALL orxAnimSet_CloneLinkTable(orxCONST orxANIMSET *_pstAnimSet);

/** Deletes a Link Table
 * @param[in]		_pstAnimSet													AnimSet to delete (should have been created using the clone function)
 */
extern orxDLLAPI orxVOID orxFASTCALL                orxAnimSet_DeleteLinkTable(orxANIMSET_LINK_TABLE *_pstLinkTable);


/** Adds an Anim to an AnimSet
 * @param[in]		_pstAnimSet													Concerned AnimSet
 * @param[in]		_pstAnim														Anim to add
 * @return Anim handle in the specified AnimSet
 */
extern orxDLLAPI orxHANDLE orxFASTCALL              orxAnimSet_AddAnim(orxANIMSET *_pstAnimSet, orxANIM *_pstAnim);

/** Removes an Anim from an AnimSet
 * @param[in]		_pstAnimSet													Concerned AnimSet
 * @param[in]		_hAnimHandle												Handle of the anim to remove
 * @return 			orxSTATUS_SUCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL              orxAnimSet_RemoveAnim(orxANIMSET *_pstAnimSet, orxHANDLE _hAnimHandle);

/** Removes all Anim from the AnimSet
 * @param[in]		_pstAnimSet													Concerned AnimSet
 * @return 			orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL              orxAnimSet_RemoveAllAnims(orxANIMSET *_pstAnimSet);


/** Adds a link between two Anims of the AnimSet
 * @param[in]		_pstAnimSet													Concerned AnimSet
 * @param[in]		_hSrcAnim														Source Anim of the link
 * @param[in]		_hDstAnim														Destination Anim of the link
 * @return 			Handle of the created link / orxHANDLE_Undefined
 */
extern orxDLLAPI orxHANDLE orxFASTCALL              orxAnimSet_AddLink(orxANIMSET *_pstAnimSet, orxHANDLE _hSrcAnim, orxHANDLE _hDstAnim);

/** Removes a link from the AnimSet
 * @param[in]		_pstAnimSet													Concerned AnimSet
 * @param[in]		_hLinkHandle												Handle of the link
 * @return			orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL              orxAnimSet_RemoveLink(orxANIMSET *_pstAnimSet, orxHANDLE _hLinkHandle);

/** Gets a direct link between two Anims, if exists
 * @param[in]		_pstAnimSet													Concerned AnimSet
 * @param[in]		_hSrcAnim														Handle of the source Anim
 * @param[in]		_hDstAnim														Handle of the destination Anim
 * @return			Handle of the direct link, orxHANDLE_Undefined if none
 */
extern orxDLLAPI orxHANDLE orxFASTCALL              orxAnimSet_GetLink(orxCONST orxANIMSET *_pstAnimSet, orxHANDLE _hSrcAnim, orxHANDLE _hDstAnim);

/** Computes all link relations
 * @param[in]		_pstAnimSet													Concerned AnimSet
 * @return			orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL              orxAnimSet_ComputeLinks(orxANIMSET *_pstAnimSet);

/** Sets a link property
 * @param[in]		_pstAnimSet													Concerned AnimSet
 * @param[in]		_hLinkHandle												Handle of the concerned link
 * @param[in]		_u32Property												ID of the property to set
 * @param[in]		_32Value														Value of the property to set
 * @return			orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL              orxAnimSet_SetLinkProperty(orxANIMSET *_pstAnimSet, orxHANDLE _hLinkHandle, orxU32 _u32Property, orxU32 _u32Value);

/** Gets a link property
 * @param[in]		_pstAnimSet													Concerned AnimSet
 * @param[in]		_hLinkHandle 												Handle of the concerned link
 * @param[in]		_u32Property												ID of the property to get
 * @return			Property value / orx32_Undefined
 */
extern orxDLLAPI orxU32 orxFASTCALL                 orxAnimSet_GetLinkProperty(orxCONST orxANIMSET *_pstAnimSet, orxHANDLE _hLinkHandle, orxU32 _u32Property);

/** Computes active Anim given current and destination Anim handles & a relative timestamp.
 * @param[in]		_pstAnimSet    											Concerned AnimSet
 * @param[in]   _hSrcAnim 													Source (current) Anim handle
 * @param[in]   _hDstAnim 													Destination Anim handle, if none (auto mode) set it to orxHANDLE_Undefined
 * @param[in,out] _pu32Time  												Pointer to the current timestamp relative to the source Anim (time elapsed since the beginning of this anim)
 * @param[out]  _pstLinkTable 											Anim Pointer link table (updated if AnimSet link table isn't static, when using loop counters for example)
 * @return Current Anim handle. If it's not the source one, _pu32Time will contain the new timestamp, relative to the new Anim
*/
extern orxDLLAPI orxHANDLE orxFASTCALL              orxAnimSet_ComputeAnim(orxANIMSET *_pstAnimSet, orxHANDLE _hSrcAnim, orxHANDLE _hDstAnim, orxU32 *_pu32Time, orxANIMSET_LINK_TABLE *_pstLinkTable);

/** AnimSet Anim get accessor
 * @param[in]		_pstAnimSet													Concerned AnimSet
 * @param[in]		_hAnimHandle												Anim handle
 * @return Anim pointer / orxNULL
 */
extern orxDLLAPI orxANIM *orxFASTCALL               orxAnimSet_GetAnim(orxCONST orxANIMSET *_pstAnimSet, orxHANDLE _hAnimHandle);

/** AnimSet Anim storage size get accessor
 * @param[in]		_pstAnimSet													Concerned AnimSet
 * @return			AnimSet Storage size / orxU32_Undefined
 */
extern orxDLLAPI orxU32 orxFASTCALL                 orxAnimSet_GetAnimStorageSize(orxCONST orxANIMSET *_pstAnimSet);

/** AnimSet Anim counter get accessor
 * @param[in]		_pstAnimSet													Concerned AnimSet
 * @return			Anim counter / orxU32_Undefined
 */
extern orxDLLAPI orxU32 orxFASTCALL                 orxAnimSet_GetAnimCounter(orxCONST orxANIMSET *_pstAnimSet);


#endif /* _orxANIMSET_H_ */


/** @} */
