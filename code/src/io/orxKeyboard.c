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
 * @file orxKeyboard.c
 * @date 22/11/2003
 * @author iarwain@orx-project.org
 *
 * @todo
 */


#include "io/orxKeyboard.h"
#include "plugin/orxPluginCore.h"
#include "debug/orxDebug.h"


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/


/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** Keyboard module setup
 */
orxVOID orxKeyboard_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_KEYBOARD, orxMODULE_ID_PLUGIN);
  orxModule_AddDependency(orxMODULE_ID_KEYBOARD, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_KEYBOARD, orxMODULE_ID_BANK);
  orxModule_AddDependency(orxMODULE_ID_KEYBOARD, orxMODULE_ID_LINKLIST);
  orxModule_AddDependency(orxMODULE_ID_KEYBOARD, orxMODULE_ID_TREE);

  return;
}

/** Gets key literal name
 * @param _eKey       Concerned key
 * @return Key's name
 */
orxSTRING orxFASTCALL orxKeyboard_GetKeyName(orxKEYBOARD_KEY _eKey)
{
  orxSTRING zResult;

#define orxKEYBOARD_DECLARE_KEY_NAME(KEY)   case orxKEYBOARD_##KEY: zResult = #KEY; break

  /* Checks */
  orxASSERT(_eKey < orxKEYBOARD_KEY_NUMBER);

  /* Depending on key */
  switch(_eKey)
  {
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_A);
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_B);
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_C);
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_D);
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_E);
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_F);
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_G);
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_H);
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_I);
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_J);
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_K);
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_L);
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_M);
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_N);
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_O);
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_P);
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_Q);
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_R);
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_S);
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_T);
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_U);
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_V);
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_W);
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_X);
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_Y);
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_Z);
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_0);
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_1);
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_2);
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_3);
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_4);
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_5);
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_6);
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_7);
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_8);
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_9);
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_NUMPAD0);
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_NUMPAD1);
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_NUMPAD2);
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_NUMPAD3);
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_NUMPAD4);
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_NUMPAD5);
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_NUMPAD6);
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_NUMPAD7);
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_NUMPAD8);
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_NUMPAD9);
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_F1);
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_F2);
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_F3);
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_F4);
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_F5);
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_F6);
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_F7);
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_F8);
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_F9);
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_F10);
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_F11);
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_F12);
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_F13);
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_F14);
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_F15);
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_LEFT);
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_RIGHT);
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_UP);
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_DOWN);
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_ESCAPE);
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_SPACE);
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_RETURN);
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_BACK);
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_TAB);
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_PAGEUP);
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_PAGEDOWN);
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_END);
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_HOME);
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_INSERT);
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_DELETE);
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_ADD);
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_SUBTRACT);
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_MULTIPLY);
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_DIVIDE);
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_PAUSE);
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_LEFT_CONTROL);
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_LEFT_SHIFT);
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_LEFT_ALT);
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_LEFT_SYSTEM);
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_RIGHT_CONTROL);
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_RIGHT_SHIFT);
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_RIGHT_ALT);
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_RIGHT_SYSTEM);
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_MENU);
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_LEFT_BRACKET);
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_RIGHT_BRACKET);
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_SEMICOLON);
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_COMMA);
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_PERIOD);
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_QUOTE);
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_SLASH);
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_BACKSLASH);
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_TILDE);
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_EQUAL);
    orxKEYBOARD_DECLARE_KEY_NAME(KEY_DASH);

    default:
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_KEYBOARD, "No name defined for key #%ld.", _eKey);

      /* Updates result */
      zResult = orxSTRING_EMPTY;
    }
  }

  /* Done! */
  return zResult;
}


/***************************************************************************
 * Plugin related                                                          *
 ***************************************************************************/

/* *** Core function definitions *** */

orxPLUGIN_DEFINE_CORE_FUNCTION(orxKeyboard_Init, orxSTATUS, orxVOID);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxKeyboard_Exit, orxVOID, orxVOID);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxKeyboard_IsKeyPressed, orxBOOL, orxKEYBOARD_KEY);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxKeyboard_Hit, orxBOOL, orxVOID);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxKeyboard_Read, orxKEYBOARD_KEY, orxVOID);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxKeyboard_ClearBuffer, orxVOID, orxVOID);


/* *** Core function info array *** */

orxPLUGIN_BEGIN_CORE_FUNCTION_ARRAY(KEYBOARD)

orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(KEYBOARD, INIT, orxKeyboard_Init)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(KEYBOARD, EXIT, orxKeyboard_Exit)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(KEYBOARD, IS_KEY_PRESSED, orxKeyboard_IsKeyPressed)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(KEYBOARD, HIT, orxKeyboard_Hit)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(KEYBOARD, READ, orxKeyboard_Read)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(KEYBOARD, CLEAR_BUFFER, orxKeyboard_ClearBuffer)

orxPLUGIN_END_CORE_FUNCTION_ARRAY(KEYBOARD)


/* *** Core function implementations *** */

/** Inits the keyboard module
 */
orxSTATUS orxKeyboard_Init()
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxKeyboard_Init)();
}

/** Exits from the keyboard module
 */
orxVOID orxKeyboard_Exit()
{
  orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxKeyboard_Exit)();
}

/** Is key pressed?
 * @param _eKey       Key to check
 * @return orxTRUE if presse / orxFALSE otherwise
 */
orxBOOL orxKeyboard_IsKeyPressed(orxKEYBOARD_KEY _eKey)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxKeyboard_IsKeyPressed)(_eKey);
}

/** Returns orxTRUE if there are keypresses waiting in the input buffer.
 * @return orxTRUE if keys have been pressed, else orxFALSE
 */
orxBOOL orxKeyboard_Hit()
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxKeyboard_Hit)();
}

/** Returns the next character from the keyboard buffer, in ASCII format.
 * If the buffer is empty, it waits until a key is pressed.
 * The low byte of the return value contains the ASCII code of the key,
 * and the high byte the scancode.
 * @return Ascii and scancode value
 */
orxKEYBOARD_KEY orxKeyboard_Read()
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxKeyboard_Read)();
}

/**  Empties the keyboard buffer.
 */
orxVOID orxKeyboard_ClearBuffer()
{
  orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxKeyboard_ClearBuffer)();
}
