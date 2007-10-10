/**
 * @file orxTest.c
 * 
 * Test Module - Offers a basic interface to display a menu of registered test functions
 * 
 */
 
 /***************************************************************************
 orxTest.c
 Test Module
 
 begin                : 02/04/2005
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

#ifdef __orxTEST__ /* Only compile the content of this file in the lib in test mode */

#include "debug/orxTest.h"

#include "core/orxClock.h"
#include "debug/orxDebug.h"
#include "main/orxParam.h"
#include "memory/orxMemory.h"
#include "io/orxTextIO.h"
#include "utils/orxString.h"

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

#define orxTEST_KU32_STATIC_FLAG_NONE         0x00000000L /**< No flags have been set */
#define orxTEST_KU32_STATIC_FLAG_READY        0x00000001L /**< The module has been initialized */

#define orxTEST_KU32_STATIC_FLAG_IN_USE       0x00000002L /**< orx runs in test mode */
#define orxTEST_KU32_STATIC_FLAG_FREEZE       0x00000004L /**< Display menu freeze flag */


#define orxTEST_KU32_MAX_REGISTERED_FUNCTIONS 256
#define orxTEST_MAIN_KU32_CHOICE_BUFFER_SIZE  16  /**< Maximum size (number of characters) for a user entry */

/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/
typedef struct __orxTEST_t
{
  orxCHAR zModule[32];            /**< Module Name */
  orxCHAR zMenuEntry[256];        /**< Description of the command */
  orxTEST_FUNCTION pfnFunction;   /**< Pointer on the registered function to execute */
  orxBOOL bDisplayed;             /**< Menu entry displayed in the menu */
} orxTEST;

typedef struct __orxTEST_STATIC_t
{
  orxU32 u32Flags;                                                  /**< Module flags */
  orxCLOCK *pstClock;
  orxTEST astTestFunctions[orxTEST_KU32_MAX_REGISTERED_FUNCTIONS];  /**< List of functions */
  orxU32 u32NbRegisteredFunc;                                       /**< Number of registered functions */
  orxU32 u32NbLibrary;            /**< Number of loaded library */
  orxHANDLE *phLibrary;           /**< Pointer on library handle array */
} orxTEST_STATIC;

/***************************************************************************
 * Module global variable                                                  *
 ***************************************************************************/
static orxTEST_STATIC sstTest;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

/** Read dynamic library of a directory and load them
 * @param _zDirName (IN)  Name of the directory
 */
