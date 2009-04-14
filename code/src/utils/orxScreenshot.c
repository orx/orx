/* Orx - Portable Game Engine
 *
 * Orx is the legal property of its developers, whose names
 * are listed in the COPYRIGHT file distributed 
 * with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

/**
 * @file orxScreenshot.c
 * @date 07/12/2003
 * @author iarwain@orx-project.org
 *
 * @todo
 * - Make a better init system for finding the next screenshot name
 *   (this one is broken is all screenshots aren't contiguous)
 */


#include "utils/orxScreenshot.h"

#include <string.h>
#include <stdio.h>

#include "debug/orxDebug.h"
#include "display/orxDisplay.h"
#include "io/orxFile.h"
#include "io/orxFileSystem.h"
#include "utils/orxString.h"
#include "memory/orxMemory.h"


/** Module flags
 */
#define orxSCREENSHOT_KU32_STATIC_FLAG_NONE       0x00000000
#define orxSCREENSHOT_KU32_STATIC_FLAG_READY      0x00000001


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Static structure
 */
typedef struct __orxSCREENSHOT_STATIC_t
{
  /* Counter */
  orxU32 u32Counter;

  /* Control flags */
  orxU32 u32Flags;

} orxSCREENSHOT_STATIC;


/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** Static data
 */
static orxSCREENSHOT_STATIC sstScreenshot;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/


/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** Screenshot module setup
 */
void orxFASTCALL orxScreenshot_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_SCREENSHOT, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_SCREENSHOT, orxMODULE_ID_FILE);
  orxModule_AddDependency(orxMODULE_ID_SCREENSHOT, orxMODULE_ID_DISPLAY);

  return;
}

/** Inits the screenshot module
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxScreenshot_Init()
{
  orxCHAR zFileName[256];
  orxFILESYSTEM_INFO stFileInfos;
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Not already Initialized? */
  if(!(sstScreenshot.u32Flags & orxSCREENSHOT_KU32_STATIC_FLAG_READY))
  {
    /* Cleans control structure */
    orxMemory_Zero(&sstScreenshot, sizeof(orxSCREENSHOT_STATIC));

    /* Valid? */
    if(orxFileSystem_Exists(orxSCREENSHOT_KZ_DIRECTORY) != orxFALSE)
    {
      /* Gets file to find name */
      orxString_NPrint(zFileName, 256, "%s/%s*.*", orxSCREENSHOT_KZ_DIRECTORY, orxSCREENSHOT_KZ_PREFIX);

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
      sstScreenshot.u32Flags = orxSCREENSHOT_KU32_STATIC_FLAG_READY;

      /* Success */
      eResult = orxSTATUS_SUCCESS;
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_SCREENSHOT, "Can not find screenshot directory.");

      /* Can't find folder */
      eResult = orxSTATUS_FAILURE;
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_SCREENSHOT, "Tried to initalize Screenshot module when it was already initialized.");

    /* Already initialized */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Done! */
  return eResult;
}

/** Exits from the screenshot module
 */
void orxFASTCALL orxScreenshot_Exit()
{
  /* Initialized? */
  if(sstScreenshot.u32Flags & orxSCREENSHOT_KU32_STATIC_FLAG_READY)
  {
    /* Updates flags */
    sstScreenshot.u32Flags &= ~orxSCREENSHOT_KU32_STATIC_FLAG_READY;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_SCREENSHOT, "Tried to exit from Screenshot module when it wasn't initialized.");
  }

  return;
}

/** Captures a screenshot
 */
void orxFASTCALL orxScreenshot_Capture()
{
  orxCHAR zName[256];

  /* Checks */
  orxASSERT(sstScreenshot.u32Flags & orxSCREENSHOT_KU32_STATIC_FLAG_READY)

  /* Computes screenshot name */
  orxString_NPrint(zName, 256, "%s/%s-%04ld.%s", orxSCREENSHOT_KZ_DIRECTORY, orxSCREENSHOT_KZ_PREFIX, sstScreenshot.u32Counter, orxSCREENSHOT_KZ_EXT);

  /* Saves it */
  orxDisplay_SaveBitmap(orxDisplay_GetScreenBitmap(), zName);

  /* Logs */
  orxDEBUG_PRINT(orxDEBUG_LEVEL_SCREENSHOT, "Screenshot captured: <%s>.", zName);

  /* Updates screenshot counter */
  sstScreenshot.u32Counter++;

  return;
}
