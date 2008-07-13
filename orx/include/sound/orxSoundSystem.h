/***************************************************************************
 *                                                                         *
 *   This library is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License           *
 *   as published by the Free Software Foundation; either version 2.1      *
 *   of the License, or (at your option) any later version.                *
 *                                                                         *
 ***************************************************************************/

/**
 * @file orxConfig.h
 * @date 23/02/2002
 * @author (C) Arcallians
 */

/**
 * @addtogroup Sound
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

/** Abstract sound structure
 */
typedef struct __orxSOUNDSYSTEM_SOUND_t orxSOUNDSYSTEM_SOUND;

/** Sound status enum
 */
typedef enum __orxSOUNDSYSTEM_STATUS_t
{
  orxSOUNDSYSTEM_STATUS_PLAY = 0,
  orxSOUNDSYSTEM_STATUS_PAUSE,
  orxSOUNDSYSTEM_STATUS_STOP,

  orxSOUNDSYSTEM_STATUS_NUMBER,

  orxSOUNDSYSTEM_STATUS_NONE = orxENUM_NONE

} orxSOUNDSYSTEM_STATUS;

/***************************************************************************
 * Functions directly implemented by orx core
 ***************************************************************************/

/** Sound system module setup
 */
extern orxDLLAPI orxVOID                              orxSoundSystem_Setup();


/***************************************************************************
 * Functions extended by plugins
 ***************************************************************************/

orxPLUGIN_DECLARE_CORE_FUNCTION(orxSoundSystem_Init, orxSTATUS);
orxPLUGIN_DECLARE_CORE_FUNCTION(orxSoundSystem_Exit, orxVOID);
orxPLUGIN_DECLARE_CORE_FUNCTION(orxSoundSystem_LoadSample, orxSOUNDSYSTEM_SOUND *, orxCONST orxSTRING);
orxPLUGIN_DECLARE_CORE_FUNCTION(orxSoundSystem_LoadStream, orxSOUNDSYSTEM_SOUND *, orxCONST orxSTRING);
orxPLUGIN_DECLARE_CORE_FUNCTION(orxSoundSystem_Unload, orxVOID, orxSOUNDSYSTEM_SOUND *);
orxPLUGIN_DECLARE_CORE_FUNCTION(orxSoundSystem_Play, orxSTATUS, orxSOUNDSYSTEM_SOUND *);
orxPLUGIN_DECLARE_CORE_FUNCTION(orxSoundSystem_Pause, orxSTATUS, orxSOUNDSYSTEM_SOUND *);
orxPLUGIN_DECLARE_CORE_FUNCTION(orxSoundSystem_Stop, orxSTATUS, orxSOUNDSYSTEM_SOUND *);
orxPLUGIN_DECLARE_CORE_FUNCTION(orxSoundSystem_SetVolume, orxSTATUS, orxSOUNDSYSTEM_SOUND *, orxFLOAT);
orxPLUGIN_DECLARE_CORE_FUNCTION(orxSoundSystem_SetPitch, orxSTATUS, orxSOUNDSYSTEM_SOUND *, orxFLOAT);
orxPLUGIN_DECLARE_CORE_FUNCTION(orxSoundSystem_SetPosition, orxSTATUS, orxSOUNDSYSTEM_SOUND *, orxCONST orxVECTOR *);
orxPLUGIN_DECLARE_CORE_FUNCTION(orxSoundSystem_Loop, orxSTATUS, orxSOUNDSYSTEM_SOUND *, orxBOOL);
orxPLUGIN_DECLARE_CORE_FUNCTION(orxSoundSystem_GetVolume, orxFLOAT, orxCONST orxSOUNDSYSTEM_SOUND *);
orxPLUGIN_DECLARE_CORE_FUNCTION(orxSoundSystem_GetPitch, orxFLOAT, orxCONST orxSOUNDSYSTEM_SOUND *);
orxPLUGIN_DECLARE_CORE_FUNCTION(orxSoundSystem_GetPosition, orxVECTOR *, orxCONST orxSOUNDSYSTEM_SOUND *, orxVECTOR *);
orxPLUGIN_DECLARE_CORE_FUNCTION(orxSoundSystem_IsLooping, orxBOOL, orxCONST orxSOUNDSYSTEM_SOUND *);
orxPLUGIN_DECLARE_CORE_FUNCTION(orxSoundSystem_GetDuration, orxFLOAT, orxCONST orxSOUNDSYSTEM_SOUND *);
orxPLUGIN_DECLARE_CORE_FUNCTION(orxSoundSystem_GetStatus, orxSOUNDSYSTEM_STATUS, orxCONST orxSOUNDSYSTEM_SOUND *);


