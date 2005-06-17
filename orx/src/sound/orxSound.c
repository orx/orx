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

orxPLUGIN_END_CORE_FUNCTION_ARRAY()

orxVOID orxFASTCALL orxSound_Plugin_Init()
{
  orxPLUGIN_REGISTER_CORE_INFO(SOUND);
}
