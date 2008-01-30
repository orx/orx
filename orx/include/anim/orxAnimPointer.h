/***************************************************************************
 *                                                                         *
 *   This library is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License           *
 *   as published by the Free Software Foundation; either version 2.1      *
 *   of the License, or (at your option) any later version.                *
 *                                                                         *
 ***************************************************************************/

/**
 * @file orxAnimPointer.h
 * @date 03/03/2004
 * @author (C) Arcallians
 * 
 * @todo 
 * C Rewrite with new Graphic/Anim system
 * V Remove dependency to orxTime and use only orxCLOCK_INFO time (for *_set and _create funtions)
 * V Add/Remove "update-all function" clock registering in orxAnimPointer_Init / orxAnimPointer_Exit.
 * V Add current anim, timestamp, dest anim, animset ref, link_table ref in structure.
 * V Duplicate link table on creation if animset is non link-static.
 * V Lock animset & update ref counter upon creation, unlock it upon destruction if updated ref counter is 0.
 * V Optimized animpointer update : will only happen if graphic is displayed on screen.
 * X Add clocks registering (using different frequencies) for an animpointer update.
 * - Add optimization with above system, to compute only if graphic is tagged as rendered.
 */

/**
 * @addtogroup AnimPointer
 * 
 * Animation (Pointer) Module.
 * Allows to creates and handle Animation Set Pointers.
 * It consists of a structure containing pointers and counters referenced to an Animation Set.
 * It also contains functions for handling and accessing animations of the referenced Animation Set.
 * Animation Pointers are structures.
 * They thus can be referenced by Graphics.
 *
 * @{
 */


#ifndef _orxANIMPOINTER_H_
#define _orxANIMPOINTER_H_


#include "orxInclude.h"

#include "anim/orxAnimSet.h"


/** AnimPointer flags
 */
#define orxANIMPOINTER_KU32_FLAG_NONE         0x00000000  /**< No flags */

#define orxANIMPOINTER_KU32_FLAG_PAUSE        0x00010000  /**< Animation id pause flag */


/** Internal AnimPointer structure
 */
typedef struct __orxANIMPOINTER_t             orxANIMPOINTER;


/** AnimPointer module setup
 */
extern orxDLLAPI orxVOID                      orxAnimPointer_Setup();

/** Inits the AnimPointer module
 */
extern orxDLLAPI orxSTATUS                    orxAnimPointer_Init();

/** Exits from the AnimPointer module
 */
extern orxDLLAPI orxVOID                      orxAnimPointer_Exit();


/** Creates an empty AnimPointer
 * @param[in]   _pstAnimSet                   AnimSet reference
 * @return      Created orxANIMPOINTER / orxNULL
 */
extern orxDLLAPI orxANIMPOINTER *orxFASTCALL  orxAnimPointer_Create(orxANIMSET *_pstAnimSet);

/** Deletes an AnimPointer
 * @param[in]   _pstAnimPointer               AnimPointer to delete
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxAnimPointer_Delete(orxANIMPOINTER *_pstAnimPointer);

/** Gets the referenced AnimSet
 * @param[in]   _pstAnimPointer               Concerned AnimPointer
 * @return      Referenced orxANIMSET
 */
extern orxDLLAPI orxANIMSET *orxFASTCALL      orxAnimPointer_GetAnimSet(orxCONST orxANIMPOINTER *_pstAnimPointer);


/** AnimPointer current Animation get accessor
 * @param[in]   _pstAnimPointer               Concerned AnimPointer
 * @return      Current Animation handle
 */
extern orxDLLAPI orxHANDLE orxFASTCALL        orxAnimPointer_GetCurrentAnim(orxCONST orxANIMPOINTER *_pstAnimPointer);

/** AnimPointer current anim data get accessor
 * @param[in]   _pstAnimPointer               Concerned AnimPointer
 * @return      Current anim data / orxNULL
 */
extern orxDLLAPI orxSTRUCTURE *orxFASTCALL    orxAnimPointer_GetCurrentAnimData(orxCONST orxANIMPOINTER *_pstAnimPointer);

/** AnimPointer current Time get accessor
 * @param[in]   _pstAnimPointer               Concerned AnimPointer
 * @return      Current time
 */
extern orxDLLAPI orxFLOAT orxFASTCALL         orxAnimPointer_GetCurrentTime(orxCONST orxANIMPOINTER *_pstAnimPointer);

/** AnimPointer Frequency get accessor
 * @param[in]   _pstAnimPointer               Concerned AnimPointer
 * @return      AnimPointer frequency
 */
extern orxDLLAPI orxFLOAT orxFASTCALL         orxAnimPointer_GetFrequency(orxCONST orxANIMPOINTER *_pstAnimPointer);

/** AnimPointer current Animation set accessor
 * @param[in]   _pstAnimPointer               Concerned AnimPointer
 * @param[in]   _hAnimHandle                  Animation handle to set
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxAnimPointer_SetAnim(orxANIMPOINTER *_pstAnimPointer, orxHANDLE _hAnimHandle);

/** AnimPointer current Time accessor
 * @param[in]   _pstAnimPointer               Concerned AnimPointer
 * @param[in]   _fTime                        Time to set
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxAnimPointer_SetTime(orxANIMPOINTER *_pstAnimPointer, orxFLOAT _fTime);

/** AnimPointer Frequency set accessor
 * @param[in]   _pstAnimPointer               Concerned AnimPointer
 * @param[in]   _fFrequency                   Frequency to set
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxAnimPointer_SetFrequency(orxANIMPOINTER *_pstAnimPointer, orxFLOAT _fFrequency);


#endif /* _orxANIMPOINTER_H_ */


/** @} */