orxSTATIC orxINLINE orxSTATUS orxSoundSystem_Init()
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxSoundSystem_Init)();
}

orxSTATIC orxINLINE orxVOID orxSoundSystem_Exit()
{
  orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxSoundSystem_Exit)();
}

orxSTATIC orxINLINE orxSOUNDSYSTEM_SOUND *orxSoundSystem_LoadSample(orxCONST orxSTRING _zFilename)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxSoundSystem_LoadSample)(_zFilename);
}

orxSTATIC orxINLINE orxSOUNDSYSTEM_SOUND *orxSoundSystem_LoadStream(orxCONST orxSTRING _zFilename)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxSoundSystem_LoadStream)(_zFilename);
}

orxSTATIC orxINLINE orxVOID orxSoundSystem_Unload(orxSOUNDSYSTEM_SOUND *_pstSound)
{
  orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxSoundSystem_Unload)(_pstSound);
}

orxSTATIC orxINLINE orxSTATUS orxSoundSystem_Play(orxSOUNDSYSTEM_SOUND *_pstSound)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxSoundSystem_Play)(_pstSound);
}

orxSTATIC orxINLINE orxSTATUS orxSoundSystem_Pause(orxSOUNDSYSTEM_SOUND *_pstSound)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxSoundSystem_Pause)(_pstSound);
}

orxSTATIC orxINLINE orxSTATUS orxSoundSystem_Stop(orxSOUNDSYSTEM_SOUND *_pstSound)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxSoundSystem_Stop)(_pstSound);
}

orxSTATIC orxINLINE orxSTATUS orxSoundSystem_SetVolume(orxSOUNDSYSTEM_SOUND *_pstSound, orxFLOAT _fVolume)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxSoundSystem_SetVolume)(_pstSound, _fVolume);
}

orxSTATIC orxINLINE orxSTATUS orxSoundSystem_SetPitch(orxSOUNDSYSTEM_SOUND *_pstSound, orxFLOAT _fPitch)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxSoundSystem_SetPitch)(_pstSound, _fPitch);
}

orxSTATIC orxINLINE orxSTATUS orxSoundSystem_SetPosition(orxSOUNDSYSTEM_SOUND *_pstSound, orxCONST orxVECTOR *_pvPosition)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxSoundSystem_SetPosition)(_pstSound, _pvPosition);
}

orxSTATIC orxINLINE orxSTATUS orxSoundSystem_Loop(orxSOUNDSYSTEM_SOUND *_pstSound, orxBOOL _bLoop)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxSoundSystem_Loop)(_pstSound, _bLoop);
}

orxSTATIC orxINLINE orxFLOAT orxSoundSystem_GetVolume(orxCONST orxSOUNDSYSTEM_SOUND *_pstSound)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxSoundSystem_GetVolume)(_pstSound);
}

orxSTATIC orxINLINE orxFLOAT orxSoundSystem_GetPitch(orxCONST orxSOUNDSYSTEM_SOUND *_pstSound)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxSoundSystem_GetPitch)(_pstSound);
}

orxSTATIC orxINLINE orxVECTOR *orxSoundSystem_GetPosition(orxCONST orxSOUNDSYSTEM_SOUND *_pstSound, orxVECTOR *_pvPosition)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxSoundSystem_GetPosition)(_pstSound, _pvPosition);
}

orxSTATIC orxINLINE orxBOOL orxSoundSystem_IsLooping(orxCONST orxSOUNDSYSTEM_SOUND *_pstSound)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxSoundSystem_IsLooping)(_pstSound);
}

orxSTATIC orxINLINE orxFLOAT orxSoundSystem_GetDuration(orxCONST orxSOUNDSYSTEM_SOUND *_pstSound)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxSoundSystem_GetDuration)(_pstSound);
}

orxSTATIC orxINLINE orxSOUNDSYSTEM_STATUS orxSoundSystem_GetStatus(orxCONST orxSOUNDSYSTEM_SOUND *_pstSound)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxSoundSystem_GetStatus)(_pstSound);
}


#endif /* _orxSOUNDSYSTEM_H_ */

/** @} */
