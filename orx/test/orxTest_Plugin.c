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
#include "memory/orxMemory.h"
#include "debug/orxTest.h"
#include "utils/orxString.h"
#include "io/orxTextIO.h"
#include "plugin/orxPlugin.h"
#include "plugin/orxPluginCore.h"

/* Inlcude the list of used core plugin API (needed to call _Init and _Exit functions) */
#include "display/orxDisplay.h"
#include "io/orxFile.h"
#include "io/orxJoystick.h"
#include "io/orxKeyboard.h"
#include "io/orxMouse.h"
#include "io/orxPackage.h"
#include "script/orxScript.h"
#include "sound/orxSound.h"
#include "core/orxTime.h"

/* Include commons libc header */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* Incude specific header files according to used platform */
#ifdef __orxLINUX__
  #include <sys/types.h>
  #include <dirent.h>
  #include <dlfcn.h>

  /* Define the seperator character for directories */
  #define DIRSEP "/"

#else
  #ifdef __orxWINDOWS__
    #include <io.h>

    /* Define the seperator character for directories */
    #define DIRSEP "\\"

  #endif /* __orxWINDOWS__ */
#endif /* __orxLINUX__ */


/******************************************************
 * DEFINES
 ******************************************************/
 
#define orxTEST_PLUGINS_KU32_PLUGINS_NAME_SIZE    64  /**< Maximum size (number of characters) for a plugin */
#define orxTEST_PLUGINS_KU32_NB_COLS              80

#define orxTEST_PLUGINS_KU32_MAX_PLUGINS_PER_TYPE 10

#define orxTEST_PLUGINS_KZ_UNLOADED_NAME          "Not loaded"
#define orxTEST_PLUGINS_KZ_UNKNOWN_NAME           "Unknown"
#define orxTEST_PLUGINS_KZ_PLUGINS_TITLE          "LOADED PLUGINS"


/******************************************************
 * TYPES AND STRUCTURES
 ******************************************************/

typedef struct __orxTEST_PLUGINS_t
{
  orxHANDLE     hPlugin;
  orxSTRING     zType;
  orxSTRING     zFile;
  orxMODULE_ID  eModuleID;
} orxTEST_PLUGINS;

typedef struct __orxTEST_PLUGINS_STATIC_t
{
  orxTEST_PLUGINS astPlugins[orxPLUGIN_CORE_ID_NUMBER];
  orxCHAR azFileName[orxTEST_PLUGINS_KU32_MAX_PLUGINS_PER_TYPE][orxTEST_PLUGINS_KU32_PLUGINS_NAME_SIZE];
} orxTEST_PLUGINS_STATIC;

orxSTATIC orxTEST_PLUGINS_STATIC sstTest_Plugin;


/******************************************************
 * TEST FUNCTIONS
 ******************************************************/

/** Display list of plugin from selected directory
 * @return Returns the number of available plugins of the selected Type
 */
orxBOOL orxTest_Plugin_BrowseDirectory(orxU32 u32Type)
{
  orxU32 u32Index;
  orxCHAR zDirName[32];
  
  /* Traverse the selected directory and store the dynamic library in the array */
  #ifdef __orxLINUX__
  
  DIR *pstDir;                                /* Pointer on directory structure */
  struct dirent *pstFile;                     /* Pointer on a dir entry (file) */

  /* Create the selected directory name */  
  orxTextIO_Printf(zDirName, "plugins"DIRSEP"core"DIRSEP"%s", sstTest_Plugin.astPlugins[u32Type].zType);
  orxTextIO_PrintLn(zDirName);

  /* Open the current directory */
  pstDir = opendir(zDirName);
  
  /* Is it a valid directory ? */
  if (pstDir != NULL)
  {
    u32Index = 0;
    
    /* Traverse the directory */
    while ((pstFile = readdir(pstDir)))
    {
      /* Check if the file name ends with .so */
      if ((strlen(pstFile->d_name) > 3) &&
          (strcmp(pstFile->d_name + (strlen(pstFile->d_name) - 3), ".so") == 0)
         )
      {
        /* Store the library name */
        orxTextIO_Printf(sstTest_Plugin.azFileName[u32Index], "%s", pstFile->d_name);

        /* Print the library name */
        orxTextIO_PrintLn("%02d - %s", u32Index, pstFile->d_name);

        u32Index++;
      }
    }
    closedir(pstDir);
  }
  else
  {
    fprintf(stderr, "Can't open directory %s\n", zDirName);
  }
  
  #else /* !LINUX */
    #ifdef __orxWINDOWS__
  
  
  struct _finddata_t stFile;  /* File datas infos */
  long lFile;                 /* File handle */
  orxCHAR zPattern[512];      /* Create the lookup pattern (_zDirName\*.dll) */
  orxCHAR zLibName[512];      /* Create the library name (dll*/

  u32Index = 0;
  
  /* Create the selected directory name */  
  orxTextIO_Printf(zDirName, "plugins"DIRSEP"core"DIRSEP"%s", sstTest_Plugin.astPlugins[u32Type].zType);
  orxTextIO_PrintLn(zDirName);

  /* Initialize Pattern*/
  orxMemory_Set(zPattern, 0, sizeof(zPattern));
 
  /* Check _directory name (overflow) */
  if (strlen(zDirName) < 500)
  {
    /* Create pattern */
    orxTextIO_Printf(zPattern, "%s\\*.dll", zDirName);
      
    /* Find first .dll file in directory */
    if ((lFile = _findfirst(zPattern, &stFile)) != -1L)
    {
      /* Create full lib name */
      orxTextIO_Printf(zLibName, "%s\\%s", zDirName, stFile.name);
      
      /* Store the library name */
      orxTextIO_Printf(sstTest_Plugin.azFileName[u32Index], "%s", zLibName);
      
      /* Print the library name */
      orxTextIO_PrintLn("%02d - %s", u32Index, zLibName);

      u32Index = 1;

      /* Find the rest of the .c files */
      while (_findnext(lFile, &stFile) == 0)
      {
        /* Create full lib name */
        orxTextIO_Printf(zLibName, "%s\\%s", zDirName, stFile.name);

        /* Store the library name */
        orxTextIO_Printf(sstTest_Plugin.azFileName[u32Index], "%s", zLibName);
        
        /* Print the library name */
        orxTextIO_PrintLn("%02d - %s", u32Index, zLibName);

        u32Index++;
      }

      _findclose(lFile);
    }
  }
  else
  {
      fprintf(stderr, "Directory name too long\n");
  }        

  
    #endif /* __orxWINDOWS__ */
  #endif /* __orxLINUX__ */
  
  return u32Index;
  
}
 
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
  orxTextIO_PrintLn(orxSTRING_EMPTY);  
  
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
  orxTextIO_PrintLn(orxSTRING_EMPTY);
  
  /* Print a line of stars */
  for (u32Index = 0; u32Index < orxTEST_PLUGINS_KU32_NB_COLS; u32Index++)
  {
    orxTextIO_Print("*");
  }
  orxTextIO_PrintLn(orxSTRING_EMPTY);

  /* Everythings done */
  orxTextIO_PrintLn("Done !");
}

