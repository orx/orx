/***************************************************************************
 *                                                                         *
 *   This library is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License           *
 *   as published by the Free Software Foundation; either version 2.1      *
 *   of the License, or (at your option) any later version.                *
 *                                                                         *
 ***************************************************************************/

/**
 * @file orxSound.h
 * @date 13/07/2008
 * @author (C) Arcallians
 */

/**
 * @addtogroup Sound
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


/** Sound module setup
 */
extern orxDLLAPI orxVOID                      orxSound_Setup();

/** Initializes the sound module
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS                    orxSound_Init();

/** Exits from the sound module
 */
extern orxDLLAPI orxVOID                      orxSound_Exit();


/** Creates sound from config
 * @param[in]   _zConfigID    Config ID
 * @ return orxSOUND / orxNULL
 */
extern orxDLLAPI orxSOUND *orxFASTCALL        orxSound_CreateFromConfig(orxCONST orxSTRING _zConfigID);

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
extern orxDLLAPI orxSTATUS orxFASTCALL        orxSound_SetPosition(orxSOUND *_pstSound, orxCONST orxVECTOR *_pvPosition);

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
extern orxDLLAPI orxFLOAT orxFASTCALL         orxSound_GetVolume(orxCONST orxSOUND *_pstSound);

/** Gets sound pitch
 * @param[in] _pstSound       Concerned Sound
 * @return orxFLOAT
 */
extern orxDLLAPI orxFLOAT orxFASTCALL         orxSound_GetPitch(orxCONST orxSOUND *_pstSound);

/** Gets sound position
 * @param[in] _pstSound       Concerned Sound
 * @param[out] _pstSound      Sound's position
 * @return orxVECTOR / orxNULL
 */
extern orxDLLAPI orxVECTOR *orxFASTCALL       orxSound_GetPosition(orxCONST orxSOUND *_pstSound, orxVECTOR *_pvPosition);

/** Is sound looping?
 * @param[in] _pstSound       Concerned Sound
 * @return orxTRUE / orxFALSE
 */
extern orxDLLAPI orxBOOL orxFASTCALL          orxSound_IsLooping(orxCONST orxSOUND *_pstSound);


/** Gets sound duration
 * @param[in] _pstSound       Concerned Sound
 * @return orxFLOAT
 */
extern orxDLLAPI orxFLOAT orxFASTCALL         orxSound_GetDuration(orxCONST orxSOUND *_pstSound);

/** Gets sound status
 * @param[in] _pstSound       Concerned Sound
 * @return orxSOUND_STATUS
 */
extern orxDLLAPI orxSOUND_STATUS orxFASTCALL  orxSound_GetStatus(orxCONST orxSOUND *_pstSound);

/** Tests sound config ID against given one
 * @param[in]   _pstSound     Concerned sound
 * @param[in]   _zConfigID    Config ID to test
 * @return      orxTRUE if it's sound one, orxFALSE otherwise
 */
extern orxDLLAPI orxBOOL orxFASTCALL          orxSound_IsConfigID(orxCONST orxSOUND *_pstSound, orxCONST orxSTRING _zConfigID);

#endif /*_orxSOUND_H_*/

/** @} */
