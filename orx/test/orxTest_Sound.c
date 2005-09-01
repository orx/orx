/**
 * @file orxTest_Sound.c
 * 
 * Sound Test Program
 * 
 */
 
 /***************************************************************************
 orxTest_Sound.c
 Sound Test Program
 
 begin                : 22/05/2005
 author               : (C) Arcallians
 email                : bestel@arcallians.org
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
#include "memory/orxMemory.h"
#include "utils/orxTest.h"
#include "io/orxTextIO.h"
#include "plugin/orxPlugin.h"
#include "sound/orxSound.h"
#include "memory/orxBank.h"

/******************************************************
 * DEFINES
 ******************************************************/

#define orxTEST_SOUND_SAMPLE_MAX 16         /* Maximum number of loaded sample */
#define orxTEST_SOUND_SAMPLE_NAME_MAX 128   /* 128 characters maximum per sample file name */

/******************************************************
 * TYPES AND STRUCTURES
 ******************************************************/

typedef struct __orxTEST_SOUND_SAMPLE_INFOS_t
{
    orxSOUND_SAMPLE *pstSample;                         /* Sample address */
    orxCHAR zFileName[orxTEST_SOUND_SAMPLE_NAME_MAX];   /* Sample file name */
} orxTEST_SOUND_SAMPLE_INFOS;

typedef struct __orxTEST_SOUND_t
{
    orxBANK *pstSampleBank; /* Loaded bank list */
} orxTEST_SOUND;

orxSTATIC orxTEST_SOUND sstTest_Sound;

/******************************************************
 * PRIVATE FUNCTIONS
 ******************************************************/

/******************************************************
 * TEST FUNCTIONS
 ******************************************************/

/** Display informations about this test module
 */
orxVOID orxTest_Sound_Infos()
{
  orxTextIO_PrintLn("This test module is able to manage sound");
}

/** Load a sample in memory from a file
 */
orxVOID orxTest_Sound_SampleLoadFromFile()
{
  orxTEST_SOUND_SAMPLE_INFOS stSampleInfos;   /* Local sample infos */
  orxTEST_SOUND_SAMPLE_INFOS *pstSampleInfos; /* Stored sample infos */

  /* Print main instructions */
  orxTextIO_PrintLn("This function will allow you to load a Sample file.");
  orxTextIO_PrintLn("It will print the Sample ID. A function allows you to");
  orxTextIO_PrintLn("Display the list of loaded Sample and address");
  
  /* Request sample file name */
  while ((orxTextIO_ReadString(stSampleInfos.zFileName, orxTEST_SOUND_SAMPLE_NAME_MAX, "Enter the sample file name to load : ") == orxSTATUS_FAILED))
  {}
  
  /* Try to load the sample */
  stSampleInfos.pstSample = orxSound_SampleLoadFromFile(stSampleInfos.zFileName);
  
  /* Loaded ? */
  if (stSampleInfos.pstSample != orxNULL)
  {
    /* Yes, store it */
    pstSampleInfos = (orxTEST_SOUND_SAMPLE_INFOS *)orxBank_Allocate(sstTest_Sound.pstSampleBank);
    
    /* Copy loaded datas */
    *pstSampleInfos = stSampleInfos;
    
    /* Display Sample Address */
    orxTextIO_PrintLn("Loaded Sample address ID : %x", pstSampleInfos);
  }
  else
  {
    orxTextIO_PrintLn("Can't load sample file (not found or not a valid format)");
  }
}

/** Unload a sample
 */
