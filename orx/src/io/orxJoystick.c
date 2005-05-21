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

orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(JOYSTICK, INIT, orxJoystick_Init)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(JOYSTICK, EXIT, orxJoystick_Exit)

orxPLUGIN_END_CORE_FUNCTION_ARRAY()

/* *** Plugin init function *** */
orxVOID orxJoystick_Plugin_Init()
{
  /* Plugin init */
  orxPLUGIN_REGISTER_CORE_INFO(JOYSTICK);

  return;
}
