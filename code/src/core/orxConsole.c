/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2012 Orx-Project
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
 * @file orxConsole.c
 * @date 13/08/2012
 * @author iarwain@orx-project.org
 *
 */


#include "core/orxConsole.h"

#include "debug/orxDebug.h"
#include "debug/orxProfiler.h"
#include "core/orxClock.h"
#include "core/orxCommand.h"
#include "core/orxEvent.h"
#include "io/orxInput.h"
#include "memory/orxMemory.h"
#include "memory/orxBank.h"
#include "utils/orxString.h"


/** Module flags
 */
#define orxCONSOLE_KU32_STATIC_FLAG_NONE              0x00000000                      /**< No flags */

#define orxCONSOLE_KU32_STATIC_FLAG_READY             0x00000001                      /**< Ready flag */
#define orxCONSOLE_KU32_STATIC_FLAG_ENABLED           0x00000002                      /**< Enabled flag */

#define orxCONSOLE_KU32_STATIC_MASK_ALL               0xFFFFFFFF                      /**< All mask */


/** Misc
 */
#define orxCONSOLE_KU32_BUFFER_SIZE                   4096                            /**< Buffer size */

#define orxCONSOLE_KZ_INPUT_SET                       "-=ConsoleSet=-"                /**< Console input set */

#define orxCONSOLE_KZ_INPUT_AUTOCOMPLETE              "AutoComplete"                  /**< Autocomplete input */
#define orxCONSOLE_KZ_INPUT_DELETE                    "Delete"                        /**< Delete input */
#define orxCONSOLE_KZ_INPUT_ENTER                     "Enter"                         /**< Enter input */
#define orxCONSOLE_KZ_INPUT_PREVIOUS                  "Previous"                      /**< Previous input */
#define orxCONSOLE_KZ_INPUT_NEXT                      "Next"                          /**< Next input */

#define orxCONSOLE_KE_KEY_AUTOCOMPLETE                orxKEYBOARD_KEY_TAB             /**< Autocomplete key */
#define orxCONSOLE_KE_KEY_DELETE                      orxKEYBOARD_KEY_BACKSPACE       /**< Delete key */
#define orxCONSOLE_KE_KEY_ENTER                       orxKEYBOARD_KEY_RETURN          /**< Enter key */
#define orxCONSOLE_KE_KEY_PREVIOUS                    orxKEYBOARD_KEY_UP              /**< Previous key */
#define orxCONSOLE_KE_KEY_NEXT                        orxKEYBOARD_KEY_DOWN            /**< Next key */


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Static structure
 */
typedef struct __orxCONSOLE_STATIC_t
{
  orxCHAR                   acBuffer[orxCONSOLE_KU32_BUFFER_SIZE];                    /**< Buffer */
  const orxSTRING           zInputSetBackup;                                          /**< Input set backup */
  orxU32                    u32Flags;                                                 /**< Control flags */

} orxCONSOLE_STATIC;


/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** Static data
 */
static orxCONSOLE_STATIC sstConsole;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

/** Update callback
 */
static void orxFASTCALL orxConsole_Update(const orxCLOCK_INFO *_pstClockInfo, void *_pContext)
{
  /* Profiles */
  orxPROFILER_PUSH_MARKER("orxConsole_Update");

  /* Is enabled? */
  if(orxFLAG_TEST(sstConsole.u32Flags, orxCONSOLE_KU32_STATIC_FLAG_ENABLED))
  {
    //! TODO
  }

  /* Profiles */
  orxPROFILER_POP_MARKER();

  /* Done! */
  return;
}

/** Starts the console
 */
static void orxFASTCALL orxConsole_Start()
{
  /* Not already enabled? */
  if(!orxFLAG_TEST(sstConsole.u32Flags, orxCONSOLE_KU32_STATIC_FLAG_ENABLED))
  {
    /* Registers update callback */
    orxClock_Register(orxClock_FindFirst(orx2F(-1.0f), orxCLOCK_TYPE_CORE), orxConsole_Update, orxNULL, orxMODULE_ID_CONSOLE, orxCLOCK_PRIORITY_HIGH);

    /* Stores current input set */
    sstConsole.zInputSetBackup = orxInput_GetCurrentSet();

    /* Replaces input set */
    orxInput_SelectSet(orxCONSOLE_KZ_INPUT_SET);

    //! TODO
  }

  /* Done! */
  return;
}

/** Stops the console
 */
static void orxFASTCALL orxConsole_Stop()
{
  /* Was enabled? */
  if(orxFLAG_TEST(sstConsole.u32Flags, orxCONSOLE_KU32_STATIC_FLAG_ENABLED))
  {
    /* Unregisters update callback */
    orxClock_Unregister(orxClock_FindFirst(orx2F(-1.0f), orxCLOCK_TYPE_CORE), orxConsole_Update);

    /* Restores previous input set */
    orxInput_SelectSet(sstConsole.zInputSetBackup);

    //! TODO    
  }

  /* Done! */
  return;
}


/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** Console module setup
 */
