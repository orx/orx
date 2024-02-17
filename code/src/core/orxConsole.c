/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008- Orx-Project
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
#include "render/orxRender.h"
#include "utils/orxString.h"


/** Module flags
 */
#define orxCONSOLE_KU32_STATIC_FLAG_NONE              0x00000000                      /**< No flags */

#define orxCONSOLE_KU32_STATIC_FLAG_READY             0x00000001                      /**< Ready flag */
#define orxCONSOLE_KU32_STATIC_FLAG_ENABLED           0x00000002                      /**< Enabled flag */
#define orxCONSOLE_KU32_STATIC_FLAG_INSERT_MODE       0x00000004                      /**< Insert mode flag */
#define orxCONSOLE_KU32_STATIC_FLAG_ECHO              0x00000008                      /**< Echo flag */

#define orxCONSOLE_KU32_STATIC_MASK_ALL               0xFFFFFFFF                      /**< All mask */


/** Misc
 */
#define orxCONSOLE_KU32_LOG_BUFFER_SIZE               65536                           /**< Log buffer size */
#define orxCONSOLE_KU32_DEFAULT_LOG_LINE_LENGTH       128                             /**< Default log line length */

#define orxCONSOLE_KU32_INPUT_ENTRY_SIZE              256                             /**< Input entry size */
#define orxCONSOLE_KU32_INPUT_ENTRY_NUMBER            64                              /**< Input entry number */

#define orxCONSOLE_KZ_CONFIG_SECTION                  "Console"                       /**< Config section name */
#define orxCONSOLE_KZ_CONFIG_TOGGLE_KEY               "ToggleKey"                     /**< Toggle key */
#define orxCONSOLE_KZ_CONFIG_INPUT_HISTORY_LIST       "InputHistoryList"              /**< Input history list */
#define orxCONSOLE_KZ_CONFIG_SCROLL_SIZE              "ScrollSize"                    /**< Scroll size */

#define orxCONSOLE_KE_DEFAULT_KEY_TOGGLE              orxKEYBOARD_KEY_BACKQUOTE       /**< Default toggle key */

#define orxCONSOLE_KE_KEY_AUTOCOMPLETE                orxKEYBOARD_KEY_TAB             /**< Autocomplete key */
#define orxCONSOLE_KE_KEY_DELETE                      orxKEYBOARD_KEY_BACKSPACE       /**< Delete key */
#define orxCONSOLE_KE_KEY_DELETE_AFTER                orxKEYBOARD_KEY_DELETE          /**< Delete after key */
#define orxCONSOLE_KE_KEY_TOGGLE_MODE                 orxKEYBOARD_KEY_INSERT          /**< Toggle mode key */
#define orxCONSOLE_KE_KEY_ENTER                       orxKEYBOARD_KEY_ENTER           /**< Enter key */
#define orxCONSOLE_KE_KEY_ENTER_ALTERNATE             orxKEYBOARD_KEY_NUMPAD_ENTER    /**< Enter alternate key */
#define orxCONSOLE_KE_KEY_PREVIOUS                    orxKEYBOARD_KEY_UP              /**< Previous key */
#define orxCONSOLE_KE_KEY_NEXT                        orxKEYBOARD_KEY_DOWN            /**< Next key */
#define orxCONSOLE_KE_KEY_LEFT                        orxKEYBOARD_KEY_LEFT            /**< Left key */
#define orxCONSOLE_KE_KEY_RIGHT                       orxKEYBOARD_KEY_RIGHT           /**< Right key */
#define orxCONSOLE_KE_KEY_START                       orxKEYBOARD_KEY_HOME            /**< Start key */
#define orxCONSOLE_KE_KEY_END                         orxKEYBOARD_KEY_END             /**< End key */
#ifdef __orxMAC__
#define orxCONSOLE_KE_KEY_CONTROL                     orxKEYBOARD_KEY_LSYSTEM         /**< Control key */
#else /* __orxMAC__ */
#define orxCONSOLE_KE_KEY_CONTROL                     orxKEYBOARD_KEY_LCTRL           /**< Control key */
#endif /* __orxMAC__ */
#define orxCONSOLE_KE_KEY_PASTE                       orxKEYBOARD_KEY_V               /**< Paste key */
#define orxCONSOLE_KE_KEY_SCROLL_UP                   orxKEYBOARD_KEY_PAGE_UP         /**< Scroll up key */
#define orxCONSOLE_KE_KEY_SCROLL_DOWN                 orxKEYBOARD_KEY_PAGE_DOWN       /**< Scroll down key */
#define orxCONSOLE_KE_BUTTON_SCROLL_UP                orxMOUSE_BUTTON_WHEEL_UP        /**< Scroll up button */
#define orxCONSOLE_KE_BUTTON_SCROLL_DOWN              orxMOUSE_BUTTON_WHEEL_DOWN      /**< Scroll down button */

#define orxCONSOLE_KU32_SCROLL_SIZE                   3

#define orxCONSOLE_KF_INPUT_RESET_FIRST_DELAY         orx2F(0.35f)
#define orxCONSOLE_KF_INPUT_RESET_DELAY               orx2F(0.05f)


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
  orxU32                    u32LogLineOffset;                                         /**< Log line offset */
  orxU32                    u32InputIndex;                                            /**< Input index */
  orxU32                    u32HistoryIndex;                                          /**< History index */
  orxCOMMAND_VAR            stLastResult;                                             /**< Last command result */
  const orxFONT            *pstFont;                                                  /**< Font */
  const orxSTRING           zPreviousInputSet;                                        /**< Previous input set */
  const orxSTRING           zCompletedCommand;                                        /**< Last completed command */
  orxINPUT_TYPE             eToggleKeyType;                                           /**< Toggle key type */
  orxENUM                   eToggleKeyID;                                             /**< Toggle key ID */
  orxINPUT_MODE             eToggleKeyMode;                                           /**< Toggle key mode */
  orxS32                    s32RepeatCounter;                                         /**< Repeat counter */
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

/** Semi-private, internal-use only forward declarations
 */
orxVECTOR *orxFASTCALL orxConfig_ToVector(const orxSTRING _zValue, orxCOLORSPACE _eColorSpace, orxVECTOR *_pvVector);

/** Resets input callback
 */
static void orxFASTCALL orxConsole_ResetInput(const orxCLOCK_INFO *_pstInfo, void *_pContext)
{
  orxSTRING zInput;

  /* Gets input */
  zInput = (orxSTRING)_pContext;

  /* Is input still active? */
  if(orxInput_IsActive(zInput) != orxFALSE)
  {
    /* Resets it */
    orxInput_SetValue(zInput, orxFLOAT_0);

    /* Adds timer */
    orxClock_AddGlobalTimer(orxConsole_ResetInput, orxCONSOLE_KF_INPUT_RESET_DELAY, 1, _pContext);

    /* Updates repeat counter */
    sstConsole.s32RepeatCounter++;
  }
  else
  {
    /* Updates repeat counter */
    sstConsole.s32RepeatCounter = 0;
  }

  /* Done! */
  return;
}

/** Repeats input
 */
static void orxFASTCALL orxConsole_RepeatInput(const orxSTRING _zInput)
{
  /* New repeat? */
  if(sstConsole.s32RepeatCounter <= 1)
  {
    /* Removes existing timer */
    orxClock_RemoveGlobalTimer(orxConsole_ResetInput, -orxFLOAT_1, (void *)_zInput);

    /* Adds timer */
    orxClock_AddGlobalTimer(orxConsole_ResetInput, orxCONSOLE_KF_INPUT_RESET_FIRST_DELAY, 1, (void *)_zInput);
  }

  /* Updates repeat counter */
  sstConsole.s32RepeatCounter = 1;
}

/** Origin save callback
 */
