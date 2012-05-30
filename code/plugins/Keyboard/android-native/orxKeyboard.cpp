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

#include <android/input.h>
#include <android/keycodes.h>

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
  orxBOOL           abKeyPressed[AKEYCODE_BUTTON_MODE + 1];
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
static int32_t orxFASTCALL orxKeyboard_Android_GetAKey(orxKEYBOARD_KEY _eKey)
{
  int32_t eResult;

  /* Depending on key */
  switch(_eKey)
  {
    case orxKEYBOARD_KEY_ESCAPE:        {eResult = AKEYCODE_BACK; break;}
    case orxKEYBOARD_KEY_MENU:          {eResult = AKEYCODE_MENU; break;}

    case orxKEYBOARD_KEY_SPACE:         {eResult = AKEYCODE_SPACE; break;}
    case orxKEYBOARD_KEY_RETURN:        {eResult = AKEYCODE_ENTER; break;}
    case orxKEYBOARD_KEY_BACKSPACE:     {eResult = AKEYCODE_DEL; break;}
    case orxKEYBOARD_KEY_TAB:           {eResult = AKEYCODE_TAB; break;}
    case orxKEYBOARD_KEY_HOME:          {eResult = AKEYCODE_HOME; break;}
    case orxKEYBOARD_KEY_ADD:           {eResult = AKEYCODE_PLUS; break;}
    case orxKEYBOARD_KEY_SUBTRACT:      {eResult = AKEYCODE_MINUS; break;}
    case orxKEYBOARD_KEY_MULTIPLY:      {eResult = AKEYCODE_STAR; break;}
    case orxKEYBOARD_KEY_RALT:          {eResult = AKEYCODE_ALT_RIGHT; break;}
    case orxKEYBOARD_KEY_RSHIFT:        {eResult = AKEYCODE_SHIFT_RIGHT; break;}
    case orxKEYBOARD_KEY_LALT:          {eResult = AKEYCODE_ALT_LEFT; break;}
    case orxKEYBOARD_KEY_LSHIFT:        {eResult = AKEYCODE_SHIFT_LEFT; break;}
    case orxKEYBOARD_KEY_LBRACKET:      {eResult = AKEYCODE_LEFT_BRACKET; break;}
    case orxKEYBOARD_KEY_RBRACKET:      {eResult = AKEYCODE_RIGHT_BRACKET; break;}
    case orxKEYBOARD_KEY_SEMICOLON:     {eResult = AKEYCODE_SEMICOLON; break;}
    case orxKEYBOARD_KEY_COMMA:         {eResult = AKEYCODE_COMMA; break;}
    case orxKEYBOARD_KEY_PERIOD:        {eResult = AKEYCODE_PERIOD; break;}
    case orxKEYBOARD_KEY_SLASH:         {eResult = AKEYCODE_SLASH; break;}
    case orxKEYBOARD_KEY_BACKSLASH:     {eResult = AKEYCODE_BACKSLASH; break;}
    case orxKEYBOARD_KEY_EQUAL:         {eResult = AKEYCODE_EQUALS; break;}
    case orxKEYBOARD_KEY_UP:            {eResult = AKEYCODE_DPAD_UP; break;}
    case orxKEYBOARD_KEY_RIGHT:         {eResult = AKEYCODE_DPAD_RIGHT; break;}
    case orxKEYBOARD_KEY_DOWN:          {eResult = AKEYCODE_DPAD_DOWN; break;}
    case orxKEYBOARD_KEY_LEFT:          {eResult = AKEYCODE_DPAD_LEFT; break;}
    case orxKEYBOARD_KEY_A:             {eResult = AKEYCODE_A; break;}
    case orxKEYBOARD_KEY_Z:             {eResult = AKEYCODE_Z; break;}
    case orxKEYBOARD_KEY_E:             {eResult = AKEYCODE_E; break;}
    case orxKEYBOARD_KEY_R:             {eResult = AKEYCODE_R; break;}
    case orxKEYBOARD_KEY_T:             {eResult = AKEYCODE_T; break;}
    case orxKEYBOARD_KEY_Y:             {eResult = AKEYCODE_Y; break;}
    case orxKEYBOARD_KEY_U:             {eResult = AKEYCODE_U; break;}
    case orxKEYBOARD_KEY_I:             {eResult = AKEYCODE_I; break;}
    case orxKEYBOARD_KEY_O:             {eResult = AKEYCODE_O; break;}
    case orxKEYBOARD_KEY_P:             {eResult = AKEYCODE_P; break;}
    case orxKEYBOARD_KEY_Q:             {eResult = AKEYCODE_Q; break;}
    case orxKEYBOARD_KEY_S:             {eResult = AKEYCODE_S; break;}
    case orxKEYBOARD_KEY_D:             {eResult = AKEYCODE_D; break;}
    case orxKEYBOARD_KEY_F:             {eResult = AKEYCODE_F; break;}
    case orxKEYBOARD_KEY_G:             {eResult = AKEYCODE_G; break;}
    case orxKEYBOARD_KEY_H:             {eResult = AKEYCODE_H; break;}
    case orxKEYBOARD_KEY_J:             {eResult = AKEYCODE_J; break;}
    case orxKEYBOARD_KEY_K:             {eResult = AKEYCODE_K; break;}
    case orxKEYBOARD_KEY_L:             {eResult = AKEYCODE_L; break;}
    case orxKEYBOARD_KEY_M:             {eResult = AKEYCODE_M; break;}
    case orxKEYBOARD_KEY_W:             {eResult = AKEYCODE_W; break;}
    case orxKEYBOARD_KEY_X:             {eResult = AKEYCODE_X; break;}
    case orxKEYBOARD_KEY_C:             {eResult = AKEYCODE_C; break;}
    case orxKEYBOARD_KEY_V:             {eResult = AKEYCODE_V; break;}
    case orxKEYBOARD_KEY_B:             {eResult = AKEYCODE_B; break;}
    case orxKEYBOARD_KEY_N:             {eResult = AKEYCODE_N; break;}
    case orxKEYBOARD_KEY_0:             {eResult = AKEYCODE_0; break;}
    case orxKEYBOARD_KEY_1:             {eResult = AKEYCODE_1; break;}
    case orxKEYBOARD_KEY_2:             {eResult = AKEYCODE_2; break;}
    case orxKEYBOARD_KEY_3:             {eResult = AKEYCODE_3; break;}
    case orxKEYBOARD_KEY_4:             {eResult = AKEYCODE_4; break;}
    case orxKEYBOARD_KEY_5:             {eResult = AKEYCODE_5; break;}
    case orxKEYBOARD_KEY_6:             {eResult = AKEYCODE_6; break;}
    case orxKEYBOARD_KEY_7:             {eResult = AKEYCODE_7; break;}
    case orxKEYBOARD_KEY_8:             {eResult = AKEYCODE_8; break;}
    case orxKEYBOARD_KEY_9:             {eResult = AKEYCODE_9; break;}
    default:                            {eResult = AKEYCODE_UNKNOWN; break;}
  }

  /* Done! */
  return eResult;
}

