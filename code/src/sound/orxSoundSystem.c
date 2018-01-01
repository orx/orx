/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2018 Orx-Project
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
 * @file orxSoundSystem.c
 * @date 23/07/2002
 * @author iarwain@orx-project.org
 *
 */


#include "sound/orxSoundSystem.h"


/** Sound system module setup
 */
void orxFASTCALL orxSoundSystem_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_SOUNDSYSTEM, orxMODULE_ID_PLUGIN);
  orxModule_AddDependency(orxMODULE_ID_SOUNDSYSTEM, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_SOUNDSYSTEM, orxMODULE_ID_BANK);
  orxModule_AddDependency(orxMODULE_ID_SOUNDSYSTEM, orxMODULE_ID_CLOCK);
  orxModule_AddDependency(orxMODULE_ID_SOUNDSYSTEM, orxMODULE_ID_THREAD);

  /* Done! */
  return;
}


/***************************************************************************
 * Plugin related                                                          *
 ***************************************************************************/

/* *** Core function definitions *** */

orxPLUGIN_DEFINE_CORE_FUNCTION(orxSoundSystem_Init, orxSTATUS, void);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxSoundSystem_Exit, void, void);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxSoundSystem_CreateSample, orxSOUNDSYSTEM_SAMPLE *, orxU32, orxU32, orxU32);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxSoundSystem_LoadSample, orxSOUNDSYSTEM_SAMPLE *, const orxSTRING);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxSoundSystem_DeleteSample, orxSTATUS, orxSOUNDSYSTEM_SAMPLE *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxSoundSystem_GetSampleInfo, orxSTATUS, const orxSOUNDSYSTEM_SAMPLE *, orxU32 *, orxU32 *, orxU32 *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxSoundSystem_SetSampleData, orxSTATUS, orxSOUNDSYSTEM_SAMPLE *, const orxS16 *, orxU32);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxSoundSystem_CreateFromSample, orxSOUNDSYSTEM_SOUND *, const orxSOUNDSYSTEM_SAMPLE *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxSoundSystem_CreateStream, orxSOUNDSYSTEM_SOUND *, orxU32, orxU32, const orxSTRING);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxSoundSystem_CreateStreamFromFile, orxSOUNDSYSTEM_SOUND *, const orxSTRING, const orxSTRING);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxSoundSystem_Delete, orxSTATUS, orxSOUNDSYSTEM_SOUND *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxSoundSystem_Play, orxSTATUS, orxSOUNDSYSTEM_SOUND *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxSoundSystem_Pause, orxSTATUS, orxSOUNDSYSTEM_SOUND *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxSoundSystem_Stop, orxSTATUS, orxSOUNDSYSTEM_SOUND *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxSoundSystem_StartRecording, orxSTATUS, const orxSTRING, orxBOOL, orxU32, orxU32);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxSoundSystem_StopRecording, orxSTATUS, void);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxSoundSystem_HasRecordingSupport, orxBOOL, void);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxSoundSystem_SetVolume, orxSTATUS, orxSOUNDSYSTEM_SOUND *, orxFLOAT);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxSoundSystem_SetPitch, orxSTATUS, orxSOUNDSYSTEM_SOUND *, orxFLOAT);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxSoundSystem_SetCursor, orxSTATUS, orxSOUNDSYSTEM_SOUND *, orxFLOAT);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxSoundSystem_SetPosition, orxSTATUS, orxSOUNDSYSTEM_SOUND *, const orxVECTOR *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxSoundSystem_SetAttenuation, orxSTATUS, orxSOUNDSYSTEM_SOUND *, orxFLOAT);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxSoundSystem_SetReferenceDistance, orxSTATUS, orxSOUNDSYSTEM_SOUND *, orxFLOAT);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxSoundSystem_Loop, orxSTATUS, orxSOUNDSYSTEM_SOUND *, orxBOOL);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxSoundSystem_GetVolume, orxFLOAT, const orxSOUNDSYSTEM_SOUND *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxSoundSystem_GetPitch, orxFLOAT, const orxSOUNDSYSTEM_SOUND *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxSoundSystem_GetCursor, orxFLOAT, const orxSOUNDSYSTEM_SOUND *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxSoundSystem_GetPosition, orxVECTOR *, const orxSOUNDSYSTEM_SOUND *, orxVECTOR *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxSoundSystem_GetAttenuation, orxFLOAT, const orxSOUNDSYSTEM_SOUND *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxSoundSystem_GetReferenceDistance, orxFLOAT, const orxSOUNDSYSTEM_SOUND *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxSoundSystem_IsLooping, orxBOOL, const orxSOUNDSYSTEM_SOUND *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxSoundSystem_GetDuration, orxFLOAT, const orxSOUNDSYSTEM_SOUND *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxSoundSystem_GetStatus, orxSOUNDSYSTEM_STATUS, const orxSOUNDSYSTEM_SOUND *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxSoundSystem_SetGlobalVolume, orxSTATUS, orxFLOAT);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxSoundSystem_GetGlobalVolume, orxFLOAT, void);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxSoundSystem_SetListenerPosition, orxSTATUS, const orxVECTOR *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxSoundSystem_GetListenerPosition, orxVECTOR *, orxVECTOR *);


