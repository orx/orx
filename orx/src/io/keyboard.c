/**
 * \file keyboard.c
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

#include "io/keyboard.h"

#include "plugin/plugin_core.h"


/********************
 *  Plugin Related  *
 ********************/

static plugin_core_st_function plugin_keyboard_spst_function[PLUGIN_KEYBOARD_KU32_FUNCTION_NUMBER] =
{
  {(plugin_function *) &keyboard_init,                PLUGIN_KEYBOARD_KU32_ID_INIT},
  {(plugin_function *) &keyboard_exit,                PLUGIN_KEYBOARD_KU32_ID_EXIT},
  {(plugin_function *) &keyboard_hit,                 PLUGIN_KEYBOARD_KU32_ID_HIT},
  {(plugin_function *) &keyboard_read,                PLUGIN_KEYBOARD_KU32_ID_READ},
  {(plugin_function *) &keyboard_keystate_get,        PLUGIN_KEYBOARD_KU32_ID_KEYSTATE_GET},
  {(plugin_function *) &keyboard_buffer_clear,        PLUGIN_KEYBOARD_KU32_ID_BUFFER_CLEAR}
};

orxVOID keyboard_plugin_init()
{
  /* Plugin init */
  plugin_core_info_add(PLUGIN_KEYBOARD_KU32_PLUGIN_ID, plugin_keyboard_spst_function, PLUGIN_KEYBOARD_KU32_FUNCTION_NUMBER);

  return;
}



/********************
 *   Core Related   *
 ********************/

PLUGIN_CORE_FUNCTION_DEFINE(keyboard_init, orxU32);
PLUGIN_CORE_FUNCTION_DEFINE(keyboard_exit, orxVOID);

PLUGIN_CORE_FUNCTION_DEFINE(keyboard_hit, orxS32);
PLUGIN_CORE_FUNCTION_DEFINE(keyboard_read, orxS32);
PLUGIN_CORE_FUNCTION_DEFINE(keyboard_keystate_get, orxS32, orxU8);
PLUGIN_CORE_FUNCTION_DEFINE(keyboard_buffer_clear, orxVOID);
