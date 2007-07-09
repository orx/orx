/***************************************************************************
 orxScreenshot.c
 Screenshot module
 
 begin                : 07/12/2003
 author               : (C) Arcallians
 email                : iarwain@arcallians.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "utils/orxScreenshot.h"

#include <string.h>
#include <stdio.h>

#include "debug/orxDebug.h"
#include "display/orxDisplay.h"
#include "io/orxFile.h"
#include "io/orxFileSystem.h"
#include "io/orxTextIO.h"
#include "memory/orxMemory.h"
#include "msg/msg_screenshot.h"


/*
 * Platform independant defines
 */

#define orxSCREENSHOT_KU32_FLAG_NONE            0x00000000
#define orxSCREENSHOT_KU32_FLAG_READY           0x00000001

/*
 * Static structure
 */
typedef struct __orxSCREENSHOT_STATIC_t
{
  /* Counter */
  orxU32 u32Counter;

  /* Control flags */
  orxU32 u32Flags;

} orxSCREENSHOT_STATIC;

/*
 * Static data
 */
orxSTATIC orxSCREENSHOT_STATIC sstScreenshot;



/***************************************************************************
 ***************************************************************************
 ******                       LOCAL FUNCTIONS                         ******
 ***************************************************************************
 ***************************************************************************/


/***************************************************************************
 ***************************************************************************
 ******                       PUBLIC FUNCTIONS                        ******
 ***************************************************************************
 ***************************************************************************/

/***************************************************************************
 orxScreenshot_Setup
 Screenshot module setup.

 returns: nothing
 ***************************************************************************/
orxVOID orxScreenshot_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_SCREENSHOT, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_SCREENSHOT, orxMODULE_ID_FILE);
  orxModule_AddDependency(orxMODULE_ID_SCREENSHOT, orxMODULE_ID_TEXTIO);
  orxModule_AddDependency(orxMODULE_ID_SCREENSHOT, orxMODULE_ID_DISPLAY);

  return;
}

/***************************************************************************
 orxScreenshot_Init
 Inits the screenshot system.

 returns: orxSTATUS_SUCCESS/orxSTATUS_FAILURE
 ***************************************************************************/
orxSTATUS orxScreenshot_Init()
{
  orxCHAR zFileName[256];
  orxFILESYSTEM_INFO stFileInfos;
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Not already Initialized? */
  if(!(sstScreenshot.u32Flags & orxSCREENSHOT_KU32_FLAG_READY))
  {
    /* Cleans control structure */
    orxMemory_Set(&sstScreenshot, 0, sizeof(orxSCREENSHOT_STATIC));

    /* Valid? */
    if(orxFileSystem_Exists(orxSCREENSHOT_KZ_DIRECTORY) != orxFALSE)
    {
      /* Gets file to find name */
      orxTextIO_Printf(zFileName, "%s/%s*.*", orxSCREENSHOT_KZ_DIRECTORY, orxSCREENSHOT_KZ_PREFIX);

      /* Finds first screenshot file */
      if(orxFileSystem_FindFirst(zFileName, &stFileInfos) != orxFALSE)
      {
        do
        {
          /* Updates screenshot counter */
          sstScreenshot.u32Counter++;
        }
        /* Till all screenshots have been found */
        while(orxFileSystem_FindNext(&stFileInfos) != orxFALSE);
        
        /* Ends the search */
        orxFileSystem_FindClose(&stFileInfos);
      }

      /* Inits Flags */
      sstScreenshot.u32Flags = orxSCREENSHOT_KU32_FLAG_READY;
      
      /* Success */
      eResult = orxSTATUS_SUCCESS;
    }
    else
    {
      /* !!! MSG !!! */

      /* Can't find folder */
      eResult = orxSTATUS_FAILURE;
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

/***************************************************************************
 orxScreenshot_Exit
 Exits from the screenshot system.

 returns: orxSTATUS_SUCCESS/orxSTATUS_FAILURE
 ***************************************************************************/
orxVOID orxScreenshot_Exit()
{
  /* Initialized? */
  if(sstScreenshot.u32Flags & orxSCREENSHOT_KU32_FLAG_READY)
  {
    /* Updates flags */
    sstScreenshot.u32Flags &= ~orxSCREENSHOT_KU32_FLAG_READY;
  }
  else
  {
    /* !!! MSG !!! */
  }

  return;
}

/***************************************************************************
 orxScreenshot_Take
 Takes a screenshot.

 returns: orxSTATUS_SUCCESS/orxSTATUS_FAILURE
 ***************************************************************************/
orxVOID orxScreenshot_Take()
{
  orxCHAR zName[256];

  /* Checks */
  orxASSERT(sstScreenshot.u32Flags & orxSCREENSHOT_KU32_FLAG_READY)

  /* Computes screenshot name */
  orxTextIO_Printf(zName, "%s/%s-%04ld.%s", orxSCREENSHOT_KZ_DIRECTORY, orxSCREENSHOT_KZ_PREFIX, sstScreenshot.u32Counter, orxSCREENSHOT_KZ_EXT);

  /* Saves it */
  orxDisplay_SaveBitmap(orxDisplay_GetScreenBitmap(), zName);

  /* Logs */
  orxDEBUG_LOG(orxDEBUG_LEVEL_SCREENSHOT, KZ_MSG_SHOT_TAKEN_S, zName);

  /* Updates screenshot counter */
  sstScreenshot.u32Counter++;

  return;
}