orxVOID orxTest_Load(orxSTRING _zDirName)
{
  #ifdef __orxLINUX__
  
  DIR *pstDir;                                /* Pointer on directory structure */
  struct dirent *pstFile;                     /* Pointer on a dir entry (file) */
  void *pHandle;                              /* Dynamic Library handle */

  /* Module initialized ? */
  orxASSERT((sstTest.u32Flags & orxTEST_KU32_STATIC_FLAG_READY) == orxTEST_KU32_STATIC_FLAG_READY);

  /* Correct parameters ? */
  orxASSERT(_zDirName != orxNULL);

  printf("Loading Test modules : \n");

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
          sstTest.phLibrary = (orxHANDLE *)realloc(sstTest.phLibrary, (sstTest.u32NbLibrary + 1) * sizeof(orxHANDLE));
          sstTest.phLibrary[sstTest.u32NbLibrary] = (orxHANDLE)pHandle;
          sstTest.u32NbLibrary++;
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
  
  /* Module initialized ? */
  orxASSERT((sstTest.u32Flags & orxTEST_KU32_STATIC_FLAG_READY) == orxTEST_KU32_STATIC_FLAG_READY);

  /* Correct parameters ? */
  orxASSERT(_zDirName != orxNULL);

  printf("Loading Test modules : \n");

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
      sstTest.phLibrary = (orxHANDLE *)malloc(sizeof(orxHANDLE));
      sstTest.phLibrary[0] = (orxHANDLE)hLibrary;
      sstTest.u32NbLibrary = 1;

      /* Find the rest of the .c files */
      while (_findnext(lFile, &stFile) == 0)
      {
        /* Create full lib name */
        sprintf(zLibName, "%s\\%s", _zDirName, stFile.name);

        /* Get a handle to the DLL module. */
        fprintf(stderr, " --> %s\n", zLibName);
        hLibrary = LoadLibrary(zLibName);

        /* Store lib handle in a new portion of memory (and increase counter)*/
        sstTest.phLibrary = (orxHANDLE *)realloc(sstTest.phLibrary, (sstTest.u32NbLibrary + 1) * sizeof(orxHANDLE));
        sstTest.phLibrary[sstTest.u32NbLibrary] = (orxHANDLE)hLibrary;
        sstTest.u32NbLibrary++;
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
orxVOID orxTest_Release()
{
  orxU32 u32Index;  /* Index used to traverse lib array */
  
  /* Module initialized ? */
  orxASSERT((sstTest.u32Flags & orxTEST_KU32_STATIC_FLAG_READY) == orxTEST_KU32_STATIC_FLAG_READY);
  
  /* Traverse and free loaded library for each platform */
  for (u32Index = 0; u32Index < sstTest.u32NbLibrary; u32Index++)
  {
    /* Release loaded library */
#ifdef __orxLINUX__

    dlclose((void*)sstTest.phLibrary[u32Index]);

#else
  #ifdef __orxWINDOWS__

    FreeLibrary((HINSTANCE)sstTest.phLibrary[u32Index]);

  #endif
#endif /* __orxLINUX__ */
  }
  
  /* Free the allocated array */
  free(sstTest.phLibrary);
}  

/** Display list of functions associated to a module
 * @param _zModuleName  (IN)  Name of the module
 */
orxSTATIC orxINLINE orxVOID orxTest_PrintModuleFunc(orxCONST orxSTRING _zModuleName)
{
  orxU32 u32Index;  /* Index used to traverse the function array */

  /* Module initialized ? */
  orxASSERT((sstTest.u32Flags & orxTEST_KU32_STATIC_FLAG_READY) == orxTEST_KU32_STATIC_FLAG_READY);

  /* Correct parameters ? */
  orxASSERT(_zModuleName != orxNULL);
  
  /* Display Module Name */
  orxTextIO_PrintLn("\n**** MODULE : %s ****", _zModuleName);
  
  /* Display all functions not already displayed and that have the same module name */
  for (u32Index = 0; u32Index < sstTest.u32NbRegisteredFunc; u32Index++)
  {
    if (!(sstTest.astTestFunctions[u32Index].bDisplayed) && (orxMemory_Compare(sstTest.astTestFunctions[u32Index].zModule, _zModuleName, orxString_Length(_zModuleName) * sizeof(orxCHAR)) == 0))
    {
      sstTest.astTestFunctions[u32Index].bDisplayed = orxTRUE;
      orxTextIO_PrintLn("* %lu - %s", u32Index, sstTest.astTestFunctions[u32Index].zMenuEntry);
    }
  }
}

/** Reset all visibility flag (set with orxTest_PrintModuleFunc to knwow if a function has already been displayed
 */
orxSTATIC orxINLINE orxVOID orxTest_ResetVisibility()
{
  orxU32 u32Index;  /* Index used to traverse the function array */
  
  /* Module initialized ? */
  orxASSERT((sstTest.u32Flags & orxTEST_KU32_STATIC_FLAG_READY) == orxTEST_KU32_STATIC_FLAG_READY);
  
  for (u32Index = 0; u32Index < sstTest.u32NbRegisteredFunc; u32Index++)
  {
    sstTest.astTestFunctions[u32Index].bDisplayed = orxFALSE;
  }
}

/** Execute a registered function
 * @param   (IN)  _hRegisteredFunc  Handle of the registered function to execute
 * @return  Returns the success / fail status (failed when the Handle is unknown)
 */
orxSTATUS orxFASTCALL orxTest_Execute(orxHANDLE _hRegisteredFunc)
{
  /* Module initialized ? */
  orxASSERT((sstTest.u32Flags & orxTEST_KU32_STATIC_FLAG_READY) == orxTEST_KU32_STATIC_FLAG_READY);

  /* Correct parameters ? */
  if ((_hRegisteredFunc != orxHANDLE_UNDEFINED) && ((orxU32)_hRegisteredFunc < sstTest.u32NbRegisteredFunc))
  {
    /* Execute the function */
    sstTest.astTestFunctions[(orxU32)_hRegisteredFunc].pfnFunction();
    
    return orxSTATUS_SUCCESS;
  }
  
  return orxSTATUS_FAILURE;
}

/** Display a Menu with registered function list
 */
orxVOID orxTest_DisplayMenu()
{
  orxU32 u32Index;  /* Index used to traverse the function array */
  
  /* Module initialized ? */
  orxASSERT((sstTest.u32Flags & orxTEST_KU32_STATIC_FLAG_READY) == orxTEST_KU32_STATIC_FLAG_READY);
  
  /* Display menu header */
  printf("\n\n*************************************\n");
  printf("********** orx TEST MENU ************\n");
  printf("*************************************\n");

  /* Display menu for each module */
  for(u32Index = 0; u32Index < sstTest.u32NbRegisteredFunc; u32Index++)
  {
    if (!sstTest.astTestFunctions[u32Index].bDisplayed)
    {
      orxTest_PrintModuleFunc(sstTest.astTestFunctions[u32Index].zModule);
    }
  }
  
  /* Reset temp helper bool */
  orxTest_ResetVisibility();
}

/** Set Test flags (the test parameter has been given)
 * @param[in] _u32NbParam Number of extra parameters read for this option
 * @param[in] _azParams   Array of extra parameters (the first one is always the option name)
 * @return Returns orxSTATUS_SUCCESS if informations read are correct, orxSTATUS_FAILURE if a problem has occured
 */
orxSTATUS orxFASTCALL orxTest_ParamTest(orxU32 _u32NbParam, orxCONST orxSTRING _azParams[])
{
  /* Set Test Flag */
  sstTest.u32Flags |= orxTEST_KU32_STATIC_FLAG_IN_USE;

  /* Load dynamic library */
  orxTest_Load("."DIRSEP"modules");

  /* Starts the test program if needed */
  orxTest_Start();

  /* Done */
  return orxSTATUS_SUCCESS;
}

/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/***************************************************************************
 orxTest_Setup
 Test module setup.

 returns: nothing
 ***************************************************************************/
orxVOID orxTest_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_TEST, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_TEST, orxMODULE_ID_BANK);
  orxModule_AddDependency(orxMODULE_ID_TEST, orxMODULE_ID_TEXTIO);
  orxModule_AddDependency(orxMODULE_ID_TEST, orxMODULE_ID_PARAM);
  orxModule_AddDependency(orxMODULE_ID_TEST, orxMODULE_ID_CLOCK);

  return;
}

