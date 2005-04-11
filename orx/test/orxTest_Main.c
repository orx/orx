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
 * @todo Test on windows.
 * @todo Cleans the code, try to reduce platform specific code impact.
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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define orxTEST_MAIN_KU32_CHOICE_BUFFER_SIZE 32

#ifdef LINUX
  #include <sys/types.h>
  #include <dirent.h>
  #include <dlfcn.h>


#else
  #error PLATFORM NOT IMPLEMENTED
#endif

int main(int argc, char **argv)
{
  char zChoice[orxTEST_MAIN_KU32_CHOICE_BUFFER_SIZE];
  int iVal;                                   /* Entry val */
  
  orxDEBUG_INIT();
  orxTest_Init();
  

/* Linux Specific content */
#ifdef LINUX
  DIR *pstDir;                                /* Pointer on directory structure */
  struct dirent *pstFile;                     /* Pointer on a dir entry (file) */
  void *pHandle;                              /* Dynamic Library handle */
  orxVOID (*orxTest_StartRegister)(orxVOID);  /* Loaded function to call */
  char *zError;                               /* Error string */

  /* Open the current directory */
  pstDir = opendir("./");
  
  /* Is it a valid directory ? */
  if (pstDir != NULL)
  {
    printf("Loading Test modules : \n");
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
          /* Get the function */
          orxTest_StartRegister = dlsym(pHandle, "orxTest_StartRegister");
          if ((zError = dlerror()) != NULL)
          {
            fprintf(stderr, "%s\n", zError);
          }
          else
          {
            /* Start module registration */
            (*orxTest_StartRegister)();
            
            /* Close the loaded library */
            /* dlclose(pHandle); -- dyn lib have to be unloaded when program exit */
          }
        }
      }
    }
    closedir(pstDir);
  }
  else
  {
    fprintf(stderr, "Can't open directory");
  }
  
#else
  #error PLATFORM NOT IMPLEMENTED
#endif

  do
  {
    orxTest_DisplayMenu();
    printf("quit : Quit the test program\n\n");
    printf("Choice : ");
    fgets(zChoice, orxTEST_MAIN_KU32_CHOICE_BUFFER_SIZE, stdin);
    if ((strlen(zChoice) > 0) && zChoice[strlen(zChoice)-1] == '\n')
    {
      zChoice[strlen(zChoice)-1] = '\0';
    }

    if (strcmp(zChoice, "quit") != 0)
    {
      iVal = atoi(zChoice);
      if (orxTest_Execute((orxHANDLE)iVal) == orxSTATUS_FAILED)
      {
        printf("Unknown command\n");
      }
      printf("Press Enter to continue\n");
      getchar();
      memset(zChoice, 0, orxTEST_MAIN_KU32_CHOICE_BUFFER_SIZE * sizeof(char));
      iVal = -1;
    }
    
  }
  while (strcmp(zChoice, "quit") != 0);
  
  
  orxTest_Exit();
  orxDEBUG_EXIT();
  
  return 0;
}
