/**
 * \file joystick.c
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

#include "io/joystick.h"

#include "plugin/orxPluginCore.h"


/********************
 *  Plugin Related  *
 ********************/

orxSTATIC orxCONST orxPLUGIN_CORE_FUNCTION sastJoystickPluginFunctionInfo[orxPLUGIN_FUNCTION_BASE_ID_JOYSTICK_NUMBER] =
{
  {(orxPLUGIN_FUNCTION *) &joystick_init,                orxPLUGIN_FUNCTION_BASE_ID_JOYSTICK_INIT},
  {(orxPLUGIN_FUNCTION *) &joystick_exit,                orxPLUGIN_FUNCTION_BASE_ID_JOYSTICK_EXIT}
};

orxVOID joystick_plugin_init()
{
  /* Plugin init */
  orxPlugin_AddCoreInfo(orxPLUGIN_CORE_ID_JOYSTICK, sastJoystickPluginFunctionInfo, sizeof(sastJoystickPluginFunctionInfo) / sizeof(orxPLUGIN_CORE_FUNCTION));

  return;
}



/********************
 *   Core Related   *
 ********************/

orxPLUGIN_DEFINE_CORE_FUNCTION(joystick_init, orxU32);
orxPLUGIN_DEFINE_CORE_FUNCTION(joystick_exit, orxVOID);
