/**
 * \file joystick_plug.c
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

#include "include.h"

#include "plugin/plugin_user.h"


/********************
 *   Core Related   *
 ********************/


uint32 joystick_init()
{
  return EXIT_FAILURE;
}

void joystick_exit()
{
  return;
}



/********************
 *  Plugin Related  *
 ********************/

static plugin_user_st_function_info joystick_plug_spst_function[PLUGIN_JOYSTICK_KUL_FUNCTION_NUMBER];

void plugin_init(int32 *_pi_fn_number, plugin_user_st_function_info **_ppst_fn_info)
{
  PLUGIN_USER_FUNCTION_START(joystick_plug_spst_function);

  PLUGIN_USER_CORE_FUNCTION_ADD(joystick_init, JOYSTICK, INIT);

  PLUGIN_USER_CORE_FUNCTION_ADD(joystick_exit, JOYSTICK, EXIT);

  PLUGIN_USER_FUNCTION_END(_pi_fn_number, _ppst_fn_info);

  return;
}
