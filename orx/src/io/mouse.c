/**
 * \file mouse.c
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

#include "io/mouse.h"

#include "plugin/plugin_core.h"


/********************
 *  Plugin Related  *
 ********************/

static plugin_core_st_function plugin_mouse_spst_function[PLUGIN_MOUSE_KUL_FUNCTION_NUMBER] =
{
  {(plugin_function *) &mouse_init,                PLUGIN_MOUSE_KUL_ID_INIT},
  {(plugin_function *) &mouse_exit,                PLUGIN_MOUSE_KUL_ID_EXIT},
  {(plugin_function *) &mouse_move_get,            PLUGIN_MOUSE_KUL_ID_MOVE_GET},
};

void mouse_plugin_init()
{
  /* Plugin init */
  plugin_core_info_add(PLUGIN_MOUSE_KUL_PLUGIN_ID, plugin_mouse_spst_function, PLUGIN_MOUSE_KUL_FUNCTION_NUMBER);

  return;
}



/********************
 *   Core Related   *
 ********************/

PLUGIN_CORE_FUNCTION_DEFINE(mouse_init, uint32);
PLUGIN_CORE_FUNCTION_DEFINE(mouse_exit, void);

PLUGIN_CORE_FUNCTION_DEFINE(mouse_move_get, void, int32 *, int32 *);
