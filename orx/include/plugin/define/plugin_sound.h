/**
 * \file plugin_sound.h
 * This header is used to define ID for sound plugin registration.
 */

/*
 begin                : 14/11/2003
 author               : (C) Gdp
 email                : iarwain@ifrance.com
 */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef _PLUGIN_SOUND_H_
#define _PLUGIN_SOUND_H_

/*********************************************
 Constants
 *********************************************/

#define PLUGIN_SOUND_KU32_PLUGIN_ID                  0x10000100

#define PLUGIN_SOUND_KU32_FUNCTION_NUMBER            0x0000000B

#define PLUGIN_SOUND_KU32_ID_INIT                    0x10000100
#define PLUGIN_SOUND_KU32_ID_EXIT                    0x10000101
#define PLUGIN_SOUND_KU32_ID_LOAD_MUSIC_FROM_FILE    0x10000102
#define PLUGIN_SOUND_KU32_ID_STOP                    0x10000103
#define PLUGIN_SOUND_KU32_ID_PAUSE                   0x10000104
#define PLUGIN_SOUND_KU32_ID_UNPAUSE                 0x10000105
#define PLUGIN_SOUND_KU32_ID_IS_PLAYING              0x10000106
#define PLUGIN_SOUND_KU32_ID_PLAY_SAMPLE             0x10000107
#define PLUGIN_SOUND_KU32_ID_RELEASE_SAMPLE          0x10000108
#define PLUGIN_SOUND_KU32_ID_SET_VOLUME              0x10000109
#define PLUGIN_SOUND_KU32_ID_PLAY_BGMUSIC_FROM_FILE  0x1000010A

#endif /* _PLUGIN_SOUND_H_ */
