/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2011 Orx-Project
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
 * @date 07/01/2009
 * @author simons.philippe@gmail.com
 *
 * Dummy sound system plugin implementation
 *
 */


#include "orxPluginAPI.h"

orxSTATUS orxFASTCALL orxSoundSystem_Dummy_Init()
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "orxSoundSystem not yet implemented on this platform");
  
  /* Done! */
  return eResult;
}

void orxFASTCALL orxSoundSystem_Dummy_Exit() {
  return;
}

orxSOUNDSYSTEM_SAMPLE *orxFASTCALL orxSoundSystem_Dummy_CreateSample(orxU32 _u32ChannelNumber, orxU32 _u32FrameNumber, orxU32 _u32SampleRate)
{
  return orxNULL;
}

orxSOUNDSYSTEM_SAMPLE *orxFASTCALL orxSoundSystem_Dummy_LoadSample(const orxSTRING _zFilename) 
{
	return orxNULL;
}

orxSTATUS orxFASTCALL orxSoundSystem_Dummy_DeleteSample(orxSOUNDSYSTEM_SAMPLE *_pstSample)
{
	return orxSTATUS_FAILURE;
}

orxSTATUS orxFASTCALL orxSoundSystem_Dummy_GetSampleInfo(const orxSOUNDSYSTEM_SAMPLE *_pstSample, orxU32 *_pu32ChannelNumber, orxU32 *_pu32FrameNumber, orxU32 *_pu32SampleRate)
{
  return orxSTATUS_FAILURE;
}

orxSTATUS orxFASTCALL orxSoundSystem_Dummy_SetSampleData(orxSOUNDSYSTEM_SAMPLE *_pstSample, const orxS16 *_as16Data, orxU32 _u32SampleNumber)
{
  return orxSTATUS_FAILURE;
}

orxSOUNDSYSTEM_SOUND *orxFASTCALL orxSoundSystem_Dummy_CreateFromSample(const orxSOUNDSYSTEM_SAMPLE *_pstSample)
{
	return orxNULL;
}

orxSOUNDSYSTEM_SOUND *orxFASTCALL orxSoundSystem_Dummy_CreateStream(orxU32 _u32ChannelNumber, orxU32 _u32SampleRate, const orxSTRING _zReference)
{
  return orxNULL;
}

orxSOUNDSYSTEM_SOUND *orxFASTCALL orxSoundSystem_Dummy_CreateStreamFromFile(const orxSTRING _zFilename, const orxSTRING _zReference)
{
  return orxNULL;
}

orxSTATUS orxFASTCALL orxSoundSystem_Dummy_Delete(orxSOUNDSYSTEM_SOUND *_pstSound)
{
  return orxSTATUS_FAILURE;
}

orxSTATUS orxFASTCALL orxSoundSystem_Dummy_Play(orxSOUNDSYSTEM_SOUND *_pstSound)
{
	return orxSTATUS_FAILURE;
}

orxSTATUS orxFASTCALL orxSoundSystem_Dummy_Pause(orxSOUNDSYSTEM_SOUND *_pstSound)
{
	return orxSTATUS_FAILURE;
}

orxSTATUS orxFASTCALL orxSoundSystem_Dummy_Stop(orxSOUNDSYSTEM_SOUND *_pstSound)
{
	return orxSTATUS_FAILURE;
}

orxSTATUS orxFASTCALL orxSoundSystem_Dummy_SetVolume(orxSOUNDSYSTEM_SOUND *_pstSound, orxFLOAT _fVolume)
{
	return orxSTATUS_FAILURE;
}

orxSTATUS orxFASTCALL orxSoundSystem_Dummy_SetPitch(orxSOUNDSYSTEM_SOUND *_pstSound, orxFLOAT _fPitch)
{
	return orxSTATUS_FAILURE;
}

orxSTATUS orxFASTCALL orxSoundSystem_Dummy_SetPosition(orxSOUNDSYSTEM_SOUND *_pstSound, const orxVECTOR *_pvPosition)
{
	return orxSTATUS_FAILURE;
}

orxSTATUS orxFASTCALL orxSoundSystem_Dummy_SetAttenuation(orxSOUNDSYSTEM_SOUND *_pstSound, orxFLOAT _fAttenuation)
{
	return orxSTATUS_FAILURE;
}

orxSTATUS orxFASTCALL orxSoundSystem_Dummy_SetReferenceDistance(orxSOUNDSYSTEM_SOUND *_pstSound, orxFLOAT _fDistance)
{
	return orxSTATUS_FAILURE;
}

orxSTATUS orxFASTCALL orxSoundSystem_Dummy_Loop(orxSOUNDSYSTEM_SOUND *_pstSound, orxBOOL _bLoop)
{
	return orxSTATUS_FAILURE;
}

