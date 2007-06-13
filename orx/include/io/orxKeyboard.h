/**
 * \file orxKeyboard.h
 */

/***************************************************************************
 begin                : 22/11/2003
 author               : (C) Arcallians
 email                : iarwain@arcallians.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _orxKEYBOARD_H_
#define _orxKEYBOARD_H_

#include "orxInclude.h"
#include "plugin/orxPluginCore.h"


/***************************************************************************
 * Functions directly implemented by orx core
 ***************************************************************************/

/** Keyboard module setup */
extern orxDLLAPI orxVOID                              orxKeyboard_Setup();


/***************************************************************************
 * Functions extended by plugins
 ***************************************************************************/

orxPLUGIN_DECLARE_CORE_FUNCTION(orxKeyboard_Init, orxSTATUS);
orxPLUGIN_DECLARE_CORE_FUNCTION(orxKeyboard_Exit, orxVOID);
orxPLUGIN_DECLARE_CORE_FUNCTION(orxKeyboard_Hit, orxBOOL);
orxPLUGIN_DECLARE_CORE_FUNCTION(orxKeyboard_Read, orxS32);
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
orxSTATIC orxINLINE orxS32 orxKeyboard_Read()
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