/* *** Core function info array *** */

orxPLUGIN_BEGIN_CORE_FUNCTION_ARRAY(SOUNDSYSTEM)

orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(SOUNDSYSTEM, INIT, orxSoundSystem_Init)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(SOUNDSYSTEM, EXIT, orxSoundSystem_Exit)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(SOUNDSYSTEM, CREATE_SAMPLE, orxSoundSystem_CreateSample)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(SOUNDSYSTEM, LOAD_SAMPLE, orxSoundSystem_LoadSample)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(SOUNDSYSTEM, DELETE_SAMPLE, orxSoundSystem_DeleteSample)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(SOUNDSYSTEM, GET_SAMPLE_INFO, orxSoundSystem_GetSampleInfo)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(SOUNDSYSTEM, SET_SAMPLE_DATA, orxSoundSystem_SetSampleData)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(SOUNDSYSTEM, CREATE_FROM_SAMPLE, orxSoundSystem_CreateFromSample)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(SOUNDSYSTEM, CREATE_STREAM, orxSoundSystem_CreateStream)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(SOUNDSYSTEM, CREATE_STREAM_FROM_FILE, orxSoundSystem_CreateStreamFromFile)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(SOUNDSYSTEM, DELETE, orxSoundSystem_Delete)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(SOUNDSYSTEM, PLAY, orxSoundSystem_Play)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(SOUNDSYSTEM, PAUSE, orxSoundSystem_Pause)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(SOUNDSYSTEM, STOP, orxSoundSystem_Stop)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(SOUNDSYSTEM, START_RECORDING, orxSoundSystem_StartRecording)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(SOUNDSYSTEM, STOP_RECORDING, orxSoundSystem_StopRecording)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(SOUNDSYSTEM, HAS_RECORDING_SUPPORT, orxSoundSystem_HasRecordingSupport)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(SOUNDSYSTEM, SET_VOLUME, orxSoundSystem_SetVolume)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(SOUNDSYSTEM, SET_PITCH, orxSoundSystem_SetPitch)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(SOUNDSYSTEM, SET_CURSOR, orxSoundSystem_SetCursor)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(SOUNDSYSTEM, SET_POSITION, orxSoundSystem_SetPosition)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(SOUNDSYSTEM, SET_ATTENUATION, orxSoundSystem_SetAttenuation)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(SOUNDSYSTEM, SET_REFERENCE_DISTANCE, orxSoundSystem_SetReferenceDistance)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(SOUNDSYSTEM, LOOP, orxSoundSystem_Loop)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(SOUNDSYSTEM, GET_VOLUME, orxSoundSystem_GetVolume)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(SOUNDSYSTEM, GET_PITCH, orxSoundSystem_GetPitch)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(SOUNDSYSTEM, GET_CURSOR, orxSoundSystem_GetCursor)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(SOUNDSYSTEM, GET_POSITION, orxSoundSystem_GetPosition)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(SOUNDSYSTEM, GET_ATTENUATION, orxSoundSystem_GetAttenuation)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(SOUNDSYSTEM, GET_REFERENCE_DISTANCE, orxSoundSystem_GetReferenceDistance)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(SOUNDSYSTEM, IS_LOOPING, orxSoundSystem_IsLooping)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(SOUNDSYSTEM, GET_DURATION, orxSoundSystem_GetDuration)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(SOUNDSYSTEM, GET_STATUS, orxSoundSystem_GetStatus)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(SOUNDSYSTEM, SET_GLOBAL_VOLUME, orxSoundSystem_SetGlobalVolume)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(SOUNDSYSTEM, GET_GLOBAL_VOLUME, orxSoundSystem_GetGlobalVolume)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(SOUNDSYSTEM, SET_LISTENER_POSITION, orxSoundSystem_SetListenerPosition)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(SOUNDSYSTEM, GET_LISTENER_POSITION, orxSoundSystem_GetListenerPosition)