orxFLOAT orxFASTCALL orxSoundSystem_Dummy_GetVolume(const orxSOUNDSYSTEM_SOUND *_pstSound)
{
  return orxFLOAT_0;
}

orxFLOAT orxFASTCALL orxSoundSystem_Dummy_GetPitch(const orxSOUNDSYSTEM_SOUND *_pstSound)
{
  return orxFLOAT_0;
}

orxVECTOR *orxFASTCALL orxSoundSystem_Dummy_GetPosition(const orxSOUNDSYSTEM_SOUND *_pstSound, orxVECTOR *_pvPosition)
{
  return orxNULL;
}

orxFLOAT orxFASTCALL orxSoundSystem_Dummy_GetAttenuation(const orxSOUNDSYSTEM_SOUND *_pstSound)
{
  return orxFLOAT_0;
}

orxFLOAT orxFASTCALL orxSoundSystem_Dummy_GetReferenceDistance(const orxSOUNDSYSTEM_SOUND *_pstSound)
{
  return orxFLOAT_0;
}

orxBOOL orxFASTCALL orxSoundSystem_Dummy_IsLooping(const orxSOUNDSYSTEM_SOUND *_pstSound)
{
  return orxFALSE;
}

orxFLOAT orxFASTCALL orxSoundSystem_Dummy_GetDuration(const orxSOUNDSYSTEM_SOUND *_pstSound)
{
  return orxFLOAT_0;
}

orxSOUNDSYSTEM_STATUS orxFASTCALL orxSoundSystem_Dummy_GetStatus(const orxSOUNDSYSTEM_SOUND *_pstSound)
{
	return orxSOUNDSYSTEM_STATUS_NONE;
}

orxSTATUS orxFASTCALL orxSoundSystem_Dummy_SetGlobalVolume(orxFLOAT _fVolume)
{
  return orxSTATUS_FAILURE;
}

orxFLOAT orxFASTCALL orxSoundSystem_Dummy_GetGlobalVolume()
{
  return orxFLOAT_0;
}

orxSTATUS orxFASTCALL orxSoundSystem_Dummy_SetListenerPosition(const orxVECTOR *_pvPosition)
{
  return orxSTATUS_FAILURE;
}

orxVECTOR *orxFASTCALL orxSoundSystem_Dummy_GetListenerPosition(orxVECTOR *_pvPosition)
{
  return orxNULL;
}

orxSTATUS orxFASTCALL orxSoundSystem_Dummy_StartRecording(const orxSTRING _zName, orxBOOL _bWriteToFile, orxU32 _u32SampleRate, orxU32 _u32ChannelNumber)
{
  return orxSTATUS_FAILURE;
}

orxSTATUS orxFASTCALL orxSoundSystem_Dummy_StopRecording()
{
  return orxSTATUS_FAILURE;
}

orxBOOL orxFASTCALL orxSoundSystem_Dummy_HasRecordingSupport()
{
  return orxFALSE;
}
/***************************************************************************
 * Plugin related                                                          *
 ***************************************************************************/

