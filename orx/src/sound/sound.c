/***************************************************************************
 sound.c

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

#include "sound/sound.h"

#include "plugin/plugin_core.h"


/********************
 *  Plugin Related  *
 ********************/

orxSTATIC plugin_core_st_function plugin_sound_spst_function[PLUGIN_SOUND_KU32_FUNCTION_NUMBER] =
{
  {(plugin_function *) &sound_init,                   PLUGIN_SOUND_KU32_ID_INIT},
  {(plugin_function *) &sound_exit,                   PLUGIN_SOUND_KU32_ID_EXIT},
  {(plugin_function *) &sound_load_music_from_file,   PLUGIN_SOUND_KU32_ID_LOAD_MUSIC_FROM_FILE},
  {(plugin_function *) &sound_stop,                   PLUGIN_SOUND_KU32_ID_STOP},
  {(plugin_function *) &sound_pause,                  PLUGIN_SOUND_KU32_ID_PAUSE},
  {(plugin_function *) &sound_unpause,                PLUGIN_SOUND_KU32_ID_UNPAUSE},
  {(plugin_function *) &sound_is_playing,             PLUGIN_SOUND_KU32_ID_IS_PLAYING},
  {(plugin_function *) &sound_play_sample,            PLUGIN_SOUND_KU32_ID_PLAY_SAMPLE},
  {(plugin_function *) &sound_release_sample,         PLUGIN_SOUND_KU32_ID_RELEASE_SAMPLE},
  {(plugin_function *) &sound_set_volume,             PLUGIN_SOUND_KU32_ID_SET_VOLUME},
  {(plugin_function *) &sound_play_bgmusic_from_file, PLUGIN_SOUND_KU32_ID_PLAY_BGMUSIC_FROM_FILE}
};

orxVOID sound_plugin_init()
{
  /* Plugin init */
  plugin_core_info_add(PLUGIN_SOUND_KU32_PLUGIN_ID, plugin_sound_spst_function, PLUGIN_SOUND_KU32_FUNCTION_NUMBER);

  return;
}



/********************
 *   Core Related   *
 ********************/

PLUGIN_CORE_FUNCTION_DEFINE(sound_init, orxU32);
PLUGIN_CORE_FUNCTION_DEFINE(sound_load_music_from_file, sound_st_sample *, orxU8 *);
PLUGIN_CORE_FUNCTION_DEFINE(sound_stop, orxBOOL, sound_st_channel);
PLUGIN_CORE_FUNCTION_DEFINE(sound_pause, orxBOOL, sound_st_channel);
PLUGIN_CORE_FUNCTION_DEFINE(sound_unpause, orxBOOL, sound_st_channel);
PLUGIN_CORE_FUNCTION_DEFINE(sound_is_playing, orxBOOL, sound_st_channel);
PLUGIN_CORE_FUNCTION_DEFINE(sound_play_sample, sound_st_channel, sound_st_channel, sound_st_sample *);
PLUGIN_CORE_FUNCTION_DEFINE(sound_release_sample, orxBOOL, sound_st_sample *);
PLUGIN_CORE_FUNCTION_DEFINE(sound_set_volume, orxBOOL, sound_st_channel, orxS32);
PLUGIN_CORE_FUNCTION_DEFINE(sound_play_bgmusic_from_file, orxBOOL, orxU8 *, orxS32);
PLUGIN_CORE_FUNCTION_DEFINE(sound_exit, orxBOOL);