static orxSTATUS orxFASTCALL orxKeyboard_Android_EventHandler(const orxEVENT *_pstEvent)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  AInputEvent *pstEventKey;

  /* Gets payload */
  pstEventKey = (AInputEvent *) _pstEvent->pstPayload;

  switch(AKeyEvent_getAction(pstEventKey))
  {
    case AKEY_EVENT_ACTION_DOWN:
      sstKeyboard.abKeyPressed[(orxU32)AKeyEvent_getKeyCode(pstEventKey)] = orxTRUE;
      break;
    case AKEY_EVENT_ACTION_UP:
      sstKeyboard.abKeyPressed[(orxU32)AKeyEvent_getKeyCode(pstEventKey)] = orxFALSE;
      break;
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
    if((eResult = orxEvent_AddHandler((orxEVENT_TYPE) (orxEVENT_TYPE_FIRST_RESERVED + 2), orxKeyboard_Android_EventHandler)) != orxSTATUS_FAILURE)
    {
      int i;
      for(i = 0; i < AKEYCODE_BUTTON_MODE + 1; i++)
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
    orxEvent_RemoveHandler((orxEVENT_TYPE) (orxEVENT_TYPE_FIRST_RESERVED + 2), orxKeyboard_Android_EventHandler);

    /* Cleans static controller */
    orxMemory_Zero(&sstKeyboard, sizeof(orxKEYBOARD_STATIC));
  }

  return;
}

extern "C" orxBOOL orxFASTCALL orxKeyboard_Android_IsKeyPressed(orxKEYBOARD_KEY _eKey)
{
  int32_t eAKey;
  orxBOOL bResult;

  /* Checks */
  orxASSERT(_eKey < orxKEYBOARD_KEY_NUMBER);
  orxASSERT((sstKeyboard.u32Flags & orxKEYBOARD_KU32_STATIC_FLAG_READY) == orxKEYBOARD_KU32_STATIC_FLAG_READY);

  /* Gets NvEvent key enum */
  eAKey = orxKeyboard_Android_GetAKey(_eKey);

  /* Valid? */
  if(eAKey != AKEYCODE_UNKNOWN)
  {
    /* Updates result */
    bResult = sstKeyboard.abKeyPressed[eAKey];
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


extern "C" orxKEYBOARD_KEY orxFASTCALL orxKeyboard_Android_Read()
{
  orxU32          i;
  orxKEYBOARD_KEY eResult = orxKEYBOARD_KEY_NONE;

  /* Checks */
  orxASSERT((sstKeyboard.u32Flags & orxKEYBOARD_KU32_STATIC_FLAG_READY) == orxKEYBOARD_KU32_STATIC_FLAG_READY);

  /* For all keys */
  for(i = 0; i < orxKEYBOARD_KEY_NUMBER; i++)
  {
    /* Is pressed? */
    if(orxKeyboard_Android_IsKeyPressed((orxKEYBOARD_KEY)i) != orxFALSE)
    {
      /* Updates result */
      eResult = (orxKEYBOARD_KEY)i;

      break;
    }
  }

  /* Done! */
  return eResult;
}

extern "C" orxBOOL orxFASTCALL orxKeyboard_Android_Hit()
{
  orxBOOL bResult;

  /* Checks */
  orxASSERT((sstKeyboard.u32Flags & orxKEYBOARD_KU32_STATIC_FLAG_READY) == orxKEYBOARD_KU32_STATIC_FLAG_READY);

  /* Updates result */
  bResult = (orxKeyboard_Android_Read() != orxKEYBOARD_KEY_NONE) ? orxTRUE : orxFALSE;

  /* Done! */
  return bResult;
}

extern "C" void orxFASTCALL orxKeyboard_Android_ClearBuffer()
{
  /* Checks */
  orxASSERT((sstKeyboard.u32Flags & orxKEYBOARD_KU32_STATIC_FLAG_READY) == orxKEYBOARD_KU32_STATIC_FLAG_READY);

  /* Not implemented yet */
  orxASSERT(orxFALSE && "Not implemented yet!");

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
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxKeyboard_Android_Hit, KEYBOARD, HIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxKeyboard_Android_Read, KEYBOARD, READ);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxKeyboard_Android_ClearBuffer, KEYBOARD, CLEAR_BUFFER);
orxPLUGIN_USER_CORE_FUNCTION_END();
