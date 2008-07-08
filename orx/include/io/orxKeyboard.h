/***************************************************************************
 *                                                                         *
 *   This library is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License           *
 *   as published by the Free Software Foundation; either version 2.1      *
 *   of the License, or (at your option) any later version.                *
 *                                                                         *
 ***************************************************************************/

/**
 * @file orxKeyboard.h
 * @date 22/11/2003
 * @author (C) Arcallians
 */

/**
 * @addtogroup IO
 * 
 * Keyboard plugin module
 * Plugin module that handles keyboard peripherals
 *
 * @{
 */


#ifndef _orxKEYBOARD_H_
#define _orxKEYBOARD_H_

#include "orxInclude.h"
#include "plugin/orxPluginCore.h"


/** Key enum
 */
typedef enum __orxKEYBOARD_KEY_t
{
  orxKEYBOARD_KEY_A = 0,
  orxKEYBOARD_KEY_B,
  orxKEYBOARD_KEY_C,
  orxKEYBOARD_KEY_D,
  orxKEYBOARD_KEY_E,
  orxKEYBOARD_KEY_F,
  orxKEYBOARD_KEY_G,
  orxKEYBOARD_KEY_H,
  orxKEYBOARD_KEY_I,
  orxKEYBOARD_KEY_J,
  orxKEYBOARD_KEY_K,
  orxKEYBOARD_KEY_L,
  orxKEYBOARD_KEY_M,
  orxKEYBOARD_KEY_N,
  orxKEYBOARD_KEY_O,
  orxKEYBOARD_KEY_P,
  orxKEYBOARD_KEY_Q,
  orxKEYBOARD_KEY_R,
  orxKEYBOARD_KEY_S,
  orxKEYBOARD_KEY_T,
  orxKEYBOARD_KEY_U,
  orxKEYBOARD_KEY_V,
  orxKEYBOARD_KEY_W,
  orxKEYBOARD_KEY_X,
  orxKEYBOARD_KEY_Y,
  orxKEYBOARD_KEY_Z,
  orxKEYBOARD_KEY_0,
  orxKEYBOARD_KEY_1,
  orxKEYBOARD_KEY_2,
  orxKEYBOARD_KEY_3,
  orxKEYBOARD_KEY_4,
  orxKEYBOARD_KEY_5,
  orxKEYBOARD_KEY_6,
  orxKEYBOARD_KEY_7,
  orxKEYBOARD_KEY_8,
  orxKEYBOARD_KEY_9,
  orxKEYBOARD_KEY_NUMPAD0,
  orxKEYBOARD_KEY_NUMPAD1,
  orxKEYBOARD_KEY_NUMPAD2,
  orxKEYBOARD_KEY_NUMPAD3,
  orxKEYBOARD_KEY_NUMPAD4,
  orxKEYBOARD_KEY_NUMPAD5,
  orxKEYBOARD_KEY_NUMPAD6,
  orxKEYBOARD_KEY_NUMPAD7,
  orxKEYBOARD_KEY_NUMPAD8,
  orxKEYBOARD_KEY_NUMPAD9,
  orxKEYBOARD_KEY_F1,
  orxKEYBOARD_KEY_F2,
  orxKEYBOARD_KEY_F3,
  orxKEYBOARD_KEY_F4,
  orxKEYBOARD_KEY_F5,
  orxKEYBOARD_KEY_F6,
  orxKEYBOARD_KEY_F7,
  orxKEYBOARD_KEY_F8,
  orxKEYBOARD_KEY_F9,
  orxKEYBOARD_KEY_F10,
  orxKEYBOARD_KEY_F11,
  orxKEYBOARD_KEY_F12,
  orxKEYBOARD_KEY_F13,
  orxKEYBOARD_KEY_F14,
  orxKEYBOARD_KEY_F15,
  orxKEYBOARD_KEY_LEFT,
  orxKEYBOARD_KEY_RIGHT,
  orxKEYBOARD_KEY_UP,
  orxKEYBOARD_KEY_DOWN,
  orxKEYBOARD_KEY_ESCAPE,
  orxKEYBOARD_KEY_SPACE,
  orxKEYBOARD_KEY_RETURN,
  orxKEYBOARD_KEY_BACK,
  orxKEYBOARD_KEY_TAB,
  orxKEYBOARD_KEY_PAGEUP,
  orxKEYBOARD_KEY_PAGEDOWN,
  orxKEYBOARD_KEY_END,
  orxKEYBOARD_KEY_HOME,
  orxKEYBOARD_KEY_INSERT,
  orxKEYBOARD_KEY_DELETE,
  orxKEYBOARD_KEY_ADD,
  orxKEYBOARD_KEY_SUBTRACT,
  orxKEYBOARD_KEY_MULTIPLY,
  orxKEYBOARD_KEY_DIVIDE,
  orxKEYBOARD_KEY_PAUSE,
  orxKEYBOARD_KEY_LEFT_CONTROL,
  orxKEYBOARD_KEY_LEFT_SHIFT,
  orxKEYBOARD_KEY_LEFT_ALT,
  orxKEYBOARD_KEY_LEFT_SYSTEM,
  orxKEYBOARD_KEY_RIGHT_CONTROL,
  orxKEYBOARD_KEY_RIGHT_SHIFT,
  orxKEYBOARD_KEY_RIGHT_ALT,
  orxKEYBOARD_KEY_RIGHT_SYSTEM,
  orxKEYBOARD_KEY_MENU,
  orxKEYBOARD_KEY_LEFT_BRACKET,
  orxKEYBOARD_KEY_RIGHT_BRACKET,
  orxKEYBOARD_KEY_SEMICOLON,
  orxKEYBOARD_KEY_COMMA,
  orxKEYBOARD_KEY_PERIOD,
  orxKEYBOARD_KEY_QUOTE,
  orxKEYBOARD_KEY_SLASH,
  orxKEYBOARD_KEY_BACKSLASH,
  orxKEYBOARD_KEY_TILDE,
  orxKEYBOARD_KEY_EQUAL,
  orxKEYBOARD_KEY_DASH,
  orxKEYBOARD_KEY_NUMBER,

  orxKEYBOARD_KEY_NONE = orxENUM_NONE

} orxKEYBOARD_KEY;


