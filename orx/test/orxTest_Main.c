/**
 * @file orxTest_Main.c
 * 
 * Main Test Program - Call registration for each module and start the TestBed Menu.
 * 
 * @note This program could directly use the plugin system or the File module
 * @note to read datas, but it can not since it has to be on the lowest level as possible
 * @note and test each module of the engine. File or Plugins are in the engine and they have
 * @note to be tested like the other one.
 * @note This is why there is a test on the platform : For file enumeration and dynamic
 * @note library loading, platform specific function are used
 * 
 * @todo WINDOWS : Find how to share static memory between exe and dll (link with -rdynamic on linux)
 * @todo WINDOWS : dll directory hard coded (.\modules). Allow the user to give a
 * @todo a list of dll directory (or file)
 */
 
 /***************************************************************************
 orxTest_Main.c
 Main Test Program
 
 begin                : 10/04/2005
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
#include "debug/orxDebug.h"

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

#define orxTEST_MAIN_KU32_FLAG_NONE  0x00000000  /**< No flags have been set */
#define orxTEST_MAIN_KU32_FLAG_READY 0x00000001  /**< The module has been initialized */

#define orxTEST_MAIN_KU32_CHOICE_BUFFER_SIZE 32 /**< Maximum size (number of characters) for a user entry */

/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/
typedef struct __orxTEST_MAIN_STATIC_t
{
  orxU32 u32Flags;      /**< App flags */
  orxU32 u32NbLibrary;  /**< Number of loaded library */
  orxHANDLE *phLibrary; /**< Pointer on library handle array */
} orxTEST_MAIN_STATIC;

/***************************************************************************
 * Module global variable                                                  *
 ***************************************************************************/
orxSTATIC orxTEST_MAIN_STATIC sstTestMain;

/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

/** Read dynamic library of a directory and load them
 * @param _zDirName (IN)  Name of the directory
 */
