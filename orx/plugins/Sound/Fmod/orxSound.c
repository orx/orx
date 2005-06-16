/**
 * @file orxSound.c
 */

/***************************************************************************
 orxSound.c
 FMOD implementation for the Sound plugin
 begin                : 23/07/2002
                        14/11/2003
                        15/06/2005
 author               : (C) Arcallians
 email                : snegri@free.fr
                        iarwain@arcallians.org
                        bestel@arcallians.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "orxInclude.h"
#include "plugin/orxPluginUser.h"
#include "debug/orxDebug.h"
#include "sound/orxSound.h"

#include <fmod.h>

#define orxSOUND_KU32_FLAG_NONE  0x00000000  /**< No flags have been set */
#define orxSOUND_KU32_FLAG_READY 0x00000001  /**< The module has been initialized */
/*
typedef FSOUND_SAMPLE GP_SOUND_SAMPLE;
typedef int32         GP_SOUND_CHANNEL;
typedef FSOUND_STREAM GP_SOUND_STREAM;

#define K_GP_SOUND_FREE    FSOUND_FREE   
#define K_GP_SOUND_NORMAL  FSOUND_NORMAL
#define K_GP_SOUND_ALL     FSOUND_ALL
#define K_GP_SOUND_LOOP_NORMAL FSOUND_LOOP_NORMAL
    
#define K_GP_SOUND_BEEP_CHANNEL 15
#define K_GP_SOUND_BG_CHANNEL   16
*/

/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

typedef struct __orxSOUND_STATIC_t
{
  orxU32 u32Flags;         /**< Flags set by the mouse plugin module */
} orxSOUND_STATIC;

/***************************************************************************
 * Module global variable                                                  *
 ***************************************************************************/
orxSTATIC orxSOUND_STATIC sstSound;

/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/


/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** Initialize the Sound Module
 * @return Returns the status of the module initialization
 */
orxSTATUS orxSound_Fmod_Init()
{
  /* Module not already initialized ? */
  orxASSERT(!(sstSound.u32Flags & orxSOUND_KU32_FLAG_READY));

  /* Cleans static controller */
  orxMemory_Set(&sstSound, 0, sizeof(orxSOUND_STATIC));

  /* Init FMOD */
  if (FSOUND_Init(44100, 32, 0))
  {
    /* Set module has ready */
    sstSound.u32Flags = orxSOUND_KU32_FLAG_READY;
  }

  /* Module successfully initialized ? */
  if (sstSound.u32Flags & orxSOUND_KU32_FLAG_READY == orxSOUND_KU32_FLAG_READY)
  {
    return orxSTATUS_SUCCESS;
  }
  else
  {
    return orxSTATUS_FAILED;
  }
}

/** Uninitialize the Sound Module
 */
orxVOID orxSound_Fmod_Exit()
{
  /* Module initialized ? */
  orxASSERT((sstSound.u32Flags & orxSOUND_KU32_FLAG_READY) == orxSOUND_KU32_FLAG_READY);
  
  /* Stop FMOD */
  FSOUND_Close();
  
  /* Module not ready now */
  sstSound.u32Flags = orxSOUND_KU32_FLAG_NONE;
}

/** Load a sample From a file
 * @param[in] _zFileName        File name to load
 * @return Returns a sample
 */
orxSOUND_SAMPLE* orxSound_Fmod_SampleLoadFromFile(orxCONST orxSTRING _zFileName)
{
  /* Module initialized ? */
  orxASSERT((sstSound.u32Flags & orxSOUND_KU32_FLAG_READY) == orxSOUND_KU32_FLAG_READY);

  /* Load sample */
  return ((orxSOUND_SAMPLE *)FSOUND_Sample_Load(FSOUND_FREE, _zFileName, FSOUND_NORMAL, 0, 0));
}

