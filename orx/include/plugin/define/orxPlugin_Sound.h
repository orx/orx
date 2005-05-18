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
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
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
  orxPLUGIN_FUNCTION_BASE_ID_SOUND_LOAD_SAMPLE_FROM_FILE,
  orxPLUGIN_FUNCTION_BASE_ID_SOUND_LOAD_SAMPLE_FROM_MEMORY,
  orxPLUGIN_FUNCTION_BASE_ID_SOUND_UNLOAD_SAMPLE,
  orxPLUGIN_FUNCTION_BASE_ID_SOUND_PLAY_SAMPLE,
  orxPLUGIN_FUNCTION_BASE_ID_SOUND_STOP,
  orxPLUGIN_FUNCTION_BASE_ID_SOUND_PAUSE,
  orxPLUGIN_FUNCTION_BASE_ID_SOUND_TEST_FLAGS,
  orxPLUGIN_FUNCTION_BASE_ID_SOUND_SET_FLAGS,
  orxPLUGIN_FUNCTION_BASE_ID_SOUND_SET_VOLUME,

  orxPLUGIN_FUNCTION_BASE_ID_SOUND_NUMBER,

  orxPLUGIN_FUNCTION_BASE_ID_SOUND_NONE = 0xFFFFFFFF
    
} orxPLUGIN_FUNCTION_BASE_ID_SOUND;


#endif /* _orxPLUGIN_SOUND_H_ */
