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


/***************************************************************************
 * Plugin related                                                          *
 ***************************************************************************/

/* *** Core function definitions *** */

orxPLUGIN_DEFINE_CORE_FUNCTION(orxKeyboard_Init, orxSTATUS);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxKeyboard_Exit, orxVOID);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxKeyboard_IsKeyPressed, orxBOOL, orxKEYBOARD_KEY);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxKeyboard_Hit, orxBOOL);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxKeyboard_Read, orxKEYBOARD_KEY);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxKeyboard_ClearBuffer, orxVOID);


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