orxBOOL orxFASTCALL orxConsole_HistorySaveCallback(const orxSTRING _zSectionName, const orxSTRING _zKeyName, const orxSTRING _zFileName, orxBOOL _bUseEncryption)
{
  orxBOOL bResult;

  /* Updates result */
  bResult = ((orxString_Compare(_zSectionName, orxCONSOLE_KZ_CONFIG_SECTION) == 0) && ((_zKeyName == orxNULL) || (orxString_Compare(_zKeyName, orxCONSOLE_KZ_CONFIG_INPUT_HISTORY_LIST) == 0))) ? orxTRUE : orxFALSE;

  /* Done! */
  return bResult;
}

/** Saves input history
 */
static void orxFASTCALL orxConsole_SaveHistory()
{
  orxU32 i, u32Count = 0;
  const orxSTRING azHistoryList[orxCONSOLE_KU32_INPUT_ENTRY_NUMBER];

  /* Did we cycle? */
  if((sstConsole.u32InputIndex != orxCONSOLE_KU32_INPUT_ENTRY_NUMBER - 1) && (sstConsole.astInputEntryList[sstConsole.u32InputIndex + 1].u32CursorIndex != 0))
  {
    /* For all old entries */
    for(i = (sstConsole.u32InputIndex + 1) % orxCONSOLE_KU32_INPUT_ENTRY_NUMBER;
        i < orxCONSOLE_KU32_INPUT_ENTRY_NUMBER;
        i++)
    {
      /* Stores it */
      azHistoryList[u32Count++] = sstConsole.astInputEntryList[i].acBuffer;
    }
  }

  /* For all recent entries */
  for(i = 0; i < sstConsole.u32InputIndex; i++)
  {
    /* Stores it */
    azHistoryList[u32Count++] = sstConsole.astInputEntryList[i].acBuffer;
  }

  /* Has entries? */
  if(u32Count > 0)
  {
    orxCHAR acBuffer[256];

    /* Pushes config section */
    orxConfig_PushSection(orxCONSOLE_KZ_CONFIG_SECTION);

    /* Stores list */
    orxConfig_SetListString(orxCONSOLE_KZ_CONFIG_INPUT_HISTORY_LIST, azHistoryList, u32Count);

    /* Pops config section */
    orxConfig_PopSection();

    /* Gets file name */
    orxString_NPrint(acBuffer, sizeof(acBuffer), "%.*s%s", orxString_GetLength(orxConfig_GetMainFileName()) - 3, orxConfig_GetMainFileName(), orxCONSOLE_KZ_CONFIG_HISTORY_FILE_EXTENSION);

    /* Saves it */
    orxConfig_Save(acBuffer, orxFALSE, orxConsole_HistorySaveCallback);
  }

  /* Done! */
  return;
}

/** Loads input history
 */
static void orxFASTCALL orxConsole_LoadHistory()
{
  orxCHAR acBuffer[256];

  /* Gets file name */
  orxString_NPrint(acBuffer, sizeof(acBuffer), "%.*s%s", orxString_GetLength(orxConfig_GetMainFileName()) - 3, orxConfig_GetMainFileName(), orxCONSOLE_KZ_CONFIG_HISTORY_FILE_EXTENSION);

  /* Loads it */
  orxConfig_Load(acBuffer);

  /* Pushes config section */
  orxConfig_PushSection(orxCONSOLE_KZ_CONFIG_SECTION);

  /* Has saved history */
  if(orxConfig_HasValue(orxCONSOLE_KZ_CONFIG_INPUT_HISTORY_LIST) != orxFALSE)
  {
    orxU32 i, u32Count;

    /* For all history entries */
    for(i = 0, u32Count = orxMIN(orxConfig_GetListCount(orxCONSOLE_KZ_CONFIG_INPUT_HISTORY_LIST), orxCONSOLE_KU32_INPUT_ENTRY_NUMBER); i < u32Count; i++)
    {
      orxCONSOLE_INPUT_ENTRY *pstEntry;

      /* Gets corresponding entry */
      pstEntry = &(sstConsole.astInputEntryList[sstConsole.u32InputIndex++]);

      /* Updates it */
      orxString_NCopy(pstEntry->acBuffer, orxConfig_GetListString(orxCONSOLE_KZ_CONFIG_INPUT_HISTORY_LIST, i), sizeof(pstEntry->acBuffer) - 1);
      pstEntry->acBuffer[sizeof(pstEntry->acBuffer) - 1] = orxCHAR_NULL;
      pstEntry->u32CursorIndex = orxString_GetLength(pstEntry->acBuffer);
    }

    /* Updates indices */
    sstConsole.u32InputIndex = sstConsole.u32HistoryIndex = u32Count % orxCONSOLE_KU32_INPUT_ENTRY_NUMBER;
  }

  /* Pops config section */
  orxConfig_PopSection();

  /* Done! */
  return;
}

/** Prints last result
 */
static orxINLINE orxU32 orxConsole_PrintLastResult(orxCHAR *_acBuffer, orxU32 _u32Size)
{
  orxU32 u32Result;

  /* Prints it */
  u32Result = orxCommand_PrintVar(_acBuffer, _u32Size, &(sstConsole.stLastResult));

  /* Done! */
  return u32Result;
}

/** Update callback
 */
