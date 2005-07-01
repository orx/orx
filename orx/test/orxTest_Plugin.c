/**
 * @file orxTest_Plugin.c
 * 
 * Plugin load / UnLoad Test Program
 * 
 */
 
 /***************************************************************************
 orxTest_Plugin.c
 Plugin load / UnLoad Test Program
 
 begin                : 1/07/2005
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
#include "utils/orxTest.h"
#include "utils/orxString.h"
#include "io/orxTextIO.h"
#include "plugin/orxPlugin.h"
#include "plugin/orxPluginCore.h"

/******************************************************
 * DEFINES
 ******************************************************/
#define orxTEST_PLUGINS_KU32_PLUGINS_NAME_SIZE  16  /**< Maximum size (number of characters) for a plugin */
#define orxTEST_PLUGINS_KU32_NB_COLS 80

#define orxTEST_PLUGINS_KZ_UNLOADED_NAME "Not loaded"
#define orxTEST_PLUGINS_KZ_UNKNOWN_NAME  "Unknown"
#define orxTEST_PLUGINS_KZ_PLUGINS_TITLE "LOADED PLUGINS"

/******************************************************
 * TYPES AND STRUCTURES
 ******************************************************/

typedef struct __orxTEST_PLUGINS_t
{
  orxHANDLE hPlugin;
  orxCHAR zType[orxTEST_PLUGINS_KU32_PLUGINS_NAME_SIZE];
  orxCHAR zFile[orxTEST_PLUGINS_KU32_PLUGINS_NAME_SIZE];
} orxTEST_PLUGINS;

typedef struct __orxTEST_PLUGINS_STATIC_t
{
  orxTEST_PLUGINS astPlugins[orxPLUGIN_CORE_ID_NUMBER];
} orxTEST_PLUGINS_STATIC;

orxSTATIC orxTEST_PLUGINS_STATIC sstTest_Plugin;
/******************************************************
 * TEST FUNCTIONS
 ******************************************************/

/** Display informations about this test module
 */
orxVOID orxTest_Plugin_Infos()
{
  orxTextIO_PrintLn("This test module will allow you to load orx plugin");
  orxTextIO_PrintLn("These plugins are needed to implement some orx core functionalities");
  orxTextIO_PrintLn("You can load \"user\" plugins too, to extend orx sets of functions");
}

/** Display the list of loaded plugins
 */
orxVOID orxTest_Plugin_ShowList()
{
  orxU32 u32Cols;
  orxU32 u32Index;
  orxU32 u32Index2;
  orxCHAR zPluginInfos[64];
  
  /* Print a line of stars */
  for (u32Index = 0; u32Index < orxTEST_PLUGINS_KU32_NB_COLS; u32Index++)
  {
    orxTextIO_Print("*");
  }
  
  /* Compute the number of space to print */
  u32Cols = (orxTEST_PLUGINS_KU32_NB_COLS - 2 - orxString_Length(orxTEST_PLUGINS_KZ_PLUGINS_TITLE)) / 2;
   
  /* Print Title */
  orxTextIO_Print("\n*");  
  for (u32Index = 0; u32Index < u32Cols; u32Index++)
  {
    orxTextIO_Print(" ");
  }
  orxTextIO_Print(orxTEST_PLUGINS_KZ_PLUGINS_TITLE);
  for (u32Index = 0; u32Index < u32Cols; u32Index++)
  {
    orxTextIO_Print(" ");
  }
  orxTextIO_PrintLn("*");  

  /* Print a line of stars */
  for (u32Index = 0; u32Index < orxTEST_PLUGINS_KU32_NB_COLS; u32Index++)
  {
    orxTextIO_Print("*");
  }
  orxTextIO_PrintLn("");  
  
  /* Print list of loaded plugins */
  for (u32Index = 0; u32Index < orxPLUGIN_CORE_ID_NUMBER; u32Index++)
  {
    /* carriage return if printing the second columns */
    if (((u32Index % 2) == 0) && (u32Index != 0))
    {
      orxTextIO_PrintLn("*");
    }

    /* Print cell content */
    orxTextIO_Printf(zPluginInfos, "* %s : %s", sstTest_Plugin.astPlugins[u32Index].zType, sstTest_Plugin.astPlugins[u32Index].zFile);
    orxTextIO_Print(zPluginInfos);
    
    /* Print spaces */
    for (u32Cols = 0; u32Cols < (orxTEST_PLUGINS_KU32_NB_COLS / 2) - orxString_Length(zPluginInfos) - (u32Index % 2); u32Cols++)
    {
      orxTextIO_Print(" ");
    }
  }
  orxTextIO_PrintLn("");
  
  /* Print a line of stars */
  for (u32Index = 0; u32Index < orxTEST_PLUGINS_KU32_NB_COLS; u32Index++)
  {
    orxTextIO_Print("*");
  }
  orxTextIO_PrintLn("");

  /* Everythings done */
  orxTextIO_PrintLn("Done !");
}

