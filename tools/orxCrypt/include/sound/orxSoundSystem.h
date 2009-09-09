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
 * @file orxSoundSystem.h
 * @date 13/07/2008
 * @author iarwain@orx-project.org
 *
 * @todo
 */

/**
 * @addtogroup orxSoundSystem
 * 
 * Sound system plugin module
 * Plugin module that handles sound system
 *
 * @{
 */


#ifndef _orxSOUNDSYSTEM_H_
#define _orxSOUNDSYSTEM_H_

#include "orxInclude.h"
#include "math/orxVector.h"
#include "plugin/orxPluginCore.h"


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Abstract sound structures
 */
typedef struct __orxSOUNDSYSTEM_SOUND_t   orxSOUNDSYSTEM_SOUND;
typedef struct __orxSOUNDSYSTEM_SAMPLE_t  orxSOUNDSYSTEM_SAMPLE;

/** Sound system status enum
 */
typedef enum __orxSOUNDSYSTEM_STATUS_t
{
  orxSOUNDSYSTEM_STATUS_PLAY = 0,
  orxSOUNDSYSTEM_STATUS_PAUSE,
  orxSOUNDSYSTEM_STATUS_STOP,

  orxSOUNDSYSTEM_STATUS_NUMBER,

  orxSOUNDSYSTEM_STATUS_NONE = orxENUM_NONE

} orxSOUNDSYSTEM_STATUS;


/** Config defines
 */
#define orxSOUNDSYSTEM_KZ_CONFIG_SECTION  "SoundSystem"
#define orxSOUNDSYSTEM_KZ_CONFIG_RATIO    "DimensionRatio"


/***************************************************************************
 * Functions directly implemented by orx core
 ***************************************************************************/

/** Sound system module setup
 */
extern orxDLLAPI void orxFASTCALL                     orxSoundSystem_Setup();


/***************************************************************************
 * Functions extended by plugins
 ***************************************************************************/

/** Inits the sound system module
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxSoundSystem_Init();

/** Exits from the sound system module
 */
extern orxDLLAPI void orxFASTCALL                     orxSoundSystem_Exit();

/** Loads a sound sample from file (cannot be played directly)
 * @param[in]   _zFilename                            Name of the file to load as a sample (completely loaded in memory, useful for sound effects)
 * @return orxSOUNDSYSTEM_SAMPLE / orxNULL
 */
extern orxDLLAPI orxSOUNDSYSTEM_SAMPLE *orxFASTCALL   orxSoundSystem_LoadSample(const orxSTRING _zFilename);

/** Unloads a sound sample
 * @param[in]   _pstSample                            Concerned sample
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxSoundSystem_UnloadSample(orxSOUNDSYSTEM_SAMPLE *_pstSample);

/** Creates a sound from preloaded sample (can be played directly)
 * @param[in]   _pstSample                            Concerned sample
 * @return orxSOUNDSYSTEM_SOUND / orxNULL
 */
extern orxDLLAPI orxSOUNDSYSTEM_SOUND *orxFASTCALL    orxSoundSystem_CreateFromSample(const orxSOUNDSYSTEM_SAMPLE *_pstSample);

/** Creates a streamed sound from file (can be played directly)
 * @param[in]   _zFilename                            Name of the file to load as a stream (won't be completely loaded in memory, useful for musics)
 * @return orxSOUNDSYSTEM_SOUND / orxNULL
 */
extern orxDLLAPI orxSOUNDSYSTEM_SOUND *orxFASTCALL    orxSoundSystem_CreateStreamFromFile(const orxSTRING _zFilename);

