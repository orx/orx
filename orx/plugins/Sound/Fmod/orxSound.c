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
 *   This library is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License           *
 *   as published by the Free Software Foundation; either version 2.1      *
 *   of the License, or (at your option) any later version.                *
 *                                                                         *
 ***************************************************************************/

#include "orxInclude.h"
#include "plugin/orxPluginUser.h"
#include "debug/orxDebug.h"
#include "sound/orxSound.h"

#include <fmod/fmod.h>

#define orxSOUND_KU32_STATIC_FLAG_NONE  0x00000000  /**< No flags have been set */
#define orxSOUND_KU32_STATIC_FLAG_READY 0x00000001  /**< The module has been initialized */

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
  orxASSERT(!(sstSound.u32Flags & orxSOUND_KU32_STATIC_FLAG_READY));

  /* Cleans static controller */
  orxMemory_Zero(&sstSound, sizeof(orxSOUND_STATIC));

  /* Init FMOD */
  if(FSOUND_Init(44100, 32, 0))
  {
    /* Set module has ready */
    sstSound.u32Flags = orxSOUND_KU32_STATIC_FLAG_READY;
  }

  /* Module successfully initialized ? */
  if(sstSound.u32Flags & orxSOUND_KU32_STATIC_FLAG_READY)
  {
    return orxSTATUS_SUCCESS;
  }
  else
  {
    return orxSTATUS_FAILURE;
  }
}

/** Uninitialize the Sound Module
 */
orxVOID orxSound_Fmod_Exit()
{
  /* Module initialized ? */
  orxASSERT((sstSound.u32Flags & orxSOUND_KU32_STATIC_FLAG_READY) == orxSOUND_KU32_STATIC_FLAG_READY);
  
  /* Stop FMOD */
  FSOUND_Close();
  
  /* Module not ready now */
  sstSound.u32Flags = orxSOUND_KU32_STATIC_FLAG_NONE;
}

/** Load a sample From a file
 * @param[in] _zFileName        File name to load
 * @return Returns a sample
 */
orxSOUND_SAMPLE* orxSound_Fmod_SampleLoadFromFile(orxCONST orxSTRING _zFileName)
{
  /* Module initialized ? */
  orxASSERT((sstSound.u32Flags & orxSOUND_KU32_STATIC_FLAG_READY) == orxSOUND_KU32_STATIC_FLAG_READY);

  /* Load sample */
  return((orxSOUND_SAMPLE *)FSOUND_Sample_Load(FSOUND_FREE, _zFileName, FSOUND_NORMAL, 0, 0));
}

/** Load a sample From memory
 * @param[in] _pMem             Memory address where is stored the sample
 * @param[in] _u32Size          Size of the data block to load
 * @return Returns a pointer on the loaded sample
 */
orxSOUND_SAMPLE* orxSound_Fmod_SampleLoadFromMemory(orxCONST orxVOID *_pMem, orxU32 _u32Size)
{
  /* Module initialized ? */
  orxASSERT((sstSound.u32Flags & orxSOUND_KU32_STATIC_FLAG_READY) == orxSOUND_KU32_STATIC_FLAG_READY);

  /* Load sample */
  return((orxSOUND_SAMPLE *)FSOUND_Sample_Load(FSOUND_FREE, _pMem, FSOUND_LOADMEMORY, 0, _u32Size));
}

/** Unload a sample
 * @param[in] _pstSample        Pointer on the sample to unload
 */
orxVOID orxSound_Fmod_SampleUnload(orxSOUND_SAMPLE *_pstSample)
{
  /* Module initialized ? */
  orxASSERT((sstSound.u32Flags & orxSOUND_KU32_STATIC_FLAG_READY) == orxSOUND_KU32_STATIC_FLAG_READY);

  /* Unload sample */
  FSOUND_Sample_Free((FSOUND_SAMPLE *)_pstSample);
}

/** Play a sample
 * @param[in] _u32Channel       Channel to use (from 0 to 4096). flags can be used to automatically select a free chanel or all the channel
 * @param[in] _pstSample        Sample to play
 * @return Returns the channel used by the sample or orxSOUND_CHANNEL_KU32_ERROR if no channel available
 */
