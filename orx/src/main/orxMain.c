/**
 * @file orxMain.c
 * 
 * Main program implementation
 * 
 */
 
 /***************************************************************************
 orxMain.c
 Main program implementation
 
 begin                : 04/09/2005
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

#include "orx.h"

#define orxMAIN_KU32_FLAG_NONE  0x00000000  /**< No flags have been set */
#define orxMAIN_KU32_FLAG_READY 0x00000001  /**< The module has been initialized */

#define orxMAIN_KU32_FLAG_TEST  0x00000002  /**< orx runs in test mode */
#define orxMAIN_KU32_FLAG_EXIT  0x00000004  /**< an Exit Event has been received */

/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/
typedef struct __orxMAIN_STATIC_t
{
  orxHANDLE hTimePlugin;

  orxU32 u32Flags; /**< Flags set by the main module */

} orxMAIN_STATIC;

/***************************************************************************
 * Module global variable                                                  *
 ***************************************************************************/
orxSTATIC orxMAIN_STATIC sstMain;

/***************************************************************************
 * Functions                                                               *
 ***************************************************************************/

/** Set Test flag (the test parameter has been given)
 * @param[in] _u32NbParam Number of extra parameters read for this option
 * @param[in] _azParams   Array of extra parameters (the first one is always the option name)
 * @return Returns orxSTATUS_SUCCESS if informations read are correct, orxSTATUS_FAILED if a problem has occured
 */
orxSTATUS orxMain_ParamTest(orxU32 _u32NbParam, orxSTRING _azParams[])
{
  /* Set Test Flag */
  sstMain.u32Flags |= orxMAIN_KU32_FLAG_TEST;
  
  /* Done */
  return orxSTATUS_SUCCESS;
}

/** Initialize the main module (will initialize all needed modules)
 */
orxSTATUS orxMain_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILED;

  /* Don't call twice the init function */
  if (!(sstMain.u32Flags & orxMAIN_KU32_FLAG_READY))
  {
    /* Inits plugin module */
    if(orxModule_Init(orxMODULE_ID_PLUGIN) == orxSTATUS_SUCCESS)
    {
      /* Loads time plugin */
      sstMain.hTimePlugin = orxPlugin_LoadUsingExt("Time_SDL", "time");
      
      /* Valid? */
      if(sstMain.hTimePlugin != orxHANDLE_Undefined)
      {
        /* Inits all remaining modules */
        if(orxModule_InitAll() == orxSTATUS_SUCCESS)
        {
          /* Sets module as initialized */
          sstMain.u32Flags |= orxMAIN_KU32_FLAG_READY;

          /* Success */
          eResult = orxSTATUS_SUCCESS;
        }
      }
    }
  }
  else
  {
    /* !!! MSG !!! */

    /* Already initialized */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Done! */
  return eResult;
}

/** Exit main module
 */
orxVOID orxMain_Exit()
{
  /* Module initialized ? */
  if((sstMain.u32Flags & orxMAIN_KU32_FLAG_READY) == orxMAIN_KU32_FLAG_READY)
  {
    /* Set module as not ready */
    sstMain.u32Flags &= ~orxMAIN_KU32_FLAG_READY;
    
    /* Exits from all modules */
    orxModule_ExitAll();
  }

  /* Done */
  return;
}

/** Run the main engine
 * @param[in] _u32NbParam Number of parameters read
 * @param[in] _azParams   Array of parameters
 */
orxVOID orxMain_Run(orxU32 _u32NbParam, orxSTRING _azParams[])
{
  /* Init the Engine */
  if(orxMain_Init() == orxSTATUS_SUCCESS)
  {
    /* Parse the command line for the second time (now all modules have registered their options) */
    if(orxParam_Parse(_u32NbParam, _azParams) == orxSTATUS_SUCCESS)
    {
      /* Main Loop (Until Exit event received) */
      while((sstMain.u32Flags & orxMAIN_KU32_FLAG_EXIT) != orxMAIN_KU32_FLAG_EXIT)
      {
        /* Update clocks */
        orxClock_Update();

        /* Sleep the program for 1ms (to help the scheduler) */
        orxTime_Delay(1);
      }
    }
  }

  /* Exits from the engine */
  orxMain_Exit();
  
  return;
}

/** Main function
 * @param[in] argc  Number of parameters
 * @param[in] argv  List array of parameters
 * @note Since the event function is not registered, the program will not
 * be able to exit properly.
 */
int main(int argc, char **argv)
{
  /* Inits the Debug System */
  orxDEBUG_INIT();

  /* Registers all modules */
  orxModule_RegisterAll();

  /* Calls all modules setup */
  orxModule_SetupAll();

  /* Inits the parser */
  if(orxModule_Init(orxMODULE_ID_PARAM) == orxSTATUS_SUCCESS)
  {
#ifdef __orxTEST__ /* Only compile this portion of code if __orxTEST__ is used */
    orxPARAM stParam;
    
    /* Register a parameter to start the test program */
    stParam.u32Flags    = 0;
    stParam.pfnParser   = orxMain_ParamTest;
    stParam.zShortName  = "T";
    stParam.zLongName   = "test";
    stParam.zShortDesc  = "Start the test program";
    stParam.zLongDesc   = "Start the test program instead of orx engine. Test program allows to test and debug orx modules.";

    orxParam_Register(&stParam);

#endif /* __orxTEST__ */
    
    /* Parses the command line */
    if(orxParam_Parse(argc, argv) == orxSTATUS_SUCCESS)
    {
      /* Starts the correct program (test program or engine) */
      if((sstMain.u32Flags & orxMAIN_KU32_FLAG_TEST) == orxMAIN_KU32_FLAG_TEST)
      {
        /* Runs the test programm */
        orxTest_Run(argc, argv);
      }
      else
      {
        /* Runs the engine */
        orxMain_Run(argc, argv);
      }
    }
  }

  /* Exit from all modules */
  orxModule_ExitAll();
    
  /* Exits from the Debug system */
  orxDEBUG_EXIT();

  return EXIT_SUCCESS;
}