orxPLUGIN_USER_CORE_FUNCTION_START( SOUNDSYSTEM);
orxPLUGIN_USER_CORE_FUNCTION_ADD( orxSoundSystem_Dummy_Init, SOUNDSYSTEM, INIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD( orxSoundSystem_Dummy_Exit, SOUNDSYSTEM, EXIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD( orxSoundSystem_Dummy_CreateSample, SOUNDSYSTEM,	CREATE_SAMPLE);
orxPLUGIN_USER_CORE_FUNCTION_ADD( orxSoundSystem_Dummy_LoadSample, SOUNDSYSTEM,	LOAD_SAMPLE);
orxPLUGIN_USER_CORE_FUNCTION_ADD( orxSoundSystem_Dummy_DeleteSample, SOUNDSYSTEM, DELETE_SAMPLE);
orxPLUGIN_USER_CORE_FUNCTION_ADD( orxSoundSystem_Dummy_GetSampleInfo, SOUNDSYSTEM, GET_SAMPLE_INFO);
orxPLUGIN_USER_CORE_FUNCTION_ADD( orxSoundSystem_Dummy_SetSampleData, SOUNDSYSTEM, SET_SAMPLE_DATA);
orxPLUGIN_USER_CORE_FUNCTION_ADD( orxSoundSystem_Dummy_CreateFromSample, SOUNDSYSTEM, CREATE_FROM_SAMPLE);
orxPLUGIN_USER_CORE_FUNCTION_ADD( orxSoundSystem_Dummy_CreateStream, SOUNDSYSTEM, CREATE_STREAM);
orxPLUGIN_USER_CORE_FUNCTION_ADD( orxSoundSystem_Dummy_CreateStreamFromFile, SOUNDSYSTEM, CREATE_STREAM_FROM_FILE);
orxPLUGIN_USER_CORE_FUNCTION_ADD( orxSoundSystem_Dummy_Delete, SOUNDSYSTEM,	DELETE);
orxPLUGIN_USER_CORE_FUNCTION_ADD( orxSoundSystem_Dummy_Play, SOUNDSYSTEM, PLAY);
orxPLUGIN_USER_CORE_FUNCTION_ADD( orxSoundSystem_Dummy_Pause,	SOUNDSYSTEM, PAUSE);
orxPLUGIN_USER_CORE_FUNCTION_ADD( orxSoundSystem_Dummy_Stop, SOUNDSYSTEM, STOP);
orxPLUGIN_USER_CORE_FUNCTION_ADD( orxSoundSystem_Dummy_SetVolume, SOUNDSYSTEM, SET_VOLUME);
orxPLUGIN_USER_CORE_FUNCTION_ADD( orxSoundSystem_Dummy_SetPitch, SOUNDSYSTEM,	SET_PITCH);
orxPLUGIN_USER_CORE_FUNCTION_ADD( orxSoundSystem_Dummy_SetPosition, SOUNDSYSTEM, SET_POSITION);
orxPLUGIN_USER_CORE_FUNCTION_ADD( orxSoundSystem_Dummy_SetAttenuation, SOUNDSYSTEM, SET_ATTENUATION);
orxPLUGIN_USER_CORE_FUNCTION_ADD( orxSoundSystem_Dummy_SetReferenceDistance, SOUNDSYSTEM, SET_REFERENCE_DISTANCE);
orxPLUGIN_USER_CORE_FUNCTION_ADD( orxSoundSystem_Dummy_Loop, SOUNDSYSTEM, LOOP);
orxPLUGIN_USER_CORE_FUNCTION_ADD( orxSoundSystem_Dummy_GetVolume, SOUNDSYSTEM, GET_VOLUME);
orxPLUGIN_USER_CORE_FUNCTION_ADD( orxSoundSystem_Dummy_GetPitch, SOUNDSYSTEM,	GET_PITCH);
orxPLUGIN_USER_CORE_FUNCTION_ADD( orxSoundSystem_Dummy_GetPosition, SOUNDSYSTEM, GET_POSITION);
orxPLUGIN_USER_CORE_FUNCTION_ADD( orxSoundSystem_Dummy_GetAttenuation, SOUNDSYSTEM, GET_ATTENUATION);
orxPLUGIN_USER_CORE_FUNCTION_ADD( orxSoundSystem_Dummy_GetReferenceDistance, SOUNDSYSTEM, GET_REFERENCE_DISTANCE);
orxPLUGIN_USER_CORE_FUNCTION_ADD( orxSoundSystem_Dummy_IsLooping, SOUNDSYSTEM, IS_LOOPING);
orxPLUGIN_USER_CORE_FUNCTION_ADD( orxSoundSystem_Dummy_GetDuration, SOUNDSYSTEM, GET_DURATION);
orxPLUGIN_USER_CORE_FUNCTION_ADD( orxSoundSystem_Dummy_GetStatus, SOUNDSYSTEM, GET_STATUS);
orxPLUGIN_USER_CORE_FUNCTION_ADD( orxSoundSystem_Dummy_SetGlobalVolume,	SOUNDSYSTEM, SET_GLOBAL_VOLUME);
orxPLUGIN_USER_CORE_FUNCTION_ADD( orxSoundSystem_Dummy_GetGlobalVolume,	SOUNDSYSTEM, GET_GLOBAL_VOLUME);
orxPLUGIN_USER_CORE_FUNCTION_ADD( orxSoundSystem_Dummy_SetListenerPosition,	SOUNDSYSTEM, SET_LISTENER_POSITION);
orxPLUGIN_USER_CORE_FUNCTION_ADD( orxSoundSystem_Dummy_GetListenerPosition, SOUNDSYSTEM, GET_LISTENER_POSITION);
orxPLUGIN_USER_CORE_FUNCTION_ADD( orxSoundSystem_Dummy_StartRecording, SOUNDSYSTEM, START_RECORDING);
orxPLUGIN_USER_CORE_FUNCTION_ADD( orxSoundSystem_Dummy_StopRecording, SOUNDSYSTEM, STOP_RECORDING);
orxPLUGIN_USER_CORE_FUNCTION_ADD( orxSoundSystem_Dummy_HasRecordingSupport, SOUNDSYSTEM, HAS_RECORDING_SUPPORT);
orxPLUGIN_USER_CORE_FUNCTION_END();
