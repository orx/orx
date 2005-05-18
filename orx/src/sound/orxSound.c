/**
 * @file orxSound.c
 * 
 * Module for sound management.
 * 
 * @todo Add Stream management and 3D sounds in the API
 */ 

/***************************************************************************
 orxSound.c

 begin                : 23/07/2002
                        14/11/2003
 author               : (C) Gdp
 email                : snegri@free.fr
                        iarwain@ifrance.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "sound/orxSound.h"
#include "plugin/orxPluginCore.h"

/********************
 *  Plugin Related  *
 ********************/

orxSTATIC orxCONST orxPLUGIN_CORE_FUNCTION sastSoundPluginFunctionInfo[orxPLUGIN_FUNCTION_BASE_ID_SOUND_NUMBER] =
{
  {(orxPLUGIN_FUNCTION *) &orxSound_Init,                 orxPLUGIN_FUNCTION_BASE_ID_SOUND_INIT},
  {(orxPLUGIN_FUNCTION *) &orxSound_Exit,                 orxPLUGIN_FUNCTION_BASE_ID_SOUND_EXIT},
  {(orxPLUGIN_FUNCTION *) &orxSound_SampleLoadFromFile,   orxPLUGIN_FUNCTION_BASE_ID_SOUND_LOAD_SAMPLE_FROM_FILE},
  {(orxPLUGIN_FUNCTION *) &orxSound_SampleLoadFromMemory, orxPLUGIN_FUNCTION_BASE_ID_SOUND_LOAD_SAMPLE_FROM_MEMORY},
  {(orxPLUGIN_FUNCTION *) &orxSound_SampleUnload,         orxPLUGIN_FUNCTION_BASE_ID_SOUND_UNLOAD_SAMPLE},
  {(orxPLUGIN_FUNCTION *) &orxSound_PlaySample,           orxPLUGIN_FUNCTION_BASE_ID_SOUND_PLAY_SAMPLE},
  {(orxPLUGIN_FUNCTION *) &orxSound_Stop,                 orxPLUGIN_FUNCTION_BASE_ID_SOUND_STOP},
  {(orxPLUGIN_FUNCTION *) &orxSound_Pause,                orxPLUGIN_FUNCTION_BASE_ID_SOUND_PAUSE},
  {(orxPLUGIN_FUNCTION *) &orxSound_TestFlags,            orxPLUGIN_FUNCTION_BASE_ID_SOUND_TEST_FLAGS},
  {(orxPLUGIN_FUNCTION *) &orxSound_SetFlags,             orxPLUGIN_FUNCTION_BASE_ID_SOUND_SET_FLAGS},
  {(orxPLUGIN_FUNCTION *) &orxSound_SetVolume,            orxPLUGIN_FUNCTION_BASE_ID_SOUND_SET_VOLUME}
};

orxVOID orxFASTCALL orxSound_Plugin_Init()
{
  /* Plugin init */
  orxPlugin_AddCoreInfo(orxPLUGIN_CORE_ID_SOUND, sastSoundPluginFunctionInfo, sizeof(sastSoundPluginFunctionInfo) / sizeof(orxPLUGIN_CORE_FUNCTION));

  return;
}



/********************
 *   Core Related   *
 ********************/

orxPLUGIN_DEFINE_CORE_FUNCTION(orxSound_Init, orxSTATUS);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxSound_Exit, orxVOID);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxSound_SampleLoadFromFile, orxSOUND_SAMPLE *, orxSTRING);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxSound_SampleLoadFromMemory, orxSOUND_SAMPLE *, orxCONST orxVOID *, orxU32);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxSound_SampleUnload, orxVOID, orxSOUND_SAMPLE *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxSound_PlaySample, orxU32, orxU32, orxSOUND_SAMPLE *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxSound_Stop, orxSTATUS, orxU32);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxSound_Pause, orxSTATUS, orxU32, orxBOOL);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxSound_TestFlags, orxBOOL, orxU32, orxU32);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxSound_SetFlags, orxVOID, orxU32, orxU32, orxU32);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxSound_SetVolume, orxVOID, orxU32, orxU8);