static void orxFASTCALL orxConsole_Update(const orxCLOCK_INFO *_pstClockInfo, void *_pContext)
{
  /* Profiles */
  orxPROFILER_PUSH_MARKER("orxConsole_Update");

  /* Is enabled? */
  if(orxFLAG_TEST(sstConsole.u32Flags, orxCONSOLE_KU32_STATIC_FLAG_ENABLED))
  {
    const orxSTRING         zKeyboardInput = orxSTRING_EMPTY;
    const orxCHAR          *pc;
    orxBOOL                 bAddImplicitSpace = orxFALSE;
    orxU32                  u32HistoryIndex = orxU32_UNDEFINED;
    orxCONSOLE_INPUT_ENTRY *pstEntry;

    /* Gets current entry */
    pstEntry = &sstConsole.astInputEntryList[sstConsole.u32InputIndex];

    /* Paste? */
    if(orxInput_HasBeenActivated(orxCONSOLE_KZ_INPUT_PASTE) != orxFALSE)
    {
      const orxSTRING zClipboard;
      orxU32          u32Length;

      /* Gets clipboard content */
      zClipboard = orxSystem_GetClipboard();

      /* Gets its length */
      u32Length = orxString_GetLength(zClipboard);

      /* Insert mode? */
      if(orxFLAG_TEST(sstConsole.u32Flags, orxCONSOLE_KU32_STATIC_FLAG_INSERT_MODE))
      {
        /* Enough room left? */
        if(pstEntry->acBuffer[orxCONSOLE_KU32_INPUT_ENTRY_SIZE - 1 - u32Length] == orxCHAR_NULL)
        {
          orxCHAR *pc;

          /* Shifts all further characters to the right */
          for(pc = pstEntry->acBuffer + orxCONSOLE_KU32_INPUT_ENTRY_SIZE - 1;
              pc >= pstEntry->acBuffer + pstEntry->u32CursorIndex + u32Length;
              pc--)
          {
            *pc = *(pc - u32Length);
          }

          /* Appends clipboard content */
          orxMemory_Copy(pstEntry->acBuffer + pstEntry->u32CursorIndex, zClipboard, u32Length);

          /* Updates cursor */
          pstEntry->u32CursorIndex += u32Length;
        }
      }
      else
      {
        /* Enough room left? */
        if(pstEntry->u32CursorIndex + u32Length < orxCONSOLE_KU32_INPUT_ENTRY_SIZE)
        {
          /* Appends clipboard content */
          orxMemory_Copy(pstEntry->acBuffer + pstEntry->u32CursorIndex, zClipboard, u32Length);

          /* Updates cursor */
          pstEntry->u32CursorIndex += u32Length;
        }
      }

      /* Clears keyboard buffer */
      orxKeyboard_ClearBuffer();
    }
    else
    {
      /* Gets keyboard char input */
      zKeyboardInput = orxKeyboard_ReadString();
    }

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
        /* Ask for unstacking, block or GUID? */
        else if((u32CharacterCodePoint == orxCOMMAND_KC_POP_MARKER)
             || (u32CharacterCodePoint == orxCOMMAND_KC_GUID_MARKER)
             || (u32CharacterCodePoint == orxCOMMAND_KC_BLOCK_MARKER))
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
          for(i = 0; pstEntry->acBuffer[pstEntry->u32CursorIndex + i] != orxCHAR_NULL; pstEntry->acBuffer[pstEntry->u32CursorIndex + i++] = orxCHAR_NULL);
        }

        /* Clears last completed command */
        sstConsole.zCompletedCommand = orxNULL;
      }

      /* Insert mode? */
      if(orxFLAG_TEST(sstConsole.u32Flags, orxCONSOLE_KU32_STATIC_FLAG_INSERT_MODE))
      {
        /* Enough room left? */
        if(pstEntry->acBuffer[orxCONSOLE_KU32_INPUT_ENTRY_SIZE - 1 - u32CharacterLength] == orxCHAR_NULL)
        {
          orxU32  i;
          orxCHAR cNext, cCurrent;

          /* Shifts all further characters one step to the right */
          for(i = 0, cCurrent = pstEntry->acBuffer[pstEntry->u32CursorIndex];
              cCurrent != orxCHAR_NULL;
              cNext = pstEntry->acBuffer[pstEntry->u32CursorIndex + i + u32CharacterLength], pstEntry->acBuffer[pstEntry->u32CursorIndex + i + u32CharacterLength] = cCurrent, cCurrent = cNext, i++);
          pstEntry->acBuffer[pstEntry->u32CursorIndex + i + u32CharacterLength] = cCurrent;

          /* Inserts new character */
          orxString_PrintUTF8Character(pstEntry->acBuffer + pstEntry->u32CursorIndex, u32CharacterLength, u32CharacterCodePoint);

          /* Updates log index */
          pstEntry->u32CursorIndex += u32CharacterLength;
        }
        else
        {
          /* Stops */
          break;
        }
      }
      else
      {
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
    }

    /* Delete? */
    if(orxInput_HasBeenActivated(orxCONSOLE_KZ_INPUT_DELETE) != orxFALSE)
    {
      /* Has character? */
      if((pstEntry->u32CursorIndex != 0) || (pstEntry->acBuffer[0] != orxCHAR_NULL))
      {
        orxU32         u32Offset, i;
        const orxCHAR *pc, *pcLast;

        /* Gets last character */
        for(pcLast = pstEntry->acBuffer, orxString_GetFirstCharacterCodePoint(pcLast, &pc);
            (*pc != orxCHAR_NULL) && ((orxU32)(pc - pstEntry->acBuffer) < pstEntry->u32CursorIndex);
            pcLast = pc, orxString_GetFirstCharacterCodePoint(pcLast, &pc));

        /* Gets offset */
        u32Offset = pstEntry->u32CursorIndex - (orxU32)(pcLast - pstEntry->acBuffer);

        /* Updates cursor index */
        pstEntry->u32CursorIndex -= u32Offset;

        /* Has a completed command? */
        if(sstConsole.zCompletedCommand != orxNULL)
        {
          /* Updates offset to bypass end of command */
          for(i = 0; pstEntry->acBuffer[pstEntry->u32CursorIndex + i] != orxCHAR_NULL; u32Offset++, i++);

          /* Clears last completed command */
          sstConsole.zCompletedCommand = orxNULL;
        }

        /* Shifts all further characters to the left */
        for(i = 0; pstEntry->acBuffer[pstEntry->u32CursorIndex + i] != orxCHAR_NULL; pstEntry->acBuffer[pstEntry->u32CursorIndex + i] = pstEntry->acBuffer[pstEntry->u32CursorIndex + i + u32Offset], i++);
      }

      /* Sets input repeat */
      orxConsole_RepeatInput(orxCONSOLE_KZ_INPUT_DELETE);
    }
    /* Delete after? */
    else if(orxInput_HasBeenActivated(orxCONSOLE_KZ_INPUT_DELETE_AFTER) != orxFALSE)
    {
      /* Has character? */
      if(pstEntry->acBuffer[pstEntry->u32CursorIndex] != orxCHAR_NULL)
      {
        orxU32 u32Offset, i;

        /* Gets offset */
        u32Offset = orxString_GetUTF8CharacterLength(orxString_GetFirstCharacterCodePoint(&(pstEntry->acBuffer[pstEntry->u32CursorIndex]), orxNULL));

        /* Has a completed command? */
        if(sstConsole.zCompletedCommand != orxNULL)
        {
          /* Updates offset to bypass end of command */
          for(i = 1; pstEntry->acBuffer[pstEntry->u32CursorIndex + i] != orxCHAR_NULL; u32Offset++, i++);

          /* Clears last completed command */
          sstConsole.zCompletedCommand = orxNULL;
        }

        /* Shifts all further characters to the left */
        for(i = 0; pstEntry->acBuffer[pstEntry->u32CursorIndex + i] != orxCHAR_NULL; pstEntry->acBuffer[pstEntry->u32CursorIndex + i] = pstEntry->acBuffer[pstEntry->u32CursorIndex + i + u32Offset], i++);
      }

      /* Sets input repeat */
      orxConsole_RepeatInput(orxCONSOLE_KZ_INPUT_DELETE_AFTER);
    }

    /* Previous history? */
    if(orxInput_HasBeenActivated(orxCONSOLE_KZ_INPUT_PREVIOUS) != orxFALSE)
    {
      /* Gets previous index */
      u32HistoryIndex = (sstConsole.u32HistoryIndex != 0) ? sstConsole.u32HistoryIndex - 1 : orxCONSOLE_KU32_INPUT_ENTRY_NUMBER - 1;

      /* Sets input repeat */
      orxConsole_RepeatInput(orxCONSOLE_KZ_INPUT_PREVIOUS);
    }
    /* Next history? */
    else if(orxInput_HasBeenActivated(orxCONSOLE_KZ_INPUT_NEXT) != orxFALSE)
    {
      /* Not already at end? */
      if(sstConsole.u32HistoryIndex != sstConsole.u32InputIndex)
      {
        /* Gets next index */
        u32HistoryIndex = (sstConsole.u32HistoryIndex == orxCONSOLE_KU32_INPUT_ENTRY_NUMBER - 1) ? 0 : sstConsole.u32HistoryIndex + 1;

        /* Sets input repeat */
        orxConsole_RepeatInput(orxCONSOLE_KZ_INPUT_NEXT);
      }
    }

    /* Should copy history entry? */
    if(u32HistoryIndex != orxU32_UNDEFINED)
    {
      /* Reached boundaries of history? */
      if(sstConsole.astInputEntryList[u32HistoryIndex].u32CursorIndex == 0)
      {
        /* Uses current history entry */
        u32HistoryIndex = sstConsole.u32HistoryIndex;
      }

      /* Back to input entry? */
      if(u32HistoryIndex == sstConsole.u32InputIndex)
      {
        /* Clears input */
        orxMemory_Zero(pstEntry->acBuffer, pstEntry->u32CursorIndex * sizeof(orxCHAR));
        pstEntry->u32CursorIndex  = 0;

        /* Resets history index */
        sstConsole.u32HistoryIndex = sstConsole.u32InputIndex;
      }
      /* Valid? */
      else if(sstConsole.astInputEntryList[u32HistoryIndex].u32CursorIndex != 0)
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

    /* Auto-complete? */
    if(orxInput_HasBeenActivated(orxCONSOLE_KZ_INPUT_AUTOCOMPLETE) != orxFALSE)
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
            (*pc != orxCHAR_NULL) && ((orxU32)(pc - pstEntry->acBuffer) < pstEntry->u32CursorIndex);
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
        orxU32 u32PrefixLength;

        /* Ends current string */
        pstEntry->acBuffer[pstEntry->u32CursorIndex] = orxCHAR_NULL;

        /* Gets next command */
        sstConsole.zCompletedCommand = orxCommand_GetNext(pcStart, sstConsole.zCompletedCommand, &u32PrefixLength);

        /* Valid? */
        if(sstConsole.zCompletedCommand != orxNULL)
        {
          orxS32 s32Offset, i;

          /* Prints it */
          s32Offset = orxString_NPrint((orxCHAR *)pcStart, orxCONSOLE_KU32_INPUT_ENTRY_SIZE - (orxU32)(pcStart - pstEntry->acBuffer), "%s", sstConsole.zCompletedCommand);

          /* Ends string */
          pstEntry->acBuffer[(pcStart - pstEntry->acBuffer) + s32Offset] = orxCHAR_NULL;

          /* Clears ends of buffer */
          for(i = 1; pstEntry->acBuffer[(pcStart - pstEntry->acBuffer) + s32Offset + i] != orxCHAR_NULL; pstEntry->acBuffer[(pcStart - pstEntry->acBuffer) + s32Offset + i++] = orxCHAR_NULL);

          /* Partial prefix? */
          if(u32PrefixLength <= (orxU32)s32Offset)
          {
            /* Updates cursor position */
            pstEntry->u32CursorIndex = (orxU32)(pcStart - pstEntry->acBuffer) + u32PrefixLength;
          }
        }
        else
        {
          orxU32 i;

          /* Clears ends of buffer */
          for(i = 1; pstEntry->acBuffer[pstEntry->u32CursorIndex + i] != orxCHAR_NULL; pstEntry->acBuffer[pstEntry->u32CursorIndex + i++] = orxCHAR_NULL);
        }

        /* Sets input repeat */
        orxConsole_RepeatInput(orxCONSOLE_KZ_INPUT_AUTOCOMPLETE);
      }
    }

    /* Enter command? */
    if(orxInput_HasBeenActivated(orxCONSOLE_KZ_INPUT_ENTER) != orxFALSE)
    {
      /* Not empty? */
      if(pstEntry->u32CursorIndex != 0)
      {
        /* Updates cursor index */
        pstEntry->u32CursorIndex = orxString_GetLength(pstEntry->acBuffer);

        /* Has last autocompleted command? */
        if(sstConsole.zCompletedCommand != orxNULL)
        {
          /* Clears last completed command */
          sstConsole.zCompletedCommand = orxNULL;
        }

        /* Logs input */
        orxConsole_Log(pstEntry->acBuffer);

        /* Evaluates it */
        if(orxCommand_Evaluate(pstEntry->acBuffer, &(sstConsole.stLastResult)) != orxNULL)
        {
          orxCHAR acValue[256];

          /* Inits value */
          acValue[0]  = ':';
          acValue[1]  = ' ';
          acValue[sizeof(acValue) - 1] = orxCHAR_NULL;

          /* Prints result */
          orxConsole_PrintLastResult(acValue + 2, 253);

          /* Logs it */
          orxConsole_Log(acValue);

          /* Should echo? */
          if(orxFLAG_TEST(sstConsole.u32Flags, orxCONSOLE_KU32_STATIC_FLAG_ECHO))
          {
            /* Echos command */
            orxLOG("$ %s", pstEntry->acBuffer);

            /* Echos result */
            orxLOG("%s", acValue);
          }
        }
        else
        {
          /* Logs failure */
          orxConsole_Log(": Invalid command!");
        }

        /* Updates input index */
        sstConsole.u32InputIndex = (sstConsole.u32InputIndex == orxCONSOLE_KU32_INPUT_ENTRY_NUMBER - 1) ? 0 : sstConsole.u32InputIndex + 1;

        /* Clears it */
        orxMemory_Zero(sstConsole.astInputEntryList[sstConsole.u32InputIndex].acBuffer, orxCONSOLE_KU32_INPUT_ENTRY_SIZE * sizeof(orxCHAR));
        sstConsole.astInputEntryList[sstConsole.u32InputIndex].u32CursorIndex = 0;

        /* Updates history index */
        sstConsole.u32HistoryIndex = sstConsole.u32InputIndex;
      }
    }

    /* Move cursor to start? */
    if(orxInput_HasBeenActivated(orxCONSOLE_KZ_INPUT_START) != orxFALSE)
    {
      /* Updates cursor position */
      pstEntry->u32CursorIndex = 0;
    }
    /* Move cursor to end? */
    else if(orxInput_HasBeenActivated(orxCONSOLE_KZ_INPUT_END) != orxFALSE)
    {
      /* Updates cursor position */
      pstEntry->u32CursorIndex = orxString_GetLength(pstEntry->acBuffer);
    }

    /* Move cursor left? */
    if(orxInput_HasBeenActivated(orxCONSOLE_KZ_INPUT_LEFT) != orxFALSE)
    {
      /* Has room? */
      if(pstEntry->u32CursorIndex > 0)
      {
        const orxCHAR  *pc, *pcLast;
        orxU32          u32CharacterCodePoint;

        /* Gets last character */
        for(pcLast = pstEntry->acBuffer, u32CharacterCodePoint = orxString_GetFirstCharacterCodePoint(pcLast, &pc);
            (*pc != orxCHAR_NULL) && ((orxU32)(pc - pstEntry->acBuffer) < pstEntry->u32CursorIndex);
            pcLast = pc, u32CharacterCodePoint = orxString_GetFirstCharacterCodePoint(pcLast, &pc));

        /* Updates cursor */
        pstEntry->u32CursorIndex -= orxString_GetUTF8CharacterLength(u32CharacterCodePoint);
      }

      /* Sets input repeat */
      orxConsole_RepeatInput(orxCONSOLE_KZ_INPUT_LEFT);
    }
    /* Move cursor right? */
    else if(orxInput_HasBeenActivated(orxCONSOLE_KZ_INPUT_RIGHT) != orxFALSE)
    {
      /* Has room? */
      if((pstEntry->u32CursorIndex < orxCONSOLE_KU32_INPUT_ENTRY_SIZE - 1) && (pstEntry->acBuffer[pstEntry->u32CursorIndex] != orxCHAR_NULL))
      {
        /* Updates cursor */
        pstEntry->u32CursorIndex += orxString_GetUTF8CharacterLength(orxString_GetFirstCharacterCodePoint(&(pstEntry->acBuffer[pstEntry->u32CursorIndex]), orxNULL));
      }

      /* Sets input repeat */
      orxConsole_RepeatInput(orxCONSOLE_KZ_INPUT_RIGHT);
    }

    /* Scroll up? */
    if(orxInput_HasBeenActivated(orxCONSOLE_KZ_INPUT_SCROLL_UP) != orxFALSE)
    {
      orxU32 u32ScrollSize;

      /* Gets scroll size */
      orxConfig_PushSection(orxCONSOLE_KZ_CONFIG_SECTION);
      u32ScrollSize = orxConfig_GetU32(orxCONSOLE_KZ_CONFIG_SCROLL_SIZE);
      u32ScrollSize = orxMAX(u32ScrollSize, 1);
      orxConfig_PopSection();

      /* Updates offset */
      sstConsole.u32LogLineOffset = (sstConsole.u32LogLineOffset <= 0xFFFFFFFF - u32ScrollSize) ? sstConsole.u32LogLineOffset + u32ScrollSize : 0xFFFFFFFF;

      /* Sets input repeat */
      orxConsole_RepeatInput(orxCONSOLE_KZ_INPUT_SCROLL_UP);
    }
    /* Scroll down? */
    else if(orxInput_HasBeenActivated(orxCONSOLE_KZ_INPUT_SCROLL_DOWN) != orxFALSE)
    {
      orxU32 u32ScrollSize;

      /* Gets scroll size */
      orxConfig_PushSection(orxCONSOLE_KZ_CONFIG_SECTION);
      u32ScrollSize = orxConfig_GetU32(orxCONSOLE_KZ_CONFIG_SCROLL_SIZE);
      u32ScrollSize = orxMAX(u32ScrollSize, 1);
      orxConfig_PopSection();

      /* Updates offset */
      sstConsole.u32LogLineOffset = (sstConsole.u32LogLineOffset >= u32ScrollSize) ? sstConsole.u32LogLineOffset - u32ScrollSize : 0;

      /* Sets input repeat */
      orxConsole_RepeatInput(orxCONSOLE_KZ_INPUT_SCROLL_DOWN);
    }

    /* Toggles mode? */
    if(orxInput_HasBeenActivated(orxCONSOLE_KZ_INPUT_TOGGLE_MODE) != orxFALSE)
    {
      /* Updates status flags */
      orxFLAG_SWAP(sstConsole.u32Flags, orxCONSOLE_KU32_STATIC_FLAG_INSERT_MODE);
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

    /* Sets insert mode */
    orxFLAG_SET(sstConsole.u32Flags, orxCONSOLE_KU32_STATIC_FLAG_INSERT_MODE, orxCONSOLE_KU32_STATIC_FLAG_NONE);

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
    case orxINPUT_EVENT_ON:
    {
      orxINPUT_EVENT_PAYLOAD *pstPayload;

      /* Gets payload */
      pstPayload = (orxINPUT_EVENT_PAYLOAD *)_pstEvent->pstPayload;

      /* Toggle? */
      if(!orxString_Compare(pstPayload->zInputName, orxCONSOLE_KZ_INPUT_TOGGLE))
      {
        /* Toggles it */
        orxConsole_Enable(!orxConsole_IsEnabled());
      }

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

/** Command: Enable
 */
void orxFASTCALL orxConsole_CommandEnable(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  /* Enable? */
  if((_u32ArgNumber == 0) || (_astArgList[0].bValue != orxFALSE))
  {
    /* Enables console */
    orxConsole_Enable(orxTRUE);

    /* Updates result */
    _pstResult->bValue = orxTRUE;
  }
  else
  {
    /* Disables console */
    orxConsole_Enable(orxFALSE);

    /* Updates result */
    _pstResult->bValue = orxFALSE;
  }

  /* Done! */
  return;
}

/** Command: Log
 */
void orxFASTCALL orxConsole_CommandLog(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  /* To system? */
  if((_u32ArgNumber > 1) && (_astArgList[1].bValue != orxFALSE))
  {
    /* Logs to system */
    orxLOG("%s", _astArgList[0].zValue);
  }
  else
  {
    /* Logs to console */
    orxConsole_Log(_astArgList[0].zValue);
  }

  /* Updates result */
  _pstResult->zValue = _astArgList[0].zValue;

  /* Done! */
  return;
}

/** Command: SetColor
 */
void orxFASTCALL orxConsole_CommandSetColor(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  /* Pushes render section */
  orxConfig_PushSection(orxRENDER_KZ_CONFIG_SECTION);

  /* Clears result */
  orxVector_SetAll(&(_pstResult->vValue), -orxFLOAT_1);

  /* Default? */
  if(_u32ArgNumber == 0)
  {
    /* Clears color */
    orxConfig_ClearValue(orxRENDER_KZ_CONFIG_CONSOLE_BACKGROUND_COLOR);
  }
  else
  {
    orxVECTOR vColor;

    /* Valid color? */
    if(orxConfig_ToVector(_astArgList[0].zValue, orxCOLORSPACE_COMPONENT, &vColor) != orxNULL)
    {
      /* Stores it */
      orxConfig_SetVector(orxRENDER_KZ_CONFIG_CONSOLE_BACKGROUND_COLOR, &vColor);

      /* Updates result */
      orxVector_Copy(&(_pstResult->vValue), &vColor);
    }
  }

  /* Done! */
  return;
}

/** Command: Echo
 */
void orxFASTCALL orxConsole_CommandEcho(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  /* Echo on? */
  if(_astArgList[0].bValue != orxFALSE)
  {
    /* Updates status */
    orxFLAG_SET(sstConsole.u32Flags, orxCONSOLE_KU32_STATIC_FLAG_ECHO, orxCONSOLE_KU32_STATIC_FLAG_NONE);
  }
  else
  {
    /* Updates status */
    orxFLAG_SET(sstConsole.u32Flags, orxCONSOLE_KU32_STATIC_FLAG_NONE, orxCONSOLE_KU32_STATIC_FLAG_ECHO);
  }

  /* Updates result */
  _pstResult->bValue = _astArgList[0].bValue;

  /* Done! */
  return;
}


/** Registers all the console commands
 */
static orxINLINE void orxConsole_RegisterCommands()
{
  /* Command: Enable */
  orxCOMMAND_REGISTER_CORE_COMMAND(Console, Enable, "Enabled?", orxCOMMAND_VAR_TYPE_BOOL, 0, 1, {"Enable = true", orxCOMMAND_VAR_TYPE_BOOL});

  /* Command: Log */
  orxCOMMAND_REGISTER_CORE_COMMAND(Console, Log, "Log", orxCOMMAND_VAR_TYPE_STRING, 1, 1, {"Text", orxCOMMAND_VAR_TYPE_STRING}, {"ToSystem = false", orxCOMMAND_VAR_TYPE_BOOL});

  /* Command: SetColor */
  orxCOMMAND_REGISTER_CORE_COMMAND(Console, SetColor, "Color", orxCOMMAND_VAR_TYPE_VECTOR, 0, 1, {"Color = <default>", orxCOMMAND_VAR_TYPE_STRING});

  /* Command: Echo */
  orxCOMMAND_REGISTER_CORE_COMMAND(Console, Echo, "Echo", orxCOMMAND_VAR_TYPE_BOOL, 0, 1, {"Echo = true", orxCOMMAND_VAR_TYPE_BOOL});

  /* Alias: Log */
  orxCommand_AddAlias("Log", "Console.Log", orxNULL);

  /* Alias: Echo */
  orxCommand_AddAlias("Echo", "Console.Echo", orxNULL);
}

/** Unregisters all the console commands
 */
static orxINLINE void orxConsole_UnregisterCommands()
{
  /* Alias: Log */
  orxCommand_RemoveAlias("Log");

  /* Alias: Echo */
  orxCommand_RemoveAlias("Echo");

  /* Command: Enable */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Console, Enable);

  /* Command: Log */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Console, Log);

  /* Command: SetColor */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Console, SetColor);

  /* Command: Echo */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Console, Echo);
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
  orxModule_AddDependency(orxMODULE_ID_CONSOLE, orxMODULE_ID_STRING);
  orxModule_AddDependency(orxMODULE_ID_CONSOLE, orxMODULE_ID_CLOCK);
  orxModule_AddDependency(orxMODULE_ID_CONSOLE, orxMODULE_ID_COMMAND);
  orxModule_AddDependency(orxMODULE_ID_CONSOLE, orxMODULE_ID_CONFIG);
  orxModule_AddDependency(orxMODULE_ID_CONSOLE, orxMODULE_ID_EVENT);
  orxModule_AddDependency(orxMODULE_ID_CONSOLE, orxMODULE_ID_PROFILER);
  orxModule_AddDependency(orxMODULE_ID_CONSOLE, orxMODULE_ID_INPUT);
  orxModule_AddDependency(orxMODULE_ID_CONSOLE, orxMODULE_ID_STRUCTURE);
  orxModule_AddDependency(orxMODULE_ID_CONSOLE, orxMODULE_ID_FONT);

  orxModule_AddOptionalDependency(orxMODULE_ID_CONSOLE, orxMODULE_ID_KEYBOARD);

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
    /* Cleans control structure */
    orxMemory_Zero(&sstConsole, sizeof(orxCONSOLE_STATIC));

    /* Stores default log line length */
    sstConsole.u32LogLineLength = orxCONSOLE_KU32_DEFAULT_LOG_LINE_LENGTH;

    /* Clears last result */
    sstConsole.stLastResult.eType = orxCOMMAND_VAR_TYPE_NONE;

    /* Stores default toggle key */
    sstConsole.eToggleKeyType = orxINPUT_TYPE_KEYBOARD_KEY;
    sstConsole.eToggleKeyID   = orxCONSOLE_KE_DEFAULT_KEY_TOGGLE;
    sstConsole.eToggleKeyMode = orxINPUT_MODE_FULL;

    /* Pushes its section */
    orxConfig_PushSection(orxCONSOLE_KZ_CONFIG_SECTION);

    /* Has toggle key? */
    if(orxConfig_HasValue(orxCONSOLE_KZ_CONFIG_TOGGLE_KEY) != orxFALSE)
    {
      orxINPUT_TYPE eType;
      orxENUM       eID;
      orxINPUT_MODE eMode;

      /* Gets its type & ID */
      if(orxInput_GetBindingType(orxConfig_GetString(orxCONSOLE_KZ_CONFIG_TOGGLE_KEY), &eType, &eID, &eMode) != orxSTATUS_FAILURE)
      {
        /* Stores it */
        sstConsole.eToggleKeyType = eType;
        sstConsole.eToggleKeyID   = eID;
        sstConsole.eToggleKeyMode = eMode;
      }
      else
      {
        /* Resets it */
        sstConsole.eToggleKeyType = orxINPUT_TYPE_NONE;
        sstConsole.eToggleKeyID   = orxENUM_NONE;
        sstConsole.eToggleKeyMode = orxINPUT_MODE_NONE;
      }
    }

    /* Doesn't have scroll size? */
    if(orxConfig_HasValue(orxCONSOLE_KZ_CONFIG_SCROLL_SIZE) == orxFALSE)
    {
      /* Uses default value */
      orxConfig_SetU32(orxCONSOLE_KZ_CONFIG_SCROLL_SIZE, orxCONSOLE_KU32_SCROLL_SIZE);
    }

    /* Pops config section */
    orxConfig_PopSection();

    /* Adds event handler */
    eResult = orxEvent_AddHandler(orxEVENT_TYPE_INPUT, orxConsole_EventHandler);

    /* Success? */
    if(eResult != orxSTATUS_FAILURE)
    {
      const orxSTRING zPreviousSet;

      /* Filters relevant event IDs */
      orxEvent_SetHandlerIDFlags(orxConsole_EventHandler, orxEVENT_TYPE_INPUT, orxNULL, orxEVENT_GET_FLAG(orxINPUT_EVENT_ON), orxEVENT_KU32_MASK_ID_ALL);

      /* Backups previous input set */
      zPreviousSet = orxInput_GetCurrentSet();

      /* Selects console input set */
      eResult = orxInput_SelectSet(orxCONSOLE_KZ_INPUT_SET);

      /* Success */
      if(eResult != orxSTATUS_FAILURE)
      {
        /* Binds console inputs */
        orxInput_Bind(orxCONSOLE_KZ_INPUT_TOGGLE, sstConsole.eToggleKeyType, sstConsole.eToggleKeyID, sstConsole.eToggleKeyMode, -1);
        orxInput_Bind(orxCONSOLE_KZ_INPUT_AUTOCOMPLETE, orxINPUT_TYPE_KEYBOARD_KEY, orxCONSOLE_KE_KEY_AUTOCOMPLETE, orxINPUT_MODE_FULL, -1);
        orxInput_Bind(orxCONSOLE_KZ_INPUT_DELETE, orxINPUT_TYPE_KEYBOARD_KEY, orxCONSOLE_KE_KEY_DELETE, orxINPUT_MODE_FULL, -1);
        orxInput_Bind(orxCONSOLE_KZ_INPUT_DELETE_AFTER, orxINPUT_TYPE_KEYBOARD_KEY, orxCONSOLE_KE_KEY_DELETE_AFTER, orxINPUT_MODE_FULL, -1);
        orxInput_Bind(orxCONSOLE_KZ_INPUT_TOGGLE_MODE, orxINPUT_TYPE_KEYBOARD_KEY, orxCONSOLE_KE_KEY_TOGGLE_MODE, orxINPUT_MODE_FULL, -1);
        orxInput_Bind(orxCONSOLE_KZ_INPUT_ENTER, orxINPUT_TYPE_KEYBOARD_KEY, orxCONSOLE_KE_KEY_ENTER, orxINPUT_MODE_FULL, -1);
        orxInput_Bind(orxCONSOLE_KZ_INPUT_ENTER, orxINPUT_TYPE_KEYBOARD_KEY, orxCONSOLE_KE_KEY_ENTER_ALTERNATE, orxINPUT_MODE_FULL, -1);
        orxInput_Bind(orxCONSOLE_KZ_INPUT_PREVIOUS, orxINPUT_TYPE_KEYBOARD_KEY, orxCONSOLE_KE_KEY_PREVIOUS, orxINPUT_MODE_FULL, -1);
        orxInput_Bind(orxCONSOLE_KZ_INPUT_NEXT, orxINPUT_TYPE_KEYBOARD_KEY, orxCONSOLE_KE_KEY_NEXT, orxINPUT_MODE_FULL, -1);
        orxInput_Bind(orxCONSOLE_KZ_INPUT_LEFT, orxINPUT_TYPE_KEYBOARD_KEY, orxCONSOLE_KE_KEY_LEFT, orxINPUT_MODE_FULL, -1);
        orxInput_Bind(orxCONSOLE_KZ_INPUT_RIGHT, orxINPUT_TYPE_KEYBOARD_KEY, orxCONSOLE_KE_KEY_RIGHT, orxINPUT_MODE_FULL, -1);
        orxInput_Bind(orxCONSOLE_KZ_INPUT_START, orxINPUT_TYPE_KEYBOARD_KEY, orxCONSOLE_KE_KEY_START, orxINPUT_MODE_FULL, -1);
        orxInput_Bind(orxCONSOLE_KZ_INPUT_END, orxINPUT_TYPE_KEYBOARD_KEY, orxCONSOLE_KE_KEY_END, orxINPUT_MODE_FULL, -1);
        orxInput_Bind(orxCONSOLE_KZ_INPUT_PASTE, orxINPUT_TYPE_KEYBOARD_KEY, orxCONSOLE_KE_KEY_CONTROL, orxINPUT_MODE_FULL, -1);
        orxInput_Bind(orxCONSOLE_KZ_INPUT_PASTE, orxINPUT_TYPE_KEYBOARD_KEY, orxCONSOLE_KE_KEY_PASTE, orxINPUT_MODE_FULL, -1);
        orxInput_Bind(orxCONSOLE_KZ_INPUT_SCROLL_UP, orxINPUT_TYPE_KEYBOARD_KEY, orxCONSOLE_KE_KEY_SCROLL_UP, orxINPUT_MODE_FULL, -1);
        orxInput_Bind(orxCONSOLE_KZ_INPUT_SCROLL_DOWN, orxINPUT_TYPE_KEYBOARD_KEY, orxCONSOLE_KE_KEY_SCROLL_DOWN, orxINPUT_MODE_FULL, -1);
        orxInput_Bind(orxCONSOLE_KZ_INPUT_SCROLL_UP, orxINPUT_TYPE_MOUSE_BUTTON, orxCONSOLE_KE_BUTTON_SCROLL_UP, orxINPUT_MODE_FULL, -1);
        orxInput_Bind(orxCONSOLE_KZ_INPUT_SCROLL_DOWN, orxINPUT_TYPE_MOUSE_BUTTON, orxCONSOLE_KE_BUTTON_SCROLL_DOWN, orxINPUT_MODE_FULL, -1);

        /* Sets combine mode for paste */
        orxInput_SetCombineMode(orxCONSOLE_KZ_INPUT_PASTE, orxTRUE);

        /* Enables set */
        orxInput_EnableSet(orxCONSOLE_KZ_INPUT_SET, orxTRUE);

        /* Restores previous set */
        orxInput_SelectSet(zPreviousSet);

        /* Registers update callback */
        eResult = orxClock_Register(orxClock_Get(orxCLOCK_KZ_CORE), orxConsole_Update, orxNULL, orxMODULE_ID_CONSOLE, orxCLOCK_PRIORITY_HIGH);

        /* Success? */
        if(eResult != orxSTATUS_FAILURE)
        {
          orxU32  i, u32Count;
          orxBOOL bDebugLevelBackup;

          /* Inits log end index */
          sstConsole.u32LogEndIndex = orxU32_UNDEFINED;

          /* Pushes config section */
          orxConfig_PushSection(orxCONSOLE_KZ_CONFIG_SECTION);

          /* For all keys */
          for(i = 0, u32Count = orxConfig_GetKeyCount(); i < u32Count; i++)
          {
            const orxSTRING zKey;

            /* Gets it */
            zKey = orxConfig_GetKey(i);

            /* Isn't toggle key, console input history nor scroll size? */
            if((orxString_Compare(zKey, orxCONSOLE_KZ_CONFIG_TOGGLE_KEY) != 0)
            && (orxString_Compare(zKey, orxCONSOLE_KZ_CONFIG_INPUT_HISTORY_LIST) != 0)
            && (orxString_Compare(zKey, orxCONSOLE_KZ_CONFIG_SCROLL_SIZE) != 0))
            {
              const orxSTRING zAlias;
              orxCHAR         cBackup = orxCHAR_NULL, *pc;

              /* Gets its content */
              zAlias = orxConfig_GetString(zKey);

              /* Finds its end */
              for(pc = (orxCHAR *)zAlias; (*pc != orxCHAR_NULL) && (*pc != ' ') && (*pc != '\t'); pc++);

              /* Has args? */
              if(*pc != orxCHAR_NULL)
              {
               /* Backups character */
               cBackup = *pc;

               /* Ends alias */
               *pc = orxCHAR_NULL;
              }

              /* Adds it as alias */
              orxCommand_AddAlias(zKey, orxConfig_GetString(zKey), (cBackup != orxCHAR_NULL) ? pc + 1 : orxNULL);

              /* Had args? */
              if(cBackup != orxCHAR_NULL)
              {
                /* Restores it */
                *pc = cBackup;
              }
            }
          }

          /* Pops config section */
          orxConfig_PopSection();

          /* Inits Flags */
          sstConsole.u32Flags = orxCONSOLE_KU32_STATIC_FLAG_READY;

          /* Registers commands */
          orxConsole_RegisterCommands();

          /* Sets default font */
          orxConsole_SetFont(orxFont_GetDefaultFont());

          /* Disables config logs */
          bDebugLevelBackup = orxDEBUG_IS_LEVEL_ENABLED(orxDEBUG_LEVEL_CONFIG);
          orxDEBUG_ENABLE_LEVEL(orxDEBUG_LEVEL_CONFIG, orxFALSE);

          /* Loads input history */
          orxConsole_LoadHistory();

          /* Reenables config logs */
          orxDEBUG_ENABLE_LEVEL(orxDEBUG_LEVEL_CONFIG, bDebugLevelBackup);
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
    /* Unregisters commands */
    orxConsole_UnregisterCommands();

    /* Stops console */
    orxConsole_Stop();

    /* Removes font */
    orxConsole_SetFont(orxNULL);

    /* Unregisters update callback */
    orxClock_Unregister(orxClock_Get(orxCLOCK_KZ_CORE), orxConsole_Update);

    /* Remove event handler */
    orxEvent_RemoveHandler(orxEVENT_TYPE_INPUT, orxConsole_EventHandler);

    /* Saves input history */
    orxConsole_SaveHistory();

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

    /* Resets log offset */
    sstConsole.u32LogLineOffset = 0;

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

/** Is the console input in insert mode?
 * @return orxTRUE if insert mode, orxFALSE otherwise (overwrite mode)
 */
orxBOOL orxFASTCALL orxConsole_IsInsertMode()
{
  orxBOOL bResult;

  /* Checks */
  orxASSERT(sstConsole.u32Flags & orxCONSOLE_KU32_STATIC_FLAG_READY);

  /* Updates result */
  bResult = orxFLAG_TEST(sstConsole.u32Flags, orxCONSOLE_KU32_STATIC_FLAG_INSERT_MODE) ? orxTRUE : orxFALSE;

  /* Done! */
  return bResult;
}

/** Sets the console toggle
* @param[in] _eInputType      Type of input peripheral
* @param[in] _eInputID        ID of button/key/axis
* @param[in] _eInputMode      Mode of input
* @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxConsole_SetToggle(orxINPUT_TYPE _eInputType, orxENUM _eInputID, orxINPUT_MODE _eInputMode)
{
  const orxSTRING zPreviousSet;
  orxSTATUS       eResult;

  /* Checks */
  orxASSERT(sstConsole.u32Flags & orxCONSOLE_KU32_STATIC_FLAG_READY);

  /* Backups previous input set */
  zPreviousSet = orxInput_GetCurrentSet();

  /* Selects console set */
  orxInput_SelectSet(orxCONSOLE_KZ_INPUT_SET);

  /* Has current bindings? */
  if((sstConsole.eToggleKeyType != orxINPUT_TYPE_NONE)
  && (sstConsole.eToggleKeyID != orxENUM_NONE)
  && (sstConsole.eToggleKeyMode != orxINPUT_MODE_NONE))
  {
    /* Unbinds current toggle */
    orxInput_Unbind(orxCONSOLE_KZ_INPUT_TOGGLE, -1);
  }

  /* Binds new toggle */
  eResult = orxInput_Bind(orxCONSOLE_KZ_INPUT_TOGGLE, _eInputType, _eInputID, _eInputMode, -1);

  /* Success? */
  if(eResult != orxSTATUS_FAILURE)
  {
    /* Stores new toggle bindings */
    sstConsole.eToggleKeyType = _eInputType;
    sstConsole.eToggleKeyID   = _eInputID;
    sstConsole.eToggleKeyMode = _eInputMode;
  }
  else
  {
    /* Clears toggle bindings */
    sstConsole.eToggleKeyType = orxINPUT_TYPE_NONE;
    sstConsole.eToggleKeyID   = orxENUM_NONE;
    sstConsole.eToggleKeyMode = orxINPUT_MODE_NONE;
  }

  /* Restores previous input set */
  orxInput_SelectSet(zPreviousSet);

  /* Done! */
  return eResult;
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
  if(sstConsole.u32LogIndex + u32TextLength + (u32TextLength / sstConsole.u32LogLineLength) + 2 > orxCONSOLE_KU32_LOG_BUFFER_SIZE)
  {
    /* Stores log end index */
    sstConsole.u32LogEndIndex = sstConsole.u32LogIndex;

    /* Resets log index */
    sstConsole.u32LogIndex = 0;
  }

  /* For all characters */
  for(u32LineLength = 0, pc = _zText; (*pc != orxCHAR_NULL) && (sstConsole.u32LogIndex < orxCONSOLE_KU32_LOG_BUFFER_SIZE - 2);)
  {
    orxU32 u32CharacterCodePoint;

    /* Gets character code point */
    u32CharacterCodePoint = orxString_GetFirstCharacterCodePoint(pc, &pc);

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
      orxU32 u32CharacterLength;

      /* Appends character to log */
      u32CharacterLength = orxString_PrintUTF8Character(sstConsole.acLogBuffer + sstConsole.u32LogIndex, orxCONSOLE_KU32_LOG_BUFFER_SIZE - sstConsole.u32LogIndex - 1, u32CharacterCodePoint);

      /* Success? */
      if(u32CharacterLength != 0)
      {
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
      else
      {
        /* Stops */
        break;
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
    /* Updates its reference count */
    orxStructure_DecreaseCount((orxFONT *)sstConsole.pstFont);
  }

  /* Is font valid? */
  if(_pstFont != orxNULL)
  {
    /* Stores it */
    sstConsole.pstFont = _pstFont;

    /* Updates its reference count */
    orxStructure_IncreaseCount((orxFONT *)sstConsole.pstFont);
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

/** Gets current completions count
 * @param[out]  _pu32MaxLength Max completion length, orxNULL to ignore
 * @return Current completions count
 */
orxU32 orxFASTCALL orxConsole_GetCompletionCount(orxU32 *_pu32MaxLength)
{
  orxU32 u32Length = 0, u32Result = 0;

  /* Checks */
  orxASSERT(sstConsole.u32Flags & orxCONSOLE_KU32_STATIC_FLAG_READY);

  /* Has completion? */
  if(sstConsole.zCompletedCommand != orxNULL)
  {
    orxCONSOLE_INPUT_ENTRY *pstEntry;
    orxCHAR                 acBuffer[orxCONSOLE_KU32_INPUT_ENTRY_SIZE], *pc;

    /* Gets entry */
    pstEntry = &(sstConsole.astInputEntryList[sstConsole.u32InputIndex]);

    /* Gets start of line */
    orxString_NPrint(acBuffer, sizeof(acBuffer), "%.*s", pstEntry->u32CursorIndex, pstEntry->acBuffer);
    for(pc = acBuffer + pstEntry->u32CursorIndex; (pc >= acBuffer) && (*pc != ' ') && (*pc != '\t') && (*pc != orxCOMMAND_KC_PUSH_MARKER); pc--)
      ;
    pc++;

    /* Not empty? */
    if(*pc != orxCHAR_NULL)
    {
      const orxSTRING zCommand;

      /* Gets completion count */
      for(u32Result = 0, zCommand = orxCommand_GetNext(pc, orxNULL, orxNULL);
          zCommand != orxNULL;
          u32Result++, zCommand = orxCommand_GetNext(pc, zCommand, orxNULL))
      {
        orxU32 u32NewLength;

        /* Gets new length */
        u32NewLength = orxString_GetLength(zCommand);

        /* Updates length */
        u32Length = orxMAX(u32Length, u32NewLength);
      }
    }
  }
  else
  {
    /* Updates result */
    u32Result = 0;
  }

  /* Asked for max length? */
  if(_pu32MaxLength != orxNULL)
  {
    /* Updates it */
    *_pu32MaxLength = u32Length;
  }

  /* Done! */
  return u32Result;
}

/** Gets completion
 * @param[in]   _u32Index     Index of the active completion
 * @param[out]  _pbActive     Is completion active, orxNULL to ignore
 * @return Completion string if found, orxSTRING_EMPTY otherwise
 */
const orxSTRING orxFASTCALL orxConsole_GetCompletion(orxU32 _u32Index, orxBOOL *_pbActive)
{
  const orxSTRING zResult;

  /* Checks */
  orxASSERT(sstConsole.u32Flags & orxCONSOLE_KU32_STATIC_FLAG_READY);

  /* Has completion? */
  if(sstConsole.zCompletedCommand != orxNULL)
  {
    orxCONSOLE_INPUT_ENTRY *pstEntry;
    orxCHAR                 acBuffer[orxCONSOLE_KU32_INPUT_ENTRY_SIZE], *pc;

    /* Gets entry */
    pstEntry = &(sstConsole.astInputEntryList[sstConsole.u32InputIndex]);

    /* Gets start of line */
    orxString_NPrint(acBuffer, sizeof(acBuffer), "%.*s", pstEntry->u32CursorIndex, pstEntry->acBuffer);
    for(pc = acBuffer + pstEntry->u32CursorIndex; (pc >= acBuffer) && (*pc != ' ') && (*pc != '\t') && (*pc != orxCOMMAND_KC_PUSH_MARKER); pc--)
      ;
    pc++;

    /* Not empty? */
    if(*pc != orxCHAR_NULL)
    {
      orxU32 u32CompletionIndex;

      /* Finds requested completion */
      for(u32CompletionIndex = 0, zResult = orxCommand_GetNext(pc, orxNULL, orxNULL);
          (u32CompletionIndex < _u32Index) && (zResult != orxNULL);
          u32CompletionIndex++, zResult = orxCommand_GetNext(pc, zResult, orxNULL))
        ;

      /* Not found? */
      if(zResult == orxNULL)
      {
        /* Updates result */
        zResult = orxSTRING_EMPTY;
      }
    }
    else
    {
      /* Clears result */
      zResult = orxSTRING_EMPTY;
    }
  }
  else
  {
    /* Clears result */
    zResult = orxSTRING_EMPTY;
  }

  /* Asked for active status? */
  if(_pbActive != orxNULL)
  {
    *_pbActive = (zResult == sstConsole.zCompletedCommand) ? orxTRUE : orxFALSE;
  }

  /* Done! */
  return zResult;
}

/** Gets log line from the end (trail)
 * @param[in]   _u32TrailLineIndex Index of the line starting from end
 * @return orxTRING / orxSTRING_EMPTY
 */
const orxSTRING orxFASTCALL orxConsole_GetTrailLogLine(orxU32 _u32TrailLineIndex)
{
  orxU32          i, u32LogIndex;
  orxBOOL         bWrapped;
  const orxSTRING zResult = orxSTRING_EMPTY;

  /* Checks */
  orxASSERT(sstConsole.u32Flags & orxCONSOLE_KU32_STATIC_FLAG_READY);

  /* For all lines */
  for(i = 0, u32LogIndex = (sstConsole.u32LogIndex != 0) ? sstConsole.u32LogIndex - 1 : 0, bWrapped = orxFALSE; i <= _u32TrailLineIndex + sstConsole.u32LogLineOffset; i++)
  {
    /* Not wrapped yet? */
    if(bWrapped == orxFALSE)
    {
      /* End of buffer? */
      if(u32LogIndex == 0)
      {
        /* Valid? */
        if(sstConsole.u32LogEndIndex != orxU32_UNDEFINED)
        {
          /* Wraps around */
          u32LogIndex = sstConsole.u32LogEndIndex - 2;

          /* Updates wrap status */
          bWrapped = orxTRUE;
        }
        else
        {
          /* Stops */
          break;
        }
      }
      else
      {
        /* Updates index */
        u32LogIndex = u32LogIndex - 1;
      }
    }
    else
    {
      /* End of buffer? */
      if(u32LogIndex <= sstConsole.u32LogIndex)
      {
        /* Stops */
        break;
      }
      else
      {
        /* Updates index */
        u32LogIndex = u32LogIndex - 1;
      }
    }

    /* Finds start of current log line */
    while((u32LogIndex != 0) && (sstConsole.acLogBuffer[u32LogIndex] != orxCHAR_NULL))
    {
      u32LogIndex--;
    }
  }

  /* Found? */
  if(i > _u32TrailLineIndex + sstConsole.u32LogLineOffset)
  {
    /* Updates result */
    zResult = &sstConsole.acLogBuffer[(u32LogIndex != 0) ? u32LogIndex + 1 : u32LogIndex];
  }

  /* Done! */
  return zResult;
}

/** Gets log line offset from the end
 * @return Log line offset from the end
 */
orxU32 orxFASTCALL orxConsole_GetTrailLogLineOffset()
{
  orxU32 u32Result;

  /* Checks */
  orxASSERT(sstConsole.u32Flags & orxCONSOLE_KU32_STATIC_FLAG_READY);

  /* Updates result */
  u32Result = sstConsole.u32LogLineOffset;

  /* Done! */
  return u32Result;
}

/** Gets input text
 * @param[out]  _pu32CursorIndex Index (ie. character position) of the cursor (any character past it has not been validated)
 * @return orxTRING / orxSTRING_EMPTY
 */
const orxSTRING orxFASTCALL orxConsole_GetInput(orxU32 *_pu32CursorIndex)
{
  const orxSTRING zResult;

  /* Checks */
  orxASSERT(sstConsole.u32Flags & orxCONSOLE_KU32_STATIC_FLAG_READY);

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
