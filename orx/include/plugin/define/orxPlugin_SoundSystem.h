/***************************************************************************
 *                                                                         *
 *   This library is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License           *
 *   as published by the Free Software Foundation; either version 2.1      *
 *   of the License, or (at your option) any later version.                *
 *                                                                         *
 ***************************************************************************/

/**
 * @file orxPlugin_SoundSystem.h
 * @date 14/11/2003
 * @author (C) Arcallians
 */

/**
 * @addtogroup Plugin
 * 
 * This header is used to define ID for sound system plugin registration
 *
 * @{
 */


#ifndef _orxPLUGIN_SOUNDSYSTEM_H_
#define _orxPLUGIN_SOUNDSYSTEM_H_


#include "plugin/define/orxPlugin_CoreID.h"


/***************************************************************************
 * Constants                                                               *
 ***************************************************************************/

typedef enum __orxPLUGIN_FUNCTION_BASE_ID_SOUNDSYSTEM_t
{
  orxPLUGIN_FUNCTION_BASE_ID_SOUNDSYSTEM_INIT = 0,
  orxPLUGIN_FUNCTION_BASE_ID_SOUNDSYSTEM_EXIT,
  orxPLUGIN_FUNCTION_BASE_ID_SOUNDSYSTEM_LOAD_SAMPLE,
  orxPLUGIN_FUNCTION_BASE_ID_SOUNDSYSTEM_UNLOAD_SAMPLE,
  orxPLUGIN_FUNCTION_BASE_ID_SOUNDSYSTEM_CREATE_FROM_SAMPLE,
  orxPLUGIN_FUNCTION_BASE_ID_SOUNDSYSTEM_CREATE_STREAM_FROM_FILE,
  orxPLUGIN_FUNCTION_BASE_ID_SOUNDSYSTEM_DELETE,
  orxPLUGIN_FUNCTION_BASE_ID_SOUNDSYSTEM_PLAY,
  orxPLUGIN_FUNCTION_BASE_ID_SOUNDSYSTEM_PAUSE,
  orxPLUGIN_FUNCTION_BASE_ID_SOUNDSYSTEM_STOP,
  orxPLUGIN_FUNCTION_BASE_ID_SOUNDSYSTEM_SET_VOLUME,
  orxPLUGIN_FUNCTION_BASE_ID_SOUNDSYSTEM_SET_PITCH,
  orxPLUGIN_FUNCTION_BASE_ID_SOUNDSYSTEM_SET_POSITION,
  orxPLUGIN_FUNCTION_BASE_ID_SOUNDSYSTEM_LOOP,
  orxPLUGIN_FUNCTION_BASE_ID_SOUNDSYSTEM_GET_VOLUME,
  orxPLUGIN_FUNCTION_BASE_ID_SOUNDSYSTEM_GET_PITCH,
  orxPLUGIN_FUNCTION_BASE_ID_SOUNDSYSTEM_GET_POSITION,
  orxPLUGIN_FUNCTION_BASE_ID_SOUNDSYSTEM_IS_LOOPING,
  orxPLUGIN_FUNCTION_BASE_ID_SOUNDSYSTEM_GET_DURATION,
  orxPLUGIN_FUNCTION_BASE_ID_SOUNDSYSTEM_GET_STATUS,

  orxPLUGIN_FUNCTION_BASE_ID_SOUNDSYSTEM_NUMBER,

  orxPLUGIN_FUNCTION_BASE_ID_SOUNDSYSTEM_NONE = orxENUM_NONE

} orxPLUGIN_FUNCTION_BASE_ID_SOUNDSYSTEM;


#endif /* _orxPLUGIN_SOUNDSYSTEM_H_ */

/** @} */
