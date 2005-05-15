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

/** Init the joystick core plugin
 */
extern orxVOID   orxJoystick_Plugin_Init();

/** Init joystick module
 * @return Returns the status of the operation
 */
extern orxSTATUS orxDLLAPI (*orxJoystick_Init)();

/** Exit joystick module
 */
extern orxVOID   orxDLLAPI (*orxJoystick_Exit)();

#endif /* _orxJOYSTICK_H_ */
