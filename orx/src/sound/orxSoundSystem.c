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
 * @file orxSoundSystem.c
 * @date 23/07/2002
 * @author iarwain@orx-project.org
 *
 * @todo
 */


#include "sound/orxSoundSystem.h"


/** Sound system module setup
 */
orxVOID orxSoundSystem_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_SOUNDSYSTEM, orxMODULE_ID_PLUGIN);
  orxModule_AddDependency(orxMODULE_ID_SOUNDSYSTEM, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_SOUNDSYSTEM, orxMODULE_ID_BANK);

  return;
}


/***************************************************************************
 * Plugin related                                                          *
 ***************************************************************************/

/* *** Core function definitions *** */

orxPLUGIN_DEFINE_CORE_FUNCTION(orxSoundSystem_Init, orxSTATUS, orxVOID);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxSoundSystem_Exit, orxVOID, orxVOID);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxSoundSystem_LoadSample, orxSOUNDSYSTEM_SAMPLE *, orxCONST orxSTRING);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxSoundSystem_UnloadSample, orxVOID, orxSOUNDSYSTEM_SAMPLE *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxSoundSystem_CreateFromSample, orxSOUNDSYSTEM_SOUND *, orxCONST orxSOUNDSYSTEM_SAMPLE *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxSoundSystem_CreateStreamFromFile, orxSOUNDSYSTEM_SOUND *, orxCONST orxSTRING);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxSoundSystem_Delete, orxVOID , orxSOUNDSYSTEM_SOUND *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxSoundSystem_Play, orxSTATUS, orxSOUNDSYSTEM_SOUND *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxSoundSystem_Pause, orxSTATUS, orxSOUNDSYSTEM_SOUND *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxSoundSystem_Stop, orxSTATUS, orxSOUNDSYSTEM_SOUND *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxSoundSystem_SetVolume, orxSTATUS, orxSOUNDSYSTEM_SOUND *, orxFLOAT);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxSoundSystem_SetPitch, orxSTATUS, orxSOUNDSYSTEM_SOUND *, orxFLOAT);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxSoundSystem_SetPosition, orxSTATUS, orxSOUNDSYSTEM_SOUND *, orxCONST orxVECTOR *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxSoundSystem_Loop, orxSTATUS, orxSOUNDSYSTEM_SOUND *, orxBOOL);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxSoundSystem_GetVolume, orxFLOAT, orxCONST orxSOUNDSYSTEM_SOUND *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxSoundSystem_GetPitch, orxFLOAT, orxCONST orxSOUNDSYSTEM_SOUND *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxSoundSystem_GetPosition, orxVECTOR *, orxCONST orxSOUNDSYSTEM_SOUND *, orxVECTOR *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxSoundSystem_IsLooping, orxBOOL, orxCONST orxSOUNDSYSTEM_SOUND *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxSoundSystem_GetDuration, orxFLOAT, orxCONST orxSOUNDSYSTEM_SOUND *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxSoundSystem_GetStatus, orxSOUNDSYSTEM_STATUS, orxCONST orxSOUNDSYSTEM_SOUND *);


/* *** Core function info array *** */

orxPLUGIN_BEGIN_CORE_FUNCTION_ARRAY(SOUNDSYSTEM)

orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(SOUNDSYSTEM, INIT, orxSoundSystem_Init)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(SOUNDSYSTEM, EXIT, orxSoundSystem_Exit)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(SOUNDSYSTEM, LOAD_SAMPLE, orxSoundSystem_LoadSample)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(SOUNDSYSTEM, UNLOAD_SAMPLE, orxSoundSystem_UnloadSample)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(SOUNDSYSTEM, CREATE_FROM_SAMPLE, orxSoundSystem_CreateFromSample)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(SOUNDSYSTEM, CREATE_STREAM_FROM_FILE, orxSoundSystem_CreateStreamFromFile)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(SOUNDSYSTEM, DELETE, orxSoundSystem_Delete)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(SOUNDSYSTEM, PLAY, orxSoundSystem_Play)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(SOUNDSYSTEM, PAUSE, orxSoundSystem_Pause)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(SOUNDSYSTEM, STOP, orxSoundSystem_Stop)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(SOUNDSYSTEM, SET_VOLUME, orxSoundSystem_SetVolume)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(SOUNDSYSTEM, SET_PITCH, orxSoundSystem_SetPitch)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(SOUNDSYSTEM, SET_POSITION, orxSoundSystem_SetPosition)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(SOUNDSYSTEM, LOOP, orxSoundSystem_Loop)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(SOUNDSYSTEM, GET_VOLUME, orxSoundSystem_GetVolume)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(SOUNDSYSTEM, GET_PITCH, orxSoundSystem_GetPitch)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(SOUNDSYSTEM, GET_POSITION, orxSoundSystem_GetPosition)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(SOUNDSYSTEM, IS_LOOPING, orxSoundSystem_IsLooping)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(SOUNDSYSTEM, GET_DURATION, orxSoundSystem_GetDuration)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(SOUNDSYSTEM, GET_STATUS, orxSoundSystem_GetStatus)