/** Load a plugin
 */
orxVOID orxTest_Plugin_Load()
{
  /* Everythings done */
  orxTextIO_PrintLn("Done !");
}

/** Unload a plugin
 */
orxVOID orxTest_Plugin_Unload()
{

  /* Everythings done */
  orxTextIO_PrintLn("Done !");
}

/******************************************************
 * DYNAMIC LIBRARY ENTRY POINTS
 ******************************************************/
orxVOID orxTest_Plugin_Init()
{
  orxU32 u32Index;
    
  /* Initialize Memory module */
  orxPlugin_Init();
  
  /* Register test functions */
  orxTest_Register("Plugin", "Display module informations", orxTest_Plugin_Infos);
  orxTest_Register("Plugin", "Display list of loaded plugins", orxTest_Plugin_ShowList);
  orxTest_Register("Plugin", "Load a plugin", orxTest_Plugin_Load);
  orxTest_Register("Plugin", "Unload a plugin", orxTest_Plugin_Unload);
  
  /* Initialize plugins array content */
  for (u32Index = 0; u32Index < orxPLUGIN_CORE_ID_NUMBER; u32Index++)
  {
    sstTest_Plugin.astPlugins[u32Index].hPlugin = orxHANDLE_Undefined;
    orxString_Copy(sstTest_Plugin.astPlugins[u32Index].zType, orxTEST_PLUGINS_KZ_UNKNOWN_NAME);
    orxString_Copy(sstTest_Plugin.astPlugins[u32Index].zFile, orxTEST_PLUGINS_KZ_UNLOADED_NAME);
  }
  orxString_Copy(sstTest_Plugin.astPlugins[orxPLUGIN_CORE_ID_DISPLAY].zType,  "display");
  orxString_Copy(sstTest_Plugin.astPlugins[orxPLUGIN_CORE_ID_FILE].zType,     "file");
  orxString_Copy(sstTest_Plugin.astPlugins[orxPLUGIN_CORE_ID_JOYSTICK].zType, "joystick");
  orxString_Copy(sstTest_Plugin.astPlugins[orxPLUGIN_CORE_ID_KEYBOARD].zType, "keyboard");
  orxString_Copy(sstTest_Plugin.astPlugins[orxPLUGIN_CORE_ID_MOUSE].zType,    "mouse");
  orxString_Copy(sstTest_Plugin.astPlugins[orxPLUGIN_CORE_ID_PACKAGE].zType,  "package");
  orxString_Copy(sstTest_Plugin.astPlugins[orxPLUGIN_CORE_ID_SCRIPT].zType,   "script");
  orxString_Copy(sstTest_Plugin.astPlugins[orxPLUGIN_CORE_ID_SOUND].zType,    "sound");
  orxString_Copy(sstTest_Plugin.astPlugins[orxPLUGIN_CORE_ID_TIMER].zType,    "timer");
}

orxVOID orxTest_Plugin_Exit()
{
  /* Uninitialize Memory module */
  orxPlugin_Exit();
}

orxTEST_DEFINE_ENTRY_POINT(orxTest_Plugin_Init, orxTest_Plugin_Exit)