orxU32 orxSound_Fmod_SamplePlay(orxU32 _u32Channel, orxSOUND_SAMPLE *_pstSample)
{
  orxU32 u32Channel;
  orxS32 s32ReturnedChannel;
  
  /* Module initialized ? */
  orxASSERT((sstSound.u32Flags & orxSOUND_KU32_STATIC_FLAG_READY) == orxSOUND_KU32_STATIC_FLAG_READY);

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
  if(s32ReturnedChannel < 0)
  {
    /* return error */
    return orxSOUND_CHANNEL_KU32_ERROR;
  }
  else
  {
    return(orxU32)s32ReturnedChannel;
  }
}

/** Stop a sample
 * @param[in] _u32Channel       Channel to stop (orxSOUND_CHANNEL_KU32_SELECT_ALL can be used to stop all channels)
 * @return Returns the status of the operation
 */
orxSTATUS orxSound_Fmod_ChannelStop(orxU32 _u32Channel)
{
   orxU32 u32Channel;
   
  /* Module initialized ? */
  orxASSERT((sstSound.u32Flags & orxSOUND_KU32_STATIC_FLAG_READY) == orxSOUND_KU32_STATIC_FLAG_READY);

  /* Select channel */
  u32Channel = (_u32Channel == orxSOUND_CHANNEL_KU32_SELECT_ALL) ? (orxU32)FSOUND_ALL : _u32Channel;
  
  /* Stop channel */
  if(FSOUND_StopSound(u32Channel))
  {
    return orxSTATUS_SUCCESS;
  }
  else
  {
    return orxSTATUS_FAILURE;
  }
}

/** Pause/UnPause a sample
 * @param[in] _u32Channel       Channel to pause
 * @param[in] _bPause           Pause the channel
 * @return Returns the status of the operation
 */
orxSTATUS orxSound_Fmod_ChannelPause(orxU32 _u32Channel, orxBOOL _bPause)
{
   orxU32 u32Channel;

  /* Module initialized ? */
  orxASSERT((sstSound.u32Flags & orxSOUND_KU32_STATIC_FLAG_READY) == orxSOUND_KU32_STATIC_FLAG_READY);
  
  /* Select channel */
  u32Channel = (_u32Channel == orxSOUND_CHANNEL_KU32_SELECT_ALL) ? (orxU32)FSOUND_ALL : _u32Channel;
  
  /* Pause channel */
  if(FSOUND_SetPaused(u32Channel, _bPause))
  {
    return orxSTATUS_SUCCESS;
  }
  else
  {
    return orxSTATUS_FAILURE;
  }
}

/** Test Sample flags
 * @param[in] _u32Channel       Channel to test
 * @param[in] _u32FlagsToTest   Flags to tests
 * @return orxTRUE if tested flags are present
 */
orxBOOL orxSound_Fmod_ChannelTestFlags(orxU32 _u32Channel, orxU32 _u32FlagsToTest)
{
  orxU32 u32FmodFlag = 0;

  /* Module initialized ? */
  orxASSERT((sstSound.u32Flags & orxSOUND_KU32_STATIC_FLAG_READY) == orxSOUND_KU32_STATIC_FLAG_READY);

  /* Get required Fmod's flags on the channel and store them */
  if(FSOUND_IsPlaying(_u32Channel))
  {
    u32FmodFlag |= orxSOUND_STATE_KU32_STARTED;
  }
  
  if(FSOUND_GetPaused(_u32Channel))
  {
    u32FmodFlag |= orxSOUND_STATE_KU32_PAUSED;
  }
  
  if(FSOUND_GetLoopMode(_u32Channel))
  {
    u32FmodFlag |= orxSOUND_STATE_KU32_LOOP;
  }
  
  /* Return test result */
  return((u32FmodFlag & _u32FlagsToTest) == _u32FlagsToTest);
}

/** Set Sample flags
 * @param[in] _u32Channel       Channel to set
 * @param[in] _u32FlagsToRemove Flags to remove
 * @param[in] _u32FlagsToRemove Flags to add
 */
