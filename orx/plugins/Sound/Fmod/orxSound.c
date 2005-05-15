/***************************************************************************
 sound_plug.c
 wrappers for FMOD library

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

#include "include.h"

#include "plugin/plugin_user.h"

#include <fmod.h>

/*********************************************
 Definitions
 *********************************************/

typedef FSOUND_SAMPLE GP_SOUND_SAMPLE;
typedef int32         GP_SOUND_CHANNEL;
typedef FSOUND_STREAM GP_SOUND_STREAM;

#define K_GP_SOUND_FREE    FSOUND_FREE   
#define K_GP_SOUND_NORMAL  FSOUND_NORMAL
#define K_GP_SOUND_ALL     FSOUND_ALL
#define K_GP_SOUND_LOOP_NORMAL FSOUND_LOOP_NORMAL
    
#define K_GP_SOUND_BEEP_CHANNEL 15
#define K_GP_SOUND_BG_CHANNEL   16


/********************
 *   Core Related   *
 ********************/


static FSOUND_STREAM *m_currentBgMusic = NULL;

uint32 sound_init()
{
    return FSOUND_Init(44100, 32, 0);
}

/* Load sample from filename in memory*/
GP_SOUND_SAMPLE *sound_load_music_from_file(char *name)
{
    return FSOUND_Sample_Load(K_GP_SOUND_FREE, name, K_GP_SOUND_NORMAL, 0, 0);
}

bool sound_stop(GP_SOUND_CHANNEL channel)
{
    return FSOUND_StopSound(channel);
}

bool sound_pause(GP_SOUND_CHANNEL channel)
{
    return FSOUND_SetPaused(channel, TRUE);
}

bool sound_unpause(GP_SOUND_CHANNEL channel)
{
    return FSOUND_SetPaused(channel, FALSE);
}

bool sound_is_playing(GP_SOUND_CHANNEL channel)
{
    return FSOUND_IsPlaying(channel);
}

GP_SOUND_CHANNEL sound_play_sample(GP_SOUND_CHANNEL channel, GP_SOUND_SAMPLE *sample)
{
    return FSOUND_PlaySound(channel, sample);
}

void sound_release_sample(GP_SOUND_SAMPLE *sample)
{
    FSOUND_Sample_Free(sample);
}

bool sound_setvolume(GP_SOUND_CHANNEL channel, int32 vol)
{
    /* range from 0(silent) to 255(full) */
    if (vol < 0 || vol > 255)
        return FALSE;
    
    return FSOUND_SetVolume(channel, vol);
}

bool sound_play_bgmusic_from_file(char *filename, int32 volume)
{
    bool res = FALSE;

    m_currentBgMusic = FSOUND_Stream_Open(filename, K_GP_SOUND_LOOP_NORMAL, 0, 0);

    if (m_currentBgMusic)
    {
        if (!FSOUND_Stream_Play(K_GP_SOUND_BG_CHANNEL, m_currentBgMusic) == K_GP_SOUND_BG_CHANNEL)
        {
            FSOUND_Stream_Close(m_currentBgMusic);
            m_currentBgMusic = NULL;
            return FALSE;
        }

        res = TRUE;
        sound_setvolume(K_GP_SOUND_BG_CHANNEL, volume);
    }

    return res;
}

bool sound_exit()
{
    if (m_currentBgMusic)
    {
        FSOUND_Stream_Close(m_currentBgMusic);
        m_currentBgMusic = NULL;
    }

 FSOUND_Close();
    return TRUE;
}




/********************
 *  Plugin Related  *
 ********************/

static plugin_user_st_function_info sound_plug_spst_function[PLUGIN_SOUND_KUL_FUNCTION_NUMBER];

void plugin_init(int32 *_pi_fn_number, plugin_user_st_function_info **_ppst_fn_info)
{
  PLUGIN_USER_FUNCTION_START(sound_plug_spst_function);


  PLUGIN_USER_CORE_FUNCTION_ADD(sound_init, SOUND, INIT);

  PLUGIN_USER_CORE_FUNCTION_ADD(sound_exit, SOUND, EXIT);

  PLUGIN_USER_CORE_FUNCTION_ADD(sound_load_music_from_file, SOUND, LOAD_MUSIC_FROM_FILE);

  PLUGIN_USER_CORE_FUNCTION_ADD(sound_stop, SOUND, STOP);

  PLUGIN_USER_CORE_FUNCTION_ADD(sound_pause, SOUND, PAUSE);

  PLUGIN_USER_CORE_FUNCTION_ADD(sound_unpause, SOUND, UNPAUSE);

  PLUGIN_USER_CORE_FUNCTION_ADD(sound_is_playing, SOUND, IS_PLAYING);

  PLUGIN_USER_CORE_FUNCTION_ADD(sound_play_sample, SOUND, PLAY_SAMPLE);

  PLUGIN_USER_CORE_FUNCTION_ADD(sound_release_sample, SOUND, RELEASE_SAMPLE);

  PLUGIN_USER_CORE_FUNCTION_ADD(sound_setvolume, SOUND, SET_VOLUME);

  PLUGIN_USER_CORE_FUNCTION_ADD(sound_play_bgmusic_from_file, SOUND, PLAY_BGMUSIC_FROM_FILE);


  PLUGIN_USER_FUNCTION_END(_pi_fn_number, _ppst_fn_info);

  return;
}
