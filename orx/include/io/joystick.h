/**
 * \file joystick.h
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

#ifndef _JOYSTICK_H_
#define _JOYSTICK_H_

#include "orxInclude.h"


extern orxVOID     joystick_plugin_init();

extern orxU32 (*joystick_init)();
extern orxVOID   (*joystick_exit)();

#endif /* _JOYSTICK_H_ */