orxVOID orxTestMain_Load(orxSTRING _zDirName)
{
  /* Module initialized ? */
  orxASSERT((sstTestMain.u32Flags & orxTEST_MAIN_KU32_FLAG_READY) == orxTEST_MAIN_KU32_FLAG_READY);

  /* Correct parameters ? */
  orxASSERT(_zDirName != orxNULL);

  printf("Loading Test modules : \n");

  #ifdef __orxLINUX__
  
  DIR *pstDir;                                /* Pointer on directory structure */
  struct dirent *pstFile;                     /* Pointer on a dir entry (file) */
  void *pHandle;                              /* Dynamic Library handle */

  /* Open the current directory */
  pstDir = opendir(_zDirName);
  
  /* Is it a valid directory ? */
  if (pstDir != NULL)
  {
    /* Traverse the directory */
    while ((pstFile = readdir(pstDir)))
    {
      /* Check if the file name ends with .so */
      if ((strlen(pstFile->d_name) > 3) &&
          (strcmp(pstFile->d_name + (strlen(pstFile->d_name) - 3), ".so") == 0)
         )
      {
        /* Load the library */
        fprintf(stderr, " --> %s\n", pstFile->d_name);
        pHandle = dlopen(pstFile->d_name, RTLD_NOW);
        if (!pHandle)
        {
          fprintf(stderr, "%s\n", dlerror());
        }
        else
        {
          /* Store lib handle in a new portion of memory (and increase counter)*/
          sstTestMain.phLibrary = (orxHANDLE *)realloc(sstTestMain.phLibrary, (sstTestMain.u32NbLibrary + 1) * sizeof(orxHANDLE));
          sstTestMain.phLibrary[sstTestMain.u32NbLibrary] = (orxHANDLE)pHandle;
          sstTestMain.u32NbLibrary++;
        }
      }
    }
    closedir(pstDir);
  }
  else
  {
    fprintf(stderr, "Can't open directory %s\n", _zDirName);
  }
  
  #else /* !LINUX */
    #ifdef __orxWINDOWS__
  
  
  struct _finddata_t stFile;  /* File datas infos */
  long lFile;                 /* File handle */
  orxCHAR zPattern[512];      /* Create the lookup pattern (_zDirName\*.dll) */
  orxCHAR zLibName[512];      /* Create the library name (dll*/
  HINSTANCE hLibrary;         /* Handle on the loaded library */
  
  /* Initialize Pattern*/
  memset(zPattern, 0, 512 * sizeof(orxCHAR));
 
  /* Check _directory name (overflow) */
  if (strlen(_zDirName) < 500)
  {
    /* Create pattern */
    sprintf(zPattern, "%s\\*.dll", _zDirName);
      
    /* Find first .dll file in directory */
    if ((lFile = _findfirst(zPattern, &stFile)) != -1L)
    {
      /* Create full lib name */
      sprintf(zLibName, "%s\\%s", _zDirName, stFile.name);

      /* Get a handle to the DLL module. */
      fprintf(stderr, " --> %s\n", zLibName);
      hLibrary = LoadLibrary(zLibName);
      
      /* Store lib handle in a new portion of memory (and increase counter)*/
      sstTestMain.phLibrary = (orxHANDLE *)malloc(sizeof(orxHANDLE));
      sstTestMain.phLibrary[0] = (orxHANDLE)hLibrary;
      sstTestMain.u32NbLibrary = 1;

      /* Find the rest of the .c files */
      while (_findnext(lFile, &stFile) == 0)
      {
        /* Create full lib name */
        sprintf(zLibName, "%s\\%s", _zDirName, stFile.name);

        /* Get a handle to the DLL module. */
        fprintf(stderr, " --> %s\n", zLibName);
        hLibrary = LoadLibrary(zLibName);

        /* Store lib handle in a new portion of memory (and increase counter)*/
        sstTestMain.phLibrary = (orxHANDLE *)realloc(sstTestMain.phLibrary, (sstTestMain.u32NbLibrary + 1) * sizeof(orxHANDLE));
        sstTestMain.phLibrary[sstTestMain.u32NbLibrary] = (orxHANDLE)hLibrary;
        sstTestMain.u32NbLibrary++;
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
}

/** Release all loaded libraries
 */
orxVOID orxTestMain_Release()
{
  orxU32 u32Index;  /* Index used to traverse lib array */
  
  /* Module initialized ? */
  orxASSERT((sstTestMain.u32Flags & orxTEST_MAIN_KU32_FLAG_READY) == orxTEST_MAIN_KU32_FLAG_READY);

  /* Traverse and free loaded library for each platform */
  for (u32Index = 0; u32Index < sstTestMain.u32NbLibrary; u32Index++)
  {
    /* Release loaded library */
#ifdef __orxLINUX__

    dlclose((void*)sstTestMain.phLibrary[u32Index]);

#else
  #ifdef __orxWINDOWS__

    FreeLibrary((HINSTANCE)sstTestMain.phLibrary[u32Index]);

  #endif
#endif /* __orxLINUX__ */
  }
  
  /* Free the allocated array */
  free(sstTestMain.phLibrary);
}  


/** Initialize TestMain static variables
 */
orxVOID orxTestMain_Init()
{
  /* App not already initialized ? */
  orxASSERT(!(sstTestMain.u32Flags & orxTEST_MAIN_KU32_FLAG_READY));

  /* Cleans static controller */
  memset(&sstTestMain, 0, sizeof(orxTEST_MAIN_STATIC));
  
  /* Set App has ready */
  sstTestMain.u32Flags = orxTEST_MAIN_KU32_FLAG_READY;

  /* Load dynamic library */
  orxTestMain_Load("."DIRSEP"modules");
}

/** Unitialize TestMain
 */
orxVOID orxTestMain_Exit()
{
  /* App initialized ? */
  orxASSERT((sstTestMain.u32Flags & orxTEST_MAIN_KU32_FLAG_READY) == orxTEST_MAIN_KU32_FLAG_READY);
  
  /* Release library */
  orxTestMain_Release();
  
  /* App not ready now */
  sstTestMain.u32Flags = orxTEST_MAIN_KU32_FLAG_READY;
}
/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/***************************************************************************
 **************************** MAIN FUNCTION ********************************
 ***************************************************************************/

int main(int argc, char **argv)
{
  char zChoice[orxTEST_MAIN_KU32_CHOICE_BUFFER_SIZE]; /* Entry read from user */
  int iVal;                                           /* value of entry */
  
  /* Minimum initialisation */
  orxDEBUG_INIT();      /* Debug module is necessary to display debug from each module */
  orxTest_Init();       /* Test Module is necessary to register test function */
  orxTestMain_Init();   /* Initialise application (load dynamic library */
  
  /* Display menu and get user entry */
  do
  {
    /* Show list of registered function */
    orxTest_DisplayMenu();
    
    /* Get user choice */
    printf("quit : Quit the test program\n\n");
    printf("Choice : ");
    fgets(zChoice, orxTEST_MAIN_KU32_CHOICE_BUFFER_SIZE, stdin);
    
    /* Check overflow */
    if ((strlen(zChoice) > 0) && zChoice[strlen(zChoice)-1] == '\n')
    {
      zChoice[strlen(zChoice)-1] = '\0';
    }
    
    /* The user wants to quir ? */
    if (strcmp(zChoice, "quit") != 0)
    {
      /* No, so parse its choice */
      iVal = atoi(zChoice);
      
      /* Execute the function associated to the user choice */
      if (orxTest_Execute((orxHANDLE)iVal) == orxSTATUS_FAILED)
      {
        /* Invalid choice was used */
        printf("Unknown command\n");
      }
      
      /* Function has been executed. Wait for a pressed key before displaying the menu (clear screen would be fine) */
      printf("Press Enter to continue\n");
      getchar();
      
      /* Reinitialize user choice */
      memset(zChoice, 0, orxTEST_MAIN_KU32_CHOICE_BUFFER_SIZE * sizeof(char));
      iVal = -1;
    }
  }
  while (strcmp(zChoice, "quit") != 0);
  
  /* Uninitialize modules */
  orxTestMain_Exit();
  orxTest_Exit();
  orxDEBUG_EXIT();
  
  /* That's all folks ! */
  return 0;
}
