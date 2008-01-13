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

#define orxMAIN_KU32_STATIC_FLAG_NONE   0x00000000  /**< No flags have been set */
#define orxMAIN_KU32_STATIC_FLAG_READY  0x00000001  /**< The module has been initialized */

#define orxMAIN_KU32_STATIC_FLAG_EXIT   0x00000002  /**< an Exit Event has been received */


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/
typedef struct __orxMAIN_STATIC_t
{
  orxU32 u32Flags; /**< Flags set by the main module */

} orxMAIN_STATIC;

/***************************************************************************
 * Module global variable                                                  *
 ***************************************************************************/
orxSTATIC orxMAIN_STATIC sstMain;

/***************************************************************************
 * Functions                                                               *
 ***************************************************************************/

/** Main module setup
 */
orxVOID orxMain_Setup()
{
  /* Adds module dependencies */
#ifdef __orxTEST__
  orxModule_AddDependency(orxMODULE_ID_MAIN, orxMODULE_ID_TEST);
#else /* __orxTEST__ */
  orxModule_AddDependency(orxMODULE_ID_MAIN, orxMODULE_ID_CLOCK);
#endif /* __orxTEST__ */

  return;
}

/** Initialize the main module (will initialize all needed modules)
 */
orxSTATUS orxMain_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Don't call twice the init function */
  if(!(sstMain.u32Flags & orxMAIN_KU32_STATIC_FLAG_READY))
  {
    /* Sets module as initialized */
    sstMain.u32Flags |= orxMAIN_KU32_STATIC_FLAG_READY;

    /* Success */
    eResult = orxSTATUS_SUCCESS;
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
  if((sstMain.u32Flags & orxMAIN_KU32_STATIC_FLAG_READY) == orxMAIN_KU32_STATIC_FLAG_READY)
  {
    /* Set module as not ready */
    sstMain.u32Flags &= ~orxMAIN_KU32_STATIC_FLAG_READY;

    /* !!! TEMP : untill exit triggered by events !!! */
    sstMain.u32Flags |= orxMAIN_KU32_STATIC_FLAG_EXIT;

//    /* Exits from all modules */
//    orxModule_ExitAll();
  }

  /* Done */
  return;
}

/** Run the main engine
 */
orxVOID orxMain_Run()
{
  /* Main Loop (Until Exit event received) */
  while((sstMain.u32Flags & orxMAIN_KU32_STATIC_FLAG_EXIT) != orxMAIN_KU32_STATIC_FLAG_EXIT)
  {
    /* Update clocks */
    orxClock_Update();

    /* Wait for next time slice */
    orxTime_Delay(orxFLOAT_0);
  }

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

  /* Registers main module */
  orxModule_Register(orxMODULE_ID_MAIN, orxMain_Setup, orxMain_Init, orxMain_Exit);

  /* Registers all modules */
  orxModule_RegisterAll();

  /* Calls all modules setup */
  orxModule_SetupAll();

  /* Inits the parser */
  if(orxModule_Init(orxMODULE_ID_PARAM) == orxSTATUS_SUCCESS)
  {
    /* Parse the command line for the second time (now all modules have registered their options) */
    if(orxParam_Parse(argc, argv) == orxSTATUS_SUCCESS)
    {
      /* Init the Engine */
      if(orxModule_Init(orxMODULE_ID_MAIN) == orxSTATUS_SUCCESS)
      {
        /* Runs the engine */
        orxMain_Run();

        /* Exits from test module */
        orxModule_Exit(orxMODULE_ID_MAIN);
      }

      /* Exits from the engine */
      orxMain_Exit();

      /* !!! TEMP : till events are used !!! */
      /* Exits from all modules */
      orxModule_ExitAll();
    }
  }

  /* Exits from the Debug system */
  orxDEBUG_EXIT();

  return EXIT_SUCCESS;
}
