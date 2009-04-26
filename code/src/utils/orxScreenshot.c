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
 */


#include "utils/orxScreenshot.h"

#include "debug/orxDebug.h"
#include "core/orxConfig.h"
#include "display/orxDisplay.h"
#include "io/orxFile.h"
#include "io/orxFileSystem.h"
#include "utils/orxString.h"
#include "memory/orxMemory.h"


/** Module flags
 */
#define orxSCREENSHOT_KU32_STATIC_FLAG_NONE                     0x00000000

#define orxSCREENSHOT_KU32_STATIC_FLAG_READY                    0x00000001
#define orxSCREENSHOT_KU32_STATIC_FLAG_VALID_DIRECTORY          0x00000002

#define orxSCREENSHOT_KU32_STATIC_MASK_ALL                      0xFFFFFFFF


/** Misc defines
 */
#define orxSCREENSHOT_KU32_BUFFER_SIZE                          256

#define orxSCREENSHOT_KZ_CONFIG_SECTION                         "Screenshot"
#define orxSCREENSHOT_KZ_CONFIG_DIRECTORY                       "Directory"
#define orxSCREENSHOT_KZ_CONFIG_BASE_NAME                       "BaseName"
#define orxSCREENSHOT_KZ_CONFIG_EXTENSION                       "Extension"
#define orxSCREENSHOT_KZ_CONFIG_DIGITS                          "Digits"


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Static structure
 */
typedef struct __orxSCREENSHOT_STATIC_t
{
  orxU32    u32ScreenshotIndex;                                 /**< Screenshot index */
  orxSTRING zDirectoryName;                                     /**< Directory name */
  orxSTRING zScreenshotBaseName;                                /**< Screenshot base name */
  orxSTRING zScreenshotExtension;                               /**< Screenshot extension */
  orxU32    u32ScreenshotDigits;                                /**< Screenshot digits */
  orxU32    u32Flags;                                           /**< Control flags */
  orxCHAR   acScreenshotBuffer[orxSCREENSHOT_KU32_BUFFER_SIZE]; /**< Screenshot file name buffer */

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

/** Computes next screenshot index
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
static orxINLINE orxSTATUS orxScreenshot_ComputeIndex()
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstScreenshot.u32Flags & orxSCREENSHOT_KU32_STATIC_FLAG_READY)

  do
  {
    /* Updates screenshot index */
    sstScreenshot.u32ScreenshotIndex++;

    /* Gets file to find name */
    orxString_NPrint(sstScreenshot.acScreenshotBuffer, orxSCREENSHOT_KU32_BUFFER_SIZE, "%s/%s%0*ld.%s", sstScreenshot.zDirectoryName, sstScreenshot.zScreenshotBaseName, sstScreenshot.u32ScreenshotDigits, sstScreenshot.u32ScreenshotIndex, sstScreenshot.zScreenshotExtension);
  }
  /* Till not found */
  while(orxFileSystem_Exists(sstScreenshot.acScreenshotBuffer) != orxFALSE);

  /* Done! */
  return eResult;
}

/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** Screenshot module setup
 */
void orxFASTCALL orxScreenshot_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_SCREENSHOT, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_SCREENSHOT, orxMODULE_ID_CONFIG);
  orxModule_AddDependency(orxMODULE_ID_SCREENSHOT, orxMODULE_ID_FILESYSTEM);
  orxModule_AddDependency(orxMODULE_ID_SCREENSHOT, orxMODULE_ID_DISPLAY);

  return;
}

