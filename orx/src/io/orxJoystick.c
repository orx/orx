/**
 * \file orxJoystick.c
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

#include "io/orxJoystick.h"
#include "plugin/orxPluginCore.h"


/********************
 *  Plugin Related  *
 ********************/

/* *** Core function info array *** */

orxPLUGIN_BEGIN_CORE_FUNCTION_ARRAY(JOYSTICK)

{(orxPLUGIN_FUNCTION *) &_orxCoreFunctionPointer_orxJoystick_Init, orxPLUGIN_FUNCTION_BASE_ID_JOYSTICK_INIT},
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(JOYSTICK, EXIT, orxJoystick_Exit)

orxPLUGIN_END_CORE_FUNCTION_ARRAY(JOYSTICK)


/* *** Core function definitions *** */

orxPLUGIN_DEFINE_CORE_FUNCTION_0(orxJoystick_Init, orxSTATUS);
orxPLUGIN_DEFINE_CORE_FUNCTION_0(orxJoystick_Exit, orxVOID);
