/**
 * \file orxJoystick.h
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

#ifndef _orxJOYSTICK_H_
#define _orxJOYSTICK_H_

#include "orxInclude.h"
#include "plugin/orxPluginCore.h"


orxPLUGIN_DECLARE_CORE_FUNCTION_0(orxJoystick_Init, orxSTATUS);
orxPLUGIN_DECLARE_CORE_FUNCTION_0(orxJoystick_Exit, orxVOID);



/** Inits the joystick core plugin.
 */
extern orxVOID   orxJoystick_Plugin_Init();

/** Inits the joystick module.
 * @return The status of the operation.
 */
orxSTATIC orxINLINE orxSTATUS orxDLLAPI orxJoystick_Init()
{
  return orxPLUGIN_BODY_CORE_FUNCTION(orxJoystick_Init)();
}
/** Exits from the joystick module.
 * @return The status of the operation.
 */

orxSTATIC orxINLINE orxVOID orxDLLAPI orxJoystick_Exit()
{
  orxPLUGIN_BODY_CORE_FUNCTION(orxJoystick_Exit)();
}

#endif /* _orxJOYSTICK_H_ */
