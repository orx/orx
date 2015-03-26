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
 * @param[in]   _pstAnimSet                   AnimSet reference
 * @return      orxANIMPOINTER / orxNULL
 */
extern orxDLLAPI orxANIMPOINTER *orxFASTCALL  orxAnimPointer_Create(orxANIMSET *_pstAnimSet);

/** Creates an animation pointer from config
 * @param[in]   _zConfigID                    Config ID
 * @return      orxANIMPOINTER / orxNULL
 */
extern orxDLLAPI orxANIMPOINTER *orxFASTCALL  orxAnimPointer_CreateFromConfig(const orxSTRING _zConfigID);

/** Deletes an AnimPointer
 * @param[in]   _pstAnimPointer               AnimPointer to delete
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxAnimPointer_Delete(orxANIMPOINTER *_pstAnimPointer);

/** Gets the referenced AnimSet
 * @param[in]   _pstAnimPointer               Concerned AnimPointer
 * @return      Referenced orxANIMSET
 */
extern orxDLLAPI orxANIMSET *orxFASTCALL      orxAnimPointer_GetAnimSet(const orxANIMPOINTER *_pstAnimPointer);


/** AnimPointer current Animation get accessor
 * @param[in]   _pstAnimPointer               Concerned AnimPointer
 * @return      Current Animation ID
 */
extern orxDLLAPI orxU32 orxFASTCALL           orxAnimPointer_GetCurrentAnim(const orxANIMPOINTER *_pstAnimPointer);

/** AnimPointer target Animation get accessor
 * @param[in]   _pstAnimPointer               Concerned AnimPointer
 * @return      Target Animation ID
 */
extern orxDLLAPI orxU32 orxFASTCALL           orxAnimPointer_GetTargetAnim(const orxANIMPOINTER *_pstAnimPointer);

/** AnimPointer current Animation name get accessor
 * @param[in]   _pstAnimPointer               Concerned AnimPointer
 * @return      Current Animation name / orxSTRING_EMPTY
 */
extern orxDLLAPI const orxSTRING orxFASTCALL  orxAnimPointer_GetCurrentAnimName(const orxANIMPOINTER *_pstAnimPointer);

/** AnimPointer target Animation ID get accessor
 * @param[in]   _pstAnimPointer               Concerned AnimPointer
 * @return      Target Animation name / orxSTRING_EMPTY
 */
extern orxDLLAPI const orxSTRING orxFASTCALL  orxAnimPointer_GetTargetAnimName(const orxANIMPOINTER *_pstAnimPointer);

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
 * @param[in]   _u32AnimID                    Animation ID to set
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxAnimPointer_SetCurrentAnim(orxANIMPOINTER *_pstAnimPointer, orxU32 _u32AnimID);

/** AnimPointer target Animation set accessor
 * @param[in]   _pstAnimPointer               Concerned AnimPointer
 * @param[in]   _u32AnimID                    Animation ID to set / orxU32_UNDEFINED
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxAnimPointer_SetTargetAnim(orxANIMPOINTER *_pstAnimPointer, orxU32 _u32AnimID);

/** AnimPointer current Animation set accessor using name
 * @param[in]   _pstAnimPointer               Concerned AnimPointer
 * @param[in]   _zAnimName                    Animation name (config's name) to set
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxAnimPointer_SetCurrentAnimFromName(orxANIMPOINTER *_pstAnimPointer, const orxSTRING _zAnimName);

/** AnimPointer target Animation set accessor using name
 * @param[in]   _pstAnimPointer               Concerned AnimPointer
 * @param[in]   _zAnimName                    Animation name (config's name) to set / orxNULL
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxAnimPointer_SetTargetAnimFromName(orxANIMPOINTER *_pstAnimPointer, const orxSTRING _zAnimName);

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