orxPLUGIN_END_CORE_FUNCTION_ARRAY(SOUNDSYSTEM)


/* *** Core function implementations *** */

orxSTATUS orxFASTCALL orxSoundSystem_Init()
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxSoundSystem_Init)();
}

void orxFASTCALL orxSoundSystem_Exit()
{
  orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxSoundSystem_Exit)();
}

orxSOUNDSYSTEM_SAMPLE *orxFASTCALL orxSoundSystem_CreateSample(orxU32 _u32ChannelNumber, orxU32 _u32FrameNumber, orxU32 _u32SampleRate)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxSoundSystem_CreateSample)(_u32ChannelNumber, _u32FrameNumber, _u32SampleRate);
}

orxSOUNDSYSTEM_SAMPLE *orxFASTCALL orxSoundSystem_LoadSample(const orxSTRING _zFilename)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxSoundSystem_LoadSample)(_zFilename);
}

orxSTATUS orxFASTCALL orxSoundSystem_DeleteSample(orxSOUNDSYSTEM_SAMPLE *_pstSample)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxSoundSystem_DeleteSample)(_pstSample);
}

orxSTATUS orxFASTCALL orxSoundSystem_GetSampleInfo(const orxSOUNDSYSTEM_SAMPLE *_pstSample, orxU32 *_pu32ChannelNumber, orxU32 *_pu32FrameNumber, orxU32 *_pu32SampleRate)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxSoundSystem_GetSampleInfo)(_pstSample, _pu32ChannelNumber, _pu32FrameNumber, _pu32SampleRate);
}

orxSTATUS orxFASTCALL orxSoundSystem_SetSampleData(orxSOUNDSYSTEM_SAMPLE *_pstSample, const orxS16 *_as16Data, orxU32 _u32SampleNumber)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxSoundSystem_SetSampleData)(_pstSample, _as16Data, _u32SampleNumber);
}

orxSOUNDSYSTEM_SOUND *orxFASTCALL orxSoundSystem_CreateFromSample(const orxSOUNDSYSTEM_SAMPLE *_pstSample)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxSoundSystem_CreateFromSample)(_pstSample);
}

orxSOUNDSYSTEM_SOUND *orxFASTCALL orxSoundSystem_CreateStream(orxU32 _u32ChannelNumber, orxU32 _u32SampleRate, const orxSTRING _zReference)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxSoundSystem_CreateStream)(_u32ChannelNumber, _u32SampleRate, _zReference);
}

orxSOUNDSYSTEM_SOUND *orxFASTCALL orxSoundSystem_CreateStreamFromFile(const orxSTRING _zFilename, const orxSTRING _zReference)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxSoundSystem_CreateStreamFromFile)(_zFilename, _zReference);
}

orxSTATUS orxFASTCALL orxSoundSystem_Delete(orxSOUNDSYSTEM_SOUND *_pstSound)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxSoundSystem_Delete)(_pstSound);
}

orxSTATUS orxFASTCALL orxSoundSystem_Play(orxSOUNDSYSTEM_SOUND *_pstSound)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxSoundSystem_Play)(_pstSound);
}

orxSTATUS orxFASTCALL orxSoundSystem_Pause(orxSOUNDSYSTEM_SOUND *_pstSound)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxSoundSystem_Pause)(_pstSound);
}

orxSTATUS orxFASTCALL orxSoundSystem_Stop(orxSOUNDSYSTEM_SOUND *_pstSound)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxSoundSystem_Stop)(_pstSound);
}

orxSTATUS orxFASTCALL orxSoundSystem_StartRecording(const orxSTRING _zName, orxBOOL _bWriteToFile, orxU32 _u32SampleRate, orxU32 _u32ChannelNumber)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxSoundSystem_StartRecording)(_zName, _bWriteToFile, _u32SampleRate, _u32ChannelNumber);
}

orxSTATUS orxFASTCALL orxSoundSystem_StopRecording()
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxSoundSystem_StopRecording)();
}

orxBOOL orxFASTCALL orxSoundSystem_HasRecordingSupport()
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxSoundSystem_HasRecordingSupport)();
}

