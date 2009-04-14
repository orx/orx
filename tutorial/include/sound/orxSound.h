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
 * @file orxSound.h
 * @date 13/07/2008
 * @author iarwain@orx-project.org
 *
 * @todo
 */

/**
 * @addtogroup orxSound
 * 
 * Sound module
 * Module that handles sound
 *
 * @{
 */


#ifndef _orxSOUND_H_
#define _orxSOUND_H_


#include "orxInclude.h"
#include "math/orxVector.h"


/** Sound status enum
 */
typedef enum __orxSOUND_STATUS_t
{
  orxSOUND_STATUS_PLAY = 0,
  orxSOUND_STATUS_PAUSE,
  orxSOUND_STATUS_STOP,

  orxSOUND_STATUS_NUMBER,

  orxSOUND_STATUS_NONE = orxENUM_NONE

} orxSOUND_STATUS;


/** Internal Sound structure
 */
typedef struct __orxSOUND_t                   orxSOUND;


/** Event enum
 */
typedef enum __orxSOUND_EVENT_t
{
  orxSOUND_EVENT_START = 0,                   /**< Event sent when a sound starts */
  orxSOUND_EVENT_STOP,                        /**< Event sent when a sound stops */

  orxSOUND_EVENT_NUMBER,

  orxSOUND_EVENT_NONE = orxENUM_NONE

} orxSOUND_EVENT;

/** Sound event payload
 */
typedef struct __orxSOUND_EVENT_PAYLOAD_t
{
  orxSOUND *pstSound;                         /**< Sound reference : 4 */
  orxSTRING zSoundName;                       /**< Sound name : 8 */

} orxSOUND_EVENT_PAYLOAD;


/** Sound module setup
 */
extern orxDLLAPI void orxFASTCALL             orxSound_Setup();

/** Initializes the sound module
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxSound_Init();

/** Exits from the sound module
 */
extern orxDLLAPI void orxFASTCALL             orxSound_Exit();


/** Creates sound from config
 * @param[in]   _zConfigID    Config ID
 * @ return orxSOUND / orxNULL
 */
extern orxDLLAPI orxSOUND *orxFASTCALL        orxSound_CreateFromConfig(const orxSTRING _zConfigID);

/** Deletes sound
 * @param[in] _pstSound       Concerned Sound
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxSound_Delete(orxSOUND *_pstSound);


/** Plays sound
 * @param[in] _pstSound       Concerned Sound
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxSound_Play(orxSOUND *_pstSound);

/** Pauses sound
 * @param[in] _pstSound       Concerned Sound
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxSound_Pause(orxSOUND *_pstSound);

/** Stops sound
 * @param[in] _pstSound       Concerned Sound
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxSound_Stop(orxSOUND *_pstSound);


/** Sets sound volume
 * @param[in] _pstSound       Concerned Sound
 * @param[in] _fVolume        Desired volume (0.0 - 1.0)
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxSound_SetVolume(orxSOUND *_pstSound, orxFLOAT _fVolume);

/** Sets sound pitch
 * @param[in] _pstSound       Concerned Sound
 * @param[in] _fPitch         Desired pitch
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxSound_SetPitch(orxSOUND *_pstSound, orxFLOAT _fPitch);

/** Sets sound position
 * @param[in] _pstSound       Concerned Sound
 * @param[in] _pvPosition     Desired position
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxSound_SetPosition(orxSOUND *_pstSound, const orxVECTOR *_pvPosition);

/** Sets sound attenuation
 * @param[in] _pstSound       Concerned Sound
 * @param[in] _fAttenuation   Desired attenuation
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxSound_SetAttenuation(orxSOUND *_pstSound, orxFLOAT _fAttenuation);

/** Sets sound reference distance
 * @param[in] _pstSound       Concerned Sound
 * @param[in] _fDistance      Within this distance, sound is perceived at its maximum volume
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxSound_SetReferenceDistance(orxSOUND *_pstSound, orxFLOAT _fDistance);

/** Loops sound
 * @param[in] _pstSound       Concerned Sound
 * @param[in] _bLoop          orxTRUE / orxFALSE
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxSound_Loop(orxSOUND *_pstSound, orxBOOL _bLoop);


/** Gets sound volume
 * @param[in] _pstSound       Concerned Sound
 * @return orxFLOAT
 */
extern orxDLLAPI orxFLOAT orxFASTCALL         orxSound_GetVolume(const orxSOUND *_pstSound);

/** Gets sound pitch
 * @param[in] _pstSound       Concerned Sound
 * @return orxFLOAT
 */
extern orxDLLAPI orxFLOAT orxFASTCALL         orxSound_GetPitch(const orxSOUND *_pstSound);

/** Gets sound position
 * @param[in]  _pstSound      Concerned Sound
 * @param[out] _pvPosition    Sound's position
 * @return orxVECTOR / orxNULL
 */
extern orxDLLAPI orxVECTOR *orxFASTCALL       orxSound_GetPosition(const orxSOUND *_pstSound, orxVECTOR *_pvPosition);

/** Gets sound attenuation
 * @param[in] _pstSound       Concerned Sound
 * @return orxFLOAT
 */
extern orxDLLAPI orxFLOAT orxFASTCALL         orxSound_GetAttenuation(const orxSOUND *_pstSound);

/** Gets sound reference distance
 * @param[in] _pstSound       Concerned Sound
 * @return orxFLOAT
 */
extern orxDLLAPI orxFLOAT orxFASTCALL         orxSound_GetReferenceDistance(const orxSOUND *_pstSound);

/** Is sound looping?
 * @param[in] _pstSound       Concerned Sound
 * @return orxTRUE / orxFALSE
 */
extern orxDLLAPI orxBOOL orxFASTCALL          orxSound_IsLooping(const orxSOUND *_pstSound);


/** Gets sound duration
 * @param[in] _pstSound       Concerned Sound
 * @return orxFLOAT
 */
extern orxDLLAPI orxFLOAT orxFASTCALL         orxSound_GetDuration(const orxSOUND *_pstSound);

/** Gets sound status
 * @param[in] _pstSound       Concerned Sound
 * @return orxSOUND_STATUS
 */
extern orxDLLAPI orxSOUND_STATUS orxFASTCALL  orxSound_GetStatus(const orxSOUND *_pstSound);

/** Gets sound config name
 * @param[in]   _pstSound     Concerned sound
 * @return      orxSTRING / orxSTRING_EMPTY
 */
extern orxDLLAPI const orxSTRING orxFASTCALL  orxSound_GetName(const orxSOUND *_pstSound);

#endif /*_orxSOUND_H_*/

/** @} */
