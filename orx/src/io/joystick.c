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

#include "plugin/plugin_core.h"


/********************
 *  Plugin Related  *
 ********************/

static plugin_core_st_function plugin_joystick_spst_function[PLUGIN_JOYSTICK_KU32_FUNCTION_NUMBER] =
{
  {(plugin_function *) &joystick_init,                PLUGIN_JOYSTICK_KU32_ID_INIT},
  {(plugin_function *) &joystick_exit,                PLUGIN_JOYSTICK_KU32_ID_EXIT}
};

orxVOID joystick_plugin_init()
{
  /* Plugin init */
  plugin_core_info_add(PLUGIN_JOYSTICK_KU32_PLUGIN_ID, plugin_joystick_spst_function, PLUGIN_JOYSTICK_KU32_FUNCTION_NUMBER);

  return;
}



/********************
 *   Core Related   *
 ********************/

PLUGIN_CORE_FUNCTION_DEFINE(joystick_init, orxU32);
PLUGIN_CORE_FUNCTION_DEFINE(joystick_exit, orxVOID);
