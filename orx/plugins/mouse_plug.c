/**
 * \file mouse_plug.c
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

#include <allegro.h>


/********************
 *   Core Related   *
 ********************/


uint32 mouse_init()
{
  return((install_mouse() == -1) ? EXIT_FAILURE : EXIT_SUCCESS);
}

void mouse_exit()
{
  return;
}

void mouse_move_get(int32 *_i_dx, int32 *_i_dy)
{
  get_mouse_mickeys((int *)_i_dx, (int *)_i_dy);

  return;
}



/********************
 *  Plugin Related  *
 ********************/

static plugin_user_st_function_info mouse_plug_spst_function[PLUGIN_MOUSE_KUL_FUNCTION_NUMBER];

void plugin_init(int32 *_pi_fn_number, plugin_user_st_function_info **_ppst_fn_info)
{
  PLUGIN_USER_FUNCTION_START(mouse_plug_spst_function);


  PLUGIN_USER_CORE_FUNCTION_ADD(mouse_init, MOUSE, INIT);

  PLUGIN_USER_CORE_FUNCTION_ADD(mouse_exit, MOUSE, EXIT);

  PLUGIN_USER_CORE_FUNCTION_ADD(mouse_move_get, MOUSE, MOVE_GET);


  PLUGIN_USER_FUNCTION_END(_pi_fn_number, _ppst_fn_info);

  return;
}
