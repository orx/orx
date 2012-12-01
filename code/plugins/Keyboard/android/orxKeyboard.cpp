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
 * @file orxKeyboard.c
 * @date 26/06/2010
 * @author iarwain@orx-project.org
 *
 * Android keyboard plugin implementation
 *
 */


#include "orxPluginAPI.h"

#include "main/orxAndroid.h"

/** Module flags
 */
#define orxKEYBOARD_KU32_STATIC_FLAG_NONE       0x00000000 /**< No flags */

#define orxKEYBOARD_KU32_STATIC_FLAG_READY      0x00000001 /**< Ready flag */

#define orxKEYBOARD_KU32_STATIC_MASK_ALL        0xFFFFFFFF /**< All mask */


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Static structure
 */
typedef struct __orxKEYBOARD_STATIC_t
{
  orxU32            u32Flags;
  orxBOOL           abKeyPressed[2];
} orxKEYBOARD_STATIC;


/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** Static data
 */
static orxKEYBOARD_STATIC sstKeyboard;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/
static orxS32 orxFASTCALL orxKeyboard_Android_GetKeyIdx(orxKEYBOARD_KEY _eKey)
{
  orxS32 eResult;

  /* Depending on key */
  switch(_eKey)
  {
    case orxKEYBOARD_KEY_ESCAPE:        {eResult = 0; break;}
    case orxKEYBOARD_KEY_MENU:          {eResult = 1; break;}

    default:                            {eResult = -1; break;}
  }

  /* Done! */
  return eResult;
}

static orxSTATUS orxFASTCALL orxKeyboard_Android_EventHandler(const orxEVENT *_pstEvent)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;
  jint* pKeyCode = (jint *) _pstEvent->pstPayload;
  orxS32 s32KeyIdx = -1;

  switch(*pKeyCode)
  {
    case KEYCODE_BACK:
      s32KeyIdx = 0;
      break;
    case KEYCODE_MENU:
      s32KeyIdx = 1;
      break;
  }

  if(s32KeyIdx != -1)
  {
    /* Depending on ID */
    switch(_pstEvent->eID)
    {
      case orxANDROID_EVENT_KEYBOARD_DOWN:
        sstKeyboard.abKeyPressed[s32KeyIdx] = orxTRUE;
        break;
      case orxANDROID_EVENT_KEYBOARD_UP:
        sstKeyboard.abKeyPressed[s32KeyIdx] = orxFALSE;
        break;
    }
  }
  else
  {
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

extern "C" orxSTATUS orxFASTCALL orxKeyboard_Android_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Wasn't already initialized? */
  if(!(sstKeyboard.u32Flags & orxKEYBOARD_KU32_STATIC_FLAG_READY))
  {
    /* Cleans static controller */
    orxMemory_Zero(&sstKeyboard, sizeof(orxKEYBOARD_STATIC));
    
    /* Adds our mouse event handlers */
    if((eResult = orxEvent_AddHandler((orxEVENT_TYPE)(orxEVENT_TYPE_FIRST_RESERVED + orxANDROID_EVENT_KEYBOARD), orxKeyboard_Android_EventHandler)) != orxSTATUS_FAILURE)
    {
      int i;
      for(i = 0; i < 2; i++)
      {
        sstKeyboard.abKeyPressed[i] = orxFALSE;
      }

      /* Updates status */
      sstKeyboard.u32Flags |= orxKEYBOARD_KU32_STATIC_FLAG_READY;
    }
 }

  /* Done! */
  return eResult;
}

extern "C" void orxFASTCALL orxKeyboard_Android_Exit()
{
  /* Was initialized? */
  if(sstKeyboard.u32Flags & orxKEYBOARD_KU32_STATIC_FLAG_READY)
  {
    /* Removes event handler */
    orxEvent_RemoveHandler((orxEVENT_TYPE)(orxEVENT_TYPE_FIRST_RESERVED + orxANDROID_EVENT_KEYBOARD), orxKeyboard_Android_EventHandler);

    /* Cleans static controller */
    orxMemory_Zero(&sstKeyboard, sizeof(orxKEYBOARD_STATIC));
  }

  return;
}

extern "C" orxBOOL orxFASTCALL orxKeyboard_Android_IsKeyPressed(orxKEYBOARD_KEY _eKey)
{
  orxS32 s32KeyIdx;
  orxBOOL bResult;

  /* Checks */
  orxASSERT(_eKey < orxKEYBOARD_KEY_NUMBER);
  orxASSERT((sstKeyboard.u32Flags & orxKEYBOARD_KU32_STATIC_FLAG_READY) == orxKEYBOARD_KU32_STATIC_FLAG_READY);

  /* Gets Event key enum */
  s32KeyIdx = orxKeyboard_Android_GetKeyIdx(_eKey);

  /* Valid? */
  if(s32KeyIdx != -1)
  {
    /* Updates result */
    bResult = sstKeyboard.abKeyPressed[s32KeyIdx];
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_KEYBOARD,"Key <%s> is not handled by this plugin.", orxKeyboard_GetKeyName(_eKey));

    /* Updates result */
    bResult = orxFALSE;
  }

  /* Done! */
  return bResult;
}


extern "C" orxKEYBOARD_KEY orxFASTCALL orxKeyboard_Android_ReadKey()
{
  orxKEYBOARD_KEY eResult = orxKEYBOARD_KEY_NONE;

  /* Not yet implemented */
  orxDEBUG_PRINT(orxDEBUG_LEVEL_KEYBOARD, "Not yet implemented!");

  /* Done! */
  return eResult;
}

extern "C" const orxSTRING orxFASTCALL orxKeyboard_Android_ReadString()
{
  const orxSTRING zResult = orxSTRING_EMPTY;

  /* Not yet implemented */
  orxDEBUG_PRINT(orxDEBUG_LEVEL_KEYBOARD, "Not yet implemented!");

  /* Done! */
  return zResult;
}

extern "C" void orxFASTCALL orxKeyboard_Android_ClearBuffer()
{
  /* Not yet implemented */
  orxDEBUG_PRINT(orxDEBUG_LEVEL_KEYBOARD, "Not yet implemented!");

  /* Done! */
  return;
}


/***************************************************************************
 * Plugin related                                                          *
 ***************************************************************************/

orxPLUGIN_USER_CORE_FUNCTION_START(KEYBOARD);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxKeyboard_Android_Init, KEYBOARD, INIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxKeyboard_Android_Exit, KEYBOARD, EXIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxKeyboard_Android_IsKeyPressed, KEYBOARD, IS_KEY_PRESSED);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxKeyboard_Android_ReadKey, KEYBOARD, READ_KEY);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxKeyboard_Android_ReadString, KEYBOARD, READ_STRING);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxKeyboard_Android_ClearBuffer, KEYBOARD, CLEAR_BUFFER);
orxPLUGIN_USER_CORE_FUNCTION_END();
