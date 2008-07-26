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
 * @addtogroup Sound
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


/** Internal SoundPointer structure */
typedef struct __orxSOUNDPOINTER_t              orxSOUNDPOINTER;


/** SoundPointer module setup
 */
extern orxDLLAPI orxVOID                        orxSoundPointer_Setup();

/** Inits the SoundPointer module
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS                      orxSoundPointer_Init();

/** Exits from the SoundPointer module
 */
extern orxDLLAPI orxVOID                        orxSoundPointer_Exit();


/** Creates an empty SoundPointer
 * @return orxSOUNDPOINTER / orxNULL
 */
extern orxDLLAPI orxSOUNDPOINTER *              orxSoundPointer_Create();

/** Deletes a SoundPointer
 * @param[in] _pstSoundPointer      Concerned SoundPointer
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxSoundPointer_Delete(orxSOUNDPOINTER *_pstSoundPointer);

/** Enables/disables a SoundPointer
 * @param[in]   _pstSoundPointer    Concerned SoundPointer
 * @param[in]   _bEnable        Enable / disable
 */
extern orxDLLAPI orxVOID orxFASTCALL            orxSoundPointer_Enable(orxSOUNDPOINTER *_pstSoundPointer, orxBOOL _bEnable);

/** Is SoundPointer enabled?
 * @param[in]   _pstSoundPointer    Concerned SoundPointer
 * @return      orxTRUE if enabled, orxFALSE otherwise
 */
extern orxDLLAPI orxBOOL orxFASTCALL            orxSoundPointer_IsEnabled(orxCONST orxSOUNDPOINTER *_pstSoundPointer);


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
extern orxDLLAPI orxSTATUS orxFASTCALL          orxSoundPointer_AddSoundFromConfig(orxSOUNDPOINTER *_pstSoundPointer, orxCONST orxSTRING _zSoundConfigID);

/** Removes a sound using using its config ID
 * @param[in]   _pstSoundPointer    Concerned SoundPointer
 * @param[in]   _zSoundConfigID     Config ID of the sound to remove
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxSoundPointer_RemoveSoundFromConfig(orxSOUNDPOINTER *_pstSoundPointer, orxCONST orxSTRING _zSoundConfigID);

/** Gets last added sound (Do *NOT* destroy it directly before removing it!!!)
 * @param[in]   _pstSoundPointer    Concerned SoundPointer
 * @return      orxSOUND / orxNULL
 */
extern orxDLLAPI orxSOUND *orxFASTCALL          orxSoundPointer_GetLastAddedSound(orxCONST orxSOUNDPOINTER *_pstSoundPointer);

#endif /* _orxSOUNDPOINTER_H_ */

/** @} */
