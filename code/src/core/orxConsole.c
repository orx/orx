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
#include "core/orxConfig.h"
#include "core/orxEvent.h"
#include "io/orxInput.h"
#include "memory/orxMemory.h"
#include "memory/orxBank.h"
#include "object/orxStructure.h"
#include "utils/orxString.h"


/** Module flags
 */
#define orxCONSOLE_KU32_STATIC_FLAG_NONE              0x00000000                      /**< No flags */

#define orxCONSOLE_KU32_STATIC_FLAG_READY             0x00000001                      /**< Ready flag */
#define orxCONSOLE_KU32_STATIC_FLAG_ENABLED           0x00000002                      /**< Enabled flag */

#define orxCONSOLE_KU32_STATIC_MASK_ALL               0xFFFFFFFF                      /**< All mask */


/** Misc
 */
#define orxCONSOLE_KU32_LOG_BUFFER_SIZE               8192                            /**< Log buffer size */
#define orxCONSOLE_KU32_LOG_LINE_LENGTH               80                              /**< Log line length */

#define orxCONSOLE_KU32_INPUT_ENTRY_SIZE              256                             /**< Input entry size */
#define orxCONSOLE_KU32_INPUT_ENTRY_NUMBER            64                              /**< Input entry number */

#define orxCONSOLE_KZ_CONFIG_SECTION                  "Console"                       /**< Config section name */
#define orxCONSOLE_KZ_CONFIG_TOGGLE_KEY               "ToggleKey"                     /**< Toggle key */

#define orxCONSOLE_KZ_INPUT_SET                       "-=ConsoleSet=-"                /**< Console input set */

#define orxCONSOLE_KZ_INPUT_TOGGLE                    "-=ToggleConsole=-"             /**< Toggle console input */
#define orxCONSOLE_KE_DEFAULT_KEY_TOGGLE              orxKEYBOARD_KEY_TILDE           /**< Default toggle key */

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

/** Input entry
 */
typedef struct __orxCONSOLE_INPUT_ENTRY_t
{
  orxCHAR acBuffer[orxCONSOLE_KU32_INPUT_ENTRY_SIZE];

} orxCONSOLE_INPUT_ENTRY;

/** Static structure
 */
typedef struct __orxCONSOLE_STATIC_t
{
  orxCHAR                   acLogBuffer[orxCONSOLE_KU32_LOG_BUFFER_SIZE];             /**< Log buffer */
  orxCONSOLE_INPUT_ENTRY    astInputEntryList[orxCONSOLE_KU32_INPUT_ENTRY_NUMBER];    /**< Input entry number */
  orxU32                    u32LogIndex;                                              /**< Log buffer index */
  orxU32                    u32LogEndIndex;                                           /**< Log end index */
  const orxFONT            *pstFont;
  const orxSTRING           zPreviousInputSet;                                        /**< Previous input set */
  orxINPUT_TYPE             eToggleKeyType;                                           /**< Toggle key type */
  orxENUM                   eToggleKeyID;                                             /**< Toggle key ID */
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

  /* Should toggle? */
  if((orxInput_IsActive(orxCONSOLE_KZ_INPUT_TOGGLE) != orxFALSE) && (orxInput_HasNewStatus(orxCONSOLE_KZ_INPUT_TOGGLE) != orxFALSE))
  {
    /* Toggles it */
    orxConsole_Enable(!orxConsole_IsEnabled());
  }

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
    /* Stores current input set */
    sstConsole.zPreviousInputSet = orxInput_GetCurrentSet();

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
    /* Restores previous input set */
    orxInput_SelectSet(sstConsole.zPreviousInputSet);

    //! TODO
  }

  /* Done! */
  return;
}

/** Event handler
 * @param[in]   _pstEvent                     Sent event
 * @return      orxSTATUS_SUCCESS if handled / orxSTATUS_FAILURE otherwise
 */
