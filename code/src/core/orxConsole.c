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
#define orxCONSOLE_KU32_LOG_BUFFER_SIZE               65536                           /**< Log buffer size */
#define orxCONSOLE_KU32_DEFAULT_LOG_LINE_LENGTH       128                             /**< Default log line length */

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
#define orxCONSOLE_KE_KEY_ENTER_ALTERNATE             orxKEYBOARD_KEY_NUMPAD_RETURN   /**< Enter alternate key */
#define orxCONSOLE_KE_KEY_PREVIOUS                    orxKEYBOARD_KEY_UP              /**< Previous key */
#define orxCONSOLE_KE_KEY_NEXT                        orxKEYBOARD_KEY_DOWN            /**< Next key */

#define orxCONSOLE_KF_DELETE_INPUT_RESET_FIRST_DELAY  orx2F(0.25f)
#define orxCONSOLE_KF_DELETE_INPUT_RESET_DELAY        orx2F(0.05f)


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Input entry
 */
typedef struct __orxCONSOLE_INPUT_ENTRY_t
{
  orxCHAR acBuffer[orxCONSOLE_KU32_INPUT_ENTRY_SIZE];
  orxU32  u32CursorIndex;

} orxCONSOLE_INPUT_ENTRY;

/** Static structure
 */
typedef struct __orxCONSOLE_STATIC_t
{
  orxCHAR                   acLogBuffer[orxCONSOLE_KU32_LOG_BUFFER_SIZE];             /**< Log buffer */
  orxCONSOLE_INPUT_ENTRY    astInputEntryList[orxCONSOLE_KU32_INPUT_ENTRY_NUMBER];    /**< Input entry number */
  orxU32                    u32LogIndex;                                              /**< Log buffer index */
  orxU32                    u32LogEndIndex;                                           /**< Log end index */
  orxU32                    u32LogLineLength;                                         /**< Log line length */
  orxU32                    u32InputIndex;                                            /**< Input index */
  orxU32                    u32HistoryIndex;                                          /**< History index */
  orxCOMMAND_VAR            stLastResult;                                             /**< Last command result */
  const orxFONT            *pstFont;                                                  /**< Font */
  const orxSTRING           zPreviousInputSet;                                        /**< Previous input set */
  const orxSTRING           zCompletedCommand;                                        /**< Last completed command */
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

/** Resets delete input
 */
static void orxFASTCALL orxConsole_ResetDeleteInput(const orxCLOCK_INFO *_pstInfo, void *_pContext)
{
  /* Is delete input still active? */
  if(orxInput_IsActive(orxCONSOLE_KZ_INPUT_DELETE) != orxFALSE)
  {
    /* Resets it */
    orxInput_SetValue(orxCONSOLE_KZ_INPUT_DELETE, orxFLOAT_0);

    /* Re-adds delete input reset timer */
    orxClock_AddGlobalTimer(orxConsole_ResetDeleteInput, orxCONSOLE_KF_DELETE_INPUT_RESET_DELAY, 1, orxNULL);
  }
}

/** Prints last result
 */
static orxINLINE orxU32 orxConsole_PrintLastResult(orxCHAR *_acBuffer, orxU32 _u32Size)
{
  orxU32 u32Result;

  /* Depending on type */
  switch(sstConsole.stLastResult.eType)
  {
    default:
    case orxCOMMAND_VAR_TYPE_STRING:
    {
      /* Updates pointer */
      u32Result = orxString_NPrint(_acBuffer, _u32Size, "%s", sstConsole.stLastResult.zValue);
    
      break;
    }
    
    case orxCOMMAND_VAR_TYPE_FLOAT:
    {
      /* Stores it */
      u32Result = orxString_NPrint(_acBuffer, _u32Size, "%g", sstConsole.stLastResult.fValue);
    
      break;
    }
    
    case orxCOMMAND_VAR_TYPE_S32:
    {
      /* Stores it */
      u32Result = orxString_NPrint(_acBuffer, _u32Size, "%d", sstConsole.stLastResult.s32Value);
    
      break;
    }
    
    case orxCOMMAND_VAR_TYPE_U32:
    {
      /* Stores it */
      u32Result = orxString_NPrint(_acBuffer, _u32Size, "%u", sstConsole.stLastResult.u32Value);
    
      break;
    }
    
    case orxCOMMAND_VAR_TYPE_S64:
    {
      /* Stores it */
      u32Result = orxString_NPrint(_acBuffer, _u32Size, "%lld", sstConsole.stLastResult.s64Value);
    
      break;
    }
    
    case orxCOMMAND_VAR_TYPE_U64:
    {
      /* Stores it */
      u32Result = orxString_NPrint(_acBuffer, _u32Size, "0x%016llX", sstConsole.stLastResult.u64Value);
    
      break;
    }
    
    case orxCOMMAND_VAR_TYPE_BOOL:
    {
      /* Stores it */
      u32Result = orxString_NPrint(_acBuffer, _u32Size, "%s", (sstConsole.stLastResult.bValue == orxFALSE) ? orxSTRING_FALSE : orxSTRING_TRUE);
    
      break;
    }
    
    case orxCOMMAND_VAR_TYPE_VECTOR:
    {
      /* Gets literal value */
      u32Result = orxString_NPrint(_acBuffer, _u32Size, "%c%g%c %g%c %g%c", orxSTRING_KC_VECTOR_START, sstConsole.stLastResult.vValue.fX, orxSTRING_KC_VECTOR_SEPARATOR, sstConsole.stLastResult.vValue.fY, orxSTRING_KC_VECTOR_SEPARATOR, sstConsole.stLastResult.vValue.fZ, orxSTRING_KC_VECTOR_END);
    
      break;
    }

    case orxCOMMAND_VAR_TYPE_NONE:
    {
      /* Ends string */
      _acBuffer[0]  = orxCHAR_NULL;
      u32Result     = 0;

      break;
    }
  }

  /* Done! */
  return u32Result;
}

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
    const orxSTRING         zKeyboardInput;
    const orxCHAR          *pc;
    orxBOOL                 bAddImplicitSpace = orxFALSE;
    orxU32                  u32HistoryIndex = orxU32_UNDEFINED;
    orxCONSOLE_INPUT_ENTRY *pstEntry;

    /* Gets current entry */
    pstEntry = &sstConsole.astInputEntryList[sstConsole.u32InputIndex];

    /* Gets keyboard char input */
    zKeyboardInput = orxKeyboard_ReadString();

    /* For all characters */
    for(pc = zKeyboardInput; *pc != orxCHAR_NULL;)
    {
      orxU32 u32CharacterCodePoint, u32CharacterLength;

      /* Gets character code point */
      u32CharacterCodePoint = orxString_GetFirstCharacterCodePoint(pc, &pc);

      /* Gets its length */
      u32CharacterLength = orxString_GetUTF8CharacterLength(u32CharacterCodePoint);

      /* Has completed command? */
      if(sstConsole.zCompletedCommand != orxNULL)
      {
        /* Validation? */
        if(u32CharacterCodePoint == ' ')
        {
          /* Updates cursor index */
          pstEntry->u32CursorIndex = orxString_GetLength(pstEntry->acBuffer);
        }
        /* Ask for unstacking? */
        else if(u32CharacterCodePoint == '<')
        {
          /* Updates cursor index */
          pstEntry->u32CursorIndex = orxString_GetLength(pstEntry->acBuffer);

          /* Enough room left? */
          if(pstEntry->u32CursorIndex + 1 < orxCONSOLE_KU32_INPUT_ENTRY_SIZE)
          {
            /* Adds implicit space */
            pstEntry->acBuffer[pstEntry->u32CursorIndex++] = ' ';

            /* Asks for extra implicit space */
            bAddImplicitSpace = orxTRUE;
          }
        }
        else
        {
          orxU32 i;

          /* Clears ends of buffer */
          for(i = 1; pstEntry->acBuffer[pstEntry->u32CursorIndex + i] != orxCHAR_NULL; pstEntry->acBuffer[pstEntry->u32CursorIndex + i++] = orxCHAR_NULL);
        }

        /* Clears last completed command */
        sstConsole.zCompletedCommand = orxNULL;
      }

      /* Enough room left? */
      if(pstEntry->u32CursorIndex + u32CharacterLength < orxCONSOLE_KU32_INPUT_ENTRY_SIZE)
      {
        /* Appends character to entry */
        orxString_PrintUTF8Character(pstEntry->acBuffer + pstEntry->u32CursorIndex, u32CharacterLength, u32CharacterCodePoint);

        /* Updates log index */
        pstEntry->u32CursorIndex += u32CharacterLength;

        /* Asked for implicit space and enough room left? */
        if((bAddImplicitSpace != orxFALSE) && (pstEntry->u32CursorIndex + 1 < orxCONSOLE_KU32_INPUT_ENTRY_SIZE))
        {
          /* Adds implicit space */
          pstEntry->acBuffer[pstEntry->u32CursorIndex++] = ' ';
        }
      }
      else
      {
        /* Stops */
        break;
      }
    }

    /* Delete? */
    if((orxInput_IsActive(orxCONSOLE_KZ_INPUT_DELETE) != orxFALSE) && (orxInput_HasNewStatus(orxCONSOLE_KZ_INPUT_DELETE) != orxFALSE))
    {
      /* Has character? */
      if((pstEntry->u32CursorIndex != 0) || (pstEntry->acBuffer[0] != orxCHAR_NULL))
      {
        orxU32  i;
        const   orxCHAR *pc, *pcLast;

        /* Gets last character */
        for(pcLast = pstEntry->acBuffer, orxString_GetFirstCharacterCodePoint(pcLast, &pc);
            (*pc != orxCHAR_NULL) && ((orxU32)(pc - pstEntry->acBuffer) < pstEntry->u32CursorIndex);
            pcLast = pc, orxString_GetFirstCharacterCodePoint(pcLast, &pc));

        /* Updates cursor index */
        pstEntry->u32CursorIndex = pcLast - pstEntry->acBuffer;

        /* Ends entry string */
        pstEntry->acBuffer[pstEntry->u32CursorIndex] = orxCHAR_NULL;

        /* Clears ends of buffer */
        for(i = 1; pstEntry->acBuffer[pstEntry->u32CursorIndex + i] != orxCHAR_NULL; pstEntry->acBuffer[pstEntry->u32CursorIndex + i++] = orxCHAR_NULL);

        /* Clears last completed command */
        sstConsole.zCompletedCommand = orxNULL;
      }

      /* Adds delete input reset timer */
      orxClock_AddGlobalTimer(orxConsole_ResetDeleteInput, orxCONSOLE_KF_DELETE_INPUT_RESET_FIRST_DELAY, 1, orxNULL);
    }

    /* Previous history? */
    if((orxInput_IsActive(orxCONSOLE_KZ_INPUT_PREVIOUS) != orxFALSE) && (orxInput_HasNewStatus(orxCONSOLE_KZ_INPUT_PREVIOUS) != orxFALSE))
    {
      /* Gets previous index */
      u32HistoryIndex = (sstConsole.u32HistoryIndex != 0) ? sstConsole.u32HistoryIndex - 1 : orxCONSOLE_KU32_INPUT_ENTRY_NUMBER - 1;
    }
    else if((orxInput_IsActive(orxCONSOLE_KZ_INPUT_NEXT) != orxFALSE) && (orxInput_HasNewStatus(orxCONSOLE_KZ_INPUT_NEXT) != orxFALSE))
    {
      /* Gets next index */
      u32HistoryIndex = (sstConsole.u32HistoryIndex == orxCONSOLE_KU32_INPUT_ENTRY_NUMBER - 1) ? 0 : sstConsole.u32HistoryIndex + 1;
    }

    /* Should copy history entry? */
    if(u32HistoryIndex != orxU32_UNDEFINED)
    {
      /* End of history? */
      if((sstConsole.astInputEntryList[u32HistoryIndex].u32CursorIndex == 0) || (u32HistoryIndex == sstConsole.u32InputIndex))
      {
        /* Uses current history entry */
        u32HistoryIndex = sstConsole.u32HistoryIndex;
      }

      /* Valid? */
      if((sstConsole.astInputEntryList[u32HistoryIndex].u32CursorIndex != 0) && (u32HistoryIndex != sstConsole.u32InputIndex))
      {
        orxU32                  i;
        orxCONSOLE_INPUT_ENTRY *pstHistoryEntry;

        /* Updates history index */
        sstConsole.u32HistoryIndex = u32HistoryIndex;

        /* Gets its entry */
        pstHistoryEntry = &sstConsole.astInputEntryList[sstConsole.u32HistoryIndex];

        /* Copies its content */
        orxMemory_Copy(pstEntry->acBuffer, pstHistoryEntry->acBuffer, pstHistoryEntry->u32CursorIndex + 1);
        pstEntry->u32CursorIndex = pstHistoryEntry->u32CursorIndex;

        /* Clears ends of buffer */
        for(i = 1; pstEntry->acBuffer[pstEntry->u32CursorIndex + i] != orxCHAR_NULL; pstEntry->acBuffer[pstEntry->u32CursorIndex + i++] = orxCHAR_NULL);

        /* Clears last completed command */
        sstConsole.zCompletedCommand = orxNULL;
      }
    }

    /* Autocomplete? */
    if((orxInput_IsActive(orxCONSOLE_KZ_INPUT_AUTOCOMPLETE) != orxFALSE) && (orxInput_HasNewStatus(orxCONSOLE_KZ_INPUT_AUTOCOMPLETE) != orxFALSE))
    {
      const orxCHAR  *pcStart = pstEntry->acBuffer;
      orxBOOL         bPrintLastResult = orxFALSE;

      /* First character? */
      if((pstEntry->u32CursorIndex == 0) && (pstEntry->acBuffer[0] == orxCHAR_NULL))
      {
        /* Prints last result */
        bPrintLastResult = orxTRUE;
      }
      else
      {
        const orxCHAR  *pc, *pcLast;
        orxU32          u32CharacterCodePoint;

        /* Gets last character */
        for(pcStart = pcLast = pstEntry->acBuffer, u32CharacterCodePoint = orxString_GetFirstCharacterCodePoint(pcLast, &pc);
            *pc != orxCHAR_NULL;
            pcLast = pc, u32CharacterCodePoint = orxString_GetFirstCharacterCodePoint(pcLast, &pc))
        {
          /* Space, tab or stack marker? */
          if((u32CharacterCodePoint == ' ')
          || (u32CharacterCodePoint == '\t')
          || (u32CharacterCodePoint == '>'))
          {
            /* Resets start */
            pcStart = pc;
          }
        }

        /* Is a white space? */
        if((*pcLast == ' ') || (*pcLast == '\t'))
        {
          /* Prints last result */
          bPrintLastResult = orxTRUE;
        }
      }

      /* Should print last result? */
      if(bPrintLastResult != orxFALSE)
      {
        /* Prints it */
        pstEntry->u32CursorIndex += orxConsole_PrintLastResult(pstEntry->acBuffer + pstEntry->u32CursorIndex, orxCONSOLE_KU32_INPUT_ENTRY_SIZE - 1 - pstEntry->u32CursorIndex);

        /* Ends string */
        pstEntry->acBuffer[pstEntry->u32CursorIndex] = orxCHAR_NULL;

        /* Clears last completed command */
        sstConsole.zCompletedCommand = orxNULL;
      }
      else
      {
        /* Ends current string */
        pstEntry->acBuffer[pstEntry->u32CursorIndex] = orxCHAR_NULL;

        /* Gets next command */
        sstConsole.zCompletedCommand = orxCommand_GetNext(pcStart, sstConsole.zCompletedCommand);

        /* Valid? */
        if(sstConsole.zCompletedCommand != orxNULL)
        {
          orxS32 s32Offset, i;

          /* Prints it */
          s32Offset = orxString_NPrint((orxCHAR *)pcStart, orxCONSOLE_KU32_INPUT_ENTRY_SIZE - 1 - (pcStart - pstEntry->acBuffer), "%s", sstConsole.zCompletedCommand);

          /* Ends string */
          pstEntry->acBuffer[(pcStart - pstEntry->acBuffer) + s32Offset] = orxCHAR_NULL;

          /* Clears ends of buffer */
          for(i = 1; pstEntry->acBuffer[(pcStart - pstEntry->acBuffer) + s32Offset + i] != orxCHAR_NULL; pstEntry->acBuffer[(pcStart - pstEntry->acBuffer) + s32Offset + i++] = orxCHAR_NULL);
        }
        else
        {
          orxU32 i;

          /* Clears ends of buffer */
          for(i = 1; pstEntry->acBuffer[pstEntry->u32CursorIndex + i] != orxCHAR_NULL; pstEntry->acBuffer[pstEntry->u32CursorIndex + i++] = orxCHAR_NULL);
        }
      }
    }

    /* Enter command? */
    if((orxInput_IsActive(orxCONSOLE_KZ_INPUT_ENTER) != orxFALSE) && (orxInput_HasNewStatus(orxCONSOLE_KZ_INPUT_ENTER) != orxFALSE))
    {
      /* Not empty? */
      if(pstEntry->u32CursorIndex != 0)
      {
        /* Has last autocompleted command? */
        if(sstConsole.zCompletedCommand != orxNULL)
        {
          /* Updates cursor index */
          pstEntry->u32CursorIndex = orxString_GetLength(pstEntry->acBuffer);

          /* Clears last completed command */
          sstConsole.zCompletedCommand = orxNULL;
        }

        /* Evaluates it */
        if(orxCommand_Evaluate(pstEntry->acBuffer, &(sstConsole.stLastResult)) != orxNULL)
        {
          orxCHAR acValue[64];

          /* Inits value */        
          acValue[0]  = ':';
          acValue[1]  = ' ';
          acValue[63] = orxCHAR_NULL;

          /* Prints result */
          orxConsole_PrintLastResult(acValue + 2, 61);

          /* Logs it */
          orxConsole_Log(acValue);
        }
        else
        {
          /* Logs failure */
           orxConsole_Log(": Invalid command!");
        }

        /* Udates input index */
        sstConsole.u32InputIndex = (sstConsole.u32InputIndex == orxCONSOLE_KU32_INPUT_ENTRY_NUMBER - 1) ? 0 : sstConsole.u32InputIndex + 1;

        /* Updates history index */
        sstConsole.u32HistoryIndex = sstConsole.u32InputIndex;
      }
    }
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

    /* Clears keyboard buffer */
    orxKeyboard_ClearBuffer();
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

    /* Stores default log line length */
    sstConsole.u32LogLineLength = orxCONSOLE_KU32_DEFAULT_LOG_LINE_LENGTH;

    /* Clears last result */
    sstConsole.stLastResult.eType = orxCOMMAND_VAR_TYPE_NONE;

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
        orxInput_Bind(orxCONSOLE_KZ_INPUT_ENTER, orxINPUT_TYPE_KEYBOARD_KEY, orxCONSOLE_KE_KEY_ENTER_ALTERNATE);
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

/** Logs to the console
 * @param[in]   _zText        Text to log
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxConsole_Log(const orxSTRING _zText)
{
  const orxCHAR  *pc;
  orxU32          u32LineLength, u32TextLength;
  orxSTATUS       eResult = orxSTATUS_SUCCESS;

  /* Profiles */
  orxPROFILER_PUSH_MARKER("orxConsole_Log");

  /* Checks */
  orxASSERT(sstConsole.u32Flags & orxCONSOLE_KU32_STATIC_FLAG_READY);

  /* Gets text length */
  u32TextLength = orxString_GetLength(_zText);

  /* End of buffer? */
  if(sstConsole.u32LogIndex + u32TextLength + (u32TextLength / sstConsole.u32LogLineLength) + 1 > orxCONSOLE_KU32_LOG_BUFFER_SIZE)
  {
    /* Stores log end index */
    sstConsole.u32LogEndIndex = sstConsole.u32LogIndex;

    /* Resets log index */
    sstConsole.u32LogIndex = 0;
  }

  /* For all characters */
  for(u32LineLength = 0, pc = _zText; *pc != orxCHAR_NULL;)
  {
    orxU32 u32CharacterCodePoint, u32CharacterLength;

    /* Gets character code point */
    u32CharacterCodePoint = orxString_GetFirstCharacterCodePoint(pc, &pc);

    /* Gets its length */
    u32CharacterLength = orxString_GetUTF8CharacterLength(u32CharacterCodePoint);

    /* EOL? */
    if(u32CharacterCodePoint == (orxU32)orxCHAR_EOL)
    {
      /* Ends string */
      sstConsole.acLogBuffer[sstConsole.u32LogIndex++] = orxCHAR_NULL;

      /* Resets line length */
      u32LineLength = 0;
    }
    else
    {
      /* Appends character to log */
      orxString_PrintUTF8Character(sstConsole.acLogBuffer + sstConsole.u32LogIndex, u32CharacterLength, u32CharacterCodePoint);

      /* Updates log index */
      sstConsole.u32LogIndex += u32CharacterLength;

      /* End of line? */
      if(u32LineLength + 1 >= sstConsole.u32LogLineLength)
      {
        /* Ends string */
        sstConsole.acLogBuffer[sstConsole.u32LogIndex++] = orxCHAR_NULL;

        /* Updates line length */
        u32LineLength = 0;
      }
      else
      {
        /* Updates line length */
        u32LineLength++;
      }
    }
  }

  /* Need EOL? */
  if(u32LineLength != 0)
  {
    /* Ends string */
    sstConsole.acLogBuffer[sstConsole.u32LogIndex++] = orxCHAR_NULL;
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

/** Gets the console font
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

/** Sets the console log line length
 * @param[in]   _u32Length    Line length to use
 * @return orxSTATUS_SUCCESS/ orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxConsole_SetLogLineLength(orxU32 _u32LineLength)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(sstConsole.u32Flags & orxCONSOLE_KU32_STATIC_FLAG_READY);

  /* Valid? */
  if(_u32LineLength > 0)
  {
    /* Stores it */
    sstConsole.u32LogLineLength = _u32LineLength;

    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }
  else
  {
    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Gets the console log line length
 * @return Console log line length
 */
orxU32 orxFASTCALL orxConsole_GetLogLineLength()
{
  orxU32 u32Result;

  /* Checks */
  orxASSERT(sstConsole.u32Flags & orxCONSOLE_KU32_STATIC_FLAG_READY);

  /* Updates result */
  u32Result = sstConsole.u32LogLineLength;

  /* Done! */
  return u32Result;
}

/** Gets log line from the end (trail)
 * @param[in]   _u32TrailLineIndex Index of the line starting from end
 * @return orxTRING / orxSTRING_EMPTY
 */
const orxSTRING orxFASTCALL orxConsole_GetTrailLogLine(orxU32 _u32TrailLineIndex)
{
  const orxSTRING zResult = orxSTRING_EMPTY;

  /* Checks */
  orxASSERT(sstConsole.u32Flags & orxCONSOLE_KU32_STATIC_FLAG_READY);

  //! TODO

  /* Done! */
  return zResult;
}

/** Gets input text
 * @param[out]  _pu32CursorIndex Index (ie. character position) of the cursor (any character past it has not been validated)
 * @return orxTRING / orxSTRING_EMPTY
 */
const orxSTRING orxFASTCALL orxConsole_GetInput(orxU32 *_pu32CursorIndex)
{
  const orxSTRING zResult;

  /* Asked for base length? */
  if(_pu32CursorIndex != orxNULL)
  {
    /* Fills it */
    *_pu32CursorIndex = sstConsole.astInputEntryList[sstConsole.u32InputIndex].u32CursorIndex;
  }

  /* Updates result */
  zResult = sstConsole.astInputEntryList[sstConsole.u32InputIndex].acBuffer;

  /* Done! */
  return zResult;
}
