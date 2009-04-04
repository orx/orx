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
 * @file orxMain.c
 * @date 18/07/2008
 * @author iarwain@orx-project.org
 *
 * Default main executable
 *
 * @todo
 */


/** Includes
 */
#include "orx.h"

/** Flags
 */
#define orxMAIN_KU32_STATIC_FLAG_NONE   0x00000000  /**< No flags */

#define orxMAIN_KU32_STATIC_FLAG_READY  0x00000001  /**< Ready flag */
#define orxMAIN_KU32_STATIC_FLAG_EXIT   0x00000002  /**< Exit flag */

#define orxMAIN_KU32_STATIC_MASK_ALL    0xFFFFFFFF  /**< All mask */

/** Misc defines
 */
#define orxMAIN_KZ_CONFIG_SECTION       "Main"      /**< Main config section */
#define orxMAIN_KZ_CONFIG_GAME_FILE     "GameFile"  /**< Game file config key */


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Static structure
 */
typedef struct __orxMAIN_STATIC_t
{
  orxU32 u32Flags;                      /**< Control flags */

} orxMAIN_STATIC;


/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** Static data
 */
static orxMAIN_STATIC sstMain;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

/** Main event handler
 * @param[in]   _pstEvent                     Sent event
 * @return      orxSTATUS_SUCCESS if handled / orxSTATUS_FAILURE otherwise
 */
static orxSTATUS orxFASTCALL orxMain_EventHandler(const orxEVENT *_pstEvent)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(_pstEvent->eType == orxEVENT_TYPE_SYSTEM);

  /* Depending on event ID */
  switch(_pstEvent->eID)
  {
    /* Close event */
    case orxSYSTEM_EVENT_CLOSE:
    {
      /* Updates status */
      orxFLAG_SET(sstMain.u32Flags, orxMAIN_KU32_STATIC_FLAG_EXIT, orxMAIN_KU32_STATIC_FLAG_NONE);

      /* Updates result */
      eResult = orxSTATUS_SUCCESS;

      break;
    }

    default:
    {
      break;
    }
  }

  /* Done! */
  return eResult;
}

/** Inits the main module
 */
orxSTATUS orxMain_Init()
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Not already initialized? */
  if(!orxFLAG_TEST(sstMain.u32Flags, orxMAIN_KU32_STATIC_FLAG_READY))
  {
    orxSTRING zGameFileName;

    /* Sets module as initialized */
    orxFLAG_SET(sstMain.u32Flags, orxMAIN_KU32_STATIC_FLAG_READY, orxMAIN_KU32_STATIC_MASK_ALL);

    /* Registers custom system event handler */
    eResult = orxEvent_AddHandler(orxEVENT_TYPE_SYSTEM, orxMain_EventHandler);

    /* Valid? */
    if(eResult != orxSTATUS_FAILURE)
    {
      /* Selects section */
      orxConfig_SelectSection(orxMAIN_KZ_CONFIG_SECTION);

      /* Has game file? */
      if(orxConfig_HasValue(orxMAIN_KZ_CONFIG_GAME_FILE) != orxFALSE)
      {
        /* Gets the game file name */
        zGameFileName = orxConfig_GetString(orxMAIN_KZ_CONFIG_GAME_FILE);

        /* Loads it */
        eResult = (orxPlugin_LoadUsingExt(zGameFileName, zGameFileName) != orxHANDLE_UNDEFINED) ? orxSTATUS_SUCCESS : orxSTATUS_FAILURE;
      }

      /* Successful? */
      if(eResult != orxSTATUS_FAILURE)
      {
        /* Updates status */
        orxFLAG_SET(sstMain.u32Flags, orxMAIN_KU32_STATIC_FLAG_READY, orxMAIN_KU32_STATIC_MASK_ALL);
      }
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Attempted to initialize Main module when already loaded.");
  }

  /* Done! */
  return eResult;
}

/** Exits from main module
 */
void orxMain_Exit()
{
  /* Module initialized ? */
  if(orxFLAG_TEST(sstMain.u32Flags, orxMAIN_KU32_STATIC_FLAG_READY))
  {
    /* Sets module as not ready */
    orxFLAG_SET(sstMain.u32Flags, orxMAIN_KU32_STATIC_FLAG_NONE, orxMAIN_KU32_STATIC_FLAG_READY);
  }

  /* Done */
  return;
}

/** Runs the main engine
 */
orxSTATUS orxMain_Run()
{
  orxSTATUS eResult;

  /* Is keyboard module initialized? */
  if(orxModule_IsInitialized(orxMODULE_ID_KEYBOARD) != orxFALSE)
  {
    static orxBOOL sbBackSpacePressed = 0, sbF11Pressed;

    /* Is escape pressed? */
    if(orxKeyboard_IsKeyPressed(orxKEYBOARD_KEY_ESCAPE) != orxFALSE)
    {
      /* Sends system close event */
      orxEvent_SendShort(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_CLOSE);
    }

    /* Was backspace pressed? */
    if(sbBackSpacePressed != orxFALSE)
    {
      /* No longer pressed? */
      if(orxKeyboard_IsKeyPressed(orxKEYBOARD_KEY_BACKSPACE) == orxFALSE)
      {
        /* Updates key status */
        sbBackSpacePressed = orxFALSE;
      }
    }
    else
    {
      /* Is backspace pressed? */
      if(orxKeyboard_IsKeyPressed(orxKEYBOARD_KEY_BACKSPACE) != orxFALSE)
      {
        /* Reloads config history */
        orxConfig_ReloadHistory();

        /* Updates key status */
        sbBackSpacePressed = orxTRUE;
      }
    }

    /* Is display module initialized? */
    if(orxModule_IsInitialized(orxMODULE_ID_DISPLAY) != orxFALSE)
    {
      /* Was F11 pressed? */
      if(sbF11Pressed != orxFALSE)
      {
        /* No longer pressed? */
        if(orxKeyboard_IsKeyPressed(orxKEYBOARD_KEY_F11) == orxFALSE)
        {
          /* Updates key status */
          sbF11Pressed = orxFALSE;
        }
      }
      else
      {
        /* Is F11 pressed? */
        if(orxKeyboard_IsKeyPressed(orxKEYBOARD_KEY_F11) != orxFALSE)
        {
          /* Toggles vsync */
          orxDisplay_EnableVSync(!orxDisplay_IsVSyncEnabled());

          /* Updates key status */
          sbF11Pressed = orxTRUE;
        }
      }
    }
  }

  /* Updates result */
  eResult = (orxFLAG_TEST(sstMain.u32Flags, orxMAIN_KU32_STATIC_FLAG_EXIT)) ? orxSTATUS_FAILURE : orxSTATUS_SUCCESS;

  /* Done! */
  return eResult;
}

/** Main entry point
 * @param[in] argc                            Number of parameters
 * @param[in] argv                            List of parameters
 * @return    EXIT_SUCCESS / EXIT_FAILURE
 */
int main(int argc, char **argv)
{
  /* Executes orx */
  orx_Execute(argc, argv, orxMain_Init, orxMain_Run, orxMain_Exit);

  /* Done! */
  return EXIT_SUCCESS;
}
