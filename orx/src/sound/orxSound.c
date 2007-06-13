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
 author               : (C) Arcallians
 email                : snegri@free.fr
                        iarwain@arcallians.org
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

/********************
 *  Plugin Related  *
 ********************/

/***************************************************************************
 orxSound_Setup
 Sound module setup.

 returns: nothing
 ***************************************************************************/
orxVOID orxSound_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_SOUND, orxMODULE_ID_PLUGIN);
  orxModule_AddDependency(orxMODULE_ID_SOUND, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_SOUND, orxMODULE_ID_BANK);
  orxModule_AddDependency(orxMODULE_ID_SOUND, orxMODULE_ID_LINKLIST);
  orxModule_AddDependency(orxMODULE_ID_SOUND, orxMODULE_ID_TREE);

  return;
}


/* *** Core function info array *** */

orxPLUGIN_BEGIN_CORE_FUNCTION_ARRAY(SOUND)

orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(SOUND, INIT, orxSound_Init)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(SOUND, EXIT, orxSound_Exit)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(SOUND, SAMPLE_LOAD_FROM_FILE, orxSound_SampleLoadFromFile)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(SOUND, SAMPLE_LOAD_FROM_MEMORY, orxSound_SampleLoadFromMemory)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(SOUND, SAMPLE_UNLOAD, orxSound_SampleUnload)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(SOUND, SAMPLE_PLAY, orxSound_SamplePlay)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(SOUND, CHANNEL_STOP, orxSound_ChannelStop)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(SOUND, CHANNEL_PAUSE, orxSound_ChannelPause)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(SOUND, CHANNEL_TEST_FLAGS, orxSound_ChannelTestFlags)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(SOUND, CHANNEL_SET_FLAGS, orxSound_ChannelSetFlags)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(SOUND, CHANNEL_SET_VOLUME, orxSound_ChannelSetVolume)

orxPLUGIN_END_CORE_FUNCTION_ARRAY(SOUND)


/* *** Core function definitions *** */

orxPLUGIN_DEFINE_CORE_FUNCTION(orxSound_Init, orxSTATUS);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxSound_Exit, orxVOID);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxSound_SampleLoadFromFile, orxSOUND_SAMPLE *, orxCONST orxSTRING);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxSound_SampleLoadFromMemory, orxSOUND_SAMPLE *, orxCONST orxVOID *, orxU32);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxSound_SampleUnload, orxVOID, orxSOUND_SAMPLE *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxSound_SamplePlay, orxU32, orxU32, orxSOUND_SAMPLE *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxSound_SampleTestFlags, orxBOOL, orxSOUND_SAMPLE *, orxU32);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxSound_SampleSetFlags, orxVOID, orxSOUND_SAMPLE *, orxU32, orxU32);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxSound_ChannelStop, orxSTATUS, orxU32);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxSound_ChannelPause, orxSTATUS, orxU32, orxBOOL);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxSound_ChannelTestFlags, orxBOOL, orxU32, orxU32);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxSound_ChannelSetFlags, orxVOID, orxU32, orxU32, orxU32);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxSound_ChannelSetVolume, orxSTATUS, orxU32, orxU8);