/** Load a sample From memory
 * @param[in] _pMem             Memory address where is stored the sample
 * @param[in] _u32Size          Size of the data block to load
 * @return Returns a pointer on the loaded sample
 */
orxSOUND_SAMPLE* orxSound_Fmod_SampleLoadFromMemory(orxCONST orxVOID *_pMem, orxU32 _u32Size)
{
  /* Module initialized ? */
  orxASSERT((sstSound.u32Flags & orxSOUND_KU32_FLAG_READY) == orxSOUND_KU32_FLAG_READY);

  /* Load sample */
  return ((orxSOUND_SAMPLE *)FSOUND_Sample_Load(FSOUND_FREE, _pMem, FSOUND_LOADMEMORY, 0, _u32Size));
}

/** Unload a sample
 * @param[in] _pstSample        Pointer on the sample to unload
 */
orxVOID orxSound_Fmod_SampleUnload(orxSOUND_SAMPLE *_pstSample)
{
  /* Module initialized ? */
  orxASSERT((sstSound.u32Flags & orxSOUND_KU32_FLAG_READY) == orxSOUND_KU32_FLAG_READY);

  /* Unload sample */
  FSOUND_Sample_Free((FSOUND_SAMPLE *)_pstSample);
}

/** Play a sample
 * @param[in] _u32Channel       Channel to use (from 0 to 4096). flags can be used to automatically select a free chanel or all the channel
 * @param[in] _pstSample        Sample to play
 * @return Returns the channel used by the sample or orxSOUND_CHANNEL_KU32_ERROR if no channel available
 */
orxU32 orxSound_Fmod_PlaySample(orxU32 _u32Channel, orxSOUND_SAMPLE *_pstSample)
{
  orxU32 u32Channel;
  orxS32 s32ReturnedChannel;
  
  /* Module initialized ? */
  orxASSERT((sstSound.u32Flags & orxSOUND_KU32_FLAG_READY) == orxSOUND_KU32_FLAG_READY);

  /* Select FMOD channel */
  switch (_u32Channel)
  {
    case orxSOUND_CHANNEL_KU32_SELECT_FREE:
      u32Channel = FSOUND_FREE;
      break;
    case orxSOUND_CHANNEL_KU32_SELECT_ALL:
      u32Channel = FSOUND_ALL;
      break;
    default:
      u32Channel = orxSOUND_CHANNEL_KU32_ERROR;
      break;
  }
  
  /* Try to play sound */
  s32ReturnedChannel = FSOUND_PlaySound(u32Channel, (FSOUND_SAMPLE *)_pstSample);
  
  /* Correct channel ? */
  if (s32ReturnedChannel < 0)
  {
    /* return error */
    return orxSOUND_CHANNEL_KU32_ERROR;
  }
  else
  {
    return (orxU32)s32ReturnedChannel;
  }
}

/** Stop a sample
 * @param[in] _u32Channel       Channel to stop (orxSOUND_CHANNEL_KU32_SELECT_ALL can be used to stop all channels)
 * @return Returns the status of the operation
 */
orxSTATUS orxSound_Fmod_Stop(orxU32 _u32Channel)
{
   orxU32 u32Channel;
   
  /* Module initialized ? */
  orxASSERT((sstSound.u32Flags & orxSOUND_KU32_FLAG_READY) == orxSOUND_KU32_FLAG_READY);

  /* Select channel */
  if (_u32Channel == orxSOUND_CHANNEL_KU32_SELECT_ALL)
  {
    u32Channel = FSOUND_ALL;
  }
  else
  {
    u32Channel = _u32Channel;
  }
  
  /* Stop channel */
  if (FSOUND_StopSound(u32Channel))
  {
    return orxSTATUS_SUCCESS;
  }
  else
  {
    return orxSTATUS_FAILED;
  }
}

/** Pause/UnPause a sample
 * @param[in] _u32Channel       Channel to pause
 * @param[in] _bPause           Pause the channel
 * @return Returns the status of the operation
 */
