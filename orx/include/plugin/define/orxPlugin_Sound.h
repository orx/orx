/**
 * \file orxPlugin_Sound.h
 * This header is used to define ID for sound plugin registration.
 */

/*
 begin                : 14/11/2003
 author               : (C) Arcallians
 email                : iarwain@arcallians.org
 */

/***************************************************************************
 *                                                                         *
 *   This library is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License           *
 *   as published by the Free Software Foundation; either version 2.1      *
 *   of the License, or (at your option) any later version.                *
 *                                                                         *
 ***************************************************************************/


#ifndef _orxPLUGIN_SOUND_H_
#define _orxPLUGIN_SOUND_H_

#include "orxPlugin_CoreID.h"


/*********************************************
 Constants
 *********************************************/
typedef enum __orxPLUGIN_FUNCTION_BASE_ID_SOUND_t
{
  orxPLUGIN_FUNCTION_BASE_ID_SOUND_INIT = 0,
  orxPLUGIN_FUNCTION_BASE_ID_SOUND_EXIT,
  orxPLUGIN_FUNCTION_BASE_ID_SOUND_SAMPLE_LOAD_FROM_FILE,
  orxPLUGIN_FUNCTION_BASE_ID_SOUND_SAMPLE_LOAD_FROM_MEMORY,
  orxPLUGIN_FUNCTION_BASE_ID_SOUND_SAMPLE_UNLOAD,
  orxPLUGIN_FUNCTION_BASE_ID_SOUND_SAMPLE_PLAY,
  orxPLUGIN_FUNCTION_BASE_ID_SOUND_CHANNEL_STOP,
  orxPLUGIN_FUNCTION_BASE_ID_SOUND_CHANNEL_PAUSE,
  orxPLUGIN_FUNCTION_BASE_ID_SOUND_CHANNEL_TEST_FLAGS,
  orxPLUGIN_FUNCTION_BASE_ID_SOUND_CHANNEL_SET_FLAGS,
  orxPLUGIN_FUNCTION_BASE_ID_SOUND_CHANNEL_SET_VOLUME,

  orxPLUGIN_FUNCTION_BASE_ID_SOUND_NUMBER,

  orxPLUGIN_FUNCTION_BASE_ID_SOUND_NONE = orxENUM_NONE
    
} orxPLUGIN_FUNCTION_BASE_ID_SOUND;


#endif /* _orxPLUGIN_SOUND_H_ */
