/**
 * \file orxKeyboard.h
 */

/***************************************************************************
 begin                : 22/11/2003
 author               : (C) Gdp
 email                : iarwain@ifrance.com
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


/** Init the keyboard core plugin
 */
extern orxVOID orxDLLAPI orxKeyboard_Plugin_Init();


/** Init the keyboard module
 */
orxPLUGIN_DECLARE_CORE_FUNCTION_0(orxKeyboard_Init, orxSTATUS);

orxSTATIC orxINLINE orxSTATUS orxDLLAPI orxKeyboard_Init()
{
  return orxPLUGIN_BODY_CORE_FUNCTION(orxKeyboard_Init)();
}

/** Exit the keyboard module
 */
orxPLUGIN_DECLARE_CORE_FUNCTION_0(orxKeyboard_Exit, orxVOID);

orxSTATIC orxINLINE orxVOID orxDLLAPI orxKeyboard_Exit()
{
  orxPLUGIN_BODY_CORE_FUNCTION(orxKeyboard_Exit)();
}

/** Returns orxTRUE if there are keypresses waiting in the input buffer.
 * @return orxTRUE if keys have been pressed, else orxFALSE
 */
orxPLUGIN_DECLARE_CORE_FUNCTION_0(orxKeyboard_Hit, orxBOOL);

orxSTATIC orxINLINE orxBOOL orxDLLAPI orxKeyboard_Hit()
{
  return orxPLUGIN_BODY_CORE_FUNCTION(orxKeyboard_Hit)();
}

/** Returns the next character from the keyboard buffer, in ASCII format.
 * If the buffer is empty, it waits until a key is pressed.
 * The low byte of the return value contains the ASCII code of the key,
 * and the high byte the scancode.
 * @return Ascii and scancode value
 */
orxPLUGIN_DECLARE_CORE_FUNCTION_0(orxKeyboard_Read, orxS32);

orxSTATIC orxINLINE orxS32 orxDLLAPI orxKeyboard_Read()
{
  return orxPLUGIN_BODY_CORE_FUNCTION(orxKeyboard_Read)();
}

/**  Empties the keyboard buffer.
 */
orxPLUGIN_DECLARE_CORE_FUNCTION_0(orxKeyboard_ClearBuffer, orxVOID);

orxSTATIC orxINLINE orxVOID orxDLLAPI orxKeyboard_ClearBuffer()
{
  orxPLUGIN_BODY_CORE_FUNCTION(orxKeyboard_ClearBuffer)();
}


#endif /* _orxKEYBOARD_H_ */