/** Initialize the test module
 */
orxSTATUS orxTest_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Not already Initialized? */
  if(!(sstTest.u32Flags & orxTEST_KU32_STATIC_FLAG_READY))
  {
    orxPARAM stParam;

    /* Initialize values */
    orxMemory_Set(&sstTest, 0, sizeof(orxTEST_STATIC));
    sstTest.u32NbRegisteredFunc = 0;
  
    /* Module ready */
    sstTest.u32Flags |= orxTEST_KU32_STATIC_FLAG_READY;
    
    /* Sets the param structure up */    
    stParam.u32Flags    = orxPARAM_KU32_FLAG_NONE;
    stParam.pfnParser   = orxTest_ParamTest;
    stParam.zShortName  = "T";
    stParam.zLongName   = "test";
    stParam.zShortDesc  = "Starts the test program.";
    stParam.zLongDesc   = "Starts the test program instead of bare orx engine. The test program allows to test and debug orx modules and run tech demo.";

    /* Registers the parameter to start the test program */
    orxParam_Register(&stParam);

    /* Success */
    eResult = orxSTATUS_SUCCESS;
  }
  else
  {
    /* !!! MSG !!! */

    /* Already initialized */
    eResult = orxSTATUS_SUCCESS;
  }

  return eResult;
}

/** Uninitialize the test module
 */
orxVOID orxTest_Exit()
{
  /* Module initialized ? */
  if ((sstTest.u32Flags & orxTEST_KU32_STATIC_FLAG_READY) == orxTEST_KU32_STATIC_FLAG_READY)
  {
    /* Release library */
    orxTest_Release();

    /* Module becomes not ready */
    sstTest.u32Flags &= ~orxTEST_KU32_STATIC_FLAG_READY;
  }

  return;
}

/** Register a new function
 * @param   (IN)  _zModuleName      Name of the module (to group a list of functions)
 * @param   (IN)  _zMenuEntry       Text displayed to describe the test function
 * @param   (IN)  _pfnFunction       Function executed when the menu entry is selected
 * @return Returns an Handle on the function
 */
orxHANDLE orxFASTCALL orxTest_Register(orxCONST orxSTRING _zModuleName, orxCONST orxSTRING _zMenuEntry, orxCONST orxTEST_FUNCTION _pfnFunction)
{
  orxTEST *pstTest; /* Structure that will store new datas */
  orxHANDLE hRet;   /* Returnd handle value */
  
  /* Module initialized ? */
  orxASSERT((sstTest.u32Flags & orxTEST_KU32_STATIC_FLAG_READY) == orxTEST_KU32_STATIC_FLAG_READY);
    
  /* Correct parameters ? */
  orxASSERT(_zModuleName != orxNULL);
  orxASSERT(_zMenuEntry != orxNULL);
  orxASSERT(_pfnFunction != orxNULL);

  /* Module full ? */
  if (sstTest.u32NbRegisteredFunc < orxTEST_KU32_MAX_REGISTERED_FUNCTIONS)
  {
    /* Copy datas in the registered function array */
    pstTest = &(sstTest.astTestFunctions[sstTest.u32NbRegisteredFunc]);
    orxMemory_Copy(pstTest->zModule, _zModuleName, 31 * sizeof(orxCHAR));
    orxMemory_Copy(pstTest->zMenuEntry, _zMenuEntry, 255 * sizeof(orxCHAR));
    pstTest->pfnFunction = _pfnFunction;
    pstTest->bDisplayed = orxFALSE;
    
    /* Set the returned handle (array index value) */
    hRet = (orxHANDLE)sstTest.u32NbRegisteredFunc;
    
    /* Increase the number of stored functions */
    sstTest.u32NbRegisteredFunc++;
  }
  else
  {
    /* No space left for new function, return an undefined handle */
    hRet = orxHANDLE_UNDEFINED;
  }
  
  return hRet;
}

