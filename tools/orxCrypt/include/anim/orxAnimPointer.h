/* Orx - Portable Game Engine
 *
 * Orx is the legal property of its developers, whose names
 * are listed in the COPYRIGHT file distributed 
 * with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

/**
 * @file orxAnimPointer.h
 * @date 03/03/2004
 * @author iarwain@orx-project.org
 *
 * @todo
 */

/**
 * @addtogroup orxAnimPointer
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


/** Internal AnimPointer structure
 */
typedef struct __orxANIMPOINTER_t             orxANIMPOINTER;


/** AnimPointer module setup
 */
extern orxDLLAPI void orxFASTCALL             orxAnimPointer_Setup();

/** Inits the AnimPointer module
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxAnimPointer_Init();

/** Exits from the AnimPointer module
 */
extern orxDLLAPI void orxFASTCALL             orxAnimPointer_Exit();


/** Creates an empty AnimPointer
 * @param[in]   _pstOwner                     AnimPointer's owner used for event callbacks (usually an orxOBJECT)
 * @param[in]   _pstAnimSet                   AnimSet reference
 * @return      orxANIMPOINTER / orxNULL
 */
extern orxDLLAPI orxANIMPOINTER *orxFASTCALL  orxAnimPointer_Create(const orxSTRUCTURE *_pstOwner, orxANIMSET *_pstAnimSet);

/** Creates an animation pointer from config
 * @param[in]   _pstOwner                     AnimPointer's owner used for event callbacks (usually an orxOBJECT)
 * @param[in]   _zConfigID                    Config ID
 * @return      orxANIMPOINTER / orxNULL
 */
extern orxDLLAPI orxANIMPOINTER *orxFASTCALL  orxAnimPointer_CreateFromConfig(const orxSTRUCTURE *_pstOwner, const orxSTRING _zConfigID);

/** Deletes an AnimPointer
 * @param[in]   _pstAnimPointer               AnimPointer to delete
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxAnimPointer_Delete(orxANIMPOINTER *_pstAnimPointer);

/** Gets an AnimPointer owner
 * @param[in]   _pstAnimPointer               Concerned AnimPointer
 * @return      orxSTRUCTURE / orxNULL
 */
extern orxDLLAPI orxSTRUCTURE *orxFASTCALL    orxAnimPointer_GetOwner(const orxANIMPOINTER *_pstAnimPointer);

/** Gets the referenced AnimSet
 * @param[in]   _pstAnimPointer               Concerned AnimPointer
 * @return      Referenced orxANIMSET
 */
extern orxDLLAPI orxANIMSET *orxFASTCALL      orxAnimPointer_GetAnimSet(const orxANIMPOINTER *_pstAnimPointer);


/** AnimPointer current Animation get accessor
 * @param[in]   _pstAnimPointer               Concerned AnimPointer
 * @return      Current Animation handle
 */
extern orxDLLAPI orxHANDLE orxFASTCALL        orxAnimPointer_GetCurrentAnimHandle(const orxANIMPOINTER *_pstAnimPointer);

/** AnimPointer target Animation get accessor
 * @param[in]   _pstAnimPointer               Concerned AnimPointer
 * @return      Target Animation handle
 */
extern orxDLLAPI orxHANDLE orxFASTCALL        orxAnimPointer_GetTargetAnimHandle(const orxANIMPOINTER *_pstAnimPointer);

/** AnimPointer current Animation ID get accessor
 * @param[in]   _pstAnimPointer               Concerned AnimPointer
 * @return      Current Animation ID
 */
extern orxDLLAPI orxU32 orxFASTCALL           orxAnimPointer_GetCurrentAnim(const orxANIMPOINTER *_pstAnimPointer);

/** AnimPointer target Animation ID get accessor
 * @param[in]   _pstAnimPointer               Concerned AnimPointer
 * @return      Target Animation ID
 */
extern orxDLLAPI orxU32 orxFASTCALL           orxAnimPointer_GetTargetAnim(const orxANIMPOINTER *_pstAnimPointer);

/** AnimPointer current anim data get accessor
 * @param[in]   _pstAnimPointer               Concerned AnimPointer
 * @return      Current anim data / orxNULL
 */
extern orxDLLAPI orxSTRUCTURE *orxFASTCALL    orxAnimPointer_GetCurrentAnimData(const orxANIMPOINTER *_pstAnimPointer);

/** AnimPointer current Time get accessor
 * @param[in]   _pstAnimPointer               Concerned AnimPointer
 * @return      Current time
 */
extern orxDLLAPI orxFLOAT orxFASTCALL         orxAnimPointer_GetCurrentTime(const orxANIMPOINTER *_pstAnimPointer);

/** AnimPointer Frequency get accessor
 * @param[in]   _pstAnimPointer               Concerned AnimPointer
 * @return      AnimPointer frequency
 */
extern orxDLLAPI orxFLOAT orxFASTCALL         orxAnimPointer_GetFrequency(const orxANIMPOINTER *_pstAnimPointer);

/** AnimPointer current Animation set accessor
 * @param[in]   _pstAnimPointer               Concerned AnimPointer
 * @param[in]   _hAnimHandle                  Animation handle to set
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxAnimPointer_SetCurrentAnimHandle(orxANIMPOINTER *_pstAnimPointer, orxHANDLE _hAnimHandle);

/** AnimPointer target Animation set accessor
 * @param[in]   _pstAnimPointer               Concerned AnimPointer
 * @param[in]   _hAnimHandle                  Animation handle to set / orxHANDLE_UNDEFINED
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxAnimPointer_SetTargetAnimHandle(orxANIMPOINTER *_pstAnimPointer, orxHANDLE _hAnimHandle);

/** AnimPointer current Animation set accessor using ID
 * @param[in]   _pstAnimPointer               Concerned AnimPointer
 * @param[in]   _u32AnimID                    Animation ID (config's name CRC) to set
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxAnimPointer_SetCurrentAnim(orxANIMPOINTER *_pstAnimPointer, orxU32 _u32AnimID);

/** AnimPointer target Animation set accessor using ID
 * @param[in]   _pstAnimPointer               Concerned AnimPointer
 * @param[in]   _u32AnimID                    Animation ID (config's name CRC) to set / orxU32_UNDEFINED
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxAnimPointer_SetTargetAnim(orxANIMPOINTER *_pstAnimPointer, orxU32 _u32AnimID);

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

/** AnimPointer pause accessor
 * @param[in]   _pstAnimPointer               Concerned AnimPointer
 * @param[in]   _bPause                       Pause / Unpause
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxAnimPointer_Pause(orxANIMPOINTER *_pstAnimPointer, orxBOOL _bPause);

#endif /* _orxANIMPOINTER_H_ */

/** @} */
