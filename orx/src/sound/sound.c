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

#include "plugin/orxPluginCore.h"


/********************
 *  Plugin Related  *
 ********************/

orxSTATIC orxCONST orxPLUGIN_CORE_FUNCTION sastSoundPluginFunctionInfo[orxPLUGIN_FUNCTION_BASE_ID_SOUND_NUMBER] =
{
  {(orxPLUGIN_FUNCTION *) &sound_init,                   orxPLUGIN_FUNCTION_BASE_ID_SOUND_INIT},
  {(orxPLUGIN_FUNCTION *) &sound_exit,                   orxPLUGIN_FUNCTION_BASE_ID_SOUND_EXIT},
  {(orxPLUGIN_FUNCTION *) &sound_load_music_from_file,   orxPLUGIN_FUNCTION_BASE_ID_SOUND_LOAD_MUSIC_FROM_FILE},
  {(orxPLUGIN_FUNCTION *) &sound_stop,                   orxPLUGIN_FUNCTION_BASE_ID_SOUND_STOP},
  {(orxPLUGIN_FUNCTION *) &sound_pause,                  orxPLUGIN_FUNCTION_BASE_ID_SOUND_PAUSE},
  {(orxPLUGIN_FUNCTION *) &sound_unpause,                orxPLUGIN_FUNCTION_BASE_ID_SOUND_UNPAUSE},
  {(orxPLUGIN_FUNCTION *) &sound_is_playing,             orxPLUGIN_FUNCTION_BASE_ID_SOUND_IS_PLAYING},
  {(orxPLUGIN_FUNCTION *) &sound_play_sample,            orxPLUGIN_FUNCTION_BASE_ID_SOUND_PLAY_SAMPLE},
  {(orxPLUGIN_FUNCTION *) &sound_release_sample,         orxPLUGIN_FUNCTION_BASE_ID_SOUND_RELEASE_SAMPLE},
  {(orxPLUGIN_FUNCTION *) &sound_set_volume,             orxPLUGIN_FUNCTION_BASE_ID_SOUND_SET_VOLUME},
  {(orxPLUGIN_FUNCTION *) &sound_play_bgmusic_from_file, orxPLUGIN_FUNCTION_BASE_ID_SOUND_PLAY_BKGD_MUSIC_FROM_FILE}
};

orxVOID sound_plugin_init()
{
  /* Plugin init */
  orxPlugin_AddCoreInfo(orxPLUGIN_CORE_ID_SOUND, sastSoundPluginFunctionInfo, sizeof(sastSoundPluginFunctionInfo) / sizeof(orxPLUGIN_CORE_FUNCTION));

  return;
}



/********************
 *   Core Related   *
 ********************/

orxPLUGIN_DEFINE_CORE_FUNCTION(sound_init, orxU32);
orxPLUGIN_DEFINE_CORE_FUNCTION(sound_load_music_from_file, sound_st_sample *, orxU8 *);
orxPLUGIN_DEFINE_CORE_FUNCTION(sound_stop, orxBOOL, sound_st_channel);
orxPLUGIN_DEFINE_CORE_FUNCTION(sound_pause, orxBOOL, sound_st_channel);
orxPLUGIN_DEFINE_CORE_FUNCTION(sound_unpause, orxBOOL, sound_st_channel);
orxPLUGIN_DEFINE_CORE_FUNCTION(sound_is_playing, orxBOOL, sound_st_channel);
orxPLUGIN_DEFINE_CORE_FUNCTION(sound_play_sample, sound_st_channel, sound_st_channel, sound_st_sample *);
orxPLUGIN_DEFINE_CORE_FUNCTION(sound_release_sample, orxBOOL, sound_st_sample *);
orxPLUGIN_DEFINE_CORE_FUNCTION(sound_set_volume, orxBOOL, sound_st_channel, orxS32);
orxPLUGIN_DEFINE_CORE_FUNCTION(sound_play_bgmusic_from_file, orxBOOL, orxU8 *, orxS32);
orxPLUGIN_DEFINE_CORE_FUNCTION(sound_exit, orxBOOL);
