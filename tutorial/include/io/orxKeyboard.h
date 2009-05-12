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
 * @file orxKeyboard.h
 * @date 22/11/2003
 * @author iarwain@orx-project.org
 *
 * @todo
 */

/**
 * @addtogroup orxKeyboard
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
  orxKEYBOARD_KEY_UP,
  orxKEYBOARD_KEY_RIGHT,
  orxKEYBOARD_KEY_DOWN,
  orxKEYBOARD_KEY_LEFT,
  orxKEYBOARD_KEY_ESCAPE,
  orxKEYBOARD_KEY_SPACE,
  orxKEYBOARD_KEY_RETURN,
  orxKEYBOARD_KEY_BACKSPACE,
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
  orxKEYBOARD_KEY_RALT,
  orxKEYBOARD_KEY_RCTRL,
  orxKEYBOARD_KEY_RSHIFT,
  orxKEYBOARD_KEY_RSYSTEM,
  orxKEYBOARD_KEY_LALT,
  orxKEYBOARD_KEY_LCTRL,
  orxKEYBOARD_KEY_LSHIFT,
  orxKEYBOARD_KEY_LSYSTEM,
  orxKEYBOARD_KEY_MENU,
  orxKEYBOARD_KEY_LBRACKET,
  orxKEYBOARD_KEY_RBRACKET,
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


/** Event enum
 */
typedef enum __orxKEYBOARD_EVENT_t
{
  orxKEYBOARD_EVENT_KEY_PRESSED = 0,
  orxKEYBOARD_EVENT_KEY_RELEASED,

  orxKEYBOARD_EVENT_NUMBER,

  orxKEYBOARD_EVENT_NONE = orxENUM_NONE

} orxKEYBOARD_EVENT;


/** Keyboard event payload
 */
typedef struct __orxKEYBOARD_EVENT_PAYLOAD_t
{
  orxKEYBOARD_KEY eKey;         /**< Keyboard key : 4 */
  orxU32          u32Unicode;   /**< Key unicode : 8 */

} orxKEYBOARD_EVENT_PAYLOAD;


/***************************************************************************
 * Functions directly implemented by orx core
 ***************************************************************************/

/** Keyboard module setup
 */
extern orxDLLAPI void orxFASTCALL             orxKeyboard_Setup();


/***************************************************************************
 * Functions extended by plugins
 ***************************************************************************/

/** Inits the keyboard module
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxKeyboard_Init();

/** Exits from the keyboard module
 */
extern orxDLLAPI void orxFASTCALL             orxKeyboard_Exit();

/** Is key pressed?
 * @param[in] _eKey       Key to check
 * @return orxTRUE if pressed / orxFALSE otherwise
 */
extern orxDLLAPI orxBOOL orxFASTCALL          orxKeyboard_IsKeyPressed(orxKEYBOARD_KEY _eKey);

/** Returns orxTRUE if there are keypresses waiting in the input buffer.
 * @return orxTRUE if keys have been pressed, else orxFALSE
 */
extern orxDLLAPI orxBOOL orxFASTCALL          orxKeyboard_Hit();

/** Gets the next character from the keyboard buffer
 * @return orxKEYBOARD_KEY
 */
extern orxDLLAPI orxKEYBOARD_KEY orxFASTCALL  orxKeyboard_Read();

/** Empties the keyboard buffer.
 */
extern orxDLLAPI void orxFASTCALL             orxKeyboard_ClearBuffer();

/** Gets key literal name
 * @param[in] _eKey       Concerned key
 * @return Key's name
 */
extern orxDLLAPI const orxSTRING orxFASTCALL  orxKeyboard_GetKeyName(orxKEYBOARD_KEY _eKey);

#endif /* _orxKEYBOARD_H_ */

/** @} */