void orxFASTCALL orxConsole_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_CONSOLE, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_CONSOLE, orxMODULE_ID_BANK);
  orxModule_AddDependency(orxMODULE_ID_CONSOLE, orxMODULE_ID_CLOCK);
  orxModule_AddDependency(orxMODULE_ID_CONSOLE, orxMODULE_ID_COMMAND);
  orxModule_AddDependency(orxMODULE_ID_CONSOLE, orxMODULE_ID_EVENT);
  orxModule_AddDependency(orxMODULE_ID_CONSOLE, orxMODULE_ID_PROFILER);
  orxModule_AddDependency(orxMODULE_ID_CONSOLE, orxMODULE_ID_INPUT);
  orxModule_AddDependency(orxMODULE_ID_CONSOLE, orxMODULE_ID_KEYBOARD);

  /* Done! */
  return;
}

/** Inits console module
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxConsole_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Not already Initialized? */
  if(!(sstConsole.u32Flags & orxCONSOLE_KU32_STATIC_FLAG_READY))
  {
    const orxSTRING zPreviousSet;

    /* Cleans control structure */
    orxMemory_Zero(&sstConsole, sizeof(orxCONSOLE_STATIC));

    /* Backups previous input set */
    zPreviousSet = orxInput_GetCurrentSet();

    /* Replaces input set */
    eResult = orxInput_SelectSet(orxCONSOLE_KZ_INPUT_SET);

    /* Success */
    if(eResult != orxSTATUS_FAILURE)
    {
      /* Binds inputs */
      orxInput_Bind(orxCONSOLE_KZ_INPUT_AUTOCOMPLETE, orxINPUT_TYPE_KEYBOARD_KEY, orxCONSOLE_KE_KEY_AUTOCOMPLETE);
      orxInput_Bind(orxCONSOLE_KZ_INPUT_DELETE, orxINPUT_TYPE_KEYBOARD_KEY, orxCONSOLE_KE_KEY_DELETE);
      orxInput_Bind(orxCONSOLE_KZ_INPUT_ENTER, orxINPUT_TYPE_KEYBOARD_KEY, orxCONSOLE_KE_KEY_ENTER);
      orxInput_Bind(orxCONSOLE_KZ_INPUT_PREVIOUS, orxINPUT_TYPE_KEYBOARD_KEY, orxCONSOLE_KE_KEY_PREVIOUS);
      orxInput_Bind(orxCONSOLE_KZ_INPUT_NEXT, orxINPUT_TYPE_KEYBOARD_KEY, orxCONSOLE_KE_KEY_NEXT);

      /* Restores previous set */
      orxInput_SelectSet(zPreviousSet);

      /* Inits Flags */
      sstConsole.u32Flags = orxCONSOLE_KU32_STATIC_FLAG_READY;
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Tried to initialize console module when it was already initialized.");

    /* Already initialized */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Done! */
  return eResult;
}

/** Exits from console module
 */
void orxFASTCALL orxConsole_Exit()
{
  /* Initialized? */
  if(sstConsole.u32Flags & orxCONSOLE_KU32_STATIC_FLAG_READY)
  {
    /* Stops console */
    orxConsole_Stop();

    /* Updates flags */
    sstConsole.u32Flags &= ~orxCONSOLE_KU32_STATIC_FLAG_READY;
  }

  /* Done! */
  return;
}

/** Enables/disables the console
 * @param[in]   _bEnable      Enable / disable
 */
void orxFASTCALL orxConsole_Enable(orxBOOL _bEnable)
{
  /* Checks */
  orxASSERT(sstConsole.u32Flags & orxCONSOLE_KU32_STATIC_FLAG_READY);

  /* Enable? */
  if(_bEnable != orxFALSE)
  {
    /* Starts console */
    orxConsole_Start();

    /* Updates status flags */
    orxFLAG_SET(sstConsole.u32Flags, orxCONSOLE_KU32_STATIC_FLAG_ENABLED, orxCONSOLE_KU32_STATIC_FLAG_NONE);
  }
  else
  {
    /* Stops console */
    orxConsole_Stop();

    /* Updates status flags */
    orxFLAG_SET(sstConsole.u32Flags, orxCONSOLE_KU32_STATIC_FLAG_NONE, orxCONSOLE_KU32_STATIC_FLAG_ENABLED);
  }

  /* Done! */
  return;
}

/** Is the console enabled?
 * @return      orxTRUE if enabled, orxFALSE otherwise
 */
orxBOOL orxFASTCALL orxConsole_IsEnabled()
{
  orxBOOL bResult;

  /* Checks */
  orxASSERT(sstConsole.u32Flags & orxCONSOLE_KU32_STATIC_FLAG_READY);

  /* Updates result */
  bResult = orxFLAG_TEST(sstConsole.u32Flags, orxCONSOLE_KU32_STATIC_FLAG_ENABLED) ? orxTRUE : orxFALSE;

  /* Done! */
  return bResult;
}

/** Logs text to the console
 * @param[in]   _zText        Text to log
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxConsole_Log(const orxSTRING _zText)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstConsole.u32Flags & orxCONSOLE_KU32_STATIC_FLAG_READY);

  //! TODO

  /* Done! */
  return eResult;
}