static orxSTATUS orxFASTCALL orxConsole_EventHandler(const orxEVENT *_pstEvent)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(_pstEvent->eType == orxEVENT_TYPE_INPUT);

  /* Depending on event ID */
  switch(_pstEvent->eID)
  {
    /* Select set */
    case orxINPUT_EVENT_SELECT_SET:
    {
      /* Forces toggle input binding */
      orxInput_Bind(orxCONSOLE_KZ_INPUT_TOGGLE, sstConsole.eToggleKeyType, sstConsole.eToggleKeyID);

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
  orxModule_AddDependency(orxMODULE_ID_CONSOLE, orxMODULE_ID_CONFIG);
  orxModule_AddDependency(orxMODULE_ID_CONSOLE, orxMODULE_ID_EVENT);
  orxModule_AddDependency(orxMODULE_ID_CONSOLE, orxMODULE_ID_PROFILER);
  orxModule_AddDependency(orxMODULE_ID_CONSOLE, orxMODULE_ID_INPUT);
  orxModule_AddDependency(orxMODULE_ID_CONSOLE, orxMODULE_ID_KEYBOARD);
  orxModule_AddDependency(orxMODULE_ID_CONSOLE, orxMODULE_ID_STRUCTURE);
  orxModule_AddDependency(orxMODULE_ID_CONSOLE, orxMODULE_ID_FONT);

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

    /* Stores default toggle key */
    sstConsole.eToggleKeyType = orxINPUT_TYPE_KEYBOARD_KEY;
    sstConsole.eToggleKeyID   = orxCONSOLE_KE_DEFAULT_KEY_TOGGLE;

    /* Pushes its section */
    orxConfig_PushSection(orxCONSOLE_KZ_CONFIG_SECTION);

    /* Has toggle key? */
    if(orxConfig_HasValue(orxCONSOLE_KZ_CONFIG_TOGGLE_KEY) != orxFALSE)
    {
      orxINPUT_TYPE eType;
      orxENUM       eID;

      /* Gets its type & ID */
      if(orxInput_GetBindingType(orxConfig_GetString(orxCONSOLE_KZ_CONFIG_TOGGLE_KEY), &eType, &eID) != orxSTATUS_FAILURE)
      {
        /* Stores it */
        sstConsole.eToggleKeyType = eType;
        sstConsole.eToggleKeyID   = eID;
      }
    }

    /* Pops config section */
    orxConfig_PopSection();

    /* Adds event handler */
    eResult = orxEvent_AddHandler(orxEVENT_TYPE_INPUT, orxConsole_EventHandler);

    /* Success? */
    if(eResult != orxSTATUS_FAILURE)
    {
      /* Backups previous input set */
      zPreviousSet = orxInput_GetCurrentSet();

      /* Replaces input set */
      eResult = orxInput_SelectSet(orxCONSOLE_KZ_INPUT_SET);

      /* Success */
      if(eResult != orxSTATUS_FAILURE)
      {
        /* Binds console inputs */
        orxInput_Bind(orxCONSOLE_KZ_INPUT_AUTOCOMPLETE, orxINPUT_TYPE_KEYBOARD_KEY, orxCONSOLE_KE_KEY_AUTOCOMPLETE);
        orxInput_Bind(orxCONSOLE_KZ_INPUT_DELETE, orxINPUT_TYPE_KEYBOARD_KEY, orxCONSOLE_KE_KEY_DELETE);
        orxInput_Bind(orxCONSOLE_KZ_INPUT_ENTER, orxINPUT_TYPE_KEYBOARD_KEY, orxCONSOLE_KE_KEY_ENTER);
        orxInput_Bind(orxCONSOLE_KZ_INPUT_PREVIOUS, orxINPUT_TYPE_KEYBOARD_KEY, orxCONSOLE_KE_KEY_PREVIOUS);
        orxInput_Bind(orxCONSOLE_KZ_INPUT_NEXT, orxINPUT_TYPE_KEYBOARD_KEY, orxCONSOLE_KE_KEY_NEXT);

        /* Restores previous set */
        orxInput_SelectSet(zPreviousSet);

        /* Registers update callback */
        eResult = orxClock_Register(orxClock_FindFirst(orx2F(-1.0f), orxCLOCK_TYPE_CORE), orxConsole_Update, orxNULL, orxMODULE_ID_CONSOLE, orxCLOCK_PRIORITY_HIGH);

        /* Success? */
        if(eResult != orxSTATUS_FAILURE)
        {
          /* Inits Flags */
          sstConsole.u32Flags = orxCONSOLE_KU32_STATIC_FLAG_READY;

          /* Sets default font */
          orxConsole_SetFont(orxFont_GetDefaultFont());
        }
        else
        {
          /* Remove event handler */
          orxEvent_RemoveHandler(orxEVENT_TYPE_INPUT, orxConsole_EventHandler);

          /* Logs message */
          orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Couldn't register console update callback.");
        }
      }
      else
      {
        /* Remove event handler */
        orxEvent_RemoveHandler(orxEVENT_TYPE_INPUT, orxConsole_EventHandler);

        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Couldn't initialize console inputs.");
      }
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Couldn't register console's event handler.");
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

    /* Removes font */
    orxConsole_SetFont(orxNULL);

    /* Unregisters update callback */
    orxClock_Unregister(orxClock_FindFirst(orx2F(-1.0f), orxCLOCK_TYPE_CORE), orxConsole_Update);

    /* Remove event handler */
    orxEvent_RemoveHandler(orxEVENT_TYPE_INPUT, orxConsole_EventHandler);

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

/** Writes text to the console
 * @param[in]   _zText        Text to log
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxConsole_Write(const orxSTRING _zText)
{
  const orxCHAR  *pc;
  orxU32          u32LineLength;
  orxSTATUS       eResult = orxSTATUS_SUCCESS;

  /* Profiles */
  orxPROFILER_PUSH_MARKER("orxConsole_Write");

  /* Checks */
  orxASSERT(sstConsole.u32Flags & orxCONSOLE_KU32_STATIC_FLAG_READY);

  /* For all characters */
  for(u32LineLength = 0, pc = _zText; *pc != orxCHAR_NULL;)
  {
    orxU32 u32CharacterCodePoint, u32CharacterLength;

    /* Gets character code point */
    u32CharacterCodePoint = orxString_GetFirstCharacterCodePoint(pc, &pc);

    /* Gets its length */
    u32CharacterLength = orxString_GetUTF8CharacterLength(u32CharacterCodePoint);

    /* Not enough room left? */
    if(sstConsole.u32LogIndex + u32CharacterLength > orxCONSOLE_KU32_LOG_BUFFER_SIZE)
    {
      /* Stores log end index */
      sstConsole.u32LogEndIndex = sstConsole.u32LogIndex;

      /* Resets log index */
      sstConsole.u32LogIndex = 0;
    }

    /* Appends character to log */
    orxString_PrintUTF8Character(sstConsole.acLogBuffer + sstConsole.u32LogIndex, u32CharacterLength, u32CharacterCodePoint);

    /* Updates log index */
    sstConsole.u32LogIndex += u32CharacterLength;
  }

  /* Profiles */
  orxPROFILER_POP_MARKER();

  /* Done! */
  return eResult;
}

/** Sets the console font
 * @param[in]   _pstFont      Font to use
 * @return orxSTATUS_SUCCESS/ orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxConsole_SetFont(const orxFONT *_pstFont)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstConsole.u32Flags & orxCONSOLE_KU32_STATIC_FLAG_READY);

  /* Has a current font? */
  if(sstConsole.pstFont != orxNULL)
  {
    /* Updates its reference counter */
    orxStructure_DecreaseCounter((orxFONT *)sstConsole.pstFont);
  }

  /* Is font valid? */
  if(_pstFont != orxNULL)
  {
    /* Stores it */
    sstConsole.pstFont = _pstFont;

    /* Updates its reference counter */
    orxStructure_IncreaseCounter((orxFONT *)sstConsole.pstFont);
  }

  /* Done! */
  return eResult;
}

/** Gets the current font
 * @return Current in-use font
 */
const orxFONT *orxFASTCALL orxConsole_GetFont()
{
  const orxFONT *pstResult;

  /* Checks */
  orxASSERT(sstConsole.u32Flags & orxCONSOLE_KU32_STATIC_FLAG_READY);

  /* Updates result */
  pstResult = sstConsole.pstFont;

  /* Done! */
  return pstResult;
}