orxSTATUS orxFASTCALL orxSoundSystem_SetVolume(orxSOUNDSYSTEM_SOUND *_pstSound, orxFLOAT _fVolume)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxSoundSystem_SetVolume)(_pstSound, _fVolume);
}

orxSTATUS orxFASTCALL orxSoundSystem_SetPitch(orxSOUNDSYSTEM_SOUND *_pstSound, orxFLOAT _fPitch)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxSoundSystem_SetPitch)(_pstSound, _fPitch);
}

orxSTATUS orxFASTCALL orxSoundSystem_SetCursor(orxSOUNDSYSTEM_SOUND *_pstSound, orxFLOAT _fCursor)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxSoundSystem_SetCursor)(_pstSound, _fCursor);
}

orxSTATUS orxFASTCALL orxSoundSystem_SetPosition(orxSOUNDSYSTEM_SOUND *_pstSound, const orxVECTOR *_pvPosition)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxSoundSystem_SetPosition)(_pstSound, _pvPosition);
}

orxSTATUS orxFASTCALL orxSoundSystem_SetAttenuation(orxSOUNDSYSTEM_SOUND *_pstSound, orxFLOAT _fAttenuation)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxSoundSystem_SetAttenuation)(_pstSound, _fAttenuation);
}

orxSTATUS orxFASTCALL orxSoundSystem_SetReferenceDistance(orxSOUNDSYSTEM_SOUND *_pstSound, orxFLOAT _fDistance)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxSoundSystem_SetReferenceDistance)(_pstSound, _fDistance);
}

orxSTATUS orxFASTCALL orxSoundSystem_Loop(orxSOUNDSYSTEM_SOUND *_pstSound, orxBOOL _bLoop)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxSoundSystem_Loop)(_pstSound, _bLoop);
}

orxFLOAT orxFASTCALL orxSoundSystem_GetVolume(const orxSOUNDSYSTEM_SOUND *_pstSound)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxSoundSystem_GetVolume)(_pstSound);
}

orxFLOAT orxFASTCALL orxSoundSystem_GetPitch(const orxSOUNDSYSTEM_SOUND *_pstSound)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxSoundSystem_GetPitch)(_pstSound);
}

orxFLOAT orxFASTCALL orxSoundSystem_GetCursor(const orxSOUNDSYSTEM_SOUND *_pstSound)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxSoundSystem_GetCursor)(_pstSound);
}

orxVECTOR *orxFASTCALL orxSoundSystem_GetPosition(const orxSOUNDSYSTEM_SOUND *_pstSound, orxVECTOR *_pvPosition)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxSoundSystem_GetPosition)(_pstSound, _pvPosition);
}

orxFLOAT orxFASTCALL orxSoundSystem_GetAttenuation(const orxSOUNDSYSTEM_SOUND *_pstSound)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxSoundSystem_GetAttenuation)(_pstSound);
}

orxFLOAT orxFASTCALL orxSoundSystem_GetReferenceDistance(const orxSOUNDSYSTEM_SOUND *_pstSound)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxSoundSystem_GetReferenceDistance)(_pstSound);
}

orxBOOL orxFASTCALL orxSoundSystem_IsLooping(const orxSOUNDSYSTEM_SOUND *_pstSound)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxSoundSystem_IsLooping)(_pstSound);
}

orxFLOAT orxFASTCALL orxSoundSystem_GetDuration(const orxSOUNDSYSTEM_SOUND *_pstSound)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxSoundSystem_GetDuration)(_pstSound);
}

orxSOUNDSYSTEM_STATUS orxFASTCALL orxSoundSystem_GetStatus(const orxSOUNDSYSTEM_SOUND *_pstSound)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxSoundSystem_GetStatus)(_pstSound);
}

orxSTATUS orxFASTCALL orxSoundSystem_SetGlobalVolume(orxFLOAT _fVolume)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxSoundSystem_SetGlobalVolume)(_fVolume);
}

orxFLOAT orxFASTCALL orxSoundSystem_GetGlobalVolume()
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxSoundSystem_GetGlobalVolume)();
}

orxSTATUS orxFASTCALL orxSoundSystem_SetListenerPosition(const orxVECTOR *_pvPosition)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxSoundSystem_SetListenerPosition)(_pvPosition);
}

orxVECTOR *orxFASTCALL orxSoundSystem_GetListenerPosition(orxVECTOR *_pvPosition)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxSoundSystem_GetListenerPosition)(_pvPosition);
}
