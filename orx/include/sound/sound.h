/***************************************************************************
 sound.h

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

#ifndef _SOUND_H_
#define _SOUND_H_

#include "include.h"

/*********************************************
 type declarations
 *********************************************/

typedef int32 sound_st_sample;
typedef int32 sound_st_channel;
typedef int32 sound_st_stream;


/*********************************************
 functions prototypes
 *********************************************/

/* Gets sound module ready for plugin */
extern void               sound_plugin_init();

/* init the sound library */
/* return TRUE or FALSE depending on success */
extern uint32           (*sound_init)();

/* load a song given a filename */
extern sound_st_sample *(*sound_load_music_from_file)(char *name);

/* stop playing a song */
/* return TRUE or FALSE depending on success */
extern bool             (*sound_stop)(sound_st_channel channel);

/* pause the given channel */
extern bool             (*sound_pause)(sound_st_channel channel);

/* unpause the given channel */
extern bool             (*sound_unpause)(sound_st_channel channel);

/* is the given song playing ? */
extern bool             (*sound_is_playing)(sound_st_channel channel);

/* play the given sample */
extern sound_st_channel (*sound_play_sample)(sound_st_channel channel, sound_st_sample *sample);

/* release the given sample */
extern bool             (*sound_release_sample)(sound_st_sample *sample);

/* set the master volume 0 <= vol <= 255*/
extern bool             (*sound_set_volume)(sound_st_channel channel, int32 vol);

/* Exits from the sound library */
/* return TRUE or FALSE depending on success */
extern bool             (*sound_exit)();

/* play background music with the given volume, in streaming */
extern bool             (*sound_play_bgmusic_from_file)(char *filename, int32 volume);

/* les wrappers a rajouter peut etre
FSOUND_SetCurrentPosition
FSOUND_GetCurrentPosition
FSOUND_SetDefaults
FSOUND_SetLoopMode
FSOUND_Sample_Alloc
FSOUND_Sample_Upload
FSOUND_Sample_Lock
FSOUND_Sample_Unlock
*/

#endif /* _SOUND_H_ */
