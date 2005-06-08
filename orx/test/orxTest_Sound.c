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

/******************************************************
 * DEFINES
 ******************************************************/

/******************************************************
 * TYPES AND STRUCTURES
 ******************************************************/

typedef struct __orxTEST_SOUND_t
{
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
orxVOID orxTest_Sound_LoadSampleFromFile()
{
}

/** Unload a sample
 */
orxVOID orxTest_Sound_UnloadSample()
{
}

/** Send a sample on a chanel
 */
orxVOID orxTest_Sound_PlaySample()
{
}

/** Stop a chanel
 */
orxVOID orxTest_Sound_Stop()
{
}

/** Pause a chanel
 */
orxVOID orxTest_Sound_Pause()
{
}

/** Show chanel flags
 */
orxVOID orxTest_Sound_ShowFlags()
{
}

/** Set new chanel flags
 */
orxVOID orxTest_Sound_SetFlags()
{
}

/** Set sound volume (on a chanel)
 */
orxVOID orxTest_Sound_SetVolume()
{
}

/******************************************************
 * DYNAMIC LIBRARY ENTRY POINTS
 ******************************************************/
orxVOID orxTest_Sound_Init()
{
  /* Initialize bank module */
  orxMemory_Init();
  orxPlugin_Init();
  orxSound_Plugin_Init();
  orxSound_Init();
  
  /* Register test functions */
  orxTest_Register("Sound", "Display module informations", orxTest_Sound_Infos);
  orxTest_Register("Sound", "Load a sample from a file", orxTest_Sound_LoadSampleFromFile);
  orxTest_Register("Sound", "Unload a sample", orxTest_Sound_UnloadSample);
  orxTest_Register("Sound", "Play a sample to a chanel", orxTest_Sound_PlaySample);
  orxTest_Register("Sound", "Stop a chanel", orxTest_Sound_Stop);
  orxTest_Register("Sound", "Pause a chanel", orxTest_Sound_Pause);
  orxTest_Register("Sound", "Show chanel flags", orxTest_Sound_ShowFlags);
  orxTest_Register("Sound", "Set chanel flags", orxTest_Sound_SetFlags);
  orxTest_Register("Sound", "Set chanel volume", orxTest_Sound_SetVolume);
  
  /* Initialize static datas */
  orxMemory_Set(&sstTest_Sound, 0, sizeof(orxTEST_SOUND));
}

orxVOID orxTest_Sound_Exit()
{
  /* Uninitialize module */
  orxSound_Exit();
  orxPlugin_Exit();
  orxMemory_Exit();
}

orxTEST_DEFINE_ENTRY_POINT(orxTest_Sound_Init, orxTest_Sound_Exit)