orxSTATUS orxSound_Fmod_Pause(orxU32 _u32Channel, orxBOOL _bPause)
{
  /* Module initialized ? */
  orxASSERT((sstSound.u32Flags & orxSOUND_KU32_FLAG_READY) == orxSOUND_KU32_FLAG_READY);
/* TODO
    return FSOUND_SetPaused(channel, TRUE);
    return FSOUND_SetPaused(channel, FALSE);
*/
}

/** Test Sample flags
 * @param[in] _u32Channel       Channel to test
 * @param[in] _u32FlagsToTest   Flags to tests
 * @return orxTRUE if tested flags are present
 */
orxBOOL orxSound_Fmod_TestFlags(orxU32 _u32Channel, orxU32 _u32FlagsToTest)
{
  /* Module initialized ? */
  orxASSERT((sstSound.u32Flags & orxSOUND_KU32_FLAG_READY) == orxSOUND_KU32_FLAG_READY);

/* TODO
    return FSOUND_IsPlaying(channel);
*/
}

/** Set Sample flags
 * @param[in] _u32Channel       Channel to set
 * @param[in] _u32FlagsToRemove Flags to remove
 * @param[in] _u32FlagsToRemove Flags to add
 */
orxVOID orxSound_Fmod_SetFlags(orxU32 _u32Channel, orxU32 _u32FlagsToRemove, orxU32 _u32FlagsToAdd)
{
  /* Module initialized ? */
  orxASSERT((sstSound.u32Flags & orxSOUND_KU32_FLAG_READY) == orxSOUND_KU32_FLAG_READY);
}

/** Set Channel volume
 * @param[in] _u32Channel       Channel to use
 * @param[in] _u8Volume         New volume
 */
orxVOID orxSound_Fmod_SetVolume(orxU32 _u32Channel, orxU8 _u8Volume)
{
  /* Module initialized ? */
  orxASSERT((sstSound.u32Flags & orxSOUND_KU32_FLAG_READY) == orxSOUND_KU32_FLAG_READY);

    /* range from 0(silent) to 255(full) */

/* TODO
    if (vol < 0 || vol > 255)
        return FALSE;
    
    return FSOUND_SetVolume(channel, vol);
*/
}


/***************************************************************************
 * Plugin Related                                                          *
 ***************************************************************************/

/***************************************************************************
 * Plugin Related                                                          *
 ***************************************************************************/
orxSTATIC orxPLUGIN_USER_FUNCTION_INFO sastSound_Function[orxPLUGIN_FUNCTION_BASE_ID_SOUND_NUMBER];

orxVOID orxPlugin_Init(orxS32 *_ps32Number, orxPLUGIN_USER_FUNCTION_INFO **_ppstInfos)
{
  orxPLUGIN_USER_FUNCTION_START(sastSound_Function);
  orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSound_Fmod_Init, SOUND, INIT);
  orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSound_Fmod_Exit, SOUND, EXIT);
  orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSound_Fmod_SampleLoadFromFile, SOUND, LOAD_SAMPLE_FROM_FILE);
  orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSound_Fmod_SampleLoadFromMemory, SOUND, LOAD_SAMPLE_FROM_MEMORY);
  orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSound_Fmod_SampleUnload, SOUND, UNLOAD_SAMPLE);
  orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSound_Fmod_PlaySample, SOUND, PLAY_SAMPLE);
  orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSound_Fmod_Stop, SOUND, STOP);
  orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSound_Fmod_Pause, SOUND, PAUSE);
  orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSound_Fmod_TestFlags, SOUND, TEST_FLAGS);
  orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSound_Fmod_SetFlags, SOUND, SET_FLAGS);
  orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSound_Fmod_SetVolume, SOUND, SET_VOLUME);
  orxPLUGIN_USER_FUNCTION_END(_ps32Number, _ppstInfos);
  return;
}