/* !!! TEMP : To remove when clean exit with events is done !!! */
orxVOID orxMain_Exit();

/** Run the test engine
 * @param[in] _u32NbParam Number of parameters read
 * @param[in] _azParams   Array of parameters
 */
orxVOID orxFASTCALL orxTest_Run(orxCONST orxCLOCK_INFO *_pstClockInfo, orxVOID *_pstContext)
{
  orxCHAR zChoice[orxTEST_MAIN_KU32_CHOICE_BUFFER_SIZE];  /* Entry read from user */
  orxS32 s32Val;                                          /* value of entry */

  /* Frozen? */
  if(sstTest.u32Flags & orxTEST_KU32_STATIC_FLAG_FREEZE)
  {
    return;
  }

  /* Show list of registered function */
  orxTest_DisplayMenu();

  /* Get user choice */
  orxTextIO_PrintLn("quit : Quit the test program");
  orxTextIO_ReadString(zChoice, orxTEST_MAIN_KU32_CHOICE_BUFFER_SIZE, "Choice : ");

  /* Check overflow */
  if((orxString_Length(zChoice) > 0) && zChoice[orxString_Length(zChoice)-1] == '\n')
  {
    zChoice[strlen(zChoice)-1] = '\0';
  }

  /* The user wants to quit ? */
  if(orxString_Compare(zChoice, "quit") != 0)
  {
    /* No, so parse its choice */
    if((orxString_ToS32(&s32Val, zChoice, 10) == orxSTATUS_FAILURE))
    {
      /* The value is not a digit */
      orxTextIO_PrintLn("The Value is not a digit");
    }
    else
    {
      if(orxTest_Execute((orxHANDLE)s32Val) == orxSTATUS_FAILURE)
      {
        /* Invalid choice was used */
        orxTextIO_PrintLn("Unknown command");
      }

      /* Freeze? */
      if(sstTest.u32Flags & orxTEST_KU32_STATIC_FLAG_FREEZE)
      {
        return;
      }
    }

    /* Function has been executed. Wait for a pressed key before displaying the menu (clear screen would be fine) */
    orxTextIO_PrintLn("Press Enter to continue");
    getchar();

    /* Reinitialize user choice */
    memset(zChoice, 0, orxTEST_MAIN_KU32_CHOICE_BUFFER_SIZE * sizeof(char));
    s32Val = -1;
  }
  else
  {
    /* Quits program */
    /* !!! TODO : use event !!! */
    /* !!! TEMP : dirty manual exit !!! */
    orxMain_Exit();
  }

  return;
}

/** Starts the test module if needed
 */
orxVOID orxFASTCALL orxTest_Start()
{
  /* In use? */
  if(sstTest.u32Flags & orxTEST_KU32_STATIC_FLAG_IN_USE)
  {
    /* Try to create the clock */
    sstTest.pstClock = orxClock_Create(100, orxCLOCK_TYPE_CORE);

    /* Succesful? */
    if(sstTest.pstClock != orxNULL)
    {
      /* Registers test function */
      if(orxClock_Register(sstTest.pstClock, orxTest_Run, orxNULL, orxMODULE_ID_TEST) == orxSTATUS_SUCCESS)
      {
      }
      else
      {
        /* !!! MSG !!! */
        
        /* Deletes clock */
        orxClock_Delete(sstTest.pstClock);
        sstTest.pstClock = orxNULL;
      }
    }
  }

  return;
}

/** Freeze test module (blocking menu display)
 * @param   (IN)  _bFreeze          Freeze/unfreeze choice
 */
orxVOID orxFASTCALL orxTest_Freeze(orxBOOL _bFreeze)
{
  /* Freezes */
  if(_bFreeze != orxFALSE)
  {
    /* Updates flags */
    sstTest.u32Flags |= orxTEST_KU32_STATIC_FLAG_FREEZE;
  }
  /* Unfreezes */
  else
  {
    /* Updates flags */
    sstTest.u32Flags &= ~orxTEST_KU32_STATIC_FLAG_FREEZE;
  }
}


#endif /* __orxTEST__ */
