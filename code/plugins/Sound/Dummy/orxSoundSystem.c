/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2022 Orx-Project
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

  orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "Not available on this platform!");

  /* Done! */
  return eResult;
}

void orxFASTCALL orxSoundSystem_Dummy_Exit()
{
  /* Done! */
  return;
}

orxSOUNDSYSTEM_SAMPLE *orxFASTCALL orxSoundSystem_Dummy_CreateSample(orxU32 _u32ChannelNumber, orxU32 _u32FrameNumber, orxU32 _u32SampleRate)
{
  /* Done! */
  return orxNULL;
}

orxSOUNDSYSTEM_SAMPLE *orxFASTCALL orxSoundSystem_Dummy_LoadSample(const orxSTRING _zFilename)
{
  /* Done! */
  return orxNULL;
}

orxSTATUS orxFASTCALL orxSoundSystem_Dummy_DeleteSample(orxSOUNDSYSTEM_SAMPLE *_pstSample)
{
  /* Done! */
  return orxSTATUS_FAILURE;
}

orxSTATUS orxFASTCALL orxSoundSystem_Dummy_GetSampleInfo(const orxSOUNDSYSTEM_SAMPLE *_pstSample, orxU32 *_pu32ChannelNumber, orxU32 *_pu32FrameNumber, orxU32 *_pu32SampleRate)
{
  /* Done! */
  return orxSTATUS_FAILURE;
}

orxSTATUS orxFASTCALL orxSoundSystem_Dummy_SetSampleData(orxSOUNDSYSTEM_SAMPLE *_pstSample, const orxFLOAT *_afData, orxU32 _u32SampleNumber)
{
  /* Done! */
  return orxSTATUS_FAILURE;
}

orxSOUNDSYSTEM_SOUND *orxFASTCALL orxSoundSystem_Dummy_CreateFromSample(orxHANDLE _hUserData, const orxSOUNDSYSTEM_SAMPLE *_pstSample)
{
  /* Done! */
  return orxNULL;
}

orxSOUNDSYSTEM_SOUND *orxFASTCALL orxSoundSystem_Dummy_CreateStream(orxHANDLE _hUserData, orxU32 _u32ChannelNumber, orxU32 _u32SampleRate)
{
  /* Done! */
  return orxNULL;
}

orxSOUNDSYSTEM_SOUND *orxFASTCALL orxSoundSystem_Dummy_LoadStream(orxHANDLE _hUserData, const orxSTRING _zFilename)
{
  /* Done! */
  return orxNULL;
}

orxSTATUS orxFASTCALL orxSoundSystem_Dummy_Delete(orxSOUNDSYSTEM_SOUND *_pstSound)
{
  /* Done! */
  return orxSTATUS_FAILURE;
}

orxSTATUS orxFASTCALL orxSoundSystem_Dummy_Play(orxSOUNDSYSTEM_SOUND *_pstSound)
{
  /* Done! */
  return orxSTATUS_FAILURE;
}

orxSTATUS orxFASTCALL orxSoundSystem_Dummy_Pause(orxSOUNDSYSTEM_SOUND *_pstSound)
{
  /* Done! */
  return orxSTATUS_FAILURE;
}

orxSTATUS orxFASTCALL orxSoundSystem_Dummy_Stop(orxSOUNDSYSTEM_SOUND *_pstSound)
{
  /* Done! */
  return orxSTATUS_FAILURE;
}

orxSTATUS orxFASTCALL orxSoundSystem_Dummy_AddFilter(orxSOUNDSYSTEM_SOUND *_pstSound, const orxSOUND_FILTER_DATA *_pstFilterData, orxBOOL _bUseCustomParam)
{
  /* Done! */
  return orxSTATUS_FAILURE;
}

orxSTATUS orxFASTCALL orxSoundSystem_Dummy_RemoveLastFilter(orxSOUNDSYSTEM_SOUND *_pstSound)
{
  /* Done! */
  return orxSTATUS_FAILURE;
}

orxSTATUS orxFASTCALL orxSoundSystem_Dummy_RemoveAllFilters(orxSOUNDSYSTEM_SOUND *_pstSound)
{
  /* Done! */
  return orxSTATUS_FAILURE;
}