/***************************************************************************
 * Functions directly implemented by orx core
 ***************************************************************************/

/** Keyboard module setup
 */
extern orxDLLAPI orxVOID                              orxKeyboard_Setup();


/***************************************************************************
 * Functions extended by plugins
 ***************************************************************************/

orxPLUGIN_DECLARE_CORE_FUNCTION(orxKeyboard_Init, orxSTATUS);
orxPLUGIN_DECLARE_CORE_FUNCTION(orxKeyboard_Exit, orxVOID);
orxPLUGIN_DECLARE_CORE_FUNCTION(orxKeyboard_IsKeyPressed, orxBOOL, orxKEYBOARD_KEY);
orxPLUGIN_DECLARE_CORE_FUNCTION(orxKeyboard_Hit, orxBOOL);
orxPLUGIN_DECLARE_CORE_FUNCTION(orxKeyboard_Read, orxKEYBOARD_KEY);
orxPLUGIN_DECLARE_CORE_FUNCTION(orxKeyboard_ClearBuffer, orxVOID);


/** Init the keyboard module
 */
orxSTATIC orxINLINE orxSTATUS orxKeyboard_Init()
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxKeyboard_Init)();
}

/** Exit the keyboard module
 */
orxSTATIC orxINLINE orxVOID orxKeyboard_Exit()
{
  orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxKeyboard_Exit)();
}

/** Is key pressed?
 * @param _eKey       Key to check
 * @return orxTRUE if presse / orxFALSE otherwise
 */
orxSTATIC orxINLINE orxBOOL orxKeyboard_IsKeyPressed(orxKEYBOARD_KEY _eKey)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxKeyboard_IsKeyPressed)(_eKey);
}

/** Returns orxTRUE if there are keypresses waiting in the input buffer.
 * @return orxTRUE if keys have been pressed, else orxFALSE
 */
orxSTATIC orxINLINE orxBOOL orxKeyboard_Hit()
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxKeyboard_Hit)();
}

/** Returns the next character from the keyboard buffer, in ASCII format.
 * If the buffer is empty, it waits until a key is pressed.
 * The low byte of the return value contains the ASCII code of the key,
 * and the high byte the scancode.
 * @return Ascii and scancode value
 */
orxSTATIC orxINLINE orxKEYBOARD_KEY orxKeyboard_Read()
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxKeyboard_Read)();
}

/**  Empties the keyboard buffer.
 */
orxSTATIC orxINLINE orxVOID orxKeyboard_ClearBuffer()
{
  orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxKeyboard_ClearBuffer)();
}


#endif /* _orxKEYBOARD_H_ */

/** @} */