orxVOID orxSound_Fmod_ChannelSetFlags(orxU32 _u32Channel, orxU32 _u32FlagsToRemove, orxU32 _u32FlagsToAdd)
{
  /* Module initialized ? */
  orxASSERT((sstSound.u32Flags & orxSOUND_KU32_STATIC_FLAG_READY) == orxSOUND_KU32_STATIC_FLAG_READY);
  
  /* Play / Stop */
  /* Stop sound ? */
  if((_u32FlagsToRemove & orxSOUND_STATE_KU32_STARTED) == orxSOUND_STATE_KU32_STARTED)
  {
    /* Stop sound */
    FSOUND_StopSound(_u32Channel);
  }
  /* Start sound (not possible to start only a sound without giving sample pointer ? */

  /* Pause / UnPause ? */
  if((_u32FlagsToRemove & orxSOUND_STATE_KU32_PAUSED) == orxSOUND_STATE_KU32_PAUSED)
  {
    /* unpause sound */
    FSOUND_SetPaused(_u32Channel, orxFALSE);
  }

  if((_u32FlagsToAdd & orxSOUND_STATE_KU32_PAUSED) == orxSOUND_STATE_KU32_PAUSED)
  {
    /* pause sound */
    FSOUND_SetPaused(_u32Channel, orxTRUE);
  }

  /* Loop / Normal ? */
  if((_u32FlagsToRemove & orxSOUND_STATE_KU32_LOOP) == orxSOUND_STATE_KU32_LOOP)
  {
    /* normal sound */
    FSOUND_SetLoopMode(_u32Channel, FSOUND_LOOP_OFF);
  }

  if((_u32FlagsToAdd & orxSOUND_STATE_KU32_LOOP) == orxSOUND_STATE_KU32_LOOP)
  {
    /* loop sound */
    FSOUND_SetLoopMode(_u32Channel, FSOUND_LOOP_NORMAL);
  }
}

/** Set Channel volume
 * @param[in] _u32Channel       Channel to use
 * @param[in] _u8Volume         New volume
 * @return Returns the status of the operation
 */
orxSTATUS orxSound_Fmod_ChannelSetVolume(orxU32 _u32Channel, orxU8 _u8Volume)
{
  /* Module initialized ? */
  orxASSERT((sstSound.u32Flags & orxSOUND_KU32_STATIC_FLAG_READY) == orxSOUND_KU32_STATIC_FLAG_READY);

  /* Set channel volume */
  if(FSOUND_SetVolume(_u32Channel, _u8Volume))
  {
    return orxSTATUS_SUCCESS;
  }
  else
  {
    return orxSTATUS_FAILURE;
  }
}


/***************************************************************************
 * Plugin Related                                                          *
 ***************************************************************************/

/***************************************************************************
 * Plugin Related                                                          *
 ***************************************************************************/
orxPLUGIN_USER_CORE_FUNCTION_START(SOUND);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSound_Fmod_Init, SOUND, INIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSound_Fmod_Exit, SOUND, EXIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSound_Fmod_SampleLoadFromFile, SOUND, SAMPLE_LOAD_FROM_FILE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSound_Fmod_SampleLoadFromMemory, SOUND, SAMPLE_LOAD_FROM_MEMORY);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSound_Fmod_SampleUnload, SOUND, SAMPLE_UNLOAD);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSound_Fmod_SamplePlay, SOUND, SAMPLE_PLAY);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSound_Fmod_ChannelStop, SOUND, CHANNEL_STOP);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSound_Fmod_ChannelPause, SOUND, CHANNEL_PAUSE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSound_Fmod_ChannelTestFlags, SOUND, CHANNEL_TEST_FLAGS);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSound_Fmod_ChannelSetFlags, SOUND, CHANNEL_SET_FLAGS);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSound_Fmod_ChannelSetVolume, SOUND, CHANNEL_SET_VOLUME);
orxPLUGIN_USER_CORE_FUNCTION_END();