orxHANDLE orxFASTCALL orxSoundSystem_Dummy_CreateBus(orxSTRINGID _stBusID)
{
  /* Done! */
  return orxHANDLE_UNDEFINED;
}

orxSTATUS orxFASTCALL orxSoundSystem_Dummy_DeleteBus(orxHANDLE _hBus)
{
  /* Done! */
  return orxSTATUS_FAILURE;
}

orxSTATUS orxFASTCALL orxSoundSystem_Dummy_SetBus(orxSOUNDSYSTEM_SOUND *_pstSound, orxHANDLE _hBus)
{
  /* Done! */
  return orxSTATUS_FAILURE;
}

orxSTATUS orxFASTCALL orxSoundSystem_Dummy_SetBusParent(orxHANDLE _hBus, orxHANDLE _hParentBus)
{
  /* Done! */
  return orxSTATUS_FAILURE;
}

orxSTATUS orxFASTCALL orxSoundSystem_Dummy_AddBusFilter(orxHANDLE _hBus, const orxSOUND_FILTER_DATA *_pstFilterData, orxBOOL _bUseCustomParam)
{
  /* Done! */
  return orxSTATUS_FAILURE;
}

orxSTATUS orxFASTCALL orxSoundSystem_Dummy_RemoveLastBusFilter(orxHANDLE _hBus)
{
  /* Done! */
  return orxSTATUS_FAILURE;
}

orxSTATUS orxFASTCALL orxSoundSystem_Dummy_RemoveAllBusFilters(orxHANDLE _hBus)
{
  /* Done! */
  return orxSTATUS_FAILURE;
}

orxSTATUS orxFASTCALL orxSoundSystem_Dummy_SetVolume(orxSOUNDSYSTEM_SOUND *_pstSound, orxFLOAT _fVolume)
{
  /* Done! */
  return orxSTATUS_FAILURE;
}

orxSTATUS orxFASTCALL orxSoundSystem_Dummy_SetPitch(orxSOUNDSYSTEM_SOUND *_pstSound, orxFLOAT _fPitch)
{
  /* Done! */
  return orxSTATUS_FAILURE;
}

orxSTATUS orxFASTCALL orxSoundSystem_Dummy_SetTime(orxSOUNDSYSTEM_SOUND *_pstSound, orxFLOAT _fTime)
{
  /* Done! */
  return orxSTATUS_FAILURE;
}

orxSTATUS orxFASTCALL orxSoundSystem_Dummy_SetPosition(orxSOUNDSYSTEM_SOUND *_pstSound, const orxVECTOR *_pvPosition)
{
  /* Done! */
  return orxSTATUS_FAILURE;
}

orxSTATUS orxFASTCALL orxSoundSystem_Dummy_SetSpatialization(orxSOUNDSYSTEM_SOUND *_pstSound, orxFLOAT _fMinDistance, orxFLOAT _fMaxDistance, orxFLOAT _fMinGain, orxFLOAT _fMaxGain, orxFLOAT _fRollOff)
{
  /* Done! */
  return orxSTATUS_FAILURE;
}

orxSTATUS orxFASTCALL orxSoundSystem_Dummy_SetPanning(orxSOUNDSYSTEM_SOUND *_pstSound, orxFLOAT _fPanning, orxBOOL _bMix)
{
  /* Done! */
  return orxSTATUS_FAILURE;
}

orxSTATUS orxFASTCALL orxSoundSystem_Dummy_Loop(orxSOUNDSYSTEM_SOUND *_pstSound, orxBOOL _bLoop)
{
  /* Done! */
  return orxSTATUS_FAILURE;
}

orxFLOAT orxFASTCALL orxSoundSystem_Dummy_GetVolume(const orxSOUNDSYSTEM_SOUND *_pstSound)
{
  /* Done! */
  return orxFLOAT_0;
}

orxFLOAT orxFASTCALL orxSoundSystem_Dummy_GetPitch(const orxSOUNDSYSTEM_SOUND *_pstSound)
{
  /* Done! */
  return orxFLOAT_0;
}

orxFLOAT orxFASTCALL orxSoundSystem_Dummy_GetTime(const orxSOUNDSYSTEM_SOUND *_pstSound)
{
  /* Done! */
  return orxFLOAT_0;
}

