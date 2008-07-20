/**
 * @file orxSound.c
 */

/***************************************************************************
 begin                : 23/07/2002
 author               : (C) Arcallians
 email                : iarwain@arcallians.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This library is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License           *
 *   as published by the Free Software Foundation; either version 2.1      *
 *   of the License, or (at your option) any later version.                *
 *                                                                         *
 ***************************************************************************/


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


/* *** Core function definitions *** */

orxPLUGIN_DEFINE_CORE_FUNCTION(orxSoundSystem_Init, orxSTATUS);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxSoundSystem_Exit, orxVOID);
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