orxVOID orxTest_Sound_SampleUnload()
{
  orxS32 s32SampleID;                           /* Sample ID to read from user */
  orxTEST_SOUND_SAMPLE_INFOS *pstSampleInfos;   /* Sample infos */
  
  /* Print main instructions */
  orxTextIO_PrintLn("This function will allow you to Unload a Sample.");

  /* Read the sample address */
  orxTextIO_ReadS32(&s32SampleID, 16, "Enter the sample ID to unload : ", orxTRUE);
  
  pstSampleInfos = orxNULL;
  
  /* Traverse Bank and check if the entered address has really been allocated */
  while ((pstSampleInfos = orxBank_GetNext(sstTest_Sound.pstSampleBank, pstSampleInfos)))
  {}
  
  /* Retrieve sample information from read address */
  if ((orxS32)pstSampleInfos == s32SampleID)
  {
    /* unload sample */
    orxSound_SampleUnload(pstSampleInfos->pstSample);
    
    /* Free cell */
    orxMemory_Set(pstSampleInfos, 0, sizeof(orxTEST_SOUND_SAMPLE_INFOS));
    orxBank_Free(sstTest_Sound.pstSampleBank, pstSampleInfos);
  }
  else
  {
    orxTextIO_PrintLn("Invalid Sample ID");
  }
}

/** Send a sample on a chanel
 */
orxVOID orxTest_Sound_SamplePlay()
{
  orxS32 s32Channel;                            /* Channel to use */
  orxS32 s32SampleID;                           /* Sample ID to read from user */
  orxTEST_SOUND_SAMPLE_INFOS *pstSampleInfos;   /* Sample infos */

  /* Print main instructions */
  orxTextIO_PrintLn("This function will send a loaded sample on a channel.");
  
  /* Read the sample address */
  orxTextIO_ReadS32(&s32SampleID, 16, "Enter the sample ID to play : ", orxTRUE);
  
  pstSampleInfos = orxNULL;
  
  /* Traverse Bank and check if the entered address has really been allocated */
  while ((pstSampleInfos = orxBank_GetNext(sstTest_Sound.pstSampleBank, pstSampleInfos)))
  {}
  
  /* Retrieve sample information from read address */
  if ((orxS32)pstSampleInfos == s32SampleID)
  {
    /* Sample has been found... Select the channel ID to use */
    orxTextIO_PrintLn("Enter the channel to use (between 0 and %d).", orxSOUND_CHANNEL_KU32_NUMBER - 1);
    orxTextIO_PrintLn("Enter %d to automatically select a free channel.", orxSOUND_CHANNEL_KU32_SELECT_FREE);
    orxTextIO_ReadS32InRange(&s32Channel, 10, 0, orxSOUND_CHANNEL_KU32_SELECT_FREE, "Channel : ", orxTRUE);
    
    /* Try to play the sample */
    orxTextIO_PrintLn("Trying to play %s...", pstSampleInfos->zFileName);
    s32Channel = orxSound_SamplePlay(s32Channel, pstSampleInfos->pstSample);
    
    if (s32Channel != orxSOUND_CHANNEL_KU32_ERROR)
    {
      orxTextIO_PrintLn("Play %s on channel %d", pstSampleInfos->zFileName, s32Channel);
    }
    else
    {
      orxTextIO_PrintLn("Error : Can't send sample on channel...");
    }
  }
  else
  {
    orxTextIO_PrintLn("Invalid Sample ID");
  }
}

/** Stop a chanel
 */
orxVOID orxTest_Sound_ChannelStop()
{
  orxS32 s32Channel;  /* Channel to use */

  /* Print main instructions */
  orxTextIO_PrintLn("This function will stop a channel.");

  /* Read the channel number */
  orxTextIO_PrintLn("Enter the channel ID to stop (between 0 and %d)", orxSOUND_CHANNEL_KU32_NUMBER - 1);
  orxTextIO_PrintLn("%d is a special number that will stop all the channels", orxSOUND_CHANNEL_KU32_SELECT_ALL);
  orxTextIO_ReadS32InRange(&s32Channel, 10, 0, orxSOUND_CHANNEL_KU32_SELECT_ALL, "Channel : ", orxTRUE);
  
  if (orxSound_ChannelStop(s32Channel) != orxSTATUS_FAILED)
  {
    /* Operation succeded */
    orxTextIO_PrintLn("Channel(s) stopped");
  }
  else
  {
    /* Operation failed */
    orxTextIO_PrintLn("Operation failed");
  }
}

/** Pause a chanel
 */