orxVECTOR *orxFASTCALL orxSoundSystem_Dummy_GetPosition(const orxSOUNDSYSTEM_SOUND *_pstSound, orxVECTOR *_pvPosition)
{
  /* Done! */
  return orxNULL;
}

orxSTATUS orxFASTCALL orxSoundSystem_Dummy_GetSpatialization(const orxSOUNDSYSTEM_SOUND *_pstSound, orxFLOAT *_pfMinDistance, orxFLOAT *_pfMaxDistance, orxFLOAT *_pfMinGain, orxFLOAT *_pfMaxGain, orxFLOAT *_pfRollOff)
{
  /* Done! */
  return orxSTATUS_FAILURE;
}

orxSTATUS orxFASTCALL orxSoundSystem_Dummy_GetPanning(const orxSOUNDSYSTEM_SOUND *_pstSound, orxFLOAT *_pfPanning, orxBOOL *_pbMix)
{
  /* Done! */
  return orxSTATUS_FAILURE;
}

orxBOOL orxFASTCALL orxSoundSystem_Dummy_IsLooping(const orxSOUNDSYSTEM_SOUND *_pstSound)
{
  /* Done! */
  return orxFALSE;
}

orxFLOAT orxFASTCALL orxSoundSystem_Dummy_GetDuration(const orxSOUNDSYSTEM_SOUND *_pstSound)
{
  /* Done! */
  return orxFLOAT_0;
}

orxSOUNDSYSTEM_STATUS orxFASTCALL orxSoundSystem_Dummy_GetStatus(const orxSOUNDSYSTEM_SOUND *_pstSound)
{
  /* Done! */
  return orxSOUNDSYSTEM_STATUS_NONE;
}

orxSTATUS orxFASTCALL orxSoundSystem_Dummy_SetGlobalVolume(orxFLOAT _fVolume)
{
  /* Done! */
  return orxSTATUS_FAILURE;
}

orxFLOAT orxFASTCALL orxSoundSystem_Dummy_GetGlobalVolume()
{
  /* Done! */
  return orxFLOAT_0;
}

orxU32 orxFASTCALL orxSoundSystem_Dummy_GetListenerCount()
{
  /* Done! */
  return 0;
}

void orxFASTCALL orxSoundSystem_Dummy_EnableListener(orxU32 _u32ListenerIndex, orxBOOL _bEnable)
{
  /* Done! */
  return;
}

orxBOOL orxFASTCALL orxSoundSystem_Dummy_IsListenerEnabled(orxU32 _u32ListenerIndex)
{
  /* Done! */
  return orxFALSE;
}

orxSTATUS orxFASTCALL orxSoundSystem_Dummy_SetListenerPosition(orxU32 _u32Index, const orxVECTOR *_pvPosition)
{
  /* Done! */
  return orxSTATUS_FAILURE;
}

orxVECTOR *orxFASTCALL orxSoundSystem_Dummy_GetListenerPosition(orxU32 _u32Index, orxVECTOR *_pvPosition)
{
  /* Done! */
  return orxNULL;
}

orxSTATUS orxFASTCALL orxSoundSystem_Dummy_StartRecording(const orxSTRING _zName, orxBOOL _bWriteToFile, orxU32 _u32SampleRate, orxU32 _u32ChannelNumber)
{
  /* Done! */
  return orxSTATUS_FAILURE;
}

orxSTATUS orxFASTCALL orxSoundSystem_Dummy_StopRecording()
{
  /* Done! */
  return orxSTATUS_FAILURE;
}

orxBOOL orxFASTCALL orxSoundSystem_Dummy_HasRecordingSupport()
{
  /* Done! */
  return orxFALSE;
}

/***************************************************************************
 * Plugin related                                                          *
 ***************************************************************************/

