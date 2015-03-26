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
 * @return orxSOUNDPOINTER / orxNULL
 */
extern orxDLLAPI orxSOUNDPOINTER *orxFASTCALL   orxSoundPointer_Create();

/** Deletes a SoundPointer
 * @param[in] _pstSoundPointer      Concerned SoundPointer
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxSoundPointer_Delete(orxSOUNDPOINTER *_pstSoundPointer);

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


/** Sets volume of all related sounds
 * @param[in] _pstSoundPointer      Concerned SoundPointer
 * @param[in] _fVolume        Desired volume (0.0 - 1.0)
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxSoundPointer_SetVolume(orxSOUNDPOINTER *_pstSoundPointer, orxFLOAT _fVolume);

/** Sets pitch of all related sounds
 * @param[in] _pstSoundPointer      Concerned SoundPointer
 * @param[in] _fPitch         Desired pitch
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxSoundPointer_SetPitch(orxSOUNDPOINTER *_pstSoundPointer, orxFLOAT _fPitch);


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

/** Removes all sounds
 * @param[in]   _pstSoundPointer    Concerned SoundPointer
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxSoundPointer_RemoveAllSounds(orxSOUNDPOINTER *_pstSoundPointer);

/** Adds a sound using its config ID
 * @param[in]   _pstSoundPointer    Concerned SoundPointer
 * @param[in]   _zSoundConfigID     Config ID of the sound to add
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxSoundPointer_AddSoundFromConfig(orxSOUNDPOINTER *_pstSoundPointer, const orxSTRING _zSoundConfigID);

/** Removes a sound using its config ID
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
