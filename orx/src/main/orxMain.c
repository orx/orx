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

#define orxMAIN_KU32_FLAG_EXIT  0x00000004  /**< an Exit Event has been received */

/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/
typedef struct __orxMAIN_STATIC_t
{
  orxU32 u32Flags; /**< Flags set by the main module */
  orxCLOCK *pstClock;
} orxMAIN_STATIC;

/***************************************************************************
 * Module global variable                                                  *
 ***************************************************************************/
orxSTATIC orxMAIN_STATIC sstMain;

/***************************************************************************
 * Functions                                                               *
 ***************************************************************************/

orxVOID orxFASTCALL orxMain_TestClock(orxCONST orxCLOCK_INFO *_pstClockInfo, orxVOID *_pstContext)
{
  orxTextIO_PrintLn("eType = %d, ", _pstClockInfo->eType);
  orxTextIO_PrintLn("u32TickCounter = %lu, ", _pstClockInfo->u32TickCounter);
  orxTextIO_PrintLn("fTickSize = %f, ", _pstClockInfo->fTickSize);
  orxTextIO_PrintLn("fTickValue = %f, ", _pstClockInfo->fTickValue);
  orxTextIO_PrintLn("eModType = %d, ", _pstClockInfo->eModType);
  orxTextIO_PrintLn("fModValue = %f, ", _pstClockInfo->fModValue);
  orxTextIO_PrintLn("fDT = %f, ", _pstClockInfo->fDT);
  orxTextIO_PrintLn("Time = %lu, ", _pstClockInfo->stTime);
}


/** Initialize the main module (will initialize all needed modules)
 */
orxSTATUS orxMain_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILED;

  /* Init Debug System */
  orxDEBUG_INIT();

  /* Don't call twice the init function */
  if (!(sstMain.u32Flags & orxMAIN_KU32_FLAG_READY))
  {
    /* Try to Init the Engine */
    if ((orxDEPEND_INIT(Depend) &
         orxDEPEND_INIT(Plugin)) == orxSTATUS_SUCCESS)
         /** Todo : Complete remaining dependencies */
    {
      if (orxPlugin_LoadUsingExt("Time_SDL", "time"))
      {
        if ((orxDEPEND_INIT(Memory) &
             orxDEPEND_INIT(TextIO) &
             orxDEPEND_INIT(Param) &
             orxDEPEND_INIT(Time) &
             orxDEPEND_INIT(Clock)) == orxSTATUS_SUCCESS)
        {
          /* Clear the static control */
          orxMemory_Set(&sstMain, 0, sizeof(orxMAIN_STATIC));
          
          /* Create a clock */
          sstMain.pstClock = orxClock_Create(1000, orxCLOCK_TYPE_CORE);
          
          /* Register a test function */
          orxClock_Register(sstMain.pstClock, orxMain_TestClock, orxNULL);

          /* Set module as initialized */
          sstMain.u32Flags |= orxMAIN_KU32_FLAG_READY;
          
          /* Success */
          eResult = orxSTATUS_SUCCESS;
        }
      }
    }
  }
  
  /* Done! */
  return eResult;
}

/** Exit main module
 */
orxVOID orxMain_Exit()
{
  /* Module initialized ? */
  if ((sstMain.u32Flags & orxMAIN_KU32_FLAG_READY) == orxMAIN_KU32_FLAG_READY)
  {
    /* Set module as not ready */
    sstMain.u32Flags &= ~orxMAIN_KU32_FLAG_READY;
  }
  
  orxDEPEND_EXIT(Clock);
  orxDEPEND_EXIT(Time);
  orxDEPEND_EXIT(Param);
  orxDEPEND_EXIT(TextIO);
  orxDEPEND_EXIT(Memory);
  orxDEPEND_EXIT(Plugin);
  orxDEPEND_EXIT(Depend);

  /* Exit Debug system */
  orxDEBUG_EXIT();

  /* Done */
  return;
}

/** Main function
 * @param[in] _u32NbParam  Number of parameters
 * @param[in] _azParam     List array of parameters
 * @note Since the event function is not registered, the program will not
 * be able to exit properly.
 */
int main(int argc, char **argv)
{
  /* Init the Engine */
  if (orxMain_Init() == orxSTATUS_SUCCESS)
  {
    /* Parse the command line */
    if (orxParam_Parse(argc, argv) == orxSTATUS_SUCCESS)
    {
      /* Main Loop (Until Exit event received) */
      while ((sstMain.u32Flags & orxMAIN_KU32_FLAG_EXIT) != orxMAIN_KU32_FLAG_EXIT)
      {
        /* Update clocks */
        orxClock_Update();
  
        /* Sleep the program for 1ms (to help the scheduler) */
        orxTime_Delay(1);
      }
    }
  }

  /* Exit the engine */
  orxMain_Exit();
  
  return 0;
}