orxPLUGIN_USER_CORE_FUNCTION_START(SOUNDSYSTEM);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_Dummy_Init, SOUNDSYSTEM, INIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_Dummy_Exit, SOUNDSYSTEM, EXIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_Dummy_CreateSample, SOUNDSYSTEM, CREATE_SAMPLE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_Dummy_LoadSample, SOUNDSYSTEM, LOAD_SAMPLE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_Dummy_DeleteSample, SOUNDSYSTEM, DELETE_SAMPLE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_Dummy_GetSampleInfo, SOUNDSYSTEM, GET_SAMPLE_INFO);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_Dummy_SetSampleData, SOUNDSYSTEM, SET_SAMPLE_DATA);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_Dummy_CreateFromSample, SOUNDSYSTEM, CREATE_FROM_SAMPLE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_Dummy_CreateStream, SOUNDSYSTEM, CREATE_STREAM);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_Dummy_LoadStream, SOUNDSYSTEM, LOAD_STREAM);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_Dummy_Delete, SOUNDSYSTEM, DELETE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_Dummy_Play, SOUNDSYSTEM, PLAY);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_Dummy_Pause, SOUNDSYSTEM, PAUSE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_Dummy_Stop, SOUNDSYSTEM, STOP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_Dummy_AddFilter, SOUNDSYSTEM, ADD_FILTER);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_Dummy_RemoveLastFilter, SOUNDSYSTEM, REMOVE_LAST_FILTER);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_Dummy_RemoveAllFilters, SOUNDSYSTEM, REMOVE_ALL_FILTERS);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_Dummy_CreateBus, SOUNDSYSTEM, CREATE_BUS);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_Dummy_DeleteBus, SOUNDSYSTEM, DELETE_BUS);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_Dummy_SetBus, SOUNDSYSTEM, SET_BUS);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_Dummy_SetBusParent, SOUNDSYSTEM, SET_BUS_PARENT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_Dummy_AddBusFilter, SOUNDSYSTEM, ADD_BUS_FILTER);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_Dummy_RemoveLastBusFilter, SOUNDSYSTEM, REMOVE_LAST_BUS_FILTER);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_Dummy_RemoveAllBusFilters, SOUNDSYSTEM, REMOVE_ALL_BUS_FILTERS);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_Dummy_SetVolume, SOUNDSYSTEM, SET_VOLUME);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_Dummy_SetPitch, SOUNDSYSTEM, SET_PITCH);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_Dummy_SetTime, SOUNDSYSTEM, SET_TIME);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_Dummy_SetPosition, SOUNDSYSTEM, SET_POSITION);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_Dummy_SetSpatialization, SOUNDSYSTEM, SET_SPATIALIZATION);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_Dummy_SetPanning, SOUNDSYSTEM, SET_PANNING);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_Dummy_Loop, SOUNDSYSTEM, LOOP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_Dummy_GetVolume, SOUNDSYSTEM, GET_VOLUME);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_Dummy_GetPitch, SOUNDSYSTEM, GET_PITCH);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_Dummy_GetTime, SOUNDSYSTEM, GET_TIME);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_Dummy_GetPosition, SOUNDSYSTEM, GET_POSITION);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_Dummy_GetSpatialization, SOUNDSYSTEM, GET_SPATIALIZATION);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_Dummy_GetPanning, SOUNDSYSTEM, GET_PANNING);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_Dummy_IsLooping, SOUNDSYSTEM, IS_LOOPING);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_Dummy_GetDuration, SOUNDSYSTEM, GET_DURATION);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_Dummy_GetStatus, SOUNDSYSTEM, GET_STATUS);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_Dummy_SetGlobalVolume, SOUNDSYSTEM, SET_GLOBAL_VOLUME);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_Dummy_GetGlobalVolume, SOUNDSYSTEM, GET_GLOBAL_VOLUME);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_Dummy_GetListenerCount, SOUNDSYSTEM, GET_LISTENER_COUNT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_Dummy_EnableListener, SOUNDSYSTEM, ENABLE_LISTENER);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_Dummy_IsListenerEnabled, SOUNDSYSTEM, IS_LISTENER_ENABLED);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_Dummy_SetListenerPosition, SOUNDSYSTEM, SET_LISTENER_POSITION);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_Dummy_GetListenerPosition, SOUNDSYSTEM, GET_LISTENER_POSITION);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_Dummy_StartRecording, SOUNDSYSTEM, START_RECORDING);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_Dummy_StopRecording, SOUNDSYSTEM, STOP_RECORDING);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_Dummy_HasRecordingSupport, SOUNDSYSTEM, HAS_RECORDING_SUPPORT);
orxPLUGIN_USER_CORE_FUNCTION_END();
