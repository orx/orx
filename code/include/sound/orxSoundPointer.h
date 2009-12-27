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
 * @file orxSoundPointer.h
 * @date 20/07/2008
 * @author iarwain@orx-project.org
 *
 * @todo
 */

/**
 * @addtogroup orxSoundPointer
 *
 * Sound pointer module
 * Allows to creates sound containers for objects.
 *
 * @{
 */


#ifndef _orxSOUNDPOINTER_H_
#define _orxSOUNDPOINTER_H_


#include "orxInclude.h"
#include "sound/orxSound.h"
#include "object/orxStructure.h"


/** Misc defines
 */
#define orxSOUNDPOINTER_KU32_SOUND_NUMBER       4


/** Internal SoundPointer structure */
typedef struct __orxSOUNDPOINTER_t              orxSOUNDPOINTER;


/** SoundPointer module setup
 */
extern orxDLLAPI void orxFASTCALL               orxSoundPointer_Setup();

/** Inits the SoundPointer module
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxSoundPointer_Init();

/** Exits from the SoundPointer module
 */
extern orxDLLAPI void orxFASTCALL               orxSoundPointer_Exit();


/** Creates an empty SoundPointer
 * @param[in]   _pstOwner           Sound's owner used for event callbacks (usually an orxOBJECT)
 * @return orxSOUNDPOINTER / orxNULL
 */
extern orxDLLAPI orxSOUNDPOINTER *orxFASTCALL   orxSoundPointer_Create(const orxSTRUCTURE *_pstOwner);

/** Deletes a SoundPointer
 * @param[in] _pstSoundPointer      Concerned SoundPointer
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxSoundPointer_Delete(orxSOUNDPOINTER *_pstSoundPointer);

/** Gets a SoundPointer owner
 * @param[in]   _pstSoundPointer    Concerned SoundPointer
 * @return      orxSTRUCTURE / orxNULL
 */
extern orxDLLAPI orxSTRUCTURE *orxFASTCALL      orxSoundPointer_GetOwner(const orxSOUNDPOINTER *_pstSoundPointer);

/** Enables/disables a SoundPointer
 * @param[in]   _pstSoundPointer    Concerned SoundPointer
 * @param[in]   _bEnable        Enable / disable
 */
extern orxDLLAPI void orxFASTCALL               orxSoundPointer_Enable(orxSOUNDPOINTER *_pstSoundPointer, orxBOOL _bEnable);

/** Is SoundPointer enabled?
 * @param[in]   _pstSoundPointer    Concerned SoundPointer
 * @return      orxTRUE if enabled, orxFALSE otherwise
 */
extern orxDLLAPI orxBOOL orxFASTCALL            orxSoundPointer_IsEnabled(const orxSOUNDPOINTER *_pstSoundPointer);


/** Plays all related sounds
 * @param[in] _pstSoundPointer      Concerned SoundPointer
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxSoundPointer_Play(orxSOUNDPOINTER *_pstSoundPointer);

/** Pauses all related sounds
 * @param[in] _pstSoundPointer      Concerned SoundPointer
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxSoundPointer_Pause(orxSOUNDPOINTER *_pstSoundPointer);

/** Stops all related sounds
 * @param[in] _pstSoundPointer      Concerned SoundPointer
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxSoundPointer_Stop(orxSOUNDPOINTER *_pstSoundPointer);


/** Adds a sound
 * @param[in]   _pstSoundPointer    Concerned SoundPointer
 * @param[in]   _pstSound           Sound to add
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxSoundPointer_AddSound(orxSOUNDPOINTER *_pstSoundPointer, orxSOUND *_pstSound);

/** Removes a sound
 * @param[in]   _pstSoundPointer    Concerned SoundPointer
 * @param[in]   _pstSound           Sound to remove
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxSoundPointer_RemoveSound(orxSOUNDPOINTER *_pstSoundPointer, orxSOUND *_pstSound);

/** Adds a sound using its config ID
 * @param[in]   _pstSoundPointer    Concerned SoundPointer
 * @param[in]   _zSoundConfigID     Config ID of the sound to add
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxSoundPointer_AddSoundFromConfig(orxSOUNDPOINTER *_pstSoundPointer, const orxSTRING _zSoundConfigID);

/** Removes a sound using using its config ID
 * @param[in]   _pstSoundPointer    Concerned SoundPointer
 * @param[in]   _zSoundConfigID     Config ID of the sound to remove
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxSoundPointer_RemoveSoundFromConfig(orxSOUNDPOINTER *_pstSoundPointer, const orxSTRING _zSoundConfigID);

/** Gets last added sound (Do *NOT* destroy it directly before removing it!!!)
 * @param[in]   _pstSoundPointer    Concerned SoundPointer
 * @return      orxSOUND / orxNULL
 */
extern orxDLLAPI orxSOUND *orxFASTCALL          orxSoundPointer_GetLastAddedSound(const orxSOUNDPOINTER *_pstSoundPointer);

#endif /* _orxSOUNDPOINTER_H_ */

/** @} */