/** Inits the screenshot module
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxScreenshot_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Not already Initialized? */
  if(!(sstScreenshot.u32Flags & orxSCREENSHOT_KU32_STATIC_FLAG_READY))
  {
    /* Cleans control structure */
    orxMemory_Zero(&sstScreenshot, sizeof(orxSCREENSHOT_STATIC));

    /* Inits flags */
    sstScreenshot.u32Flags = orxSCREENSHOT_KU32_STATIC_FLAG_READY;

    /* Uses default directory, base name, extension & digits */
    sstScreenshot.zDirectoryName        = orxSCREENSHOT_KZ_DEFAULT_DIRECTORY_NAME;
    sstScreenshot.zScreenshotBaseName   = orxSCREENSHOT_KZ_DEFAULT_BASE_NAME;
    sstScreenshot.zScreenshotExtension  = orxSCREENSHOT_KZ_DEFAULT_EXTENSION;
    sstScreenshot.u32ScreenshotDigits   = orxSCREENSHOT_KU32_DEFAULT_DIGITS;

    /* Pushes section */
    if((orxConfig_HasSection(orxSCREENSHOT_KZ_CONFIG_SECTION) != orxFALSE)
    && (orxConfig_PushSection(orxSCREENSHOT_KZ_CONFIG_SECTION) != orxSTATUS_FAILURE))
    {
      orxSTRING zValue;
      orxU32    u32Digits;

      /* Gets directory name */
      zValue = orxConfig_GetString(orxSCREENSHOT_KZ_CONFIG_DIRECTORY);

      /* Valid? */
      if((zValue != orxNULL) && (zValue != orxSTRING_EMPTY))
      {
        /* Stores it */
        sstScreenshot.zDirectoryName = zValue;
      }

      /* Gets screenshot base name */
      zValue = orxConfig_GetString(orxSCREENSHOT_KZ_CONFIG_BASE_NAME);

      /* Valid? */
      if((zValue != orxNULL) && (zValue != orxSTRING_EMPTY))
      {
        /* Stores it */
        sstScreenshot.zScreenshotBaseName = zValue;
      }

      /* Gets screenshot extension */
      zValue = orxConfig_GetString(orxSCREENSHOT_KZ_CONFIG_EXTENSION);

      /* Valid? */
      if((zValue != orxNULL) && (zValue != orxSTRING_EMPTY))
      {
        /* Stores it */
        sstScreenshot.zScreenshotExtension = zValue;
      }

      /* Gets digit number */
      if((u32Digits = orxConfig_GetU32(orxSCREENSHOT_KZ_CONFIG_DIGITS)) > 0)
      {
        /* Stores it */
        sstScreenshot.u32ScreenshotDigits = u32Digits;
      }

      /* Pops previous section */
      orxConfig_PopSection();
    }

    /* Valid? */
    if(orxFileSystem_Exists(sstScreenshot.zDirectoryName) != orxFALSE)
    {
      /* Computes current index */
      eResult = orxScreenshot_ComputeIndex();

      /* Success? */
      if(eResult != orxSTATUS_FAILURE)
      {
        /* Inits Flags */
        orxFLAG_SET(sstScreenshot.u32Flags, orxSCREENSHOT_KU32_STATIC_FLAG_VALID_DIRECTORY, orxSCREENSHOT_KU32_STATIC_FLAG_NONE);
      }
      else
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_SCREENSHOT, "Can't compute current screenshot index.");
      }
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_SCREENSHOT, "Invalid directory [%s]. Please create it to enable screenshots.", sstScreenshot.zDirectoryName);

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
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxScreenshot_Capture()
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(sstScreenshot.u32Flags & orxSCREENSHOT_KU32_STATIC_FLAG_READY);

  /* Has valid directory? */
  if(orxFLAG_TEST(sstScreenshot.u32Flags, orxSCREENSHOT_KU32_STATIC_FLAG_VALID_DIRECTORY))
  {
    /* Saves it */
    eResult = orxDisplay_SaveBitmap(orxDisplay_GetScreenBitmap(), sstScreenshot.acScreenshotBuffer);

    /* Success? */
    if(eResult != orxSTATUS_FAILURE)
    {
      /* Logs */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_SCREENSHOT, "Screenshot captured to [%s].", sstScreenshot.acScreenshotBuffer);

      /* Computes screenshot index */
      orxScreenshot_ComputeIndex();
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_SCREENSHOT, "Can't capture screenshot as directory [%s] is invalid.", sstScreenshot.zDirectoryName);

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}