/** Deletes a sound
 * @param[in]   _pstSound                             Concerned sound
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxSoundSystem_Delete(orxSOUNDSYSTEM_SOUND *_pstSound);

/** Plays a sound
 * @param[in]   _pstSound                             Concerned sound
 * @return orxSTATUS_SUCCESS / orxSTATSUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxSoundSystem_Play(orxSOUNDSYSTEM_SOUND *_pstSound);

/** Pauses a sound
 * @param[in]   _pstSound                             Concerned sound
 * @return orxSTATUS_SUCCESS / orxSTATSUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxSoundSystem_Pause(orxSOUNDSYSTEM_SOUND *_pstSound);

/** Stops a sound
 * @param[in]   _pstSound                             Concerned sound
 * @return orxSTATUS_SUCCESS / orxSTATSUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxSoundSystem_Stop(orxSOUNDSYSTEM_SOUND *_pstSound);

/** Sets a sound volume
 * @param[in]   _pstSound                             Concerned sound
 * @param[in]   _fVolume                              Volume to set [0, 1]
 * @return orxSTATUS_SUCCESS / orxSTATSUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxSoundSystem_SetVolume(orxSOUNDSYSTEM_SOUND *_pstSound, orxFLOAT _fVolume);

/** Sets a sound pitch
 * @param[in]   _pstSound                             Concerned sound
 * @param[in]   _fPitch                               Pitch to set
 * @return orxSTATUS_SUCCESS / orxSTATSUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxSoundSystem_SetPitch(orxSOUNDSYSTEM_SOUND *_pstSound, orxFLOAT _fPitch);

/** Sets a sound position
 * @param[in]   _pstSound                             Concerned sound
 * @param[in]   _pvPosition                           Position to set
 * @return orxSTATUS_SUCCESS / orxSTATSUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxSoundSystem_SetPosition(orxSOUNDSYSTEM_SOUND *_pstSound, const orxVECTOR *_pvPosition);

/** Sets a sound attenuation
 * @param[in] _pstSound                               Concerned Sound
 * @param[in] _fAttenuation                           Desired attenuation
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxSoundSystem_SetAttenuation(orxSOUNDSYSTEM_SOUND *_pstSound, orxFLOAT _fAttenuation);

/** Sets a sound reference distance
 * @param[in] _pstSound                               Concerned Sound
 * @param[in] _fDistance                              Within this distance, sound is perceived at its maximum volume
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxSoundSystem_SetReferenceDistance(orxSOUNDSYSTEM_SOUND *_pstSound, orxFLOAT _fDistance);

/** Loops a sound
 * @param[in]   _pstSound                             Concerned sound
 * @param[in]   _bLoop                                Loop / no loop
 * @return orxSTATUS_SUCCESS / orxSTATSUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxSoundSystem_Loop(orxSOUNDSYSTEM_SOUND *_pstSound, orxBOOL _bLoop);

/** Gets a sound volume
 * @param[in]   _pstSound                             Concerned sound
 * @return Sound's volume
 */
extern orxDLLAPI orxFLOAT orxFASTCALL                 orxSoundSystem_GetVolume(const orxSOUNDSYSTEM_SOUND *_pstSound);

/** Gets a sound pitch
 * @param[in]   _pstSound                             Concerned sound
 * @return Sound's pitch
 */
extern orxDLLAPI orxFLOAT orxFASTCALL                 orxSoundSystem_GetPitch(const orxSOUNDSYSTEM_SOUND *_pstSound);

/** Gets a sound position
 * @param[in]   _pstSound                             Concerned sound
 * @param[out]  _pvPosition                           Position to get
 * @return Sound's position
 */
extern orxDLLAPI orxVECTOR *orxFASTCALL               orxSoundSystem_GetPosition(const orxSOUNDSYSTEM_SOUND *_pstSound, orxVECTOR *_pvPosition);

/** Gets a sound attenuation
 * @param[in] _pstSound                               Concerned Sound
 * @return Sound's attenuation
 */
extern orxDLLAPI orxFLOAT orxFASTCALL                 orxSoundSystem_GetAttenuation(const orxSOUNDSYSTEM_SOUND *_pstSound);

/** Gets a sound reference distance
 * @param[in] _pstSound                               Concerned Sound
 * @return Sound's reference distance
 */
extern orxDLLAPI orxFLOAT orxFASTCALL                 orxSoundSystem_GetReferenceDistance(const orxSOUNDSYSTEM_SOUND *_pstSound);

/** Is sound looping?
 * @param[in]   _pstSound                             Concerned sound
 * @return orxTRUE if looping, orxFALSE otherwise
 */
extern orxDLLAPI orxBOOL orxFASTCALL                  orxSoundSystem_IsLooping(const orxSOUNDSYSTEM_SOUND *_pstSound);

/** Gets a sound duration
 * @param[in]   _pstSound                             Concerned sound
 * @return Sound's duration (seconds)
 */
extern orxDLLAPI orxFLOAT orxFASTCALL                 orxSoundSystem_GetDuration(const orxSOUNDSYSTEM_SOUND *_pstSound);

/** Gets a sound status (play/pause/stop)
 * @param[in]   _pstSound                             Concerned sound
 * @return orxSOUNDSYSTEM_STATUS
 */
extern orxDLLAPI orxSOUNDSYSTEM_STATUS orxFASTCALL    orxSoundSystem_GetStatus(const orxSOUNDSYSTEM_SOUND *_pstSound);

/** Sets global volume
 * @param[in] _fGlobalVolume                          Global volume to set
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxSoundSystem_SetGlobalVolume(orxFLOAT _fGlobalVolume);

/** Gets global volume
 * @return Gobal volume
 */
extern orxDLLAPI orxFLOAT orxFASTCALL                 orxSoundSystem_GetGlobalVolume();

/** Sets listener position
 * @param[in] _pvPosition                             Desired position
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxSoundSystem_SetListenerPosition(const orxVECTOR *_pvPosition);

/** Gets listener position
 * @param[out] _pvPosition                            Listener's position
 * @return orxVECTOR / orxNULL
 */
extern orxDLLAPI orxVECTOR *orxFASTCALL               orxSoundSystem_GetListenerPosition(orxVECTOR *_pvPosition);

#endif /* _orxSOUNDSYSTEM_H_ */

/** @} */