orxPLUGIN_END_CORE_FUNCTION_ARRAY(SOUNDSYSTEM)


/* *** Core function implementations *** */

orxSTATUS orxSoundSystem_Init()
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxSoundSystem_Init)();
}

orxVOID orxSoundSystem_Exit()
{
  orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxSoundSystem_Exit)();
}

orxSOUNDSYSTEM_SAMPLE *orxSoundSystem_LoadSample(orxCONST orxSTRING _zFilename)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxSoundSystem_LoadSample)(_zFilename);
}

orxVOID orxSoundSystem_UnloadSample(orxSOUNDSYSTEM_SAMPLE *_pstSample)
{
  orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxSoundSystem_UnloadSample)(_pstSample);
}

orxSOUNDSYSTEM_SOUND *orxSoundSystem_CreateFromSample(orxCONST orxSOUNDSYSTEM_SAMPLE *_pstSample)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxSoundSystem_CreateFromSample)(_pstSample);
}

orxSOUNDSYSTEM_SOUND *orxSoundSystem_CreateStreamFromFile(orxCONST orxSTRING _zFilename)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxSoundSystem_CreateStreamFromFile)(_zFilename);
}

orxVOID orxSoundSystem_Delete(orxSOUNDSYSTEM_SOUND *_pstSound)
{
  orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxSoundSystem_Delete)(_pstSound);
}

orxSTATUS orxSoundSystem_Play(orxSOUNDSYSTEM_SOUND *_pstSound)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxSoundSystem_Play)(_pstSound);
}

orxSTATUS orxSoundSystem_Pause(orxSOUNDSYSTEM_SOUND *_pstSound)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxSoundSystem_Pause)(_pstSound);
}

orxSTATUS orxSoundSystem_Stop(orxSOUNDSYSTEM_SOUND *_pstSound)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxSoundSystem_Stop)(_pstSound);
}

orxSTATUS orxSoundSystem_SetVolume(orxSOUNDSYSTEM_SOUND *_pstSound, orxFLOAT _fVolume)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxSoundSystem_SetVolume)(_pstSound, _fVolume);
}

orxSTATUS orxSoundSystem_SetPitch(orxSOUNDSYSTEM_SOUND *_pstSound, orxFLOAT _fPitch)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxSoundSystem_SetPitch)(_pstSound, _fPitch);
}

orxSTATUS orxSoundSystem_SetPosition(orxSOUNDSYSTEM_SOUND *_pstSound, orxCONST orxVECTOR *_pvPosition)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxSoundSystem_SetPosition)(_pstSound, _pvPosition);
}

orxSTATUS orxSoundSystem_Loop(orxSOUNDSYSTEM_SOUND *_pstSound, orxBOOL _bLoop)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxSoundSystem_Loop)(_pstSound, _bLoop);
}

orxFLOAT orxSoundSystem_GetVolume(orxCONST orxSOUNDSYSTEM_SOUND *_pstSound)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxSoundSystem_GetVolume)(_pstSound);
}

orxFLOAT orxSoundSystem_GetPitch(orxCONST orxSOUNDSYSTEM_SOUND *_pstSound)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxSoundSystem_GetPitch)(_pstSound);
}

orxVECTOR *orxSoundSystem_GetPosition(orxCONST orxSOUNDSYSTEM_SOUND *_pstSound, orxVECTOR *_pvPosition)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxSoundSystem_GetPosition)(_pstSound, _pvPosition);
}

orxBOOL orxSoundSystem_IsLooping(orxCONST orxSOUNDSYSTEM_SOUND *_pstSound)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxSoundSystem_IsLooping)(_pstSound);
}

orxFLOAT orxSoundSystem_GetDuration(orxCONST orxSOUNDSYSTEM_SOUND *_pstSound)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxSoundSystem_GetDuration)(_pstSound);
}

orxSOUNDSYSTEM_STATUS orxSoundSystem_GetStatus(orxCONST orxSOUNDSYSTEM_SOUND *_pstSound)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxSoundSystem_GetStatus)(_pstSound);
}
