/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2018 Orx-Project
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 *    1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 *
 *    2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 *    3. This notice may not be removed or altered from any source
 *    distribution.
 */

/**
 * @file orxMain.c
 * @date 18/07/2008
 * @author iarwain@orx-project.org
 *
 * Default main executable
 *
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
  orxSTATUS eResult = orxSTATUS_SUCCESS;

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
orxSTATUS orxFASTCALL orxMain_Init()
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Not already initialized? */
  if(!orxFLAG_TEST(sstMain.u32Flags, orxMAIN_KU32_STATIC_FLAG_READY))
  {
    const orxSTRING zGameFileName;

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
        eResult = (orxPlugin_LoadShadow(zGameFileName, zGameFileName) != orxHANDLE_UNDEFINED) ? orxSTATUS_SUCCESS : orxSTATUS_FAILURE;
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
void orxFASTCALL orxMain_Exit()
{
  /* Module initialized ? */
  if(orxFLAG_TEST(sstMain.u32Flags, orxMAIN_KU32_STATIC_FLAG_READY))
  {
    /* Removes event handler */
    orxEvent_RemoveHandler(orxEVENT_TYPE_SYSTEM, orxMain_EventHandler);

    /* Sets module as not ready */
    orxFLAG_SET(sstMain.u32Flags, orxMAIN_KU32_STATIC_FLAG_NONE, orxMAIN_KU32_STATIC_FLAG_READY);
  }

  /* Done */
  return;
}

/** Runs the main engine
 */
orxSTATUS orxFASTCALL orxMain_Run()
{
  orxSTATUS eResult;

  /* Is keyboard module initialized? */
  if(orxModule_IsInitialized(orxMODULE_ID_KEYBOARD) != orxFALSE)
  {
    static orxBOOL sbInsertPressed = orxFALSE, sbF11Pressed = orxFALSE, sbF12Pressed = orxFALSE;

    /* Is escape pressed? */
    if(orxKeyboard_IsKeyPressed(orxKEYBOARD_KEY_ESCAPE) != orxFALSE)
    {
      /* Sends system close event */
      orxEvent_SendShort(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_CLOSE);
    }

    /* Was insert pressed? */
    if(sbInsertPressed != orxFALSE)
    {
      /* No longer pressed? */
      if(orxKeyboard_IsKeyPressed(orxKEYBOARD_KEY_INSERT) == orxFALSE)
      {
        /* Updates key status */
        sbInsertPressed = orxFALSE;
      }
    }
    else
    {
      /* Is insert pressed? */
      if(orxKeyboard_IsKeyPressed(orxKEYBOARD_KEY_INSERT) != orxFALSE)
      {
        /* Reloads config history */
        orxConfig_ReloadHistory();

        /* Updates key status */
        sbInsertPressed = orxTRUE;
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

      /* Was F12 pressed? */
      if(sbF12Pressed != orxFALSE)
      {
        /* No longer pressed? */
        if(orxKeyboard_IsKeyPressed(orxKEYBOARD_KEY_F12) == orxFALSE)
        {
          /* Updates key status */
          sbF12Pressed = orxFALSE;
        }
      }
      else
      {
        /* Is F12 pressed? */
        if(orxKeyboard_IsKeyPressed(orxKEYBOARD_KEY_F12) != orxFALSE)
        {
          /* Captures a screenshot */
          orxScreenshot_Capture();

          /* Updates key status */
          sbF12Pressed = orxTRUE;
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

#ifdef __orxMSVC__

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
  /* Executes orx */
  orx_WinExecute(orxMain_Init, orxMain_Run, orxMain_Exit);

  /* Done! */
  return EXIT_SUCCESS;
}

#endif /* __orxMSVC__ */