/** Load a plugin
 */
orxVOID orxTest_Plugin_Load()
{ 
  orxU32 u32Index;
  orxS32 s32TypeResult;
  orxS32 s32FileResult;
    
  /* First, Display the list of available plugin type */
  orxTextIO_PrintLn("Select a plugin type to load from the list above");
  
  for (u32Index = 0; u32Index < orxPLUGIN_CORE_ID_NUMBER; u32Index++)
  {
    orxTextIO_PrintLn("%02d - %s", u32Index, sstTest_Plugin.astPlugins[u32Index].zType);
  } 
   
  /* Then, get the type of plugins that the user wants to load */
  orxTextIO_ReadS32InRange(&s32TypeResult, 10, 0, orxPLUGIN_CORE_ID_NUMBER - 1, "Choice : ", orxTRUE);
  
  /* Now, show the list of available plugins of this type */
  u32Index = orxTest_Plugin_BrowseDirectory(s32TypeResult);
  if (u32Index > 0)
  {
    /* Get the user selection */
    orxTextIO_ReadS32InRange(&s32FileResult, 10, 0, u32Index - 1, "Choice : ", orxTRUE);
  
    /* Is there a plugin already loaded ? */
    if (sstTest_Plugin.astPlugins[s32TypeResult].hPlugin != orxHANDLE_UNDEFINED)
    {
      /* Unload the plugin */
      orxTextIO_PrintLn("Unloading %s...", sstTest_Plugin.astPlugins[s32TypeResult].zFile);
      
      /* Unload the plugin */
      orxPlugin_Unload(sstTest_Plugin.astPlugins[s32TypeResult].hPlugin);
    }
  
    /* Try to load the plugin */
    orxTextIO_PrintLn("Loading %s...", sstTest_Plugin.azFileName[s32FileResult]);
    
    sstTest_Plugin.astPlugins[s32TypeResult].hPlugin = orxPlugin_Load(sstTest_Plugin.azFileName[s32FileResult], sstTest_Plugin.astPlugins[s32TypeResult].zType);
    if (sstTest_Plugin.astPlugins[s32TypeResult].hPlugin == orxHANDLE_UNDEFINED)
    {
      orxTextIO_PrintLn("Can't load plugin...");
      
      /* Store plugin name */
      sstTest_Plugin.astPlugins[s32TypeResult].zFile = sstTest_Plugin.azFileName[s32FileResult];
    }
    else
    {
      orxTextIO_PrintLn("Plugin loaded");
      
      /* Init the loaded plugin (call the plugin init function) */
      if(orxModule_Init(sstTest_Plugin.astPlugins[s32TypeResult].eModuleID) == orxSTATUS_SUCCESS)
      {
        orxTextIO_PrintLn("Plugin successfully initialized");
      }
      else
      {
        orxTextIO_PrintLn("Can't Init the new loaded plugin, there is a problem somewhere... I'm going to unload the plugin...");
        
        /* Unload the plugin */
        orxPlugin_Unload(sstTest_Plugin.astPlugins[s32TypeResult].hPlugin);
        sstTest_Plugin.astPlugins[s32TypeResult].hPlugin = orxHANDLE_UNDEFINED;
      }
    }
  }
  else
  {
    /* No plugins are available for this type */
    orxTextIO_PrintLn("No plugins are available for this type");
  }
  
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
orxVOID orxTest_RegisterCorePlugin()
{
  orxU32 u32Index;
    
  /* Initialize Memory module */
  orxModule_Init(orxMODULE_ID_PLUGIN);
  
  /* Register test functions */
  orxTest_Register("Plugin", "Display module informations", orxTest_Plugin_Infos);
  orxTest_Register("Plugin", "Display list of loaded plugins", orxTest_Plugin_ShowList);
  orxTest_Register("Plugin", "Load a plugin", orxTest_Plugin_Load);
  orxTest_Register("Plugin", "Unload a plugin", orxTest_Plugin_Unload);
  
  /* Initialize plugins array content */
  for (u32Index = 0; u32Index < orxPLUGIN_CORE_ID_NUMBER; u32Index++)
  {
    sstTest_Plugin.astPlugins[u32Index].hPlugin = orxHANDLE_UNDEFINED;
    sstTest_Plugin.astPlugins[u32Index].zType   = orxTEST_PLUGINS_KZ_UNKNOWN_NAME;
    sstTest_Plugin.astPlugins[u32Index].zFile   = orxTEST_PLUGINS_KZ_UNLOADED_NAME;
  }
  
  /* Set plugin name (used for name and directory), and Init / Exit function associated */
  sstTest_Plugin.astPlugins[orxPLUGIN_CORE_ID_DISPLAY].zType      = "display";
  sstTest_Plugin.astPlugins[orxPLUGIN_CORE_ID_DISPLAY].eModuleID  = orxMODULE_ID_DISPLAY;
  
  sstTest_Plugin.astPlugins[orxPLUGIN_CORE_ID_FILE].zType         = "file";
  sstTest_Plugin.astPlugins[orxPLUGIN_CORE_ID_FILE].eModuleID     = orxMODULE_ID_FILE;
  
  sstTest_Plugin.astPlugins[orxPLUGIN_CORE_ID_JOYSTICK].zType     = "joystick";
  sstTest_Plugin.astPlugins[orxPLUGIN_CORE_ID_JOYSTICK].eModuleID = orxMODULE_ID_JOYSTICK;
  
  sstTest_Plugin.astPlugins[orxPLUGIN_CORE_ID_KEYBOARD].zType     = "keyboard";
  sstTest_Plugin.astPlugins[orxPLUGIN_CORE_ID_KEYBOARD].eModuleID = orxMODULE_ID_KEYBOARD;
  
  sstTest_Plugin.astPlugins[orxPLUGIN_CORE_ID_MOUSE].zType        = "mouse";
  sstTest_Plugin.astPlugins[orxPLUGIN_CORE_ID_MOUSE].eModuleID    = orxMODULE_ID_MOUSE;
  
  sstTest_Plugin.astPlugins[orxPLUGIN_CORE_ID_PACKAGE].zType      = "package";
  sstTest_Plugin.astPlugins[orxPLUGIN_CORE_ID_PACKAGE].eModuleID  = orxMODULE_ID_PACKAGE;
  
  sstTest_Plugin.astPlugins[orxPLUGIN_CORE_ID_SCRIPT].zType       = "script";
  sstTest_Plugin.astPlugins[orxPLUGIN_CORE_ID_SCRIPT].eModuleID   = orxMODULE_ID_SCRIPT;
  
  sstTest_Plugin.astPlugins[orxPLUGIN_CORE_ID_SOUND].zType        = "sound";
  sstTest_Plugin.astPlugins[orxPLUGIN_CORE_ID_SOUND].eModuleID    = orxMODULE_ID_SOUND;
  
  sstTest_Plugin.astPlugins[orxPLUGIN_CORE_ID_TIME].zType         = "time";
  sstTest_Plugin.astPlugins[orxPLUGIN_CORE_ID_TIME].eModuleID     = orxMODULE_ID_TIME;

  orxMemory_Set(&sstTest_Plugin.azFileName, 0, sizeof(sstTest_Plugin.azFileName));
}

orxVOID orxTest_Plugin_Exit()
{
//  orxU32 u32Index;
//  
//  /* Unload all the loaded plugins before exit */
//  for (u32Index = 0; u32Index < orxPLUGIN_CORE_ID_NUMBER; u32Index++)
//  {
//    if (sstTest_Plugin.astPlugins[u32Index].hPlugin != orxHANDLE_UNDEFINED)
//    {
//      /* Unload the selected plugin */
//      orxPlugin_Unload(sstTest_Plugin.astPlugins[u32Index].hPlugin);
//      sstTest_Plugin.astPlugins[u32Index].hPlugin = orxHANDLE_UNDEFINED;
//    }
//  }
}

orxTEST_DEFINE_ENTRY_POINT(orxTest_RegisterCorePlugin, orxTest_Plugin_Exit)