orxVOID orxTest_Sound_ChannelPause()
{
  orxS32 s32Channel;  /* Channel to use */

  /* Print main instructions */
  orxTextIO_PrintLn("This function will pause / unpause a channel.");
  orxTextIO_PrintLn("WARNING : This function make a call to orxSound_TestFlags to know what is the current pause state of the channel");
  orxTextIO_PrintLn("        : BE SURE TO TEST THAT THIS FUNCTION WORKS BEFORE TO TEST THIS ONE. (Testbed function that print used flags is the one that allow this test)");

  /* Read the channel number */
  orxTextIO_PrintLn("Enter the channel ID to pause / unpause (between 0 and %d)", orxSOUND_CHANNEL_KU32_NUMBER - 1);
  orxTextIO_ReadS32InRange(&s32Channel, 10, 0, orxSOUND_CHANNEL_KU32_SELECT_ALL, "Channel : ", orxTRUE);

  /* Is it already paused ? */
  if (orxSound_ChannelTestFlags(s32Channel, orxSOUND_STATE_KU32_PAUSED) == orxSOUND_STATE_KU32_PAUSED)
  {
    /* Yes, unpause it */
    orxSound_ChannelPause(s32Channel, orxFALSE);
  }
  else
  {
    /* Not paused, pause it */
    orxSound_ChannelPause(s32Channel, orxTRUE);
  }
}

/** Show chanel flags
 */
orxVOID orxTest_Sound_ChannelShowFlags()
{
  orxS32 s32Channel;  /* Channel to use */

  /* Print main instructions */
  orxTextIO_PrintLn("This function will print channel's flags.");

  /* Read the channel number */
  orxTextIO_PrintLn("Enter the channel ID to pause / unpause (between 0 and %d)", orxSOUND_CHANNEL_KU32_NUMBER - 1);
  orxTextIO_ReadS32InRange(&s32Channel, 10, 0, orxSOUND_CHANNEL_KU32_NUMBER - 1, "Channel : ", orxTRUE);
  
  orxTextIO_PrintLn("*** FLAGS ***");
  
  /* Channel started ? */
  orxTextIO_Print("SAMPLE STARTED : ");
  if (orxSound_ChannelTestFlags(s32Channel, orxSOUND_STATE_KU32_STARTED) == orxSOUND_STATE_KU32_STARTED)
  {
    orxTextIO_PrintLn("YES");
  }
  else
  {
    orxTextIO_PrintLn("NO");
  }
  
  /* Channel paused ? */
  orxTextIO_Print("SAMPLE PAUSED : ");
  if (orxSound_ChannelTestFlags(s32Channel, orxSOUND_STATE_KU32_PAUSED) == orxSOUND_STATE_KU32_PAUSED)
  {
    orxTextIO_PrintLn("YES");
  }
  else
  {
    orxTextIO_PrintLn("NO");
  }
  
  /* Channel loop ? */
  orxTextIO_Print("SAMPLE LOOP : ");
  if (orxSound_ChannelTestFlags(s32Channel, orxSOUND_STATE_KU32_LOOP) == orxSOUND_STATE_KU32_LOOP)
  {
    orxTextIO_PrintLn("YES");
  }
  else
  {
    orxTextIO_PrintLn("NO");
  }
}

/** Set new chanel flags
 */
orxVOID orxTest_Sound_ChannelSetFlags()
{
  orxS32 s32Channel;  /* Channel to use */
  orxS32 s32Flags;    /* Flags to set */

  /* Print main instructions */
  orxTextIO_PrintLn("This function will set flags on a channel.");

  /* Read the channel number */
  orxTextIO_PrintLn("Enter the channel ID to set (between 0 and %d)", orxSOUND_CHANNEL_KU32_NUMBER - 1);
  orxTextIO_ReadS32InRange(&s32Channel, 10, 0, orxSOUND_CHANNEL_KU32_NUMBER - 1, "Channel : ", orxTRUE);
  
  /* Read the flags to set */
  orxTextIO_PrintLn("Available flags list :");
  orxTextIO_PrintLn("orxSOUND_STATE_KU32_STARTED = %08x", orxSOUND_STATE_KU32_STARTED);
  orxTextIO_PrintLn("orxSOUND_STATE_KU32_PAUSED = %08x", orxSOUND_STATE_KU32_PAUSED);
  orxTextIO_PrintLn("orxSOUND_STATE_KU32_LOOP = %08x", orxSOUND_STATE_KU32_LOOP);
  orxTextIO_ReadS32(&s32Flags, 10, "Flags to set on the channel : ", orxTRUE);

  /* Remove all flags and set new ones */
  orxSound_ChannelSetFlags(s32Channel, 0xFFFFFFFF, s32Flags);
} 

/** Set sound volume (on a chanel)
 */
orxVOID orxTest_Sound_ChannelSetVolume()
{
  orxS32 s32Channel;  /* Channel to use */
  orxS32 s32Volume;   /* new channel volume */

  /* Print main instructions */
  orxTextIO_PrintLn("This function will set new volume on a channel (to choose between 0 and 255).");

  /* Read the channel number */
  orxTextIO_PrintLn("Enter the channel ID to set (between 0 and %d)", orxSOUND_CHANNEL_KU32_NUMBER - 1);
  orxTextIO_ReadS32InRange(&s32Channel, 10, 0, orxSOUND_CHANNEL_KU32_NUMBER - 1, "Channel : ", orxTRUE);

  /* Read the new volume */
  orxTextIO_ReadS32InRange(&s32Volume, 10, 0, 255, "Enter the volume to set : ", orxTRUE);
  
  /* Set the new volume */
  orxSound_ChannelSetVolume(s32Channel, (orxU8)s32Volume);
}

/******************************************************
 * DYNAMIC LIBRARY ENTRY POINTS
 ******************************************************/
orxVOID orxTest_Sound_Init()
{
  orxMAIN_INIT_MODULE(Plugin);  /* Initialize Plugin module */
  orxSound_Plugin_Init();
  
  /* orxSound_Init (orxMAIN_INIT_MODULE(Sound)) have to be called after the plugin
   * loading. I have to find a way to do it
   */
  
  /* Register test functions */
  orxTest_Register("Sound", "Display module informations", orxTest_Sound_Infos);
  orxTest_Register("Sound", "Load a sample from a file", orxTest_Sound_SampleLoadFromFile);
  orxTest_Register("Sound", "Unload a sample", orxTest_Sound_SampleUnload);
  orxTest_Register("Sound", "Play a sample to a chanel", orxTest_Sound_SamplePlay);
  orxTest_Register("Sound", "Stop a chanel", orxTest_Sound_ChannelStop);
  orxTest_Register("Sound", "Pause a chanel", orxTest_Sound_ChannelPause);
  orxTest_Register("Sound", "Show chanel flags", orxTest_Sound_ChannelShowFlags);
  orxTest_Register("Sound", "Set chanel flags", orxTest_Sound_ChannelSetFlags);
  orxTest_Register("Sound", "Set chanel volume", orxTest_Sound_ChannelSetVolume);
  
  /* Initialize static datas */
  orxMemory_Set(&sstTest_Sound, 0, sizeof(orxTEST_SOUND));
  
  /* Create samples bank */
  sstTest_Sound.pstSampleBank = orxBank_Create(orxTEST_SOUND_SAMPLE_MAX, sizeof(orxTEST_SOUND_SAMPLE_INFOS), orxBANK_KU32_FLAGS_NONE, orxMEMORY_TYPE_MAIN);
}

orxVOID orxTest_Sound_Exit()
{
  orxTEST_SOUND_SAMPLE_INFOS *pstSampleInfos;

  /* Traverse bank and unload sample */
  while ((pstSampleInfos = (orxTEST_SOUND_SAMPLE_INFOS *)orxBank_GetNext(sstTest_Sound.pstSampleBank, NULL)))
  {
    /* Unload a loaded sample */
    orxSound_SampleUnload(pstSampleInfos->pstSample);
  }
  
  /* Destroy bank */
  orxBank_Delete(sstTest_Sound.pstSampleBank);
    
  /* Uninitialize module */
  orxMAIN_EXIT_MODULE(Plugin);
}

orxTEST_DEFINE_ENTRY_POINT(orxTest_Sound_Init, orxTest_Sound_Exit)
